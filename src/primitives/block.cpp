// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-present The Bitcoin Core developers
// Copyright (c) 2014-2023 The Vertcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/block.h>

#include <hash.h>
#include <tinyformat.h>
#include <util/strencodings.h>
#include <crypto/common.h>
#include <chainparams.h>
#include <crypto/verthash.h>
#include <crypto/scrypt.h>
#include <crypto/Lyra2RE/Lyra2RE.h>
#include <util/chaintype.h>

#include <memory>
#include <sstream>

uint256 CBlockHeader::GetHash() const
{
    return (HashWriter{} << *this).GetHash();
}

uint256 CBlockHeader::GetPoWHash(const int nHeight) const
{
   uint256 thash;
   char *out = ((char *)(thash.begin()));

   if((Params().GetChainType() == ChainType::TESTNET && nHeight >= VERTHASH_FORKBLOCK_TESTNET) ||
       (Params().GetChainType() == ChainType::MAIN && nHeight >= VERTHASH_FORKBLOCK_MAINNET) ||
       (Params().GetChainType() == ChainType::REGTEST))
   {
       Verthash::Hash(this->begin(), out);
   }
   else if((Params().GetChainType() == ChainType::TESTNET && nHeight > 158220) || nHeight > 1080000)
   {
        lyra2re3_hash(this->begin(), out);
   }
   else if(Params().GetChainType() == ChainType::TESTNET || nHeight >= 347000) // New Lyra2re2 Testnet
   {
        lyra2re2_hash(this->begin(), out);
   }
   else if(nHeight >= 208301)
   {
        lyra2re_hash(this->begin(), out);
   }
   else
   {
        scrypt_N_1_1_256(this->begin(), out, 10);
   }
   return thash;
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
