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
 * \brief Switch type operations for:  NI2, 4ESS, 5ESS, DMS-100
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
 * \brief Encode the DMS-100 RLT_OperationInd result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_dms100_RLT_OperationInd_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args)
{
	return asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 0,
		args->dms100.RLT_OperationInd.call_id);
}

/*!
 * \brief Encode the DMS-100 RLT_ThirdParty invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_dms100_RLT_ThirdParty_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseDms100RLTThirdParty_ARG *rlt_thirdparty;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	rlt_thirdparty = &args->dms100.RLT_ThirdParty;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 0,
		rlt_thirdparty->call_id));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1,
		rlt_thirdparty->reason));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Decode the DMS-100 RLT_OperationInd result argument parameters.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param tag Component tag that identified this structure.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param args Arguments to fill in from the decoded buffer.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *rose_dec_dms100_RLT_OperationInd_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_result_args *args)
{
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 0);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "callId", tag, pos, end, &value));
	args->dms100.RLT_OperationInd.call_id = value;

	return pos;
}

/*!
 * \brief Decode the DMS-100 RLT_ThirdParty invoke argument parameters.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param tag Component tag that identified this structure.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param args Arguments to fill in from the decoded buffer.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *rose_dec_dms100_RLT_ThirdParty_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	struct roseDms100RLTThirdParty_ARG *rlt_third_party;

	rlt_third_party = &args->dms100.RLT_ThirdParty;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  RLT_ThirdParty %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 0);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "callId", tag, pos, seq_end, &value));
	rlt_third_party->call_id = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 1);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "reason", tag, pos, seq_end, &value));
	rlt_third_party->reason = value;

	/* Fixup will skip over any OPTIONAL information */
	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Encode the NI2 InformationFollowing invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_ni2_InformationFollowing_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args)
{
	/* Encode the unknown enumeration value. */
	return asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		args->ni2.InformationFollowing.value);
}

/*!
 * \brief Encode the NI2 InitiateTransfer invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_ni2_InitiateTransfer_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseNi2InitiateTransfer_ARG *initiate_transfer;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	initiate_transfer = &args->ni2.InitiateTransfer;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
		initiate_transfer->call_reference));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Decode the NI2 InformationFollowing invoke argument parameters.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param tag Component tag that identified this structure.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param args Arguments to fill in from the decoded buffer.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *rose_dec_ni2_InformationFollowing_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "unknown", tag, pos, end, &value));
	args->ni2.InformationFollowing.value = value;

	return pos;
}

/*!
 * \brief Decode the NI2 InitiateTransfer invoke argument parameters.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param tag Component tag that identified this structure.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param args Arguments to fill in from the decoded buffer.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *rose_dec_ni2_InitiateTransfer_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	struct roseNi2InitiateTransfer_ARG *initiate_transfer;

	initiate_transfer = &args->ni2.InitiateTransfer;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  InitiateTransfer %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "callReference", tag, pos, seq_end, &value));
	initiate_transfer->call_reference = value;

	/* Fixup will skip over any OPTIONAL information */
	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/* ------------------------------------------------------------------- */
/* end rose_other.c */
