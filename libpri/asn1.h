/*
 * libpri: An implementation of Primary Rate ISDN
 *
 * Copyright (C) 2009 Digium, Inc.
 *
 * Richard Mudgett <rmudgett@digium.com>
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2 as published by the
 * Free Software Foundation. See the LICENSE file included with
 * this program for more details.
 *
 * In addition, when this program is distributed with Asterisk in
 * any form that would qualify as a 'combined work' or as a
 * 'derivative work' (but not mere aggregation), you can redistribute
 * and/or modify the combination under the terms of the license
 * provided with that copy of Asterisk, instead of the license
 * terms granted here.
 */

/*!
 * \file
 * \brief ASN.1 definitions and prototypes
 *
 * \details
 * This file contains all ASN.1 primitive data structures and
 * definitions needed for ROSE component encoding and decoding.
 *
 * ROSE - Remote Operations Service Element
 * ASN.1 - Abstract Syntax Notation 1
 * APDU - Application Protocol Data Unit
 *
 * \author Richard Mudgett <rmudgett@digium.com>
 */

#ifndef _LIBPRI_ASN1_H
#define _LIBPRI_ASN1_H

#include <string.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------- */

/*! ASN.1 Identifier Octet - Tag class bits */
#define ASN1_CLASS_MASK				0xc0
#define ASN1_CLASS_UNIVERSAL		0x00	/*!< Universal primitive data types */
#define ASN1_CLASS_APPLICATION		0x40	/*!< Application wide data tag */
#define ASN1_CLASS_CONTEXT_SPECIFIC	0x80	/*!< Context specifc data tag */
#define ASN1_CLASS_PRIVATE			0xc0	/*!< Private organization data tag */

/*! ASN.1 Identifier Octet - Primitive/Constructor bit */
#define ASN1_PC_MASK				0x20
#define ASN1_PC_PRIMITIVE			0x00
#define ASN1_PC_CONSTRUCTED			0x20

/*! ASN.1 Identifier Octet - Universal data types */
#define ASN1_TYPE_MASK				0x1f
#define ASN1_TYPE_INDEF_TERM		0x00	/*  0 */
#define ASN1_TYPE_BOOLEAN			0x01	/*  1 */
#define ASN1_TYPE_INTEGER			0x02	/*  2 */
#define ASN1_TYPE_BIT_STRING		0x03	/*  3 */
#define ASN1_TYPE_OCTET_STRING		0x04	/*  4 */
#define ASN1_TYPE_NULL				0x05	/*  5 */
#define ASN1_TYPE_OBJECT_IDENTIFIER	0x06	/*  6 */
#define ASN1_TYPE_OBJECT_DESCRIPTOR	0x07	/*  7 */
#define ASN1_TYPE_EXTERN			0x08	/*  8 */
#define ASN1_TYPE_REAL				0x09	/*  9 */
#define ASN1_TYPE_ENUMERATED		0x0a	/* 10 */
#define ASN1_TYPE_EMBEDDED_PDV		0x0b	/* 11 */
#define ASN1_TYPE_UTF8_STRING		0x0c	/* 12 */
#define ASN1_TYPE_RELATIVE_OID		0x0d	/* 13 */
/* 0x0e & 0x0f are reserved for future ASN.1 editions */
#define ASN1_TYPE_SEQUENCE			0x10	/* 16 */
#define ASN1_TYPE_SET				0x11	/* 17 */
#define ASN1_TYPE_NUMERIC_STRING	0x12	/* 18 */
#define ASN1_TYPE_PRINTABLE_STRING	0x13	/* 19 */
#define ASN1_TYPE_TELETEX_STRING	0x14	/* 20 */
#define ASN1_TYPE_VIDEOTEX_STRING	0x15	/* 21 */
#define ASN1_TYPE_IA5_STRING		0x16	/* 22 */
#define ASN1_TYPE_UTC_TIME			0x17	/* 23 */
#define ASN1_TYPE_GENERALIZED_TIME	0x18	/* 24 */
#define ASN1_TYPE_GRAPHIC_STRING	0x19	/* 25 */
#define ASN1_TYPE_VISIBLE_STRING	0x1a	/* 26 */
#define ASN1_TYPE_ISO646_STRING		0x1a	/* 26 */
#define ASN1_TYPE_GENERAL_STRING	0x1b	/* 27 */
#define ASN1_TYPE_UNIVERSAL_STRING	0x1c	/* 28 */
#define ASN1_TYPE_CHAR_STRING		0x1d	/* 29 */
#define ASN1_TYPE_BMP_STRING		0x1e	/* 30 */
#define ASN1_TYPE_EXTENSION			0x1f	/* 31 */

