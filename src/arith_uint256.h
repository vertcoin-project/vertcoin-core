// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2014-2017 The Vertcoin developers
// Copyright (c) 2014-2018 The Verticalcoin Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_ARITH_UINT256_H
#define BITCOIN_ARITH_UINT256_H

#include <assert.h>
#include <cstring>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>
#include <inttypes.h>
#include <stdio.h>
#include <limits.h>

class uint256;

class uint_error : public std::runtime_error {
public:
    explicit uint_error(const std::string& str) : std::runtime_error(str) {}
};

/** Template base class for unsigned big integers. */
template<unsigned int BITS>
class base_uint
{
protected:
    enum { WIDTH=BITS/32 };
    uint32_t pn[WIDTH];
public:

    base_uint()
    {
        static_assert(BITS/32 > 0 && BITS%32 == 0, "Template parameter BITS must be a positive multiple of 32.");

        for (int i = 0; i < WIDTH; i++)
            pn[i] = 0;
    }

    base_uint(const base_uint& b)
    {
        static_assert(BITS/32 > 0 && BITS%32 == 0, "Template parameter BITS must be a positive multiple of 32.");

        for (int i = 0; i < WIDTH; i++)
            pn[i] = b.pn[i];
    }

    base_uint& operator=(const base_uint& b)
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] = b.pn[i];
        return *this;
    }

    base_uint(uint64_t b)
    {
        static_assert(BITS/32 > 0 && BITS%32 == 0, "Template parameter BITS must be a positive multiple of 32.");

        pn[0] = (unsigned int)b;
        pn[1] = (unsigned int)(b >> 32);
        for (int i = 2; i < WIDTH; i++)
            pn[i] = 0;
    }

    explicit base_uint(const std::string& str);
    explicit base_uint(const std::vector<unsigned char>& vch);

    bool operator!() const
    {
        for (int i = 0; i < WIDTH; i++)
            if (pn[i] != 0)
                return false;
        return true;
    }

    const base_uint operator~() const
    {
        base_uint ret;
        for (int i = 0; i < WIDTH; i++)
            ret.pn[i] = ~pn[i];
        return ret;
    }

    const base_uint operator-() const
    {
        base_uint ret;
        for (int i = 0; i < WIDTH; i++)
            ret.pn[i] = ~pn[i];
        ret++;
        return ret;
    }

    double getdouble() const;

    base_uint& operator=(uint64_t b)
    {
        pn[0] = (unsigned int)b;
        pn[1] = (unsigned int)(b >> 32);
        for (int i = 2; i < WIDTH; i++)
            pn[i] = 0;
        return *this;
    }

    base_uint& operator^=(const base_uint& b)
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] ^= b.pn[i];
        return *this;
    }

    base_uint& operator&=(const base_uint& b)
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] &= b.pn[i];
        return *this;
    }

    base_uint& operator|=(const base_uint& b)
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] |= b.pn[i];
        return *this;
    }

    base_uint& operator^=(uint64_t b)
    {
        pn[0] ^= (unsigned int)b;
        pn[1] ^= (unsigned int)(b >> 32);
        return *this;
    }

    base_uint& operator|=(uint64_t b)
    {
        pn[0] |= (unsigned int)b;
        pn[1] |= (unsigned int)(b >> 32);
        return *this;
    }

    base_uint& operator<<=(unsigned int shift);
    base_uint& operator>>=(unsigned int shift);

    base_uint& operator+=(const base_uint& b)
    {
        uint64_t carry = 0;
        for (int i = 0; i < WIDTH; i++)
        {
            uint64_t n = carry + pn[i] + b.pn[i];
            pn[i] = n & 0xffffffff;
            carry = n >> 32;
        }
        return *this;
    }

    base_uint& operator-=(const base_uint& b)
    {
        *this += -b;
        return *this;
    }

    base_uint& operator+=(uint64_t b64)
    {
        base_uint b;
        b = b64;
        *this += b;
        return *this;
    }

    base_uint& operator-=(uint64_t b64)
    {
        base_uint b;
        b = b64;
        *this += -b;
        return *this;
    }

    base_uint& operator*=(uint32_t b32);
    base_uint& operator*=(const base_uint& b);
    base_uint& operator/=(const base_uint& b);

    base_uint& operator++()
    {
        // prefix operator
        int i = 0;
        while (i < WIDTH && ++pn[i] == 0)
            i++;
        return *this;
    }

    const base_uint operator++(int)
    {
        // postfix operator
        const base_uint ret = *this;
        ++(*this);
        return ret;
    }

    base_uint& operator--()
    {
        // prefix operator
        int i = 0;
        while (i < WIDTH && --pn[i] == (uint32_t)-1)
            i++;
        return *this;
    }

    const base_uint operator--(int)
    {
        // postfix operator
        const base_uint ret = *this;
        --(*this);
        return ret;
    }

    int CompareTo(const base_uint& b) const;
    bool EqualTo(uint64_t b) const;

    friend inline const base_uint operator+(const base_uint& a, const base_uint& b) { return base_uint(a) += b; }
    friend inline const base_uint operator-(const base_uint& a, const base_uint& b) { return base_uint(a) -= b; }
    friend inline const base_uint operator*(const base_uint& a, const base_uint& b) { return base_uint(a) *= b; }
    friend inline const base_uint operator/(const base_uint& a, const base_uint& b) { return base_uint(a) /= b; }
    friend inline const base_uint operator|(const base_uint& a, const base_uint& b) { return base_uint(a) |= b; }
    friend inline const base_uint operator&(const base_uint& a, const base_uint& b) { return base_uint(a) &= b; }
    friend inline const base_uint operator^(const base_uint& a, const base_uint& b) { return base_uint(a) ^= b; }
    friend inline const base_uint operator>>(const base_uint& a, int shift) { return base_uint(a) >>= shift; }
    friend inline const base_uint operator<<(const base_uint& a, int shift) { return base_uint(a) <<= shift; }
    friend inline const base_uint operator*(const base_uint& a, uint32_t b) { return base_uint(a) *= b; }
    friend inline bool operator==(const base_uint& a, const base_uint& b) { return memcmp(a.pn, b.pn, sizeof(a.pn)) == 0; }
    friend inline bool operator!=(const base_uint& a, const base_uint& b) { return memcmp(a.pn, b.pn, sizeof(a.pn)) != 0; }
    friend inline bool operator>(const base_uint& a, const base_uint& b) { return a.CompareTo(b) > 0; }
    friend inline bool operator<(const base_uint& a, const base_uint& b) { return a.CompareTo(b) < 0; }
    friend inline bool operator>=(const base_uint& a, const base_uint& b) { return a.CompareTo(b) >= 0; }
    friend inline bool operator<=(const base_uint& a, const base_uint& b) { return a.CompareTo(b) <= 0; }
    friend inline bool operator==(const base_uint& a, uint64_t b) { return a.EqualTo(b); }
    friend inline bool operator!=(const base_uint& a, uint64_t b) { return !a.EqualTo(b); }

    std::string GetHex() const;
    void SetHex(const char* psz);
    void SetHex(const std::string& str);
    std::string ToString() const;

    unsigned int size() const
    {
        return sizeof(pn);
    }

    /**
     * Returns the position of the highest bit set plus one, or zero if the
     * value is zero.
     */
    unsigned int bits() const;

    uint64_t GetLow64() const
    {
        static_assert(WIDTH >= 2, "Assertion WIDTH >= 2 failed (WIDTH = BITS / 32). BITS is a template parameter.");
        return pn[0] | (uint64_t)pn[1] << 32;
    }
};

