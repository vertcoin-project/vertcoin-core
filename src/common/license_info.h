// Copyright (c) The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://opensource.org/license/mit/.

#ifndef BITCOIN_COMMON_LICENSE_INFO_H
#define BITCOIN_COMMON_LICENSE_INFO_H

#include <string>

/** Vertcoin Core copyright line after strPrefix, then the Bitcoin Core one after strPrefixBitcoin */
std::string CopyrightHolders(const std::string& strPrefix, const std::string& strPrefixBitcoin);

/** Returns licensing information (for -version) */
std::string LicenseInfo();

#endif // BITCOIN_COMMON_LICENSE_INFO_H
