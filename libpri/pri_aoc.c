/*
 * libpri: An implementation of Primary Rate ISDN
 *
 * Copyright (C) 2010 Digium, Inc.
 *
 * Richard Mudgett <rmudgett@digium.com>
 * David Vossel <dvossel@digium.com>
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
 * \brief Advice Of Charge (AOC) facility support.
 *
 * \author Richard Mudgett <rmudgett@digium.com>
 */


#include "compat.h"
#include "libpri.h"
#include "pri_internal.h"
#include "pri_facility.h"


/* ------------------------------------------------------------------- */

/*!
 * \internal
 * \brief Fill in the AOC subcmd amount from the ETSI amount.
 *
 * \param subcmd_amount AOC subcmd amount.
 * \param etsi_amount AOC ETSI amount.
 *
 * \return Nothing
 */
static void aoc_etsi_subcmd_amount(struct pri_aoc_amount *subcmd_amount, const struct roseEtsiAOCAmount *etsi_amount)
{
	subcmd_amount->cost = etsi_amount->currency;
	subcmd_amount->multiplier = etsi_amount->multiplier;
}

/*!
 * \internal
 * \brief Fill in the ETSI amount from the AOC subcmd amount.
 *
 * \param subcmd_amount AOC subcmd amount.
 * \param etsi_amount AOC ETSI amount.
 *
 * \return Nothing
 */
static void aoc_enc_etsi_subcmd_amount(const struct pri_aoc_amount *subcmd_amount, struct roseEtsiAOCAmount *etsi_amount)
{
	etsi_amount->currency = subcmd_amount->cost;
	etsi_amount->multiplier = subcmd_amount->multiplier;
}

/*!
 * \internal
 * \brief Fill in the AOC subcmd time from the ETSI time.
 *
 * \param subcmd_time AOC subcmd time.
 * \param etsi_time AOC ETSI time.
 *
 * \return Nothing
 */
static void aoc_etsi_subcmd_time(struct pri_aoc_time *subcmd_time, const struct roseEtsiAOCTime *etsi_time)
{
	subcmd_time->length = etsi_time->length;
	subcmd_time->scale = etsi_time->scale;
}

/*!
 * \internal
 * \brief Fill in the ETSI Time from the AOC subcmd time.
 *
 * \param subcmd_time AOC subcmd time.
 * \param etsi_time AOC ETSI time.
 *
 * \return Nothing
 */
static void aoc_enc_etsi_subcmd_time(const struct pri_aoc_time *subcmd_time, struct roseEtsiAOCTime *etsi_time)
{
	etsi_time->length = subcmd_time->length;
	etsi_time->scale = subcmd_time->scale;
}

/*!
 * \internal
 * \brief Fill in the AOC subcmd recorded currency from the ETSI recorded currency.
 *
 * \param subcmd_recorded AOC subcmd recorded currency.
 * \param etsi_recorded AOC ETSI recorded currency.
 *
 * \return Nothing
 */
static void aoc_etsi_subcmd_recorded_currency(struct pri_aoc_recorded_currency *subcmd_recorded, const struct roseEtsiAOCRecordedCurrency *etsi_recorded)
{
	aoc_etsi_subcmd_amount(&subcmd_recorded->amount, &etsi_recorded->amount);
	libpri_copy_string(subcmd_recorded->currency, (char *) etsi_recorded->currency,
		sizeof(subcmd_recorded->currency));
}

/*!
 * \internal
 * \brief Fill in the the ETSI recorded currency from the subcmd currency info
 *
 * \param subcmd_recorded AOC subcmd recorded currency.
 * \param etsi_recorded AOC ETSI recorded currency.
 *
 * \return Nothing
 */
static void aoc_enc_etsi_subcmd_recorded_currency(const struct pri_aoc_recorded_currency *subcmd_recorded, struct roseEtsiAOCRecordedCurrency *etsi_recorded)
{
	aoc_enc_etsi_subcmd_amount(&subcmd_recorded->amount, &etsi_recorded->amount);
	libpri_copy_string((char *) etsi_recorded->currency,
			subcmd_recorded->currency,
			sizeof(etsi_recorded->currency));
}

/*!
 * \internal
 * \brief Fill in the AOC subcmd recorded units from the ETSI recorded units.
 *
 * \param subcmd_recorded AOC subcmd recorded units list.
 * \param etsi_recorded AOC ETSI recorded units list.
 *
 * \return Nothing
 */
static void aoc_etsi_subcmd_recorded_units(struct pri_aoc_recorded_units *subcmd_recorded, const struct roseEtsiAOCRecordedUnitsList *etsi_recorded)
{
	int idx;

	/* Fill in the itemized list of recorded units. */
	for (idx = 0; idx < etsi_recorded->num_records
		&& idx < ARRAY_LEN(subcmd_recorded->item); ++idx) {
		if (etsi_recorded->list[idx].not_available) {
			subcmd_recorded->item[idx].number = -1;
		} else {
			subcmd_recorded->item[idx].number = etsi_recorded->list[idx].number_of_units;
		}
		if (etsi_recorded->list[idx].type_of_unit_present) {
			subcmd_recorded->item[idx].type = etsi_recorded->list[idx].type_of_unit;
		} else {
			subcmd_recorded->item[idx].type = -1;
		}
	}
	subcmd_recorded->num_items = idx;
}

/*!
 * \internal
 * \brief Fill in the ETSI recorded units from the AOC subcmd recorded units.
 *
 * \param subcmd_recorded AOC subcmd recorded units list.
 * \param etsi_recorded AOC ETSI recorded units list.
 *
 * \return Nothing
 */
static void aoc_enc_etsi_subcmd_recorded_units(const struct pri_aoc_recorded_units *subcmd_recorded, struct roseEtsiAOCRecordedUnitsList *etsi_recorded)
{
	int i;

	/* Fill in the itemized list of recorded units. */
	for (i = 0; i < subcmd_recorded->num_items; i++) {
		if (subcmd_recorded->item[i].number >= 0) {
			etsi_recorded->list[i].number_of_units = subcmd_recorded->item[i].number;
		} else {
			etsi_recorded->list[i].not_available = 1;
		}
		if (subcmd_recorded->item[i].type > 0) {
			etsi_recorded->list[i].type_of_unit = subcmd_recorded->item[i].type;
			etsi_recorded->list[i].type_of_unit_present = 1;
		}
	}
	etsi_recorded->num_records = i;

	if (!etsi_recorded->num_records) {
		etsi_recorded->list[0].not_available = 1;
		etsi_recorded->list[0].type_of_unit_present = 0;
		etsi_recorded->num_records = 1;
	}
}

/*!
 * \brief Handle the ETSI ChargingRequest.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Q.931 call leg.
 * \param invoke Decoded ROSE invoke message contents.
 *
 * \return Nothing
 */
void aoc_etsi_aoc_request(struct pri *ctrl, q931_call *call, const struct rose_msg_invoke *invoke)
{
	struct pri_subcommand *subcmd;
	int request;

	if (!ctrl->aoc_support) {
		send_facility_error(ctrl, call, invoke->invoke_id, ROSE_ERROR_Gen_NotSubscribed);
		return;
	}
	switch (invoke->args.etsi.ChargingRequest.charging_case) {
	case 0:/* chargingInformationAtCallSetup */
		request = PRI_AOC_REQUEST_S;
		break;
	case 1:/* chargingDuringACall */
		request = PRI_AOC_REQUEST_D;
		break;
	case 2:/* chargingAtTheEndOfACall */
		request = PRI_AOC_REQUEST_E;
		break;
	default:
		send_facility_error(ctrl, call, invoke->invoke_id, ROSE_ERROR_Gen_NotImplemented);
		return;
	}

	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		send_facility_error(ctrl, call, invoke->invoke_id, ROSE_ERROR_Gen_NotAvailable);
		return;
	}

	subcmd->cmd = PRI_SUBCMD_AOC_CHARGING_REQ;
	subcmd->u.aoc_request.invoke_id = invoke->invoke_id;
	subcmd->u.aoc_request.charging_request = request;
}