/** 256-bit unsigned big integer. */
class arith_uint256 : public base_uint<256> {
public:
    arith_uint256() {}
    arith_uint256(const base_uint<256>& b) : base_uint<256>(b) {}
    arith_uint256(uint64_t b) : base_uint<256>(b) {}
    explicit arith_uint256(const std::string& str) : base_uint<256>(str) {}
    explicit arith_uint256(const std::vector<unsigned char>& vch) : base_uint<256>(vch) {}

    /**
     * The "compact" format is a representation of a whole
     * number N using an unsigned 32bit number similar to a
     * floating point format.
     * The most significant 8 bits are the unsigned exponent of base 256.
     * This exponent can be thought of as "number of bytes of N".
     * The lower 23 bits are the mantissa.
     * Bit number 24 (0x800000) represents the sign of N.
     * N = (-1^sign) * mantissa * 256^(exponent-3)
     *
     * Satoshi's original implementation used BN_bn2mpi() and BN_mpi2bn().
     * MPI uses the most significant bit of the first byte as sign.
     * Thus 0x1234560000 is compact (0x05123456)
     * and  0xc0de000000 is compact (0x0600c0de)
     *
     * Vertcoin only uses this "compact" format for encoding difficulty
     * targets, which are unsigned 256bit quantities.  Thus, all the
     * complexities of the sign bit and using base 256 are probably an
     * implementation accident.
     */
    arith_uint256& SetCompact(uint32_t nCompact, bool *pfNegative = nullptr, bool *pfOverflow = nullptr);
    uint32_t GetCompact(bool fNegative = false) const;

