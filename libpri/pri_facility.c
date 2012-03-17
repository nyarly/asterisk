/*
 * libpri: An implementation of Primary Rate ISDN
 *
 * Written by Matthew Fredrickson <creslin@digium.com>
 *
 * Copyright (C) 2004-2005, Digium, Inc.
 * All Rights Reserved.
 */

/*
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

#include "compat.h"
#include "libpri.h"
#include "pri_internal.h"
#include "pri_facility.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

const char *pri_facility_error2str(int facility_error_code)
{
	return rose_error2str(facility_error_code);
}

const char *pri_facility_reject2str(int facility_reject_code)
{
	return rose_reject2str(facility_reject_code);
}

static int redirectingreason_from_q931(struct pri *ctrl, int redirectingreason)
{
	int value;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_QSIG:
		switch (redirectingreason) {
		case PRI_REDIR_UNKNOWN:
			value = QSIG_DIVERT_REASON_UNKNOWN;
			break;
		case PRI_REDIR_FORWARD_ON_BUSY:
			value = QSIG_DIVERT_REASON_CFB;
			break;
		case PRI_REDIR_FORWARD_ON_NO_REPLY:
			value = QSIG_DIVERT_REASON_CFNR;
			break;
		case PRI_REDIR_UNCONDITIONAL:
			value = QSIG_DIVERT_REASON_CFU;
			break;
		case PRI_REDIR_DEFLECTION:
		case PRI_REDIR_DTE_OUT_OF_ORDER:
		case PRI_REDIR_FORWARDED_BY_DTE:
			pri_message(ctrl,
				"!! Don't know how to convert Q.931 redirection reason %d to Q.SIG\n",
				redirectingreason);
			/* Fall through */
		default:
			value = QSIG_DIVERT_REASON_UNKNOWN;
			break;
		}
		break;
	default:
		switch (redirectingreason) {
		case PRI_REDIR_UNKNOWN:
			value = Q952_DIVERT_REASON_UNKNOWN;
			break;
		case PRI_REDIR_FORWARD_ON_BUSY:
			value = Q952_DIVERT_REASON_CFB;
			break;
		case PRI_REDIR_FORWARD_ON_NO_REPLY:
			value = Q952_DIVERT_REASON_CFNR;
			break;
		case PRI_REDIR_DEFLECTION:
			value = Q952_DIVERT_REASON_CD;
			break;
		case PRI_REDIR_UNCONDITIONAL:
			value = Q952_DIVERT_REASON_CFU;
			break;
		case PRI_REDIR_DTE_OUT_OF_ORDER:
		case PRI_REDIR_FORWARDED_BY_DTE:
			pri_message(ctrl,
				"!! Don't know how to convert Q.931 redirection reason %d to Q.952\n",
				redirectingreason);
			/* Fall through */
		default:
			value = Q952_DIVERT_REASON_UNKNOWN;
			break;
		}
		break;
	}

	return value;
}

static int redirectingreason_for_q931(struct pri *ctrl, int redirectingreason)
{
	int value;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_QSIG:
		switch (redirectingreason) {
		case QSIG_DIVERT_REASON_UNKNOWN:
			value = PRI_REDIR_UNKNOWN;
			break;
		case QSIG_DIVERT_REASON_CFU:
			value = PRI_REDIR_UNCONDITIONAL;
			break;
		case QSIG_DIVERT_REASON_CFB:
			value = PRI_REDIR_FORWARD_ON_BUSY;
			break;
		case QSIG_DIVERT_REASON_CFNR:
			value = PRI_REDIR_FORWARD_ON_NO_REPLY;
			break;
		default:
			pri_message(ctrl, "!! Unknown Q.SIG diversion reason %d\n",
				redirectingreason);
			value = PRI_REDIR_UNKNOWN;
			break;
		}
		break;
	default:
		switch (redirectingreason) {
		case Q952_DIVERT_REASON_UNKNOWN:
			value = PRI_REDIR_UNKNOWN;
			break;
		case Q952_DIVERT_REASON_CFU:
			value = PRI_REDIR_UNCONDITIONAL;
			break;
		case Q952_DIVERT_REASON_CFB:
			value = PRI_REDIR_FORWARD_ON_BUSY;
			break;
		case Q952_DIVERT_REASON_CFNR:
			value = PRI_REDIR_FORWARD_ON_NO_REPLY;
			break;
		case Q952_DIVERT_REASON_CD:
			value = PRI_REDIR_DEFLECTION;
			break;
		case Q952_DIVERT_REASON_IMMEDIATE:
			pri_message(ctrl,
				"!! Dont' know how to convert Q.952 diversion reason IMMEDIATE to PRI analog\n");
			value = PRI_REDIR_UNKNOWN;	/* ??? */
			break;
		default:
			pri_message(ctrl, "!! Unknown Q.952 diversion reason %d\n",
				redirectingreason);
			value = PRI_REDIR_UNKNOWN;
			break;
		}
		break;
	}

	return value;
}

/*!
 * \internal
 * \brief Convert the Q.931 type-of-number field to facility.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param ton Q.931 ton/plan octet.
 *
 * \return PartyNumber enumeration value.
 */
static int typeofnumber_from_q931(struct pri *ctrl, int ton)
{
	int value;

	switch ((ton >> 4) & 0x03) {
	default:
		pri_message(ctrl, "!! Unsupported Q.931 TypeOfNumber value (%d)\n", ton);
		/* fall through */
	case PRI_TON_UNKNOWN:
		value = Q932_TON_UNKNOWN;
		break;
	case PRI_TON_INTERNATIONAL:
		value = Q932_TON_INTERNATIONAL;
		break;
	case PRI_TON_NATIONAL:
		value = Q932_TON_NATIONAL;
		break;
	case PRI_TON_NET_SPECIFIC:
		value = Q932_TON_NET_SPECIFIC;
		break;
	case PRI_TON_SUBSCRIBER:
		value = Q932_TON_SUBSCRIBER;
		break;
	case PRI_TON_ABBREVIATED:
		value = Q932_TON_ABBREVIATED;
		break;
	}

	return value;
}

static int typeofnumber_for_q931(struct pri *ctrl, int ton)
{
	int value;

	switch (ton) {
	default:
		pri_message(ctrl, "!! Invalid TypeOfNumber %d\n", ton);
		/* fall through */
	case Q932_TON_UNKNOWN:
		value = PRI_TON_UNKNOWN;
		break;
	case Q932_TON_INTERNATIONAL:
		value = PRI_TON_INTERNATIONAL;
		break;
	case Q932_TON_NATIONAL:
		value = PRI_TON_NATIONAL;
		break;
	case Q932_TON_NET_SPECIFIC:
		value = PRI_TON_NET_SPECIFIC;
		break;
	case Q932_TON_SUBSCRIBER:
		value = PRI_TON_SUBSCRIBER;
		break;
	case Q932_TON_ABBREVIATED:
		value = PRI_TON_ABBREVIATED;
		break;
	}

	return value << 4;
}

/*!
 * \internal
 * \brief Convert the Q.931 numbering plan field to facility.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param plan Q.931 ton/plan octet.
 *
 * \return PartyNumber enumeration value.
 */
static int numbering_plan_from_q931(struct pri *ctrl, int plan)
{
	int value;

	switch (plan & 0x0F) {
	default:
		pri_message(ctrl, "!! Unsupported Q.931 numbering plan value (%d)\n", plan);
		/* fall through */
	case PRI_NPI_UNKNOWN:
		value = 0;	/* unknown */
		break;
	case PRI_NPI_E163_E164:
		value = 1;	/* public */
		break;
	case PRI_NPI_X121:
		value = 3;	/* data */
		break;
	case PRI_NPI_F69:
		value = 4;	/* telex */
		break;
	case PRI_NPI_NATIONAL:
		value = 8;	/* nationalStandard */
		break;
	case PRI_NPI_PRIVATE:
		value = 5;	/* private */
		break;
	}

	return value;
}

/*!
 * \internal
 * \brief Convert the PartyNumber numbering plan to Q.931 plan field value.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param plan PartyNumber enumeration value.
 *
 * \return Q.931 plan field value.
 */
static int numbering_plan_for_q931(struct pri *ctrl, int plan)
{
	int value;

	switch (plan) {
	default:
		pri_message(ctrl,
			"!! Unsupported PartyNumber to Q.931 numbering plan value (%d)\n", plan);
		/* fall through */
	case 0:	/* unknown */
		value = PRI_NPI_UNKNOWN;
		break;
	case 1:	/* public */
		value = PRI_NPI_E163_E164;
		break;
	case 3:	/* data */
		value = PRI_NPI_X121;
		break;
	case 4:	/* telex */
		value = PRI_NPI_F69;
		break;
	case 5:	/* private */
		value = PRI_NPI_PRIVATE;
		break;
	case 8:	/* nationalStandard */
		value = PRI_NPI_NATIONAL;
		break;
	}

	return value;
}

/*!
 * \internal
 * \brief Convert the Q.931 number presentation field to facility.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param presentation Q.931 presentation/screening octet.
 * \param number_present Non-zero if the number is available.
 *
 * \return Presented<Number/Address><Screened/Unscreened> enumeration value.
 */
static int presentation_from_q931(struct pri *ctrl, int presentation, int number_present)
{
	int value;

	switch (presentation & PRI_PRES_RESTRICTION) {
	case PRI_PRES_ALLOWED:
		value = 0;	/* presentationAllowed<Number/Address> */
		break;
	default:
		pri_message(ctrl, "!! Unsupported Q.931 number presentation value (%d)\n",
			presentation);
		/* fall through */
	case PRI_PRES_RESTRICTED:
		if (number_present) {
			value = 3;	/* presentationRestricted<Number/Address> */
		} else {
			value = 1;	/* presentationRestricted */
		}
		break;
	case PRI_PRES_UNAVAILABLE:
		value = 2;	/* numberNotAvailableDueToInterworking */
		break;
	}

	return value;
}

/*!
 * \internal
 * \brief Convert the Presented<Number/Address><Screened/Unscreened> presentation
 * to Q.931 presentation field value.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param presentation Presented<Number/Address><Screened/Unscreened> value.
 *
 * \return Q.931 presentation field value.
 */
static int presentation_for_q931(struct pri *ctrl, int presentation)
{
	int value;

	switch (presentation) {
	case 0:	/* presentationAllowed<Number/Address> */
		value = PRI_PRES_ALLOWED;
		break;
	default:
		pri_message(ctrl,
			"!! Unsupported Presented<Number/Address><Screened/Unscreened> to Q.931 value (%d)\n",
			presentation);
		/* fall through */
	case 1:	/* presentationRestricted */
	case 3:	/* presentationRestricted<Number/Address> */
		value = PRI_PRES_RESTRICTED;
		break;
	case 2:	/* numberNotAvailableDueToInterworking */
		value = PRI_PRES_UNAVAILABLE;
		break;
	}

	return value;
}

/*!
 * \internal
 * \brief Convert the Q.931 number presentation field to Q.SIG name presentation.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param presentation Q.931 presentation/screening octet.
 * \param name_present Non-zero if the name is available.
 *
 * \return Name presentation enumeration value.
 */
static int qsig_name_presentation_from_q931(struct pri *ctrl, int presentation, int name_present)
{
	int value;

	switch (presentation & PRI_PRES_RESTRICTION) {
	case PRI_PRES_ALLOWED:
		if (name_present) {
			value = 1;	/* presentation_allowed */
		} else {
			value = 4;	/* name_not_available */
		}
		break;
	default:
		pri_message(ctrl, "!! Unsupported Q.931 number presentation value (%d)\n",
			presentation);
		/* fall through */
	case PRI_PRES_RESTRICTED:
		if (name_present) {
			value = 2;	/* presentation_restricted */
		} else {
			value = 3;	/* presentation_restricted_null */
		}
		break;
	case PRI_PRES_UNAVAILABLE:
		value = 4;	/* name_not_available */
		break;
	}

	return value;
}

/*!
 * \internal
 * \brief Convert the Q.SIG name presentation to Q.931 presentation field value.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param presentation Q.SIG name presentation value.
 *
 * \return Q.931 presentation field value.
 */
static int qsig_name_presentation_for_q931(struct pri *ctrl, int presentation)
{
	int value;

	switch (presentation) {
	case 1:	/* presentation_allowed */
		value = PRI_PRES_ALLOWED;
		break;
	default:
		pri_message(ctrl,
			"!! Unsupported Q.SIG name presentation to Q.931 value (%d)\n",
			presentation);
		/* fall through */
	case 2:	/* presentation_restricted */
	case 3:	/* presentation_restricted_null */
		value = PRI_PRES_RESTRICTED;
		break;
	case 0:	/* optional_name_not_present */
	case 4:	/* name_not_available */
		value = PRI_PRES_UNAVAILABLE;
		break;
	}

	return value;
}

/*!
 * \internal
 * \brief Convert number presentation to Q.SIG diversion subscription notification.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param presentation Number presentation value.
 *
 * \return Q.SIG diversion subscription notification value.
 */
static int presentation_to_subscription(struct pri *ctrl, int presentation)
{
	/* derive subscription value from presentation value */

	switch (presentation & PRI_PRES_RESTRICTION) {
	case PRI_PRES_ALLOWED:
		return QSIG_NOTIFICATION_WITH_DIVERTED_TO_NR;
	case PRI_PRES_RESTRICTED:
		return QSIG_NOTIFICATION_WITHOUT_DIVERTED_TO_NR;
	case PRI_PRES_UNAVAILABLE:	/* Number not available due to interworking */
		return QSIG_NOTIFICATION_WITHOUT_DIVERTED_TO_NR;	/* ?? QSIG_NO_NOTIFICATION */
	default:
		pri_message(ctrl, "!! Unknown Q.SIG presentationIndicator 0x%02x\n",
			presentation);
		return QSIG_NOTIFICATION_WITHOUT_DIVERTED_TO_NR;
	}
}

/*!
 * \brief Copy the given rose party number to the q931_party_number
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param q931_number Q.931 party number structure
 * \param rose_number ROSE party number structure
 *
 * \note It is assumed that the q931_number has been initialized before calling.
 *
 * \return Nothing
 */
void rose_copy_number_to_q931(struct pri *ctrl, struct q931_party_number *q931_number,
	const struct rosePartyNumber *rose_number)
{
	//q931_party_number_init(q931_number);
	libpri_copy_string(q931_number->str, (char *) rose_number->str,
		sizeof(q931_number->str));
	q931_number->plan = numbering_plan_for_q931(ctrl, rose_number->plan)
		| typeofnumber_for_q931(ctrl, rose_number->ton);
	q931_number->valid = 1;
}

/*!
 * \brief Copy the given rose subaddress to the q931_party_subaddress.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param q931_subaddress Q.931 party subaddress structure
 * \param rose_subaddress ROSE subaddress structure
 *
 * \note It is assumed that the q931_subaddress has been initialized before calling.
 *
 * \return Nothing
 */
void rose_copy_subaddress_to_q931(struct pri *ctrl,
	struct q931_party_subaddress *q931_subaddress,
	const struct rosePartySubaddress *rose_subaddress)
{
	//q931_party_subaddress_init(q931_subaddress);
	if (!rose_subaddress->length) {
		/* Subaddress is not present. */
		return;
	}

	switch (rose_subaddress->type) {
	case 0:/* UserSpecified */
		q931_subaddress->type = 2;/* user_specified */
		q931_subaddress->valid = 1;
		q931_subaddress->length = rose_subaddress->length;
		if (sizeof(q931_subaddress->data) <= q931_subaddress->length) {
			q931_subaddress->length = sizeof(q931_subaddress->data) - 1;
		}
		memcpy(q931_subaddress->data, rose_subaddress->u.user_specified.information,
			q931_subaddress->length);
		q931_subaddress->data[q931_subaddress->length] = '\0';
		if (rose_subaddress->u.user_specified.odd_count_present) {
			q931_subaddress->odd_even_indicator =
				rose_subaddress->u.user_specified.odd_count;
		}
		break;
	case 1:/* NSAP */
		q931_subaddress->type = 0;/* nsap */
		q931_subaddress->valid = 1;
		libpri_copy_string((char *) q931_subaddress->data,
			(char *) rose_subaddress->u.nsap, sizeof(q931_subaddress->data));
		q931_subaddress->length = strlen((char *) q931_subaddress->data);
		break;
	default:
		/* Don't know how to encode so assume it is not present. */
		break;
	}
}

/*!
 * \brief Copy the given rose address to the q931_party_address.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param q931_address Q.931 party address structure
 * \param rose_address ROSE address structure
 *
 * \note It is assumed that the q931_address has been initialized before calling.
 *
 * \return Nothing
 */
void rose_copy_address_to_q931(struct pri *ctrl, struct q931_party_address *q931_address,
	const struct roseAddress *rose_address)
{
	rose_copy_number_to_q931(ctrl, &q931_address->number, &rose_address->number);
	rose_copy_subaddress_to_q931(ctrl, &q931_address->subaddress,
		&rose_address->subaddress);
}

/*!
 * \brief Copy the given rose address to the q931_party_id address.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param q931_address Q.931 party id structure to fill address
 * \param rose_address ROSE address structure
 *
 * \note It is assumed that the q931_address has been initialized before calling.
 *
 * \return Nothing
 */
void rose_copy_address_to_id_q931(struct pri *ctrl, struct q931_party_id *q931_address,
	const struct roseAddress *rose_address)
{
	rose_copy_number_to_q931(ctrl, &q931_address->number, &rose_address->number);
	rose_copy_subaddress_to_q931(ctrl, &q931_address->subaddress,
		&rose_address->subaddress);
}

/*!
 * \brief Copy the given rose presented screened party number to the q931_party_number
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param q931_number Q.931 party number structure
 * \param rose_presented ROSE presented screened party number structure
 *
 * \return Nothing
 */
void rose_copy_presented_number_screened_to_q931(struct pri *ctrl,
	struct q931_party_number *q931_number,
	const struct rosePresentedNumberScreened *rose_presented)
{
	q931_party_number_init(q931_number);
	q931_number->valid = 1;
	q931_number->presentation = presentation_for_q931(ctrl, rose_presented->presentation);
	switch (rose_presented->presentation) {
	case 0:	/* presentationAllowedNumber */
	case 3:	/* presentationRestrictedNumber */
		q931_number->presentation |=
			(rose_presented->screened.screening_indicator & PRI_PRES_NUMBER_TYPE);
		rose_copy_number_to_q931(ctrl, q931_number,
			&rose_presented->screened.number);
		break;
	default:
		q931_number->presentation |= PRI_PRES_USER_NUMBER_UNSCREENED;
		break;
	}
}

/*!
 * \brief Copy the given rose presented unscreened party number to the q931_party_number
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param q931_number Q.931 party number structure
 * \param rose_presented ROSE presented unscreened party number structure
 *
 * \return Nothing
 */
void rose_copy_presented_number_unscreened_to_q931(struct pri *ctrl,
	struct q931_party_number *q931_number,
	const struct rosePresentedNumberUnscreened *rose_presented)
{
	q931_party_number_init(q931_number);
	q931_number->valid = 1;
	q931_number->presentation = presentation_for_q931(ctrl,
		rose_presented->presentation) | PRI_PRES_USER_NUMBER_UNSCREENED;
	switch (rose_presented->presentation) {
	case 0:	/* presentationAllowedNumber */
	case 3:	/* presentationRestrictedNumber */
		rose_copy_number_to_q931(ctrl, q931_number, &rose_presented->number);
		break;
	default:
		break;
	}
}

/*!
 * \brief Copy the given rose presented screened party address to the q931_party_number
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param q931_address Q.931 party id structure to fill the address
 * \param rose_presented ROSE presented screened party address structure
 *
 * \return Nothing
 */
void rose_copy_presented_address_screened_to_id_q931(struct pri *ctrl,
	struct q931_party_id *q931_address,
	const struct rosePresentedAddressScreened *rose_presented)
{
	q931_party_number_init(&q931_address->number);
	q931_party_subaddress_init(&q931_address->subaddress);
	q931_address->number.valid = 1;
	q931_address->number.presentation = presentation_for_q931(ctrl,
		rose_presented->presentation);
	switch (rose_presented->presentation) {
	case 0:	/* presentationAllowedAddress */
	case 3:	/* presentationRestrictedAddress */
		q931_address->number.presentation |=
			(rose_presented->screened.screening_indicator & PRI_PRES_NUMBER_TYPE);
		rose_copy_number_to_q931(ctrl, &q931_address->number,
			&rose_presented->screened.number);
		rose_copy_subaddress_to_q931(ctrl, &q931_address->subaddress,
			&rose_presented->screened.subaddress);
		break;
	default:
		q931_address->number.presentation |= PRI_PRES_USER_NUMBER_UNSCREENED;
		break;
	}
}