/*!
 * \internal
 * \brief Fill in the AOC-S subcmd currency info list of chargeable items.
 *
 * \param aoc_s AOC-S info list of chargeable items.
 * \param info ETSI info list of chargeable items.
 *
 * \return Nothing
 */
static void aoc_etsi_subcmd_aoc_s_currency_info(struct pri_subcmd_aoc_s *aoc_s, const struct roseEtsiAOCSCurrencyInfoList *info)
{
	int idx;

	/* Fill in the itemized list of chargeable items. */
	for (idx = 0; idx < info->num_records && idx < ARRAY_LEN(aoc_s->item); ++idx) {
		/* What is being charged. */
		switch (info->list[idx].charged_item) {
		case 0:/* basicCommunication */
			aoc_s->item[idx].chargeable = PRI_AOC_CHARGED_ITEM_BASIC_COMMUNICATION;
			break;
		case 1:/* callAttempt */
			aoc_s->item[idx].chargeable = PRI_AOC_CHARGED_ITEM_CALL_ATTEMPT;
			break;
		case 2:/* callSetup */
			aoc_s->item[idx].chargeable = PRI_AOC_CHARGED_ITEM_CALL_SETUP;
			break;
		case 3:/*  userToUserInfo */
			aoc_s->item[idx].chargeable = PRI_AOC_CHARGED_ITEM_USER_USER_INFO;
			break;
		case 4:/* operationOfSupplementaryServ */
			aoc_s->item[idx].chargeable = PRI_AOC_CHARGED_ITEM_SUPPLEMENTARY_SERVICE;
			break;
		default:
			aoc_s->item[idx].chargeable = PRI_AOC_CHARGED_ITEM_NOT_AVAILABLE;
			break;
		}

		/* Rate method being used. */
		switch (info->list[idx].currency_type) {
		case 0:/* specialChargingCode */
			aoc_s->item[idx].rate_type = PRI_AOC_RATE_TYPE_SPECIAL_CODE;
			aoc_s->item[idx].rate.special = info->list[idx].u.special_charging_code;
			break;
		case 1:/* durationCurrency */
			aoc_s->item[idx].rate_type = PRI_AOC_RATE_TYPE_DURATION;
			aoc_etsi_subcmd_amount(&aoc_s->item[idx].rate.duration.amount,
				&info->list[idx].u.duration.amount);
			aoc_etsi_subcmd_time(&aoc_s->item[idx].rate.duration.time,
				&info->list[idx].u.duration.time);
			if (info->list[idx].u.duration.granularity_present) {
				aoc_etsi_subcmd_time(&aoc_s->item[idx].rate.duration.granularity,
					&info->list[idx].u.duration.granularity);
			} else {
				aoc_s->item[idx].rate.duration.granularity.length = 0;
				aoc_s->item[idx].rate.duration.granularity.scale =
					PRI_AOC_TIME_SCALE_HUNDREDTH_SECOND;
			}
			aoc_s->item[idx].rate.duration.charging_type =
				info->list[idx].u.duration.charging_type;
			libpri_copy_string(aoc_s->item[idx].rate.duration.currency,
				(char *) info->list[idx].u.duration.currency,
				sizeof(aoc_s->item[idx].rate.duration.currency));
			break;
		case 2:/* flatRateCurrency */
			aoc_s->item[idx].rate_type = PRI_AOC_RATE_TYPE_FLAT;
			aoc_etsi_subcmd_amount(&aoc_s->item[idx].rate.flat.amount,
				&info->list[idx].u.flat_rate.amount);
			libpri_copy_string(aoc_s->item[idx].rate.flat.currency,
				(char *) info->list[idx].u.flat_rate.currency,
				sizeof(aoc_s->item[idx].rate.flat.currency));
			break;
		case 3:/* volumeRateCurrency */
			aoc_s->item[idx].rate_type = PRI_AOC_RATE_TYPE_VOLUME;
			aoc_etsi_subcmd_amount(&aoc_s->item[idx].rate.volume.amount,
				&info->list[idx].u.volume_rate.amount);
			aoc_s->item[idx].rate.volume.unit = info->list[idx].u.volume_rate.unit;
			libpri_copy_string(aoc_s->item[idx].rate.volume.currency,
				(char *) info->list[idx].u.volume_rate.currency,
				sizeof(aoc_s->item[idx].rate.volume.currency));
			break;
		case 4:/* freeOfCharge */
			aoc_s->item[idx].rate_type = PRI_AOC_RATE_TYPE_FREE;
			break;
		default:
		case 5:/* currencyInfoNotAvailable */
			aoc_s->item[idx].rate_type = PRI_AOC_RATE_TYPE_NOT_AVAILABLE;
			break;
		}
	}
	aoc_s->num_items = idx;
}

/*!
 * \internal
 * \brief Fill in the currency info list of chargeable items from a aoc_s subcmd
 *
 * \param aoc_s AOC-S info list of chargeable items.
 * \param info ETSI info list of chargeable items.
 *
 * \return Nothing
 */
static void enc_etsi_subcmd_aoc_s_currency_info(const struct pri_subcmd_aoc_s *aoc_s, struct roseEtsiAOCSCurrencyInfoList *info)
{
	int idx;

	for (idx = 0; idx < aoc_s->num_items && idx < ARRAY_LEN(info->list); ++idx) {
		/* What is being charged. */
		switch (aoc_s->item[idx].chargeable) {
		default:
		case PRI_AOC_CHARGED_ITEM_BASIC_COMMUNICATION:
			info->list[idx].charged_item = 0;/* basicCommunication */
			break;
		case PRI_AOC_CHARGED_ITEM_CALL_ATTEMPT:
			info->list[idx].charged_item = 1;/* callAttempt */
			break;
		case PRI_AOC_CHARGED_ITEM_CALL_SETUP:
			info->list[idx].charged_item = 2;/* callSetup */
			break;
		case PRI_AOC_CHARGED_ITEM_USER_USER_INFO:
			info->list[idx].charged_item = 3;/* userToUserInfo */
			break;
		case PRI_AOC_CHARGED_ITEM_SUPPLEMENTARY_SERVICE:
			info->list[idx].charged_item = 4;/* operationOfSupplementaryServ */
			break;
		}

		/* Rate method being used. */
		switch (aoc_s->item[idx].rate_type) {
		case PRI_AOC_RATE_TYPE_SPECIAL_CODE:
			info->list[idx].currency_type = 0;/* specialChargingCode */
			info->list[idx].u.special_charging_code = aoc_s->item[idx].rate.special;
			break;
		case PRI_AOC_RATE_TYPE_DURATION:
			info->list[idx].currency_type = 1;/* durationCurrency */
			aoc_enc_etsi_subcmd_amount(&aoc_s->item[idx].rate.duration.amount,
				&info->list[idx].u.duration.amount);
			aoc_enc_etsi_subcmd_time(&aoc_s->item[idx].rate.duration.time,
				&info->list[idx].u.duration.time);
			if (aoc_s->item[idx].rate.duration.granularity.length) {
				info->list[idx].u.duration.granularity_present = 1;
				aoc_enc_etsi_subcmd_time(&aoc_s->item[idx].rate.duration.granularity,
					&info->list[idx].u.duration.granularity);
			} else {
				info->list[idx].u.duration.granularity_present = 0;
			}
			info->list[idx].u.duration.charging_type = aoc_s->item[idx].rate.duration.charging_type;
			libpri_copy_string((char *) info->list[idx].u.duration.currency,
				aoc_s->item[idx].rate.duration.currency,
				sizeof((char *) info->list[idx].u.duration.currency));
			break;
		case PRI_AOC_RATE_TYPE_FLAT:
			info->list[idx].currency_type = 2;/* flatRateCurrency */
			aoc_enc_etsi_subcmd_amount(&aoc_s->item[idx].rate.flat.amount,
				&info->list[idx].u.flat_rate.amount);
			libpri_copy_string((char *) info->list[idx].u.flat_rate.currency,
				aoc_s->item[idx].rate.flat.currency,
				sizeof((char *) info->list[idx].u.flat_rate.currency));
			break;
		case PRI_AOC_RATE_TYPE_VOLUME:
			info->list[idx].currency_type = 3;/* volumeRateCurrency */
			aoc_enc_etsi_subcmd_amount(&aoc_s->item[idx].rate.volume.amount,
				&info->list[idx].u.volume_rate.amount);
			info->list[idx].u.volume_rate.unit = aoc_s->item[idx].rate.volume.unit;
			libpri_copy_string((char *) info->list[idx].u.volume_rate.currency,
				aoc_s->item[idx].rate.volume.currency,
				sizeof((char *) info->list[idx].u.volume_rate.currency));
			break;
		case PRI_AOC_RATE_TYPE_FREE:
			info->list[idx].currency_type = 4;/* freeOfCharge */
			break;
		default:
		case PRI_AOC_RATE_TYPE_NOT_AVAILABLE:
			info->list[idx].currency_type = 5;/* currencyInfoNotAvailable */
			break;
		}
	}
	if (!idx) {
		/* We cannot send an empty list so create a dummy list element. */
		info->list[idx].charged_item = 0;/* basicCommunication */
		info->list[idx].currency_type = 5;/* currencyInfoNotAvailable */
		++idx;
	}
	info->num_records = idx;
}

