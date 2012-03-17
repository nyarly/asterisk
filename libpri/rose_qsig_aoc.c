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
 * \brief Q.SIG ROSE Advice-Of-Charge (AOC) operations
 *
 * SS-AOC-Operations ECMA-212 Annex E Table E.1
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
 * \brief Encode the Time type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param time Time information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_qsig_AOC_Time(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, unsigned tag, const struct roseQsigAOCTime *time)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1,
		time->length));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2, time->scale));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \internal
 * \brief Encode the Amount type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param amount Amount information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_qsig_AOC_Amount(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, unsigned tag, const struct roseQsigAOCAmount *amount)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1,
		amount->currency));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2,
		amount->multiplier));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \internal
 * \brief Encode the RecordedCurrency type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param recorded Recorded currency information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_qsig_AOC_RecordedCurrency(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, unsigned tag,
	const struct roseQsigAOCRecordedCurrency *recorded)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, asn1_enc_string_max(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1,
		recorded->currency, sizeof(recorded->currency) - 1));
	ASN1_CALL(pos, rose_enc_qsig_AOC_Amount(ctrl, pos, end,
		ASN1_CLASS_CONTEXT_SPECIFIC | 2, &recorded->amount));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \internal
 * \brief Encode the DurationCurrency type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param duration Duration currency information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_qsig_AOC_DurationCurrency(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, unsigned tag,
	const struct roseQsigAOCDurationCurrency *duration)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, asn1_enc_string_max(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1,
		duration->currency, sizeof(duration->currency) - 1));
	ASN1_CALL(pos, rose_enc_qsig_AOC_Amount(ctrl, pos, end,
		ASN1_CLASS_CONTEXT_SPECIFIC | 2, &duration->amount));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 3,
		duration->charging_type));
	ASN1_CALL(pos, rose_enc_qsig_AOC_Time(ctrl, pos, end,
		ASN1_CLASS_CONTEXT_SPECIFIC | 4, &duration->time));
	if (duration->granularity_present) {
		ASN1_CALL(pos, rose_enc_qsig_AOC_Time(ctrl, pos, end,
			ASN1_CLASS_CONTEXT_SPECIFIC | 5, &duration->granularity));
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \internal
 * \brief Encode the FlatRateCurrency type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param flat_rate Flat rate currency information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_qsig_AOC_FlatRateCurrency(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, unsigned tag,
	const struct roseQsigAOCFlatRateCurrency *flat_rate)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, asn1_enc_string_max(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1,
		flat_rate->currency, sizeof(flat_rate->currency) - 1));
	ASN1_CALL(pos, rose_enc_qsig_AOC_Amount(ctrl, pos, end,
		ASN1_CLASS_CONTEXT_SPECIFIC | 2, &flat_rate->amount));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \internal
 * \brief Encode the VolumeRateCurrency type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param volume_rate Volume rate currency information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_qsig_AOC_VolumeRateCurrency(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, unsigned tag,
	const struct roseQsigAOCVolumeRateCurrency *volume_rate)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, asn1_enc_string_max(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1,
		volume_rate->currency, sizeof(volume_rate->currency) - 1));
	ASN1_CALL(pos, rose_enc_qsig_AOC_Amount(ctrl, pos, end,
		ASN1_CLASS_CONTEXT_SPECIFIC | 2, &volume_rate->amount));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 3,
		volume_rate->unit));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \internal
 * \brief Encode the AOCSCurrencyInfo type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param currency_info Currency information record to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_qsig_AOCSCurrencyInfo(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, unsigned tag,
	const struct roseQsigAOCSCurrencyInfo *currency_info)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		currency_info->charged_item));

	switch (currency_info->currency_type) {
	case 0:	/* specialChargingCode */
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
			currency_info->u.special_charging_code));
		break;
	case 1:	/* durationCurrency */
		ASN1_CALL(pos, rose_enc_qsig_AOC_DurationCurrency(ctrl, pos, end,
			ASN1_CLASS_CONTEXT_SPECIFIC | 1, &currency_info->u.duration));
		break;
	case 2:	/* flatRateCurrency */
		ASN1_CALL(pos, rose_enc_qsig_AOC_FlatRateCurrency(ctrl, pos, end,
			ASN1_CLASS_CONTEXT_SPECIFIC | 2, &currency_info->u.flat_rate));
		break;
	case 3:	/* volumeRateCurrency */
		ASN1_CALL(pos, rose_enc_qsig_AOC_VolumeRateCurrency(ctrl, pos, end,
			ASN1_CLASS_CONTEXT_SPECIFIC | 3, &currency_info->u.volume_rate));
		break;
	case 4:	/* freeOfCharge */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 4));
		break;
	case 5:	/* currencyInfoNotAvailable */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 5));
		break;
	case 6:	/* freeOfChargeFromBeginning */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 6));
		break;
	default:
		ASN1_ENC_ERROR(ctrl, "Unknown currency type");
		return NULL;
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \internal
 * \brief Encode the AOCSCurrencyInfoList type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param currency_info Currency information list to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_qsig_AOCSCurrencyInfoList(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, unsigned tag,
	const struct roseQsigAOCSCurrencyInfoList *currency_info)
{
	unsigned index;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	for (index = 0; index < currency_info->num_records; ++index) {
		ASN1_CALL(pos, rose_enc_qsig_AOCSCurrencyInfo(ctrl, pos, end, ASN1_TAG_SEQUENCE,
			&currency_info->list[index]));
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \internal
 * \brief Encode the ChargingAssociation type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param charging Charging association information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_qsig_AOC_ChargingAssociation(struct pri *ctrl,
	unsigned char *pos, unsigned char *end,
	const struct roseQsigAOCChargingAssociation *charging)
{
	unsigned char *explicit_len;

	switch (charging->type) {
	case 0:	/* charge_identifier */
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER, charging->id));
		break;
	case 1:	/* charged_number */
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 0);
		ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end, &charging->number));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
		break;
	default:
		ASN1_ENC_ERROR(ctrl, "Unknown ChargingAssociation type");
		return NULL;
	}

	return pos;
}