/*!
 * \brief Copy the given rose party name to the q931_party_name
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param qsig_name Q.SIG party name structure
 * \param rose_name Q.SIG ROSE party name structure
 *
 * \return Nothing
 */
void rose_copy_name_to_q931(struct pri *ctrl, struct q931_party_name *qsig_name,
	const struct roseQsigName *rose_name)
{
	//q931_party_name_init(qsig_name);
	qsig_name->valid = 1;
	qsig_name->presentation = qsig_name_presentation_for_q931(ctrl,
		rose_name->presentation);
	qsig_name->char_set = rose_name->char_set;
	libpri_copy_string(qsig_name->str, (char *) rose_name->data, sizeof(qsig_name->str));
}

/*!
 * \brief Copy the given q931_party_number to the rose party number
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param rose_number ROSE party number structure
 * \param q931_number Q.931 party number structure
 *
 * \return Nothing
 */
void q931_copy_number_to_rose(struct pri *ctrl, struct rosePartyNumber *rose_number,
	const struct q931_party_number *q931_number)
{
	rose_number->plan = numbering_plan_from_q931(ctrl, q931_number->plan);
	rose_number->ton = typeofnumber_from_q931(ctrl, q931_number->plan);
	/* Truncate the q931_number->str if necessary. */
	libpri_copy_string((char *) rose_number->str, q931_number->str,
		sizeof(rose_number->str));
	rose_number->length = strlen((char *) rose_number->str);
}

/*!
 * \brief Copy the given q931_party_subaddress to the rose subaddress.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param rose_subaddress ROSE subaddress structure
 * \param q931_subaddress Q.931 party subaddress structure
 *
 * \return Nothing
 */
void q931_copy_subaddress_to_rose(struct pri *ctrl,
	struct rosePartySubaddress *rose_subaddress,
	const struct q931_party_subaddress *q931_subaddress)
{
	if (!q931_subaddress->valid) {
		/* Subaddress is not present. */
		rose_subaddress->length = 0;
		return;
	}

	switch (q931_subaddress->type) {
	case 0:	/* NSAP */
		rose_subaddress->type = 1;/* NSAP */
		libpri_copy_string((char *) rose_subaddress->u.nsap,
			(char *) q931_subaddress->data, sizeof(rose_subaddress->u.nsap));
		rose_subaddress->length = strlen((char *) rose_subaddress->u.nsap);
		break;
	case 2:	/* user_specified */
		rose_subaddress->type = 0;/* UserSpecified */
		rose_subaddress->length = q931_subaddress->length;
		if (sizeof(rose_subaddress->u.user_specified.information)
			<= rose_subaddress->length) {
			rose_subaddress->length =
				sizeof(rose_subaddress->u.user_specified.information) - 1;
		} else {
			if (q931_subaddress->odd_even_indicator) {
				rose_subaddress->u.user_specified.odd_count_present = 1;
				rose_subaddress->u.user_specified.odd_count = 1;
			}
		}
		memcpy(rose_subaddress->u.user_specified.information, q931_subaddress->data,
			rose_subaddress->length);
		rose_subaddress->u.user_specified.information[rose_subaddress->length] = '\0';
		break;
	default:
		/* Don't know how to encode so assume it is not present. */
		rose_subaddress->length = 0;
		break;
	}
}

/*!
 * \brief Copy the given q931_party_address to the rose address.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param rose_address ROSE address structure
 * \param q931_address Q.931 party address structure
 *
 * \return Nothing
 */
void q931_copy_address_to_rose(struct pri *ctrl, struct roseAddress *rose_address,
	const struct q931_party_address *q931_address)
{
	q931_copy_number_to_rose(ctrl, &rose_address->number, &q931_address->number);
	q931_copy_subaddress_to_rose(ctrl, &rose_address->subaddress,
		&q931_address->subaddress);
}

/*!
 * \brief Copy the given q931_party_id address to the rose address.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param rose_address ROSE address structure
 * \param q931_address Q.931 party id structure to give address
 *
 * \return Nothing
 */
void q931_copy_id_address_to_rose(struct pri *ctrl, struct roseAddress *rose_address,
	const struct q931_party_id *q931_address)
{
	q931_copy_number_to_rose(ctrl, &rose_address->number, &q931_address->number);
	q931_copy_subaddress_to_rose(ctrl, &rose_address->subaddress,
		&q931_address->subaddress);
}

/*!
 * \brief Copy the given q931_party_number to the rose presented screened party number
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param rose_presented ROSE presented screened party number structure
 * \param q931_number Q.931 party number structure
 *
 * \return Nothing
 */
void q931_copy_presented_number_screened_to_rose(struct pri *ctrl,
	struct rosePresentedNumberScreened *rose_presented,
	const struct q931_party_number *q931_number)
{
	if (q931_number->valid) {
		rose_presented->presentation =
			presentation_from_q931(ctrl, q931_number->presentation, q931_number->str[0]);
		rose_presented->screened.screening_indicator =
			q931_number->presentation & PRI_PRES_NUMBER_TYPE;
		q931_copy_number_to_rose(ctrl, &rose_presented->screened.number, q931_number);
	} else {
		rose_presented->presentation = 2;/* numberNotAvailableDueToInterworking */
	}
}

/*!
 * \brief Copy the given q931_party_number to the rose presented unscreened party number
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param rose_presented ROSE presented unscreened party number structure
 * \param q931_number Q.931 party number structure
 *
 * \return Nothing
 */
void q931_copy_presented_number_unscreened_to_rose(struct pri *ctrl,
	struct rosePresentedNumberUnscreened *rose_presented,
	const struct q931_party_number *q931_number)
{
	if (q931_number->valid) {
		rose_presented->presentation =
			presentation_from_q931(ctrl, q931_number->presentation, q931_number->str[0]);
		q931_copy_number_to_rose(ctrl, &rose_presented->number, q931_number);
	} else {
		rose_presented->presentation = 2;/* numberNotAvailableDueToInterworking */
	}
}

/*!
 * \brief Copy the given q931_party_number to the rose presented screened party address
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param rose_presented ROSE presented screened party address structure
 * \param q931_address Q.931 party id structure to get the address
 *
 * \return Nothing
 */
void q931_copy_presented_id_address_screened_to_rose(struct pri *ctrl,
	struct rosePresentedAddressScreened *rose_presented,
	const struct q931_party_id *q931_address)
{
	if (q931_address->number.valid) {
		rose_presented->presentation =
			presentation_from_q931(ctrl, q931_address->number.presentation,
				q931_address->number.str[0]);
		rose_presented->screened.screening_indicator =
			q931_address->number.presentation & PRI_PRES_NUMBER_TYPE;
		q931_copy_number_to_rose(ctrl, &rose_presented->screened.number,
			&q931_address->number);
		q931_copy_subaddress_to_rose(ctrl, &rose_presented->screened.subaddress,
			&q931_address->subaddress);
	} else {
		rose_presented->presentation = 2;/* numberNotAvailableDueToInterworking */
	}
}

/*!
 * \brief Copy the given q931_party_name to the rose party name
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param rose_name Q.SIG ROSE party name structure
 * \param qsig_name Q.SIG party name structure
 *
 * \return Nothing
 */
void q931_copy_name_to_rose(struct pri *ctrl, struct roseQsigName *rose_name,
	const struct q931_party_name *qsig_name)
{
	if (qsig_name->valid) {
		rose_name->presentation = qsig_name_presentation_from_q931(ctrl,
			qsig_name->presentation, qsig_name->str[0]);
		rose_name->char_set = qsig_name->char_set;
		/* Truncate the qsig_name->str if necessary. */
		libpri_copy_string((char *) rose_name->data, qsig_name->str, sizeof(rose_name->data));
		rose_name->length = strlen((char *) rose_name->data);
	} else {
		rose_name->presentation = 4;/* name_not_available */
	}
}

/*!
 * \internal
 * \brief Encode the Q.SIG DivertingLegInformation1 invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param call Call leg from which to encode diversion leg 1.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_qsig_diverting_leg_information1(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, q931_call *call)
{
	struct fac_extension_header header;
	struct rose_msg_invoke msg;

	memset(&header, 0, sizeof(header));
	header.nfe_present = 1;
	header.nfe.source_entity = 0;	/* endPINX */
	header.nfe.destination_entity = 0;	/* endPINX */
	header.interpretation_present = 1;
	header.interpretation = 0;	/* discardAnyUnrecognisedInvokePdu */
	pos = facility_encode_header(ctrl, pos, end, &header);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_QSIG_DivertingLegInformation1;
	msg.invoke_id = get_invokeid(ctrl);
	msg.args.qsig.DivertingLegInformation1.diversion_reason =
		redirectingreason_from_q931(ctrl, call->redirecting.reason);

	/* subscriptionOption is the redirecting.to.number.presentation */
	msg.args.qsig.DivertingLegInformation1.subscription_option =
		presentation_to_subscription(ctrl, call->redirecting.to.number.presentation);

	/* nominatedNr is the redirecting.to.number */
	q931_copy_number_to_rose(ctrl,
		&msg.args.qsig.DivertingLegInformation1.nominated_number,
		&call->redirecting.to.number);

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode the ETSI DivertingLegInformation1 invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param call Call leg from which to encode diversion leg 1.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_diverting_leg_information1(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, q931_call *call)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_ETSI_DivertingLegInformation1;
	msg.invoke_id = get_invokeid(ctrl);
	msg.args.etsi.DivertingLegInformation1.diversion_reason =
		redirectingreason_from_q931(ctrl, call->redirecting.reason);

	if (call->redirecting.to.number.valid) {
		msg.args.etsi.DivertingLegInformation1.subscription_option = 2;

		/* divertedToNumber is the redirecting.to.number */
		msg.args.etsi.DivertingLegInformation1.diverted_to_present = 1;
		q931_copy_presented_number_unscreened_to_rose(ctrl,
			&msg.args.etsi.DivertingLegInformation1.diverted_to,
			&call->redirecting.to.number);
	} else {
		msg.args.etsi.DivertingLegInformation1.subscription_option = 1;
	}
	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \brief Encode and queue the DivertingLegInformation1 invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode diversion leg 1.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int rose_diverting_leg_information1_encode(struct pri *ctrl, q931_call *call)
{
	unsigned char buffer[256];
	unsigned char *end;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		end = enc_etsi_diverting_leg_information1(ctrl, buffer, buffer + sizeof(buffer),
			call);
		break;
	case PRI_SWITCH_QSIG:
		end = enc_qsig_diverting_leg_information1(ctrl, buffer, buffer + sizeof(buffer),
			call);
		break;
	default:
		return -1;
	}
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL);
}

/*!
 * \internal
 * \brief Encode the Q.SIG DivertingLegInformation2 invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param call Call leg from which to encode diversion leg 2.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_qsig_diverting_leg_information2(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, q931_call *call)
{
	struct fac_extension_header header;
	struct rose_msg_invoke msg;

	memset(&header, 0, sizeof(header));
	header.nfe_present = 1;
	header.nfe.source_entity = 0;	/* endPINX */
	header.nfe.destination_entity = 0;	/* endPINX */
	header.interpretation_present = 1;
	header.interpretation = 0;	/* discardAnyUnrecognisedInvokePdu */
	pos = facility_encode_header(ctrl, pos, end, &header);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_QSIG_DivertingLegInformation2;
	msg.invoke_id = get_invokeid(ctrl);

	/* diversionCounter is the redirecting.count */
	msg.args.qsig.DivertingLegInformation2.diversion_counter = call->redirecting.count;

	msg.args.qsig.DivertingLegInformation2.diversion_reason =
		redirectingreason_from_q931(ctrl, call->redirecting.reason);

	/* divertingNr is the redirecting.from.number */
	msg.args.qsig.DivertingLegInformation2.diverting_present = 1;
	q931_copy_presented_number_unscreened_to_rose(ctrl,
		&msg.args.qsig.DivertingLegInformation2.diverting,
		&call->redirecting.from.number);

	/* redirectingName is the redirecting.from.name */
	if (call->redirecting.from.name.valid) {
		msg.args.qsig.DivertingLegInformation2.redirecting_name_present = 1;
		q931_copy_name_to_rose(ctrl,
			&msg.args.qsig.DivertingLegInformation2.redirecting_name,
			&call->redirecting.from.name);
	}

	if (1 < call->redirecting.count) {
		/* originalCalledNr is the redirecting.orig_called.number */
		msg.args.qsig.DivertingLegInformation2.original_called_present = 1;
		q931_copy_presented_number_unscreened_to_rose(ctrl,
			&msg.args.qsig.DivertingLegInformation2.original_called,
			&call->redirecting.orig_called.number);

		msg.args.qsig.DivertingLegInformation2.original_diversion_reason_present = 1;
		if (call->redirecting.orig_called.number.valid) {
			msg.args.qsig.DivertingLegInformation2.original_diversion_reason =
				redirectingreason_from_q931(ctrl, call->redirecting.orig_reason);
		} else {
			msg.args.qsig.DivertingLegInformation2.original_diversion_reason =
				QSIG_DIVERT_REASON_UNKNOWN;
		}

		/* originalCalledName is the redirecting.orig_called.name */
		if (call->redirecting.orig_called.name.valid) {
			msg.args.qsig.DivertingLegInformation2.original_called_name_present = 1;
			q931_copy_name_to_rose(ctrl,
				&msg.args.qsig.DivertingLegInformation2.original_called_name,
				&call->redirecting.orig_called.name);
		}
	}

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode the ETSI DivertingLegInformation2 invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param call Call leg from which to encode diversion leg 2.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_diverting_leg_information2(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, q931_call *call)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_ETSI_DivertingLegInformation2;
	msg.invoke_id = get_invokeid(ctrl);

	/* diversionCounter is the redirecting.count */
	msg.args.etsi.DivertingLegInformation2.diversion_counter = call->redirecting.count;

	msg.args.etsi.DivertingLegInformation2.diversion_reason =
		redirectingreason_from_q931(ctrl, call->redirecting.reason);

	/* divertingNr is the redirecting.from.number */
	msg.args.etsi.DivertingLegInformation2.diverting_present = 1;
	q931_copy_presented_number_unscreened_to_rose(ctrl,
		&msg.args.etsi.DivertingLegInformation2.diverting,
		&call->redirecting.from.number);

	if (1 < call->redirecting.count) {
		/* originalCalledNr is the redirecting.orig_called.number */
		msg.args.etsi.DivertingLegInformation2.original_called_present = 1;
		q931_copy_presented_number_unscreened_to_rose(ctrl,
			&msg.args.etsi.DivertingLegInformation2.original_called,
			&call->redirecting.orig_called.number);
	}

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode and queue the DivertingLegInformation2 invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode diversion leg 2.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_diverting_leg_information2_encode(struct pri *ctrl, q931_call *call)
{
	unsigned char buffer[256];
	unsigned char *end;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		end = enc_etsi_diverting_leg_information2(ctrl, buffer, buffer + sizeof(buffer),
			call);
		break;
	case PRI_SWITCH_QSIG:
		end = enc_qsig_diverting_leg_information2(ctrl, buffer, buffer + sizeof(buffer),
			call);
		break;
	default:
		return -1;
	}
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, Q931_SETUP, buffer, end - buffer, NULL);
}

/*!
 * \internal
 * \brief Encode the Q.SIG DivertingLegInformation3 invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param call Call leg from which to encode diversion leg 3.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_qsig_diverting_leg_information3(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, q931_call *call)
{
	struct fac_extension_header header;
	struct rose_msg_invoke msg;

	memset(&header, 0, sizeof(header));
	header.nfe_present = 1;
	header.nfe.source_entity = 0;	/* endPINX */
	header.nfe.destination_entity = 0;	/* endPINX */
	header.interpretation_present = 1;
	header.interpretation = 0;	/* discardAnyUnrecognisedInvokePdu */
	pos = facility_encode_header(ctrl, pos, end, &header);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_QSIG_DivertingLegInformation3;
	msg.invoke_id = get_invokeid(ctrl);

	/* redirecting.to.number.presentation also indicates if name presentation is allowed */
	if ((call->redirecting.to.number.presentation & PRI_PRES_RESTRICTION) == PRI_PRES_ALLOWED) {
		msg.args.qsig.DivertingLegInformation3.presentation_allowed_indicator = 1;	/* TRUE */

		/* redirectionName is the redirecting.to.name */
		if (call->redirecting.to.name.valid) {
			msg.args.qsig.DivertingLegInformation3.redirection_name_present = 1;
			q931_copy_name_to_rose(ctrl,
				&msg.args.qsig.DivertingLegInformation3.redirection_name,
				&call->redirecting.to.name);
		}
	}

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode the ETSI DivertingLegInformation3 invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param call Call leg from which to encode diversion leg 3.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_diverting_leg_information3(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, q931_call *call)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_ETSI_DivertingLegInformation3;
	msg.invoke_id = get_invokeid(ctrl);

	if ((call->redirecting.to.number.presentation & PRI_PRES_RESTRICTION) == PRI_PRES_ALLOWED) {
		msg.args.etsi.DivertingLegInformation3.presentation_allowed_indicator = 1;	/* TRUE */
	}

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \brief Encode and queue the DivertingLegInformation3 invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode diversion leg 3.
 * \param messagetype Q.931 message type to add facility ie to.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int rose_diverting_leg_information3_encode(struct pri *ctrl, q931_call *call,
	int messagetype)
{
	unsigned char buffer[256];
	unsigned char *end;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		end = enc_etsi_diverting_leg_information3(ctrl, buffer, buffer + sizeof(buffer),
			call);
		break;
	case PRI_SWITCH_QSIG:
		end = enc_qsig_diverting_leg_information3(ctrl, buffer, buffer + sizeof(buffer),
			call);
		break;
	default:
		return -1;
	}
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, messagetype, buffer, end - buffer, NULL);
}

/*!
 * \internal
 * \brief Encode the rltThirdParty invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param callwithid Call-ID information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_dms100_rlt_initiate_transfer(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const q931_call *callwithid)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_DMS100_RLT_ThirdParty;
	msg.invoke_id = ROSE_DMS100_RLT_THIRD_PARTY;
	msg.args.dms100.RLT_ThirdParty.call_id = callwithid->rlt_call_id & 0xFFFFFF;
	msg.args.dms100.RLT_ThirdParty.reason = 0;	/* unused, set to 129 */
	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \brief Send the rltThirdParty: Invoke.
 *
 * \note For PRI_SWITCH_DMS100 only.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param c1 Q.931 call leg 1
 * \param c2 Q.931 call leg 2
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int rlt_initiate_transfer(struct pri *ctrl, q931_call *c1, q931_call *c2)
{
	unsigned char buffer[256];
	unsigned char *end;
	q931_call *apdubearer;
	q931_call *callwithid;

	if (c2->transferable) {
		apdubearer = c1;
		callwithid = c2;
	} else if (c1->transferable) {
		apdubearer = c2;
		callwithid = c1;
	} else {
		return -1;
	}

	end =
		enc_dms100_rlt_initiate_transfer(ctrl, buffer, buffer + sizeof(buffer),
		callwithid);
	if (!end) {
		return -1;
	}

	if (pri_call_apdu_queue(apdubearer, Q931_FACILITY, buffer, end - buffer, NULL)) {
		return -1;
	}

	if (q931_facility(apdubearer->pri, apdubearer)) {
		pri_message(ctrl, "Could not schedule facility message for call %d\n",
			apdubearer->cr);
		return -1;
	}
	return 0;
}