/*!
 * \brief Handle the ETSI AOCSCurrency message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param invoke Decoded ROSE invoke message contents.
 *
 * \return Nothing
 */
void aoc_etsi_aoc_s_currency(struct pri *ctrl, const struct rose_msg_invoke *invoke)
{
	struct pri_subcommand *subcmd;

	if (!ctrl->aoc_support) {
		return;
	}
	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		return;
	}

	subcmd->cmd = PRI_SUBCMD_AOC_S;
	if (!invoke->args.etsi.AOCSCurrency.type) {
		subcmd->u.aoc_s.num_items = 0;
		return;
	}

	/* Fill in the itemized list of chargeable items. */
	aoc_etsi_subcmd_aoc_s_currency_info(&subcmd->u.aoc_s,
		&invoke->args.etsi.AOCSCurrency.currency_info);
}

/*!
 * \brief Handle the ETSI AOCSSpecialArr message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param invoke Decoded ROSE invoke message contents.
 *
 * \return Nothing
 */
void aoc_etsi_aoc_s_special_arrangement(struct pri *ctrl, const struct rose_msg_invoke *invoke)
{
	struct pri_subcommand *subcmd;

	if (!ctrl->aoc_support) {
		return;
	}
	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		return;
	}
	subcmd->cmd = PRI_SUBCMD_AOC_S;
	if (!invoke->args.etsi.AOCSSpecialArr.type) {
		subcmd->u.aoc_s.num_items = 0;
		return;
	}
	subcmd->u.aoc_s.num_items = 1;
	subcmd->u.aoc_s.item[0].chargeable = PRI_AOC_CHARGED_ITEM_SPECIAL_ARRANGEMENT;
	subcmd->u.aoc_s.item[0].rate_type = PRI_AOC_RATE_TYPE_SPECIAL_CODE;
	subcmd->u.aoc_s.item[0].rate.special =
		invoke->args.etsi.AOCSSpecialArr.special_arrangement;
}

/*!
 * \internal
 * \brief Determine the AOC-D subcmd billing_id value.
 *
 * \param billing_id_present TRUE if billing_id valid.
 * \param billing_id ETSI billing id from ROSE.
 *
 * \return enum PRI_AOC_D_BILLING_ID value
 */
static enum PRI_AOC_D_BILLING_ID aoc_etsi_subcmd_aoc_d_billing_id(int billing_id_present, int billing_id)
{
	enum PRI_AOC_D_BILLING_ID value;

	if (billing_id_present) {
		switch (billing_id) {
		case 0:/* normalCharging */
			value = PRI_AOC_D_BILLING_ID_NORMAL;
			break;
		case 1:/* reverseCharging */
			value = PRI_AOC_D_BILLING_ID_REVERSE;
			break;
		case 2:/* creditCardCharging */
			value = PRI_AOC_D_BILLING_ID_CREDIT_CARD;
			break;
		default:
			value = PRI_AOC_D_BILLING_ID_NOT_AVAILABLE;
			break;
		}
	} else {
		value = PRI_AOC_D_BILLING_ID_NOT_AVAILABLE;
	}
	return value;
}

/*!
 * \brief Handle the ETSI AOCDCurrency message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param invoke Decoded ROSE invoke message contents.
 *
 * \return Nothing
 */
void aoc_etsi_aoc_d_currency(struct pri *ctrl, const struct rose_msg_invoke *invoke)
{
	struct pri_subcommand *subcmd;

	if (!ctrl->aoc_support) {
		return;
	}
	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		return;
	}

	subcmd->cmd = PRI_SUBCMD_AOC_D;
	switch (invoke->args.etsi.AOCDCurrency.type) {
	default:
	case 0:/* charge_not_available */
		subcmd->u.aoc_d.charge = PRI_AOC_DE_CHARGE_NOT_AVAILABLE;
		break;
	case 1:/* free_of_charge */
		subcmd->u.aoc_d.charge = PRI_AOC_DE_CHARGE_FREE;
		break;
	case 2:/* specific_currency */
		subcmd->u.aoc_d.charge = PRI_AOC_DE_CHARGE_CURRENCY;
		aoc_etsi_subcmd_recorded_currency(&subcmd->u.aoc_d.recorded.money,
			&invoke->args.etsi.AOCDCurrency.specific.recorded);
		subcmd->u.aoc_d.billing_accumulation =
			invoke->args.etsi.AOCDCurrency.specific.type_of_charging_info;
		subcmd->u.aoc_d.billing_id = aoc_etsi_subcmd_aoc_d_billing_id(
			invoke->args.etsi.AOCDCurrency.specific.billing_id_present,
			invoke->args.etsi.AOCDCurrency.specific.billing_id);
		break;
	}
}

/*!
 * \brief Handle the ETSI AOCDChargingUnit message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param invoke Decoded ROSE invoke message contents.
 *
 * \return Nothing
 */
void aoc_etsi_aoc_d_charging_unit(struct pri *ctrl, const struct rose_msg_invoke *invoke)
{
	struct pri_subcommand *subcmd;

	if (!ctrl->aoc_support) {
		return;
	}
	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		return;
	}

	subcmd->cmd = PRI_SUBCMD_AOC_D;
	switch (invoke->args.etsi.AOCDChargingUnit.type) {
	default:
	case 0:/* charge_not_available */
		subcmd->u.aoc_d.charge = PRI_AOC_DE_CHARGE_NOT_AVAILABLE;
		break;
	case 1:/* free_of_charge */
		subcmd->u.aoc_d.charge = PRI_AOC_DE_CHARGE_FREE;
		break;
	case 2:/* specific_charging_units */
		subcmd->u.aoc_d.charge = PRI_AOC_DE_CHARGE_UNITS;
		aoc_etsi_subcmd_recorded_units(&subcmd->u.aoc_d.recorded.unit,
			&invoke->args.etsi.AOCDChargingUnit.specific.recorded);
		subcmd->u.aoc_d.billing_accumulation =
			invoke->args.etsi.AOCDChargingUnit.specific.type_of_charging_info;
		subcmd->u.aoc_d.billing_id = aoc_etsi_subcmd_aoc_d_billing_id(
			invoke->args.etsi.AOCDChargingUnit.specific.billing_id_present,
			invoke->args.etsi.AOCDChargingUnit.specific.billing_id);
		break;
	}
}