    friend uint256 ArithToUint256(const arith_uint256 &);
    friend arith_uint256 UintToArith256(const uint256 &);
};

uint256 ArithToUint256(const arith_uint256 &);
arith_uint256 UintToArith256(const uint256 &);

/** Base class without constructors for uint256 and uint160.
 * This makes the compiler let you use it in a union.
 */
template<unsigned int BITS>
class base_uint_old
{
protected:
    enum { WIDTH=BITS/32 };
    uint32_t pn[WIDTH];
public:

    bool operator!() const
    {
        for (int i = 0; i < WIDTH; i++)
            if (pn[i] != 0)
                return false;
        return true;
    }

    const base_uint_old operator~() const
    {
        base_uint_old ret;
        for (int i = 0; i < WIDTH; i++)
            ret.pn[i] = ~pn[i];
        return ret;
    }

    const base_uint_old operator-() const
    {
        base_uint_old ret;
        for (int i = 0; i < WIDTH; i++)
            ret.pn[i] = ~pn[i];
        ret++;
        return ret;
    }

    double getdouble() const
    {
        double ret = 0.0;
        double fact = 1.0;
        for (int i = 0; i < WIDTH; i++) {
            ret += fact * pn[i];
            fact *= 4294967296.0;
        }
        return ret;
    }

    base_uint_old& operator=(uint64_t b)
    {
        pn[0] = (unsigned int)b;
        pn[1] = (unsigned int)(b >> 32);
        for (int i = 2; i < WIDTH; i++)
            pn[i] = 0;
        return *this;
    }

