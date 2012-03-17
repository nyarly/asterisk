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
 * \brief ROSE Advice Of Charge (AOC) operations
 *
 * Advice of Charge (AOC) supplementary service EN 300 182-1
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
static unsigned char *rose_enc_etsi_AOC_Time(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, unsigned tag, const struct roseEtsiAOCTime *time)
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
static unsigned char *rose_enc_etsi_AOC_Amount(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, unsigned tag, const struct roseEtsiAOCAmount *amount)
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
static unsigned char *rose_enc_etsi_AOC_RecordedCurrency(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, unsigned tag,
	const struct roseEtsiAOCRecordedCurrency *recorded)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, asn1_enc_string_max(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1,
		recorded->currency, sizeof(recorded->currency) - 1));
	ASN1_CALL(pos, rose_enc_etsi_AOC_Amount(ctrl, pos, end,
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
static unsigned char *rose_enc_etsi_AOC_DurationCurrency(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, unsigned tag,
	const struct roseEtsiAOCDurationCurrency *duration)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, asn1_enc_string_max(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1,
		duration->currency, sizeof(duration->currency) - 1));
	ASN1_CALL(pos, rose_enc_etsi_AOC_Amount(ctrl, pos, end,
		ASN1_CLASS_CONTEXT_SPECIFIC | 2, &duration->amount));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 3,
		duration->charging_type));
	ASN1_CALL(pos, rose_enc_etsi_AOC_Time(ctrl, pos, end,
		ASN1_CLASS_CONTEXT_SPECIFIC | 4, &duration->time));
	if (duration->granularity_present) {
		ASN1_CALL(pos, rose_enc_etsi_AOC_Time(ctrl, pos, end,
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
static unsigned char *rose_enc_etsi_AOC_FlatRateCurrency(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, unsigned tag,
	const struct roseEtsiAOCFlatRateCurrency *flat_rate)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, asn1_enc_string_max(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1,
		flat_rate->currency, sizeof(flat_rate->currency) - 1));
	ASN1_CALL(pos, rose_enc_etsi_AOC_Amount(ctrl, pos, end,
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
static unsigned char *rose_enc_etsi_AOC_VolumeRateCurrency(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, unsigned tag,
	const struct roseEtsiAOCVolumeRateCurrency *volume_rate)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, asn1_enc_string_max(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1,
		volume_rate->currency, sizeof(volume_rate->currency) - 1));
	ASN1_CALL(pos, rose_enc_etsi_AOC_Amount(ctrl, pos, end,
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
static unsigned char *rose_enc_etsi_AOCSCurrencyInfo(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, unsigned tag,
	const struct roseEtsiAOCSCurrencyInfo *currency_info)
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
		ASN1_CALL(pos, rose_enc_etsi_AOC_DurationCurrency(ctrl, pos, end,
			ASN1_CLASS_CONTEXT_SPECIFIC | 1, &currency_info->u.duration));
		break;
	case 2:	/* flatRateCurrency */
		ASN1_CALL(pos, rose_enc_etsi_AOC_FlatRateCurrency(ctrl, pos, end,
			ASN1_CLASS_CONTEXT_SPECIFIC | 2, &currency_info->u.flat_rate));
		break;
	case 3:	/* volumeRateCurrency */
		ASN1_CALL(pos, rose_enc_etsi_AOC_VolumeRateCurrency(ctrl, pos, end,
			ASN1_CLASS_CONTEXT_SPECIFIC | 3, &currency_info->u.volume_rate));
		break;
	case 4:	/* freeOfCharge */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 4));
		break;
	case 5:	/* currencyInfoNotAvailable */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 5));
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
static unsigned char *rose_enc_etsi_AOCSCurrencyInfoList(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, unsigned tag,
	const struct roseEtsiAOCSCurrencyInfoList *currency_info)
{
	unsigned index;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	for (index = 0; index < currency_info->num_records; ++index) {
		ASN1_CALL(pos, rose_enc_etsi_AOCSCurrencyInfo(ctrl, pos, end, ASN1_TAG_SEQUENCE,
			&currency_info->list[index]));
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \internal
 * \brief Encode the RecordedUnits type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param recorded Recorded units information record to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_etsi_AOC_RecordedUnits(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, unsigned tag,
	const struct roseEtsiAOCRecordedUnits *recorded)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	if (recorded->not_available) {
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_TYPE_NULL));
	} else {
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
			recorded->number_of_units));
	}

	if (recorded->type_of_unit_present) {
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
			recorded->type_of_unit));
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \internal
 * \brief Encode the RecordedUnitsList type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param recorded_info Recorded units information list to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_etsi_AOC_RecordedUnitsList(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, unsigned tag,
	const struct roseEtsiAOCRecordedUnitsList *recorded_info)
{
	unsigned index;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	for (index = 0; index < recorded_info->num_records; ++index) {
		ASN1_CALL(pos, rose_enc_etsi_AOC_RecordedUnits(ctrl, pos, end, ASN1_TAG_SEQUENCE,
			&recorded_info->list[index]));
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
static unsigned char *rose_enc_etsi_AOC_ChargingAssociation(struct pri *ctrl,
	unsigned char *pos, unsigned char *end,
	const struct roseEtsiAOCChargingAssociation *charging)
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
 * \internal
 * \brief Encode the AOCECurrencyInfo type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param currency_info Currency information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_etsi_AOCECurrencyInfo(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, unsigned tag,
	const struct roseEtsiAOCECurrencyInfo *currency_info)
{
	unsigned char *seq_len;
	unsigned char *specific_seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	if (currency_info->free_of_charge) {
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1));
	} else {
		ASN1_CONSTRUCTED_BEGIN(specific_seq_len, pos, end, ASN1_TAG_SEQUENCE);

		ASN1_CALL(pos, rose_enc_etsi_AOC_RecordedCurrency(ctrl, pos, end,
			ASN1_CLASS_CONTEXT_SPECIFIC | 1, &currency_info->specific.recorded));

		if (currency_info->specific.billing_id_present) {
			ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2,
				currency_info->specific.billing_id));
		}

		ASN1_CONSTRUCTED_END(specific_seq_len, pos, end);
	}

	if (currency_info->charging_association_present) {
		ASN1_CALL(pos, rose_enc_etsi_AOC_ChargingAssociation(ctrl, pos, end,
			&currency_info->charging_association));
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \internal
 * \brief Encode the AOCEChargingUnitInfo type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param charging_unit Charging unit information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_etsi_AOCEChargingUnitInfo(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, unsigned tag,
	const struct roseEtsiAOCEChargingUnitInfo *charging_unit)
{
	unsigned char *seq_len;
	unsigned char *specific_seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	if (charging_unit->free_of_charge) {
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1));
	} else {
		ASN1_CONSTRUCTED_BEGIN(specific_seq_len, pos, end, ASN1_TAG_SEQUENCE);

		ASN1_CALL(pos, rose_enc_etsi_AOC_RecordedUnitsList(ctrl, pos, end,
			ASN1_CLASS_CONTEXT_SPECIFIC | 1, &charging_unit->specific.recorded));

		if (charging_unit->specific.billing_id_present) {
			ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2,
				charging_unit->specific.billing_id));
		}

		ASN1_CONSTRUCTED_END(specific_seq_len, pos, end);
	}

	if (charging_unit->charging_association_present) {
		ASN1_CALL(pos, rose_enc_etsi_AOC_ChargingAssociation(ctrl, pos, end,
			&charging_unit->charging_association));
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the ChargingRequest invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_ChargingRequest_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		args->etsi.ChargingRequest.charging_case);
}

