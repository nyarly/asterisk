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
 * \brief ROSE Call diversion operations
 *
 * Diversion Supplementary Services ETS 300 207-1 Table 3
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
 * \brief Encode the ServedUserNr type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param served_user_number Served user number information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_etsi_ServedUserNumber(struct pri *ctrl,
	unsigned char *pos, unsigned char *end,
	const struct rosePartyNumber *served_user_number)
{
	if (served_user_number->length) {
		/* Forward this number */
		pos = rose_enc_PartyNumber(ctrl, pos, end, served_user_number);
	} else {
		/* Forward all numbers */
		pos = asn1_enc_null(pos, end, ASN1_TYPE_NULL);
	}

	return pos;
}

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
static unsigned char *rose_enc_etsi_IntResult(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, unsigned tag, const struct roseEtsiForwardingRecord *int_result)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, rose_enc_etsi_ServedUserNumber(ctrl, pos, end,
		&int_result->served_user_number));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		int_result->basic_service));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED, int_result->procedure));
	ASN1_CALL(pos, rose_enc_Address(ctrl, pos, end, ASN1_TAG_SEQUENCE,
		&int_result->forwarded_to));

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
static unsigned char *rose_enc_etsi_IntResultList(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, unsigned tag,
	const struct roseEtsiForwardingList *int_result_list)
{
	unsigned index;
	unsigned char *set_len;

	ASN1_CONSTRUCTED_BEGIN(set_len, pos, end, tag);

	for (index = 0; index < int_result_list->num_records; ++index) {
		ASN1_CALL(pos, rose_enc_etsi_IntResult(ctrl, pos, end, ASN1_TAG_SEQUENCE,
			&int_result_list->list[index]));
	}

	ASN1_CONSTRUCTED_END(set_len, pos, end);

	return pos;
}

/*!
 * \internal
 * \brief Encode the ServedUserNumberList type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SET unless the caller implicitly
 *   tags it otherwise.
 * \param served_user_number_list Served user record list information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_etsi_ServedUserNumberList(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, unsigned tag,
	const struct roseEtsiServedUserNumberList *served_user_number_list)
{
	unsigned index;
	unsigned char *set_len;

	ASN1_CONSTRUCTED_BEGIN(set_len, pos, end, tag);

	for (index = 0; index < served_user_number_list->num_records; ++index) {
		ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
			&served_user_number_list->number[index]));
	}

	ASN1_CONSTRUCTED_END(set_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the ActivationDiversion invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_ActivationDiversion_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiActivationDiversion_ARG *activation_diversion;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	activation_diversion = &args->etsi.ActivationDiversion;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		activation_diversion->procedure));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		activation_diversion->basic_service));
	ASN1_CALL(pos, rose_enc_Address(ctrl, pos, end, ASN1_TAG_SEQUENCE,
		&activation_diversion->forwarded_to));
	ASN1_CALL(pos, rose_enc_etsi_ServedUserNumber(ctrl, pos, end,
		&activation_diversion->served_user_number));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the DeactivationDiversion invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_DeactivationDiversion_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiDeactivationDiversion_ARG *deactivation_diversion;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	deactivation_diversion = &args->etsi.DeactivationDiversion;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		deactivation_diversion->procedure));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		deactivation_diversion->basic_service));
	ASN1_CALL(pos, rose_enc_etsi_ServedUserNumber(ctrl, pos, end,
		&deactivation_diversion->served_user_number));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the ActivationStatusNotificationDiv invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_ActivationStatusNotificationDiv_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiActivationStatusNotificationDiv_ARG
		*activation_status_notification_div;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	activation_status_notification_div = &args->etsi.ActivationStatusNotificationDiv;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		activation_status_notification_div->procedure));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		activation_status_notification_div->basic_service));
	ASN1_CALL(pos, rose_enc_Address(ctrl, pos, end, ASN1_TAG_SEQUENCE,
		&activation_status_notification_div->forwarded_to));
	ASN1_CALL(pos, rose_enc_etsi_ServedUserNumber(ctrl, pos, end,
		&activation_status_notification_div->served_user_number));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the DeactivationStatusNotificationDiv invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_DeactivationStatusNotificationDiv_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiDeactivationStatusNotificationDiv_ARG
		*deactivation_status_notification_div;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	deactivation_status_notification_div = &args->etsi.DeactivationStatusNotificationDiv;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		deactivation_status_notification_div->procedure));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		deactivation_status_notification_div->basic_service));
	ASN1_CALL(pos, rose_enc_etsi_ServedUserNumber(ctrl, pos, end,
		&deactivation_status_notification_div->served_user_number));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the InterrogationDiversion invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_InterrogationDiversion_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiInterrogationDiversion_ARG *interrogation_diversion;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	interrogation_diversion = &args->etsi.InterrogationDiversion;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		interrogation_diversion->procedure));
	if (interrogation_diversion->basic_service) {
		/* Not the DEFAULT value */
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
			interrogation_diversion->basic_service));
	}
	ASN1_CALL(pos, rose_enc_etsi_ServedUserNumber(ctrl, pos, end,
		&interrogation_diversion->served_user_number));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the InterrogationDiversion result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_InterrogationDiversion_RES(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_result_args *args)
{
	return rose_enc_etsi_IntResultList(ctrl, pos, end, ASN1_TAG_SET,
		&args->etsi.InterrogationDiversion);
}

