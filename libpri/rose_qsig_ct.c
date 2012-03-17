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
 * \brief Q.SIG ROSE Call-Transfer-Operations (CT)
 *
 * Call-Transfer-Operations ECMA-178 Annex F Table F.1
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
 * \brief Encode the Q.SIG CallTransferIdentify result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_CallTransferIdentify_RES(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_result_args *args)
{
	unsigned char *seq_len;
	const struct roseQsigCTIdentifyRes_RES *call_transfer_identify;

	call_transfer_identify = &args->qsig.CallTransferIdentify;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ASN1_CALL(pos, asn1_enc_string_max(pos, end, ASN1_TYPE_NUMERIC_STRING,
		call_transfer_identify->call_id, sizeof(call_transfer_identify->call_id) - 1));
	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
		&call_transfer_identify->rerouting_number));

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the Q.SIG CallTransferInitiate invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_CallTransferInitiate_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args)
{
	unsigned char *seq_len;
	const struct roseQsigCTInitiateArg_ARG *call_transfer_initiate;

	call_transfer_initiate = &args->qsig.CallTransferInitiate;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ASN1_CALL(pos, asn1_enc_string_max(pos, end, ASN1_TYPE_NUMERIC_STRING,
		call_transfer_initiate->call_id, sizeof(call_transfer_initiate->call_id) - 1));
	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
		&call_transfer_initiate->rerouting_number));

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the Q.SIG CallTransferSetup invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_CallTransferSetup_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	unsigned char *seq_len;
	const struct roseQsigCTSetupArg_ARG *call_transfer_setup;

	call_transfer_setup = &args->qsig.CallTransferSetup;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ASN1_CALL(pos, asn1_enc_string_max(pos, end, ASN1_TYPE_NUMERIC_STRING,
		call_transfer_setup->call_id, sizeof(call_transfer_setup->call_id) - 1));

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the Q.SIG CallTransferActive invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_CallTransferActive_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	unsigned char *seq_len;
	const struct roseQsigCTActiveArg_ARG *call_transfer_active;

	call_transfer_active = &args->qsig.CallTransferActive;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ASN1_CALL(pos, rose_enc_PresentedAddressScreened(ctrl, pos, end,
		&call_transfer_active->connected));

	if (call_transfer_active->q931ie.length) {
		ASN1_CALL(pos, rose_enc_Q931ie(ctrl, pos, end, ASN1_CLASS_APPLICATION | 0,
			&call_transfer_active->q931ie));
	}

	if (call_transfer_active->connected_name_present) {
		ASN1_CALL(pos, rose_enc_qsig_Name(ctrl, pos, end,
			&call_transfer_active->connected_name));
	}

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the Q.SIG CallTransferComplete invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_CallTransferComplete_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args)
{
	unsigned char *seq_len;
	const struct roseQsigCTCompleteArg_ARG *call_transfer_complete;

	call_transfer_complete = &args->qsig.CallTransferComplete;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		call_transfer_complete->end_designation));

	ASN1_CALL(pos, rose_enc_PresentedNumberScreened(ctrl, pos, end,
		&call_transfer_complete->redirection));

	if (call_transfer_complete->q931ie.length) {
		ASN1_CALL(pos, rose_enc_Q931ie(ctrl, pos, end, ASN1_CLASS_APPLICATION | 0,
			&call_transfer_complete->q931ie));
	}

	if (call_transfer_complete->redirection_name_present) {
		ASN1_CALL(pos, rose_enc_qsig_Name(ctrl, pos, end,
			&call_transfer_complete->redirection_name));
	}

	if (call_transfer_complete->call_status) {
		/* Not the DEFAULT value */
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
			call_transfer_complete->call_status));
	}

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the Q.SIG CallTransferUpdate invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_CallTransferUpdate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	unsigned char *seq_len;
	const struct roseQsigCTUpdateArg_ARG *call_transfer_update;

	call_transfer_update = &args->qsig.CallTransferUpdate;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ASN1_CALL(pos, rose_enc_PresentedNumberScreened(ctrl, pos, end,
		&call_transfer_update->redirection));

	if (call_transfer_update->redirection_name_present) {
		ASN1_CALL(pos, rose_enc_qsig_Name(ctrl, pos, end,
			&call_transfer_update->redirection_name));
	}

	if (call_transfer_update->q931ie.length) {
		ASN1_CALL(pos, rose_enc_Q931ie(ctrl, pos, end, ASN1_CLASS_APPLICATION | 0,
			&call_transfer_update->q931ie));
	}

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the Q.SIG SubaddressTransfer invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_SubaddressTransfer_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	unsigned char *seq_len;
	const struct roseQsigSubaddressTransferArg_ARG *subaddress_transfer;

	subaddress_transfer = &args->qsig.SubaddressTransfer;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ASN1_CALL(pos, rose_enc_PartySubaddress(ctrl, pos, end,
		&subaddress_transfer->redirection_subaddress));

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the Q.SIG DummyArg invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 *
 * \details
 * DummyArg ::= CHOICE {
 *     none                NULL,
 *     extension           [1] IMPLICIT Extension,
 *     multipleExtension   [2] IMPLICIT SEQUENCE OF Extension
 * }
 */
