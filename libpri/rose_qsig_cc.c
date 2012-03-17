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
 * \brief Q.SIG ROSE SS-CC-Operations (CC)
 *
 * SS-CC-Operations ECMA-186 Annex F Table F.1
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
 * \internal
 * \brief Encode the CcExtension type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 *
 * \details
 * CcExtension ::= CHOICE {
 *     none        NULL,
 *     single      [14] IMPLICIT Extension,
 *     multiple    [15] IMPLICIT SEQUENCE OF Extension
 * }
 */
static unsigned char *rose_enc_qsig_CcExtension(struct pri *ctrl, unsigned char *pos,
	unsigned char *end)
{
	return asn1_enc_null(pos, end, ASN1_TYPE_NULL);
}

/*!
 * \internal
 * \brief Encode the CcRequestArg type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param cc_request_arg Call-completion request arguments to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_qsig_CcRequestArg(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, unsigned tag, const struct roseQsigCcRequestArg *cc_request_arg)
{
	unsigned char *seq_len;
	unsigned char *exp_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, rose_enc_PresentedNumberUnscreened(ctrl, pos, end,
		&cc_request_arg->number_a));
	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end, &cc_request_arg->number_b));
	ASN1_CALL(pos, rose_enc_Q931ie(ctrl, pos, end, ASN1_CLASS_APPLICATION | 0,
		&cc_request_arg->q931ie));

	if (cc_request_arg->subaddr_a.length) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(exp_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 10);
		ASN1_CALL(pos, rose_enc_PartySubaddress(ctrl, pos, end,
			&cc_request_arg->subaddr_a));
		ASN1_CONSTRUCTED_END(exp_len, pos, end);
	}

	if (cc_request_arg->subaddr_b.length) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(exp_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 11);
		ASN1_CALL(pos, rose_enc_PartySubaddress(ctrl, pos, end,
			&cc_request_arg->subaddr_b));
		ASN1_CONSTRUCTED_END(exp_len, pos, end);
	}

	if (cc_request_arg->can_retain_service) {
		/* Not the DEFAULT value */
		ASN1_CALL(pos, asn1_enc_boolean(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 12,
			cc_request_arg->can_retain_service));
	}

	if (cc_request_arg->retain_sig_connection_present) {
		ASN1_CALL(pos, asn1_enc_boolean(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 13,
			cc_request_arg->retain_sig_connection));
	}

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the Q.SIG CcbsRequest invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_CcbsRequest_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return rose_enc_qsig_CcRequestArg(ctrl, pos, end, ASN1_TAG_SEQUENCE,
		&args->qsig.CcbsRequest);
}

/*!
 * \brief Encode the Q.SIG CcnrRequest invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_CcnrRequest_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return rose_enc_qsig_CcRequestArg(ctrl, pos, end, ASN1_TAG_SEQUENCE,
		&args->qsig.CcnrRequest);
}

/*!
 * \internal
 * \brief Encode the CcRequestRes type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param cc_request_res Call-completion request result to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_qsig_CcRequestRes(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, unsigned tag, const struct roseQsigCcRequestRes *cc_request_res)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	if (cc_request_res->no_path_reservation) {
		/* Not the DEFAULT value */
		ASN1_CALL(pos, asn1_enc_boolean(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 0,
			cc_request_res->no_path_reservation));
	}

	if (cc_request_res->retain_service) {
		/* Not the DEFAULT value */
		ASN1_CALL(pos, asn1_enc_boolean(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1,
			cc_request_res->retain_service));
	}

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the Q.SIG CcbsRequest result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_CcbsRequest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args)
{
	return rose_enc_qsig_CcRequestRes(ctrl, pos, end, ASN1_TAG_SEQUENCE,
		&args->qsig.CcbsRequest);
}

