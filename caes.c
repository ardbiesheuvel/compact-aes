/*
 * Compact AES using standard C
 *
 * Copyright (C) 2017 Ard Biesheuvel <ard.biesheuvel@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "caes.h"

static const uint8_t sbox[] = {
	0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5,
	0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
	0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,
	0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
	0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc,
	0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
	0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a,
	0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
	0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,
	0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
	0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b,
	0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
	0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85,
	0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
	0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,
	0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
	0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17,
	0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
	0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88,
	0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
	0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,
	0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
	0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9,
	0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
	0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6,
	0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
	0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,
	0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
	0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94,
	0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
	0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68,
	0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16,
};

static const uint8_t inv_sbox[] = {
	0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38,
	0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
	0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87,
	0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
	0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d,
	0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
	0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2,
	0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
	0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16,
	0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
	0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda,
	0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
	0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a,
	0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
	0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02,
	0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
	0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea,
	0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
	0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85,
	0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
	0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89,
	0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
	0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20,
	0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
	0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31,
	0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
	0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d,
	0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
	0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0,
	0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
	0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26,
	0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d,
};

static uint32_t ror32(uint32_t w, int l)
{
	/* rotate right by l bits */
	return (w >> l) | (w << (32 - l));
}

static uint32_t mul_by_x(uint32_t w)
{
	/* multiply by polynomial 'x' (0b10) in GF(2^8) */
	return ((w & 0x80808080) >> 7) * 0x1b ^
	       ((w & 0x7f7f7f7f) << 1);
}

static uint32_t mul_by_x2(uint32_t w)
{
	uint32_t y = w & 0xc0c0c0c0;

	/* multiply by polynomial 'x^2' (0b100) in GF(2^8) */
	return (y >> 2) ^ (y >> 3) ^ (y >> 5) ^ (y >> 6) ^
	       (w & 0x3f3f3f3f) << 2;
}

static uint32_t mix_columns(uint32_t x)
{
	/*
	 * Perform the following matrix multiplication in GF(2^8)
	 *
	 * | 0x2 0x3 0x1 0x1 |   | x[0] |
	 * | 0x1 0x2 0x3 0x1 |   | x[1] |
	 * | 0x1 0x1 0x2 0x3 | x | x[2] |
	 * | 0x3 0x1 0x1 0x3 |   | x[3] |
	 */
	uint32_t y = mul_by_x(x) ^ ror32(x, 16);

	return y ^ ror32(x ^ y, 8);
}

static uint32_t inv_mix_columns(uint32_t x)
{
	/*
	 * Perform the following matrix multiplication in GF(2^8)
	 *
	 * | 0xe 0xb 0xd 0x9 |   | x[0] |
	 * | 0x9 0xe 0xb 0xd |   | x[1] |
	 * | 0xd 0x9 0xe 0xb | x | x[2] |
	 * | 0xb 0xd 0x9 0xe |   | x[3] |
	 *
	 * which can conveniently be reduced to
	 *
	 * | 0x2 0x3 0x1 0x1 |   | 0x5 0x0 0x4 0x0 |   | x[0] |
	 * | 0x1 0x2 0x3 0x1 |   | 0x0 0x5 0x0 0x4 |   | x[1] |
	 * | 0x1 0x1 0x2 0x3 | x | 0x4 0x0 0x5 0x0 | x | x[2] |
	 * | 0x3 0x1 0x1 0x2 |   | 0x0 0x4 0x0 0x5 |   | x[3] |
	 */
	uint32_t y = mul_by_x2(x);

	return mix_columns(x ^ y ^ ror32(y, 16));
}

static uint32_t subshift(const uint32_t *in, int pos)
{
	return (sbox[0xff & in[pos]]) ^
	       (sbox[0xff & (in[(pos + 1) % 4] >>  8)] <<  8) ^
	       (sbox[0xff & (in[(pos + 2) % 4] >> 16)] << 16) ^
	       (sbox[0xff & (in[(pos + 3) % 4] >> 24)] << 24);
}

static uint32_t inv_subshift(const uint32_t *in, int pos)
{
	return (inv_sbox[0xff & in[pos]]) ^
	       (inv_sbox[0xff & (in[(pos + 3) % 4] >>  8)] <<  8) ^
	       (inv_sbox[0xff & (in[(pos + 2) % 4] >> 16)] << 16) ^
	       (inv_sbox[0xff & (in[(pos + 1) % 4] >> 24)] << 24);
}

static uint32_t get_le32_word(const uint8_t *p)
{
	return ((uint32_t)p[0]) |
	       ((uint32_t)p[1] <<  8) |
	       ((uint32_t)p[2] << 16) |
	       ((uint32_t)p[3] << 24);
}

static void put_le32_word(uint32_t val, uint8_t *p)
{
	p[0] = (uint8_t)val;
	p[1] = (uint8_t)(val >>  8);
	p[2] = (uint8_t)(val >> 16);
	p[3] = (uint8_t)(val >> 24);
}