/*!
 * \brief Encode the DiversionInformation invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_DiversionInformation_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiDiversionInformation_ARG *diversion_information;
	unsigned char *seq_len;
	unsigned char *explicit_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	diversion_information = &args->etsi.DiversionInformation;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		diversion_information->diversion_reason));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		diversion_information->basic_service));
	if (diversion_information->served_user_subaddress.length) {
		ASN1_CALL(pos, rose_enc_PartySubaddress(ctrl, pos, end,
			&diversion_information->served_user_subaddress));
	}
	if (diversion_information->calling_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 0);
		ASN1_CALL(pos, rose_enc_PresentedAddressScreened(ctrl, pos, end,
			&diversion_information->calling));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}
	if (diversion_information->original_called_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1);
		ASN1_CALL(pos, rose_enc_PresentedNumberUnscreened(ctrl, pos, end,
			&diversion_information->original_called));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}
	if (diversion_information->last_diverting_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2);
		ASN1_CALL(pos, rose_enc_PresentedNumberUnscreened(ctrl, pos, end,
			&diversion_information->last_diverting));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}
	if (diversion_information->last_diverting_reason_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 3);
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
			diversion_information->last_diverting_reason));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}
	if (diversion_information->q931ie.length) {
		ASN1_CALL(pos, rose_enc_Q931ie(ctrl, pos, end, ASN1_CLASS_APPLICATION | 0,
			&diversion_information->q931ie));
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the CallDeflection invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CallDeflection_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiCallDeflection_ARG *call_deflection;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	call_deflection = &args->etsi.CallDeflection;
	ASN1_CALL(pos, rose_enc_Address(ctrl, pos, end, ASN1_TAG_SEQUENCE,
		&call_deflection->deflection));
	if (call_deflection->presentation_allowed_to_diverted_to_user_present) {
		ASN1_CALL(pos, asn1_enc_boolean(pos, end, ASN1_TYPE_BOOLEAN,
			call_deflection->presentation_allowed_to_diverted_to_user));
	}

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
unsigned char *rose_enc_etsi_CallRerouting_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiCallRerouting_ARG *call_rerouting;
	unsigned char *seq_len;
	unsigned char *explicit_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	call_rerouting = &args->etsi.CallRerouting;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		call_rerouting->rerouting_reason));
	ASN1_CALL(pos, rose_enc_Address(ctrl, pos, end, ASN1_TAG_SEQUENCE,
		&call_rerouting->called_address));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
		call_rerouting->rerouting_counter));
	ASN1_CALL(pos, rose_enc_Q931ie(ctrl, pos, end, ASN1_CLASS_APPLICATION | 0,
		&call_rerouting->q931ie));

	/* EXPLICIT tag */
	ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1);
	ASN1_CALL(pos, rose_enc_PresentedNumberUnscreened(ctrl, pos, end,
		&call_rerouting->last_rerouting));
	ASN1_CONSTRUCTED_END(explicit_len, pos, end);

	if (call_rerouting->subscription_option) {
		/* Not the DEFAULT value */
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2);
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
			call_rerouting->subscription_option));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}

	if (call_rerouting->calling_subaddress.length) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 3);
		ASN1_CALL(pos, rose_enc_PartySubaddress(ctrl, pos, end,
			&call_rerouting->calling_subaddress));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the InterrogateServedUserNumbers result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_InterrogateServedUserNumbers_RES(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_result_args *args)
{
	return rose_enc_etsi_ServedUserNumberList(ctrl, pos, end, ASN1_TAG_SET,
		&args->etsi.InterrogateServedUserNumbers);
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
unsigned char *rose_enc_etsi_DivertingLegInformation1_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiDivertingLegInformation1_ARG *diverting_leg_information_1;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	diverting_leg_information_1 = &args->etsi.DivertingLegInformation1;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		diverting_leg_information_1->diversion_reason));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		diverting_leg_information_1->subscription_option));
	if (diverting_leg_information_1->diverted_to_present) {
		ASN1_CALL(pos, rose_enc_PresentedNumberUnscreened(ctrl, pos, end,
			&diverting_leg_information_1->diverted_to));
	}

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
unsigned char *rose_enc_etsi_DivertingLegInformation2_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiDivertingLegInformation2_ARG *diverting_leg_information_2;
	unsigned char *seq_len;
	unsigned char *explicit_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	diverting_leg_information_2 = &args->etsi.DivertingLegInformation2;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
		diverting_leg_information_2->diversion_counter));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		diverting_leg_information_2->diversion_reason));

	if (diverting_leg_information_2->diverting_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1);
		ASN1_CALL(pos, rose_enc_PresentedNumberUnscreened(ctrl, pos, end,
			&diverting_leg_information_2->diverting));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}

	if (diverting_leg_information_2->original_called_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2);
		ASN1_CALL(pos, rose_enc_PresentedNumberUnscreened(ctrl, pos, end,
			&diverting_leg_information_2->original_called));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}

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
unsigned char *rose_enc_etsi_DivertingLegInformation3_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args)
{
	return asn1_enc_boolean(pos, end, ASN1_TYPE_BOOLEAN,
		args->etsi.DivertingLegInformation3.presentation_allowed_indicator);
}