/*!
 * \brief Encode the Q.SIG CcnrRequest result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_CcnrRequest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args)
{
	return rose_enc_qsig_CcRequestRes(ctrl, pos, end, ASN1_TAG_SEQUENCE,
		&args->qsig.CcnrRequest);
}

/*!
 * \internal
 * \brief Encode the CcOptionalArg type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param cc_optional_arg Call-completion optional arguments to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_qsig_CcOptionalArg(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct roseQsigCcOptionalArg *cc_optional_arg)
{
	unsigned char *seq_len;
	unsigned char *exp_len;

	if (!cc_optional_arg->full_arg_present) {
		return rose_enc_qsig_CcExtension(ctrl, pos, end);
	}

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 0);

	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end, &cc_optional_arg->number_a));
	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end, &cc_optional_arg->number_b));
	ASN1_CALL(pos, rose_enc_Q931ie(ctrl, pos, end, ASN1_CLASS_APPLICATION | 0,
		&cc_optional_arg->q931ie));

	if (cc_optional_arg->subaddr_a.length) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(exp_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 10);
		ASN1_CALL(pos, rose_enc_PartySubaddress(ctrl, pos, end,
			&cc_optional_arg->subaddr_a));
		ASN1_CONSTRUCTED_END(exp_len, pos, end);
	}

	if (cc_optional_arg->subaddr_b.length) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(exp_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 11);
		ASN1_CALL(pos, rose_enc_PartySubaddress(ctrl, pos, end,
			&cc_optional_arg->subaddr_b));
		ASN1_CONSTRUCTED_END(exp_len, pos, end);
	}

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the Q.SIG CcCancel invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enq_qsig_CcCancel_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return rose_enc_qsig_CcOptionalArg(ctrl, pos, end, &args->qsig.CcCancel);
}

/*!
 * \brief Encode the Q.SIG CcExecPossible invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enq_qsig_CcExecPossible_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return rose_enc_qsig_CcOptionalArg(ctrl, pos, end, &args->qsig.CcExecPossible);
}

/*!
 * \brief Encode the Q.SIG CcPathReserve invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_CcPathReserve_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return rose_enc_qsig_CcExtension(ctrl, pos, end);
}

/*!
 * \brief Encode the Q.SIG CcPathReserve result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_CcPathReserve_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args)
{
	return rose_enc_qsig_CcExtension(ctrl, pos, end);
}

/*!
 * \brief Encode the Q.SIG CcRingout invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_CcRingout_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return rose_enc_qsig_CcExtension(ctrl, pos, end);
}

/*!
 * \brief Encode the Q.SIG CcSuspend invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_CcSuspend_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return rose_enc_qsig_CcExtension(ctrl, pos, end);
}

/*!
 * \brief Encode the Q.SIG CcResume invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_CcResume_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return rose_enc_qsig_CcExtension(ctrl, pos, end);
}

/*!
 * \internal
 * \brief Decode the CcExtension argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 *
 * \details
 * CcExtension ::= CHOICE {
 *     none        NULL,
 *     single      [14] IMPLICIT Extension,
 *     multiple    [15] IMPLICIT SEQUENCE OF Extension
 * }
 */
static const unsigned char *rose_dec_qsig_CcExtension(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end)
{
	int length;
	int ext_offset;
	const unsigned char *ext_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s CcExtension\n", name);
	}
	switch (tag & ~ASN1_PC_MASK) {
	case ASN1_TYPE_NULL:
		/* Must not be constructed but we will not check for it for simplicity. */
		return asn1_dec_null(ctrl, "none", tag, pos, end);
	case ASN1_CLASS_CONTEXT_SPECIFIC | 14:
		name = "single";
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 15:
		name = "multiple";
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(ext_end, ext_offset, length, pos, end);

	/* Fixup will skip over the manufacturer extension information */
	ASN1_END_FIXUP(ctrl, pos, ext_offset, ext_end, end);

	return pos;
}

