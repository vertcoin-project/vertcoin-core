// Copyright (c) 2018 The Vertcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERTCOIN_CRYPTO_VERTHASH_DATFILE_H
#define VERTCOIN_CRYPTO_VERTHASH_DATFILE_H

#include <stdint.h>
#include <stdlib.h>
#include <validation.h>
#include <string>
#include <uint256.h>
#include <util.h>
#include <chainparams.h>
#include <streams.h>
#include <fs.h>
#include <crypto/sha3.h>

// By default don't keep a mining datafile
#define DEFAULT_MINING_DATAFILE ""

class VerthashDatFile
{
public:
    static void UpdateMiningDataFile();
private:
    static uint32_t Fnv1a(uint32_t a, uint32_t b);
    static CCriticalSection cs_Datfile;
};

#endif // VERTCOIN_CRYPTO_VERTHASH_DATFILE_H