/*!
 * \internal
 * \brief Decode the ServedUserNr argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param served_user_number Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_etsi_ServedUserNumber(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct rosePartyNumber *served_user_number)
{
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s ServedUserNumber\n", name);
	}
	if (tag == ASN1_TYPE_NULL) {
		served_user_number->length = 0;
		pos = asn1_dec_null(ctrl, "allNumbers", tag, pos, end);
	} else {
		/* Must be a PartyNumber (Which is itself a CHOICE) */
		pos =
			rose_dec_PartyNumber(ctrl, "individualNumber", tag, pos, end,
				served_user_number);
	}
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
static const unsigned char *rose_dec_etsi_IntResult(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiForwardingRecord *int_result)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s IntResult %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_etsi_ServedUserNumber(ctrl, "servedUserNr", tag, pos,
		seq_end, &int_result->served_user_number));

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
	ASN1_CALL(pos, rose_dec_Address(ctrl, "forwardedToAddress", tag, pos, seq_end,
		&int_result->forwarded_to));

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
static const unsigned char *rose_dec_etsi_IntResultList(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiForwardingList *int_result_list)
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
			ASN1_CALL(pos, rose_dec_etsi_IntResult(ctrl, "listEntry", tag, pos, set_end,
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
 * \internal
 * \brief Decode the ServedUserNumberList argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param served_user_number_list Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_etsi_ServedUserNumberList(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiServedUserNumberList *served_user_number_list)
{
	int length;
	int set_offset;
	const unsigned char *set_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s ServedUserNumberList %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(set_end, set_offset, length, pos, end);

	served_user_number_list->num_records = 0;
	while (pos < set_end && *pos != ASN1_INDEF_TERM) {
		if (served_user_number_list->num_records <
			ARRAY_LEN(served_user_number_list->number)) {
			ASN1_CALL(pos, asn1_dec_tag(pos, set_end, &tag));
			ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "listEntry", tag, pos, set_end,
				&served_user_number_list->number[served_user_number_list->num_records]));
			++served_user_number_list->num_records;
		} else {
			/* Too many records */
			return NULL;
		}
	}

	ASN1_END_FIXUP(ctrl, pos, set_offset, set_end, end);

	return pos;
}