/*!
 * \brief Encode the ChargingRequest result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_ChargingRequest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args)
{
	switch (args->etsi.ChargingRequest.type) {
	case 0:	/* currency_info_list */
		ASN1_CALL(pos, rose_enc_etsi_AOCSCurrencyInfoList(ctrl, pos, end,
			ASN1_TAG_SEQUENCE, &args->etsi.ChargingRequest.u.currency_info));
		break;
	case 1:	/* special_arrangement_info */
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
			args->etsi.ChargingRequest.u.special_arrangement));
		break;
	case 2:	/* charging_info_follows */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_TYPE_NULL));
		break;
	default:
		ASN1_ENC_ERROR(ctrl, "Unknown ChargingRequst type");
		return NULL;
	}

	return pos;
}

/*!
 * \brief Encode the AOCSCurrency invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_AOCSCurrency_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	switch (args->etsi.AOCSCurrency.type) {
	case 0:	/* charge_not_available */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_TYPE_NULL));
		break;
	case 1:	/* currency_info_list */
		if (args->etsi.AOCSCurrency.currency_info.num_records) {
			ASN1_CALL(pos, rose_enc_etsi_AOCSCurrencyInfoList(ctrl, pos, end,
				ASN1_TAG_SEQUENCE, &args->etsi.AOCSCurrency.currency_info));
		} else {
			/* There were no records so encode as charge_not_available */
			ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_TYPE_NULL));
		}
		break;
	default:
		ASN1_ENC_ERROR(ctrl, "Unknown AOCSCurrency type");
		return NULL;
	}

	return pos;
}