    base_uint_old& operator^=(const base_uint_old& b)
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] ^= b.pn[i];
        return *this;
    }

    base_uint_old& operator&=(const base_uint_old& b)
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] &= b.pn[i];
        return *this;
    }

    base_uint_old& operator|=(const base_uint_old& b)
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] |= b.pn[i];
        return *this;
    }

    base_uint_old& operator^=(uint64_t b)
    {
        pn[0] ^= (unsigned int)b;
        pn[1] ^= (unsigned int)(b >> 32);
        return *this;
    }

    base_uint_old& operator|=(uint64_t b)
    {
        pn[0] |= (unsigned int)b;
        pn[1] |= (unsigned int)(b >> 32);
        return *this;
    }

    base_uint_old& operator<<=(unsigned int shift)
    {
        base_uint_old a(*this);
        for (int i = 0; i < WIDTH; i++)
            pn[i] = 0;
        int k = shift / 32;
        shift = shift % 32;
        for (int i = 0; i < WIDTH; i++)
        {
            if (i+k+1 < WIDTH && shift != 0)
                pn[i+k+1] |= (a.pn[i] >> (32-shift));
            if (i+k < WIDTH)
                pn[i+k] |= (a.pn[i] << shift);
        }
        return *this;
    }

    base_uint_old& operator>>=(unsigned int shift)
    {
        base_uint_old a(*this);
        for (int i = 0; i < WIDTH; i++)
            pn[i] = 0;
        int k = shift / 32;
        shift = shift % 32;
        for (int i = 0; i < WIDTH; i++)
        {
            if (i-k-1 >= 0 && shift != 0)
                pn[i-k-1] |= (a.pn[i] << (32-shift));
            if (i-k >= 0)
                pn[i-k] |= (a.pn[i] >> shift);
        }
        return *this;
    }

    base_uint_old& operator+=(const base_uint_old& b)
    {
        uint64_t carry = 0;
        for (int i = 0; i < WIDTH; i++)
        {
            uint64_t n = carry + pn[i] + b.pn[i];
            pn[i] = n & 0xffffffff;
            carry = n >> 32;
        }
        return *this;
    }

    base_uint_old& operator-=(const base_uint_old& b)
    {
        *this += -b;
        return *this;
    }

    base_uint_old& operator+=(uint64_t b64)
    {
        base_uint_old b;
        b = b64;
        *this += b;
        return *this;
    }

    base_uint_old& operator-=(uint64_t b64)
    {
        base_uint_old b;
        b = b64;
        *this += -b;
        return *this;
    }


    base_uint_old& operator++()
    {
        // prefix operator
        int i = 0;
        while (++pn[i] == 0 && i < WIDTH-1)
            i++;
        return *this;
    }

    const base_uint_old operator++(int)
    {
        // postfix operator
        const base_uint_old ret = *this;
        ++(*this);
        return ret;
    }

    base_uint_old& operator--()
    {
        // prefix operator
        int i = 0;
        while (--pn[i] == -1 && i < WIDTH-1)
            i++;
        return *this;
    }

    const base_uint_old operator--(int)
    {
        // postfix operator
        const base_uint_old ret = *this;
        --(*this);
        return ret;
    }


    friend inline bool operator<(const base_uint_old& a, const base_uint_old& b)
    {
        for (int i = base_uint_old::WIDTH-1; i >= 0; i--)
        {
            if (a.pn[i] < b.pn[i])
                return true;
            else if (a.pn[i] > b.pn[i])
                return false;
        }
        return false;
    }

    friend inline bool operator<=(const base_uint_old& a, const base_uint_old& b)
    {
        for (int i = base_uint_old::WIDTH-1; i >= 0; i--)
        {
            if (a.pn[i] < b.pn[i])
                return true;
            else if (a.pn[i] > b.pn[i])
                return false;
        }
        return true;
    }

    friend inline bool operator>(const base_uint_old& a, const base_uint_old& b)
    {
        for (int i = base_uint_old::WIDTH-1; i >= 0; i--)
        {
            if (a.pn[i] > b.pn[i])
                return true;
            else if (a.pn[i] < b.pn[i])
                return false;
        }
        return false;
    }

    friend inline bool operator>=(const base_uint_old& a, const base_uint_old& b)
    {
        for (int i = base_uint_old::WIDTH-1; i >= 0; i--)
        {
            if (a.pn[i] > b.pn[i])
                return true;
            else if (a.pn[i] < b.pn[i])
                return false;
        }
        return true;
    }

    friend inline bool operator==(const base_uint_old& a, const base_uint_old& b)
    {
        for (int i = 0; i < base_uint_old::WIDTH; i++)
            if (a.pn[i] != b.pn[i])
                return false;
        return true;
    }

    friend inline bool operator==(const base_uint_old& a, uint64_t b)
    {
        if (a.pn[0] != (unsigned int)b)
            return false;
        if (a.pn[1] != (unsigned int)(b >> 32))
            return false;
        for (int i = 2; i < base_uint_old::WIDTH; i++)
            if (a.pn[i] != 0)
                return false;
        return true;
    }

    friend inline bool operator!=(const base_uint_old& a, const base_uint_old& b)
    {
        return (!(a == b));
    }

    friend inline bool operator!=(const base_uint_old& a, uint64_t b)
    {
        return (!(a == b));
    }



    std::string GetHex() const
    {
        char psz[sizeof(pn)*2 + 1];
        for (unsigned int i = 0; i < sizeof(pn); i++)
            sprintf(psz + i*2, "%02x", ((unsigned char*)pn)[sizeof(pn) - i - 1]);
        return std::string(psz, psz + sizeof(pn)*2);
    }

    void SetHex(const char* psz)
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] = 0;

        // skip leading spaces
        while (isspace(*psz))
            psz++;

        // skip 0x
        if (psz[0] == '0' && tolower(psz[1]) == 'x')
            psz += 2;

        // hex string to uint
        static const unsigned char phexdigit[256] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0, 0,0xa,0xb,0xc,0xd,0xe,0xf,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0xa,0xb,0xc,0xd,0xe,0xf,0,0,0,0,0,0,0,0,0 };
        const char* pbegin = psz;
        while (phexdigit[(unsigned char)*psz] || *psz == '0')
            psz++;
        psz--;
        unsigned char* p1 = (unsigned char*)pn;
        unsigned char* pend = p1 + WIDTH * 4;
        while (psz >= pbegin && p1 < pend)
        {
            *p1 = phexdigit[(unsigned char)*psz--];
            if (psz >= pbegin)
            {
                *p1 |= (phexdigit[(unsigned char)*psz--] << 4);
                p1++;
            }
        }
    }

    void SetHex(const std::string& str)
    {
        SetHex(str.c_str());
    }

    std::string ToString() const
    {
        return (GetHex());
    }

    unsigned char* begin()
    {
        return (unsigned char*)&pn[0];
    }

    unsigned char* end()
    {
        return (unsigned char*)&pn[WIDTH];
    }

    const unsigned char* begin() const
    {
        return (unsigned char*)&pn[0];
    }

    const unsigned char* end() const
    {
        return (unsigned char*)&pn[WIDTH];
    }

    unsigned int size() const
    {
        return sizeof(pn);
    }

    uint64_t Get64(int n=0) const
    {
        return pn[2*n] | (uint64_t)pn[2*n+1] << 32;
    }