/*!
 * \internal
 * \brief Fill in the AOC-E subcmd charging association from the ETSI charging association.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param subcmd_association AOC-E subcmd charging association.
 * \param etsi_association AOC-E ETSI charging association.
 *
 * \return Nothing
 */
static void aoc_etsi_subcmd_aoc_e_charging_association(struct pri *ctrl, struct pri_aoc_e_charging_association *subcmd_association, const struct roseEtsiAOCChargingAssociation *etsi_association)
{
	struct q931_party_number q931_number;

	switch (etsi_association->type) {
	case 0:/* charge_identifier */
		subcmd_association->charging_type = PRI_AOC_E_CHARGING_ASSOCIATION_ID;
		subcmd_association->charge.id = etsi_association->id;
		break;
	case 1:/* charged_number */
		subcmd_association->charging_type = PRI_AOC_E_CHARGING_ASSOCIATION_NUMBER;
		q931_party_number_init(&q931_number);
		rose_copy_number_to_q931(ctrl, &q931_number, &etsi_association->number);
		q931_party_number_copy_to_pri(&subcmd_association->charge.number, &q931_number);
		break;
	default:
		subcmd_association->charging_type = PRI_AOC_E_CHARGING_ASSOCIATION_NOT_AVAILABLE;
		break;
	}
}

/*!
 * \internal
 * \brief Determine the AOC-E subcmd billing_id value.
 *
 * \param billing_id_present TRUE if billing_id valid.
 * \param billing_id ETSI billing id from ROSE.
 *
 * \return enum PRI_AOC_E_BILLING_ID value
 */
static enum PRI_AOC_E_BILLING_ID aoc_etsi_subcmd_aoc_e_billing_id(int billing_id_present, int billing_id)
{
	enum PRI_AOC_E_BILLING_ID value;

	if (billing_id_present) {
		switch (billing_id) {
		case 0:/* normalCharging */
			value = PRI_AOC_E_BILLING_ID_NORMAL;
			break;
		case 1:/* reverseCharging */
			value = PRI_AOC_E_BILLING_ID_REVERSE;
			break;
		case 2:/* creditCardCharging */
			value = PRI_AOC_E_BILLING_ID_CREDIT_CARD;
			break;
		case 3:/* callForwardingUnconditional */
			value = PRI_AOC_E_BILLING_ID_CALL_FORWARDING_UNCONDITIONAL;
			break;
		case 4:/* callForwardingBusy */
			value = PRI_AOC_E_BILLING_ID_CALL_FORWARDING_BUSY;
			break;
		case 5:/* callForwardingNoReply */
			value = PRI_AOC_E_BILLING_ID_CALL_FORWARDING_NO_REPLY;
			break;
		case 6:/* callDeflection */
			value = PRI_AOC_E_BILLING_ID_CALL_DEFLECTION;
			break;
		case 7:/* callTransfer */
			value = PRI_AOC_E_BILLING_ID_CALL_TRANSFER;
			break;
		default:
			value = PRI_AOC_E_BILLING_ID_NOT_AVAILABLE;
			break;
		}
	} else {
		value = PRI_AOC_E_BILLING_ID_NOT_AVAILABLE;
	}
	return value;
}

/*!
 * \internal
 * \brief Determine the ETSI AOC-E billing_id value from the subcmd.
 *
 * \param billing_id from upper layer.
 *
 * \retval -1 failure
 * \retval etsi billing id
 */
static int aoc_subcmd_aoc_e_etsi_billing_id(enum PRI_AOC_E_BILLING_ID billing_id)
{
	switch (billing_id) {
	case PRI_AOC_E_BILLING_ID_NORMAL:
		return 0;/* normalCharging */
	case PRI_AOC_E_BILLING_ID_REVERSE:
		return 1;/* reverseCharging */
	case PRI_AOC_E_BILLING_ID_CREDIT_CARD:
		return 2;/* creditCardCharging */
	case PRI_AOC_E_BILLING_ID_CALL_FORWARDING_UNCONDITIONAL:
		return 3;/* callForwardingUnconditional */
	case PRI_AOC_E_BILLING_ID_CALL_FORWARDING_BUSY:
		return 4;/* callForwardingBusy */
	case PRI_AOC_E_BILLING_ID_CALL_FORWARDING_NO_REPLY:
		return 5;/* callForwardingNoReply */
	case PRI_AOC_E_BILLING_ID_CALL_DEFLECTION:
		return 6;/* callDeflection */
	case PRI_AOC_E_BILLING_ID_CALL_TRANSFER:
		return 7;/* callTransfer */
	case PRI_AOC_E_BILLING_ID_NOT_AVAILABLE:
		break;
	}

	return -1;
}

/*!
 * \internal
 * \brief Determine the ETSI AOC-D billing_id value from the subcmd.
 *
 * \param billing_id from upper layer.
 *
 * \retval -1 failure
 * \retval etsi billing id
 */
static int aoc_subcmd_aoc_d_etsi_billing_id(enum PRI_AOC_D_BILLING_ID billing_id)
{
	switch (billing_id) {
	case PRI_AOC_D_BILLING_ID_NORMAL:
		return 0;/* normalCharging */
	case PRI_AOC_D_BILLING_ID_REVERSE:
		return 1;/* reverseCharging */
	case PRI_AOC_D_BILLING_ID_CREDIT_CARD:
		return 2;/* creditCardCharging */
	case PRI_AOC_D_BILLING_ID_NOT_AVAILABLE:
		break;
	}

	return -1;
}

/*!
 * \brief Handle the ETSI AOCECurrency message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Q.931 call leg.
 * \param invoke Decoded ROSE invoke message contents.
 *
 * \return Nothing
 */
void aoc_etsi_aoc_e_currency(struct pri *ctrl, q931_call *call, const struct rose_msg_invoke *invoke)
{
	struct pri_subcommand *subcmd;

	if (!ctrl->aoc_support) {
		return;
	}
	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		return;
	}

	subcmd->cmd = PRI_SUBCMD_AOC_E;
	subcmd->u.aoc_e.associated.charging_type =
		PRI_AOC_E_CHARGING_ASSOCIATION_NOT_AVAILABLE;

	if (!invoke->args.etsi.AOCECurrency.type) {
		subcmd->u.aoc_e.charge = PRI_AOC_DE_CHARGE_NOT_AVAILABLE;
		return;
	}

	/* Fill in charging association if present. */
	if (invoke->args.etsi.AOCECurrency.currency_info.charging_association_present) {
		aoc_etsi_subcmd_aoc_e_charging_association(ctrl, &subcmd->u.aoc_e.associated,
			&invoke->args.etsi.AOCECurrency.currency_info.charging_association);
	}

	/* Call was free of charge. */
	if (invoke->args.etsi.AOCECurrency.currency_info.free_of_charge) {
		subcmd->u.aoc_e.charge = PRI_AOC_DE_CHARGE_FREE;
		return;
	}

	/* Fill in currency cost of call. */
	subcmd->u.aoc_e.charge = PRI_AOC_DE_CHARGE_CURRENCY;
	aoc_etsi_subcmd_recorded_currency(&subcmd->u.aoc_e.recorded.money,
		&invoke->args.etsi.AOCECurrency.currency_info.specific.recorded);
	subcmd->u.aoc_e.billing_id = aoc_etsi_subcmd_aoc_e_billing_id(
		invoke->args.etsi.AOCECurrency.currency_info.specific.billing_id_present,
		invoke->args.etsi.AOCECurrency.currency_info.specific.billing_id);
}