/*!
 * \internal
 * \brief Decode the CcRequestArg argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param cc_request_arg Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_qsig_CcRequestArg(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigCcRequestArg *cc_request_arg)
{
	int32_t value;
	int length;
	int seq_offset;
	int explicit_offset;
	const unsigned char *explicit_end;
	const unsigned char *seq_end;
	const unsigned char *save_pos;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s CcRequestArg %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PresentedNumberUnscreened(ctrl, "numberA", tag, pos, seq_end,
		&cc_request_arg->number_a));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "numberB", tag, pos, seq_end,
		&cc_request_arg->number_b));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag & ~ASN1_PC_MASK, ASN1_CLASS_APPLICATION | 0);
	ASN1_CALL(pos, rose_dec_Q931ie(ctrl, "service", tag, pos, seq_end,
		&cc_request_arg->q931ie, sizeof(cc_request_arg->q931ie_contents)));

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	cc_request_arg->subaddr_a.length = 0;
	cc_request_arg->subaddr_b.length = 0;
	cc_request_arg->can_retain_service = 0;	/* DEFAULT FALSE */
	cc_request_arg->retain_sig_connection_present = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag) {
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 10:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_PartySubaddress(ctrl, "subaddrA", tag, pos,
				explicit_end, &cc_request_arg->subaddr_a));

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 11:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_PartySubaddress(ctrl, "subaddrB", tag, pos,
				explicit_end, &cc_request_arg->subaddr_b));

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 12:
			ASN1_CALL(pos, asn1_dec_boolean(ctrl, "can-retain-service", tag, pos,
				seq_end, &value));
			cc_request_arg->can_retain_service = value;
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 13:
			ASN1_CALL(pos, asn1_dec_boolean(ctrl, "retain-sig-connection", tag, pos,
				seq_end, &value));
			cc_request_arg->retain_sig_connection = value;
			cc_request_arg->retain_sig_connection_present = 1;
			break;
		case ASN1_TYPE_NULL:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 14:
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 14:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 15:
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 15:
			ASN1_CALL(pos, rose_dec_qsig_CcExtension(ctrl, "extension", tag, pos,
				seq_end));
			break;
		default:
			pos = save_pos;
			goto cancel_options;
		}
	}
cancel_options:;

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG CcbsRequest invoke argument parameters.
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
const unsigned char *rose_dec_qsig_CcbsRequest_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	return rose_dec_qsig_CcRequestArg(ctrl, "CcbsRequest", tag, pos, end,
		&args->qsig.CcbsRequest);
}

/*!
 * \brief Decode the Q.SIG CcnrRequest invoke argument parameters.
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
const unsigned char *rose_dec_qsig_CcnrRequest_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	return rose_dec_qsig_CcRequestArg(ctrl, "CcnrRequest", tag, pos, end,
		&args->qsig.CcnrRequest);
}

/*!
 * \internal
 * \brief Decode the CcRequestRes argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param cc_request_res Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_qsig_CcRequestRes(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigCcRequestRes *cc_request_res)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	const unsigned char *save_pos;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s CcRequestRes %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	cc_request_res->no_path_reservation = 0;	/* DEFAULT FALSE */
	cc_request_res->retain_service = 0;	/* DEFAULT FALSE */
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag) {
		case ASN1_CLASS_CONTEXT_SPECIFIC | 0:
			ASN1_CALL(pos, asn1_dec_boolean(ctrl, "no-path-reservation", tag, pos,
				seq_end, &value));
			cc_request_res->no_path_reservation = value;
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
			ASN1_CALL(pos, asn1_dec_boolean(ctrl, "retain-service", tag, pos, seq_end,
				&value));
			cc_request_res->retain_service = value;
			break;
		case ASN1_TYPE_NULL:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 14:
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 14:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 15:
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 15:
			ASN1_CALL(pos, rose_dec_qsig_CcExtension(ctrl, "extension", tag, pos,
				seq_end));
			break;
		default:
			pos = save_pos;
			goto cancel_options;
		}
	}
cancel_options:;

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG CcbsRequest result argument parameters.
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
const unsigned char *rose_dec_qsig_CcbsRequest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_result_args *args)
{
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	return rose_dec_qsig_CcRequestRes(ctrl, "CcbsRequest", tag, pos, end,
		&args->qsig.CcbsRequest);
}