#define ASN1_TAG_SEQUENCE	(ASN1_CLASS_UNIVERSAL | ASN1_PC_CONSTRUCTED | ASN1_TYPE_SEQUENCE)
#define ASN1_TAG_SET		(ASN1_CLASS_UNIVERSAL | ASN1_PC_CONSTRUCTED | ASN1_TYPE_SET)

#define ASN1_INDEF_TERM	(ASN1_CLASS_UNIVERSAL | ASN1_PC_PRIMITIVE | ASN1_TYPE_INDEF_TERM)
#define ASN1_INDEF_TERM_LEN		2

struct asn1_oid {
	/*! \brief Number of subidentifier values in OID list */
	u_int16_t num_values;

	/*!
	 * \brief OID subidentifier value list
	 * \note The first value is really the first two OID subidentifiers.
	 * They are compressed using this formula:
	 * First_Value = (First_Subidentifier * 40) + Second_Subidentifier
	 */
	u_int16_t value[10];
};

#define ASN1_CALL(new_pos, do_it)   \
	do                              \
	{                               \
		(new_pos) = (do_it);        \
		if (!(new_pos)) {           \
			return NULL;            \
		}                           \
	} while (0)

/*! \brief Determine the ending position of the set or sequence to verify the length. */
#define ASN1_END_SETUP(component_end, offset, length, pos, end) \
	do {                                                        \
		if ((length) < 0) {                                     \
			(offset) = ASN1_INDEF_TERM_LEN;                     \
			(component_end) = (end);                            \
		} else {                                                \
			(offset) = 0;                                       \
			(component_end) = (pos) + (length);                 \
		}                                                       \
	} while (0)

/*! \brief Account for the indefinite length terminator of the set or sequence. */
#define ASN1_END_FIXUP(ctrl, pos, offset, component_end, end)                   \
	do {                                                                        \
		if (offset) {                                                           \
			ASN1_CALL((pos), asn1_dec_indef_end_fixup((ctrl), (pos), (end)));   \
		} else if ((pos) != (component_end)) {                                  \
			if ((ctrl)->debug & PRI_DEBUG_APDU) {                               \
				pri_message((ctrl),                                             \
					"  Skipping unused constructed component octets!\n");       \
			}                                                                   \
			(pos) = (component_end);                                            \
		}                                                                       \
	} while (0)

#define ASN1_DID_NOT_EXPECT_TAG(ctrl, tag)                                      \
	do {                                                                        \
		if ((ctrl)->debug & PRI_DEBUG_APDU) {                                   \
			pri_message((ctrl), "  Did not expect: %s\n", asn1_tag2str(tag));   \
		}                                                                       \
	} while (0)

#define ASN1_CHECK_TAG(ctrl, actual_tag, match_tag, expected_tag)   \
	do {                                                            \
		if ((match_tag) != (expected_tag)) {                        \
			ASN1_DID_NOT_EXPECT_TAG((ctrl), (actual_tag));          \
			return NULL;                                            \
		}                                                           \
	} while (0)


const unsigned char *asn1_dec_tag(const unsigned char *tag_pos, const unsigned char *end,
	unsigned *tag);
const unsigned char *asn1_dec_length(const unsigned char *len_pos,
	const unsigned char *end, int *length);
const unsigned char *asn1_dec_indef_end_fixup(struct pri *ctrl, const unsigned char *pos,
	const unsigned char *end);

const unsigned char *asn1_dec_boolean(struct pri *ctrl, const char *name, unsigned tag,
	const unsigned char *pos, const unsigned char *end, int32_t *value);