/*!
 * \internal
 * \brief Encode the rltOperationInd invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_dms100_rlt_transfer_ability(struct pri *ctrl,
	unsigned char *pos, unsigned char *end)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_DMS100_RLT_OperationInd;
	msg.invoke_id = ROSE_DMS100_RLT_OPERATION_IND;
	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Send the rltOperationInd: Invoke.
 *
 * \note For PRI_SWITCH_DMS100 only.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Q.931 call leg
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int add_dms100_transfer_ability_apdu(struct pri *ctrl, q931_call *call)
{
	unsigned char buffer[256];
	unsigned char *end;

	end = enc_dms100_rlt_transfer_ability(ctrl, buffer, buffer + sizeof(buffer));
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, Q931_SETUP, buffer, end - buffer, NULL);
}

/*!
 * \internal
 * \brief Encode the NI2 InformationFollowing invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_ni2_information_following(struct pri *ctrl, unsigned char *pos,
	unsigned char *end)
{
	struct fac_extension_header header;
	struct rose_msg_invoke msg;

	memset(&header, 0, sizeof(header));
	header.interpretation_present = 1;
	header.interpretation = 0;	/* discardAnyUnrecognisedInvokePdu */
	pos = facility_encode_header(ctrl, pos, end, &header);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_NI2_InformationFollowing;
	msg.invoke_id = get_invokeid(ctrl);
	msg.args.ni2.InformationFollowing.value = 0;
	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode the Q.SIG CallingName invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param name Name data which to encode name.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_qsig_calling_name(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct q931_party_name *name)
{
	struct fac_extension_header header;
	struct rose_msg_invoke msg;

	memset(&header, 0, sizeof(header));
	if (ctrl->switchtype == PRI_SWITCH_QSIG) {
		header.nfe_present = 1;
		header.nfe.source_entity = 0;	/* endPINX */
		header.nfe.destination_entity = 0;	/* endPINX */
	}
	header.interpretation_present = 1;
	header.interpretation = 0;	/* discardAnyUnrecognisedInvokePdu */
	pos = facility_encode_header(ctrl, pos, end, &header);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_QSIG_CallingName;
	msg.invoke_id = get_invokeid(ctrl);

	/* CallingName */
	q931_copy_name_to_rose(ctrl, &msg.args.qsig.CallingName.name, name);

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Send caller name information.
 *
 * \note For PRI_SWITCH_NI2 and PRI_SWITCH_QSIG.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode name.
 * \param cpe TRUE if we are the CPE side.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int add_callername_facility_ies(struct pri *ctrl, q931_call *call, int cpe)
{
	unsigned char buffer[256];
	unsigned char *end;
	int mymessage;

	if (!call->local_id.name.valid) {
		return 0;
	}

	if (ctrl->switchtype == PRI_SWITCH_NI2 && !cpe) {
		end = enc_ni2_information_following(ctrl, buffer, buffer + sizeof(buffer));
		if (!end) {
			return -1;
		}

		if (pri_call_apdu_queue(call, Q931_SETUP, buffer, end - buffer, NULL)) {
			return -1;
		}

		/*
		 * We can reuse the buffer since the queue function doesn't
		 * need it.
		 */
	}

	/* CallingName is the local_id.name */
	end = enc_qsig_calling_name(ctrl, buffer, buffer + sizeof(buffer),
		&call->local_id.name);
	if (!end) {
		return -1;
	}

	if (cpe) {
		mymessage = Q931_SETUP;
	} else {
		mymessage = Q931_FACILITY;
	}

	return pri_call_apdu_queue(call, mymessage, buffer, end - buffer, NULL);
}
/* End Callername */

/* MWI related encode and decode functions */

/*!
 * \internal
 * \brief Encode the Q.SIG MWIActivate invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param req Served user setup request information.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_qsig_mwi_activate_message(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, struct pri_sr *req)
{
	struct fac_extension_header header;
	struct rose_msg_invoke msg;

	memset(&header, 0, sizeof(header));
	header.nfe_present = 1;
	header.nfe.source_entity = 0;	/* endPINX */
	header.nfe.destination_entity = 0;	/* endPINX */
	header.interpretation_present = 1;
	header.interpretation = 0;	/* discardAnyUnrecognisedInvokePdu */
	pos = facility_encode_header(ctrl, pos, end, &header);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_QSIG_MWIActivate;
	msg.invoke_id = get_invokeid(ctrl);

	/* The called.number is the served user */
	q931_copy_number_to_rose(ctrl, &msg.args.qsig.MWIActivate.served_user_number,
		&req->called.number);
	/*
	 * For now, we will just force the numbering plan to unknown to preserve
	 * the original behaviour.
	 */
	msg.args.qsig.MWIActivate.served_user_number.plan = 0;	/* unknown */

	msg.args.qsig.MWIActivate.basic_service = 1;	/* speech */

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode the Q.SIG MWIDeactivate invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param req Served user setup request information.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_qsig_mwi_deactivate_message(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, struct pri_sr *req)
{
	struct fac_extension_header header;
	struct rose_msg_invoke msg;

	memset(&header, 0, sizeof(header));
	header.nfe_present = 1;
	header.nfe.source_entity = 0;	/* endPINX */
	header.nfe.destination_entity = 0;	/* endPINX */
	header.interpretation_present = 1;
	header.interpretation = 0;	/* discardAnyUnrecognisedInvokePdu */
	pos = facility_encode_header(ctrl, pos, end, &header);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_QSIG_MWIDeactivate;
	msg.invoke_id = get_invokeid(ctrl);

	/* The called.number is the served user */
	q931_copy_number_to_rose(ctrl, &msg.args.qsig.MWIDeactivate.served_user_number,
		&req->called.number);
	/*
	 * For now, we will just force the numbering plan to unknown to preserve
	 * the original behaviour.
	 */
	msg.args.qsig.MWIDeactivate.served_user_number.plan = 0;	/* unknown */

	msg.args.qsig.MWIDeactivate.basic_service = 1;	/* speech */

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \brief Encode and queue the Q.SIG MWIActivate/MWIDeactivate invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg to queue message.
 * \param req Served user setup request information.
 * \param activate Nonzero to do the activate message.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int mwi_message_send(struct pri *ctrl, q931_call *call, struct pri_sr *req, int activate)
{
	unsigned char buffer[255];
	unsigned char *end;

	if (!req->called.number.valid || !req->called.number.str[0]) {
		return -1;
	}

	if (activate) {
		end = enc_qsig_mwi_activate_message(ctrl, buffer, buffer + sizeof(buffer), req);
	} else {
		end =
			enc_qsig_mwi_deactivate_message(ctrl, buffer, buffer + sizeof(buffer), req);
	}
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, Q931_SETUP, buffer, end - buffer, NULL);
}

/*!
 * \internal
 * \brief Encode a MWI indication.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param vm_id Controlling party number (NULL if not present).
 * \param basic_service Basic service enum (-1 if not present).
 * \param num_messages NumberOfMessages (-1 if not present).
 * \param caller_id Controlling party privided number (NULL if not present).
 * \param timestamp When message left. (Generalized Time format, NULL if not present)
 * \param message_reference Message reference number (-1 if not present).
 * \param message_status Message status: added(0), removed(1).
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_mwi_indicate_message(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct pri_party_id *vm_id, int basic_service,
	int num_messages, const struct pri_party_id *caller_id, const char *timestamp,
	int message_reference, int message_status)
{
	struct rose_msg_invoke msg;
	struct q931_party_number number;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_ETSI_MWIIndicate;
	msg.invoke_id = get_invokeid(ctrl);

	if (vm_id && vm_id->number.valid) {
		pri_copy_party_number_to_q931(&number, &vm_id->number);
		q931_copy_number_to_rose(ctrl, &msg.args.etsi.MWIIndicate.controlling_user_number,
			&number);
	}
	if (-1 < basic_service) {
		msg.args.etsi.MWIIndicate.basic_service_present = 1;
		msg.args.etsi.MWIIndicate.basic_service = basic_service;
	}
	if (-1 < num_messages) {
		msg.args.etsi.MWIIndicate.number_of_messages_present = 1;
		msg.args.etsi.MWIIndicate.number_of_messages = num_messages;
	}
	if (caller_id && caller_id->number.valid) {
		pri_copy_party_number_to_q931(&number, &caller_id->number);
		q931_copy_number_to_rose(ctrl,
			&msg.args.etsi.MWIIndicate.controlling_user_provided_number, &number);
	}
	if (timestamp && timestamp[0]) {
		msg.args.etsi.MWIIndicate.time_present = 1;
		libpri_copy_string((char *) msg.args.etsi.MWIIndicate.time.str, timestamp,
			sizeof(msg.args.etsi.MWIIndicate.time.str));
	}
	if (-1 < message_reference) {
		msg.args.etsi.MWIIndicate.message_id_present = 1;
		msg.args.etsi.MWIIndicate.message_id.reference_number = message_reference;
		msg.args.etsi.MWIIndicate.message_id.status = message_status;
	}

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode and queue a MWI indication.
 *
 * \param ctrl D channel controller.
 * \param call Call leg to queue message.
 * \param vm_id Voicemail system number (NULL if not present).
 * \param basic_service Basic service enum (-1 if not present).
 * \param num_messages NumberOfMessages (-1 if not present).
 * \param caller_id Party leaving message (NULL if not present).
 * \param timestamp When message left. (Generalized Time format, NULL if not present)
 * \param message_reference Message reference number (-1 if not present).
 * \param message_status Message status: added(0), removed(1).
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_mwi_indicate_encode(struct pri *ctrl, struct q931_call *call,
	const struct pri_party_id *vm_id, int basic_service, int num_messages,
	const struct pri_party_id *caller_id, const char *timestamp, int message_reference,
	int message_status)
{
	unsigned char buffer[255];
	unsigned char *end;

	end = enc_etsi_mwi_indicate_message(ctrl, buffer, buffer + sizeof(buffer), vm_id,
		basic_service, num_messages, caller_id, timestamp, message_reference,
		message_status);
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL);
}

int pri_mwi_indicate_v2(struct pri *ctrl, const struct pri_party_id *mailbox,
	const struct pri_party_id *vm_id, int basic_service, int num_messages,
	const struct pri_party_id *caller_id, const char *timestamp, int message_reference,
	int message_status)
{
	struct q931_call *call;
	struct q931_party_id called;

	if (!ctrl) {
		return -1;
	}

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		if (!BRI_NT_PTMP(ctrl)) {
			return -1;
		}
		call = ctrl->link.dummy_call;
		if (!call) {
			return -1;
		}
		break;
	default:
		return -1;
	}

	pri_copy_party_id_to_q931(&called, mailbox);
	if (rose_mwi_indicate_encode(ctrl, call, vm_id, basic_service, num_messages,
		caller_id, timestamp, message_reference, message_status)
		|| q931_facility_called(ctrl, call, &called)) {
		pri_message(ctrl,
			"Could not schedule facility message for MWI indicate message.\n");
		return -1;
	}

	return 0;
}

int pri_mwi_indicate(struct pri *ctrl, const struct pri_party_id *mailbox,
	int basic_service, int num_messages, const struct pri_party_id *caller_id,
	const char *timestamp, int message_reference, int message_status)
{
	return pri_mwi_indicate_v2(ctrl, mailbox, mailbox, basic_service, num_messages,
	caller_id, timestamp, message_reference, message_status);
}
/* End MWI */

/* EECT functions */
/*!
 * \internal
 * \brief Encode the NI2 InitiateTransfer invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param call Call leg from which to encode transfer information.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_ni2_initiate_transfer(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, q931_call *call)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_NI2_InitiateTransfer;
	msg.invoke_id = get_invokeid(ctrl);
	/* Let's do the trickery to make sure the flag is correct */
	msg.args.ni2.InitiateTransfer.call_reference = call->cr ^ Q931_CALL_REFERENCE_FLAG;
	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \brief Start a 2BCT
 *
 * \note Called for PRI_SWITCH_NI2, PRI_SWITCH_LUCENT5E, and PRI_SWITCH_ATT4ESS
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param c1 Q.931 call leg 1
 * \param c2 Q.931 call leg 2
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int eect_initiate_transfer(struct pri *ctrl, q931_call *c1, q931_call *c2)
{
	unsigned char buffer[255];
	unsigned char *end;

	end = enc_ni2_initiate_transfer(ctrl, buffer, buffer + sizeof(buffer), c2);
	if (!end) {
		return -1;
	}

	if (pri_call_apdu_queue(c1, Q931_FACILITY, buffer, end - buffer, NULL)) {
		pri_message(ctrl, "Could not queue APDU in facility message\n");
		return -1;
	}

	/* Remember that if we queue a facility IE for a facility message we
	 * have to explicitly send the facility message ourselves */

	if (q931_facility(c1->pri, c1)) {
		pri_message(ctrl, "Could not schedule facility message for call %d\n", c1->cr);
		return -1;
	}

	return 0;
}
/* End EECT */

/* QSIG CF CallRerouting */
/*!
 * \internal
 * \brief Encode the Q.SIG CallRerouting invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param call Q.931 call leg.
 * \param calling Call rerouting/deflecting updated caller data.
 * \param deflection Call rerouting/deflecting redirection data.
 * \param subscription_option Diverting user subscription option to specify if caller is notified.
 *
 * \note
 * deflection->to is the new called number and must always be present.
 * \note
 * subscription option:
 * noNotification(0),
 * notificationWithoutDivertedToNr(1),
 * notificationWithDivertedToNr(2)
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_qsig_call_rerouting(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, q931_call *call, const struct q931_party_id *calling,
	const struct q931_party_redirecting *deflection, int subscription_option)
{
	struct fac_extension_header header;
	struct rose_msg_invoke msg;
	unsigned char *q931ie_pos;

	memset(&header, 0, sizeof(header));
	header.nfe_present = 1;
	header.nfe.source_entity = 0;	/* endPINX */
	header.nfe.destination_entity = 0;	/* endPINX */
	header.interpretation_present = 1;
	header.interpretation = 2;	/* rejectAnyUnrecognisedInvokePdu */
	pos = facility_encode_header(ctrl, pos, end, &header);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_QSIG_CallRerouting;
	msg.invoke_id = get_invokeid(ctrl);

	msg.args.qsig.CallRerouting.rerouting_reason =
		redirectingreason_from_q931(ctrl, deflection->reason);

	/* calledAddress is the passed in deflection->to address */
	q931_copy_id_address_to_rose(ctrl, &msg.args.qsig.CallRerouting.called, &deflection->to);

	msg.args.qsig.CallRerouting.diversion_counter = deflection->count;

	/* pSS1InfoElement */
	q931ie_pos = msg.args.qsig.CallRerouting.q931ie_contents;
	*q931ie_pos++ = 0x04;	/* Bearer Capability IE */
	*q931ie_pos++ = 0x03;	/* len */
	*q931ie_pos++ = 0x80 | call->bc.transcapability;	/* Rxed transfer capability. */
	*q931ie_pos++ = 0x90;	/* circuit mode, 64kbit/s */
	*q931ie_pos++ = 0xa3;	/* level1 protocol, a-law */
	*q931ie_pos++ = 0x95;	/* locking shift to codeset 5 (national use) */
	*q931ie_pos++ = 0x32;	/* Unknown ie */
	*q931ie_pos++ = 0x01;	/* Unknown ie len */
	*q931ie_pos++ = 0x81;	/* Unknown ie body */
	msg.args.qsig.CallRerouting.q931ie.length = q931ie_pos
		- msg.args.qsig.CallRerouting.q931ie_contents;

	/* lastReroutingNr is the passed in deflection->from.number */
	q931_copy_presented_number_unscreened_to_rose(ctrl,
		&msg.args.qsig.CallRerouting.last_rerouting, &deflection->from.number);

	msg.args.qsig.CallRerouting.subscription_option = subscription_option;

	/* callingNumber is the passed in calling->number */
	q931_copy_presented_number_screened_to_rose(ctrl,
		&msg.args.qsig.CallRerouting.calling, &calling->number);

	/* callingPartySubaddress is the passed in calling->subaddress if valid */
	q931_copy_subaddress_to_rose(ctrl, &msg.args.qsig.CallRerouting.calling_subaddress,
		&calling->subaddress);

	/* callingName is the passed in calling->name if valid */
	if (calling->name.valid) {
		msg.args.qsig.CallRerouting.calling_name_present = 1;
		q931_copy_name_to_rose(ctrl, &msg.args.qsig.CallRerouting.calling_name,
			&calling->name);
	}

	if (1 < deflection->count) {
		/* originalCalledNr is the deflection->orig_called.number */
		msg.args.qsig.CallRerouting.original_called_present = 1;
		q931_copy_presented_number_unscreened_to_rose(ctrl,
			&msg.args.qsig.CallRerouting.original_called,
			&deflection->orig_called.number);

		msg.args.qsig.CallRerouting.original_rerouting_reason_present = 1;
		if (deflection->orig_called.number.valid) {
			msg.args.qsig.CallRerouting.original_rerouting_reason =
				redirectingreason_from_q931(ctrl, deflection->orig_reason);
		} else {
			msg.args.qsig.CallRerouting.original_rerouting_reason =
				QSIG_DIVERT_REASON_UNKNOWN;
		}

		/* originalCalledName is the deflection->orig_called.name */
		if (deflection->orig_called.name.valid) {
			msg.args.qsig.CallRerouting.original_called_name_present = 1;
			q931_copy_name_to_rose(ctrl,
				&msg.args.qsig.CallRerouting.original_called_name,
				&deflection->orig_called.name);
		}
	}

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode the ETSI CallRerouting invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param call Q.931 call leg.
 * \param calling Call rerouting/deflecting updated caller data.
 * \param deflection Call rerouting/deflecting redirection data.
 * \param subscription_option Diverting user subscription option to specify if caller is notified.
 *
 * \note
 * deflection->to is the new called number and must always be present.
 * \note
 * subscription option:
 * noNotification(0),
 * notificationWithoutDivertedToNr(1),
 * notificationWithDivertedToNr(2)
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_call_rerouting(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, q931_call *call, const struct q931_party_id *calling,
	const struct q931_party_redirecting *deflection, int subscription_option)
{
	struct rose_msg_invoke msg;
	unsigned char *q931ie_pos;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_ETSI_CallRerouting;
	msg.invoke_id = get_invokeid(ctrl);

	msg.args.etsi.CallRerouting.rerouting_reason =
		redirectingreason_from_q931(ctrl, deflection->reason);

	/* calledAddress is the passed in deflection->to address */
	q931_copy_id_address_to_rose(ctrl, &msg.args.etsi.CallRerouting.called_address,
		&deflection->to);

	msg.args.etsi.CallRerouting.rerouting_counter = deflection->count;

	/* q931InfoElement */
	q931ie_pos = msg.args.etsi.CallRerouting.q931ie_contents;
	*q931ie_pos++ = 0x04;	/* Bearer Capability IE */
	*q931ie_pos++ = 0x03;	/* len */
	*q931ie_pos++ = 0x80 | call->bc.transcapability;	/* Rxed transfer capability. */
	*q931ie_pos++ = 0x90;	/* circuit mode, 64kbit/s */
	*q931ie_pos++ = 0xa3;	/* level1 protocol, a-law */
	msg.args.etsi.CallRerouting.q931ie.length = q931ie_pos
		- msg.args.etsi.CallRerouting.q931ie_contents;

	/* lastReroutingNr is the passed in deflection->from.number */
	q931_copy_presented_number_unscreened_to_rose(ctrl,
		&msg.args.etsi.CallRerouting.last_rerouting, &deflection->from.number);

	msg.args.etsi.CallRerouting.subscription_option = subscription_option;

	/* callingPartySubaddress is the passed in calling->subaddress if valid */
	q931_copy_subaddress_to_rose(ctrl, &msg.args.etsi.CallRerouting.calling_subaddress,
		&calling->subaddress);

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode the ETSI CallDeflection invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param call Q.931 call leg.
 * \param deflection Call deflection address.
 *
 * \note
 * deflection is the new called number and must always be present.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_call_deflection(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, q931_call *call, const struct q931_party_id *deflection)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_ETSI_CallDeflection;
	msg.invoke_id = get_invokeid(ctrl);

	/* deflectionAddress is the passed in deflection->to address */
	q931_copy_id_address_to_rose(ctrl, &msg.args.etsi.CallDeflection.deflection,
		deflection);

	msg.args.etsi.CallDeflection.presentation_allowed_to_diverted_to_user_present = 1;
	switch (deflection->number.presentation & PRI_PRES_RESTRICTION) {
	case PRI_PRES_ALLOWED:
		msg.args.etsi.CallDeflection.presentation_allowed_to_diverted_to_user = 1;
		break;
	default:
	case PRI_PRES_UNAVAILABLE:
	case PRI_PRES_RESTRICTED:
		break;
	}

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode and queue the CallRerouting/CallDeflection message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param caller Call rerouting/deflecting updated caller data. (NULL if data not updated.)
 * \param deflection Call rerouting/deflecting redirection data.
 * \param subscription_option Diverting user subscription option to specify if caller is notified.
 *
 * \note
 * deflection->to is the new called number and must always be present.
 * \note
 * subscription option:
 * noNotification(0),
 * notificationWithoutDivertedToNr(1),
 * notificationWithDivertedToNr(2)
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_reroute_request_encode(struct pri *ctrl, q931_call *call,
	const struct q931_party_id *caller, const struct q931_party_redirecting *deflection,
	int subscription_option)
{
	unsigned char buffer[256];
	unsigned char *end;

	if (!caller) {
		/*
		 * We are deflecting an incoming call back to the network.
		 * Therefore, the Caller-ID is the remote party.
		 */
		caller = &call->remote_id;
	}

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		if (PTMP_MODE(ctrl)) {
			end =
				enc_etsi_call_deflection(ctrl, buffer, buffer + sizeof(buffer), call,
					&deflection->to);
		} else {
			end =
				enc_etsi_call_rerouting(ctrl, buffer, buffer + sizeof(buffer), call,
					caller, deflection, subscription_option);
		}
		break;
	case PRI_SWITCH_QSIG:
		end =
			enc_qsig_call_rerouting(ctrl, buffer, buffer + sizeof(buffer), call, caller,
				deflection, subscription_option);
		break;
	default:
		return -1;
	}
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL);
}

