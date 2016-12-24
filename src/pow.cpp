// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "arith_uint256.h"
#include "chain.h"
#include "primitives/block.h"
#include "uint256.h"


unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
        static const int64_t        		BlocksTargetSpacing                        	= 2.5 * 60; // 2.5 minutes
        unsigned int                		TimeDaySeconds                                	= 60 * 60 * 24;
        int64_t                                 PastSecondsMin                                	= TimeDaySeconds * 0.25;
        int64_t                                 PastSecondsMax                                	= TimeDaySeconds * 7;
        uint64_t                                PastBlocksMin                                	= PastSecondsMin / BlocksTargetSpacing;
        uint64_t                                PastBlocksMax                                	= PastSecondsMax / BlocksTargetSpacing;

	int nHeight = pindexLast->nHeight + 1;
	if (Params().NetworkIDString() == CBaseChainParams::TESTNET)
	    if  (nHeight < 2116) {
	    	return GetNextWorkRequired_Bitcoin(pindexLast, pblock, params);
	    }
	    return KimotoGravityWell(pindexLast, pblock, BlocksTargetSpacing, PastBlocksMin, PastBlocksMax);
	}
	else
	{
		if (nHeight < 26754) {
		    return GetNextWorkRequiredBitcoin(pindexLast, pblock, params);
	}	}
		else if (nHeight == 208301) {
	   	    return 0x1e0ffff0;
		}
		return KimotoGravityWell(pindexLast, pblock, BlocksTargetSpacing, PastBlocksMin, PastBlocksMax);
	}
	return KimotoGravityWell(pindexLast, pblock, BlocksTargetSpacing, PastBlocksMin, PastBlocksMax);
}

unsigned int GetNextWorkRequired_Bitcoin(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;

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
    // Go back by what we want to be 14 days worth of blocks
    // Litecoin: This fixes an issue where a 51% attack can change difficulty at will.
    // Go back the full period unless it's the first retarget after genesis. Code courtesy of Art Forz
    int blockstogoback = params.DifficultyAdjustmentInterval()-1;
    if ((pindexLast->nHeight+1) != params.DifficultyAdjustmentInterval())
        blockstogoback = params.DifficultyAdjustmentInterval();

    // Go back by what we want to be 14 days worth of blocks
    const CBlockIndex* pindexFirst = pindexLast;
    for (int i = 0; pindexFirst && i < blockstogoback; i++)
        pindexFirst = pindexFirst->pprev;

    assert(pindexFirst);

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
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
    // Litecoin: intermediate uint256 can overflow by 1 bit
    bool fShift = bnNew.bits() > 235;
    if (fShift)
        bnNew >>= 1;
    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;
    if (fShift)
        bnNew <<= 1;

    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}



unsigned int static KimotoGravityWell(const CBlockIndex* pindexLast, const CBlockHeader *pblock, uint64_t TargetBlocksSpacingSeconds, uint64_t PastBlocksMin, uint64_t PastBlocksMax)
{
   /* current difficulty formula, megacoin - kimoto gravity well */
   const CBlockIndex *BlockLastSolved = pindexLast;
   const CBlockIndex *BlockReading = pindexLast;
   const CBlockHeader *BlockCreating = pblock;

   BlockCreating = BlockCreating;

   uint64_t PastBlocksMass = 0;
   int64_t PastRateActualSeconds = 0;
   int64_t PastRateTargetSeconds = 0;
   double PastRateAdjustmentRatio = double(1);
   arith_uint256 PastDifficultyAverage;
   arith_uint256 PastDifficultyAveragePrev;
   double EventHorizonDeviation;
   double EventHorizonDeviationFast;
   double EventHorizonDeviationSlow;
   const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);

   if (Params().NetworkIDString() == CBaseChainParams::TESTNET)
   {
        if (pindexLast->nHeight + 1 == 100)
            return 0x1e0ffff0;
        }

        // testnet to 12 block difficulty adjustment interval
        if ((pindexLast->nHeight+1) % nKGWInterval != 0)
        {
                arith_uint256 bnNew;
                bnNew.SetCompact(pindexLast->nBits);
                if (bnNew > bnPowLimit) { bnNew = bnPowLimit; }
                return bnNew.GetCompact();
        }
   }

   if (BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 || (uint64_t)BlockLastSolved->nHeight < PastBlocksMin) { return bnPowLimit.GetCompact() }
   for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {
        if (PastBlocksMax > 0 && i > PastBlocksMax) { break; }
            PastBlocksMass++;

	if (i == 1) { PastDifficultyAverage.SetCompact(BlockReading->nBits); }
        else { PastDifficultyAverage = ((CBigNum().SetCompact(BlockReading->nBits) - PastDifficultyAveragePrev) / i) + PastDifficultyAveragePrev; }

	PastDifficultyAveragePrev = PastDifficultyAverage;
        PastRateActualSeconds = BlockLastSolved->GetBlockTime() - BlockReading->GetBlockTime();
        PastRateTargetSeconds = TargetBlocksSpacingSeconds * PastBlocksMass;
        PastRateAdjustmentRatio = double(1);

        if (PastRateActualSeconds < 0) { PastRateActualSeconds = 0; }
        if (PastRateActualSeconds != 0 && PastRateTargetSeconds != 0) {
            PastRateAdjustmentRatio = double(PastRateTargetSeconds) / double(PastRateActualSeconds);
        }

        EventHorizonDeviation = 1 + (0.7084 * pow((double(PastBlocksMass)/double(144)), -1.228));
        EventHorizonDeviationFast = EventHorizonDeviation;
        EventHorizonDeviationSlow = 1 / EventHorizonDeviation;

        if (PastBlocksMass >= PastBlocksMin) {
            if ((PastRateAdjustmentRatio <= EventHorizonDeviationSlow) || (PastRateAdjustmentRatio >= EventHorizonDeviationFast)) { assert(BlockReading); break; }
        }

        if (BlockReading->pprev == NULL) { assert(BlockReading); break; }
                 BlockReading = BlockReading->pprev;
        }

        arith_uint256 bnNew(PastDifficultyAverage);
        if (PastRateActualSeconds != 0 && PastRateTargetSeconds != 0) {
             bnNew *= PastRateActualSeconds;
             bnNew /= PastRateTargetSeconds;
        }

	if (bnNew > bnPowLimit)
	    bnNew = bnPowLimit;

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
