// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/block.h>

#include <hash.h>
#include <tinyformat.h>
#include <utilstrencodings.h>
#include <crypto/common.h>
#include <chainparams.h>

uint256 CBlockHeader::GetHash() const
{
    return SerializeHash(*this);
}

uint256 CBlockHeader::GetPoWHash(const int nHeight) const
{
   uint256 thash;
   if((Params().NetworkIDString() == CBaseChainParams::TESTNET && nHeight > 250000) || nHeight > 2000000)
   {
       switch (GetAlgo())
       {
            case ALGO_LYRA2REV3:
                lyra2re3_hash(BEGIN(nVersion), BEGIN(thash));
                break;
            case ALGO_NEWALGO1:
                lyra2re3_hash(BEGIN(nVersion), BEGIN(thash));
                break;
            case ALGO_NEWALGO2:
                lyra2re3_hash(BEGIN(nVersion), BEGIN(thash));
                break;
       }
   }
   else if((Params().NetworkIDString() == CBaseChainParams::TESTNET && nHeight > 158220) || nHeight > 1080000)
   {
        lyra2re3_hash(BEGIN(nVersion), BEGIN(thash));
   }
   else if(Params().NetworkIDString() == CBaseChainParams::TESTNET || nHeight >= 347000) // New Lyra2re2 Testnet
   {
        lyra2re2_hash(BEGIN(nVersion), BEGIN(thash));
   }
   else if(nHeight >= 208301)
   {
   	    lyra2re_hash(BEGIN(nVersion), BEGIN(thash));
   }
   else
   {
   	    scrypt_N_1_1_256(BEGIN(nVersion), BEGIN(thash), 10);
   }
   return thash;
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, pow_algo=%d, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        GetAlgo(),
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}

int GetAlgo(int nVersion)
{
    switch (nVersion & BLOCK_VERSION_ALGO)
    {
        case 0:
            return ALGO_LYRA2REV3;
            break;
        case BLOCK_VERSION_NEWALGO1:
            return ALGO_NEWALGO1;
            break;
        case BLOCK_VERSION_NEWALGO2:
            return ALGO_NEWALGO2;
            break;
    }
    return ALGO_LYRA2REV3;
}

std::string GetAlgoName(int algo)
{
    switch (algo)
    {
            case ALGO_LYRA2REV3:
                return std::string("lyra2re3");
                break;
            case ALGO_NEWALGO1:
                return std::string("newalgo1");
                break;
            case ALGO_NEWALGO2:
                return std::string("newalgo2");
                break;
    }
    return std::string("unknown");
}