/*!
 * \brief Handle the ETSI AOCEChargingUnit message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Q.931 call leg.
 * \param invoke Decoded ROSE invoke message contents.
 *
 * \return Nothing
 */
void aoc_etsi_aoc_e_charging_unit(struct pri *ctrl, q931_call *call, const struct rose_msg_invoke *invoke)
{
	struct pri_subcommand *subcmd;
	unsigned idx;

	/* Fill in legacy stuff. */
	call->aoc_units = 0;
	if (invoke->args.etsi.AOCEChargingUnit.type == 1
		&& !invoke->args.etsi.AOCEChargingUnit.charging_unit.free_of_charge) {
		for (idx =
			invoke->args.etsi.AOCEChargingUnit.charging_unit.specific.recorded.num_records;
			idx--;) {
			if (!invoke->args.etsi.AOCEChargingUnit.charging_unit.specific.recorded.
				list[idx].not_available) {
				call->aoc_units +=
					invoke->args.etsi.AOCEChargingUnit.charging_unit.specific.
					recorded.list[idx].number_of_units;
			}
		}
	}

	if (!ctrl->aoc_support) {
		return;
	}
	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		return;
	}

	subcmd->cmd = PRI_SUBCMD_AOC_E;
	subcmd->u.aoc_e.associated.charging_type =
		PRI_AOC_E_CHARGING_ASSOCIATION_NOT_AVAILABLE;

	if (!invoke->args.etsi.AOCEChargingUnit.type) {
		subcmd->u.aoc_e.charge = PRI_AOC_DE_CHARGE_NOT_AVAILABLE;
		return;
	}

	/* Fill in charging association if present. */
	if (invoke->args.etsi.AOCEChargingUnit.charging_unit.charging_association_present) {
		aoc_etsi_subcmd_aoc_e_charging_association(ctrl, &subcmd->u.aoc_e.associated,
			&invoke->args.etsi.AOCEChargingUnit.charging_unit.charging_association);
	}

	/* Call was free of charge. */
	if (invoke->args.etsi.AOCEChargingUnit.charging_unit.free_of_charge) {
		subcmd->u.aoc_e.charge = PRI_AOC_DE_CHARGE_FREE;
		return;
	}

	/* Fill in unit cost of call. */
	subcmd->u.aoc_e.charge = PRI_AOC_DE_CHARGE_UNITS;
	aoc_etsi_subcmd_recorded_units(&subcmd->u.aoc_e.recorded.unit,
		&invoke->args.etsi.AOCEChargingUnit.charging_unit.specific.recorded);
	subcmd->u.aoc_e.billing_id = aoc_etsi_subcmd_aoc_e_billing_id(
		invoke->args.etsi.AOCEChargingUnit.charging_unit.specific.billing_id_present,
		invoke->args.etsi.AOCEChargingUnit.charging_unit.specific.billing_id);
}

void pri_aoc_events_enable(struct pri *ctrl, int enable)
{
	if (ctrl) {
		ctrl->aoc_support = enable ? 1 : 0;
	}
}