/*!
 * \brief Encode the AOCSSpecialArr invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_AOCSSpecialArr_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	switch (args->etsi.AOCSSpecialArr.type) {
	case 0:	/* charge_not_available */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_TYPE_NULL));
		break;
	case 1:	/* special_arrangement_info */
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
			args->etsi.AOCSSpecialArr.special_arrangement));
		break;
	default:
		ASN1_ENC_ERROR(ctrl, "Unknown AOCSSpecialArr type");
		return NULL;
	}

	return pos;
}

/*!
 * \brief Encode the AOCDCurrency invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_AOCDCurrency_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiAOCDCurrency_ARG *aoc_d;
	unsigned char *seq_len;

	aoc_d = &args->etsi.AOCDCurrency;
	switch (aoc_d->type) {
	case 0:	/* charge_not_available */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_TYPE_NULL));
		break;
	case 1:	/* free_of_charge */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1));
		break;
	case 2:	/* specific_currency */
		ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

		ASN1_CALL(pos, rose_enc_etsi_AOC_RecordedCurrency(ctrl, pos, end,
			ASN1_CLASS_CONTEXT_SPECIFIC | 1, &aoc_d->specific.recorded));
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2,
			aoc_d->specific.type_of_charging_info));
		if (aoc_d->specific.billing_id_present) {
			ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 3,
				aoc_d->specific.billing_id));
		}

		ASN1_CONSTRUCTED_END(seq_len, pos, end);
		break;
	default:
		ASN1_ENC_ERROR(ctrl, "Unknown AOCDCurrency type");
		return NULL;
	}

	return pos;
}

/*!
 * \brief Encode the AOCDChargingUnit invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_AOCDChargingUnit_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiAOCDChargingUnit_ARG *aoc_d;
	unsigned char *seq_len;

	aoc_d = &args->etsi.AOCDChargingUnit;
	switch (aoc_d->type) {
	case 0:	/* charge_not_available */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_TYPE_NULL));
		break;
	case 1:	/* free_of_charge */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1));
		break;
	case 2:	/* specific_charging_units */
		ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

		ASN1_CALL(pos, rose_enc_etsi_AOC_RecordedUnitsList(ctrl, pos, end,
			ASN1_CLASS_CONTEXT_SPECIFIC | 1, &aoc_d->specific.recorded));
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2,
			aoc_d->specific.type_of_charging_info));
		if (aoc_d->specific.billing_id_present) {
			ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 3,
				aoc_d->specific.billing_id));
		}

		ASN1_CONSTRUCTED_END(seq_len, pos, end);
		break;
	default:
		ASN1_ENC_ERROR(ctrl, "Unknown AOCDChargingUnit type");
		return NULL;
	}

	return pos;
}

/*!
 * \brief Encode the AOCECurrency invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_AOCECurrency_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	switch (args->etsi.AOCECurrency.type) {
	case 0:	/* charge_not_available */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_TYPE_NULL));
		break;
	case 1:	/* currency_info */
		ASN1_CALL(pos, rose_enc_etsi_AOCECurrencyInfo(ctrl, pos, end, ASN1_TAG_SEQUENCE,
			&args->etsi.AOCECurrency.currency_info));
		break;
	default:
		ASN1_ENC_ERROR(ctrl, "Unknown AOCECurrency type");
		return NULL;
	}

	return pos;
}