/*!
 * \brief Send the CallRerouting/CallDeflection message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param caller Call rerouting/deflecting updated caller data. (NULL if data not updated.)
 * \param deflection Call rerouting/deflecting redirection data.
 * \param subscription_option Diverting user subscription option to specify if caller is notified.
 *
 * \note
 * deflection->to is the new called number and must always be present.
 * \note
 * subscription option:
 * noNotification(0),
 * notificationWithoutDivertedToNr(1),
 * notificationWithDivertedToNr(2)
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int send_reroute_request(struct pri *ctrl, q931_call *call,
	const struct q931_party_id *caller, const struct q931_party_redirecting *deflection,
	int subscription_option)
{
	if (!deflection->to.number.str[0]) {
		/* Must have a deflect to number.  That is the point of deflection. */
		return -1;
	}
	if (rose_reroute_request_encode(ctrl, call, caller, deflection, subscription_option)
		|| q931_facility(ctrl, call)) {
		pri_message(ctrl,
			"Could not schedule facility message for CallRerouting/CallDeflection message.\n");
		return -1;
	}

	return 0;
}

/*!
 * \brief Send the Q.SIG CallRerouting invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Q.931 call leg.
 * \param dest Destination number.
 * \param original Original called number.
 * \param reason Rerouting reason: cfu, cfb, cfnr
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int qsig_cf_callrerouting(struct pri *ctrl, q931_call *call, const char *dest,
	const char *original, const char *reason)
{
	struct q931_party_redirecting reroute;

	q931_party_redirecting_init(&reroute);

	/* Rerouting to the dest number. */
	reroute.to.number.valid = 1;
	reroute.to.number.plan = (PRI_TON_UNKNOWN << 4) | PRI_NPI_E163_E164;
	reroute.to.number.presentation = PRI_PRES_ALLOWED | PRI_PRES_USER_NUMBER_UNSCREENED;
	libpri_copy_string(reroute.to.number.str, dest, sizeof(reroute.to.number.str));

	/* Rerouting from the original number. */
	if (original) {
		reroute.from.number.valid = 1;
		reroute.from.number.plan = (PRI_TON_UNKNOWN << 4) | PRI_NPI_E163_E164;
		libpri_copy_string(reroute.from.number.str, original, sizeof(reroute.from.number.str));
	} else {
		q931_party_address_to_id(&reroute.from, &call->called);
	}
	reroute.from.number.presentation = PRI_PRES_ALLOWED | PRI_PRES_USER_NUMBER_UNSCREENED;

	/* Decode the rerouting reason. */
	reroute.reason = PRI_REDIR_UNKNOWN;
	if (!reason) {
		/* No reason for rerouting given. */
	} else if (!strcasecmp(reason, "cfu")) {
		reroute.reason = PRI_REDIR_UNCONDITIONAL;
	} else if (!strcasecmp(reason, "cfb")) {
		reroute.reason = PRI_REDIR_FORWARD_ON_BUSY;
	} else if (!strcasecmp(reason, "cfnr")) {
		reroute.reason = PRI_REDIR_FORWARD_ON_NO_REPLY;
	}

	reroute.count = (call->redirecting.count < PRI_MAX_REDIRECTS)
		? call->redirecting.count + 1 : PRI_MAX_REDIRECTS;

	if (!call->redirecting.orig_called.number.valid) {
		/*
		 * Since we do not already have an originally called party, we
		 * must either be the first redirected to party or this call
		 * has not been redirected before.
		 *
		 * Preserve who redirected to us as the originally called party.
		 */
		reroute.orig_called = call->redirecting.from;
		reroute.orig_reason = call->redirecting.reason;
	} else {
		reroute.orig_called = call->redirecting.orig_called;
		reroute.orig_reason = call->redirecting.orig_reason;
	}

	return send_reroute_request(ctrl, call, NULL, &reroute, 0 /* noNotification */);
}
/* End QSIG CC-CallRerouting */

/*
 * From Mantis issue 7778 description: (ETS 300 258, ISO 13863)
 * After both legs of the call are setup and Asterisk has a successful "tromboned" or bridged call ...
 * Asterisk sees both 'B' channels (from trombone) are on same PRI/technology and initiates "Path Replacement" events
 * a. Asterisk sends "Transfer Complete" messages to both call legs
 * b. QSIG Switch sends "PathReplacement" message on one of the legs (random 1-10sec timer expires - 1st leg to send is it!)
 * c. Asterisk rebroadcasts "PathReplacement" message to other call leg
 * d. QSIG Switch sends "Disconnect" message on one of the legs (same random timer sequence as above)
 * e. Asterisk rebroadcasts "Disconnect" message to other call leg
 * f. QSIG Switch disconnects Asterisk call legs - callers are now within QSIG switch
 *
 * Just need to resend the message to the other tromboned leg of the call.
 */
static int anfpr_pathreplacement_respond(struct pri *ctrl, q931_call *call, q931_ie *ie)
{
	int res;

	pri_call_apdu_queue_cleanup(call->bridged_call);

	/* Send message */
	res = pri_call_apdu_queue(call->bridged_call, Q931_FACILITY, ie->data, ie->len, NULL);
	if (res) {
		pri_message(ctrl, "Could not queue ADPU in facility message\n");
		return -1;
	}

	/* Remember that if we queue a facility IE for a facility message we
	 * have to explicitly send the facility message ourselves */

	res = q931_facility(call->bridged_call->pri, call->bridged_call);
	if (res) {
		pri_message(ctrl, "Could not schedule facility message for call %d\n",
			call->bridged_call->cr);
		return -1;
	}

	return 0;
}

/* AFN-PR */
/*!
 * \brief Start a Q.SIG path replacement.
 *
 * \note Called for PRI_SWITCH_QSIG
 *
 * \note Did all the tests to see if we're on the same PRI and
 * are on a compatible switchtype.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param c1 Q.931 call leg 1
 * \param c2 Q.931 call leg 2
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int anfpr_initiate_transfer(struct pri *ctrl, q931_call *c1, q931_call *c2)
{
	unsigned char buffer[255];
	unsigned char *pos;
	unsigned char *end;
	int res;
	struct fac_extension_header header;
	struct rose_msg_invoke msg;

	end = buffer + sizeof(buffer);

	memset(&header, 0, sizeof(header));
	header.nfe_present = 1;
	header.nfe.source_entity = 0;	/* endPINX */
	header.nfe.destination_entity = 0;	/* endPINX */
	header.interpretation_present = 1;
	header.interpretation = 2;	/* rejectAnyUnrecognisedInvokePdu */
	pos = facility_encode_header(ctrl, buffer, end, &header);
	if (!pos) {
		return -1;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_QSIG_CallTransferComplete;
	msg.invoke_id = get_invokeid(ctrl);
	msg.args.qsig.CallTransferComplete.end_designation = 0;	/* primaryEnd */
	msg.args.qsig.CallTransferComplete.redirection.presentation = 1;	/* presentationRestricted */
	msg.args.qsig.CallTransferComplete.call_status = 1;	/* alerting */
	pos = rose_encode_invoke(ctrl, pos, end, &msg);
	if (!pos) {
		return -1;
	}

	res = pri_call_apdu_queue(c1, Q931_FACILITY, buffer, pos - buffer, NULL);
	if (res) {
		pri_message(ctrl, "Could not queue ADPU in facility message\n");
		return -1;
	}

	/* Remember that if we queue a facility IE for a facility message we
	 * have to explicitly send the facility message ourselves */

	res = q931_facility(c1->pri, c1);
	if (res) {
		pri_message(ctrl, "Could not schedule facility message for call %d\n", c1->cr);
		return -1;
	}

	/* Reuse the previous message header */
	pos = facility_encode_header(ctrl, buffer, end, &header);
	if (!pos) {
		return -1;
	}

	/* Update the previous message */
	msg.invoke_id = get_invokeid(ctrl);
	msg.args.qsig.CallTransferComplete.end_designation = 1;	/* secondaryEnd */
	pos = rose_encode_invoke(ctrl, pos, end, &msg);
	if (!pos) {
		return -1;
	}

	res = pri_call_apdu_queue(c2, Q931_FACILITY, buffer, pos - buffer, NULL);
	if (res) {
		pri_message(ctrl, "Could not queue ADPU in facility message\n");
		return -1;
	}

	/* Remember that if we queue a facility IE for a facility message we
	 * have to explicitly send the facility message ourselves */

	res = q931_facility(c2->pri, c2);
	if (res) {
		pri_message(ctrl, "Could not schedule facility message for call %d\n", c2->cr);
		return -1;
	}

	return 0;
}
/* End AFN-PR */

/*!
 * \internal
 * \brief Encode ETSI ExplicitEctExecute message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param link_id Identifier of other call involved in transfer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_ect_explicit_execute(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, int link_id)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = get_invokeid(ctrl);
	msg.operation = ROSE_ETSI_ExplicitEctExecute;

	msg.args.etsi.ExplicitEctExecute.link_id = link_id;

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief ECT LinkId response callback function.
 *
 * \param reason Reason callback is called.
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param apdu APDU queued entry.  Do not change!
 * \param msg APDU response message data.  (NULL if was not the reason called.)
 *
 * \return TRUE if no more responses are expected.
 */
static int etsi_ect_link_id_rsp(enum APDU_CALLBACK_REASON reason, struct pri *ctrl,
	struct q931_call *call, struct apdu_event *apdu, const struct apdu_msg_data *msg)
{
	unsigned char buffer[256];
	unsigned char *end;
	q931_call *call_2;

	switch (reason) {
	case APDU_CALLBACK_REASON_MSG_RESULT:
		call_2 = apdu->response.user.ptr;
		if (!q931_is_call_valid(ctrl, call_2)) {
			/* Call is no longer present. */
			break;
		}

		end = enc_etsi_ect_explicit_execute(ctrl, buffer, buffer + sizeof(buffer),
			msg->response.result->args.etsi.EctLinkIdRequest.link_id);
		if (!end) {
			break;
		}

		/* Remember that if we queue a facility IE for a facility message we
		 * have to explicitly send the facility message ourselves */
		if (pri_call_apdu_queue(call_2, Q931_FACILITY, buffer, end - buffer, NULL)
			|| q931_facility(call_2->pri, call_2)) {
			pri_message(ctrl, "Could not schedule facility message for call %d\n",
				call_2->cr);
		}
		break;
	default:
		break;
	}
	return 1;
}

/*!
 * \internal
 * \brief Encode ETSI ECT LinkId request message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_ect_link_id_req(struct pri *ctrl, unsigned char *pos,
	unsigned char *end)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = get_invokeid(ctrl);
	msg.operation = ROSE_ETSI_EctLinkIdRequest;

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \brief Start an Explicit Call Transfer (ECT) sequence between the two calls.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call_1 Q.931 call leg 1
 * \param call_2 Q.931 call leg 2
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int etsi_initiate_transfer(struct pri *ctrl, q931_call *call_1, q931_call *call_2)
{
	unsigned char buffer[256];
	unsigned char *end;
	struct apdu_callback_data response;

	end = enc_etsi_ect_link_id_req(ctrl, buffer, buffer + sizeof(buffer));
	if (!end) {
		return -1;
	}

	memset(&response, 0, sizeof(response));
	response.invoke_id = ctrl->last_invoke;
	response.timeout_time = ctrl->timers[PRI_TIMER_T_RESPONSE];
	response.callback = etsi_ect_link_id_rsp;
	response.user.ptr = call_2;

	/* Remember that if we queue a facility IE for a facility message we
	 * have to explicitly send the facility message ourselves */
	if (pri_call_apdu_queue(call_1, Q931_FACILITY, buffer, end - buffer, &response)
		|| q931_facility(call_1->pri, call_1)) {
		pri_message(ctrl, "Could not schedule facility message for call %d\n",
			call_1->cr);
		return -1;
	}

	return 0;
}

/*!
 * \internal
 * \brief Encode ETSI ECT LinkId result respnose message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param invoke_id Invoke id to put in result message.
 * \param link_id Requested link id to put in result message.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_ect_link_id_rsp(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, int invoke_id, int link_id)
{
	struct rose_msg_result msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = invoke_id;
	msg.operation = ROSE_ETSI_EctLinkIdRequest;

	msg.args.etsi.EctLinkIdRequest.link_id = link_id;

	pos = rose_encode_result(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Send EctLinkIdRequest result response message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param invoke_id Invoke id to put in result message.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int send_ect_link_id_rsp(struct pri *ctrl, q931_call *call, int invoke_id)
{
	unsigned char buffer[256];
	unsigned char *end;

	end = enc_etsi_ect_link_id_rsp(ctrl, buffer, buffer + sizeof(buffer), invoke_id,
		call->link_id);
	if (!end) {
		return -1;
	}

	/* Remember that if we queue a facility IE for a facility message we
	 * have to explicitly send the facility message ourselves */
	if (pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL)
		|| q931_facility(call->pri, call)) {
		pri_message(ctrl, "Could not schedule facility message for call %d\n", call->cr);
		return -1;
	}

	return 0;
}

/*!
 * \internal
 * \brief Process the received ETSI EctExecute message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param invoke_id Invoke id to put in response message.
 *
 * \details
 * 1) Find the active call implied by the transfer request.
 * 2) Create the PRI_SUBCMD_TRANSFER_CALL event.
 *
 * \retval ROSE_ERROR_None on success.
 * \retval error_code on error.
 */
static enum rose_error_code etsi_ect_execute_transfer(struct pri *ctrl, q931_call *call, int invoke_id)
{
	enum rose_error_code error_code;
	struct pri_subcommand *subcmd;
	q931_call *call_active;

	switch (call->ourcallstate) {
	case Q931_CALL_STATE_OUTGOING_CALL_PROCEEDING:
	case Q931_CALL_STATE_CALL_DELIVERED:
	case Q931_CALL_STATE_CALL_RECEIVED:
	case Q931_CALL_STATE_CONNECT_REQUEST:
	case Q931_CALL_STATE_INCOMING_CALL_PROCEEDING:
	case Q931_CALL_STATE_ACTIVE:
		if (call->master_call->hold_state != Q931_HOLD_STATE_CALL_HELD) {
			/* EctExecute must be sent on the held call. */
			error_code = ROSE_ERROR_Gen_InvalidCallState;
			break;
		}
		/* Held call is being transferred. */
		call_active = q931_find_held_active_call(ctrl, call);
		if (!call_active) {
			error_code = ROSE_ERROR_Gen_NotAvailable;
			break;
		}

		/* Setup transfer subcommand */
		subcmd = q931_alloc_subcommand(ctrl);
		if (!subcmd) {
			error_code = ROSE_ERROR_Gen_NotAvailable;
			break;
		}
		subcmd->cmd = PRI_SUBCMD_TRANSFER_CALL;
		subcmd->u.transfer.call_1 = call->master_call;
		subcmd->u.transfer.call_2 = call_active;
		subcmd->u.transfer.is_call_1_held = 1;
		subcmd->u.transfer.is_call_2_held = 0;
		subcmd->u.transfer.invoke_id = invoke_id;

		error_code = ROSE_ERROR_None;
		break;
	default:
		error_code = ROSE_ERROR_Gen_InvalidCallState;
		break;
	}

	return error_code;
}

/*!
 * \internal
 * \brief Process the received ETSI ExplicitEctExecute message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param invoke_id Invoke id to put in response message.
 * \param link_id Link id of the other call involved in the transfer.
 *
 * \details
 * 1) Find the other call specified by the link_id in transfer request.
 * 2) Create the PRI_SUBCMD_TRANSFER_CALL event.
 *
 * \retval ROSE_ERROR_None on success.
 * \retval error_code on error.
 */
static enum rose_error_code etsi_explicit_ect_execute_transfer(struct pri *ctrl, q931_call *call, int invoke_id, int link_id)
{
	enum rose_error_code error_code;
	struct pri_subcommand *subcmd;
	q931_call *call_2;

	switch (call->ourcallstate) {
	case Q931_CALL_STATE_OUTGOING_CALL_PROCEEDING:
	case Q931_CALL_STATE_CALL_DELIVERED:
	case Q931_CALL_STATE_CALL_RECEIVED:
	case Q931_CALL_STATE_CONNECT_REQUEST:
	case Q931_CALL_STATE_INCOMING_CALL_PROCEEDING:
	case Q931_CALL_STATE_ACTIVE:
		call_2 = q931_find_link_id_call(ctrl, link_id);
		if (!call_2 || call_2 == call->master_call) {
			error_code = ROSE_ERROR_Gen_NotAvailable;
			break;
		}

		/* Setup transfer subcommand */
		subcmd = q931_alloc_subcommand(ctrl);
		if (!subcmd) {
			error_code = ROSE_ERROR_Gen_NotAvailable;
			break;
		}
		subcmd->cmd = PRI_SUBCMD_TRANSFER_CALL;
		subcmd->u.transfer.call_1 = call->master_call;
		subcmd->u.transfer.call_2 = call_2;
		subcmd->u.transfer.is_call_1_held =
			(call->master_call->hold_state == Q931_HOLD_STATE_CALL_HELD) ? 1 : 0;
		subcmd->u.transfer.is_call_2_held =
			(call_2->hold_state == Q931_HOLD_STATE_CALL_HELD) ? 1 : 0;
		subcmd->u.transfer.invoke_id = invoke_id;

		error_code = ROSE_ERROR_None;
		break;
	default:
		error_code = ROSE_ERROR_Gen_InvalidCallState;
		break;
	}

	return error_code;
}

/* ===== Call Transfer Supplementary Service (ECMA-178) ===== */

