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
 * \brief Internal definitions and prototypes for ROSE.
 *
 * \author Richard Mudgett <rmudgett@digium.com>
 */

#ifndef _LIBPRI_ROSE_INTERNAL_H
#define _LIBPRI_ROSE_INTERNAL_H

#include "rose.h"

#ifdef __cplusplus
extern "C" {
#endif


/* ------------------------------------------------------------------- */


/* Embedded-Q931-Types */
unsigned char *rose_enc_Q931ie(struct pri *ctrl, unsigned char *pos, unsigned char *end,
	unsigned tag, const struct roseQ931ie *q931ie);

const unsigned char *rose_dec_Q931ie(struct pri *ctrl, const char *name, unsigned tag,
	const unsigned char *pos, const unsigned char *end, struct roseQ931ie *q931ie,
	size_t contents_size);

/* Addressing-Data-Elements */
unsigned char *rose_enc_PartyNumber(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct rosePartyNumber *party_number);
unsigned char *rose_enc_PartySubaddress(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct rosePartySubaddress *party_subaddress);
unsigned char *rose_enc_Address(struct pri *ctrl, unsigned char *pos, unsigned char *end,
	unsigned tag, const struct roseAddress *address);
unsigned char *rose_enc_PresentedNumberUnscreened(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct rosePresentedNumberUnscreened *party);
unsigned char *rose_enc_NumberScreened(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, unsigned tag, const struct roseNumberScreened *screened);
unsigned char *rose_enc_PresentedNumberScreened(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct rosePresentedNumberScreened *party);
unsigned char *rose_enc_AddressScreened(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, unsigned tag, const struct roseAddressScreened *screened);
unsigned char *rose_enc_PresentedAddressScreened(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct rosePresentedAddressScreened *party);

const unsigned char *rose_dec_PartyNumber(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct rosePartyNumber *party_number);
const unsigned char *rose_dec_PartySubaddress(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct rosePartySubaddress *party_subaddress);
const unsigned char *rose_dec_Address(struct pri *ctrl, const char *name, unsigned tag,
	const unsigned char *pos, const unsigned char *end, struct roseAddress *address);
const unsigned char *rose_dec_PresentedNumberUnscreened(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct rosePresentedNumberUnscreened *party);
const unsigned char *rose_dec_NumberScreened(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseNumberScreened *screened);
const unsigned char *rose_dec_PresentedNumberScreened(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct rosePresentedNumberScreened *party);
const unsigned char *rose_dec_AddressScreened(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseAddressScreened *screened);
const unsigned char *rose_dec_PresentedAddressScreened(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct rosePresentedAddressScreened *party);

/* ETSI Advice-of-Charge (AOC) */
unsigned char *rose_enc_etsi_ChargingRequest_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_ChargingRequest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args);
unsigned char *rose_enc_etsi_AOCSCurrency_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_AOCSSpecialArr_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_AOCDCurrency_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_AOCDChargingUnit_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_AOCECurrency_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_AOCEChargingUnit_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);

const unsigned char *rose_dec_etsi_ChargingRequest_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_ChargingRequest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);
const unsigned char *rose_dec_etsi_AOCSCurrency_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_AOCSSpecialArr_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_AOCDCurrency_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_AOCDChargingUnit_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_AOCECurrency_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_AOCEChargingUnit_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);

/* ETSI Call Diversion */
unsigned char *rose_enc_etsi_ActivationDiversion_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_DeactivationDiversion_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_ActivationStatusNotificationDiv_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_DeactivationStatusNotificationDiv_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_InterrogationDiversion_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_InterrogationDiversion_RES(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_result_args *args);
unsigned char *rose_enc_etsi_DiversionInformation_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_CallDeflection_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_CallRerouting_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_InterrogateServedUserNumbers_RES(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_result_args *args);
unsigned char *rose_enc_etsi_DivertingLegInformation1_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_DivertingLegInformation2_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_DivertingLegInformation3_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args);

const unsigned char *rose_dec_etsi_ActivationDiversion_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_DeactivationDiversion_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_ActivationStatusNotificationDiv_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_DeactivationStatusNotificationDiv_ARG(struct pri
	*ctrl, unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_InterrogationDiversion_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_InterrogationDiversion_RES(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);
const unsigned char *rose_dec_etsi_DiversionInformation_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_CallDeflection_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_CallRerouting_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_InterrogateServedUserNumbers_RES(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);
const unsigned char *rose_dec_etsi_DivertingLegInformation1_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_DivertingLegInformation2_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_DivertingLegInformation3_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);

