// Copyright (c) 2018 The Vertcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERTCOIN_CRYPTO_VERTHASH_DATFILE_H
#define VERTCOIN_CRYPTO_VERTHASH_DATFILE_H

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <uint256.h>
#include <common/args.h>
#include <sync.h>
#include <streams.h>
#include <util/fs.h>
#include <crypto/tiny_sha3/sha3.h>

class VerthashDatFile
{
public:
    static void CreateMiningDataFile();
    static void DeleteMiningDataFile();
private:
    static RecursiveMutex cs_Datfile;
};

#endif // VERTCOIN_CRYPTO_VERTHASH_DATFILE_H