/*!
 * \brief Encode the Q.SIG ChargeRequest invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_ChargeRequest_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	unsigned index;
	unsigned char *seq_len;
	unsigned char *advice_len;
	const struct roseQsigChargeRequestArg_ARG *charge_request;

	charge_request = &args->qsig.ChargeRequest;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	/* SEQUENCE SIZE(0..7) OF AdviceModeCombination */
	ASN1_CONSTRUCTED_BEGIN(advice_len, pos, end, ASN1_TAG_SEQUENCE);
	for (index = 0; index < charge_request->num_records; ++index) {
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
			charge_request->advice_mode_combinations[index]));
	}
	ASN1_CONSTRUCTED_END(advice_len, pos, end);

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the Q.SIG ChargeRequest result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_ChargeRequest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args)
{
	unsigned char *seq_len;
	const struct roseQsigChargeRequestRes_RES *charge_request;

	charge_request = &args->qsig.ChargeRequest;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		charge_request->advice_mode_combination));

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the Q.SIG AocFinal invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_AocFinal_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	unsigned char *seq_len;
	unsigned char *specific_len;
	const struct roseQsigAocFinalArg_ARG *aoc_final;

	aoc_final = &args->qsig.AocFinal;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	switch (aoc_final->type) {
	case 0:	/* charge_not_available */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 0));
		break;
	case 1:	/* free_of_charge */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1));
		break;
	case 2:	/* specific_currency */
		ASN1_CONSTRUCTED_BEGIN(specific_len, pos, end, ASN1_TAG_SEQUENCE);

		ASN1_CALL(pos, rose_enc_qsig_AOC_RecordedCurrency(ctrl, pos, end,
			ASN1_CLASS_CONTEXT_SPECIFIC | 1, &aoc_final->specific.recorded));

		if (aoc_final->specific.billing_id_present) {
			ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2,
				aoc_final->specific.billing_id));
		}

		ASN1_CONSTRUCTED_END(specific_len, pos, end);
		break;
	default:
		ASN1_ENC_ERROR(ctrl, "Unknown AocFinal type");
		return NULL;
	}

	if (aoc_final->charging_association_present) {
		ASN1_CALL(pos, rose_enc_qsig_AOC_ChargingAssociation(ctrl, pos, end,
			&aoc_final->charging_association));
	}

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the Q.SIG AocInterim invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_AocInterim_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	unsigned char *seq_len;
	unsigned char *specific_len;
	const struct roseQsigAocInterimArg_ARG *aoc_interim;

	aoc_interim = &args->qsig.AocInterim;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	switch (aoc_interim->type) {
	case 0:	/* charge_not_available */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 0));
		break;
	case 1:	/* free_of_charge */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1));
		break;
	case 2:	/* specific_currency */
		ASN1_CONSTRUCTED_BEGIN(specific_len, pos, end, ASN1_TAG_SEQUENCE);

		ASN1_CALL(pos, rose_enc_qsig_AOC_RecordedCurrency(ctrl, pos, end,
			ASN1_CLASS_CONTEXT_SPECIFIC | 1, &aoc_interim->specific.recorded));

		if (aoc_interim->specific.billing_id_present) {
			ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2,
				aoc_interim->specific.billing_id));
		}

		ASN1_CONSTRUCTED_END(specific_len, pos, end);
		break;
	default:
		ASN1_ENC_ERROR(ctrl, "Unknown AocInterim type");
		return NULL;
	}

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the Q.SIG AocRate invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_AocRate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	unsigned char *seq_len;
	const struct roseQsigAocRateArg_ARG *aoc_rate;

	aoc_rate = &args->qsig.AocRate;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	switch (aoc_rate->type) {
	case 0:	/* charge_not_available */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_TYPE_NULL));
		break;
	case 1:	/* currency_info_list */
		ASN1_CALL(pos, rose_enc_qsig_AOCSCurrencyInfoList(ctrl, pos, end,
			ASN1_TAG_SEQUENCE, &aoc_rate->currency_info));
		break;
	default:
		ASN1_ENC_ERROR(ctrl, "Unknown AocRate type");
		return NULL;
	}

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the Q.SIG AocComplete invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_AocComplete_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	unsigned char *seq_len;
	const struct roseQsigAocCompleteArg_ARG *aoc_complete;

	aoc_complete = &args->qsig.AocComplete;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
		&aoc_complete->charged_user_number));

	if (aoc_complete->charging_association_present) {
		ASN1_CALL(pos, rose_enc_qsig_AOC_ChargingAssociation(ctrl, pos, end,
			&aoc_complete->charging_association));
	}

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the Q.SIG AocComplete result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_AocComplete_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args)
{
	unsigned char *seq_len;
	const struct roseQsigAocCompleteRes_RES *aoc_complete;

	aoc_complete = &args->qsig.AocComplete;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		aoc_complete->charging_option));

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the Q.SIG AocDivChargeReq invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_AocDivChargeReq_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	unsigned char *seq_len;
	const struct roseQsigAocDivChargeReqArg_ARG *aoc_div_charge_req;

	aoc_div_charge_req = &args->qsig.AocDivChargeReq;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
		&aoc_div_charge_req->diverting_user_number));

	if (aoc_div_charge_req->charging_association_present) {
		ASN1_CALL(pos, rose_enc_qsig_AOC_ChargingAssociation(ctrl, pos, end,
			&aoc_div_charge_req->charging_association));
	}

	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		aoc_div_charge_req->diversion_type));

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \internal
 * \brief Decode the Time type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param time Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_qsig_AOC_Time(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigAOCTime *time)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s Time %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 1);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "lengthOfTimeUnit", tag, pos, seq_end, &value));
	time->length = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 2);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "scale", tag, pos, seq_end, &value));
	time->scale = value;

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \internal
 * \brief Decode the Amount type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param amount Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_qsig_AOC_Amount(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigAOCAmount *amount)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s Amount %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 1);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "currencyAmount", tag, pos, seq_end, &value));
	amount->currency = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 2);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "multiplier", tag, pos, seq_end, &value));
	amount->multiplier = value;

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \internal
 * \brief Decode the RecordedCurrency type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param recorded Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_qsig_AOC_RecordedCurrency(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigAOCRecordedCurrency *recorded)
{
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	size_t str_len;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s RecordedCurrency %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag & ~ASN1_PC_MASK, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 1);
	ASN1_CALL(pos, asn1_dec_string_max(ctrl, "rCurrency", tag, pos, seq_end,
		sizeof(recorded->currency), recorded->currency, &str_len));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag,
		ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 2);
	ASN1_CALL(pos, rose_dec_qsig_AOC_Amount(ctrl, "rAmount", tag, pos, seq_end,
		&recorded->amount));

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \internal
 * \brief Decode the DurationCurrency type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param duration Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_qsig_AOC_DurationCurrency(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigAOCDurationCurrency *duration)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	size_t str_len;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s DurationCurrency %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag & ~ASN1_PC_MASK, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 1);
	ASN1_CALL(pos, asn1_dec_string_max(ctrl, "dCurrency", tag, pos, seq_end,
		sizeof(duration->currency), duration->currency, &str_len));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag,
		ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 2);
	ASN1_CALL(pos, rose_dec_qsig_AOC_Amount(ctrl, "dAmount", tag, pos, seq_end,
		&duration->amount));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 3);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "dChargingType", tag, pos, seq_end, &value));
	duration->charging_type = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag,
		ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 4);
	ASN1_CALL(pos, rose_dec_qsig_AOC_Time(ctrl, "dTime", tag, pos, seq_end,
		&duration->time));

	if (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CHECK_TAG(ctrl, tag, tag,
			ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 5);
		ASN1_CALL(pos, rose_dec_qsig_AOC_Time(ctrl, "dGranularity", tag, pos, seq_end,
			&duration->granularity));
		duration->granularity_present = 1;
	} else {
		duration->granularity_present = 0;
	}

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \internal
 * \brief Decode the FlatRateCurrency type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param flat_rate Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_qsig_AOC_FlatRateCurrency(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigAOCFlatRateCurrency *flat_rate)
{
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	size_t str_len;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s FlatRateCurrency %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag & ~ASN1_PC_MASK, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 1);
	ASN1_CALL(pos, asn1_dec_string_max(ctrl, "fRCurrency", tag, pos, seq_end,
		sizeof(flat_rate->currency), flat_rate->currency, &str_len));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag,
		ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 2);
	ASN1_CALL(pos, rose_dec_qsig_AOC_Amount(ctrl, "fRAmount", tag, pos, seq_end,
		&flat_rate->amount));

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \internal
 * \brief Decode the VolumeRateCurrency type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param volume_rate Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_qsig_AOC_VolumeRateCurrency(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigAOCVolumeRateCurrency *volume_rate)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	size_t str_len;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s VolumeRateCurrency %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag & ~ASN1_PC_MASK, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 1);
	ASN1_CALL(pos, asn1_dec_string_max(ctrl, "vRCurrency", tag, pos, seq_end,
		sizeof(volume_rate->currency), volume_rate->currency, &str_len));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag,
		ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 2);
	ASN1_CALL(pos, rose_dec_qsig_AOC_Amount(ctrl, "vRAmount", tag, pos, seq_end,
		&volume_rate->amount));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 3);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "vRVolumeUnit", tag, pos, seq_end, &value));
	volume_rate->unit = value;

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \internal
 * \brief Decode the AOCSCurrencyInfo type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param currency_info Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_qsig_AOCSCurrencyInfo(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigAOCSCurrencyInfo *currency_info)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s AOCSCurrencyInfo %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "chargedItem", tag, pos, seq_end, &value));
	currency_info->charged_item = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	switch (tag) {
	case ASN1_TYPE_INTEGER:
		currency_info->currency_type = 0;	/* specialChargingCode */
		ASN1_CALL(pos, asn1_dec_int(ctrl, "specialChargingCode", tag, pos, seq_end,
			&value));
		currency_info->u.special_charging_code = value;
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 1:
		currency_info->currency_type = 1;	/* durationCurrency */
		ASN1_CALL(pos, rose_dec_qsig_AOC_DurationCurrency(ctrl, "durationCurrency", tag,
			pos, seq_end, &currency_info->u.duration));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 2:
		currency_info->currency_type = 2;	/* flatRateCurrency */
		ASN1_CALL(pos, rose_dec_qsig_AOC_FlatRateCurrency(ctrl, "flatRateCurrency", tag,
			pos, seq_end, &currency_info->u.flat_rate));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 3:
		currency_info->currency_type = 3;	/* volumeRateCurrency */
		ASN1_CALL(pos, rose_dec_qsig_AOC_VolumeRateCurrency(ctrl, "volumeRateCurrency",
			tag, pos, seq_end, &currency_info->u.volume_rate));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 4:
		currency_info->currency_type = 4;	/* freeOfCharge */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "freeOfCharge", tag, pos, seq_end));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 5:
		currency_info->currency_type = 5;	/* currencyInfoNotAvailable */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "currencyInfoNotAvailable", tag, pos,
			seq_end));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 6:
		currency_info->currency_type = 6;	/* freeOfChargeFromBeginning */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "freeOfChargeFromBeginning", tag, pos,
			seq_end));
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \internal
 * \brief Decode the AOCSCurrencyInfoList type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param currency_info Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_qsig_AOCSCurrencyInfoList(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigAOCSCurrencyInfoList *currency_info)
{
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s AOCSCurrencyInfoList %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	currency_info->num_records = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		if (currency_info->num_records < ARRAY_LEN(currency_info->list)) {
			ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
			ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
			ASN1_CALL(pos, rose_dec_qsig_AOCSCurrencyInfo(ctrl, "listEntry", tag, pos,
				seq_end, &currency_info->list[currency_info->num_records]));
			++currency_info->num_records;
		} else {
			/* Too many records */
			return NULL;
		}
	}

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \internal
 * \brief Decode the ChargingAssociation type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param charging Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_qsig_AOC_ChargingAssociation(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigAOCChargingAssociation *charging)
{
	int32_t value;
	int length;
	int explicit_offset;
	const unsigned char *explicit_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s ChargingAssociation\n", name);
	}
	switch (tag) {
	case ASN1_TYPE_INTEGER:
		charging->type = 0;	/* charge_identifier */
		ASN1_CALL(pos, asn1_dec_int(ctrl, "chargeIdentifier", tag, pos, end, &value));
		charging->id = value;
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 0:
		charging->type = 1;	/* charged_number */

		/* Remove EXPLICIT tag */
		if (ctrl->debug & PRI_DEBUG_APDU) {
			pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
		}
		ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
		ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, end);

		ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
		ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "chargedNumber", tag, pos,
			explicit_end, &charging->number));

		ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, end);
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	return pos;
}