unsigned char *rose_enc_qsig_DummyArg_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return asn1_enc_null(pos, end, ASN1_TYPE_NULL);
}

/*!
 * \brief Encode the Q.SIG DummyRes result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 *
 * \details
 * DummyRes ::= CHOICE {
 *     none                NULL,
 *     extension           [1] IMPLICIT Extension,
 *     multipleExtension   [2] IMPLICIT SEQUENCE OF Extension
 * }
 */
unsigned char *rose_enc_qsig_DummyRes_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args)
{
	return asn1_enc_null(pos, end, ASN1_TYPE_NULL);
}

/*!
 * \brief Decode the Q.SIG CallTransferIdentify result argument parameters.
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
const unsigned char *rose_dec_qsig_CallTransferIdentify_RES(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args)
{
	size_t str_len;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	struct roseQsigCTIdentifyRes_RES *call_transfer_identify;

	call_transfer_identify = &args->qsig.CallTransferIdentify;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  CallTransferIdentify %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag & ~ASN1_PC_MASK, tag, ASN1_TYPE_NUMERIC_STRING);
	ASN1_CALL(pos, asn1_dec_string_max(ctrl, "callIdentity", tag, pos, seq_end,
		sizeof(call_transfer_identify->call_id), call_transfer_identify->call_id,
		&str_len));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "reroutingNumber", tag, pos, seq_end,
		&call_transfer_identify->rerouting_number));

	/* Fixup will skip over any OPTIONAL manufacturer extension information */
	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG CallTransferInitiate invoke argument parameters.
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
const unsigned char *rose_dec_qsig_CallTransferInitiate_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	size_t str_len;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	struct roseQsigCTInitiateArg_ARG *call_transfer_initiate;

	call_transfer_initiate = &args->qsig.CallTransferInitiate;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  CallTransferInitiate %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag & ~ASN1_PC_MASK, tag, ASN1_TYPE_NUMERIC_STRING);
	ASN1_CALL(pos, asn1_dec_string_max(ctrl, "callIdentity", tag, pos, seq_end,
		sizeof(call_transfer_initiate->call_id), call_transfer_initiate->call_id,
		&str_len));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "reroutingNumber", tag, pos, seq_end,
		&call_transfer_initiate->rerouting_number));

	/* Fixup will skip over any OPTIONAL manufacturer extension information */
	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG CallTransferSetup invoke argument parameters.
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
const unsigned char *rose_dec_qsig_CallTransferSetup_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	size_t str_len;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	struct roseQsigCTSetupArg_ARG *call_transfer_setup;

	call_transfer_setup = &args->qsig.CallTransferSetup;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  CallTransferSetup %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag & ~ASN1_PC_MASK, tag, ASN1_TYPE_NUMERIC_STRING);
	ASN1_CALL(pos, asn1_dec_string_max(ctrl, "callIdentity", tag, pos, seq_end,
		sizeof(call_transfer_setup->call_id), call_transfer_setup->call_id, &str_len));

	/* Fixup will skip over any OPTIONAL manufacturer extension information */
	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG CallTransferActive invoke argument parameters.
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
const unsigned char *rose_dec_qsig_CallTransferActive_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	const unsigned char *save_pos;
	struct roseQsigCTActiveArg_ARG *call_transfer_active;

	call_transfer_active = &args->qsig.CallTransferActive;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  CallTransferActive %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PresentedAddressScreened(ctrl, "connectedAddress", tag, pos,
		seq_end, &call_transfer_active->connected));

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	call_transfer_active->q931ie.length = 0;
	call_transfer_active->connected_name_present = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag & ~ASN1_PC_MASK) {
		case ASN1_CLASS_APPLICATION | 0:
			ASN1_CALL(pos, rose_dec_Q931ie(ctrl, "basicCallInfoElements", tag, pos,
				seq_end, &call_transfer_active->q931ie,
				sizeof(call_transfer_active->q931ie_contents)));
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 0:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 2:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 3:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 4:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 7:
			ASN1_CALL(pos, rose_dec_qsig_Name(ctrl, "connectedName", tag, pos, seq_end,
				&call_transfer_active->connected_name));
			call_transfer_active->connected_name_present = 1;
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 9:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 10:
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  argumentExtension %s\n", asn1_tag2str(tag));
			}
			/* Fixup will skip over the manufacturer extension information */
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
 * \brief Decode the Q.SIG CallTransferComplete invoke argument parameters.
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
const unsigned char *rose_dec_qsig_CallTransferComplete_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	const unsigned char *save_pos;
	struct roseQsigCTCompleteArg_ARG *call_transfer_complete;

	call_transfer_complete = &args->qsig.CallTransferComplete;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  CallTransferComplete %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "endDesignation", tag, pos, seq_end, &value));
	call_transfer_complete->end_designation = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PresentedNumberScreened(ctrl, "redirectionNumber", tag, pos,
		seq_end, &call_transfer_complete->redirection));

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	call_transfer_complete->q931ie.length = 0;
	call_transfer_complete->redirection_name_present = 0;
	call_transfer_complete->call_status = 0;	/* DEFAULT answered */
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag & ~ASN1_PC_MASK) {
		case ASN1_CLASS_APPLICATION | 0:
			ASN1_CALL(pos, rose_dec_Q931ie(ctrl, "basicCallInfoElements", tag, pos,
				seq_end, &call_transfer_complete->q931ie,
				sizeof(call_transfer_complete->q931ie_contents)));
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 0:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 2:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 3:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 4:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 7:
			ASN1_CALL(pos, rose_dec_qsig_Name(ctrl, "redirectionName", tag, pos, seq_end,
				&call_transfer_complete->redirection_name));
			call_transfer_complete->redirection_name_present = 1;
			break;
		case ASN1_TYPE_ENUMERATED:
			/* Must not be constructed but we will not check for it for simplicity. */
			ASN1_CALL(pos, asn1_dec_int(ctrl, "callStatus", tag, pos, seq_end, &value));
			call_transfer_complete->call_status = value;
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 9:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 10:
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  argumentExtension %s\n", asn1_tag2str(tag));
			}
			/* Fixup will skip over the manufacturer extension information */
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
 * \brief Decode the Q.SIG CallTransferUpdate invoke argument parameters.
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
const unsigned char *rose_dec_qsig_CallTransferUpdate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	const unsigned char *save_pos;
	struct roseQsigCTUpdateArg_ARG *call_transfer_update;

	call_transfer_update = &args->qsig.CallTransferUpdate;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  CallTransferUpdate %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PresentedNumberScreened(ctrl, "redirectionNumber", tag, pos,
		seq_end, &call_transfer_update->redirection));

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	call_transfer_update->redirection_name_present = 0;
	call_transfer_update->q931ie.length = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag & ~ASN1_PC_MASK) {
		case ASN1_CLASS_CONTEXT_SPECIFIC | 0:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 2:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 3:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 4:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 7:
			ASN1_CALL(pos, rose_dec_qsig_Name(ctrl, "redirectionName", tag, pos, seq_end,
				&call_transfer_update->redirection_name));
			call_transfer_update->redirection_name_present = 1;
			break;
		case ASN1_CLASS_APPLICATION | 0:
			ASN1_CALL(pos, rose_dec_Q931ie(ctrl, "basicCallInfoElements", tag, pos,
				seq_end, &call_transfer_update->q931ie,
				sizeof(call_transfer_update->q931ie_contents)));
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 9:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 10:
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  argumentExtension %s\n", asn1_tag2str(tag));
			}
			/* Fixup will skip over the manufacturer extension information */
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
 * \brief Decode the Q.SIG SubaddressTransfer invoke argument parameters.
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
const unsigned char *rose_dec_qsig_SubaddressTransfer_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	struct roseQsigSubaddressTransferArg_ARG *subaddress_transfer;

	subaddress_transfer = &args->qsig.SubaddressTransfer;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  SubaddressTransfer %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartySubaddress(ctrl, "redirectionSubaddress", tag, pos,
		seq_end, &subaddress_transfer->redirection_subaddress));

	/* Fixup will skip over any OPTIONAL manufacturer extension information */
	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG DummyArg invoke argument parameters.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param tag Component tag that identified this structure.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param args Arguments to fill in from the decoded buffer.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 *
 * \details
 * DummyArg ::= CHOICE {
 *     none                NULL,
 *     extension           [1] IMPLICIT Extension,
 *     multipleExtension   [2] IMPLICIT SEQUENCE OF Extension
 * }
 */
const unsigned char *rose_dec_qsig_DummyArg_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	const char *name;
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	switch (tag) {
	case ASN1_TYPE_NULL:
		return asn1_dec_null(ctrl, "none", tag, pos, end);
	case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 1:
		name = "extension Extension";
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 2:
		name = "multipleExtension SEQUENCE OF Extension";
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	/* Fixup will skip over the manufacturer extension information */
	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG DummyRes result argument parameters.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param tag Component tag that identified this structure.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param args Arguments to fill in from the decoded buffer.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 *
 * \details
 * DummyRes ::= CHOICE {
 *     none                NULL,
 *     extension           [1] IMPLICIT Extension,
 *     multipleExtension   [2] IMPLICIT SEQUENCE OF Extension
 * }
 */
const unsigned char *rose_dec_qsig_DummyRes_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_result_args *args)
{
	const char *name;
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	switch (tag) {
	case ASN1_TYPE_NULL:
		return asn1_dec_null(ctrl, "none", tag, pos, end);
	case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 1:
		name = "extension Extension";
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 2:
		name = "multipleExtension SEQUENCE OF Extension";
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	/* Fixup will skip over the manufacturer extension information */
	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/* ------------------------------------------------------------------- */
/* end rose_qsig_ct.c */
