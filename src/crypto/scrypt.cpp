/*
 * Copyright 2009 Colin Percival
 * Copyright 2011 ArtForz
 * Copyright 2012-2013 pooler
 * Copyright 2014-2017 The Vertcoin Developers
 * Copyright (c) 2014-2018 The Verticalcoin Developers
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file was originally written by Colin Percival as part of the Tarsnap
 * online backup system.
 */

#include "scrypt.h"
#include "util.h"
#include "hmac_sha256.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


/*static inline uint32_t scrypt_be32dec(const void *pp)
{
	const uint8_t *p = (uint8_t const *)pp;
	return ((uint32_t)(p[3]) + ((uint32_t)(p[2]) << 8) +
	    ((uint32_t)(p[1]) << 16) + ((uint32_t)(p[0]) << 24));
}*/

static inline void scrypt_be32enc(void *pp, uint32_t x)
{
	uint8_t *p = (uint8_t *)pp;
	p[3] = x & 0xff;
	p[2] = (x >> 8) & 0xff;
	p[1] = (x >> 16) & 0xff;
	p[0] = (x >> 24) & 0xff;
}

/**
 * PBKDF2_SHA256(passwd, passwdlen, salt, saltlen, c, buf, dkLen):
 * Compute PBKDF2(passwd, salt, c, dkLen) using HMAC-SHA256 as the PRF, and
 * write the output to buf.  The value dkLen must be at most 32 * (2^32 - 1).
 */
void
PBKDF2_SHA256(const uint8_t *passwd, size_t passwdlen, const uint8_t *salt,
    size_t saltlen, uint64_t c, uint8_t *buf, size_t dkLen)
{
	size_t i;
	uint8_t ivec[4];
	uint8_t U[32];
	uint8_t T[32];
	uint64_t j;
	int k;
	size_t clen;

	/* Compute HMAC state after processing P and S. */

	CHMAC_SHA256 PShctx(passwd, passwdlen);
	CHMAC_SHA256 hctx(passwd, passwdlen);
	PShctx.Write(salt, saltlen);

	/* Iterate through the blocks. */
	for (i = 0; i * 32 < dkLen; i++) {
		/* Generate INT(i + 1). */
		scrypt_be32enc(ivec, (uint32_t)(i + 1));

		/* Compute U_1 = PRF(P, S || INT(i)). */
		memcpy(&hctx, &PShctx, sizeof(CHMAC_SHA256));
        hctx.Write(ivec, 4);
        hctx.Finalize(U);

		/* T_i = U_1 ... */
		memcpy(T, U, 32);

		for (j = 2; j <= c; j++) {
			/* Compute U_j. */
			CHMAC_SHA256 hctxi(passwd, passwdlen);
			hctxi.Write(U, 32);
			hctxi.Finalize(U);

			/* ... xor U_j ... */
			for (k = 0; k < 32; k++)
				T[k] ^= U[k];
		}

		/* Copy as many bytes as necessary into buf. */
		clen = dkLen - i * 32;
		if (clen > 32)
			clen = 32;
		memcpy(&buf[i * 32], T, clen);
	}

	/* Clean PShctx, since we never called _Final on it. */
	memset(&PShctx, 0, sizeof(CHMAC_SHA256));
}

#define ROTL(a, b) (((a) << (b)) | ((a) >> (32 - (b))))

static inline void xor_salsa8(uint32_t B[16], const uint32_t Bx[16])
{
	uint32_t x00,x01,x02,x03,x04,x05,x06,x07,x08,x09,x10,x11,x12,x13,x14,x15;
	int i;

	x00 = (B[ 0] ^= Bx[ 0]);
	x01 = (B[ 1] ^= Bx[ 1]);
	x02 = (B[ 2] ^= Bx[ 2]);
	x03 = (B[ 3] ^= Bx[ 3]);
	x04 = (B[ 4] ^= Bx[ 4]);
	x05 = (B[ 5] ^= Bx[ 5]);
	x06 = (B[ 6] ^= Bx[ 6]);
	x07 = (B[ 7] ^= Bx[ 7]);
	x08 = (B[ 8] ^= Bx[ 8]);
	x09 = (B[ 9] ^= Bx[ 9]);
	x10 = (B[10] ^= Bx[10]);
	x11 = (B[11] ^= Bx[11]);
	x12 = (B[12] ^= Bx[12]);
	x13 = (B[13] ^= Bx[13]);
	x14 = (B[14] ^= Bx[14]);
	x15 = (B[15] ^= Bx[15]);
	for (i = 0; i < 8; i += 2) {
		/* Operate on columns. */
		x04 ^= ROTL(x00 + x12,  7);  x09 ^= ROTL(x05 + x01,  7);
		x14 ^= ROTL(x10 + x06,  7);  x03 ^= ROTL(x15 + x11,  7);

		x08 ^= ROTL(x04 + x00,  9);  x13 ^= ROTL(x09 + x05,  9);
		x02 ^= ROTL(x14 + x10,  9);  x07 ^= ROTL(x03 + x15,  9);

		x12 ^= ROTL(x08 + x04, 13);  x01 ^= ROTL(x13 + x09, 13);
		x06 ^= ROTL(x02 + x14, 13);  x11 ^= ROTL(x07 + x03, 13);

		x00 ^= ROTL(x12 + x08, 18);  x05 ^= ROTL(x01 + x13, 18);
		x10 ^= ROTL(x06 + x02, 18);  x15 ^= ROTL(x11 + x07, 18);

		/* Operate on rows. */
		x01 ^= ROTL(x00 + x03,  7);  x06 ^= ROTL(x05 + x04,  7);
		x11 ^= ROTL(x10 + x09,  7);  x12 ^= ROTL(x15 + x14,  7);

		x02 ^= ROTL(x01 + x00,  9);  x07 ^= ROTL(x06 + x05,  9);
		x08 ^= ROTL(x11 + x10,  9);  x13 ^= ROTL(x12 + x15,  9);

		x03 ^= ROTL(x02 + x01, 13);  x04 ^= ROTL(x07 + x06, 13);
		x09 ^= ROTL(x08 + x11, 13);  x14 ^= ROTL(x13 + x12, 13);

		x00 ^= ROTL(x03 + x02, 18);  x05 ^= ROTL(x04 + x07, 18);
		x10 ^= ROTL(x09 + x08, 18);  x15 ^= ROTL(x14 + x13, 18);
	}
	B[ 0] += x00;
	B[ 1] += x01;
	B[ 2] += x02;
	B[ 3] += x03;
	B[ 4] += x04;
	B[ 5] += x05;
	B[ 6] += x06;
	B[ 7] += x07;
	B[ 8] += x08;
	B[ 9] += x09;
	B[10] += x10;
	B[11] += x11;
	B[12] += x12;
	B[13] += x13;
	B[14] += x14;
	B[15] += x15;
}