/*!
 * \brief Encode the AOCEChargingUnit invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_AOCEChargingUnit_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	switch (args->etsi.AOCEChargingUnit.type) {
	case 0:	/* charge_not_available */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_TYPE_NULL));
		break;
	case 1:	/* charging_unit */
		ASN1_CALL(pos, rose_enc_etsi_AOCEChargingUnitInfo(ctrl, pos, end,
			ASN1_TAG_SEQUENCE, &args->etsi.AOCEChargingUnit.charging_unit));
		break;
	default:
		ASN1_ENC_ERROR(ctrl, "Unknown AOCEChargingUnit type");
		return NULL;
	}

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
static const unsigned char *rose_dec_etsi_AOC_Time(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiAOCTime *time)
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
static const unsigned char *rose_dec_etsi_AOC_Amount(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiAOCAmount *amount)
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
static const unsigned char *rose_dec_etsi_AOC_RecordedCurrency(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiAOCRecordedCurrency *recorded)
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
	ASN1_CALL(pos, rose_dec_etsi_AOC_Amount(ctrl, "rAmount", tag, pos, seq_end,
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
static const unsigned char *rose_dec_etsi_AOC_DurationCurrency(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiAOCDurationCurrency *duration)
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
	ASN1_CALL(pos, rose_dec_etsi_AOC_Amount(ctrl, "dAmount", tag, pos, seq_end,
		&duration->amount));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 3);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "dChargingType", tag, pos, seq_end, &value));
	duration->charging_type = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag,
		ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 4);
	ASN1_CALL(pos, rose_dec_etsi_AOC_Time(ctrl, "dTime", tag, pos, seq_end,
		&duration->time));

	if (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CHECK_TAG(ctrl, tag, tag,
			ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 5);
		ASN1_CALL(pos, rose_dec_etsi_AOC_Time(ctrl, "dGranularity", tag, pos, seq_end,
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
static const unsigned char *rose_dec_etsi_AOC_FlatRateCurrency(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiAOCFlatRateCurrency *flat_rate)
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
	ASN1_CALL(pos, rose_dec_etsi_AOC_Amount(ctrl, "fRAmount", tag, pos, seq_end,
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
static const unsigned char *rose_dec_etsi_AOC_VolumeRateCurrency(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiAOCVolumeRateCurrency *volume_rate)
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
	ASN1_CALL(pos, rose_dec_etsi_AOC_Amount(ctrl, "vRAmount", tag, pos, seq_end,
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
static const unsigned char *rose_dec_etsi_AOCSCurrencyInfo(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiAOCSCurrencyInfo *currency_info)
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
		ASN1_CALL(pos, rose_dec_etsi_AOC_DurationCurrency(ctrl, "durationCurrency", tag,
			pos, seq_end, &currency_info->u.duration));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 2:
		currency_info->currency_type = 2;	/* flatRateCurrency */
		ASN1_CALL(pos, rose_dec_etsi_AOC_FlatRateCurrency(ctrl, "flatRateCurrency", tag,
			pos, seq_end, &currency_info->u.flat_rate));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 3:
		currency_info->currency_type = 3;	/* volumeRateCurrency */
		ASN1_CALL(pos, rose_dec_etsi_AOC_VolumeRateCurrency(ctrl, "volumeRateCurrency",
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
static const unsigned char *rose_dec_etsi_AOCSCurrencyInfoList(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiAOCSCurrencyInfoList *currency_info)
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
			ASN1_CALL(pos, rose_dec_etsi_AOCSCurrencyInfo(ctrl, "listEntry", tag, pos,
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
 * \brief Decode the RecordedUnits type.
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
static const unsigned char *rose_dec_etsi_AOC_RecordedUnits(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiAOCRecordedUnits *recorded)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s RecordedUnits %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	switch (tag) {
	case ASN1_TYPE_INTEGER:
		recorded->not_available = 0;
		ASN1_CALL(pos, asn1_dec_int(ctrl, "recordedNumberOfUnits", tag, pos, seq_end,
			&value));
		recorded->number_of_units = value;
		break;
	case ASN1_TYPE_NULL:
		recorded->not_available = 1;
		recorded->number_of_units = 0;
		ASN1_CALL(pos, asn1_dec_null(ctrl, "notAvailable", tag, pos, seq_end));
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	if (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
		ASN1_CALL(pos, asn1_dec_int(ctrl, "recordedTypeOfUnits", tag, pos, seq_end,
			&value));
		recorded->type_of_unit = value;
		recorded->type_of_unit_present = 1;
	} else {
		recorded->type_of_unit_present = 0;
	}

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \internal
 * \brief Decode the RecordedUnitsList type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param recorded_info Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_etsi_AOC_RecordedUnitsList(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiAOCRecordedUnitsList *recorded_info)
{
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s RecordedUnitsList %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	recorded_info->num_records = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		if (recorded_info->num_records < ARRAY_LEN(recorded_info->list)) {
			ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
			ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
			ASN1_CALL(pos, rose_dec_etsi_AOC_RecordedUnits(ctrl, "listEntry", tag, pos,
				seq_end, &recorded_info->list[recorded_info->num_records]));
			++recorded_info->num_records;
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
static const unsigned char *rose_dec_etsi_AOC_ChargingAssociation(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiAOCChargingAssociation *charging)
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
 * \internal
 * \brief Decode the AOCECurrencyInfo type.
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
static const unsigned char *rose_dec_etsi_AOCECurrencyInfo(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiAOCECurrencyInfo *currency_info)
{
	int32_t value;
	int length;
	int seq_offset;
	int specific_offset;
	const unsigned char *seq_end;
	const unsigned char *specific_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s AOCECurrencyInfo %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	switch (tag) {
	case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		currency_info->free_of_charge = 1;
		ASN1_CALL(pos, asn1_dec_null(ctrl, "freeOfCharge", tag, pos, seq_end));
		break;
	case ASN1_TAG_SEQUENCE:
		currency_info->free_of_charge = 0;
		if (ctrl->debug & PRI_DEBUG_APDU) {
			pri_message(ctrl, "  specificCurrency %s\n", asn1_tag2str(tag));
		}
		ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
		ASN1_END_SETUP(specific_end, specific_offset, length, pos, seq_end);

		ASN1_CALL(pos, asn1_dec_tag(pos, specific_end, &tag));
		ASN1_CHECK_TAG(ctrl, tag, tag,
			ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 1);
		ASN1_CALL(pos, rose_dec_etsi_AOC_RecordedCurrency(ctrl, "recordedCurrency", tag,
			pos, specific_end, &currency_info->specific.recorded));

		if (pos < specific_end && *pos != ASN1_INDEF_TERM) {
			ASN1_CALL(pos, asn1_dec_tag(pos, specific_end, &tag));
			ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 2);
			ASN1_CALL(pos, asn1_dec_int(ctrl, "billingId", tag, pos, specific_end,
				&value));
			currency_info->specific.billing_id = value;
			currency_info->specific.billing_id_present = 1;
		} else {
			currency_info->specific.billing_id_present = 0;
		}

		ASN1_END_FIXUP(ctrl, pos, specific_offset, specific_end, seq_end);
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	if (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CALL(pos, rose_dec_etsi_AOC_ChargingAssociation(ctrl, "chargingAssociation",
			tag, pos, seq_end, &currency_info->charging_association));
		currency_info->charging_association_present = 1;
	} else {
		currency_info->charging_association_present = 0;
	}

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \internal
 * \brief Decode the AOCEChargingUnitInfo type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param charging_unit Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_etsi_AOCEChargingUnitInfo(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiAOCEChargingUnitInfo *charging_unit)
{
	int32_t value;
	int length;
	int seq_offset;
	int specific_offset;
	const unsigned char *seq_end;
	const unsigned char *specific_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s AOCEChargingUnitInfo %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	switch (tag) {
	case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		charging_unit->free_of_charge = 1;
		ASN1_CALL(pos, asn1_dec_null(ctrl, "freeOfCharge", tag, pos, seq_end));
		break;
	case ASN1_TAG_SEQUENCE:
		charging_unit->free_of_charge = 0;
		if (ctrl->debug & PRI_DEBUG_APDU) {
			pri_message(ctrl, "  specificChargingUnits %s\n", asn1_tag2str(tag));
		}
		ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
		ASN1_END_SETUP(specific_end, specific_offset, length, pos, seq_end);

		ASN1_CALL(pos, asn1_dec_tag(pos, specific_end, &tag));
		ASN1_CHECK_TAG(ctrl, tag, tag,
			ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 1);
		ASN1_CALL(pos, rose_dec_etsi_AOC_RecordedUnitsList(ctrl, "recordedUnitsList",
			tag, pos, specific_end, &charging_unit->specific.recorded));

		if (pos < specific_end && *pos != ASN1_INDEF_TERM) {
			ASN1_CALL(pos, asn1_dec_tag(pos, specific_end, &tag));
			ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 2);
			ASN1_CALL(pos, asn1_dec_int(ctrl, "billingId", tag, pos, specific_end,
				&value));
			charging_unit->specific.billing_id = value;
			charging_unit->specific.billing_id_present = 1;
		} else {
			charging_unit->specific.billing_id_present = 0;
		}

		ASN1_END_FIXUP(ctrl, pos, specific_offset, specific_end, seq_end);
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	if (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CALL(pos, rose_dec_etsi_AOC_ChargingAssociation(ctrl, "chargingAssociation",
			tag, pos, seq_end, &charging_unit->charging_association));
		charging_unit->charging_association_present = 1;
	} else {
		charging_unit->charging_association_present = 0;
	}

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the ChargingRequest invoke argument parameters.
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
const unsigned char *rose_dec_etsi_ChargingRequest_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "chargingCase", tag, pos, end, &value));
	args->etsi.ChargingRequest.charging_case = value;

	return pos;
}

/*!
 * \brief Decode the ChargingRequest result parameters.
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
const unsigned char *rose_dec_etsi_ChargingRequest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_result_args *args)
{
	struct roseEtsiChargingRequest_RES *charging_request;
	int32_t value;

	charging_request = &args->etsi.ChargingRequest;
	switch (tag) {
	case ASN1_TAG_SEQUENCE:
		charging_request->type = 0;	/* currency_info_list */
		ASN1_CALL(pos, rose_dec_etsi_AOCSCurrencyInfoList(ctrl, "currencyList", tag, pos,
			end, &charging_request->u.currency_info));
		break;
	case ASN1_TYPE_INTEGER:
		charging_request->type = 1;	/* special_arrangement_info */
		ASN1_CALL(pos, asn1_dec_int(ctrl, "specialArrangement", tag, pos, end, &value));
		charging_request->u.special_arrangement = value;
		break;
	case ASN1_TYPE_NULL:
		charging_request->type = 2;	/* charging_info_follows */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "chargingInfoFollows", tag, pos, end));
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	return pos;
}

/*!
 * \brief Decode the AOCSCurrency invoke argument parameters.
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
const unsigned char *rose_dec_etsi_AOCSCurrency_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	struct roseEtsiAOCSCurrency_ARG *aoc_s;

	aoc_s = &args->etsi.AOCSCurrency;
	switch (tag) {
	case ASN1_TYPE_NULL:
		aoc_s->type = 0;	/* charge_not_available */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "chargeNotAvailable", tag, pos, end));
		break;
	case ASN1_TAG_SEQUENCE:
		aoc_s->type = 1;	/* currency_info_list */
		ASN1_CALL(pos, rose_dec_etsi_AOCSCurrencyInfoList(ctrl, "currencyInfo", tag, pos,
			end, &aoc_s->currency_info));
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	return pos;
}

/*!
 * \brief Decode the AOCSSpecialArr invoke argument parameters.
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
const unsigned char *rose_dec_etsi_AOCSSpecialArr_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	struct roseEtsiAOCSSpecialArr_ARG *aoc_s;
	int32_t value;

	aoc_s = &args->etsi.AOCSSpecialArr;
	switch (tag) {
	case ASN1_TYPE_NULL:
		aoc_s->type = 0;	/* charge_not_available */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "chargeNotAvailable", tag, pos, end));
		break;
	case ASN1_TYPE_INTEGER:
		aoc_s->type = 1;	/* special_arrangement_info */
		ASN1_CALL(pos, asn1_dec_int(ctrl, "specialArrangement", tag, pos, end, &value));
		aoc_s->special_arrangement = value;
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	return pos;
}

/*!
 * \brief Decode the AOCDCurrency invoke argument parameters.
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
const unsigned char *rose_dec_etsi_AOCDCurrency_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	struct roseEtsiAOCDCurrency_ARG *aoc_d;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	int32_t value;

	aoc_d = &args->etsi.AOCDCurrency;
	switch (tag) {
	case ASN1_TYPE_NULL:
		aoc_d->type = 0;	/* charge_not_available */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "chargeNotAvailable", tag, pos, end));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		aoc_d->type = 1;	/* free_of_charge */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "freeOfCharge", tag, pos, end));
		break;
	case ASN1_TAG_SEQUENCE:
		aoc_d->type = 2;	/* specific_currency */

		if (ctrl->debug & PRI_DEBUG_APDU) {
			pri_message(ctrl, "  specificCurrency %s\n", asn1_tag2str(tag));
		}
		ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
		ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CHECK_TAG(ctrl, tag, tag,
			ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 1);
		ASN1_CALL(pos, rose_dec_etsi_AOC_RecordedCurrency(ctrl, "recordedCurrency", tag,
			pos, seq_end, &aoc_d->specific.recorded));

		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 2);
		ASN1_CALL(pos, asn1_dec_int(ctrl, "typeOfChargingInfo", tag, pos, seq_end,
			&value));
		aoc_d->specific.type_of_charging_info = value;

		if (pos < seq_end && *pos != ASN1_INDEF_TERM) {
			ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
			ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 3);
			ASN1_CALL(pos, asn1_dec_int(ctrl, "billingId", tag, pos, seq_end, &value));
			aoc_d->specific.billing_id = value;
			aoc_d->specific.billing_id_present = 1;
		} else {
			aoc_d->specific.billing_id_present = 0;
		}

		ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	return pos;
}

