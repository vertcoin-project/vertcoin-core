// Copyright (c) 2018 The Vertcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERTCOIN_CRYPTO_VERTHASH_H
#define VERTCOIN_CRYPTO_VERTHASH_H

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <uint256.h>
#include <common/args.h>
#include <streams.h>
#include <util/fs.h>
#include <crypto/tiny_sha3/sha3.h>

#define VERTHASH_FORKBLOCK_TESTNET 231000
#define VERTHASH_FORKBLOCK_MAINNET 1500000

/** Default for -blockmaxweight, which controls the range of block weights the mining code will create **/

class Verthash
{
public:
    static void Hash(const char* input, char* output);
    static bool VerifyDatFile();
    static void LoadInRam();
    /** Maps the datafile read-only instead of loading it, so the OS can page it out.
     *  Returns false if it couldn't be mapped; Hash then reads the file instead. */
    static bool MapDatFile();
    /** Takes ownership of data, allocated with malloc, as the datafile in RAM. */
    static void SetDatFile(unsigned char* data, size_t size);
    /** Frees or unmaps the datafile. */
    static void Unload();
    /** Whether the datafile is in memory, loaded into RAM or mapped. */
    static bool IsLoaded();
private:
    static unsigned char *datFile;
    static size_t datFileSize;
    // datFile holds the whole datafile, in RAM or mapped.
    static bool datFileInRam;
    static bool datFileMapped;
};

#endif // VERTCOIN_CRYPTO_VERTHASH_H