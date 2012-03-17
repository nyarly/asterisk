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
 * \brief Q.SIG ROSE Call-Diversion-Operations
 *
 * Call-Diversion-Operations ECMA-174 Annex F Table F.1
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
 * \brief Encode the IntResult type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param int_result Forwarding record information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_qsig_IntResult(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, unsigned tag, const struct roseQsigForwardingRecord *int_result)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
		&int_result->served_user_number));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		int_result->basic_service));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED, int_result->procedure));
	ASN1_CALL(pos, rose_enc_Address(ctrl, pos, end, ASN1_TAG_SEQUENCE,
		&int_result->diverted_to));
	if (int_result->remote_enabled) {
		/* Not the DEFAULT value */
		ASN1_CALL(pos, asn1_enc_boolean(pos, end, ASN1_TYPE_BOOLEAN,
			int_result->remote_enabled));
	}

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \internal
 * \brief Encode the IntResultList type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SET unless the caller implicitly
 *   tags it otherwise.
 * \param int_result_list Forwarding record list information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_qsig_IntResultList(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, unsigned tag,
	const struct roseQsigForwardingList *int_result_list)
{
	unsigned index;
	unsigned char *set_len;

	ASN1_CONSTRUCTED_BEGIN(set_len, pos, end, tag);

	for (index = 0; index < int_result_list->num_records; ++index) {
		ASN1_CALL(pos, rose_enc_qsig_IntResult(ctrl, pos, end, ASN1_TAG_SEQUENCE,
			&int_result_list->list[index]));
	}

	ASN1_CONSTRUCTED_END(set_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the ActivateDiversionQ invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_ActivateDiversionQ_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseQsigActivateDiversionQ_ARG *activate_diversion_q;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	activate_diversion_q = &args->qsig.ActivateDiversionQ;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		activate_diversion_q->procedure));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		activate_diversion_q->basic_service));
	ASN1_CALL(pos, rose_enc_Address(ctrl, pos, end, ASN1_TAG_SEQUENCE,
		&activate_diversion_q->diverted_to));
	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
		&activate_diversion_q->served_user_number));
	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
		&activate_diversion_q->activating_user_number));

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the DeactivateDiversionQ invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_DeactivateDiversionQ_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseQsigDeactivateDiversionQ_ARG *deactivate_diversion_q;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	deactivate_diversion_q = &args->qsig.DeactivateDiversionQ;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		deactivate_diversion_q->procedure));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		deactivate_diversion_q->basic_service));
	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
		&deactivate_diversion_q->served_user_number));
	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
		&deactivate_diversion_q->deactivating_user_number));

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the InterrogateDiversionQ invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_InterrogateDiversionQ_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseQsigInterrogateDiversionQ_ARG *interrogate_diversion_q;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	interrogate_diversion_q = &args->qsig.InterrogateDiversionQ;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		interrogate_diversion_q->procedure));
	if (interrogate_diversion_q->basic_service) {
		/* Not the DEFAULT value */
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
			interrogate_diversion_q->basic_service));
	}
	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
		&interrogate_diversion_q->served_user_number));
	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
		&interrogate_diversion_q->interrogating_user_number));

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the InterrogateDiversionQ result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_InterrogateDiversionQ_RES(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_result_args *args)
{
	return rose_enc_qsig_IntResultList(ctrl, pos, end, ASN1_TAG_SET,
		&args->qsig.InterrogateDiversionQ);
}

