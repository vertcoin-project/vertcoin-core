// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <cmath>

#include <pow.h>

#include <arith_uint256.h>
#include <bignum.h>
#include <chain.h>
#include <primitives/block.h>
#include <uint256.h>
#include <crypto/verthash.h>

arith_uint256 preVerthashPoWLimit(~arith_uint256(0) >> 20);
CBigNum bnPreVerthashPoWLimit(preVerthashPoWLimit);

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    CBigNum bnProofOfWorkLimit(params.powLimit);
    static const int64_t        BlocksTargetSpacing  = 2.5 * 60; // 2.5 minutes
    unsigned int                TimeDaySeconds       = 60 * 60 * 24;
    int64_t                     PastSecondsMin       = TimeDaySeconds * 0.25;
    int64_t                     PastSecondsMax       = TimeDaySeconds * 7;
    uint64_t                    PastBlocksMin        = PastSecondsMin / BlocksTargetSpacing;
    uint64_t                    PastBlocksMax        = PastSecondsMax / BlocksTargetSpacing;

    const int nHeight = pindexLast->nHeight + 1;

    if(params.testnet) {
        if(nHeight < 2116) {
            return GetNextWorkRequired_Bitcoin(pindexLast, pblock, params);
        } else if(nHeight >= VERTHASH_FORKBLOCK && nHeight < 208054) { // Set diff to mindiff on testnet Verthash fork
            return bnProofOfWorkLimit.GetCompact();
        }

        if(nHeight % 12 != 0) {
            CBigNum bnNew;
	        bnNew.SetCompact(pindexLast->nBits);
            if (nHeight < VERTHASH_FORKBLOCK && bnNew > bnProofOfWorkLimit) { bnNew = bnProofOfWorkLimit; }
            if (nHeight >= VERTHASH_FORKBLOCK && bnNew > bnPreVerthashPoWLimit) { bnNew = bnPreVerthashPoWLimit; }
            return bnNew.GetCompact();
        }
    } else {
        if(nHeight < 26754) {
            return GetNextWorkRequired_Bitcoin(pindexLast, pblock, params);
        } else if(nHeight == 208301) {
            return 0x1e0ffff0;
        } else if(nHeight >= 1080000 && nHeight < 1080010) { // Force difficulty for 10 blocks
            return 0x1b0ffff0;
        }
    }
    return KimotoGravityWell(pindexLast, pblock, BlocksTargetSpacing, PastBlocksMin, PastBlocksMax, params);
}