/* ETSI Explicit Call Transfer (ECT) */
unsigned char *rose_enc_etsi_ExplicitEctExecute_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_SubaddressTransfer_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_EctLinkIdRequest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args);
unsigned char *rose_enc_etsi_EctInform_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_EctLoopTest_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_EctLoopTest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args);

const unsigned char *rose_dec_etsi_ExplicitEctExecute_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_SubaddressTransfer_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_EctLinkIdRequest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);
const unsigned char *rose_dec_etsi_EctInform_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_EctLoopTest_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_EctLoopTest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);

/* ETSI Status Request */
unsigned char *rose_enc_etsi_StatusRequest_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_StatusRequest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args);

const unsigned char *rose_dec_etsi_StatusRequest_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_StatusRequest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);

/* ETSI Call-Completion Busy Status (CCBS) / Call-Completion No Reply (CCNR) */
unsigned char *rose_enc_etsi_CallInfoRetain_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_EraseCallLinkageID_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_CCBSDeactivate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_CCBSErase_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_CCBSRemoteUserFree_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_CCBSCall_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_CCBSBFree_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_CCBSStopAlerting_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_CCBSStatusRequest_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_CCBSStatusRequest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args);
unsigned char *rose_enc_etsi_CCBSRequest_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_CCNRRequest_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_CCBSRequest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args);
unsigned char *rose_enc_etsi_CCNRRequest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args);
unsigned char *rose_enc_etsi_CCBSInterrogate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_CCNRInterrogate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_CCBSInterrogate_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args);
unsigned char *rose_enc_etsi_CCNRInterrogate_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args);

const unsigned char *rose_dec_etsi_CallInfoRetain_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_EraseCallLinkageID_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_CCBSDeactivate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_CCBSErase_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_CCBSRemoteUserFree_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_CCBSCall_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_CCBSBFree_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_CCBSStopAlerting_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_CCBSStatusRequest_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_CCBSStatusRequest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);
const unsigned char *rose_dec_etsi_CCBSRequest_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_CCNRRequest_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_CCBSRequest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);
const unsigned char *rose_dec_etsi_CCNRRequest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);
const unsigned char *rose_dec_etsi_CCBSInterrogate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_CCNRInterrogate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_CCBSInterrogate_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);
const unsigned char *rose_dec_etsi_CCNRInterrogate_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);

/* ETSI CCBS-T/CCNR-T */
unsigned char *rose_enc_etsi_CCBS_T_Request_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_CCNR_T_Request_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_CCBS_T_Request_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args);
unsigned char *rose_enc_etsi_CCNR_T_Request_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args);

const unsigned char *rose_dec_etsi_CCBS_T_Request_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_CCNR_T_Request_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_CCBS_T_Request_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);
const unsigned char *rose_dec_etsi_CCNR_T_Request_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);

/* ETSI Message Waiting Indication (MWI) */
unsigned char *rose_enc_etsi_MWIActivate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_MWIDeactivate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_etsi_MWIIndicate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);

const unsigned char *rose_dec_etsi_MWIActivate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_MWIDeactivate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_etsi_MWIIndicate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);

/* Q.SIG Name-Operations */
unsigned char *rose_enc_qsig_Name(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct roseQsigName *name);

const unsigned char *rose_dec_qsig_Name(struct pri *ctrl, const char *fname,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigName *name);

unsigned char *rose_enc_qsig_CallingName_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_CalledName_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_ConnectedName_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_BusyName_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);

const unsigned char *rose_dec_qsig_CallingName_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_CalledName_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_ConnectedName_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_BusyName_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);

/*
 * Q.SIG Dummy invoke/result argument used by:
 * SS-AOC-Operations,
 * Call-Transfer-Operations,
 * Call-Diversion-Operations,
 * and SS-MWI-Operations.
 */
unsigned char *rose_enc_qsig_DummyArg_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_DummyRes_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args);

const unsigned char *rose_dec_qsig_DummyArg_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_DummyRes_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);