/*!
 * \brief Decode the AOCDChargingUnit invoke argument parameters.
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
const unsigned char *rose_dec_etsi_AOCDChargingUnit_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	struct roseEtsiAOCDChargingUnit_ARG *aoc_d;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	int32_t value;

	aoc_d = &args->etsi.AOCDChargingUnit;
	switch (tag) {
	case ASN1_TYPE_NULL:
		aoc_d->type = 0;	/* charge_not_available */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "chargeNotAvailable", tag, pos, end));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		aoc_d->type = 1;	/* free_of_charge */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "freeOfCharge", tag, pos, end));
		break;
	case ASN1_TAG_SEQUENCE:
		aoc_d->type = 2;	/* specific_charging_units */

		if (ctrl->debug & PRI_DEBUG_APDU) {
			pri_message(ctrl, "  specificChargingUnits %s\n", asn1_tag2str(tag));
		}
		ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
		ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CHECK_TAG(ctrl, tag, tag,
			ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 1);
		ASN1_CALL(pos, rose_dec_etsi_AOC_RecordedUnitsList(ctrl, "recordedUnitsList",
			tag, pos, seq_end, &aoc_d->specific.recorded));

		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 2);
		ASN1_CALL(pos, asn1_dec_int(ctrl, "typeOfChargingInfo", tag, pos, seq_end,
			&value));
		aoc_d->specific.type_of_charging_info = value;

		if (pos < seq_end && *pos != ASN1_INDEF_TERM) {
			ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
			ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_CLASS_CONTEXT_SPECIFIC | 3);
			ASN1_CALL(pos, asn1_dec_int(ctrl, "billingId", tag, pos, seq_end, &value));
			aoc_d->specific.billing_id = value;
			aoc_d->specific.billing_id_present = 1;
		} else {
			aoc_d->specific.billing_id_present = 0;
		}

		ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	return pos;
}