void scrypt_N_1_1_256_sp_generic(const char *input, char *output, char *scratchpad, unsigned char Nfactor)
{
	uint8_t B[128];
	uint32_t X[32];
	uint32_t *V;
	uint32_t i, j, k, N;

	V = (uint32_t *)(((uintptr_t)(scratchpad) + 63) & ~ (uintptr_t)(63));

	PBKDF2_SHA256((const uint8_t *)input, 80, (const uint8_t *)input, 80, 1, B, 128);

	for (k = 0; k < 32; k++)
		X[k] = scrypt_le32dec(&B[4 * k]);
        
        N = (1 << (Nfactor + 1));
        
	for (i = 0; i < N; i++) {
		memcpy(&V[i * 32], X, 128);
		xor_salsa8(&X[0], &X[16]);
		xor_salsa8(&X[16], &X[0]);
	}
	for (i = 0; i < N; i++) {
		//j = 32 * (X[16] & 1023);
                j = 32 * (X[16] & (N-1));
		for (k = 0; k < 32; k++)
			X[k] ^= V[j + k];
		xor_salsa8(&X[0], &X[16]);
		xor_salsa8(&X[16], &X[0]);
	}

	for (k = 0; k < 32; k++)
		scrypt_le32enc(&B[4 * k], X[k]);

	PBKDF2_SHA256((const uint8_t *)input, 80, B, 128, 1, (uint8_t *)output, 32);
}

#if defined(USE_SSE2)
#if defined(_M_X64) || defined(__x86_64__) || defined(_M_AMD64) || (defined(MAC_OSX) && defined(__i386__))
/* Always SSE2 */
void scrypt_detect_sse2(unsigned int cpuid_edx)
{
    printf("scrypt: using scrypt-sse2 as built.\n");
}
#else
/* Detect SSE2 */
void (*scrypt_N_1_1_256_sp)(const char *input, char *output, char *scratchpad, unsigned char Nfactor);

void scrypt_detect_sse2(unsigned int cpuid_edx)
{
    if (cpuid_edx & 1<<26)
    {
        scrypt_N_1_1_256_sp = &scrypt_N_1_1_256_sp_sse2;
        printf("scrypt: using scrypt-sse2 as detected.\n");
    }
    else
    {
        scrypt_N_1_1_256_sp = &scrypt_N_1_1_256_sp_generic;
        printf("scrypt: using scrypt-generic, SSE2 unavailable.\n");
    }
}
#endif
#endif

void scrypt_N_1_1_256(const char *input, char *output, unsigned char Nfactor)
{
	char scratchpad[((1 << (Nfactor + 1)) * 128 ) + 63];
#if defined(USE_SSE2)
        // Detection would work, but in cases where we KNOW it always has SSE2,
        // it is faster to use directly than to use a function pointer or conditional.
#if defined(_M_X64) || defined(__x86_64__) || defined(_M_AMD64) || (defined(MAC_OSX) && defined(__i386__))
        // Always SSE2: x86_64 or Intel MacOS X
        scrypt_N_1_1_256_sp_sse2(input, output, scratchpad, Nfactor);
#else
        // Detect SSE2: 32bit x86 Linux or Windows
        scrypt_N_1_1_256_sp(input, output, scratchpad, Nfactor);
#endif
#else
        // Generic scrypt
        scrypt_N_1_1_256_sp_generic(input, output, scratchpad, Nfactor);
#endif
}