/*!
 * \internal
 * \brief Encode the ETSI AOCECurrency invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param aoc_e the AOC-E data to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_aoce_currency(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct pri_subcmd_aoc_e *aoc_e)
{
	struct rose_msg_invoke msg;
	struct q931_party_number q931_number;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_ETSI_AOCECurrency;
	msg.invoke_id = get_invokeid(ctrl);

	if (aoc_e->charge == PRI_AOC_DE_CHARGE_FREE) {
		msg.args.etsi.AOCECurrency.type = 1;	/* currency_info */
		msg.args.etsi.AOCECurrency.currency_info.free_of_charge = 1;
	} else if ((aoc_e->charge == PRI_AOC_DE_CHARGE_CURRENCY) && (aoc_e->recorded.money.amount.cost >= 0)) {
		msg.args.etsi.AOCECurrency.type = 1;	/* currency_info */
		aoc_enc_etsi_subcmd_recorded_currency(&aoc_e->recorded.money,
			&msg.args.etsi.AOCECurrency.currency_info.specific.recorded);
	} else {
		msg.args.etsi.AOCECurrency.type = 0;	/* charge_not_available */
	}

	if (aoc_subcmd_aoc_e_etsi_billing_id(aoc_e->billing_id) != -1) {
		msg.args.etsi.AOCECurrency.currency_info.specific.billing_id_present = 1;
		msg.args.etsi.AOCECurrency.currency_info.specific.billing_id =
			aoc_subcmd_aoc_e_etsi_billing_id(aoc_e->billing_id);
	}

	switch (aoc_e->associated.charging_type) {
	case PRI_AOC_E_CHARGING_ASSOCIATION_NUMBER:
		msg.args.etsi.AOCECurrency.currency_info.charging_association_present = 1;
		msg.args.etsi.AOCECurrency.currency_info.charging_association.type = 1; /* charged_number */
		pri_copy_party_number_to_q931(&q931_number, &aoc_e->associated.charge.number);
		q931_copy_number_to_rose(ctrl,
			&msg.args.etsi.AOCECurrency.currency_info.charging_association.number,
			&q931_number);
		break;
	case PRI_AOC_E_CHARGING_ASSOCIATION_ID:
		msg.args.etsi.AOCECurrency.currency_info.charging_association_present = 1;
		msg.args.etsi.AOCECurrency.currency_info.charging_association.type = 0; /* charge_identifier */
		msg.args.etsi.AOCECurrency.currency_info.charging_association.id =
			aoc_e->associated.charge.id;
		break;
	default:
		/* do nothing */
		break;
	}

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode the ETSI AOCEChargingUnit invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param aoc_e the AOC-E data to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_aoce_charging_unit(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct pri_subcmd_aoc_e *aoc_e)
{
	struct rose_msg_invoke msg;
	struct q931_party_number q931_number;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_ETSI_AOCEChargingUnit;
	msg.invoke_id = get_invokeid(ctrl);

	if (aoc_e->charge == PRI_AOC_DE_CHARGE_FREE) {
		msg.args.etsi.AOCEChargingUnit.type = 1;	/* charging_unit */
		msg.args.etsi.AOCEChargingUnit.charging_unit.free_of_charge = 1;

	} else if ((aoc_e->charge == PRI_AOC_DE_CHARGE_UNITS) &&  (aoc_e->recorded.unit.num_items > 0)) {
		msg.args.etsi.AOCEChargingUnit.type = 1;	/* charging_unit */
		aoc_enc_etsi_subcmd_recorded_units(&aoc_e->recorded.unit,
			&msg.args.etsi.AOCEChargingUnit.charging_unit.specific.recorded);
	} else {
		msg.args.etsi.AOCEChargingUnit.type = 0;	/* charge_not_available */
	}

	if (aoc_subcmd_aoc_e_etsi_billing_id(aoc_e->billing_id) != -1) {
		msg.args.etsi.AOCEChargingUnit.charging_unit.specific.billing_id_present = 1;
		msg.args.etsi.AOCEChargingUnit.charging_unit.specific.billing_id =
			aoc_subcmd_aoc_e_etsi_billing_id(aoc_e->billing_id);
	}

	switch (aoc_e->associated.charging_type) {
	case PRI_AOC_E_CHARGING_ASSOCIATION_NUMBER:
		msg.args.etsi.AOCEChargingUnit.charging_unit.charging_association_present = 1;
		msg.args.etsi.AOCEChargingUnit.charging_unit.charging_association.type = 1; /* charged_number */
		pri_copy_party_number_to_q931(&q931_number, &aoc_e->associated.charge.number);
		q931_copy_number_to_rose(ctrl,
			&msg.args.etsi.AOCEChargingUnit.charging_unit.charging_association.number,
			&q931_number);
		break;
	case PRI_AOC_E_CHARGING_ASSOCIATION_ID:
		msg.args.etsi.AOCEChargingUnit.charging_unit.charging_association_present = 1;
		msg.args.etsi.AOCEChargingUnit.charging_unit.charging_association.type = 0; /* charge_identifier */
		msg.args.etsi.AOCEChargingUnit.charging_unit.charging_association.id =
			aoc_e->associated.charge.id;
		break;
	default:
		/* do nothing */
		break;
	}

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode the ETSI AOCDChargingUnit invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param aoc_d the AOC-D data to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_aocd_charging_unit(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct pri_subcmd_aoc_d *aoc_d)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_ETSI_AOCDChargingUnit;
	msg.invoke_id = get_invokeid(ctrl);

	if (aoc_d->charge == PRI_AOC_DE_CHARGE_FREE) {
		msg.args.etsi.AOCDChargingUnit.type = 1;	/* free_of_charge */
	} else if ((aoc_d->charge == PRI_AOC_DE_CHARGE_UNITS) &&  (aoc_d->recorded.unit.num_items > 0)) {
		msg.args.etsi.AOCDChargingUnit.type = 2;	/* specific_charging_units */
		aoc_enc_etsi_subcmd_recorded_units(&aoc_d->recorded.unit,
			&msg.args.etsi.AOCDChargingUnit.specific.recorded);
	} else {
		msg.args.etsi.AOCDChargingUnit.type = 0;	/* charge_not_available */
	}

	if (aoc_subcmd_aoc_d_etsi_billing_id(aoc_d->billing_id) != -1) {
		msg.args.etsi.AOCDChargingUnit.specific.billing_id_present = 1;
		msg.args.etsi.AOCDChargingUnit.specific.billing_id =
			aoc_subcmd_aoc_d_etsi_billing_id(aoc_d->billing_id);
	}

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode the ETSI AOCDCurrency invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param aoc_d the AOC-D data to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_aocd_currency(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct pri_subcmd_aoc_d *aoc_d)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_ETSI_AOCDCurrency;
	msg.invoke_id = get_invokeid(ctrl);

	if (aoc_d->charge == PRI_AOC_DE_CHARGE_FREE) {
		msg.args.etsi.AOCDCurrency.type = 1;	/* free_of_charge */
	} else if ((aoc_d->charge == PRI_AOC_DE_CHARGE_CURRENCY) && (aoc_d->recorded.money.amount.cost >= 0)) {
		msg.args.etsi.AOCDCurrency.type = 2;	/* specific_currency */
		aoc_enc_etsi_subcmd_recorded_currency(&aoc_d->recorded.money,
			&msg.args.etsi.AOCDCurrency.specific.recorded);
	} else {
		msg.args.etsi.AOCDCurrency.type = 0;	/* charge_not_available */
	}

	if (aoc_subcmd_aoc_d_etsi_billing_id(aoc_d->billing_id) != -1) {
		msg.args.etsi.AOCDCurrency.specific.billing_id_present = 1;
		msg.args.etsi.AOCDCurrency.specific.billing_id =
			aoc_subcmd_aoc_d_etsi_billing_id(aoc_d->billing_id);
	}

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode the ETSI AOCSSpecialArr invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param aoc_s the AOC-S data to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_aocs_special_arrangement(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct pri_subcmd_aoc_s *aoc_s)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_ETSI_AOCSSpecialArr;
	msg.invoke_id = get_invokeid(ctrl);

	if (!aoc_s->num_items || (aoc_s->item[0].rate_type != PRI_AOC_RATE_TYPE_SPECIAL_CODE)) {
		msg.args.etsi.AOCSSpecialArr.type = 0;/* charge_not_available */
	} else {
		msg.args.etsi.AOCSSpecialArr.type = 1;/* special_arrangement_info */
		msg.args.etsi.AOCSSpecialArr.special_arrangement = aoc_s->item[0].rate.special;
	}

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode the ETSI AOCSCurrency invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param aoc_s the AOC-S data to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_aocs_currency(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct pri_subcmd_aoc_s *aoc_s)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_ETSI_AOCSCurrency;
	msg.invoke_id = get_invokeid(ctrl);

	if (aoc_s->num_items) {
		msg.args.etsi.AOCSCurrency.type = 1; /* currency_info_list */
		enc_etsi_subcmd_aoc_s_currency_info(aoc_s, &msg.args.etsi.AOCSCurrency.currency_info);
	} else {
		msg.args.etsi.AOCSCurrency.type = 0; /* charge_not_available */
	}

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode the ETSI ChargingRequest Response message
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param response the response to the request
 * \param invoke_id the request's invoke id
 * \param aoc_s the rate list associated with a response to AOC-S request
 *    Could be NULL.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_aoc_request_response(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, enum PRI_AOC_REQ_RSP response, int invoke_id, const struct pri_subcmd_aoc_s *aoc_s)
{
	struct rose_msg_result msg_result = { 0, };
	struct rose_msg_error msg_error = { 0, };
	int is_error = 0;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	switch (response) {
	case PRI_AOC_REQ_RSP_CURRENCY_INFO_LIST:
		if (!aoc_s) {
			return NULL;
		}
		enc_etsi_subcmd_aoc_s_currency_info(aoc_s, &msg_result.args.etsi.ChargingRequest.u.currency_info);
		msg_result.args.etsi.ChargingRequest.type = 0;/* currency_info_list */
		break;
	case PRI_AOC_REQ_RSP_SPECIAL_ARR:
		if (!aoc_s) {
			return NULL;
		}
		msg_result.args.etsi.ChargingRequest.type = 1;/* special_arrangement_info */
		msg_result.args.etsi.ChargingRequest.u.special_arrangement = aoc_s->item[0].rate.special;
		break;
	case PRI_AOC_REQ_RSP_CHARGING_INFO_FOLLOWS:
		msg_result.args.etsi.ChargingRequest.type = 2;/* charging_info_follows */
		break;
	case PRI_AOC_REQ_RSP_ERROR_NOT_IMPLEMENTED:
		msg_error.code = ROSE_ERROR_Gen_NotImplemented;
		is_error = 1;
		break;
	default:
	case PRI_AOC_REQ_RSP_ERROR_NOT_AVAILABLE:
		is_error = 1;
		msg_error.code = ROSE_ERROR_Gen_NotAvailable;
		break;
	}

	if (is_error) {
		msg_error.invoke_id = invoke_id;
		pos = rose_encode_error(ctrl, pos, end, &msg_error);
	} else {
		msg_result.operation = ROSE_ETSI_ChargingRequest;
		msg_result.invoke_id = invoke_id;
		pos = rose_encode_result(ctrl, pos, end, &msg_result);
	}

	return pos;
}

/*!
 * \internal
 * \brief Encode the ETSI ChargingRequest invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param aoc_request the aoc charging request data to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_aoc_request(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, enum PRI_AOC_REQUEST request)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.operation = ROSE_ETSI_ChargingRequest;
	msg.invoke_id = get_invokeid(ctrl);

	switch (request) {
	case PRI_AOC_REQUEST_S:
		msg.args.etsi.ChargingRequest.charging_case = 0;/* chargingInformationAtCallSetup */
		break;
	case PRI_AOC_REQUEST_D:
		msg.args.etsi.ChargingRequest.charging_case = 1;/* chargingDuringACall */
		break;
	case PRI_AOC_REQUEST_E:
		msg.args.etsi.ChargingRequest.charging_case = 2;/* chargingAtTheEndOfACall */
		break;
	default:
		/* no valid request parameters are present */
		return NULL;
	}

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Send the ETSI AOC Request Response message for an AOC-S request
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode AOC.
 * \param invoke_id the request's invoke id
 * \param aoc_s Optional AOC-S rate list for response
 *
 * \note if aoc_s is NULL, then a response will be sent back as AOC-S not available.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int aoc_s_request_response_encode(struct pri *ctrl, q931_call *call, int invoke_id, const struct pri_subcmd_aoc_s *aoc_s)
{
	unsigned char buffer[255];
	unsigned char *end = NULL;
	int response;

	if (!aoc_s) {
		response = PRI_AOC_REQ_RSP_ERROR_NOT_AVAILABLE;
	} else if (aoc_s->num_items
		&& aoc_s->item[0].chargeable == PRI_AOC_CHARGED_ITEM_SPECIAL_ARRANGEMENT) {
		response = PRI_AOC_REQ_RSP_SPECIAL_ARR;
	} else {
		response = PRI_AOC_REQ_RSP_CURRENCY_INFO_LIST;
	}

	end = enc_etsi_aoc_request_response(ctrl, buffer, buffer + sizeof(buffer), response, invoke_id, aoc_s);
	if (!end) {
		return -1;
	}

	/* Remember that if we queue a facility IE for a facility message we
	 * have to explicitly send the facility message ourselves */
	if (pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL)
		|| q931_facility(call->pri, call)) {
		pri_message(ctrl, "Could not schedule aoc request response facility message for call %d\n", call->cr);
		return -1;
	}

	return 0;
}