/*!
 * \internal
 * \brief Encode the Q.SIG CallTransferComplete invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param call Call leg from which to encode call transfer.
 * \param call_status TRUE if call is alerting.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_qsig_call_transfer_complete(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, q931_call *call, int call_status)
{
	struct fac_extension_header header;
	struct rose_msg_invoke msg;

	memset(&header, 0, sizeof(header));
	header.nfe_present = 1;
	header.nfe.source_entity = 0;	/* endPINX */
	header.nfe.destination_entity = 0;	/* endPINX */
	header.interpretation_present = 1;
	header.interpretation = 0;	/* discardAnyUnrecognisedInvokePdu */
	pos = facility_encode_header(ctrl, pos, end, &header);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_QSIG_CallTransferComplete;
	msg.invoke_id = get_invokeid(ctrl);
	msg.args.qsig.CallTransferComplete.end_designation = 0;	/* primaryEnd */

	/* redirectionNumber is the local_id.number */
	q931_copy_presented_number_screened_to_rose(ctrl,
		&msg.args.qsig.CallTransferComplete.redirection, &call->local_id.number);

	/* redirectionName is the local_id.name */
	if (call->local_id.name.valid) {
		msg.args.qsig.CallTransferComplete.redirection_name_present = 1;
		q931_copy_name_to_rose(ctrl,
			&msg.args.qsig.CallTransferComplete.redirection_name,
			&call->local_id.name);
	}

	if (call_status) {
		msg.args.qsig.CallTransferComplete.call_status = 1;	/* alerting */
	}
	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode the ETSI EctInform invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param call Call leg from which to encode inform message.
 * \param call_status TRUE if call is alerting.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_ect_inform(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, q931_call *call, int call_status)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_ETSI_EctInform;
	msg.invoke_id = get_invokeid(ctrl);

	if (!call_status) {
		msg.args.etsi.EctInform.status = 1;/* active */

		/*
		 * EctInform(active) contains the redirectionNumber
		 * redirectionNumber is the local_id.number
		 */
		msg.args.etsi.EctInform.redirection_present = 1;
		q931_copy_presented_number_unscreened_to_rose(ctrl,
			&msg.args.etsi.EctInform.redirection, &call->local_id.number);
	}

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode and queue the CallTransferComplete/EctInform invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode call transfer.
 * \param call_status TRUE if call is alerting.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_call_transfer_complete_encode(struct pri *ctrl, q931_call *call,
	int call_status)
{
	unsigned char buffer[256];
	unsigned char *end;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		end =
			enc_etsi_ect_inform(ctrl, buffer, buffer + sizeof(buffer), call, call_status);
		break;
	case PRI_SWITCH_QSIG:
		end =
			enc_qsig_call_transfer_complete(ctrl, buffer, buffer + sizeof(buffer), call,
			call_status);
		break;
	default:
		return -1;
	}
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL);
}

/* ===== End Call Transfer Supplementary Service (ECMA-178) ===== */

/*!
 * \internal
 * \brief Encode the Q.SIG CalledName invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param name Name data which to encode name.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_qsig_called_name(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct q931_party_name *name)
{
	struct fac_extension_header header;
	struct rose_msg_invoke msg;

	memset(&header, 0, sizeof(header));
	header.nfe_present = 1;
	header.nfe.source_entity = 0;	/* endPINX */
	header.nfe.destination_entity = 0;	/* endPINX */
	header.interpretation_present = 1;
	header.interpretation = 0;	/* discardAnyUnrecognisedInvokePdu */
	pos = facility_encode_header(ctrl, pos, end, &header);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_QSIG_CalledName;
	msg.invoke_id = get_invokeid(ctrl);

	/* CalledName */
	q931_copy_name_to_rose(ctrl, &msg.args.qsig.CalledName.name, name);

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode and queue the Q.SIG CalledName invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode name.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int rose_called_name_encode(struct pri *ctrl, q931_call *call, int messagetype)
{
	unsigned char buffer[256];
	unsigned char *end;

	/* CalledName is the local_id.name */
	end = enc_qsig_called_name(ctrl, buffer, buffer + sizeof(buffer),
		&call->local_id.name);
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, messagetype, buffer, end - buffer, NULL);
}

/*!
 * \internal
 * \brief Encode the Q.SIG ConnectedName invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param name Name data which to encode name.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_qsig_connected_name(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct q931_party_name *name)
{
	struct fac_extension_header header;
	struct rose_msg_invoke msg;

	memset(&header, 0, sizeof(header));
	header.nfe_present = 1;
	header.nfe.source_entity = 0;	/* endPINX */
	header.nfe.destination_entity = 0;	/* endPINX */
	header.interpretation_present = 1;
	header.interpretation = 0;	/* discardAnyUnrecognisedInvokePdu */
	pos = facility_encode_header(ctrl, pos, end, &header);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_QSIG_ConnectedName;
	msg.invoke_id = get_invokeid(ctrl);

	/* ConnectedName */
	q931_copy_name_to_rose(ctrl, &msg.args.qsig.ConnectedName.name, name);

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode and queue the Q.SIG ConnectedName invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode name.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int rose_connected_name_encode(struct pri *ctrl, q931_call *call, int messagetype)
{
	unsigned char buffer[256];
	unsigned char *end;

	/* ConnectedName is the local_id.name */
	end = enc_qsig_connected_name(ctrl, buffer, buffer + sizeof(buffer),
		&call->local_id.name);
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, messagetype, buffer, end - buffer, NULL);
}

/*!
 * \brief Put the APDU on the call queue.
 *
 * \param call Call to enqueue message.
 * \param messagetype Q.931 message type.
 * \param apdu Facility ie contents buffer.
 * \param apdu_len Length of the contents buffer.
 * \param response Sender supplied information to handle APDU response messages.
 *        NULL if don't care about responses.
 *
 * \note
 * Only APDU messages with an invoke component can supply a response pointer.
 * If any other APDU messages supply a response pointer then aliasing of the
 * invoke_id can occur.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int pri_call_apdu_queue(q931_call *call, int messagetype, const unsigned char *apdu, int apdu_len, struct apdu_callback_data *response)
{
	struct apdu_event *cur = NULL;
	struct apdu_event *new_event = NULL;

	if (!call || !messagetype || !apdu
		|| apdu_len < 1 || sizeof(new_event->apdu) < apdu_len) {
		return -1;
	}
	switch (messagetype) {
	case Q931_FACILITY:
		break;
	default:
		if (q931_is_dummy_call(call)) {
			pri_error(call->pri, "!! Cannot send %s message on dummy call reference.\n",
				msg2str(messagetype));
			return -1;
		}
		break;
	}

	new_event = calloc(1, sizeof(*new_event));
	if (!new_event) {
		pri_error(call->pri, "!! Malloc failed!\n");
		return -1;
	}

	/* Fill in the APDU event */
	new_event->message = messagetype;
	if (response) {
		new_event->response = *response;
	}
	new_event->call = call;
	new_event->apdu_len = apdu_len;
	memcpy(new_event->apdu, apdu, apdu_len);

	/* Append APDU event to the end of the list. */
	if (call->apdus) {
		for (cur = call->apdus; cur->next; cur = cur->next) {
		}
		cur->next = new_event;
	} else {
		call->apdus = new_event;
	}

	return 0;
}

/* Used by q931.c to cleanup the apdu queue upon destruction of a call */
void pri_call_apdu_queue_cleanup(q931_call *call)
{
	struct apdu_event *cur_event;
	struct apdu_event *free_event;

	if (call) {
		cur_event = call->apdus;
		call->apdus = NULL;
		while (cur_event) {
			if (cur_event->response.callback) {
				/* Stop any response timeout. */
				pri_schedule_del(call->pri, cur_event->timer);
				cur_event->timer = 0;

				/* Indicate to callback that the APDU is being cleaned up. */
				cur_event->response.callback(APDU_CALLBACK_REASON_CLEANUP, call->pri,
					call, cur_event, NULL);
			}

			free_event = cur_event;
			cur_event = cur_event->next;
			free(free_event);
		}
	}
}

/*!
 * \brief Find an outstanding APDU with the given invoke id.
 *
 * \param call Call to find APDU.
 * \param invoke_id Invoke id to match outstanding APDUs in queue.
 *
 * \retval apdu_event if found.
 * \retval NULL if not found.
 */
struct apdu_event *pri_call_apdu_find(struct q931_call *call, int invoke_id)
{
	struct apdu_event *apdu;

	if (invoke_id == APDU_INVALID_INVOKE_ID) {
		/* No need to search the list since it cannot be in there. */
		return NULL;
	}
	for (apdu = call->apdus; apdu; apdu = apdu->next) {
		/*
		 * Note: The APDU cannot be sent and still in the queue without a
		 * callback and timeout timer active.  Therefore, an invoke_id of
		 * zero is valid and not just the result of a memset().
		 */
		if (apdu->response.invoke_id == invoke_id && apdu->sent) {
			break;
		}
	}
	return apdu;
}

/*!
 * \brief Extract the given APDU event from the given call.
 *
 * \param call Call to remove the APDU.
 * \param extract APDU event to extract.
 *
 * \retval TRUE on success.
 * \retval FALSE on error.
 */
int pri_call_apdu_extract(struct q931_call *call, struct apdu_event *extract)
{
	struct apdu_event **prev;
	struct apdu_event *cur;

	/* Find APDU in list. */
	for (prev = &call->apdus, cur = call->apdus;
		cur;
		prev = &cur->next, cur = cur->next) {
		if (cur == extract) {
			/* Stop any response timeout. */
			pri_schedule_del(call->pri, cur->timer);
			cur->timer = 0;

			/* Remove APDU from list. */
			*prev = cur->next;

			/* Found and extracted APDU from list. */
			return 1;
		}
	}

	/* Did not find the APDU in the list. */
	return 0;
}

/*!
 * \brief Delete the given APDU event from the given call.
 *
 * \param call Call to remove the APDU.
 * \param doomed APDU event to delete.
 *
 * \return Nothing
 */
void pri_call_apdu_delete(struct q931_call *call, struct apdu_event *doomed)
{
	if (pri_call_apdu_extract(call, doomed)) {
		free(doomed);
	}
}

/*! \note Only called when sending the SETUP message. */
int pri_call_add_standard_apdus(struct pri *ctrl, q931_call *call)
{
	if (!ctrl->sendfacility) {
		return 0;
	}

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		if (call->aoc_charging_request) {
			aoc_charging_request_send(ctrl, call, call->aoc_charging_request);
		}
		if (PTMP_MODE(ctrl)) {
			/* PTMP mode */
			break;
		}
		/* PTP mode */
		if (call->redirecting.count) {
			rose_diverting_leg_information2_encode(ctrl, call);

			/*
			 * Expect a DivertingLegInformation3 to update the COLR of the
			 * redirecting-to party we are attempting to call now.
			 */
			call->redirecting.state = Q931_REDIRECTING_STATE_EXPECTING_RX_DIV_LEG_3;
		}
		break;
	case PRI_SWITCH_QSIG:
		/* For Q.SIG it does network and cpe operations */
		if (call->redirecting.count) {
			rose_diverting_leg_information2_encode(ctrl, call);

			/*
			 * Expect a DivertingLegInformation3 to update the COLR of the
			 * redirecting-to party we are attempting to call now.
			 */
			call->redirecting.state = Q931_REDIRECTING_STATE_EXPECTING_RX_DIV_LEG_3;
		}
		add_callername_facility_ies(ctrl, call, 1);
		break;
	case PRI_SWITCH_NI2:
		add_callername_facility_ies(ctrl, call, (ctrl->localtype == PRI_CPE));
		break;
	case PRI_SWITCH_DMS100:
		if (ctrl->localtype == PRI_CPE) {
			add_dms100_transfer_ability_apdu(ctrl, call);
		}
		break;
	default:
		break;
	}

	return 0;
}

/*!
 * \brief Send the CallTransferComplete/EctInform invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode call transfer.
 * \param call_status TRUE if call is alerting.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int send_call_transfer_complete(struct pri *ctrl, q931_call *call, int call_status)
{
	int status;

	status = rose_call_transfer_complete_encode(ctrl, call, call_status);
	if (!status) {
		if (!call_status && call->local_id.number.valid
			&& (ctrl->display_flags.send & PRI_DISPLAY_OPTION_NAME_UPDATE)) {
			status = q931_facility_display_name(ctrl, call, &call->local_id.name);
		} else {
			status = q931_facility(ctrl, call);
		}
	}
	if (status) {
		pri_message(ctrl,
			"Could not schedule facility message for call transfer completed.\n");
		return -1;
	}

	return 0;
}

/*!
 * \internal
 * \brief Encode the ETSI RequestSubaddress invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_request_subaddress(struct pri *ctrl, unsigned char *pos,
	unsigned char *end)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_ETSI_RequestSubaddress;
	msg.invoke_id = get_invokeid(ctrl);

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \brief Encode and queue the RequestSubaddress invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode message.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int rose_request_subaddress_encode(struct pri *ctrl, struct q931_call *call)
{
	unsigned char buffer[256];
	unsigned char *end;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		end = enc_etsi_request_subaddress(ctrl, buffer, buffer + sizeof(buffer));
		break;
	case PRI_SWITCH_QSIG:
	default:
		return -1;
	}
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL);
}

/*!
 * \internal
 * \brief Encode the ETSI SubaddressTransfer invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param call Call leg from which to encode message.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_subaddress_transfer(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, struct q931_call *call)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_ETSI_SubaddressTransfer;
	msg.invoke_id = get_invokeid(ctrl);

	if (!call->local_id.subaddress.valid) {
		return NULL;
	}
	q931_copy_subaddress_to_rose(ctrl, &msg.args.etsi.SubaddressTransfer.subaddress,
		&call->local_id.subaddress);

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode the Q.SIG SubaddressTransfer invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param call Call leg from which to encode message.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_qsig_subaddress_transfer(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, struct q931_call *call)
{
	struct fac_extension_header header;
	struct rose_msg_invoke msg;

	memset(&header, 0, sizeof(header));
	header.nfe_present = 1;
	header.nfe.source_entity = 0;	/* endPINX */
	header.nfe.destination_entity = 0;	/* endPINX */
	header.interpretation_present = 1;
	header.interpretation = 0;	/* discardAnyUnrecognisedInvokePdu */
	pos = facility_encode_header(ctrl, pos, end, &header);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_QSIG_SubaddressTransfer;
	msg.invoke_id = get_invokeid(ctrl);

	if (!call->local_id.subaddress.valid) {
		return NULL;
	}
	q931_copy_subaddress_to_rose(ctrl,
		&msg.args.qsig.SubaddressTransfer.redirection_subaddress,
		&call->local_id.subaddress);

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode and queue the SubaddressTransfer invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode message.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_subaddress_transfer_encode(struct pri *ctrl, struct q931_call *call)
{
	unsigned char buffer[256];
	unsigned char *end;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		end =
			enc_etsi_subaddress_transfer(ctrl, buffer, buffer + sizeof(buffer), call);
		break;
	case PRI_SWITCH_QSIG:
		end =
			enc_qsig_subaddress_transfer(ctrl, buffer, buffer + sizeof(buffer), call);
		break;
	default:
		return -1;
	}
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL);
}

/*!
 * \brief Send a FACILITY SubaddressTransfer.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int send_subaddress_transfer(struct pri *ctrl, struct q931_call *call)
{
	if (rose_subaddress_transfer_encode(ctrl, call)
		|| q931_facility(ctrl, call)) {
		pri_message(ctrl,
			"Could not schedule facility message for subaddress transfer.\n");
		return -1;
	}

	return 0;
}

/*!
 * \internal
 * \brief Handle the received RequestSubaddress facility.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg
 *
 * \return Nothing
 */
static void etsi_request_subaddress(struct pri *ctrl, struct q931_call *call)
{
	struct q931_party_name name;
	int changed = 0;

	switch (call->notify) {
	case PRI_NOTIFY_TRANSFER_ACTIVE:
		if (q931_party_number_cmp(&call->remote_id.number, &call->redirection_number)) {
			/* The remote party number information changed. */
			call->remote_id.number = call->redirection_number;
			changed = 1;
		}
		/* Fall through */
	case PRI_NOTIFY_TRANSFER_ALERTING:
		if (ctrl->display_flags.receive & PRI_DISPLAY_OPTION_NAME_UPDATE) {
			if (q931_display_name_get(call, &name)) {
				if (q931_party_name_cmp(&call->remote_id.name, &name)) {
					/* The remote party name information changed. */
					call->remote_id.name = name;
					changed = 1;
				}
			}
		}
		if (call->redirection_number.valid
			&& q931_party_number_cmp(&call->remote_id.number, &call->redirection_number)) {
			/* The remote party number information changed. */
			call->remote_id.number = call->redirection_number;
			changed = 1;
		}
		if (call->remote_id.subaddress.valid) {
			/*
			 * Clear the subaddress as the remote party has been changed.
			 * Any new subaddress will arrive later.
			 */
			q931_party_subaddress_init(&call->remote_id.subaddress);
			changed = 1;
		}
		if (changed) {
			call->incoming_ct_state = INCOMING_CT_STATE_POST_CONNECTED_LINE;
		}
		break;
	default:
		break;
	}

	/* Send our subaddress back if we have one. */
	if (call->local_id.subaddress.valid) {
		send_subaddress_transfer(ctrl, call);
	}
}

/*!
 * \internal
 * \brief Handle the received SubaddressTransfer facility subaddress.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg
 * \param subaddr Received subaddress of remote party.
 *
 * \return Nothing
 */
static void handle_subaddress_transfer(struct pri *ctrl, struct q931_call *call, const struct rosePartySubaddress *subaddr)
{
	int changed = 0;
	struct q931_party_name name;
	struct q931_party_subaddress q931_subaddress;

	q931_party_subaddress_init(&q931_subaddress);
	rose_copy_subaddress_to_q931(ctrl, &q931_subaddress, subaddr);
	if (q931_party_subaddress_cmp(&call->remote_id.subaddress, &q931_subaddress)) {
		call->remote_id.subaddress = q931_subaddress;
		changed = 1;
	}
	if (call->redirection_number.valid
		&& q931_party_number_cmp(&call->remote_id.number, &call->redirection_number)) {
		/* The remote party number information changed. */
		call->remote_id.number = call->redirection_number;
		changed = 1;
	}
	if (ctrl->display_flags.receive & PRI_DISPLAY_OPTION_NAME_UPDATE) {
		if (q931_display_name_get(call, &name)) {
			if (q931_party_name_cmp(&call->remote_id.name, &name)) {
				/* The remote party name information changed. */
				call->remote_id.name = name;
				changed = 1;
			}
		}
	}
	if (changed) {
		call->incoming_ct_state = INCOMING_CT_STATE_POST_CONNECTED_LINE;
	}
}

/*!
 * \internal
 * \brief Encode a plain facility ETSI error code.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param call Call leg from which to encode error message response.
 * \param invoke_id Invoke id to put in error message response.
 * \param code Error code to put in error message response.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_error(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, q931_call *call, int invoke_id, enum rose_error_code code)
{
	struct rose_msg_error msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = invoke_id;
	msg.code = code;

	pos = rose_encode_error(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode a plain facility Q.SIG error code.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param call Call leg from which to encode error message response.
 * \param invoke_id Invoke id to put in error message response.
 * \param code Error code to put in error message response.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_qsig_error(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, q931_call *call, int invoke_id, enum rose_error_code code)
{
	struct fac_extension_header header;
	struct rose_msg_error msg;

	memset(&header, 0, sizeof(header));
	header.nfe_present = 1;
	header.nfe.source_entity = 0;	/* endPINX */
	header.nfe.destination_entity = 0;	/* endPINX */
	header.interpretation_present = 1;
	header.interpretation = 0;	/* discardAnyUnrecognisedInvokePdu */
	pos = facility_encode_header(ctrl, pos, end, &header);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = invoke_id;
	msg.code = code;

	pos = rose_encode_error(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \brief Encode and queue a plain facility error code.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode error message response.
 * \param msgtype Q.931 message type to put facility ie in.
 * \param invoke_id Invoke id to put in error message response.
 * \param code Error code to put in error message response.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int rose_error_msg_encode(struct pri *ctrl, q931_call *call, int msgtype, int invoke_id,
	enum rose_error_code code)
{
	unsigned char buffer[256];
	unsigned char *end;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		end =
			enc_etsi_error(ctrl, buffer, buffer + sizeof(buffer), call, invoke_id, code);
		break;
	case PRI_SWITCH_QSIG:
		end =
			enc_qsig_error(ctrl, buffer, buffer + sizeof(buffer), call, invoke_id, code);
		break;
	default:
		return -1;
	}
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, msgtype, buffer, end - buffer, NULL);
}