//    unsigned int GetSerializeSize(int nType=0, int nVersion=PROTOCOL_VERSION) const
    unsigned int GetSerializeSize(int nType, int nVersion) const
    {
        return sizeof(pn);
    }

    template<typename Stream>
//    void Serialize(Stream& s, int nType=0, int nVersion=PROTOCOL_VERSION) const
    void Serialize(Stream& s, int nType, int nVersion) const
    {
        s.write((char*)pn, sizeof(pn));
    }

    template<typename Stream>
//    void Unserialize(Stream& s, int nType=0, int nVersion=PROTOCOL_VERSION)
    void Unserialize(Stream& s, int nType, int nVersion)
    {
        s.read((char*)pn, sizeof(pn));
    }


    friend class uint160_old;
    friend class uint256_old;
    friend inline int Testuint256AdHoc(std::vector<std::string> vArg);
};


typedef base_uint_old<160> base_uint_old160;
typedef base_uint_old<256> base_uint_old256;



//
// uint160_old and uint256_old could be implemented as templates, but to keep
// compile errors and debugging cleaner, they're copy and pasted.
//



//////////////////////////////////////////////////////////////////////////////
//
// uint160_old
//

/** 160-bit unsigned integer */
class uint160_old : public base_uint_old160
{
public:
    typedef base_uint_old160 basetype;

    uint160_old()
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] = 0;
    }

    uint160_old(const basetype& b)
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] = b.pn[i];
    }

    uint160_old& operator=(const basetype& b)
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] = b.pn[i];
        return *this;
    }

    uint160_old(uint64_t b)
    {
        pn[0] = (unsigned int)b;
        pn[1] = (unsigned int)(b >> 32);
        for (int i = 2; i < WIDTH; i++)
            pn[i] = 0;
    }

    uint160_old& operator=(uint64_t b)
    {
        pn[0] = (unsigned int)b;
        pn[1] = (unsigned int)(b >> 32);
        for (int i = 2; i < WIDTH; i++)
            pn[i] = 0;
        return *this;
    }

    explicit uint160_old(const std::string& str)
    {
        SetHex(str);
    }

    explicit uint160_old(const std::vector<unsigned char>& vch)
    {
        if (vch.size() == sizeof(pn))
            memcpy(pn, &vch[0], sizeof(pn));
        else
            *this = 0;
    }
};

inline bool operator==(const uint160_old& a, uint64_t b)                           { return (base_uint_old160)a == b; }
inline bool operator!=(const uint160_old& a, uint64_t b)                           { return (base_uint_old160)a != b; }
inline const uint160_old operator<<(const base_uint_old160& a, unsigned int shift)   { return uint160_old(a) <<= shift; }
inline const uint160_old operator>>(const base_uint_old160& a, unsigned int shift)   { return uint160_old(a) >>= shift; }
inline const uint160_old operator<<(const uint160_old& a, unsigned int shift)        { return uint160_old(a) <<= shift; }
inline const uint160_old operator>>(const uint160_old& a, unsigned int shift)        { return uint160_old(a) >>= shift; }