/*!
 * \internal
 * \brief Send the ETSI AOC Request Response message for AOC-D and AOC-E requests
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode AOC.
 * \param response the response to the request
 * \param invoke_id the request's invoke id
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int aoc_de_request_response_encode(struct pri *ctrl, q931_call *call, enum PRI_AOC_REQ_RSP response, int invoke_id)
{
	unsigned char buffer[255];
	unsigned char *end = NULL;

	end = enc_etsi_aoc_request_response(ctrl, buffer, buffer + sizeof(buffer), response, invoke_id, NULL);
	if (!end) {
		return -1;
	}

	/* Remember that if we queue a facility IE for a facility message we
	 * have to explicitly send the facility message ourselves */
	if (pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL)
		|| q931_facility(call->pri, call)) {
		pri_message(ctrl, "Could not schedule aoc request response facility message for call %d\n", call->cr);
		return -1;
	}

	return 0;
}

/*!
 * \internal
 * \brief AOC-Request response callback function.
 *
 * \param reason Reason callback is called.
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param apdu APDU queued entry.  Do not change!
 * \param msg APDU response message data.  (NULL if was not the reason called.)
 *
 * \return TRUE if no more responses are expected.
 */
static int pri_aoc_request_get_response(enum APDU_CALLBACK_REASON reason, struct pri *ctrl, struct q931_call *call, struct apdu_event *apdu, const struct apdu_msg_data *msg)
{
	struct pri_subcommand *subcmd;

	if ((reason == APDU_CALLBACK_REASON_ERROR) ||
		(reason == APDU_CALLBACK_REASON_CLEANUP)) {
		return 1;
	}

	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		return 1;
	}

	memset(&subcmd->u.aoc_request_response, 0, sizeof(subcmd->u.aoc_request_response));
	subcmd->u.aoc_request_response.charging_request = apdu->response.user.value;
	subcmd->cmd = PRI_SUBCMD_AOC_CHARGING_REQ_RSP;

	switch (reason) {
	case APDU_CALLBACK_REASON_MSG_ERROR:
		switch (msg->response.error->code) {
		case ROSE_ERROR_Gen_NotImplemented:
			subcmd->u.aoc_request_response.charging_response = PRI_AOC_REQ_RSP_ERROR_NOT_IMPLEMENTED;
			break;
		case ROSE_ERROR_Gen_NotAvailable:
			subcmd->u.aoc_request_response.charging_response = PRI_AOC_REQ_RSP_ERROR_NOT_AVAILABLE;
			break;
		default:
			subcmd->u.aoc_request_response.charging_response = PRI_AOC_REQ_RSP_ERROR;
			break;
		}
		break;
	case APDU_CALLBACK_REASON_MSG_REJECT:
		subcmd->u.aoc_request_response.charging_response = PRI_AOC_REQ_RSP_ERROR_REJECT;
		break;
	case APDU_CALLBACK_REASON_TIMEOUT:
		subcmd->u.aoc_request_response.charging_response = PRI_AOC_REQ_RSP_ERROR_TIMEOUT;
		break;
	case APDU_CALLBACK_REASON_MSG_RESULT:
		switch (msg->response.result->args.etsi.ChargingRequest.type) {
		case 0:/* currency_info_list */
			subcmd->u.aoc_request_response.valid_aoc_s = 1;
			subcmd->u.aoc_request_response.charging_response = PRI_AOC_REQ_RSP_CURRENCY_INFO_LIST;
			aoc_etsi_subcmd_aoc_s_currency_info(&subcmd->u.aoc_request_response.aoc_s,
				&msg->response.result->args.etsi.ChargingRequest.u.currency_info);
			break;
		case 1:/* special_arrangement_info */
			subcmd->u.aoc_request_response.valid_aoc_s = 1;
			subcmd->u.aoc_request_response.charging_response = PRI_AOC_REQ_RSP_SPECIAL_ARR;
			subcmd->u.aoc_request_response.aoc_s.num_items = 1;
			subcmd->u.aoc_request_response.aoc_s.item[0].chargeable = PRI_AOC_CHARGED_ITEM_SPECIAL_ARRANGEMENT;
			subcmd->u.aoc_request_response.aoc_s.item[0].rate_type = PRI_AOC_RATE_TYPE_SPECIAL_CODE;
			subcmd->u.aoc_request_response.aoc_s.item[0].rate.special =
				msg->response.result->args.etsi.ChargingRequest.u.special_arrangement;
			break;
		case 2:/* charging_info_follows */
			subcmd->u.aoc_request_response.charging_response = PRI_AOC_REQ_RSP_CHARGING_INFO_FOLLOWS;
			break;
		default:
			subcmd->u.aoc_request_response.charging_response = PRI_AOC_REQ_RSP_ERROR;
			break;
		}
		break;
	default:
		subcmd->u.aoc_request_response.charging_response = PRI_AOC_REQ_RSP_ERROR;
		break;
	}

	return 1;
}

/*!
 * \internal
 * \brief Send the ETSI AOC Request invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode AOC.
 * \param aoc_request the aoc charging request payload data to encode.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int aoc_charging_request_encode(struct pri *ctrl, q931_call *call, enum PRI_AOC_REQUEST request)
{
	unsigned char buffer[255];
	unsigned char *end = NULL;
	struct apdu_callback_data response;

	end = enc_etsi_aoc_request(ctrl, buffer, buffer + sizeof(buffer), request);
	if (!end) {
		return -1;
	}

	memset(&response, 0, sizeof(response));
	response.invoke_id = ctrl->last_invoke;
	response.timeout_time = APDU_TIMEOUT_MSGS_ONLY;
	response.num_messages = 1;
	response.message_type[0] = Q931_CONNECT;
	response.callback = pri_aoc_request_get_response;
	response.user.value = request;

	/* in the case of an AOC request message, we queue this on a SETUP message and
	 * do not have to send it ourselves in this function */
	return pri_call_apdu_queue(call, Q931_SETUP, buffer, end - buffer, &response);
}

