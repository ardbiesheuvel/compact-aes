/*
 * Compact AES using standard C
 *
 * Copyright (C) 2017 Ard Biesheuvel <ard.biesheuvel@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdint.h>

#define CAES_RK_WORDS	60

void caes_encrypt(uint8_t *out, const uint8_t *in, uint32_t keylen,
		  const uint32_t rk[]);

void caes_decrypt(uint8_t *out, const uint8_t *in, uint32_t keylen,
		  const uint32_t rk[]);

int caes_expand_key(uint32_t rk[], const uint8_t *key, uint32_t keylen);