inline const uint160_old operator^(const base_uint_old160& a, const base_uint_old160& b) { return uint160_old(a) ^= b; }
inline const uint160_old operator&(const base_uint_old160& a, const base_uint_old160& b) { return uint160_old(a) &= b; }
inline const uint160_old operator|(const base_uint_old160& a, const base_uint_old160& b) { return uint160_old(a) |= b; }
inline const uint160_old operator+(const base_uint_old160& a, const base_uint_old160& b) { return uint160_old(a) += b; }
inline const uint160_old operator-(const base_uint_old160& a, const base_uint_old160& b) { return uint160_old(a) -= b; }

inline bool operator<(const base_uint_old160& a, const uint160_old& b)          { return (base_uint_old160)a <  (base_uint_old160)b; }
inline bool operator<=(const base_uint_old160& a, const uint160_old& b)         { return (base_uint_old160)a <= (base_uint_old160)b; }
inline bool operator>(const base_uint_old160& a, const uint160_old& b)          { return (base_uint_old160)a >  (base_uint_old160)b; }
inline bool operator>=(const base_uint_old160& a, const uint160_old& b)         { return (base_uint_old160)a >= (base_uint_old160)b; }
inline bool operator==(const base_uint_old160& a, const uint160_old& b)         { return (base_uint_old160)a == (base_uint_old160)b; }
inline bool operator!=(const base_uint_old160& a, const uint160_old& b)         { return (base_uint_old160)a != (base_uint_old160)b; }
inline const uint160_old operator^(const base_uint_old160& a, const uint160_old& b) { return (base_uint_old160)a ^  (base_uint_old160)b; }
inline const uint160_old operator&(const base_uint_old160& a, const uint160_old& b) { return (base_uint_old160)a &  (base_uint_old160)b; }
inline const uint160_old operator|(const base_uint_old160& a, const uint160_old& b) { return (base_uint_old160)a |  (base_uint_old160)b; }
inline const uint160_old operator+(const base_uint_old160& a, const uint160_old& b) { return (base_uint_old160)a +  (base_uint_old160)b; }
inline const uint160_old operator-(const base_uint_old160& a, const uint160_old& b) { return (base_uint_old160)a -  (base_uint_old160)b; }

inline bool operator<(const uint160_old& a, const base_uint_old160& b)          { return (base_uint_old160)a <  (base_uint_old160)b; }
inline bool operator<=(const uint160_old& a, const base_uint_old160& b)         { return (base_uint_old160)a <= (base_uint_old160)b; }
inline bool operator>(const uint160_old& a, const base_uint_old160& b)          { return (base_uint_old160)a >  (base_uint_old160)b; }
inline bool operator>=(const uint160_old& a, const base_uint_old160& b)         { return (base_uint_old160)a >= (base_uint_old160)b; }
inline bool operator==(const uint160_old& a, const base_uint_old160& b)         { return (base_uint_old160)a == (base_uint_old160)b; }
inline bool operator!=(const uint160_old& a, const base_uint_old160& b)         { return (base_uint_old160)a != (base_uint_old160)b; }
inline const uint160_old operator^(const uint160_old& a, const base_uint_old160& b) { return (base_uint_old160)a ^  (base_uint_old160)b; }
inline const uint160_old operator&(const uint160_old& a, const base_uint_old160& b) { return (base_uint_old160)a &  (base_uint_old160)b; }
inline const uint160_old operator|(const uint160_old& a, const base_uint_old160& b) { return (base_uint_old160)a |  (base_uint_old160)b; }
inline const uint160_old operator+(const uint160_old& a, const base_uint_old160& b) { return (base_uint_old160)a +  (base_uint_old160)b; }
inline const uint160_old operator-(const uint160_old& a, const base_uint_old160& b) { return (base_uint_old160)a -  (base_uint_old160)b; }