/*!
 * \internal
 * \brief Send the ETSI AOCS invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode AOC.
 * \param aoc_s the AOC-S payload data to encode.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int aoc_s_encode(struct pri *ctrl, q931_call *call, const struct pri_subcmd_aoc_s *aoc_s)
{
	unsigned char buffer[255];
	unsigned char *end = NULL;

	if (aoc_s->item[0].chargeable == PRI_AOC_CHARGED_ITEM_SPECIAL_ARRANGEMENT) {
		end = enc_etsi_aocs_special_arrangement(ctrl, buffer, buffer + sizeof(buffer), aoc_s);
	} else {
		end = enc_etsi_aocs_currency(ctrl, buffer, buffer + sizeof(buffer), aoc_s);
	}
	if (!end) {
		return -1;
	}

	/* Remember that if we queue a facility IE for a facility message we
	 * have to explicitly send the facility message ourselves */
	if (pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL)
		|| q931_facility(call->pri, call)) {
		pri_message(ctrl, "Could not schedule aoc-s facility message for call %d\n", call->cr);
		return -1;
	}

	return 0;
}

/*!
 * \internal
 * \brief Send the ETSI AOCD invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode AOC.
 * \param aoc_d the AOC-D payload data to encode.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int aoc_d_encode(struct pri *ctrl, q931_call *call, const struct pri_subcmd_aoc_d *aoc_d)
{
	unsigned char buffer[255];
	unsigned char *end = NULL;

	switch (aoc_d->charge) {
	case PRI_AOC_DE_CHARGE_NOT_AVAILABLE:
	case PRI_AOC_DE_CHARGE_FREE:
	case PRI_AOC_DE_CHARGE_CURRENCY:
		end = enc_etsi_aocd_currency(ctrl, buffer, buffer + sizeof(buffer), aoc_d);
		break;
	case PRI_AOC_DE_CHARGE_UNITS:
		end = enc_etsi_aocd_charging_unit(ctrl, buffer, buffer + sizeof(buffer), aoc_d);
		break;
	}
	if (!end) {
		return -1;
	}

	/* Remember that if we queue a facility IE for a facility message we
	 * have to explicitly send the facility message ourselves */
	if (pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL)
		|| q931_facility(call->pri, call)) {
		pri_message(ctrl, "Could not schedule aoc-d facility message for call %d\n", call->cr);
		return -1;
	}

	return 0;
}

/*!
 * \internal
 * \brief Send the ETSI AOCE invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode AOC.
 * \param aoc_e the AOC-E payload data to encode.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int aoc_e_encode(struct pri *ctrl, q931_call *call, const struct pri_subcmd_aoc_e *aoc_e)
{
	unsigned char buffer[255];
	unsigned char *end = NULL;

	switch (aoc_e->charge) {
	case PRI_AOC_DE_CHARGE_NOT_AVAILABLE:
	case PRI_AOC_DE_CHARGE_FREE:
	case PRI_AOC_DE_CHARGE_CURRENCY:
		end = enc_etsi_aoce_currency(ctrl, buffer, buffer + sizeof(buffer), aoc_e);
		break;
	case PRI_AOC_DE_CHARGE_UNITS:
		end = enc_etsi_aoce_charging_unit(ctrl, buffer, buffer + sizeof(buffer), aoc_e);
		break;
	}
	if (!end) {
		return -1;
	}

	if (pri_call_apdu_queue(call, Q931_ANY_MESSAGE, buffer, end - buffer, NULL)) {
		pri_message(ctrl, "Could not schedule aoc-e facility message for call %d\n", call->cr);
		return -1;
	}

	return 0;
}

int pri_aoc_de_request_response_send(struct pri *ctrl, q931_call *call, int response, int invoke_id)
{
	if (!ctrl || !pri_is_call_valid(ctrl, call)) {
		return -1;
	}

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		return aoc_de_request_response_encode(ctrl, call, response, invoke_id);
	case PRI_SWITCH_QSIG:
		break;
	default:
		return -1;
	}

	return 0;
}

int pri_aoc_s_request_response_send(struct pri *ctrl, q931_call *call, int invoke_id, const struct pri_subcmd_aoc_s *aoc_s)
{
	if (!ctrl || !pri_is_call_valid(ctrl, call)) {
		return -1;
	}

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		return aoc_s_request_response_encode(ctrl, call, invoke_id, aoc_s);
	case PRI_SWITCH_QSIG:
		break;
	default:
		return -1;
	}

	return 0;
}

/*!
 * \brief Send AOC request message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param aoc types to request
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int aoc_charging_request_send(struct pri *ctrl, q931_call *call, enum PRI_AOC_REQUEST aoc_request_flag)
{
	int res;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		if (BRI_NT_PTMP(ctrl)) {
			/*
			 * We are not setup to handle responses from multiple phones.
			 * Besides, it is silly to ask for AOC from a phone.
			 */
			return -1;
		}
		res = 0;
		if (aoc_request_flag & PRI_AOC_REQUEST_S) {
			res |= aoc_charging_request_encode(ctrl, call, PRI_AOC_REQUEST_S);
		}
		if (aoc_request_flag & PRI_AOC_REQUEST_D) {
			res |= aoc_charging_request_encode(ctrl, call, PRI_AOC_REQUEST_D);
		}
		if (aoc_request_flag & PRI_AOC_REQUEST_E) {
			res |= aoc_charging_request_encode(ctrl, call, PRI_AOC_REQUEST_E);
		}
		return res;
	case PRI_SWITCH_QSIG:
		break;
	default:
		return -1;
	}

	return 0;
}

int pri_aoc_s_send(struct pri *ctrl, q931_call *call, const struct pri_subcmd_aoc_s *aoc_s)
{
	if (!ctrl || !pri_is_call_valid(ctrl, call)) {
		return -1;
	}

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		return aoc_s_encode(ctrl, call, aoc_s);
	case PRI_SWITCH_QSIG:
		break;
	default:
		return -1;
	}

	return 0;
}

int pri_aoc_d_send(struct pri *ctrl, q931_call *call, const struct pri_subcmd_aoc_d *aoc_d)
{
	if (!ctrl || !pri_is_call_valid(ctrl, call)) {
		return -1;
	}

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		return aoc_d_encode(ctrl, call, aoc_d);
	case PRI_SWITCH_QSIG:
		break;
	default:
		return -1;
	}
	return 0;
}

int pri_aoc_e_send(struct pri *ctrl, q931_call *call, const struct pri_subcmd_aoc_e *aoc_e)
{
	if (!ctrl || !pri_is_call_valid(ctrl, call)) {
		return -1;
	}

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		return aoc_e_encode(ctrl, call, aoc_e);
	case PRI_SWITCH_QSIG:
		break;
	default:
		return -1;
	}

	return 0;
}

int pri_sr_set_aoc_charging_request(struct pri_sr *sr, int charging_request)
{
	if (charging_request & PRI_AOC_REQUEST_S) {
		sr->aoc_charging_request |= PRI_AOC_REQUEST_S;
	}
	if (charging_request & PRI_AOC_REQUEST_D) {
		sr->aoc_charging_request |= PRI_AOC_REQUEST_D;
	}
	if (charging_request & PRI_AOC_REQUEST_E) {
		sr->aoc_charging_request |= PRI_AOC_REQUEST_E;
	}

	return 0;
}

/* ------------------------------------------------------------------- */
/* end pri_aoc.c */
