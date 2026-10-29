// Copyright (c) 2026 The Vertcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <bench/bench.h>
#include <chain.h>
#include <chainparams.h>
#include <pow.h>
#include <test/util/setup_common.h>
#include <util/chaintype.h>

#include <vector>

/**
 * A mainnet chain past the Verthash fork with blocks exactly on target, so
 * that every Kimoto Gravity Well walks back the full 4032 blocks.
 */
static std::vector<CBlockIndex> BuildSteadyChain(size_t length)
{
    std::vector<CBlockIndex> blocks(length);
    for (size_t i{0}; i < blocks.size(); ++i) {
        blocks[i].nHeight = 2'000'000 + static_cast<int>(i);
        blocks[i].pprev = i == 0 ? nullptr : &blocks[i - 1];
        blocks[i].nTime = 1'600'000'000 + 150 * static_cast<uint32_t>(i);
        blocks[i].nBits = 0x1b0ffff0 + static_cast<uint32_t>(i % 1000);
    }
    return blocks;
}

static void KimotoGravityWellFullWindow(benchmark::Bench& bench)
{
    const auto testing_setup{MakeNoLogFileContext<const BasicTestingSetup>(ChainType::MAIN)};
    const auto& params{Params().GetConsensus()};
    const std::vector<CBlockIndex> blocks = BuildSteadyChain(5'000);
    CBlockHeader header;
    bench.run([&] {
        ankerl::nanobench::doNotOptimizeAway(GetNextWorkRequired(&blocks.back(), &header, params));
    });
}

BENCHMARK(KimotoGravityWellFullWindow);
