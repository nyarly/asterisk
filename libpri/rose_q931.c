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
 * \brief ROSE Q.931 ie encode/decode functions
 *
 * \author Richard Mudgett <rmudgett@digium.com>
 */


#include "compat.h"
#include "libpri.h"
#include "pri_internal.h"
#include "rose.h"
#include "rose_internal.h"
#include "asn1.h"


/* ------------------------------------------------------------------- */

/*!
 * \brief Encode the Q.931 ie value.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_CLASS_APPLICATION | 0 unless the caller
 *   implicitly tags it otherwise.
 * \param q931ie Q931 ie information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_Q931ie(struct pri *ctrl, unsigned char *pos, unsigned char *end,
	unsigned tag, const struct roseQ931ie *q931ie)
{
	return asn1_enc_string_bin(pos, end, tag, q931ie->contents, q931ie->length);
}

/*!
 * \brief Decode the Q.931 ie value.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param q931ie Parameter storage to fill.
 * \param contents_size Amount of space "allocated" for the q931ie->contents
 * element.  Must have enough room for a null terminator.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *rose_dec_Q931ie(struct pri *ctrl, const char *name, unsigned tag,
	const unsigned char *pos, const unsigned char *end, struct roseQ931ie *q931ie,
	size_t contents_size)
{
	size_t str_len;

	/* NOTE: The q931ie->contents memory is "allocated" after the struct. */
	ASN1_CALL(pos, asn1_dec_string_bin(ctrl, name, tag, pos, end, contents_size,
		q931ie->contents, &str_len));
	q931ie->length = str_len;

	/*
	 * NOTE: We may want to do some basic decoding of the Q.931 ie list
	 * for debug purposes.
	 */

	return pos;
}

/* ------------------------------------------------------------------- */
/* end rose_q931.c */