/*!
 * \brief Encode and send a plain facility error code.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode error message response.
 * \param invoke_id Invoke id to put in error message response.
 * \param code Error code to put in error message response.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int send_facility_error(struct pri *ctrl, q931_call *call, int invoke_id,
	enum rose_error_code code)
{
	if (rose_error_msg_encode(ctrl, call, Q931_FACILITY, invoke_id, code)
		|| q931_facility(ctrl, call)) {
		pri_message(ctrl,
			"Could not schedule facility message for error message.\n");
		return -1;
	}

	return 0;
}

/*!
 * \internal
 * \brief Encode a plain facility ETSI result ok.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param call Call leg from which to encode result ok message response.
 * \param invoke_id Invoke id to put in result ok message response.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_result_ok(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, q931_call *call, int invoke_id)
{
	struct rose_msg_result msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = invoke_id;
	msg.operation = ROSE_None;

	pos = rose_encode_result(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode a plain facility Q.SIG result ok.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param call Call leg from which to encode result ok message response.
 * \param invoke_id Invoke id to put in result ok message response.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_qsig_result_ok(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, q931_call *call, int invoke_id)
{
	struct fac_extension_header header;
	struct rose_msg_result msg;

	memset(&header, 0, sizeof(header));
	header.nfe_present = 1;
	header.nfe.source_entity = 0;	/* endPINX */
	header.nfe.destination_entity = 0;	/* endPINX */
	header.interpretation_present = 1;
	header.interpretation = 0;	/* discardAnyUnrecognisedInvokePdu */
	pos = facility_encode_header(ctrl, pos, end, &header);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = invoke_id;
	msg.operation = ROSE_None;

	pos = rose_encode_result(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \brief Encode and queue a plain ROSE result ok.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode result ok message response.
 * \param msgtype Q.931 message type to put facility ie in.
 * \param invoke_id Invoke id to put in result ok message response.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int rose_result_ok_encode(struct pri *ctrl, q931_call *call, int msgtype, int invoke_id)
{
	unsigned char buffer[256];
	unsigned char *end;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		end =
			enc_etsi_result_ok(ctrl, buffer, buffer + sizeof(buffer), call, invoke_id);
		break;
	case PRI_SWITCH_QSIG:
		end =
			enc_qsig_result_ok(ctrl, buffer, buffer + sizeof(buffer), call, invoke_id);
		break;
	default:
		return -1;
	}
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, msgtype, buffer, end - buffer, NULL);
}

/*!
 * \brief Encode and send a FACILITY message with a plain ROSE result ok.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode result ok message response.
 * \param invoke_id Invoke id to put in result ok message response.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int send_facility_result_ok(struct pri *ctrl, q931_call *call, int invoke_id)
{
	if (rose_result_ok_encode(ctrl, call, Q931_FACILITY, invoke_id)
		|| q931_facility(ctrl, call)) {
		pri_message(ctrl,
			"Could not schedule facility message for result OK message.\n");
		return -1;
	}

	return 0;
}

int pri_rerouting_rsp(struct pri *ctrl, q931_call *call, int invoke_id, enum PRI_REROUTING_RSP_CODE code)
{
	enum rose_error_code rose_err;

	if (!ctrl || !pri_is_call_valid(ctrl, call)) {
		return -1;
	}

	/* Convert the public rerouting response code to an error code or result ok. */
	rose_err = ROSE_ERROR_Gen_ResourceUnavailable;
	switch (code) {
	case PRI_REROUTING_RSP_OK_CLEAR:
		/*
		 * Send the response out on the next message which should be
		 * either Q931_DISCONNECT or Q931_RELEASE depending upon who
		 * initiates the disconnect first.
		 */
		return rose_result_ok_encode(ctrl, call, Q931_ANY_MESSAGE, invoke_id);
	case PRI_REROUTING_RSP_OK_RETAIN:
		return send_facility_result_ok(ctrl, call, invoke_id);
	case PRI_REROUTING_RSP_NOT_SUBSCRIBED:
		rose_err = ROSE_ERROR_Gen_NotSubscribed;
		break;
	case PRI_REROUTING_RSP_NOT_AVAILABLE:
		rose_err = ROSE_ERROR_Gen_NotAvailable;
		break;
	case PRI_REROUTING_RSP_NOT_ALLOWED:
		rose_err = ROSE_ERROR_Gen_SupplementaryServiceInteractionNotAllowed;
		break;
	case PRI_REROUTING_RSP_INVALID_NUMBER:
		rose_err = ROSE_ERROR_Div_InvalidDivertedToNr;
		break;
	case PRI_REROUTING_RSP_SPECIAL_SERVICE_NUMBER:
		rose_err = ROSE_ERROR_Div_SpecialServiceNr;
		break;
	case PRI_REROUTING_RSP_DIVERSION_TO_SELF:
		rose_err = ROSE_ERROR_Div_DiversionToServedUserNr;
		break;
	case PRI_REROUTING_RSP_MAX_DIVERSIONS_EXCEEDED:
		rose_err = ROSE_ERROR_Div_NumberOfDiversionsExceeded;
		break;
	case PRI_REROUTING_RSP_RESOURCE_UNAVAILABLE:
		rose_err = ROSE_ERROR_Gen_ResourceUnavailable;
		break;
	}
	return send_facility_error(ctrl, call, invoke_id, rose_err);
}

int pri_transfer_rsp(struct pri *ctrl, q931_call *call, int invoke_id, int is_successful)
{
	if (!ctrl || !pri_is_call_valid(ctrl, call)) {
		return -1;
	}

	if (is_successful) {
		return rose_result_ok_encode(ctrl, call, Q931_DISCONNECT, invoke_id);
	} else {
		return send_facility_error(ctrl, call, invoke_id, ROSE_ERROR_Gen_NotAvailable);
	}
}

/*!
 * \internal
 * \brief MCIDRequest response callback function.
 *
 * \param reason Reason callback is called.
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param apdu APDU queued entry.  Do not change!
 * \param msg APDU response message data.  (NULL if was not the reason called.)
 *
 * \return TRUE if no more responses are expected.
 */
static int mcid_req_response(enum APDU_CALLBACK_REASON reason, struct pri *ctrl, struct q931_call *call, struct apdu_event *apdu, const struct apdu_msg_data *msg)
{
	struct pri_subcommand *subcmd;
	int status;
	int fail_code;

	switch (reason) {
	case APDU_CALLBACK_REASON_TIMEOUT:
		status = 1;/* timeout */
		fail_code = 0;
		break;
	case APDU_CALLBACK_REASON_MSG_RESULT:
		status = 0;/* success */
		fail_code = 0;
		break;
	case APDU_CALLBACK_REASON_MSG_ERROR:
		status = 2;/* error */
		fail_code = msg->response.error->code;
		break;
	case APDU_CALLBACK_REASON_MSG_REJECT:
		status = 3;/* reject */
		fail_code = 0;
		fail_code = msg->response.reject->code;
		break;
	default:
		return 1;
	}
	subcmd = q931_alloc_subcommand(ctrl);
	if (subcmd) {
		/* Indicate that our MCID request has completed. */
		subcmd->cmd = PRI_SUBCMD_MCID_RSP;
		subcmd->u.mcid_rsp.status = status;
		subcmd->u.mcid_rsp.fail_code = fail_code;
	} else {
		/* Oh, well. */
	}
	return 1;
}

/*!
 * \internal
 * \brief Encode a MCIDRequest message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param call Call leg from which to encode message.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_mcid_req(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, q931_call *call)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = get_invokeid(ctrl);
	msg.operation = ROSE_ETSI_MCIDRequest;

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode and queue a MCID request message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode message.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_mcid_req_encode(struct pri *ctrl, q931_call *call)
{
	unsigned char buffer[256];
	unsigned char *end;
	struct apdu_callback_data response;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		end = enc_etsi_mcid_req(ctrl, buffer, buffer + sizeof(buffer), call);
		break;
	default:
		return -1;
	}
	if (!end) {
		return -1;
	}

	memset(&response, 0, sizeof(response));
	response.invoke_id = ctrl->last_invoke;
	response.timeout_time = ctrl->timers[PRI_TIMER_T_RESPONSE];
	response.callback = mcid_req_response;

	return pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, &response);
}

int pri_mcid_req_send(struct pri *ctrl, q931_call *call)
{
	if (!ctrl || !pri_is_call_valid(ctrl, call)) {
		return -1;
	}
	if (call->cc.originated) {
		/* We can only send MCID if we answered the call. */
		return -1;
	}

	if (rose_mcid_req_encode(ctrl, call) || q931_facility(ctrl, call)) {
		pri_message(ctrl,
			"Could not schedule facility message for MCID request message.\n");
		return -1;
	}

	return 0;
}

void pri_mcid_enable(struct pri *ctrl, int enable)
{
	if (ctrl) {
		ctrl->mcid_support = enable ? 1 : 0;
	}
}

/*!
 * \brief Handle the ROSE reject message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which the message came.
 * \param msgtype Q.931 message type ie is in.
 * \param ie Raw ie contents.
 * \param header Decoded facility header before ROSE.
 * \param reject Decoded ROSE reject message contents.
 *
 * \return Nothing
 */
void rose_handle_reject(struct pri *ctrl, q931_call *call, int msgtype, q931_ie *ie,
	const struct fac_extension_header *header, const struct rose_msg_reject *reject)
{
	q931_call *orig_call;
	struct apdu_event *apdu;
	struct apdu_msg_data msg;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		/* Gripe to the user about getting rejected. */
		pri_message(ctrl, "ROSE REJECT:\n");
		if (reject->invoke_id_present) {
			pri_message(ctrl, "\tINVOKE ID: %d\n", reject->invoke_id);
		}
		pri_message(ctrl, "\tPROBLEM: %s\n", rose_reject2str(reject->code));
	}

	switch (ctrl->switchtype) {
	case PRI_SWITCH_DMS100:
		/* The DMS-100 switch apparently handles invoke_id as an invoke operation. */
		return;
	default:
		break;
	}

	if (!reject->invoke_id_present) {
		/*
		 * No invoke id to look up so we cannot match it to any outstanding APDUs.
		 * This REJECT is apparently meant for someone monitoring the link.
		 */
		return;
	}
	orig_call = NULL;/* To make some compilers happy. */
	apdu = NULL;
	if (q931_is_dummy_call(call)) {
		/*
		 * The message was likely sent on the broadcast dummy call reference call
		 * and the reject came in on a specific dummy call reference call.
		 * Look for the original invocation message on the
		 * broadcast dummy call reference call first.
		 */
		orig_call = ctrl->link.dummy_call;
		if (orig_call) {
			apdu = pri_call_apdu_find(orig_call, reject->invoke_id);
		}
	}
	if (!apdu) {
		apdu = pri_call_apdu_find(call, reject->invoke_id);
		if (!apdu) {
			return;
		}
		orig_call = call;
	}
	msg.response.reject = reject;
	msg.type = msgtype;
	if (apdu->response.callback(APDU_CALLBACK_REASON_MSG_REJECT, ctrl, call, apdu, &msg)) {
		pri_call_apdu_delete(orig_call, apdu);
	}
}

/*!
 * \brief Handle the ROSE error message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which the message came.
 * \param msgtype Q.931 message type ie is in.
 * \param ie Raw ie contents.
 * \param header Decoded facility header before ROSE.
 * \param error Decoded ROSE error message contents.
 *
 * \return Nothing
 */
void rose_handle_error(struct pri *ctrl, q931_call *call, int msgtype, q931_ie *ie,
	const struct fac_extension_header *header, const struct rose_msg_error *error)
{
	const char *dms100_operation;
	q931_call *orig_call;
	struct apdu_event *apdu;
	struct apdu_msg_data msg;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		/* Gripe to the user about getting an error. */
		pri_message(ctrl, "ROSE RETURN ERROR:\n");
		switch (ctrl->switchtype) {
		case PRI_SWITCH_DMS100:
			switch (error->invoke_id) {
			case ROSE_DMS100_RLT_OPERATION_IND:
				dms100_operation = "RLT_OPERATION_IND";
				break;
			case ROSE_DMS100_RLT_THIRD_PARTY:
				dms100_operation = "RLT_THIRD_PARTY";
				break;
			default:
				dms100_operation = NULL;
				break;
			}
			if (dms100_operation) {
				pri_message(ctrl, "\tOPERATION: %s\n", dms100_operation);
				break;
			}
			/* fall through */
		default:
			pri_message(ctrl, "\tINVOKE ID: %d\n", error->invoke_id);
			break;
		}
		pri_message(ctrl, "\tERROR: %s\n", rose_error2str(error->code));
	}

	switch (ctrl->switchtype) {
	case PRI_SWITCH_DMS100:
		/* The DMS-100 switch apparently handles invoke_id as an invoke operation. */
		return;
	default:
		break;
	}

	orig_call = NULL;/* To make some compilers happy. */
	apdu = NULL;
	if (q931_is_dummy_call(call)) {
		/*
		 * The message was likely sent on the broadcast dummy call reference call
		 * and the error came in on a specific dummy call reference call.
		 * Look for the original invocation message on the
		 * broadcast dummy call reference call first.
		 */
		orig_call = ctrl->link.dummy_call;
		if (orig_call) {
			apdu = pri_call_apdu_find(orig_call, error->invoke_id);
		}
	}
	if (!apdu) {
		apdu = pri_call_apdu_find(call, error->invoke_id);
		if (!apdu) {
			return;
		}
		orig_call = call;
	}
	msg.response.error = error;
	msg.type = msgtype;
	if (apdu->response.callback(APDU_CALLBACK_REASON_MSG_ERROR, ctrl, call, apdu, &msg)) {
		pri_call_apdu_delete(orig_call, apdu);
	}
}

/*!
 * \brief Handle the ROSE result message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which the message came.
 * \param msgtype Q.931 message type ie is in.
 * \param ie Raw ie contents.
 * \param header Decoded facility header before ROSE.
 * \param result Decoded ROSE result message contents.
 *
 * \return Nothing
 */
void rose_handle_result(struct pri *ctrl, q931_call *call, int msgtype, q931_ie *ie,
	const struct fac_extension_header *header, const struct rose_msg_result *result)
{
	q931_call *orig_call;
	struct apdu_event *apdu;
	struct apdu_msg_data msg;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_DMS100:
		/* The DMS-100 switch apparently handles invoke_id as an invoke operation. */
		switch (result->invoke_id) {
		case ROSE_DMS100_RLT_OPERATION_IND:
			if (result->operation != ROSE_DMS100_RLT_OperationInd) {
				pri_message(ctrl, "Invalid Operation value in return result! %s\n",
					rose_operation2str(result->operation));
				break;
			}

			/* We have enough data to transfer the call */
			call->rlt_call_id = result->args.dms100.RLT_OperationInd.call_id;
			call->transferable = 1;
			break;
		case ROSE_DMS100_RLT_THIRD_PARTY:
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "Successfully completed RLT transfer!\n");
			}
			break;
		default:
			pri_message(ctrl, "Could not parse invoke of type %d!\n", result->invoke_id);
			break;
		}
		return;
	default:
		break;
	}

	orig_call = NULL;/* To make some compilers happy. */
	apdu = NULL;
	if (q931_is_dummy_call(call)) {
		/*
		 * The message was likely sent on the broadcast dummy call reference call
		 * and the result came in on a specific dummy call reference call.
		 * Look for the original invocation message on the
		 * broadcast dummy call reference call first.
		 */
		orig_call = ctrl->link.dummy_call;
		if (orig_call) {
			apdu = pri_call_apdu_find(orig_call, result->invoke_id);
		}
	}
	if (!apdu) {
		apdu = pri_call_apdu_find(call, result->invoke_id);
		if (!apdu) {
			return;
		}
		orig_call = call;
	}
	msg.response.result = result;
	msg.type = msgtype;
	if (apdu->response.callback(APDU_CALLBACK_REASON_MSG_RESULT, ctrl, call, apdu, &msg)) {
		pri_call_apdu_delete(orig_call, apdu);
	}
}

/*!
 * \brief Handle the ROSE invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which the message came.
 * \param msgtype Q.931 message type ie is in.
 * \param ie Raw ie contents.
 * \param header Decoded facility header before ROSE.
 * \param invoke Decoded ROSE invoke message contents.
 *
 * \return Nothing
 */