void caes_encrypt(uint8_t *out, const uint8_t *in, uint32_t keylen,
		  const uint32_t rk[])
{
	const uint32_t rounds = 6 + keylen / 4;
	const uint32_t *rkp = rk + 4;
	uint32_t st0[4], st1[4];
	int round;

	st0[0] = rk[0] ^ get_le32_word(in);
	st0[1] = rk[1] ^ get_le32_word(in + 4);
	st0[2] = rk[2] ^ get_le32_word(in + 8);
	st0[3] = rk[3] ^ get_le32_word(in + 12);

	for (round = 0;; round += 2) {
		st1[0] = mix_columns(subshift(st0, 0)) ^ rkp[0];
		st1[1] = mix_columns(subshift(st0, 1)) ^ rkp[1];
		st1[2] = mix_columns(subshift(st0, 2)) ^ rkp[2];
		st1[3] = mix_columns(subshift(st0, 3)) ^ rkp[3];

		if (round == rounds - 2)
			break;

		st0[0] = mix_columns(subshift(st1, 0)) ^ rkp[4];
		st0[1] = mix_columns(subshift(st1, 1)) ^ rkp[5];
		st0[2] = mix_columns(subshift(st1, 2)) ^ rkp[6];
		st0[3] = mix_columns(subshift(st1, 3)) ^ rkp[7];

		rkp += 8;
	}

	put_le32_word(subshift(st1, 0) ^ rkp[4], out);
	put_le32_word(subshift(st1, 1) ^ rkp[5], out + 4);
	put_le32_word(subshift(st1, 2) ^ rkp[6], out + 8);
	put_le32_word(subshift(st1, 3) ^ rkp[7], out + 12);
}

void caes_decrypt(uint8_t *out, const uint8_t *in, uint32_t keylen,
		  const uint32_t rk[])
{
	const uint32_t rounds = 6 + keylen / 4;
	const uint32_t *rkp = rk + 4 * rounds;
	uint32_t st0[4], st1[4];
	int round;

	st0[0] = rkp[0] ^ get_le32_word(in);
	st0[1] = rkp[1] ^ get_le32_word(in + 4);
	st0[2] = rkp[2] ^ get_le32_word(in + 8);
	st0[3] = rkp[3] ^ get_le32_word(in + 12);

	for (round = 0;; round += 2) {
		rkp -= 8;

		st1[0] = inv_mix_columns(inv_subshift(st0, 0) ^ rkp[4]);
		st1[1] = inv_mix_columns(inv_subshift(st0, 1) ^ rkp[5]);
		st1[2] = inv_mix_columns(inv_subshift(st0, 2) ^ rkp[6]);
		st1[3] = inv_mix_columns(inv_subshift(st0, 3) ^ rkp[7]);

		if (round == rounds - 2)
			break;

		st0[0] = inv_mix_columns(inv_subshift(st1, 0) ^ rkp[0]);
		st0[1] = inv_mix_columns(inv_subshift(st1, 1) ^ rkp[1]);
		st0[2] = inv_mix_columns(inv_subshift(st1, 2) ^ rkp[2]);
		st0[3] = inv_mix_columns(inv_subshift(st1, 3) ^ rkp[3]);
	}

	put_le32_word(inv_subshift(st1, 0) ^ rk[0], out);
	put_le32_word(inv_subshift(st1, 1) ^ rk[1], out + 4);
	put_le32_word(inv_subshift(st1, 2) ^ rk[2], out + 8);
	put_le32_word(inv_subshift(st1, 3) ^ rk[3], out + 12);
}

static uint32_t subw(const uint32_t in)
{
	/*
	 * Perform a forward Sbox substitution on each byte of the input.
	 */
	return (sbox[0xff & in]) ^
	       (sbox[0xff & (in >>  8)] <<  8) ^
	       (sbox[0xff & (in >> 16)] << 16) ^
	       (sbox[0xff & (in >> 24)] << 24);
}

int caes_expand_key(uint32_t rk[], const uint8_t *key, uint32_t keylen)
{
	uint32_t kwords = keylen / sizeof(uint32_t);
	uint32_t rc, i;

	if (keylen != 16 && keylen != 24 && keylen != 32)
		return -1;

	for (i = 0; i < kwords; i++)
		rk[i] = get_le32_word(key + i * sizeof(uint32_t));

	for (i = 0, rc = 1; i < 10; i++, rc = mul_by_x(rc)) {
		uint32_t *rki = rk + (i * kwords);
		uint32_t *rko = rki + kwords;

		rko[0] = ror32(subw(rki[kwords - 1]), 8) ^ rc ^ rki[0];
		rko[1] = rko[0] ^ rki[1];
		rko[2] = rko[1] ^ rki[2];
		rko[3] = rko[2] ^ rki[3];

		if (keylen == 24) {
			if (i >= 7)
				break;
			rko[4] = rko[3] ^ rki[4];
			rko[5] = rko[4] ^ rki[5];
		} else if (keylen == 32) {
			if (i >= 6)
				break;
			rko[4] = subw(rko[3]) ^ rki[4];
			rko[5] = rko[4] ^ rki[5];
			rko[6] = rko[5] ^ rki[6];
			rko[7] = rko[6] ^ rki[7];
		}
	}
	return 0;
}