/*!
 * \brief Decode the Q.SIG ChargeRequest invoke argument parameters.
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
const unsigned char *rose_dec_qsig_ChargeRequest_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	int advice_offset;
	const unsigned char *seq_end;
	const unsigned char *advice_end;
	struct roseQsigChargeRequestArg_ARG *charge_request;

	charge_request = &args->qsig.ChargeRequest;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  ChargeRequest %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	/* SEQUENCE SIZE(0..7) OF AdviceModeCombination */
	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  adviceModeCombinations %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
	ASN1_END_SETUP(advice_end, advice_offset, length, pos, seq_end);

	/* Decode SIZE(0..7) OF AdviceModeCombination */
	charge_request->num_records = 0;
	while (pos < advice_end && *pos != ASN1_INDEF_TERM) {
		if (charge_request->num_records <
			ARRAY_LEN(charge_request->advice_mode_combinations)) {
			ASN1_CALL(pos, asn1_dec_tag(pos, advice_end, &tag));
			ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
			ASN1_CALL(pos, asn1_dec_int(ctrl, "adviceModeCombination", tag, pos,
				advice_end, &value));
			charge_request->advice_mode_combinations[charge_request->num_records] =
				value;
			++charge_request->num_records;
		} else {
			/* Too many records */
			return NULL;
		}
	}

	ASN1_END_FIXUP(ctrl, pos, advice_offset, advice_end, seq_end);

	/* Fixup will skip over any OPTIONAL manufacturer extension information */
	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG ChargeRequest result argument parameters.
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
const unsigned char *rose_dec_qsig_ChargeRequest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_result_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	struct roseQsigChargeRequestRes_RES *charge_request;

	charge_request = &args->qsig.ChargeRequest;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  ChargeRequest %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "adviceModeCombination", tag, pos, seq_end,
		&value));
	charge_request->advice_mode_combination = value;

	/* Fixup will skip over any OPTIONAL manufacturer extension information */
	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG AocFinal invoke argument parameters.
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
const unsigned char *rose_dec_qsig_AocFinal_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	int specific_offset;
	const unsigned char *seq_end;
	const unsigned char *specific_end;
	const unsigned char *save_pos;
	struct roseQsigAocFinalArg_ARG *aoc_final;

	aoc_final = &args->qsig.AocFinal;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  AocFinal %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	switch (tag) {
	case ASN1_CLASS_CONTEXT_SPECIFIC | 0:
		aoc_final->type = 0;	/* charge_not_available */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "chargeNotAvailable", tag, pos, seq_end));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		aoc_final->type = 1;	/* free_of_charge */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "freeOfCharge", tag, pos, seq_end));
		break;
	case ASN1_TAG_SEQUENCE:
		aoc_final->type = 2;	/* specific_currency */
		if (ctrl->debug & PRI_DEBUG_APDU) {
			pri_message(ctrl, "  specificCurrency %s\n", asn1_tag2str(tag));
		}
		ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
		ASN1_END_SETUP(specific_end, specific_offset, length, pos, seq_end);

		ASN1_CALL(pos, asn1_dec_tag(pos, specific_end, &tag));
		ASN1_CHECK_TAG(ctrl, tag, tag,
			ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 1);
		ASN1_CALL(pos, rose_dec_qsig_AOC_RecordedCurrency(ctrl, "recordedCurrency", tag,
			pos, specific_end, &aoc_final->specific.recorded));

		if (pos < specific_end && *pos != ASN1_INDEF_TERM) {
			ASN1_CALL(pos, asn1_dec_tag(pos, specific_end, &tag));
			ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 2);
			ASN1_CALL(pos, asn1_dec_int(ctrl, "finalBillingId", tag, pos, specific_end,
				&value));
			aoc_final->specific.billing_id = value;
			aoc_final->specific.billing_id_present = 1;
		} else {
			aoc_final->specific.billing_id_present = 0;
		}

		ASN1_END_FIXUP(ctrl, pos, specific_offset, specific_end, seq_end);
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	aoc_final->charging_association_present = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag) {
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 0:
		case ASN1_TYPE_INTEGER:
			ASN1_CALL(pos, rose_dec_qsig_AOC_ChargingAssociation(ctrl,
				"chargingAssociation", tag, pos, seq_end,
				&aoc_final->charging_association));
			aoc_final->charging_association_present = 1;
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 1:
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 2:
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  finalArgExtension %s\n", asn1_tag2str(tag));
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
 * \brief Decode the Q.SIG AocInterim invoke argument parameters.
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
const unsigned char *rose_dec_qsig_AocInterim_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	int specific_offset;
	const unsigned char *seq_end;
	const unsigned char *specific_end;
	struct roseQsigAocInterimArg_ARG *aoc_interim;

	aoc_interim = &args->qsig.AocInterim;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  AocInterim %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	switch (tag) {
	case ASN1_CLASS_CONTEXT_SPECIFIC | 0:
		aoc_interim->type = 0;	/* charge_not_available */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "chargeNotAvailable", tag, pos, seq_end));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		aoc_interim->type = 1;	/* free_of_charge */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "freeOfCharge", tag, pos, seq_end));
		break;
	case ASN1_TAG_SEQUENCE:
		aoc_interim->type = 2;	/* specific_currency */
		if (ctrl->debug & PRI_DEBUG_APDU) {
			pri_message(ctrl, "  specificCurrency %s\n", asn1_tag2str(tag));
		}
		ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
		ASN1_END_SETUP(specific_end, specific_offset, length, pos, seq_end);

		ASN1_CALL(pos, asn1_dec_tag(pos, specific_end, &tag));
		ASN1_CHECK_TAG(ctrl, tag, tag,
			ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 1);
		ASN1_CALL(pos, rose_dec_qsig_AOC_RecordedCurrency(ctrl, "recordedCurrency", tag,
			pos, specific_end, &aoc_interim->specific.recorded));

		if (pos < specific_end && *pos != ASN1_INDEF_TERM) {
			ASN1_CALL(pos, asn1_dec_tag(pos, specific_end, &tag));
			ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 2);
			ASN1_CALL(pos, asn1_dec_int(ctrl, "interimBillingId", tag, pos, specific_end,
				&value));
			aoc_interim->specific.billing_id = value;
			aoc_interim->specific.billing_id_present = 1;
		} else {
			aoc_interim->specific.billing_id_present = 0;
		}

		ASN1_END_FIXUP(ctrl, pos, specific_offset, specific_end, seq_end);
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	/* Fixup will skip over any OPTIONAL manufacturer extension information */
	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG AocRate invoke argument parameters.
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
const unsigned char *rose_dec_qsig_AocRate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	struct roseQsigAocRateArg_ARG *aoc_rate;

	aoc_rate = &args->qsig.AocRate;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  AocRate %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	switch (tag) {
	case ASN1_TYPE_NULL:
		aoc_rate->type = 0;	/* charge_not_available */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "chargeNotAvailable", tag, pos, seq_end));
		break;
	case ASN1_TAG_SEQUENCE:
		aoc_rate->type = 1;	/* currency_info_list */
		ASN1_CALL(pos, rose_dec_qsig_AOCSCurrencyInfoList(ctrl, "aocSCurrencyInfoList",
			tag, pos, seq_end, &aoc_rate->currency_info));
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	/* Fixup will skip over any OPTIONAL manufacturer extension information */
	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG AocComplete invoke argument parameters.
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
const unsigned char *rose_dec_qsig_AocComplete_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	const unsigned char *save_pos;
	struct roseQsigAocCompleteArg_ARG *aoc_complete;

	aoc_complete = &args->qsig.AocComplete;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  AocComplete %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "chargedUser", tag, pos, seq_end,
		&aoc_complete->charged_user_number));

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	aoc_complete->charging_association_present = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag) {
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 0:
		case ASN1_TYPE_INTEGER:
			ASN1_CALL(pos, rose_dec_qsig_AOC_ChargingAssociation(ctrl,
				"chargingAssociation", tag, pos, seq_end,
				&aoc_complete->charging_association));
			aoc_complete->charging_association_present = 1;
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 1:
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 2:
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  completeArgExtension %s\n", asn1_tag2str(tag));
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
 * \brief Decode the Q.SIG AocComplete result argument parameters.
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
const unsigned char *rose_dec_qsig_AocComplete_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_result_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	struct roseQsigAocCompleteRes_RES *aoc_complete;

	aoc_complete = &args->qsig.AocComplete;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  AocComplete %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "chargingOption", tag, pos, seq_end, &value));
	aoc_complete->charging_option = value;

	/* Fixup will skip over any OPTIONAL manufacturer extension information */
	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG AocDivChargeReq invoke argument parameters.
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
const unsigned char *rose_dec_qsig_AocDivChargeReq_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	struct roseQsigAocDivChargeReqArg_ARG *aoc_div_charge_req;

	aoc_div_charge_req = &args->qsig.AocDivChargeReq;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  AocDivChargeReq %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "divertingUser", tag, pos, seq_end,
		&aoc_div_charge_req->diverting_user_number));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	switch (tag) {
	case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 0:
	case ASN1_TYPE_INTEGER:
		ASN1_CALL(pos, rose_dec_qsig_AOC_ChargingAssociation(ctrl, "chargingAssociation",
			tag, pos, seq_end, &aoc_div_charge_req->charging_association));
		aoc_div_charge_req->charging_association_present = 1;

		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		break;
	default:
		aoc_div_charge_req->charging_association_present = 0;
		break;
	}

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "diversionType", tag, pos, seq_end, &value));
	aoc_div_charge_req->diversion_type = value;

	/* Fixup will skip over any OPTIONAL manufacturer extension information */
	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/* ------------------------------------------------------------------- */
/* end rose_qsig_aoc.c */
