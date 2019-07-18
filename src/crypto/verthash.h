// Copyright (c) 2018 The Vertcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERTCOIN_CRYPTO_VERTHASH_H
#define VERTCOIN_CRYPTO_VERTHASH_H

#include <stdint.h>
#include <stdlib.h>
#include <validation.h>
#include <string>
#include <uint256.h>
#include <util/system.h>
#include <chainparams.h>
#include <streams.h>
#include <fs.h>
#include <crypto/sha3.h>

#define VERTHASH_FORKBLOCK 208044

/** Default for -blockmaxweight, which controls the range of block weights the mining code will create **/

class Verthash
{
public:
    static void Hash(const char* input, char* output, const int height);
    static bool HasMiningDataFileForHeight(int height);
private:
    static uint32_t Fnv1a(uint32_t a, uint32_t b);
    static std::string FindCacheByHeight(const int height);
};

#endif // VERTCOIN_CRYPTO_VERTHASH_H