/*!
 * \brief Encode the CheckRestriction invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_CheckRestriction_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseQsigCheckRestriction_ARG *check_restriction;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	check_restriction = &args->qsig.CheckRestriction;
	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
		&check_restriction->served_user_number));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		check_restriction->basic_service));
	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
		&check_restriction->diverted_to_number));

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the CallRerouting invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_CallRerouting_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseQsigCallRerouting_ARG *call_rerouting;
	unsigned char *seq_len;
	unsigned char *exp_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	call_rerouting = &args->qsig.CallRerouting;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		call_rerouting->rerouting_reason));
	if (call_rerouting->original_rerouting_reason_present) {
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 0,
			call_rerouting->original_rerouting_reason));
	}
	ASN1_CALL(pos, rose_enc_Address(ctrl, pos, end, ASN1_TAG_SEQUENCE,
		&call_rerouting->called));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
		call_rerouting->diversion_counter));
	ASN1_CALL(pos, rose_enc_Q931ie(ctrl, pos, end, ASN1_CLASS_APPLICATION | 0,
		&call_rerouting->q931ie));

	/* EXPLICIT tag */
	ASN1_CONSTRUCTED_BEGIN(exp_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1);
	ASN1_CALL(pos, rose_enc_PresentedNumberUnscreened(ctrl, pos, end,
		&call_rerouting->last_rerouting));
	ASN1_CONSTRUCTED_END(exp_len, pos, end);

	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2,
		call_rerouting->subscription_option));

	if (call_rerouting->calling_subaddress.length) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(exp_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 3);
		ASN1_CALL(pos, rose_enc_PartySubaddress(ctrl, pos, end,
			&call_rerouting->calling_subaddress));
		ASN1_CONSTRUCTED_END(exp_len, pos, end);
	}

	/* EXPLICIT tag */
	ASN1_CONSTRUCTED_BEGIN(exp_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 4);
	ASN1_CALL(pos, rose_enc_PresentedNumberScreened(ctrl, pos, end,
		&call_rerouting->calling));
	ASN1_CONSTRUCTED_END(exp_len, pos, end);

	if (call_rerouting->calling_name_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(exp_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 5);
		ASN1_CALL(pos, rose_enc_qsig_Name(ctrl, pos, end,
			&call_rerouting->calling_name));
		ASN1_CONSTRUCTED_END(exp_len, pos, end);
	}

	if (call_rerouting->original_called_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(exp_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 6);
		ASN1_CALL(pos, rose_enc_PresentedNumberUnscreened(ctrl, pos, end,
			&call_rerouting->original_called));
		ASN1_CONSTRUCTED_END(exp_len, pos, end);
	}

	if (call_rerouting->redirecting_name_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(exp_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 7);
		ASN1_CALL(pos, rose_enc_qsig_Name(ctrl, pos, end,
			&call_rerouting->redirecting_name));
		ASN1_CONSTRUCTED_END(exp_len, pos, end);
	}

	if (call_rerouting->original_called_name_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(exp_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 8);
		ASN1_CALL(pos, rose_enc_qsig_Name(ctrl, pos, end,
			&call_rerouting->original_called_name));
		ASN1_CONSTRUCTED_END(exp_len, pos, end);
	}

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the DivertingLegInformation1 invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_DivertingLegInformation1_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseQsigDivertingLegInformation1_ARG *diverting_leg_information_1;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	diverting_leg_information_1 = &args->qsig.DivertingLegInformation1;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		diverting_leg_information_1->diversion_reason));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		diverting_leg_information_1->subscription_option));
	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
		&diverting_leg_information_1->nominated_number));

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the DivertingLegInformation2 invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_DivertingLegInformation2_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseQsigDivertingLegInformation2_ARG *diverting_leg_information_2;
	unsigned char *seq_len;
	unsigned char *exp_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	diverting_leg_information_2 = &args->qsig.DivertingLegInformation2;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
		diverting_leg_information_2->diversion_counter));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		diverting_leg_information_2->diversion_reason));
	if (diverting_leg_information_2->original_diversion_reason_present) {
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 0,
			diverting_leg_information_2->original_diversion_reason));
	}

	if (diverting_leg_information_2->diverting_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(exp_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1);
		ASN1_CALL(pos, rose_enc_PresentedNumberUnscreened(ctrl, pos, end,
			&diverting_leg_information_2->diverting));
		ASN1_CONSTRUCTED_END(exp_len, pos, end);
	}

	if (diverting_leg_information_2->original_called_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(exp_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2);
		ASN1_CALL(pos, rose_enc_PresentedNumberUnscreened(ctrl, pos, end,
			&diverting_leg_information_2->original_called));
		ASN1_CONSTRUCTED_END(exp_len, pos, end);
	}

	if (diverting_leg_information_2->redirecting_name_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(exp_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 3);
		ASN1_CALL(pos, rose_enc_qsig_Name(ctrl, pos, end,
			&diverting_leg_information_2->redirecting_name));
		ASN1_CONSTRUCTED_END(exp_len, pos, end);
	}

	if (diverting_leg_information_2->original_called_name_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(exp_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 4);
		ASN1_CALL(pos, rose_enc_qsig_Name(ctrl, pos, end,
			&diverting_leg_information_2->original_called_name));
		ASN1_CONSTRUCTED_END(exp_len, pos, end);
	}

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the DivertingLegInformation3 invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_DivertingLegInformation3_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseQsigDivertingLegInformation3_ARG *diverting_leg_information_3;
	unsigned char *seq_len;
	unsigned char *exp_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	diverting_leg_information_3 = &args->qsig.DivertingLegInformation3;
	ASN1_CALL(pos, asn1_enc_boolean(pos, end, ASN1_TYPE_BOOLEAN,
		diverting_leg_information_3->presentation_allowed_indicator));

	if (diverting_leg_information_3->redirection_name_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(exp_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 0);
		ASN1_CALL(pos, rose_enc_qsig_Name(ctrl, pos, end,
			&diverting_leg_information_3->redirection_name));
		ASN1_CONSTRUCTED_END(exp_len, pos, end);
	}

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \internal
 * \brief Decode the IntResult argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param int_result Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_qsig_IntResult(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigForwardingRecord *int_result)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	const unsigned char *save_pos;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s IntResult %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "servedUserNr", tag, pos, seq_end,
		&int_result->served_user_number));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "basicService", tag, pos, seq_end, &value));
	int_result->basic_service = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "procedure", tag, pos, seq_end, &value));
	int_result->procedure = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	ASN1_CALL(pos, rose_dec_Address(ctrl, "divertedToAddress", tag, pos, seq_end,
		&int_result->diverted_to));

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	int_result->remote_enabled = 0;	/* DEFAULT FALSE */
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag & ~ASN1_PC_MASK) {
		case ASN1_TYPE_BOOLEAN:
			/* Must not be constructed but we will not check for it for simplicity. */
			ASN1_CALL(pos, asn1_dec_boolean(ctrl, "remoteEnabled", tag, pos, seq_end,
				&value));
			int_result->remote_enabled = value;
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 2:
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  extension %s\n", asn1_tag2str(tag));
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
 * \internal
 * \brief Decode the IntResultList argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param int_result_list Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_qsig_IntResultList(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigForwardingList *int_result_list)
{
	int length;
	int set_offset;
	const unsigned char *set_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s IntResultList %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(set_end, set_offset, length, pos, end);

	int_result_list->num_records = 0;
	while (pos < set_end && *pos != ASN1_INDEF_TERM) {
		if (int_result_list->num_records < ARRAY_LEN(int_result_list->list)) {
			ASN1_CALL(pos, asn1_dec_tag(pos, set_end, &tag));
			ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
			ASN1_CALL(pos, rose_dec_qsig_IntResult(ctrl, "listEntry", tag, pos, set_end,
				&int_result_list->list[int_result_list->num_records]));
			++int_result_list->num_records;
		} else {
			/* Too many records */
			return NULL;
		}
	}

	ASN1_END_FIXUP(ctrl, pos, set_offset, set_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG ActivateDiversionQ invoke argument parameters.
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
const unsigned char *rose_dec_qsig_ActivateDiversionQ_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	struct roseQsigActivateDiversionQ_ARG *activate_diversion_q;

	activate_diversion_q = &args->qsig.ActivateDiversionQ;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  ActivateDiversionQ %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "procedure", tag, pos, seq_end, &value));
	activate_diversion_q->procedure = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "basicService", tag, pos, seq_end, &value));
	activate_diversion_q->basic_service = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	ASN1_CALL(pos, rose_dec_Address(ctrl, "divertedToAddress", tag, pos, seq_end,
		&activate_diversion_q->diverted_to));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "servedUserNr", tag, pos, seq_end,
		&activate_diversion_q->served_user_number));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "activatingUserNr", tag, pos, seq_end,
		&activate_diversion_q->activating_user_number));

	/* Fixup will skip over any OPTIONAL manufacturer extension information */
	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG DeactivateDiversionQ invoke argument parameters.
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
const unsigned char *rose_dec_qsig_DeactivateDiversionQ_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	struct roseQsigDeactivateDiversionQ_ARG *deactivate_diversion_q;

	deactivate_diversion_q = &args->qsig.DeactivateDiversionQ;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  DeactivateDiversionQ %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "procedure", tag, pos, seq_end, &value));
	deactivate_diversion_q->procedure = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "basicService", tag, pos, seq_end, &value));
	deactivate_diversion_q->basic_service = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "servedUserNr", tag, pos, seq_end,
		&deactivate_diversion_q->served_user_number));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "deactivatingUserNr", tag, pos, seq_end,
		&deactivate_diversion_q->deactivating_user_number));

	/* Fixup will skip over any OPTIONAL manufacturer extension information */
	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG InterrogateDiversionQ invoke argument parameters.
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
const unsigned char *rose_dec_qsig_InterrogateDiversionQ_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	struct roseQsigInterrogateDiversionQ_ARG *interrogate_diversion_q;

	interrogate_diversion_q = &args->qsig.InterrogateDiversionQ;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  InterrogateDiversionQ %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "procedure", tag, pos, seq_end, &value));
	interrogate_diversion_q->procedure = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	if (tag == ASN1_TYPE_ENUMERATED) {
		ASN1_CALL(pos, asn1_dec_int(ctrl, "basicService", tag, pos, seq_end, &value));
		interrogate_diversion_q->basic_service = value;

		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	} else {
		interrogate_diversion_q->basic_service = 0;	/* allServices */
	}

	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "servedUserNr", tag, pos, seq_end,
		&interrogate_diversion_q->served_user_number));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "interrogatingUserNr", tag, pos, seq_end,
		&interrogate_diversion_q->interrogating_user_number));

	/* Fixup will skip over any OPTIONAL manufacturer extension information */
	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG InterrogateDiversionQ result argument parameters.
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
const unsigned char *rose_dec_qsig_InterrogateDiversionQ_RES(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args)
{
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SET);
	return rose_dec_qsig_IntResultList(ctrl, "InterrogateDiversionQ", tag, pos, end,
		&args->qsig.InterrogateDiversionQ);
}