/*!
 * \brief Decode the AOCECurrency invoke argument parameters.
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
const unsigned char *rose_dec_etsi_AOCECurrency_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	struct roseEtsiAOCECurrency_ARG *aoc_e;

	aoc_e = &args->etsi.AOCECurrency;
	switch (tag) {
	case ASN1_TYPE_NULL:
		aoc_e->type = 0;	/* charge_not_available */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "chargeNotAvailable", tag, pos, end));
		break;
	case ASN1_TAG_SEQUENCE:
		aoc_e->type = 1;	/* currency_info */
		ASN1_CALL(pos, rose_dec_etsi_AOCECurrencyInfo(ctrl, "currencyInfo", tag, pos,
			end, &aoc_e->currency_info));
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	return pos;
}

/*!
 * \brief Decode the AOCEChargingUnit invoke argument parameters.
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
const unsigned char *rose_dec_etsi_AOCEChargingUnit_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	struct roseEtsiAOCEChargingUnit_ARG *aoc_e;

	aoc_e = &args->etsi.AOCEChargingUnit;
	switch (tag) {
	case ASN1_TYPE_NULL:
		aoc_e->type = 0;	/* charge_not_available */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "chargeNotAvailable", tag, pos, end));
		break;
	case ASN1_TAG_SEQUENCE:
		aoc_e->type = 1;	/* charging_unit */
		ASN1_CALL(pos, rose_dec_etsi_AOCEChargingUnitInfo(ctrl, "chargingUnitInfo", tag,
			pos, end, &aoc_e->charging_unit));
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	return pos;
}

/* ------------------------------------------------------------------- */
/* end rose_etsi_aoc.c */
