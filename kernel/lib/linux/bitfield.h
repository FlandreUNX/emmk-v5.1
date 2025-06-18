/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2014 Felix Fietkau <nbd@nbd.name>
 * Copyright (C) 2004 - 2009 Ivo van Doorn <IvDoorn@gmail.com>
 */

#ifndef _LINUX_BITFIELD_H
#define _LINUX_BITFIELD_H

/*
 * Bitfield access macros
 *
 * FIELD_{GET,PREP} macros take as first parameter shifted mask
 * from which they extract the base mask and shift amount.
 * Mask must be a compilation time constant.
 *
 * Example:
 *
 *  #define REG_FIELD_A  GENMASK(6, 0)
 *  #define REG_FIELD_B  BIT(7)
 *  #define REG_FIELD_C  GENMASK(15, 8)
 *  #define REG_FIELD_D  GENMASK(31, 16)
 *
 * Get:
 *  a = FIELD_GET(REG_FIELD_A, reg);
 *  b = FIELD_GET(REG_FIELD_B, reg);
 *
 * Set:
 *  reg = FIELD_PREP(REG_FIELD_A, 1) |
 *	  FIELD_PREP(REG_FIELD_B, 0) |
 *	  FIELD_PREP(REG_FIELD_C, c) |
 *	  FIELD_PREP(REG_FIELD_D, 0x40);
 *
 * Modify:
 *  reg &= ~REG_FIELD_C;
 *  reg |= FIELD_PREP(REG_FIELD_C, c);
 */

#define __bf_shf(x) (__builtin_ffsll(x) - 1)

#define __scalar_type_to_unsigned_cases(type)				\
		unsigned type:	(unsigned type)0,			\
		signed type:	(unsigned type)0

#define __unsigned_scalar_typeof(x) typeof(				\
		_Generic((x),						\
			char:	(unsigned char)0,			\
			__scalar_type_to_unsigned_cases(char),		\
			__scalar_type_to_unsigned_cases(short),		\
			__scalar_type_to_unsigned_cases(int),		\
			__scalar_type_to_unsigned_cases(long),		\
			__scalar_type_to_unsigned_cases(long long),	\
			default: (x)))

#define __bf_cast_unsigned(type, x)	((__unsigned_scalar_typeof(type))(x))

#define __BF_FIELD_CHECK(_mask, _reg, _val, _pfx)
            
#define KLREG32(addr)                  (*(volatile uint32_t *)(uint32_t)(addr))
#define KLREG16(addr)                  (*(volatile uint16_t *)(uint32_t)(addr))
#define KLREG8(addr)                   (*(volatile uint8_t *)(uint32_t)(addr))
#define KLBIT(x)                       ((uint32_t)((uint32_t)0x01U<<(x)))
#define KLBITS(start, end)             ((0xFFFFFFFFUL << (start)) & (0xFFFFFFFFUL >> (31U - (uint32_t)(end)))) 
#define KLGET_BITS(regval, start, end) (((regval) & KLBITS((start),(end))) >> (start))

/**
 * FIELD_MAX() - produce the maximum value representable by a field
 * @_mask: shifted mask defining the field's length and position
 *
 * FIELD_MAX() returns the maximum value that can be held in the field
 * specified by @_mask.
 */
#define FIELD_MAX(_mask)						\
	({								\
		__BF_FIELD_CHECK(_mask, 0ULL, 0ULL, "FIELD_MAX: ");	\
		(typeof(_mask))((_mask) >> __bf_shf(_mask));		\
	})

/**
 * FIELD_FIT() - check if value fits in the field
 * @_mask: shifted mask defining the field's length and position
 * @_val:  value to test against the field
 *
 * Return: true if @_val can fit inside @_mask, false if @_val is too big.
 */
#define FIELD_FIT(_mask, _val)						\
	({								\
		__BF_FIELD_CHECK(_mask, 0ULL, 0ULL, "FIELD_FIT: ");	\
		!((((typeof(_mask))_val) << __bf_shf(_mask)) & ~(_mask)); \
	})

/**
 * FIELD_PREP() - prepare a bitfield element
 * @_mask: shifted mask defining the field's length and position
 * @_val:  value to put in the field
 *
 * FIELD_PREP() masks and shifts up the value.  The result should
 * be combined with other fields of the bitfield using logical OR.
 */
#define FIELD_PREP(_mask, _val)						\
	({								\
		__BF_FIELD_CHECK(_mask, 0ULL, _val, "FIELD_PREP: ");	\
		((typeof(_mask))(_val) << __bf_shf(_mask)) & (_mask);	\
	})

/**
 * FIELD_GET() - extract a bitfield element
 * @_mask: shifted mask defining the field's length and position
 * @_reg:  value of entire bitfield
 *
 * FIELD_GET() extracts the field specified by @_mask from the
 * bitfield passed in as @_reg by masking and shifting it down.
 */
#define FIELD_GET(_mask, _reg)						\
	({								\
		__BF_FIELD_CHECK(_mask, _reg, 0U, "FIELD_GET: ");	\
		(typeof(_mask))(((_reg) & (_mask)) >> __bf_shf(_mask));	\
	})

#endif