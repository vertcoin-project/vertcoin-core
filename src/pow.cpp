// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2014-2017 The Vertcoin Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cmath>

#include "pow.h"

#include "arith_uint256.h"
#include "bignum.h"
#include "chain.h"
#include "primitives/block.h"
#include "uint256.h"

static CBigNum bnProofOfWorkLimit(~uint256_old(0) >> 20);

#define BlocksTargetSpacing     (2.5 * 60) // 2.5 minutes
#define TimeDaySeconds          (60 * 60 * 24)
#define PastSecondsMin          (TimeDaySeconds * 0.25)
#define PastSecondsMax          (TimeDaySeconds * 7)
#define PastBlocksMin           (PastSecondsMin / BlocksTargetSpacing)
#define PastBlocksMax           (PastSecondsMax / BlocksTargetSpacing)

double                          KGWs[(int)PastBlocksMax];

std::vector<CBlockIndex*>       KGWChain;

CCriticalSection                KGW_Locker;

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    const int nHeight = pindexLast->nHeight + 1;

    if(params.testnet) {
        if(nHeight < 2116) {
	    	return GetNextWorkRequired_Bitcoin(pindexLast, pblock, params);
	    }
	    
  	    // testnet to 12 block difficulty adjustment interval
        if(nHeight % 12 != 0)
        {
	        CBigNum bnNew;
	        bnNew.SetCompact(pindexLast->nBits);
	        if (bnNew > bnProofOfWorkLimit) { bnNew = bnProofOfWorkLimit; }
	        return bnNew.GetCompact();
        }
    } else {
	    if (nHeight < 26754) {
	        return GetNextWorkRequired_Bitcoin(pindexLast, pblock, params);
	    } else if (nHeight == 208301) {
       	    return 0x1e0ffff0;
	    }
    }
    
    return KimotoGravityWell(pindexLast, pblock, params);
}

unsigned int GetNextWorkRequired_Bitcoin(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    assert(pindexLast != nullptr);
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

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

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
}

unsigned int KimotoGravityWell(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params) {
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

    LOCK(KGW_Locker);

    LogPrintf("%s: cache tip=%s height=%d version=0x%08x log2_work=%.8g tx=%lu date='%s'\n", __func__,
      BlockReading->GetBlockHash().ToString(), BlockReading->nHeight, BlockReading->nVersion,
      log(BlockReading->nChainWork.getdouble())/log(2.0), (unsigned long)BlockReading->nChainTx,
      DateTimeStrFormat("%Y-%m-%d %H:%M:%S", BlockReading->GetBlockTime()));

    LogPrintf("%s: height=%d, KGWChain.size()=%d\n", __func__, BlockReading->nHeight, KGWChain.size());

    // cachePosition is zero-based index
    // KGWChain.size() is one-based count
    // PastBlocksMax is one-based count
    int cachePosition = KGWChain.size() > 0 ? BlockReading->nHeight - KGWChain[0]->nHeight : 0;

    LogPrintf("%s: height=%d, cachePosition=%d, KGWChain.size()=%d\n", __func__, BlockReading->nHeight, cachePosition, KGWChain.size());

    if(((int)KGWChain.size() - 1 < cachePosition && KGWChain.size() <= PastBlocksMax) || KGWChain.size() == 0) {
        LogPrintf("%s: Resizing KGWChain...\n", __func__);
        KGWChain.resize(KGWChain.size() + 1);
    }

    LogPrintf("%s: height=%d, cachePosition=%d, KGWChain.size()=%d\n", __func__, BlockReading->nHeight, cachePosition, KGWChain.size());

    if (cachePosition >= (int)PastBlocksMax) {
	LogPrintf("%s: Shifting KGWChain... cachePosition:%d, PastBlocksMax:%d\n", __func__, cachePosition, PastBlocksMax);
        for(int i = 0; i < KGWChain.size() - 1; i++){
            // 30785-26753, 30765-26754
            KGWChain[i] = KGWChain[i+1];
        }
    }
 
    KGWChain[cachePosition] = (CBlockIndex *)BlockReading;

    // Init the KGW array 
    if(KGWs[0] <= 0){
        for(unsigned int i = 0; i < PastBlocksMax; i++){
            KGWs[i] = 1 + (0.7084 * std::pow((double(i+1)/double(144)), -1.228));
	}
    }

    if (BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 || (uint64_t)BlockLastSolved->nHeight < PastBlocksMin) { return UintToArith256(params.powLimit).GetCompact(); }

        for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {
            if (PastBlocksMax > 0 && i > PastBlocksMax) { break; }
            PastBlocksMass++;
	
            if (i == 1)        { PastDifficultyAverage.SetCompact(BlockReading->nBits); }
            else                { PastDifficultyAverage = ((CBigNum().SetCompact(BlockReading->nBits) - PastDifficultyAveragePrev) / i) + PastDifficultyAveragePrev; }
            PastDifficultyAveragePrev = PastDifficultyAverage;

            PastRateActualSeconds                        = BlockLastSolved->GetBlockTime() - BlockReading->GetBlockTime();
            PastRateTargetSeconds                        = BlocksTargetSpacing * PastBlocksMass;
            PastRateAdjustmentRatio                        = double(1);
            if (PastRateActualSeconds < 0) { PastRateActualSeconds = 0; }
            if (PastRateActualSeconds != 0 && PastRateTargetSeconds != 0) {
                PastRateAdjustmentRatio                        = double(PastRateTargetSeconds) / double(PastRateActualSeconds);
            }
            EventHorizonDeviation                        = KGWs[PastBlocksMass-1];
            EventHorizonDeviationFast                = EventHorizonDeviation;
            EventHorizonDeviationSlow                = 1 / EventHorizonDeviation;

            if (PastBlocksMass >= PastBlocksMin) {
                    if ((PastRateAdjustmentRatio <= EventHorizonDeviationSlow) || (PastRateAdjustmentRatio >= EventHorizonDeviationFast)) { assert(BlockReading); break; }
            }

            if(KGWChain.size() >= PastBlocksMax) LogPrintf("%s: height=%d, cachePosition=%d, KGWChain.size()=%d\n", __func__, BlockReading->nHeight, cachePosition, KGWChain.size());
            
            if (cachePosition - 1 >= 0){
            	BlockReading = KGWChain[cachePosition - 1]; 
            } else {
                if (BlockReading->pprev == NULL) { assert(BlockReading); break; }
                BlockReading = BlockReading->pprev;
            }
            cachePosition--;
        }

        CBigNum bnNew(PastDifficultyAverage);
        if (PastRateActualSeconds != 0 && PastRateTargetSeconds != 0) {
                bnNew *= PastRateActualSeconds;
                bnNew /= PastRateTargetSeconds;
        }

        if (bnNew > bnProofOfWorkLimit) {
	    bnNew = bnProofOfWorkLimit;
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