/*!
 * \brief Decode the Q.SIG CcnrRequest result argument parameters.
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
const unsigned char *rose_dec_qsig_CcnrRequest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_result_args *args)
{
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	return rose_dec_qsig_CcRequestRes(ctrl, "CcnrRequest", tag, pos, end,
		&args->qsig.CcnrRequest);
}

/*!
 * \internal
 * \brief Decode the CcOptionalArg argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param cc_optional_arg Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_qsig_CcOptionalArg(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigCcOptionalArg *cc_optional_arg)
{
	int length;
	int seq_offset;
	int explicit_offset;
	const unsigned char *explicit_end;
	const unsigned char *seq_end;
	const unsigned char *save_pos;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s CcOptionalArg\n", name);
	}
	if (tag != (ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 0)) {
		cc_optional_arg->full_arg_present = 0;
		return rose_dec_qsig_CcExtension(ctrl, "extArg", tag, pos, end);
	}
	cc_optional_arg->full_arg_present = 1;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  fullArg %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "numberA", tag, pos, seq_end,
		&cc_optional_arg->number_a));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "numberB", tag, pos, seq_end,
		&cc_optional_arg->number_b));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag & ~ASN1_PC_MASK, ASN1_CLASS_APPLICATION | 0);
	ASN1_CALL(pos, rose_dec_Q931ie(ctrl, "service", tag, pos, seq_end,
		&cc_optional_arg->q931ie, sizeof(cc_optional_arg->q931ie_contents)));

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	cc_optional_arg->subaddr_a.length = 0;
	cc_optional_arg->subaddr_b.length = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag) {
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 10:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_PartySubaddress(ctrl, "subaddrA", tag, pos,
				explicit_end, &cc_optional_arg->subaddr_a));

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 11:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_PartySubaddress(ctrl, "subaddrB", tag, pos,
				explicit_end, &cc_optional_arg->subaddr_b));

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_TYPE_NULL:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 14:
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 14:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 15:
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 15:
			ASN1_CALL(pos, rose_dec_qsig_CcExtension(ctrl, "extension", tag, pos,
				seq_end));
			break;
		default:
			pos = save_pos;
			goto cancel_options;
		}
	}
cancel_options:;

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG CcCancel invoke argument parameters.
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
const unsigned char *rose_dec_qsig_CcCancel_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	return rose_dec_qsig_CcOptionalArg(ctrl, "CcCancel", tag, pos, end,
		&args->qsig.CcCancel);
}

/*!
 * \brief Decode the Q.SIG CcExecPossible invoke argument parameters.
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
const unsigned char *rose_dec_qsig_CcExecPossible_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	return rose_dec_qsig_CcOptionalArg(ctrl, "CcExecPossible", tag, pos, end,
		&args->qsig.CcCancel);
}

/*!
 * \brief Decode the Q.SIG CcPathReserve invoke argument parameters.
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
const unsigned char *rose_dec_qsig_CcPathReserve_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	return rose_dec_qsig_CcExtension(ctrl, "CcPathReserve", tag, pos, end);
}

/*!
 * \brief Decode the Q.SIG CcPathReserve result argument parameters.
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
const unsigned char *rose_dec_qsig_CcPathReserve_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_result_args *args)
{
	return rose_dec_qsig_CcExtension(ctrl, "CcPathReserve", tag, pos, end);
}

/*!
 * \brief Decode the Q.SIG CcRingout invoke argument parameters.
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
const unsigned char *rose_dec_qsig_CcRingout_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	return rose_dec_qsig_CcExtension(ctrl, "CcRingout", tag, pos, end);
}

/*!
 * \brief Decode the Q.SIG CcSuspend invoke argument parameters.
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
const unsigned char *rose_dec_qsig_CcSuspend_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	return rose_dec_qsig_CcExtension(ctrl, "CcSuspend", tag, pos, end);
}

/*!
 * \brief Decode the Q.SIG CcResume invoke argument parameters.
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
const unsigned char *rose_dec_qsig_CcResume_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	return rose_dec_qsig_CcExtension(ctrl, "CcResume", tag, pos, end);
}

/* ------------------------------------------------------------------- */
/* end rose_qsig_cc.c */