inline bool operator<(const uint160_old& a, const uint160_old& b)               { return (base_uint_old160)a <  (base_uint_old160)b; }
inline bool operator<=(const uint160_old& a, const uint160_old& b)              { return (base_uint_old160)a <= (base_uint_old160)b; }
inline bool operator>(const uint160_old& a, const uint160_old& b)               { return (base_uint_old160)a >  (base_uint_old160)b; }
inline bool operator>=(const uint160_old& a, const uint160_old& b)              { return (base_uint_old160)a >= (base_uint_old160)b; }
inline bool operator==(const uint160_old& a, const uint160_old& b)              { return (base_uint_old160)a == (base_uint_old160)b; }
inline bool operator!=(const uint160_old& a, const uint160_old& b)              { return (base_uint_old160)a != (base_uint_old160)b; }
inline const uint160_old operator^(const uint160_old& a, const uint160_old& b)      { return (base_uint_old160)a ^  (base_uint_old160)b; }
inline const uint160_old operator&(const uint160_old& a, const uint160_old& b)      { return (base_uint_old160)a &  (base_uint_old160)b; }
inline const uint160_old operator|(const uint160_old& a, const uint160_old& b)      { return (base_uint_old160)a |  (base_uint_old160)b; }
inline const uint160_old operator+(const uint160_old& a, const uint160_old& b)      { return (base_uint_old160)a +  (base_uint_old160)b; }
inline const uint160_old operator-(const uint160_old& a, const uint160_old& b)      { return (base_uint_old160)a -  (base_uint_old160)b; }






//////////////////////////////////////////////////////////////////////////////
//
// uint256_old
//

/** 256-bit unsigned integer */
class uint256_old : public base_uint_old256
{
public:
    typedef base_uint_old256 basetype;

    uint256_old()
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] = 0;
    }

    uint256_old(const basetype& b)
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] = b.pn[i];
    }

    uint256_old& operator=(const basetype& b)
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] = b.pn[i];
        return *this;
    }

    uint256_old(uint64_t b)
    {
        pn[0] = (unsigned int)b;
        pn[1] = (unsigned int)(b >> 32);
        for (int i = 2; i < WIDTH; i++)
            pn[i] = 0;
    }

    uint256_old& operator=(uint64_t b)
    {
        pn[0] = (unsigned int)b;
        pn[1] = (unsigned int)(b >> 32);
        for (int i = 2; i < WIDTH; i++)
            pn[i] = 0;
        return *this;
    }

    explicit uint256_old(const std::string& str)
    {
        SetHex(str);
    }

    explicit uint256_old(const std::vector<unsigned char>& vch)
    {
        if (vch.size() == sizeof(pn))
            memcpy(pn, &vch[0], sizeof(pn));
        else
            *this = 0;
    }
};

inline bool operator==(const uint256_old& a, uint64_t b)                           { return (base_uint_old256)a == b; }
inline bool operator!=(const uint256_old& a, uint64_t b)                           { return (base_uint_old256)a != b; }
inline const uint256_old operator<<(const base_uint_old256& a, unsigned int shift)   { return uint256_old(a) <<= shift; }
inline const uint256_old operator>>(const base_uint_old256& a, unsigned int shift)   { return uint256_old(a) >>= shift; }
inline const uint256_old operator<<(const uint256_old& a, unsigned int shift)        { return uint256_old(a) <<= shift; }
inline const uint256_old operator>>(const uint256_old& a, unsigned int shift)        { return uint256_old(a) >>= shift; }

inline const uint256_old operator^(const base_uint_old256& a, const base_uint_old256& b) { return uint256_old(a) ^= b; }
inline const uint256_old operator&(const base_uint_old256& a, const base_uint_old256& b) { return uint256_old(a) &= b; }
inline const uint256_old operator|(const base_uint_old256& a, const base_uint_old256& b) { return uint256_old(a) |= b; }
inline const uint256_old operator+(const base_uint_old256& a, const base_uint_old256& b) { return uint256_old(a) += b; }
inline const uint256_old operator-(const base_uint_old256& a, const base_uint_old256& b) { return uint256_old(a) -= b; }