/* Q.SIG SS-AOC-Operations */
unsigned char *rose_enc_qsig_ChargeRequest_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_ChargeRequest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args);
unsigned char *rose_enc_qsig_AocFinal_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_AocInterim_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_AocRate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_AocComplete_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_AocComplete_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args);
unsigned char *rose_enc_qsig_AocDivChargeReq_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);

const unsigned char *rose_dec_qsig_ChargeRequest_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_ChargeRequest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);
const unsigned char *rose_dec_qsig_AocFinal_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_AocInterim_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_AocRate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_AocComplete_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_AocComplete_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);
const unsigned char *rose_dec_qsig_AocDivChargeReq_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);

/* Q.SIG Call-Diversion-Operations */
unsigned char *rose_enc_qsig_ActivateDiversionQ_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_DeactivateDiversionQ_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_InterrogateDiversionQ_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_InterrogateDiversionQ_RES(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_result_args *args);
unsigned char *rose_enc_qsig_CheckRestriction_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_CallRerouting_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_DivertingLegInformation1_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_DivertingLegInformation2_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_DivertingLegInformation3_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args);

const unsigned char *rose_dec_qsig_ActivateDiversionQ_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_DeactivateDiversionQ_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_InterrogateDiversionQ_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_InterrogateDiversionQ_RES(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);
const unsigned char *rose_dec_qsig_CheckRestriction_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_CallRerouting_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_DivertingLegInformation1_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_DivertingLegInformation2_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_DivertingLegInformation3_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);

/* Q.SIG Call-Transfer-Operations (CT) */
unsigned char *rose_enc_qsig_CallTransferIdentify_RES(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_result_args *args);
unsigned char *rose_enc_qsig_CallTransferInitiate_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_CallTransferSetup_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_CallTransferActive_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_CallTransferComplete_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_CallTransferUpdate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_SubaddressTransfer_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);

const unsigned char *rose_dec_qsig_CallTransferIdentify_RES(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);
const unsigned char *rose_dec_qsig_CallTransferInitiate_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_CallTransferSetup_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_CallTransferActive_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_CallTransferComplete_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_CallTransferUpdate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_SubaddressTransfer_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);

/* Q.SIG SS-CC-Operations */
unsigned char *rose_enc_qsig_CcbsRequest_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_CcnrRequest_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_CcbsRequest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args);
unsigned char *rose_enc_qsig_CcnrRequest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args);
unsigned char *rose_enq_qsig_CcCancel_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enq_qsig_CcExecPossible_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_CcPathReserve_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_CcPathReserve_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args);
unsigned char *rose_enc_qsig_CcRingout_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_CcSuspend_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_CcResume_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);

const unsigned char *rose_dec_qsig_CcbsRequest_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_CcnrRequest_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_CcbsRequest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);
const unsigned char *rose_dec_qsig_CcnrRequest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);
const unsigned char *rose_dec_qsig_CcCancel_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_CcExecPossible_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_CcPathReserve_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_CcPathReserve_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);
const unsigned char *rose_dec_qsig_CcRingout_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_CcSuspend_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_CcResume_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);

/* Q.SIG SS-MWI-Operations */
unsigned char *rose_enc_qsig_MWIActivate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_MWIDeactivate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_MWIInterrogate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_qsig_MWIInterrogate_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args);

const unsigned char *rose_dec_qsig_MWIActivate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_MWIDeactivate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_MWIInterrogate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_qsig_MWIInterrogate_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);

/* Northern Telecom DMS-100 operations */
unsigned char *rose_enc_dms100_RLT_OperationInd_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args);
unsigned char *rose_enc_dms100_RLT_ThirdParty_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);

const unsigned char *rose_dec_dms100_RLT_OperationInd_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_result_args *args);
const unsigned char *rose_dec_dms100_RLT_ThirdParty_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);

/* National ISDN 2 (NI2) operations */
unsigned char *rose_enc_ni2_InformationFollowing_ARG(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const union rose_msg_invoke_args *args);
unsigned char *rose_enc_ni2_InitiateTransfer_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args);

const unsigned char *rose_dec_ni2_InformationFollowing_ARG(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);
const unsigned char *rose_dec_ni2_InitiateTransfer_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args);


/* ------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif	/* _LIBPRI_ROSE_INTERNAL_H */
/* ------------------------------------------------------------------- */
/* end rose_internal.h */