/*!
 * \brief Decode the Q.SIG CheckRestriction invoke argument parameters.
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
const unsigned char *rose_dec_qsig_CheckRestriction_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	struct roseQsigCheckRestriction_ARG *check_restriction;

	check_restriction = &args->qsig.CheckRestriction;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  CheckRestriction %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "servedUserNr", tag, pos, seq_end,
		&check_restriction->served_user_number));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "basicService", tag, pos, seq_end, &value));
	check_restriction->basic_service = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "divertedToNr", tag, pos, seq_end,
		&check_restriction->diverted_to_number));

	/* Fixup will skip over any OPTIONAL manufacturer extension information */
	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG CallRerouting invoke argument parameters.
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
const unsigned char *rose_dec_qsig_CallRerouting_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	int explicit_offset;
	const unsigned char *explicit_end;
	const unsigned char *seq_end;
	const unsigned char *save_pos;
	struct roseQsigCallRerouting_ARG *call_rerouting;

	call_rerouting = &args->qsig.CallRerouting;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  CallRerouting %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "reroutingReason", tag, pos, seq_end, &value));
	call_rerouting->rerouting_reason = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	if (tag == (ASN1_CLASS_CONTEXT_SPECIFIC | 0)) {
		ASN1_CALL(pos, asn1_dec_int(ctrl, "originalReroutingReason", tag, pos, seq_end,
			&value));
		call_rerouting->original_rerouting_reason = value;
		call_rerouting->original_rerouting_reason_present = 1;

		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	} else {
		call_rerouting->original_rerouting_reason_present = 0;
	}

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	ASN1_CALL(pos, rose_dec_Address(ctrl, "calledAddress", tag, pos, seq_end,
		&call_rerouting->called));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "diversionCounter", tag, pos, seq_end, &value));
	call_rerouting->diversion_counter = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag & ~ASN1_PC_MASK, ASN1_CLASS_APPLICATION | 0);
	ASN1_CALL(pos, rose_dec_Q931ie(ctrl, "pSS1InfoElement", tag, pos, seq_end,
		&call_rerouting->q931ie, sizeof(call_rerouting->q931ie_contents)));

	/* Remove EXPLICIT tag */
	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag,
		ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 1);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
	ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

	ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
	ASN1_CALL(pos, rose_dec_PresentedNumberUnscreened(ctrl, "lastReroutingNr", tag, pos,
		explicit_end, &call_rerouting->last_rerouting));

	ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 2);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "subscriptionOption", tag, pos, seq_end, &value));
	call_rerouting->subscription_option = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	if (tag == (ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 3)) {
		/* Remove EXPLICIT tag */
		if (ctrl->debug & PRI_DEBUG_APDU) {
			pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
		}
		ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
		ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

		ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
		ASN1_CALL(pos, rose_dec_PartySubaddress(ctrl, "callingPartySubaddress", tag, pos,
			explicit_end, &call_rerouting->calling_subaddress));

		ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);

		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	} else {
		call_rerouting->calling_subaddress.length = 0;
	}

	/* Remove EXPLICIT tag */
	ASN1_CHECK_TAG(ctrl, tag, tag,
		ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 4);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
	ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

	ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
	ASN1_CALL(pos, rose_dec_PresentedNumberScreened(ctrl, "callingNumber", tag, pos,
		explicit_end, &call_rerouting->calling));

	ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	call_rerouting->calling_name_present = 0;
	call_rerouting->redirecting_name_present = 0;
	call_rerouting->original_called_name_present = 0;
	call_rerouting->original_called_present = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag) {
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 5:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_qsig_Name(ctrl, "callingName", tag, pos,
				explicit_end, &call_rerouting->calling_name));
			call_rerouting->calling_name_present = 1;

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 6:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_PresentedNumberUnscreened(ctrl, "originalCalledNr",
				tag, pos, explicit_end, &call_rerouting->original_called));
			call_rerouting->original_called_present = 1;

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 7:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_qsig_Name(ctrl, "redirectingName", tag, pos,
				explicit_end, &call_rerouting->redirecting_name));
			call_rerouting->redirecting_name_present = 1;

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 8:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_qsig_Name(ctrl, "originalCalledName", tag, pos,
				explicit_end, &call_rerouting->original_called_name));
			call_rerouting->original_called_name_present = 1;

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 9:
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 9:
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 10:
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  extension %s\n", asn1_tag2str(tag));
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
 * \brief Decode the Q.SIG DivertingLegInformation1 invoke argument parameters.
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
const unsigned char *rose_dec_qsig_DivertingLegInformation1_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	struct roseQsigDivertingLegInformation1_ARG *diverting_leg_information_1;

	diverting_leg_information_1 = &args->qsig.DivertingLegInformation1;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  DivertingLegInformation1 %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "diversionReason", tag, pos, seq_end, &value));
	diverting_leg_information_1->diversion_reason = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "subscriptionOption", tag, pos, seq_end, &value));
	diverting_leg_information_1->subscription_option = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "nominatedNr", tag, pos, seq_end,
		&diverting_leg_information_1->nominated_number));

	/* Fixup will skip over any OPTIONAL manufacturer extension information */
	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG DivertingLegInformation2 invoke argument parameters.
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
const unsigned char *rose_dec_qsig_DivertingLegInformation2_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	int explicit_offset;
	const unsigned char *explicit_end;
	const unsigned char *seq_end;
	const unsigned char *save_pos;
	struct roseQsigDivertingLegInformation2_ARG *diverting_leg_information_2;

	diverting_leg_information_2 = &args->qsig.DivertingLegInformation2;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  DivertingLegInformation2 %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "diversionCounter", tag, pos, seq_end, &value));
	diverting_leg_information_2->diversion_counter = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "diversionReason", tag, pos, seq_end, &value));
	diverting_leg_information_2->diversion_reason = value;

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	diverting_leg_information_2->original_diversion_reason_present = 0;
	diverting_leg_information_2->diverting_present = 0;
	diverting_leg_information_2->original_called_present = 0;
	diverting_leg_information_2->redirecting_name_present = 0;
	diverting_leg_information_2->original_called_name_present = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag) {
		case ASN1_CLASS_CONTEXT_SPECIFIC | 0:
			ASN1_CALL(pos, asn1_dec_int(ctrl, "originalDiversionReason", tag, pos,
				seq_end, &value));
			diverting_leg_information_2->original_diversion_reason = value;
			diverting_leg_information_2->original_diversion_reason_present = 1;
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 1:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_PresentedNumberUnscreened(ctrl, "divertingNr", tag,
				pos, explicit_end, &diverting_leg_information_2->diverting));
			diverting_leg_information_2->diverting_present = 1;

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 2:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_PresentedNumberUnscreened(ctrl, "originalCalledNr",
				tag, pos, explicit_end, &diverting_leg_information_2->original_called));
			diverting_leg_information_2->original_called_present = 1;

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 3:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_qsig_Name(ctrl, "redirectingName", tag, pos,
				explicit_end, &diverting_leg_information_2->redirecting_name));
			diverting_leg_information_2->redirecting_name_present = 1;

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 4:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_qsig_Name(ctrl, "originalCalledName", tag, pos,
				explicit_end, &diverting_leg_information_2->original_called_name));
			diverting_leg_information_2->original_called_name_present = 1;

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 5:
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 5:
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 6:
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  extension %s\n", asn1_tag2str(tag));
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
 * \brief Decode the Q.SIG DivertingLegInformation3 invoke argument parameters.
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
const unsigned char *rose_dec_qsig_DivertingLegInformation3_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	int explicit_offset;
	const unsigned char *explicit_end;
	const unsigned char *seq_end;
	const unsigned char *save_pos;
	struct roseQsigDivertingLegInformation3_ARG *diverting_leg_information_3;

	diverting_leg_information_3 = &args->qsig.DivertingLegInformation3;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  DivertingLegInformation3 %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_BOOLEAN);
	ASN1_CALL(pos, asn1_dec_boolean(ctrl, "presentationAllowedIndicator", tag, pos,
		seq_end, &value));
	diverting_leg_information_3->presentation_allowed_indicator = value;

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	diverting_leg_information_3->redirection_name_present = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag) {
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 0:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_qsig_Name(ctrl, "redirectionName", tag, pos,
				explicit_end, &diverting_leg_information_3->redirection_name));
			diverting_leg_information_3->redirection_name_present = 1;

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 1:
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 2:
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  extension %s\n", asn1_tag2str(tag));
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

/* ------------------------------------------------------------------- */
/* end rose_qsig_diversion.c */