inline bool operator<(const base_uint_old256& a, const uint256_old& b)          { return (base_uint_old256)a <  (base_uint_old256)b; }
inline bool operator<=(const base_uint_old256& a, const uint256_old& b)         { return (base_uint_old256)a <= (base_uint_old256)b; }
inline bool operator>(const base_uint_old256& a, const uint256_old& b)          { return (base_uint_old256)a >  (base_uint_old256)b; }
inline bool operator>=(const base_uint_old256& a, const uint256_old& b)         { return (base_uint_old256)a >= (base_uint_old256)b; }
inline bool operator==(const base_uint_old256& a, const uint256_old& b)         { return (base_uint_old256)a == (base_uint_old256)b; }
inline bool operator!=(const base_uint_old256& a, const uint256_old& b)         { return (base_uint_old256)a != (base_uint_old256)b; }
inline const uint256_old operator^(const base_uint_old256& a, const uint256_old& b) { return (base_uint_old256)a ^  (base_uint_old256)b; }
inline const uint256_old operator&(const base_uint_old256& a, const uint256_old& b) { return (base_uint_old256)a &  (base_uint_old256)b; }
inline const uint256_old operator|(const base_uint_old256& a, const uint256_old& b) { return (base_uint_old256)a |  (base_uint_old256)b; }
inline const uint256_old operator+(const base_uint_old256& a, const uint256_old& b) { return (base_uint_old256)a +  (base_uint_old256)b; }
inline const uint256_old operator-(const base_uint_old256& a, const uint256_old& b) { return (base_uint_old256)a -  (base_uint_old256)b; }

inline bool operator<(const uint256_old& a, const base_uint_old256& b)          { return (base_uint_old256)a <  (base_uint_old256)b; }
inline bool operator<=(const uint256_old& a, const base_uint_old256& b)         { return (base_uint_old256)a <= (base_uint_old256)b; }
inline bool operator>(const uint256_old& a, const base_uint_old256& b)          { return (base_uint_old256)a >  (base_uint_old256)b; }
inline bool operator>=(const uint256_old& a, const base_uint_old256& b)         { return (base_uint_old256)a >= (base_uint_old256)b; }
inline bool operator==(const uint256_old& a, const base_uint_old256& b)         { return (base_uint_old256)a == (base_uint_old256)b; }
inline bool operator!=(const uint256_old& a, const base_uint_old256& b)         { return (base_uint_old256)a != (base_uint_old256)b; }
inline const uint256_old operator^(const uint256_old& a, const base_uint_old256& b) { return (base_uint_old256)a ^  (base_uint_old256)b; }
inline const uint256_old operator&(const uint256_old& a, const base_uint_old256& b) { return (base_uint_old256)a &  (base_uint_old256)b; }
inline const uint256_old operator|(const uint256_old& a, const base_uint_old256& b) { return (base_uint_old256)a |  (base_uint_old256)b; }
inline const uint256_old operator+(const uint256_old& a, const base_uint_old256& b) { return (base_uint_old256)a +  (base_uint_old256)b; }
inline const uint256_old operator-(const uint256_old& a, const base_uint_old256& b) { return (base_uint_old256)a -  (base_uint_old256)b; }

inline bool operator<(const uint256_old& a, const uint256_old& b)               { return (base_uint_old256)a <  (base_uint_old256)b; }
inline bool operator<=(const uint256_old& a, const uint256_old& b)              { return (base_uint_old256)a <= (base_uint_old256)b; }
inline bool operator>(const uint256_old& a, const uint256_old& b)               { return (base_uint_old256)a >  (base_uint_old256)b; }
inline bool operator>=(const uint256_old& a, const uint256_old& b)              { return (base_uint_old256)a >= (base_uint_old256)b; }
inline bool operator==(const uint256_old& a, const uint256_old& b)              { return (base_uint_old256)a == (base_uint_old256)b; }
inline bool operator!=(const uint256_old& a, const uint256_old& b)              { return (base_uint_old256)a != (base_uint_old256)b; }
inline const uint256_old operator^(const uint256_old& a, const uint256_old& b)      { return (base_uint_old256)a ^  (base_uint_old256)b; }
inline const uint256_old operator&(const uint256_old& a, const uint256_old& b)      { return (base_uint_old256)a &  (base_uint_old256)b; }
inline const uint256_old operator|(const uint256_old& a, const uint256_old& b)      { return (base_uint_old256)a |  (base_uint_old256)b; }
inline const uint256_old operator+(const uint256_old& a, const uint256_old& b)      { return (base_uint_old256)a +  (base_uint_old256)b; }
inline const uint256_old operator-(const uint256_old& a, const uint256_old& b) { return (base_uint_old256)a - (base_uint_old256)b; }

#endif // BITCOIN_ARITH_UINT256_H
