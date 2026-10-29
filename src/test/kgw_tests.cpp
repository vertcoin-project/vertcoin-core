// Copyright (c) 2026 The Vertcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <bignum.h>
#include <chain.h>
#include <chainparams.h>
#include <crypto/verthash.h>
#include <pow.h>
#include <test/util/setup_common.h>
#include <util/chaintype.h>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

namespace {

/**
 * Kimoto Gravity Well as it was written when the 32.x rebase started, kept
 * as the reference that the optimised KimotoGravityWell() must match exactly.
 * Only the proof-of-work limits are built per call instead of once per
 * process, so that one test binary can check more than one chain.
 */
unsigned int ReferenceKimotoGravityWell(const CBlockIndex* pindexLast, uint64_t TargetBlocksSpacingSeconds, uint64_t PastBlocksMin, uint64_t PastBlocksMax, const Consensus::Params& params)
{
    const CBlockIndex* BlockLastSolved = pindexLast;
    const CBlockIndex* BlockReading = pindexLast;
    uint64_t PastBlocksMass = 0;
    int64_t PastRateActualSeconds = 0;
    int64_t PastRateTargetSeconds = 0;
    double PastRateAdjustmentRatio = double(1);
    CBigNum PastDifficultyAverage;
    CBigNum PastDifficultyAveragePrev;
    double EventHorizonDeviation;
    double EventHorizonDeviationFast;
    double EventHorizonDeviationSlow;
    const CBigNum bnProofOfWorkLimit(params.powLimit);
    const CBigNum bnPreVerthashProofOfWorkLimit(params.preVerthashPowLimit);

    if (BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 || static_cast<uint64_t>(BlockLastSolved->nHeight) < PastBlocksMin) { return UintToArith256(params.powLimit).GetCompact(); }

    for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {
        if (PastBlocksMax > 0 && i > PastBlocksMax) { break; }
        PastBlocksMass++;

        if (i == 1) {
            PastDifficultyAverage.SetCompact(BlockReading->nBits);
        } else {
            PastDifficultyAverage = ((CBigNum().SetCompact(BlockReading->nBits) - PastDifficultyAveragePrev) / i) + PastDifficultyAveragePrev;
        }
        PastDifficultyAveragePrev = PastDifficultyAverage;

        PastRateActualSeconds = BlockLastSolved->GetBlockTime() - BlockReading->GetBlockTime();
        PastRateTargetSeconds = TargetBlocksSpacingSeconds * PastBlocksMass;
        PastRateAdjustmentRatio = double(1);
        if (PastRateActualSeconds < 0) { PastRateActualSeconds = 0; }
        if (PastRateActualSeconds != 0 && PastRateTargetSeconds != 0) {
            PastRateAdjustmentRatio = double(PastRateTargetSeconds) / double(PastRateActualSeconds);
        }
        EventHorizonDeviation = 1 + (0.7084 * std::pow((double(PastBlocksMass)/double(144)), -1.228));
        EventHorizonDeviationFast = EventHorizonDeviation;
        EventHorizonDeviationSlow = 1 / EventHorizonDeviation;

        if (PastBlocksMass >= PastBlocksMin) {
            if ((PastRateAdjustmentRatio <= EventHorizonDeviationSlow) || (PastRateAdjustmentRatio >= EventHorizonDeviationFast)) { break; }
        }
        if (BlockReading->pprev == NULL ||
            (Params().GetChainType() == ChainType::MAIN && (BlockReading->nHeight == 1080000 || BlockReading->nHeight == VERTHASH_FORKBLOCK_MAINNET))) {
            break;
        }
        BlockReading = BlockReading->pprev;
    }

    CBigNum bnNew(PastDifficultyAverage);
    if (PastRateActualSeconds != 0 && PastRateTargetSeconds != 0) {
        bnNew *= PastRateActualSeconds;
        bnNew /= PastRateTargetSeconds;
    }

    if (Params().GetChainType() == ChainType::MAIN) {
        if (static_cast<uint64_t>(BlockLastSolved->nHeight) + 1 >= VERTHASH_FORKBLOCK_MAINNET) {
            if (bnNew > bnProofOfWorkLimit) return bnProofOfWorkLimit.GetCompact();
        } else if (bnNew > bnPreVerthashProofOfWorkLimit) {
            return bnPreVerthashProofOfWorkLimit.GetCompact();
        }
    } else if (Params().GetChainType() == ChainType::TESTNET) {
        if (static_cast<uint64_t>(BlockLastSolved->nHeight) + 1 >= VERTHASH_FORKBLOCK_TESTNET) {
            if (bnNew > bnProofOfWorkLimit) return bnProofOfWorkLimit.GetCompact();
        } else if (bnNew > bnPreVerthashProofOfWorkLimit) {
            return bnPreVerthashProofOfWorkLimit.GetCompact();
        }
    } else if (bnNew > bnProofOfWorkLimit) {
        bnNew = bnProofOfWorkLimit;
    }

    return bnNew.GetCompact();
}

/** How the block times of a random chain advance. */
enum class Spacing { STEADY, JITTER, ERRATIC, BURSTS };

/**
 * Fill blocks with a chain starting at start_height whose difficulty takes a
 * random walk and whose timestamps advance according to spacing, sometimes
 * going backwards.
 */
void BuildRandomChain(std::vector<CBlockIndex>& blocks, int start_height, Spacing spacing, FastRandomContext& rng)
{
    uint32_t exponent{0x1b};
    uint32_t mantissa{0x400000};
    int64_t time{1'400'000'000};
    for (size_t i{0}; i < blocks.size(); ++i) {
        CBlockIndex& block{blocks[i]};
        block.nHeight = start_height + static_cast<int>(i);
        block.pprev = i == 0 ? nullptr : &blocks[i - 1];
        // A random walk of the target, mostly small steps, with an occasional
        // jump of a byte of exponent either way.
        mantissa = static_cast<uint32_t>(std::clamp<int64_t>(int64_t{mantissa} + static_cast<int64_t>(rng.randrange(0x20000)) - 0x10000, 0x008000, 0x7fffff));
        if (rng.randrange(500) == 0) exponent = std::clamp<uint32_t>(exponent + rng.randrange(3) - 1, 0x19, 0x1e);
        block.nBits = (exponent << 24) | mantissa;
        switch (spacing) {
        case Spacing::STEADY: time += 150; break;
        case Spacing::JITTER: time += 140 + static_cast<int64_t>(rng.randrange(21)); break;
        case Spacing::ERRATIC: time += static_cast<int64_t>(rng.randrange(900)) - 100; break;
        case Spacing::BURSTS: time += rng.randrange(20) == 0 ? 3000 : static_cast<int64_t>(rng.randrange(60)); break;
        }
        block.nTime = static_cast<uint32_t>(time);
    }
}

/** Check KimotoGravityWell() against the reference on many tips of random chains. */
void CheckAgainstReference(int start_height, size_t length, uint64_t past_blocks_min, uint64_t past_blocks_max, const Consensus::Params& params, FastRandomContext& rng)
{
    for (const Spacing spacing : {Spacing::STEADY, Spacing::JITTER, Spacing::ERRATIC, Spacing::BURSTS}) {
        std::vector<CBlockIndex> blocks(length);
        BuildRandomChain(blocks, start_height, spacing, rng);
        // Every tip near the start of the chain, where the walk reaches the
        // first block, then a sample of the rest.
        for (size_t i{0}; i < blocks.size(); i += i < 300 ? 1 : 1 + rng.randrange(40)) {
            const CBlockIndex* tip{&blocks[i]};
            BOOST_CHECK_EQUAL(KimotoGravityWell(tip, nullptr, 150, past_blocks_min, past_blocks_max, params),
                              ReferenceKimotoGravityWell(tip, 150, past_blocks_min, past_blocks_max, params));
        }
    }
}

struct TestnetSetup : public BasicTestingSetup {
    TestnetSetup() : BasicTestingSetup{ChainType::TESTNET} {}
};

} // namespace

BOOST_FIXTURE_TEST_SUITE(kgw_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(kgw_matches_reference_mainnet)
{
    const auto& params{Params().GetConsensus()};
    // Ordinary heights, then chains that cross the forks KGW does not look past.
    CheckAgainstReference(26'754, 5'000, 144, 4032, params, m_rng);
    CheckAgainstReference(2'000'000, 5'000, 144, 4032, params, m_rng);
    CheckAgainstReference(1'080'000 - 2'000, 5'000, 144, 4032, params, m_rng);
    CheckAgainstReference(VERTHASH_FORKBLOCK_MAINNET - 2'000, 5'000, 144, 4032, params, m_rng);
    // Windows other than the ones Vertcoin uses.
    CheckAgainstReference(2'000'000, 1'000, 10, 100, params, m_rng);
    CheckAgainstReference(2'000'000, 6'000, 144, 5'000, params, m_rng);
    CheckAgainstReference(2'000'000, 300, 144, 0, params, m_rng);
}

BOOST_FIXTURE_TEST_CASE(kgw_matches_reference_testnet, TestnetSetup)
{
    const auto& params{Params().GetConsensus()};
    CheckAgainstReference(2'116, 5'000, 144, 4032, params, m_rng);
    CheckAgainstReference(VERTHASH_FORKBLOCK_TESTNET - 2'000, 5'000, 144, 4032, params, m_rng);
}

BOOST_AUTO_TEST_SUITE_END()