/*!
 * \brief Decode the ActivationDiversion invoke argument parameters.
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
const unsigned char *rose_dec_etsi_ActivationDiversion_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	struct roseEtsiActivationDiversion_ARG *activation_diversion;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  ActivationDiversion %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	activation_diversion = &args->etsi.ActivationDiversion;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "procedure", tag, pos, seq_end, &value));
	activation_diversion->procedure = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "basicService", tag, pos, seq_end, &value));
	activation_diversion->basic_service = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	ASN1_CALL(pos, rose_dec_Address(ctrl, "forwardedToAddress", tag, pos, seq_end,
		&activation_diversion->forwarded_to));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_etsi_ServedUserNumber(ctrl, "servedUserNr", tag, pos,
		seq_end, &activation_diversion->served_user_number));

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the DeactivationDiversion invoke argument parameters.
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
const unsigned char *rose_dec_etsi_DeactivationDiversion_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	struct roseEtsiDeactivationDiversion_ARG *deactivation_diversion;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  DeactivationDiversion %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	deactivation_diversion = &args->etsi.DeactivationDiversion;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "procedure", tag, pos, seq_end, &value));
	deactivation_diversion->procedure = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "basicService", tag, pos, seq_end, &value));
	deactivation_diversion->basic_service = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_etsi_ServedUserNumber(ctrl, "servedUserNr", tag, pos,
		seq_end, &deactivation_diversion->served_user_number));

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the ActivationStatusNotificationDiv invoke argument parameters.
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
const unsigned char *rose_dec_etsi_ActivationStatusNotificationDiv_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	struct roseEtsiActivationStatusNotificationDiv_ARG
		*activation_status_notification_div;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  ActivationStatusNotificationDiv %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	activation_status_notification_div = &args->etsi.ActivationStatusNotificationDiv;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "procedure", tag, pos, seq_end, &value));
	activation_status_notification_div->procedure = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "basicService", tag, pos, seq_end, &value));
	activation_status_notification_div->basic_service = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	ASN1_CALL(pos, rose_dec_Address(ctrl, "forwardedToAddress", tag, pos, seq_end,
		&activation_status_notification_div->forwarded_to));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_etsi_ServedUserNumber(ctrl, "servedUserNr", tag, pos,
		seq_end, &activation_status_notification_div->served_user_number));

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the DeactivationStatusNotificationDiv invoke argument parameters.
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
const unsigned char *rose_dec_etsi_DeactivationStatusNotificationDiv_ARG(struct pri
	*ctrl, unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	struct roseEtsiDeactivationStatusNotificationDiv_ARG
		*deactivation_status_notification_div;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  DeactivationStatusNotificationDiv %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	deactivation_status_notification_div = &args->etsi.DeactivationStatusNotificationDiv;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "procedure", tag, pos, seq_end, &value));
	deactivation_status_notification_div->procedure = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "basicService", tag, pos, seq_end, &value));
	deactivation_status_notification_div->basic_service = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_etsi_ServedUserNumber(ctrl, "forwardedToAddress", tag, pos,
		seq_end, &deactivation_status_notification_div->served_user_number));

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the InterrogationDiversion invoke argument parameters.
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
const unsigned char *rose_dec_etsi_InterrogationDiversion_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	struct roseEtsiInterrogationDiversion_ARG *interrogation_diversion;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  InterrogationDiversion %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	interrogation_diversion = &args->etsi.InterrogationDiversion;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "procedure", tag, pos, seq_end, &value));
	interrogation_diversion->procedure = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	if (tag == ASN1_TYPE_ENUMERATED) {
		ASN1_CALL(pos, asn1_dec_int(ctrl, "basicService", tag, pos, seq_end, &value));

		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	} else {
		value = 0;	/* DEFAULT BasicService value (allServices) */
	}
	interrogation_diversion->basic_service = value;

	ASN1_CALL(pos, rose_dec_etsi_ServedUserNumber(ctrl, "servedUserNr", tag, pos,
		seq_end, &interrogation_diversion->served_user_number));

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the InterrogationDiversion result parameters.
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
const unsigned char *rose_dec_etsi_InterrogationDiversion_RES(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args)
{
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SET);
	return rose_dec_etsi_IntResultList(ctrl, "diversionList", tag, pos, end,
		&args->etsi.InterrogationDiversion);
}