void rose_handle_invoke(struct pri *ctrl, q931_call *call, int msgtype, q931_ie *ie,
	const struct fac_extension_header *header, const struct rose_msg_invoke *invoke)
{
	struct pri_cc_record *cc_record;
	struct pri_subcommand *subcmd;
	struct q931_party_id party_id;
	struct q931_party_address party_address;
	struct q931_party_redirecting deflection;
	enum rose_error_code error_code;

	switch (invoke->operation) {
#if 0	/* Not handled yet */
	case ROSE_ETSI_ActivationDiversion:
		break;
	case ROSE_ETSI_DeactivationDiversion:
		break;
	case ROSE_ETSI_ActivationStatusNotificationDiv:
		break;
	case ROSE_ETSI_DeactivationStatusNotificationDiv:
		break;
	case ROSE_ETSI_InterrogationDiversion:
		break;
	case ROSE_ETSI_DiversionInformation:
		break;
#endif	/* Not handled yet */
	case ROSE_ETSI_CallDeflection:
		if (!ctrl->deflection_support) {
			send_facility_error(ctrl, call, invoke->invoke_id,
				ROSE_ERROR_Gen_NotSubscribed);
			break;
		}
		if (!q931_master_pass_event(ctrl, call, msgtype)) {
			/* Some other user is further along to connecting than this call. */
			send_facility_error(ctrl, call, invoke->invoke_id,
				ROSE_ERROR_Div_IncomingCallAccepted);
			break;
		}
		if (call->master_call->deflection_in_progress) {
			/* Someone else is already doing a call deflection. */
			send_facility_error(ctrl, call, invoke->invoke_id,
				ROSE_ERROR_Div_RequestAlreadyAccepted);
			break;
		}
		subcmd = q931_alloc_subcommand(ctrl);
		if (!subcmd) {
			/*
			 * ROSE_ERROR_Gen_ResourceUnavailable was not in the list of allowed codes,
			 * but we will send it anyway.
			 */
			send_facility_error(ctrl, call, invoke->invoke_id,
				ROSE_ERROR_Gen_ResourceUnavailable);
			break;
		}

		call->master_call->deflection_in_progress = 1;

		q931_party_redirecting_init(&deflection);

		/* Deflecting from the called address. */
		q931_party_address_to_id(&deflection.from, &call->called);
		if (invoke->args.etsi.CallDeflection.presentation_allowed_to_diverted_to_user_present) {
			deflection.from.number.presentation =
				invoke->args.etsi.CallDeflection.presentation_allowed_to_diverted_to_user
				? PRI_PRES_ALLOWED | PRI_PRES_USER_NUMBER_UNSCREENED
				: PRI_PRES_RESTRICTED | PRI_PRES_USER_NUMBER_UNSCREENED;
		} else {
			deflection.from.number.presentation =
				PRI_PRES_RESTRICTED | PRI_PRES_USER_NUMBER_UNSCREENED;
		}

		/* Deflecting to the new address. */
		rose_copy_address_to_id_q931(ctrl, &deflection.to,
			&invoke->args.etsi.CallDeflection.deflection);
		deflection.to.number.presentation = deflection.from.number.presentation;

		deflection.count = (call->redirecting.count < PRI_MAX_REDIRECTS)
			? call->redirecting.count + 1 : PRI_MAX_REDIRECTS;
		deflection.reason = PRI_REDIR_DEFLECTION;
		if (deflection.count == 1) {
			deflection.orig_called = deflection.from;
			deflection.orig_reason = deflection.reason;
		} else {
			deflection.orig_called = call->redirecting.orig_called;
			deflection.orig_reason = call->redirecting.orig_reason;
		}

		subcmd->cmd = PRI_SUBCMD_REROUTING;
		subcmd->u.rerouting.invoke_id = invoke->invoke_id;
		subcmd->u.rerouting.subscription_option = 3;/* notApplicable */
		q931_party_id_copy_to_pri(&subcmd->u.rerouting.caller, &call->local_id);
		q931_party_redirecting_copy_to_pri(&subcmd->u.rerouting.deflection,
			&deflection);
		break;
	case ROSE_ETSI_CallRerouting:
		if (!ctrl->deflection_support) {
			send_facility_error(ctrl, call, invoke->invoke_id,
				ROSE_ERROR_Gen_NotSubscribed);
			break;
		}
		subcmd = q931_alloc_subcommand(ctrl);
		if (!subcmd) {
			send_facility_error(ctrl, call, invoke->invoke_id,
				ROSE_ERROR_Gen_ResourceUnavailable);
			break;
		}

		q931_party_redirecting_init(&deflection);

		/* Rerouting from the last address. */
		rose_copy_presented_number_unscreened_to_q931(ctrl, &deflection.from.number,
			&invoke->args.etsi.CallRerouting.last_rerouting);

		/* Rerouting to the new address. */
		rose_copy_address_to_id_q931(ctrl, &deflection.to,
			&invoke->args.etsi.CallRerouting.called_address);
		switch (invoke->args.etsi.CallRerouting.subscription_option) {
		default:
		case 0: /* noNotification */
		case 1: /* notificationWithoutDivertedToNr */
			deflection.to.number.presentation =
				PRI_PRES_RESTRICTED | PRI_PRES_USER_NUMBER_UNSCREENED;
			break;
		case 2: /* notificationWithDivertedToNr */
			deflection.to.number.presentation =
				PRI_PRES_ALLOWED | PRI_PRES_USER_NUMBER_UNSCREENED;
			break;
		}

		/* Calling party subaddress update. */
		party_id = call->local_id;

		deflection.count = invoke->args.etsi.CallRerouting.rerouting_counter;
		deflection.reason = redirectingreason_for_q931(ctrl,
			invoke->args.etsi.CallRerouting.rerouting_reason);
		if (deflection.count == 1) {
			deflection.orig_called = deflection.from;
			deflection.orig_reason = deflection.reason;
		} else {
			deflection.orig_called = call->redirecting.orig_called;
			deflection.orig_reason = call->redirecting.orig_reason;
		}

		subcmd->cmd = PRI_SUBCMD_REROUTING;
		subcmd->u.rerouting.invoke_id = invoke->invoke_id;
		subcmd->u.rerouting.subscription_option =
			invoke->args.etsi.CallRerouting.subscription_option;
		q931_party_id_copy_to_pri(&subcmd->u.rerouting.caller, &party_id);
		q931_party_redirecting_copy_to_pri(&subcmd->u.rerouting.deflection,
			&deflection);
		break;
#if 0	/* Not handled yet */
	case ROSE_ETSI_InterrogateServedUserNumbers:
		break;
#endif	/* Not handled yet */
	case ROSE_ETSI_DivertingLegInformation1:
		if (invoke->args.etsi.DivertingLegInformation1.diverted_to_present) {
			rose_copy_presented_number_unscreened_to_q931(ctrl, &party_id.number,
				&invoke->args.etsi.DivertingLegInformation1.diverted_to);
			/*
			 * We set the presentation value since the sender cannot know the
			 * presentation value preference of the destination party.
			 */
			if (party_id.number.str[0]) {
				party_id.number.presentation =
					PRI_PRES_ALLOWED | PRI_PRES_USER_NUMBER_UNSCREENED;
			} else {
				party_id.number.presentation =
					PRI_PRES_UNAVAILABLE | PRI_PRES_USER_NUMBER_UNSCREENED;
			}
		} else {
			q931_party_number_init(&party_id.number);
			party_id.number.valid = 1;
		}

		/*
		 * Unless otherwise indicated by CONNECT, the divertedToNumber will be
		 * the remote_id.number.
		 */
		if (!call->connected_number_in_message) {
			call->remote_id.number = party_id.number;
		}

		/* divertedToNumber is put in redirecting.to.number */
		switch (invoke->args.etsi.DivertingLegInformation1.subscription_option) {
		default:
		case 0:	/* noNotification */
		case 1:	/* notificationWithoutDivertedToNr */
			q931_party_number_init(&call->redirecting.to.number);
			call->redirecting.to.number.valid = 1;
			call->redirecting.to.number.presentation =
				PRI_PRES_RESTRICTED | PRI_PRES_USER_NUMBER_UNSCREENED;
			break;
		case 2:	/* notificationWithDivertedToNr */
			call->redirecting.to.number = party_id.number;
			break;
		}

		call->redirecting.reason = redirectingreason_for_q931(ctrl,
			invoke->args.etsi.DivertingLegInformation1.diversion_reason);
		if (call->redirecting.count < PRI_MAX_REDIRECTS) {
			++call->redirecting.count;
		}
		call->redirecting.state = Q931_REDIRECTING_STATE_EXPECTING_RX_DIV_LEG_3;
		break;
	case ROSE_ETSI_DivertingLegInformation2:
		call->redirecting.state = Q931_REDIRECTING_STATE_PENDING_TX_DIV_LEG_3;
		call->redirecting.count =
			invoke->args.etsi.DivertingLegInformation2.diversion_counter;
		if (!call->redirecting.count) {
			/* To be safe, make sure that the count is non-zero. */
			call->redirecting.count = 1;
		}
		call->redirecting.reason = redirectingreason_for_q931(ctrl,
			invoke->args.etsi.DivertingLegInformation2.diversion_reason);

		/* divertingNr is put in redirecting.from.number */
		if (invoke->args.etsi.DivertingLegInformation2.diverting_present) {
			rose_copy_presented_number_unscreened_to_q931(ctrl,
				&call->redirecting.from.number,
				&invoke->args.etsi.DivertingLegInformation2.diverting);
		} else if (!call->redirecting_number_in_message) {
			q931_party_number_init(&call->redirecting.from.number);
			call->redirecting.from.number.valid = 1;
		}

		call->redirecting.orig_reason = PRI_REDIR_UNKNOWN;

		/* originalCalledNr is put in redirecting.orig_called.number */
		if (invoke->args.etsi.DivertingLegInformation2.original_called_present) {
			rose_copy_presented_number_unscreened_to_q931(ctrl,
				&call->redirecting.orig_called.number,
				&invoke->args.etsi.DivertingLegInformation2.original_called);
		} else {
			q931_party_number_init(&call->redirecting.orig_called.number);
		}
		break;
	case ROSE_ETSI_DivertingLegInformation3:
		/*
		 * Unless otherwise indicated by CONNECT, this will be the
		 * remote_id.number.presentation.
		 */
		if (!invoke->args.etsi.DivertingLegInformation3.presentation_allowed_indicator) {
			call->redirecting.to.number.presentation =
				PRI_PRES_RESTRICTED | PRI_PRES_USER_NUMBER_UNSCREENED;
			if (!call->connected_number_in_message) {
				call->remote_id.number.presentation =
					PRI_PRES_RESTRICTED | PRI_PRES_USER_NUMBER_UNSCREENED;
			}
		}

		switch (call->redirecting.state) {
		case Q931_REDIRECTING_STATE_EXPECTING_RX_DIV_LEG_3:
			call->redirecting.state = Q931_REDIRECTING_STATE_IDLE;
			subcmd = q931_alloc_subcommand(ctrl);
			if (!subcmd) {
				break;
			}
			/* Setup redirecting subcommand */
			subcmd->cmd = PRI_SUBCMD_REDIRECTING;
			q931_party_redirecting_copy_to_pri(&subcmd->u.redirecting,
				&call->redirecting);
			break;
		default:
			break;
		}
		break;
	case ROSE_ETSI_ChargingRequest:
		aoc_etsi_aoc_request(ctrl, call, invoke);
		break;
	case ROSE_ETSI_AOCSCurrency:
		aoc_etsi_aoc_s_currency(ctrl, invoke);
		break;
	case ROSE_ETSI_AOCSSpecialArr:
		aoc_etsi_aoc_s_special_arrangement(ctrl, invoke);
		break;
	case ROSE_ETSI_AOCDCurrency:
		aoc_etsi_aoc_d_currency(ctrl, invoke);
		break;
	case ROSE_ETSI_AOCDChargingUnit:
		aoc_etsi_aoc_d_charging_unit(ctrl, invoke);
		break;
	case ROSE_ETSI_AOCECurrency:
		aoc_etsi_aoc_e_currency(ctrl, call, invoke);
		break;
	case ROSE_ETSI_AOCEChargingUnit:
		aoc_etsi_aoc_e_charging_unit(ctrl, call, invoke);
		break;
#if 0	/* Not handled yet */
	case ROSE_ITU_IdentificationOfCharge:
		break;
#endif	/* Not handled yet */
	case ROSE_ETSI_EctExecute:
		if (!ctrl->transfer_support) {
			send_facility_error(ctrl, call, invoke->invoke_id,
				ROSE_ERROR_Gen_NotSubscribed);
			break;
		}
		error_code = etsi_ect_execute_transfer(ctrl, call, invoke->invoke_id);
		if (error_code != ROSE_ERROR_None) {
			send_facility_error(ctrl, call, invoke->invoke_id, error_code);
		}
		break;
	case ROSE_ETSI_ExplicitEctExecute:
		error_code = etsi_explicit_ect_execute_transfer(ctrl, call, invoke->invoke_id,
			invoke->args.etsi.ExplicitEctExecute.link_id);
		if (error_code != ROSE_ERROR_None) {
			send_facility_error(ctrl, call, invoke->invoke_id, error_code);
		}
		break;
	case ROSE_ETSI_RequestSubaddress:
		etsi_request_subaddress(ctrl, call);
		break;
	case ROSE_ETSI_SubaddressTransfer:
		handle_subaddress_transfer(ctrl, call,
			&invoke->args.etsi.SubaddressTransfer.subaddress);
		break;
	case ROSE_ETSI_EctLinkIdRequest:
		if (!ctrl->transfer_support) {
			send_facility_error(ctrl, call, invoke->invoke_id,
				ROSE_ERROR_Gen_ResourceUnavailable);
			break;
		}
		/*
		 * Use the invoke_id sequence number as a link_id.
		 * It should be safe enough to do this.  If not then we will have to search
		 * the call pool to ensure that the link_id is not already in use.
		 */
		call->master_call->link_id = get_invokeid(ctrl);
		call->master_call->is_link_id_valid = 1;
		send_ect_link_id_rsp(ctrl, call, invoke->invoke_id);
		break;
	case ROSE_ETSI_EctInform:
		if (ctrl->display_flags.receive & PRI_DISPLAY_OPTION_NAME_UPDATE) {
			q931_display_name_get(call, &call->remote_id.name);
		}

		/* redirectionNumber is put in remote_id.number */
		if (invoke->args.etsi.EctInform.redirection_present) {
			rose_copy_presented_number_unscreened_to_q931(ctrl,
				&call->remote_id.number, &invoke->args.etsi.EctInform.redirection);
		}

		/*
		 * Clear the subaddress as the remote party has been changed.
		 * Any new subaddress will arrive later.
		 */
		q931_party_subaddress_init(&call->remote_id.subaddress);

		if (!invoke->args.etsi.EctInform.status) {
			/* The remote party for the transfer has not answered yet. */
			call->incoming_ct_state = INCOMING_CT_STATE_EXPECT_CT_ACTIVE;
		} else {
			call->incoming_ct_state = INCOMING_CT_STATE_POST_CONNECTED_LINE;
		}

		/* Send our subaddress back if we have one. */
		if (call->local_id.subaddress.valid) {
			send_subaddress_transfer(ctrl, call);
		}
		break;
	case ROSE_ETSI_EctLoopTest:
		/*
		 * The ETS 300 369 specification does a very poor job describing
		 * how this message is used to detect loops.
		 */
		send_facility_error(ctrl, call, invoke->invoke_id, ROSE_ERROR_Gen_NotAvailable);
		break;
#if defined(STATUS_REQUEST_PLACE_HOLDER)
	case ROSE_ETSI_StatusRequest:
		/* Not handled yet */
		break;
#endif	/* defined(STATUS_REQUEST_PLACE_HOLDER) */
	case ROSE_ETSI_CallInfoRetain:
		if (!ctrl->cc_support) {
			/*
			 * Blocking the cc-available event effectively
			 * disables call completion for outgoing calls.
			 */
			break;
		}
		if (call->cc.record) {
			/* Duplicate message!  Should not happen. */
			break;
		}
		cc_record = pri_cc_new_record(ctrl, call);
		if (!cc_record) {
			break;
		}
		cc_record->signaling = ctrl->link.dummy_call;
		/*
		 * Since we received this facility, we will not be allocating any
		 * reference and linkage id's.
		 */
		cc_record->call_linkage_id =
			invoke->args.etsi.CallInfoRetain.call_linkage_id & 0x7F;
		cc_record->original_call = call;
		call->cc.record = cc_record;
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_AVAILABLE);
		break;
	case ROSE_ETSI_CCBSRequest:
		pri_cc_ptmp_request(ctrl, call, invoke);
		break;
	case ROSE_ETSI_CCNRRequest:
		pri_cc_ptmp_request(ctrl, call, invoke);
		break;
	case ROSE_ETSI_CCBSDeactivate:
		cc_record = pri_cc_find_by_reference(ctrl,
			invoke->args.etsi.CCBSDeactivate.ccbs_reference);
		if (!cc_record) {
			send_facility_error(ctrl, call, invoke->invoke_id,
				ROSE_ERROR_CCBS_InvalidCCBSReference);
			break;
		}
		send_facility_result_ok(ctrl, call, invoke->invoke_id);
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_LINK_CANCEL);
		break;
	case ROSE_ETSI_CCBSInterrogate:
		pri_cc_interrogate_rsp(ctrl, call, invoke);
		break;
	case ROSE_ETSI_CCNRInterrogate:
		pri_cc_interrogate_rsp(ctrl, call, invoke);
		break;
	case ROSE_ETSI_CCBSErase:
		cc_record = pri_cc_find_by_reference(ctrl,
			invoke->args.etsi.CCBSErase.ccbs_reference);
		if (!cc_record) {
			/*
			 * Ignore any status requests that we do not have a record.
			 * We will not participate in any CC requests that we did
			 * not initiate.
			 */
			break;
		}
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_LINK_CANCEL);
		break;
	case ROSE_ETSI_CCBSRemoteUserFree:
		cc_record = pri_cc_find_by_reference(ctrl,
			invoke->args.etsi.CCBSRemoteUserFree.ccbs_reference);
		if (!cc_record) {
			/*
			 * Ignore any status requests that we do not have a record.
			 * We will not participate in any CC requests that we did
			 * not initiate.
			 */
			break;
		}
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_REMOTE_USER_FREE);
		break;
	case ROSE_ETSI_CCBSCall:
		cc_record = pri_cc_find_by_reference(ctrl,
			invoke->args.etsi.CCBSCall.ccbs_reference);
		if (!cc_record) {
			rose_error_msg_encode(ctrl, call, Q931_ANY_MESSAGE, invoke->invoke_id,
				ROSE_ERROR_CCBS_InvalidCCBSReference);
			call->cc.hangup_call = 1;
			break;
		}

		/* Save off data to know how to send back any response. */
		cc_record->response.signaling = call;
		cc_record->response.invoke_operation = invoke->operation;
		cc_record->response.invoke_id = invoke->invoke_id;

		pri_cc_event(ctrl, call, cc_record, CC_EVENT_RECALL);
		break;
	case ROSE_ETSI_CCBSStatusRequest:
		cc_record = pri_cc_find_by_reference(ctrl,
			invoke->args.etsi.CCBSStatusRequest.ccbs_reference);
		if (!cc_record) {
			/*
			 * Ignore any status requests that we do not have a record.
			 * We will not participate in any CC requests that we did
			 * not initiate.
			 */
			break;
		}

		/* Save off data to know how to send back any response. */
		//cc_record->response.signaling = call;
		cc_record->response.invoke_operation = invoke->operation;
		cc_record->response.invoke_id = invoke->invoke_id;

		subcmd = q931_alloc_subcommand(ctrl);
		if (!subcmd) {
			break;
		}

		subcmd->cmd = PRI_SUBCMD_CC_STATUS_REQ;
		subcmd->u.cc_status_req.cc_id = cc_record->record_id;
		break;
	case ROSE_ETSI_CCBSBFree:
		cc_record = pri_cc_find_by_reference(ctrl,
			invoke->args.etsi.CCBSBFree.ccbs_reference);
		if (!cc_record) {
			/*
			 * Ignore any status requests that we do not have a record.
			 * We will not participate in any CC requests that we did
			 * not initiate.
			 */
			break;
		}
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_B_FREE);
		break;
	case ROSE_ETSI_EraseCallLinkageID:
		cc_record = pri_cc_find_by_linkage(ctrl,
			invoke->args.etsi.EraseCallLinkageID.call_linkage_id);
		if (!cc_record) {
			/*
			 * Ignore any status requests that we do not have a record.
			 * We will not participate in any CC requests that we did
			 * not initiate.
			 */
			break;
		}
		/*
		 * T_RETENTION expired on the network side so we will pretend
		 * that it expired on our side.
		 */
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_TIMEOUT_T_RETENTION);
		break;
	case ROSE_ETSI_CCBSStopAlerting:
		cc_record = pri_cc_find_by_reference(ctrl,
			invoke->args.etsi.CCBSStopAlerting.ccbs_reference);
		if (!cc_record) {
			/*
			 * Ignore any status requests that we do not have a record.
			 * We will not participate in any CC requests that we did
			 * not initiate.
			 */
			break;
		}
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_STOP_ALERTING);
		break;
	case ROSE_ETSI_CCBS_T_Request:
		pri_cc_ptp_request(ctrl, call, msgtype, invoke);
		break;
	case ROSE_ETSI_CCNR_T_Request:
		pri_cc_ptp_request(ctrl, call, msgtype, invoke);
		break;
	case ROSE_ETSI_CCBS_T_Call:
		if (msgtype != Q931_SETUP) {
			/* Ignore since it did not come in on the correct message. */
			break;
		}

		/*
		 * If we cannot find the cc_record we should still pass up the
		 * CC call indication but with a -1 for the cc_id.
		 * The upper layer would then need to search its records for a
		 * matching CC.  The call may have come in on a different interface.
		 */
		q931_party_id_to_address(&party_address, &call->remote_id);
		cc_record = pri_cc_find_by_addressing(ctrl, &party_address, &call->called,
			call->cc.saved_ie_contents.length, call->cc.saved_ie_contents.data);
		if (cc_record) {
			pri_cc_event(ctrl, call, cc_record, CC_EVENT_RECALL);
		} else {
			subcmd = q931_alloc_subcommand(ctrl);
			if (!subcmd) {
				break;
			}

			subcmd->cmd = PRI_SUBCMD_CC_CALL;
			subcmd->u.cc_call.cc_id = -1;
		}
		break;
	case ROSE_ETSI_CCBS_T_Suspend:
		cc_record = call->cc.record;
		if (!cc_record) {
			break;
		}
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_SUSPEND);
		break;
	case ROSE_ETSI_CCBS_T_Resume:
		cc_record = call->cc.record;
		if (!cc_record) {
			break;
		}
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_RESUME);
		break;
	case ROSE_ETSI_CCBS_T_RemoteUserFree:
		cc_record = call->cc.record;
		if (!cc_record) {
			break;
		}
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_REMOTE_USER_FREE);
		break;
	case ROSE_ETSI_CCBS_T_Available:
		if (!ctrl->cc_support) {
			/*
			 * Blocking the cc-available event effectively
			 * disables call completion for outgoing calls.
			 */
			break;
		}
		if (call->cc.record) {
			/* Duplicate message!  Should not happen. */
			break;
		}
		cc_record = pri_cc_new_record(ctrl, call);
		if (!cc_record) {
			break;
		}
		cc_record->original_call = call;
		call->cc.record = cc_record;
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_AVAILABLE);
		break;
	case ROSE_ETSI_MCIDRequest:
		if (q931_is_dummy_call(call)) {
			/* Don't even dignify this with a response. */
			break;
		}
		if (!ctrl->mcid_support) {
			send_facility_error(ctrl, call, invoke->invoke_id,
				ROSE_ERROR_Gen_NotSubscribed);
			break;
		}
		if (!call->cc.originated) {
			send_facility_error(ctrl, call, invoke->invoke_id,
				ROSE_ERROR_Gen_NotIncomingCall);
			break;
		}
		switch (call->ourcallstate) {
		case Q931_CALL_STATE_ACTIVE:
		case Q931_CALL_STATE_DISCONNECT_INDICATION:
		case Q931_CALL_STATE_DISCONNECT_REQUEST:/* XXX We are really in the wrong state for this mode. */
			subcmd = q931_alloc_subcommand(ctrl);
			if (!subcmd) {
				send_facility_error(ctrl, call, invoke->invoke_id,
					ROSE_ERROR_Gen_NotAvailable);
				break;
			}

			subcmd->cmd = PRI_SUBCMD_MCID_REQ;
			q931_party_id_copy_to_pri(&subcmd->u.mcid_req.originator, &call->local_id);
			q931_party_id_copy_to_pri(&subcmd->u.mcid_req.answerer, &call->remote_id);

			send_facility_result_ok(ctrl, call, invoke->invoke_id);
			break;
		default:
			send_facility_error(ctrl, call, invoke->invoke_id,
				ROSE_ERROR_Gen_InvalidCallState);
			break;
		}
		break;
#if 0	/* Not handled yet */
	case ROSE_ETSI_MWIActivate:
		break;
	case ROSE_ETSI_MWIDeactivate:
		break;
	case ROSE_ETSI_MWIIndicate:
		break;
