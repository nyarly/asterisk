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
 * \brief ROSE Explicit Call Transfer operations.
 *
 * Explicit Call Transfer (ECT) Supplementary Services ETS 300 369-1
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
 * \brief Encode the ExplicitEctExecute invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_ExplicitEctExecute_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
		args->etsi.ExplicitEctExecute.link_id);
}

/*!
 * \brief Encode the SubaddressTransfer invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_SubaddressTransfer_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return rose_enc_PartySubaddress(ctrl, pos, end,
		&args->etsi.SubaddressTransfer.subaddress);
}

/*!
 * \brief Encode the EctLinkIdRequest result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_EctLinkIdRequest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args)
{
	return asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
		args->etsi.EctLinkIdRequest.link_id);
}

/*!
 * \brief Encode the EctInform invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_EctInform_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiEctInform_ARG *ect_inform;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ect_inform = &args->etsi.EctInform;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED, ect_inform->status));
	if (ect_inform->redirection_present) {
		ASN1_CALL(pos, rose_enc_PresentedNumberUnscreened(ctrl, pos, end,
			&ect_inform->redirection));
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the EctLoopTest invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_EctLoopTest_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
		args->etsi.EctLoopTest.call_transfer_id);
}

/*!
 * \brief Encode the EctLoopTest result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_EctLoopTest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args)
{
	return asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		args->etsi.EctLoopTest.loop_result);
}

/*!
 * \brief Decode the ExplicitEctExecute invoke argument parameters.
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
const unsigned char *rose_dec_etsi_ExplicitEctExecute_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "linkId", tag, pos, end, &value));
	args->etsi.ExplicitEctExecute.link_id = value;

	return pos;
}

/*!
 * \brief Decode the SubaddressTransfer invoke argument parameters.
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
const unsigned char *rose_dec_etsi_SubaddressTransfer_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	return rose_dec_PartySubaddress(ctrl, "transferredToSubaddress", tag, pos, end,
		&args->etsi.SubaddressTransfer.subaddress);
}

/*!
 * \brief Decode the EctLinkIdRequest result argument parameters.
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
const unsigned char *rose_dec_etsi_EctLinkIdRequest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_result_args *args)
{
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "linkId", tag, pos, end, &value));
	args->etsi.EctLinkIdRequest.link_id = value;

	return pos;
}

/*!
 * \brief Decode the EctInform invoke argument parameters.
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
const unsigned char *rose_dec_etsi_EctInform_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	struct roseEtsiEctInform_ARG *ect_inform;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	int32_t value;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  EctInform %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ect_inform = &args->etsi.EctInform;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "callStatus", tag, pos, seq_end, &value));
	ect_inform->status = value;

	if (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CALL(pos, rose_dec_PresentedNumberUnscreened(ctrl, "redirectionNumber", tag,
			pos, seq_end, &ect_inform->redirection));
		ect_inform->redirection_present = 1;
	} else {
		ect_inform->redirection_present = 0;
	}

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the EctLoopTest invoke argument parameters.
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
const unsigned char *rose_dec_etsi_EctLoopTest_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "callTransferId", tag, pos, end, &value));
	args->etsi.EctLoopTest.call_transfer_id = value;

	return pos;
}

/*!
 * \brief Decode the EctLoopTest result argument parameters.
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
const unsigned char *rose_dec_etsi_EctLoopTest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_result_args *args)
{
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "loopResult", tag, pos, end, &value));
	args->etsi.EctLoopTest.loop_result = value;

	return pos;
}

/* ------------------------------------------------------------------- */
/* end rose_etsi_ect.c */