const unsigned char *asn1_dec_int(struct pri *ctrl, const char *name, unsigned tag,
	const unsigned char *pos, const unsigned char *end, int32_t *value);
const unsigned char *asn1_dec_null(struct pri *ctrl, const char *name, unsigned tag,
	const unsigned char *pos, const unsigned char *end);
const unsigned char *asn1_dec_oid(struct pri *ctrl, const char *name, unsigned tag,
	const unsigned char *pos, const unsigned char *end, struct asn1_oid *oid);
const unsigned char *asn1_dec_string_bin(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end, size_t buf_size,
	unsigned char *str, size_t *str_len);
const unsigned char *asn1_dec_string_max(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end, size_t buf_size,
	unsigned char *str, size_t *str_len);

const char *asn1_tag2str(unsigned tag);
void asn1_dump(struct pri *ctrl, const unsigned char *start_asn1,
	const unsigned char *end);


#define ASN1_LEN_FORM_SHORT     1	/*!< Hint that the final length will be less than 128 octets */
#define ASN1_LEN_FORM_LONG_U8   2	/*!< Hint that the final length will be less than 256 octets */
#define ASN1_LEN_FORM_LONG_U16  3	/*!< Hint that the final length will be less than 65536 octets */
#define ASN1_LEN_INIT(len_pos, end, form_hint)  \
	do {                                        \
		if ((end) < (len_pos) + (form_hint)) {  \
			return NULL;                        \
		}                                       \
		*(len_pos) = (form_hint);               \
		(len_pos) += (form_hint);               \
	} while (0)

#define ASN1_LEN_FIXUP(len_pos, component_end, end) \
	ASN1_CALL((component_end), asn1_enc_length_fixup((len_pos), (component_end), (end)))

/*! \brief Use to begin encoding explicit tags, SET, and SEQUENCE constructed groupings. */
#define ASN1_CONSTRUCTED_BEGIN(len_pos_save, pos, end, tag) \
	do {                                                    \
		if ((end) < (pos) + (1 + ASN1_LEN_FORM_SHORT)) {    \
			return NULL;                                    \
		}                                                   \
		*(pos)++ = (tag) | ASN1_PC_CONSTRUCTED;             \
		(len_pos_save) = (pos);                             \
		*(pos) = ASN1_LEN_FORM_SHORT;                       \
		(pos) += ASN1_LEN_FORM_SHORT;                       \
	} while (0)

/*! \brief Use to end encoding explicit tags, SET, and SEQUENCE constructed groupings. */
#define ASN1_CONSTRUCTED_END(len_pos, component_end, end)   \
	ASN1_CALL((component_end), asn1_enc_length_fixup((len_pos), (component_end), (end)))

#define ASN1_ENC_ERROR(ctrl, msg) \
	pri_error((ctrl), "%s error: %s\n", __FUNCTION__, (msg))

unsigned char *asn1_enc_length(unsigned char *len_pos, unsigned char *end,
	size_t str_len);
unsigned char *asn1_enc_length_fixup(unsigned char *len_pos,
	unsigned char *component_end, unsigned char *end);

unsigned char *asn1_enc_boolean(unsigned char *pos, unsigned char *end, unsigned tag,
	int32_t value);
unsigned char *asn1_enc_int(unsigned char *pos, unsigned char *end, unsigned tag,
	int32_t value);
unsigned char *asn1_enc_null(unsigned char *pos, unsigned char *end, unsigned tag);
unsigned char *asn1_enc_oid(unsigned char *pos, unsigned char *end, unsigned tag,
	const struct asn1_oid *oid);
unsigned char *asn1_enc_string_bin(unsigned char *pos, unsigned char *end, unsigned tag,
	const unsigned char *str, size_t str_len);
unsigned char *asn1_enc_string_max(unsigned char *pos, unsigned char *end, unsigned tag,
	const unsigned char *str, size_t max_len);

/* ------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif	/* _LIBPRI_ASN1_H */
/* ------------------------------------------------------------------- */
/* end asn1.h */