#endif	/* Not handled yet */
	case ROSE_QSIG_CallingName:
		if (ctrl->display_flags.receive & PRI_DISPLAY_OPTION_NAME_UPDATE) {
			q931_display_name_get(call, &call->remote_id.name);
		}

		/* CallingName is put in remote_id.name */
		rose_copy_name_to_q931(ctrl, &call->remote_id.name,
			&invoke->args.qsig.CallingName.name);

		switch (msgtype) {
		case Q931_SETUP:
		case Q931_CONNECT:
			/* The caller name will automatically be reported. */
			break;
		default:
			/* Setup connected line subcommand */
			subcmd = q931_alloc_subcommand(ctrl);
			if (!subcmd) {
				break;
			}
			subcmd->cmd = PRI_SUBCMD_CONNECTED_LINE;
			q931_party_id_copy_to_pri(&subcmd->u.connected_line.id, &call->remote_id);
			break;
		}
		break;
	case ROSE_QSIG_CalledName:
		if (ctrl->display_flags.receive & PRI_DISPLAY_OPTION_NAME_UPDATE) {
			q931_display_name_get(call, &call->remote_id.name);
		}

		/* CalledName is put in remote_id.name */
		rose_copy_name_to_q931(ctrl, &call->remote_id.name,
			&invoke->args.qsig.CalledName.name);

		switch (msgtype) {
		case Q931_SETUP:
		case Q931_CONNECT:
			/* The called name will automatically be reported. */
			break;
		default:
			/* Setup connected line subcommand */
			subcmd = q931_alloc_subcommand(ctrl);
			if (!subcmd) {
				break;
			}
			subcmd->cmd = PRI_SUBCMD_CONNECTED_LINE;
			q931_party_id_copy_to_pri(&subcmd->u.connected_line.id, &call->remote_id);
			break;
		}
		break;
	case ROSE_QSIG_ConnectedName:
		if (ctrl->display_flags.receive & PRI_DISPLAY_OPTION_NAME_UPDATE) {
			q931_display_name_get(call, &call->remote_id.name);
		}

		/* ConnectedName is put in remote_id.name */
		rose_copy_name_to_q931(ctrl, &call->remote_id.name,
			&invoke->args.qsig.ConnectedName.name);

		switch (msgtype) {
		case Q931_SETUP:
		case Q931_CONNECT:
			/* The connected line name will automatically be reported. */
			break;
		default:
			/* Setup connected line subcommand */
			subcmd = q931_alloc_subcommand(ctrl);
			if (!subcmd) {
				break;
			}
			subcmd->cmd = PRI_SUBCMD_CONNECTED_LINE;
			q931_party_id_copy_to_pri(&subcmd->u.connected_line.id, &call->remote_id);
			break;
		}
		break;
#if 0	/* Not handled yet */
	case ROSE_QSIG_BusyName:
		break;
#endif	/* Not handled yet */
#if 0	/* Not handled yet */
	case ROSE_QSIG_ChargeRequest:
		break;
	case ROSE_QSIG_GetFinalCharge:
		break;
	case ROSE_QSIG_AocFinal:
		break;
	case ROSE_QSIG_AocInterim:
		break;
	case ROSE_QSIG_AocRate:
		break;
	case ROSE_QSIG_AocComplete:
		break;
	case ROSE_QSIG_AocDivChargeReq:
		break;
#endif	/* Not handled yet */
#if 0	/* Not handled yet */
	case ROSE_QSIG_CallTransferIdentify:
		break;
	case ROSE_QSIG_CallTransferAbandon:
		break;
	case ROSE_QSIG_CallTransferInitiate:
		break;
	case ROSE_QSIG_CallTransferSetup:
		break;
#endif	/* Not handled yet */
	case ROSE_QSIG_CallTransferActive:
		if (ctrl->display_flags.receive & PRI_DISPLAY_OPTION_NAME_UPDATE) {
			q931_display_name_get(call, &call->remote_id.name);
		}

		call->incoming_ct_state = INCOMING_CT_STATE_POST_CONNECTED_LINE;

		/* connectedAddress is put in remote_id */
		rose_copy_presented_address_screened_to_id_q931(ctrl, &call->remote_id,
			&invoke->args.qsig.CallTransferActive.connected);

		/* connectedName is put in remote_id.name */
		if (invoke->args.qsig.CallTransferActive.connected_name_present) {
			rose_copy_name_to_q931(ctrl, &call->remote_id.name,
				&invoke->args.qsig.CallTransferActive.connected_name);
		}
		break;
	case ROSE_QSIG_CallTransferComplete:
		if (ctrl->display_flags.receive & PRI_DISPLAY_OPTION_NAME_UPDATE) {
			q931_display_name_get(call, &call->remote_id.name);
		}

		/* redirectionNumber is put in remote_id.number */
		rose_copy_presented_number_screened_to_q931(ctrl, &call->remote_id.number,
			&invoke->args.qsig.CallTransferComplete.redirection);

		/* redirectionName is put in remote_id.name */
		if (invoke->args.qsig.CallTransferComplete.redirection_name_present) {
			rose_copy_name_to_q931(ctrl, &call->remote_id.name,
				&invoke->args.qsig.CallTransferComplete.redirection_name);
		}

		/*
		 * Clear the subaddress as the remote party has been changed.
		 * Any new subaddress will arrive later.
		 */
		q931_party_subaddress_init(&call->remote_id.subaddress);

		if (invoke->args.qsig.CallTransferComplete.call_status == 1) {
			/* The remote party for the transfer has not answered yet. */
			call->incoming_ct_state = INCOMING_CT_STATE_EXPECT_CT_ACTIVE;
		} else {
			call->incoming_ct_state = INCOMING_CT_STATE_POST_CONNECTED_LINE;
		}

		/* Send our subaddress back if we have one. */
		if (call->local_id.subaddress.valid) {
			send_subaddress_transfer(ctrl, call);
		}
		break;
	case ROSE_QSIG_CallTransferUpdate:
		party_id = call->remote_id;

		if (ctrl->display_flags.receive & PRI_DISPLAY_OPTION_NAME_UPDATE) {
			q931_display_name_get(call, &party_id.name);
		}

		/* redirectionNumber is put in party_id.number */
		rose_copy_presented_number_screened_to_q931(ctrl, &party_id.number,
			&invoke->args.qsig.CallTransferUpdate.redirection);

		/* redirectionName is put in party_id.name */
		if (invoke->args.qsig.CallTransferUpdate.redirection_name_present) {
			rose_copy_name_to_q931(ctrl, &party_id.name,
				&invoke->args.qsig.CallTransferUpdate.redirection_name);
		}

		if (q931_party_id_cmp(&party_id, &call->remote_id)) {
			/* The remote_id data has changed. */
			call->remote_id = party_id;
			switch (call->incoming_ct_state) {
			case INCOMING_CT_STATE_IDLE:
				call->incoming_ct_state = INCOMING_CT_STATE_POST_CONNECTED_LINE;
				break;
			default:
				break;
			}
		}
		break;
	case ROSE_QSIG_SubaddressTransfer:
		handle_subaddress_transfer(ctrl, call,
			&invoke->args.qsig.SubaddressTransfer.redirection_subaddress);
		break;
	case ROSE_QSIG_PathReplacement:
		anfpr_pathreplacement_respond(ctrl, call, ie);
		break;
#if 0	/* Not handled yet */
	case ROSE_QSIG_ActivateDiversionQ:
		break;
	case ROSE_QSIG_DeactivateDiversionQ:
		break;
	case ROSE_QSIG_InterrogateDiversionQ:
		break;
	case ROSE_QSIG_CheckRestriction:
		break;
#endif	/* Not handled yet */
	case ROSE_QSIG_CallRerouting:
		if (!ctrl->deflection_support) {
			send_facility_error(ctrl, call, invoke->invoke_id,
				ROSE_ERROR_Gen_NotSubscribed);
			break;
		}
		subcmd = q931_alloc_subcommand(ctrl);
		if (!subcmd) {
			send_facility_error(ctrl, call, invoke->invoke_id,
				ROSE_ERROR_Gen_ResourceUnavailable);
			break;
		}

		q931_party_redirecting_init(&deflection);

		/* Rerouting from the last address. */
		rose_copy_presented_number_unscreened_to_q931(ctrl, &deflection.from.number,
			&invoke->args.qsig.CallRerouting.last_rerouting);
		if (invoke->args.qsig.CallRerouting.redirecting_name_present) {
			rose_copy_name_to_q931(ctrl, &deflection.from.name,
				&invoke->args.qsig.CallRerouting.redirecting_name);
		}

		/* Rerouting to the new address. */
		rose_copy_address_to_id_q931(ctrl, &deflection.to,
			&invoke->args.qsig.CallRerouting.called);
		switch (invoke->args.qsig.CallRerouting.subscription_option) {
		default:
		case 0: /* noNotification */
		case 1: /* notificationWithoutDivertedToNr */
			deflection.to.number.presentation =
				PRI_PRES_RESTRICTED | PRI_PRES_USER_NUMBER_UNSCREENED;
			break;
		case 2: /* notificationWithDivertedToNr */
			deflection.to.number.presentation =
				PRI_PRES_ALLOWED | PRI_PRES_USER_NUMBER_UNSCREENED;
			break;
		}

		/* Calling party update. */
		party_id = call->local_id;
		rose_copy_presented_number_screened_to_q931(ctrl, &party_id.number,
			&invoke->args.qsig.CallRerouting.calling);
		if (invoke->args.qsig.CallRerouting.calling_name_present) {
			rose_copy_name_to_q931(ctrl, &party_id.name,
				&invoke->args.qsig.CallRerouting.calling_name);
		}

		deflection.count = invoke->args.qsig.CallRerouting.diversion_counter;
		deflection.reason = redirectingreason_for_q931(ctrl,
			invoke->args.qsig.CallRerouting.rerouting_reason);

		/* Original called party update. */
		if (deflection.count == 1) {
			deflection.orig_called = deflection.from;
			deflection.orig_reason = deflection.reason;
		} else {
			deflection.orig_called = call->redirecting.orig_called;
			deflection.orig_reason = call->redirecting.orig_reason;
		}
		if (invoke->args.qsig.CallRerouting.original_called_present) {
			rose_copy_presented_number_unscreened_to_q931(ctrl,
				&deflection.orig_called.number,
				&invoke->args.qsig.CallRerouting.original_called);
		}
		if (invoke->args.qsig.CallRerouting.original_called_name_present) {
			rose_copy_name_to_q931(ctrl, &deflection.orig_called.name,
				&invoke->args.qsig.CallRerouting.original_called_name);
		}
		if (invoke->args.qsig.CallRerouting.original_rerouting_reason_present) {
			deflection.orig_reason = redirectingreason_for_q931(ctrl,
				invoke->args.qsig.CallRerouting.original_rerouting_reason);
		}

		subcmd->cmd = PRI_SUBCMD_REROUTING;
		subcmd->u.rerouting.invoke_id = invoke->invoke_id;
		subcmd->u.rerouting.subscription_option =
			invoke->args.qsig.CallRerouting.subscription_option;
		q931_party_id_copy_to_pri(&subcmd->u.rerouting.caller, &party_id);
		q931_party_redirecting_copy_to_pri(&subcmd->u.rerouting.deflection,
			&deflection);
		break;
	case ROSE_QSIG_DivertingLegInformation1:
		q931_party_number_init(&party_id.number);
		rose_copy_number_to_q931(ctrl, &party_id.number,
			&invoke->args.qsig.DivertingLegInformation1.nominated_number);
		if (party_id.number.str[0]) {
			party_id.number.presentation =
				PRI_PRES_ALLOWED | PRI_PRES_USER_NUMBER_UNSCREENED;
		}

		/*
		 * Unless otherwise indicated by CONNECT, the nominatedNr will be
		 * the remote_id.number.
		 */
		if (!call->connected_number_in_message) {
			call->remote_id.number = party_id.number;
		}

		/* nominatedNr is put in redirecting.to.number */
		switch (invoke->args.qsig.DivertingLegInformation1.subscription_option) {
		default:
		case QSIG_NO_NOTIFICATION:
		case QSIG_NOTIFICATION_WITHOUT_DIVERTED_TO_NR:
			q931_party_number_init(&call->redirecting.to.number);
			call->redirecting.to.number.valid = 1;
			call->redirecting.to.number.presentation =
				PRI_PRES_RESTRICTED | PRI_PRES_USER_NUMBER_UNSCREENED;
			break;
		case QSIG_NOTIFICATION_WITH_DIVERTED_TO_NR:
			call->redirecting.to.number = party_id.number;
			break;
		}

		call->redirecting.reason = redirectingreason_for_q931(ctrl,
			invoke->args.qsig.DivertingLegInformation1.diversion_reason);
		if (call->redirecting.count < PRI_MAX_REDIRECTS) {
			++call->redirecting.count;
		}
		call->redirecting.state = Q931_REDIRECTING_STATE_EXPECTING_RX_DIV_LEG_3;
		break;
	case ROSE_QSIG_DivertingLegInformation2:
		call->redirecting.state = Q931_REDIRECTING_STATE_PENDING_TX_DIV_LEG_3;
		call->redirecting.count =
			invoke->args.qsig.DivertingLegInformation2.diversion_counter;
		if (!call->redirecting.count) {
			/* To be safe, make sure that the count is non-zero. */
			call->redirecting.count = 1;
		}
		call->redirecting.reason = redirectingreason_for_q931(ctrl,
			invoke->args.qsig.DivertingLegInformation2.diversion_reason);

		/* divertingNr is put in redirecting.from.number */
		if (invoke->args.qsig.DivertingLegInformation2.diverting_present) {
			rose_copy_presented_number_unscreened_to_q931(ctrl,
				&call->redirecting.from.number,
				&invoke->args.qsig.DivertingLegInformation2.diverting);
		} else if (!call->redirecting_number_in_message) {
			q931_party_number_init(&call->redirecting.from.number);
			call->redirecting.from.number.valid = 1;
		}

		/* redirectingName is put in redirecting.from.name */
		if (invoke->args.qsig.DivertingLegInformation2.redirecting_name_present) {
			rose_copy_name_to_q931(ctrl, &call->redirecting.from.name,
				&invoke->args.qsig.DivertingLegInformation2.redirecting_name);
		} else {
			q931_party_name_init(&call->redirecting.from.name);
		}

		call->redirecting.orig_reason = PRI_REDIR_UNKNOWN;
		if (invoke->args.qsig.DivertingLegInformation2.original_diversion_reason_present) {
			call->redirecting.orig_reason = redirectingreason_for_q931(ctrl,
				invoke->args.qsig.DivertingLegInformation2.original_diversion_reason);
		}

		/* originalCalledNr is put in redirecting.orig_called.number */
		if (invoke->args.qsig.DivertingLegInformation2.original_called_present) {
			rose_copy_presented_number_unscreened_to_q931(ctrl,
				&call->redirecting.orig_called.number,
				&invoke->args.qsig.DivertingLegInformation2.original_called);
		} else {
			q931_party_number_init(&call->redirecting.orig_called.number);
		}

		/* originalCalledName is put in redirecting.orig_called.name */
		if (invoke->args.qsig.DivertingLegInformation2.original_called_name_present) {
			rose_copy_name_to_q931(ctrl, &call->redirecting.orig_called.name,
				&invoke->args.qsig.DivertingLegInformation2.original_called_name);
		} else {
			q931_party_name_init(&call->redirecting.orig_called.name);
		}
		break;
	case ROSE_QSIG_DivertingLegInformation3:
		/*
		 * Unless otherwise indicated by CONNECT, this will be the
		 * remote_id.number.presentation.
		 */
		if (!invoke->args.qsig.DivertingLegInformation3.presentation_allowed_indicator) {
			call->redirecting.to.number.presentation =
				PRI_PRES_RESTRICTED | PRI_PRES_USER_NUMBER_UNSCREENED;
			if (!call->connected_number_in_message) {
				call->remote_id.number.presentation =
					PRI_PRES_RESTRICTED | PRI_PRES_USER_NUMBER_UNSCREENED;
			}
		}

		/* redirectionName is put in redirecting.to.name */
		if (invoke->args.qsig.DivertingLegInformation3.redirection_name_present) {
			rose_copy_name_to_q931(ctrl, &call->redirecting.to.name,
				&invoke->args.qsig.DivertingLegInformation3.redirection_name);
			if (!invoke->args.qsig.DivertingLegInformation3.presentation_allowed_indicator) {
				call->redirecting.to.name.presentation = PRI_PRES_RESTRICTED;
			}
		} else {
			q931_party_name_init(&call->redirecting.to.name);
		}

		switch (call->redirecting.state) {
		case Q931_REDIRECTING_STATE_EXPECTING_RX_DIV_LEG_3:
			call->redirecting.state = Q931_REDIRECTING_STATE_IDLE;
			subcmd = q931_alloc_subcommand(ctrl);
			if (!subcmd) {
				break;
			}
			/* Setup redirecting subcommand */
			subcmd->cmd = PRI_SUBCMD_REDIRECTING;
			q931_party_redirecting_copy_to_pri(&subcmd->u.redirecting,
				&call->redirecting);
			break;
		default:
			break;
		}
		break;
#if 0	/* Not handled yet */
	case ROSE_QSIG_CfnrDivertedLegFailed:
		break;
#endif	/* Not handled yet */
	case ROSE_QSIG_CcbsRequest:
		pri_cc_qsig_request(ctrl, call, msgtype, invoke);
		break;
	case ROSE_QSIG_CcnrRequest:
		pri_cc_qsig_request(ctrl, call, msgtype, invoke);
		break;
	case ROSE_QSIG_CcCancel:
		pri_cc_qsig_cancel(ctrl, call, msgtype, invoke);
		break;
	case ROSE_QSIG_CcExecPossible:
		pri_cc_qsig_exec_possible(ctrl, call, msgtype, invoke);
		break;
	case ROSE_QSIG_CcPathReserve:
		/*!
		 * \todo It may be possible for us to accept the ccPathReserve call.
		 * We could certainly never initiate it.
		 */
		rose_error_msg_encode(ctrl, call, Q931_ANY_MESSAGE, invoke->invoke_id,
			ROSE_ERROR_QSIG_FailedDueToInterworking);
		call->cc.hangup_call = 1;
		break;
	case ROSE_QSIG_CcRingout:
		if (msgtype != Q931_SETUP) {
			/*
			 * Ignore since it did not come in on the correct message.
			 *
			 * It could come in on a FACILITY message if we supported
			 * incoming ccPathReserve calls.
			 */
			break;
		}

		q931_party_id_to_address(&party_address, &call->remote_id);
		cc_record = pri_cc_find_by_addressing(ctrl, &party_address, &call->called,
			call->cc.saved_ie_contents.length, call->cc.saved_ie_contents.data);
		if (cc_record) {
			/* Save off data to know how to send back any response. */
			cc_record->response.signaling = call;
			cc_record->response.invoke_operation = invoke->operation;
			cc_record->response.invoke_id = invoke->invoke_id;

			pri_cc_event(ctrl, call, cc_record, CC_EVENT_RECALL);
		} else {
			rose_error_msg_encode(ctrl, call, Q931_ANY_MESSAGE, invoke->invoke_id,
				ROSE_ERROR_QSIG_FailureToMatch);
			call->cc.hangup_call = 1;
		}
		break;
	case ROSE_QSIG_CcSuspend:
		cc_record = call->cc.record;
		if (!cc_record) {
			break;
		}
		cc_record->fsm.qsig.msgtype = msgtype;
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_SUSPEND);
		break;
	case ROSE_QSIG_CcResume:
		cc_record = call->cc.record;
		if (!cc_record) {
			break;
		}
		cc_record->fsm.qsig.msgtype = msgtype;
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_RESUME);
		break;
#if 0	/* Not handled yet */
	case ROSE_QSIG_MWIActivate:
		break;
	case ROSE_QSIG_MWIDeactivate:
		break;
	case ROSE_QSIG_MWIInterrogate:
		break;
#endif	/* Not handled yet */
	default:
		if (ctrl->debug & PRI_DEBUG_APDU) {
			pri_message(ctrl,
				"!! ROSE invoke operation not handled on switchtype:%s! %s\n",
				pri_switch2str(ctrl->switchtype), rose_operation2str(invoke->operation));
		}
		break;
	}
}