/*!
 * \brief Decode the DiversionInformation invoke argument parameters.
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
const unsigned char *rose_dec_etsi_DiversionInformation_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	struct roseEtsiDiversionInformation_ARG *diversion_information;
	int length;
	int seq_offset;
	int explicit_offset;
	const unsigned char *seq_end;
	const unsigned char *explicit_end;
	const unsigned char *save_pos;
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  DiversionInformation %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	diversion_information = &args->etsi.DiversionInformation;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "diversionReason", tag, pos, seq_end, &value));
	diversion_information->diversion_reason = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "basicService", tag, pos, seq_end, &value));
	diversion_information->basic_service = value;

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	diversion_information->served_user_subaddress.length = 0;
	diversion_information->calling_present = 0;
	diversion_information->original_called_present = 0;
	diversion_information->last_diverting_present = 0;
	diversion_information->last_diverting_reason_present = 0;
	diversion_information->q931ie.length = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag) {
		case ASN1_TAG_SEQUENCE:
		case ASN1_TYPE_OCTET_STRING:
		case ASN1_TYPE_OCTET_STRING | ASN1_PC_CONSTRUCTED:
			ASN1_CALL(pos, rose_dec_PartySubaddress(ctrl, "servedUserSubaddress", tag,
				pos, seq_end, &diversion_information->served_user_subaddress));
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 0:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_PresentedAddressScreened(ctrl, "callingAddress", tag,
				pos, explicit_end, &diversion_information->calling));
			diversion_information->calling_present = 1;

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 1:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_PresentedNumberUnscreened(ctrl, "originalCalledNr",
				tag, pos, explicit_end, &diversion_information->original_called));
			diversion_information->original_called_present = 1;

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
			ASN1_CALL(pos, rose_dec_PresentedNumberUnscreened(ctrl, "lastDivertingNr",
				tag, pos, explicit_end, &diversion_information->last_diverting));
			diversion_information->last_diverting_present = 1;

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
			ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
			ASN1_CALL(pos, asn1_dec_int(ctrl, "lastDivertingReason", tag, pos,
				explicit_end, &value));
			diversion_information->last_diverting_reason = value;
			diversion_information->last_diverting_reason_present = 1;

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_APPLICATION | 0:
		case ASN1_CLASS_APPLICATION | ASN1_PC_CONSTRUCTED | 0:
			ASN1_CALL(pos, rose_dec_Q931ie(ctrl, "userInfo", tag, pos, seq_end,
				&diversion_information->q931ie,
				sizeof(diversion_information->q931ie_contents)));
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
 * \brief Decode the CallDeflection invoke argument parameters.
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
const unsigned char *rose_dec_etsi_CallDeflection_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	struct roseEtsiCallDeflection_ARG *call_deflection;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  CallDeflection %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	call_deflection = &args->etsi.CallDeflection;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	ASN1_CALL(pos, rose_dec_Address(ctrl, "deflectionAddress", tag, pos, seq_end,
		&call_deflection->deflection));

	if (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_BOOLEAN);
		ASN1_CALL(pos, asn1_dec_boolean(ctrl, "presentationAllowedDivertedToUser", tag,
			pos, seq_end, &value));
		call_deflection->presentation_allowed_to_diverted_to_user = value;
		call_deflection->presentation_allowed_to_diverted_to_user_present = 1;
	} else {
		call_deflection->presentation_allowed_to_diverted_to_user_present = 0;
	}

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the CallRerouting invoke argument parameters.
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
const unsigned char *rose_dec_etsi_CallRerouting_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	struct roseEtsiCallRerouting_ARG *call_rerouting;
	int length;
	int seq_offset;
	int explicit_offset;
	const unsigned char *seq_end;
	const unsigned char *explicit_end;
	const unsigned char *save_pos;
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  CallRerouting %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	call_rerouting = &args->etsi.CallRerouting;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "reroutingReason", tag, pos, seq_end, &value));
	call_rerouting->rerouting_reason = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	ASN1_CALL(pos, rose_dec_Address(ctrl, "calledAddress", tag, pos, seq_end,
		&call_rerouting->called_address));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "reroutingCounter", tag, pos, seq_end, &value));
	call_rerouting->rerouting_counter = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag & ~ASN1_PC_MASK, ASN1_CLASS_APPLICATION | 0);
	ASN1_CALL(pos, rose_dec_Q931ie(ctrl, "q931ie", tag, pos, seq_end,
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

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	call_rerouting->subscription_option = 0;	/* DEFAULT value noNotification */
	call_rerouting->calling_subaddress.length = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag) {
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 2:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
			ASN1_CALL(pos, asn1_dec_int(ctrl, "subscriptionOption", tag, pos,
				explicit_end, &value));
			call_rerouting->subscription_option = value;

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
			ASN1_CALL(pos, rose_dec_PartySubaddress(ctrl, "callingPartySubaddress", tag,
				pos, explicit_end, &call_rerouting->calling_subaddress));

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
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
 * \brief Decode the InterrogateServedUserNumbers result parameters.
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
const unsigned char *rose_dec_etsi_InterrogateServedUserNumbers_RES(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args)
{
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SET);
	return rose_dec_etsi_ServedUserNumberList(ctrl, "interrogateServedUserNumbers", tag,
		pos, end, &args->etsi.InterrogateServedUserNumbers);
}

