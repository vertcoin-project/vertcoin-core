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
#include <crypto/tiny_sha3/sha3.h>

#define VERTHASH_FORKBLOCK_TESTNET 208553

/** Default for -blockmaxweight, which controls the range of block weights the mining code will create **/

class Verthash
{
public:
    static void Hash(const char* input, char* output);
};

#endif // VERTCOIN_CRYPTO_VERTHASH_H