unsigned int GetNextWorkRequired_Bitcoin(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    assert(pindexLast != nullptr);
    unsigned int nProofOfWorkLimit = bnPreVerthashPoWLimit.GetCompact();

    // Only change once per difficulty adjustment interval
    if ((pindexLast->nHeight+1) % params.DifficultyAdjustmentInterval() != 0)
    {
        if (params.fPowAllowMinDifficultyBlocks)
        {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 10 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }

    // Litecoin: This fixes an issue where a 51% attack can change difficulty at will.
    // Go back the full period unless it's the first retarget after genesis. Code courtesy of Art Forz
    int blockstogoback = params.DifficultyAdjustmentInterval()-1;
    if ((pindexLast->nHeight+1) != params.DifficultyAdjustmentInterval())
        blockstogoback = params.DifficultyAdjustmentInterval();

    // Go back by what we want to be 14 days worth of blocks
    const CBlockIndex* pindexFirst = pindexLast;
    for (int i = 0; pindexFirst && i < blockstogoback; i++)
        pindexFirst = pindexFirst->pprev;

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
}

unsigned int KimotoGravityWell(const CBlockIndex* pindexLast,
                               const CBlockHeader *pblock,
                               uint64_t TargetBlocksSpacingSeconds,
                               uint64_t PastBlocksMin,
                               uint64_t PastBlocksMax,
                               const Consensus::Params& params) {
    /* current difficulty formula - kimoto gravity well */
    const CBlockIndex *BlockLastSolved                                = pindexLast;
    const CBlockIndex *BlockReading                                = pindexLast;
    uint64_t                                PastBlocksMass                                = 0;
    int64_t                                PastRateActualSeconds                = 0;
    int64_t                                PastRateTargetSeconds                = 0;
    double                                PastRateAdjustmentRatio                = double(1);
    CBigNum                                PastDifficultyAverage;
    CBigNum                                PastDifficultyAveragePrev;
    double                                EventHorizonDeviation;
    double                                EventHorizonDeviationFast;
    double                                EventHorizonDeviationSlow;

    CBigNum bnProofOfWorkLimit(params.powLimit);

    if (BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 || (uint64_t)BlockLastSolved->nHeight < PastBlocksMin) {
        if(params.testnet && (uint64_t)BlockLastSolved->nHeight+1 >= 206442) {
            return bnProofOfWorkLimit.GetCompact();
        }
        return bnPreVerthashPoWLimit.GetCompact();
    }

    for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {
        if (PastBlocksMax > 0 && i > PastBlocksMax) { break; }
        PastBlocksMass++;

        if (i == 1)        { PastDifficultyAverage.SetCompact(BlockReading->nBits); }
        else                { PastDifficultyAverage = ((CBigNum().SetCompact(BlockReading->nBits) - PastDifficultyAveragePrev) / i) + PastDifficultyAveragePrev; }
        PastDifficultyAveragePrev = PastDifficultyAverage;

        PastRateActualSeconds                        = BlockLastSolved->GetBlockTime() - BlockReading->GetBlockTime();
        PastRateTargetSeconds                        = TargetBlocksSpacingSeconds * PastBlocksMass;
        PastRateAdjustmentRatio                        = double(1);
        if (PastRateActualSeconds < 0) { PastRateActualSeconds = 0; }
        if (PastRateActualSeconds != 0 && PastRateTargetSeconds != 0) {
            PastRateAdjustmentRatio                        = double(PastRateTargetSeconds) / double(PastRateActualSeconds);
        }
        EventHorizonDeviation                        = 1 + (0.7084 * std::pow((double(PastBlocksMass)/double(144)), -1.228));
        EventHorizonDeviationFast                = EventHorizonDeviation;
        EventHorizonDeviationSlow                = 1 / EventHorizonDeviation;

        if (PastBlocksMass >= PastBlocksMin) {
                if ((PastRateAdjustmentRatio <= EventHorizonDeviationSlow) || (PastRateAdjustmentRatio >= EventHorizonDeviationFast)) { assert(BlockReading); break; }
        }
        if (BlockReading->pprev == NULL ||
            (!params.testnet && BlockReading->nHeight == 1080000) || // Don't calculate past fork block on mainnet
            (!params.testnet && BlockReading->nHeight == 206442)) // Don't calculate past fork block on testnet
        {
                assert(BlockReading);
                break;
        }
        BlockReading = BlockReading->pprev;
    }

    CBigNum bnNew(PastDifficultyAverage);
    if (PastRateActualSeconds != 0 && PastRateTargetSeconds != 0) {
            bnNew *= PastRateActualSeconds;
            bnNew /= PastRateTargetSeconds;
    }

    if(params.testnet && (uint64_t)BlockLastSolved->nHeight+1 >= 206442 && (bnNew > bnProofOfWorkLimit)) {
        return bnProofOfWorkLimit.GetCompact();
    } else if (bnNew > bnPreVerthashPoWLimit) {
        bnNew = bnPreVerthashPoWLimit;
	}

    return bnNew.GetCompact();
}

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    if (nActualTimespan < params.nPowTargetTimespan/4)
        nActualTimespan = params.nPowTargetTimespan/4;
    if (nActualTimespan > params.nPowTargetTimespan*4)
        nActualTimespan = params.nPowTargetTimespan*4;

    // Retarget
    arith_uint256 bnNew;
    arith_uint256 bnOld;
    bnNew.SetCompact(pindexLast->nBits);
    bnOld = bnNew;
    bool fShift = bnNew.bits() > 235;
    if (fShift)
        bnNew >>= 1;
    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;
    if (fShift)
        bnNew <<= 1;

    if (bnNew > preVerthashPoWLimit)
        bnNew = preVerthashPoWLimit;

    return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}