/*!
 * \brief Decode the DivertingLegInformation1 invoke argument parameters.
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
const unsigned char *rose_dec_etsi_DivertingLegInformation1_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	struct roseEtsiDivertingLegInformation1_ARG *diverting_leg_informtion_1;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  DivertingLegInformation1 %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	diverting_leg_informtion_1 = &args->etsi.DivertingLegInformation1;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "diversionReason", tag, pos, seq_end, &value));
	diverting_leg_informtion_1->diversion_reason = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "subscriptionOption", tag, pos, seq_end, &value));
	diverting_leg_informtion_1->subscription_option = value;

	if (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CALL(pos, rose_dec_PresentedNumberUnscreened(ctrl, "divertedToNumber", tag,
			pos, seq_end, &diverting_leg_informtion_1->diverted_to));
		diverting_leg_informtion_1->diverted_to_present = 1;
	} else {
		diverting_leg_informtion_1->diverted_to_present = 0;
	}

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the DivertingLegInformation2 invoke argument parameters.
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
const unsigned char *rose_dec_etsi_DivertingLegInformation2_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	struct roseEtsiDivertingLegInformation2_ARG *diverting_leg_information_2;
	int length;
	int seq_offset;
	int explicit_offset;
	const unsigned char *seq_end;
	const unsigned char *explicit_end;
	const unsigned char *save_pos;
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  DivertingLegInformation2 %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	diverting_leg_information_2 = &args->etsi.DivertingLegInformation2;

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
	diverting_leg_information_2->diverting_present = 0;
	diverting_leg_information_2->original_called_present = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag) {
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
 * \brief Decode the DivertingLegInformation3 invoke argument parameters.
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
const unsigned char *rose_dec_etsi_DivertingLegInformation3_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_BOOLEAN);
	ASN1_CALL(pos, asn1_dec_boolean(ctrl, "presentationAllowedIndicator", tag, pos, end,
		&value));
	args->etsi.DivertingLegInformation3.presentation_allowed_indicator = value;

	return pos;
}

/* ------------------------------------------------------------------- */
/* end rose_etsi_diversion.c */
