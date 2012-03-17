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
 * \brief ROSE Status-Request/CCBS/CCBS-T/CCNR/CCNR-T operations
 *
 * Status-Request ETS 300 196-1 D.7
 * CCBS Supplementary Services ETS 300 359-1
 * CCNR Supplementary Services ETS 301 065-1
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
 * \brief Encode the array of call information details type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param call_information Call information record to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_etsi_CallInformation(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, unsigned tag,
	const struct roseEtsiCallInformation *call_information)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, rose_enc_Address(ctrl, pos, end, ASN1_TAG_SEQUENCE,
		&call_information->address_of_b));
	ASN1_CALL(pos, rose_enc_Q931ie(ctrl, pos, end, ASN1_CLASS_APPLICATION | 0,
		&call_information->q931ie));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
		call_information->ccbs_reference));
	if (call_information->subaddress_of_a.length) {
		ASN1_CALL(pos, rose_enc_PartySubaddress(ctrl, pos, end,
			&call_information->subaddress_of_a));
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \internal
 * \brief Encode the array of call information details type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param call_details Call detail list information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_etsi_CallDetails(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, unsigned tag, const struct roseEtsiCallDetailsList *call_details)
{
	unsigned index;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	for (index = 0; index < call_details->num_records; ++index) {
		ASN1_CALL(pos, rose_enc_etsi_CallInformation(ctrl, pos, end, ASN1_TAG_SEQUENCE,
			&call_details->list[index]));
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the StatusRequest invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_StatusRequest_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiStatusRequest_ARG *status_request;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	status_request = &args->etsi.StatusRequest;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		status_request->compatibility_mode));
	ASN1_CALL(pos, rose_enc_Q931ie(ctrl, pos, end, ASN1_CLASS_APPLICATION | 0,
		&status_request->q931ie));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the StatusRequest result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_StatusRequest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args)
{
	return asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED, args->etsi.StatusRequest.status);
}

/*!
 * \brief Encode the CallInfoRetain invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CallInfoRetain_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
		args->etsi.CallInfoRetain.call_linkage_id);
}

/*!
 * \brief Encode the EraseCallLinkageID invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_EraseCallLinkageID_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
		args->etsi.EraseCallLinkageID.call_linkage_id);
}

/*!
 * \brief Encode the CCBSDeactivate invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCBSDeactivate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
		args->etsi.CCBSDeactivate.ccbs_reference);
}

/*!
 * \brief Encode the CCBSErase invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCBSErase_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiCCBSErase_ARG *ccbs_erase;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ccbs_erase = &args->etsi.CCBSErase;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		ccbs_erase->recall_mode));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
		ccbs_erase->ccbs_reference));
	ASN1_CALL(pos, rose_enc_Address(ctrl, pos, end, ASN1_TAG_SEQUENCE,
		&ccbs_erase->address_of_b));
	ASN1_CALL(pos, rose_enc_Q931ie(ctrl, pos, end, ASN1_CLASS_APPLICATION | 0,
		&ccbs_erase->q931ie));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED, ccbs_erase->reason));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the CCBSRemoteUserFree invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCBSRemoteUserFree_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiCCBSRemoteUserFree_ARG *ccbs_remote_user_free;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ccbs_remote_user_free = &args->etsi.CCBSRemoteUserFree;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		ccbs_remote_user_free->recall_mode));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
		ccbs_remote_user_free->ccbs_reference));
	ASN1_CALL(pos, rose_enc_Address(ctrl, pos, end, ASN1_TAG_SEQUENCE,
		&ccbs_remote_user_free->address_of_b));
	ASN1_CALL(pos, rose_enc_Q931ie(ctrl, pos, end, ASN1_CLASS_APPLICATION | 0,
		&ccbs_remote_user_free->q931ie));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the CCBSCall invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCBSCall_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return asn1_enc_int(pos, end, ASN1_TYPE_INTEGER, args->etsi.CCBSCall.ccbs_reference);
}

/*!
 * \brief Encode the CCBSBFree invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCBSBFree_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiCCBSBFree_ARG *ccbs_b_free;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ccbs_b_free = &args->etsi.CCBSBFree;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		ccbs_b_free->recall_mode));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
		ccbs_b_free->ccbs_reference));
	ASN1_CALL(pos, rose_enc_Address(ctrl, pos, end, ASN1_TAG_SEQUENCE,
		&ccbs_b_free->address_of_b));
	ASN1_CALL(pos, rose_enc_Q931ie(ctrl, pos, end, ASN1_CLASS_APPLICATION | 0,
		&ccbs_b_free->q931ie));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the CCBSStopAlerting invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCBSStopAlerting_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
		args->etsi.CCBSStopAlerting.ccbs_reference);
}

/*!
 * \brief Encode the CCBSStatusRequest invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCBSStatusRequest_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiCCBSStatusRequest_ARG *ccbs_status_request;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ccbs_status_request = &args->etsi.CCBSStatusRequest;
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		ccbs_status_request->recall_mode));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
		ccbs_status_request->ccbs_reference));
	ASN1_CALL(pos, rose_enc_Q931ie(ctrl, pos, end, ASN1_CLASS_APPLICATION | 0,
		&ccbs_status_request->q931ie));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the CCBSStatusRequest result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCBSStatusRequest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args)
{
	return asn1_enc_boolean(pos, end, ASN1_TYPE_BOOLEAN,
		args->etsi.CCBSStatusRequest.free);
}

/*!
 * \internal
 * \brief Encode the CCBS/CCNR-Request invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param ccbs_request Information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_etsi_CC_Request_ARG_Backend(struct pri *ctrl,
	unsigned char *pos, unsigned char *end,
	const struct roseEtsiCCBSRequest_ARG *ccbs_request)
{
	return asn1_enc_int(pos, end, ASN1_TYPE_INTEGER, ccbs_request->call_linkage_id);
}

/*!
 * \brief Encode the CCBS-Request invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCBSRequest_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return rose_enc_etsi_CC_Request_ARG_Backend(ctrl, pos, end, &args->etsi.CCBSRequest);
}

/*!
 * \brief Encode the CCNR-Request invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCNRRequest_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return rose_enc_etsi_CC_Request_ARG_Backend(ctrl, pos, end, &args->etsi.CCNRRequest);
}

/*!
 * \internal
 * \brief Encode the CCBS/CCNR-Request result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param ccbs_request Information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_etsi_CC_Request_RES_Backend(struct pri *ctrl,
	unsigned char *pos, unsigned char *end,
	const struct roseEtsiCCBSRequest_RES *ccbs_request)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		ccbs_request->recall_mode));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
		ccbs_request->ccbs_reference));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the CCBS-Request result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCBSRequest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args)
{
	return rose_enc_etsi_CC_Request_RES_Backend(ctrl, pos, end, &args->etsi.CCBSRequest);
}

/*!
 * \brief Encode the CCNR-Request result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCNRRequest_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args)
{
	return rose_enc_etsi_CC_Request_RES_Backend(ctrl, pos, end, &args->etsi.CCNRRequest);
}

/*!
 * \internal
 * \brief Encode the CCBS/CCNR-Interrogate invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param ccbs_interrogate Information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_etsi_CC_Interrogate_ARG_Backend(struct pri *ctrl,
	unsigned char *pos, unsigned char *end,
	const struct roseEtsiCCBSInterrogate_ARG *ccbs_interrogate)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	if (ccbs_interrogate->ccbs_reference_present) {
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
			ccbs_interrogate->ccbs_reference));
	}
	if (ccbs_interrogate->a_party_number.length) {
		ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
			&ccbs_interrogate->a_party_number));
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the CCBSInterrogate invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCBSInterrogate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return rose_enc_etsi_CC_Interrogate_ARG_Backend(ctrl, pos, end,
		&args->etsi.CCBSInterrogate);
}

/*!
 * \brief Encode the CCNRInterrogate invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCNRInterrogate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return rose_enc_etsi_CC_Interrogate_ARG_Backend(ctrl, pos, end,
		&args->etsi.CCNRInterrogate);
}

/*!
 * \internal
 * \brief Encode the CCBS/CCNR-Interrogate result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param ccbs_interrogate Information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_etsi_CC_Interrogate_RES_Backend(struct pri *ctrl,
	unsigned char *pos, unsigned char *end,
	const struct roseEtsiCCBSInterrogate_RES *ccbs_interrogate)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		ccbs_interrogate->recall_mode));
	if (ccbs_interrogate->call_details.num_records) {
		ASN1_CALL(pos, rose_enc_etsi_CallDetails(ctrl, pos, end, ASN1_TAG_SEQUENCE,
			&ccbs_interrogate->call_details));
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the CCBSInterrogate result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCBSInterrogate_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args)
{
	return rose_enc_etsi_CC_Interrogate_RES_Backend(ctrl, pos, end,
		&args->etsi.CCBSInterrogate);
}

/*!
 * \brief Encode the CCNRInterrogate result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCNRInterrogate_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args)
{
	return rose_enc_etsi_CC_Interrogate_RES_Backend(ctrl, pos, end,
		&args->etsi.CCNRInterrogate);
}

/*!
 * \internal
 * \brief Encode the CCBS-T/CCNR-T-Request invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param ccbs_t_request Information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_etsi_CC_T_Request_ARG_Backend(struct pri *ctrl,
	unsigned char *pos, unsigned char *end,
	const struct roseEtsiCCBS_T_Request_ARG *ccbs_t_request)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ASN1_CALL(pos, rose_enc_Address(ctrl, pos, end, ASN1_TAG_SEQUENCE,
		&ccbs_t_request->destination));
	ASN1_CALL(pos, rose_enc_Q931ie(ctrl, pos, end, ASN1_CLASS_APPLICATION | 0,
		&ccbs_t_request->q931ie));
	if (ccbs_t_request->retention_supported) {
		/* Not the DEFAULT value */
		ASN1_CALL(pos, asn1_enc_boolean(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1,
			ccbs_t_request->retention_supported));
	}
	if (ccbs_t_request->presentation_allowed_indicator_present) {
		ASN1_CALL(pos, asn1_enc_boolean(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2,
			ccbs_t_request->presentation_allowed_indicator));
	}
	if (ccbs_t_request->originating.number.length) {
		ASN1_CALL(pos, rose_enc_Address(ctrl, pos, end, ASN1_TAG_SEQUENCE,
			&ccbs_t_request->originating));
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the CCBS_T_Request invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCBS_T_Request_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return rose_enc_etsi_CC_T_Request_ARG_Backend(ctrl, pos, end,
		&args->etsi.CCBS_T_Request);
}

/*!
 * \brief Encode the CCNR_T_Request invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCNR_T_Request_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return rose_enc_etsi_CC_T_Request_ARG_Backend(ctrl, pos, end,
		&args->etsi.CCNR_T_Request);
}

/*!
 * \internal
 * \brief Encode the CCBS-T/CCNR-T-Request result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param ccbs_t_request Information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_etsi_CC_T_Request_RES_Backend(struct pri *ctrl,
	unsigned char *pos, unsigned char *end,
	const struct roseEtsiCCBS_T_Request_RES *ccbs_t_request)
{
	return asn1_enc_boolean(pos, end, ASN1_TYPE_BOOLEAN,
		ccbs_t_request->retention_supported);
}

/*!
 * \brief Encode the CCBS_T_Request result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCBS_T_Request_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args)
{
	return rose_enc_etsi_CC_T_Request_RES_Backend(ctrl, pos, end,
		&args->etsi.CCBS_T_Request);
}

/*!
 * \brief Encode the CCNR_T_Request result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_CCNR_T_Request_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args)
{
	return rose_enc_etsi_CC_T_Request_RES_Backend(ctrl, pos, end,
		&args->etsi.CCNR_T_Request);
}

/*!
 * \internal
 * \brief Decode the CallInformation argument parameters.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param call_information Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_etsi_CallInformation(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiCallInformation *call_information)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s CallInformation %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	ASN1_CALL(pos, rose_dec_Address(ctrl, "addressOfB", tag, pos, seq_end,
		&call_information->address_of_b));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag & ~ASN1_PC_MASK, ASN1_CLASS_APPLICATION | 0);
	ASN1_CALL(pos, rose_dec_Q931ie(ctrl, "q931ie", tag, pos, seq_end,
		&call_information->q931ie, sizeof(call_information->q931ie_contents)));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "ccbsReference", tag, pos, seq_end, &value));
	call_information->ccbs_reference = value;

	if (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		/* The optional subaddress must be present since there is something left. */
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CALL(pos, rose_dec_PartySubaddress(ctrl, "subaddressOfA", tag, pos, seq_end,
			&call_information->subaddress_of_a));
	} else {
		/* Subaddress not present */
		call_information->subaddress_of_a.length = 0;
	}

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \internal
 * \brief Decode the array of call information details argument parameters.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param call_details Parameter storage array to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_etsi_CallDetails(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiCallDetailsList *call_details)
{
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s CallDetails %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	call_details->num_records = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		if (ARRAY_LEN(call_details->list) <= call_details->num_records) {
			/* Too many records */
			return NULL;
		}
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
		ASN1_CALL(pos, rose_dec_etsi_CallInformation(ctrl, "listEntry", tag, pos,
			seq_end, &call_details->list[call_details->num_records]));
		++call_details->num_records;
	}

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the StatusRequest invoke argument parameters.
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
const unsigned char *rose_dec_etsi_StatusRequest_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	struct roseEtsiStatusRequest_ARG *status_request;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  StatusRequest %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	status_request = &args->etsi.StatusRequest;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "compatibilityMode", tag, pos, seq_end, &value));
	status_request->compatibility_mode = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag & ~ASN1_PC_MASK, ASN1_CLASS_APPLICATION | 0);
	ASN1_CALL(pos, rose_dec_Q931ie(ctrl, "q931ie", tag, pos, seq_end,
		&status_request->q931ie, sizeof(status_request->q931ie_contents)));

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the StatusRequest result argument parameters.
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
const unsigned char *rose_dec_etsi_StatusRequest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_result_args *args)
{
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "status", tag, pos, end, &value));
	args->etsi.StatusRequest.status = value;

	return pos;
}

/*!
 * \brief Decode the CallInfoRetain invoke argument parameters.
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
const unsigned char *rose_dec_etsi_CallInfoRetain_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "callLinkageId", tag, pos, end, &value));
	args->etsi.CallInfoRetain.call_linkage_id = value;

	return pos;
}

/*!
 * \brief Decode the EraseCallLinkageID invoke argument parameters.
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
const unsigned char *rose_dec_etsi_EraseCallLinkageID_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "callLinkageId", tag, pos, end, &value));
	args->etsi.EraseCallLinkageID.call_linkage_id = value;

	return pos;
}

/*!
 * \brief Decode the CCBSDeactivate invoke argument parameters.
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
const unsigned char *rose_dec_etsi_CCBSDeactivate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "ccbsReference", tag, pos, end, &value));
	args->etsi.CCBSDeactivate.ccbs_reference = value;

	return pos;
}

/*!
 * \brief Decode the CCBSErase invoke argument parameters.
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
const unsigned char *rose_dec_etsi_CCBSErase_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	struct roseEtsiCCBSErase_ARG *ccbs_erase;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  CCBSErase %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ccbs_erase = &args->etsi.CCBSErase;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "recallMode", tag, pos, seq_end, &value));
	ccbs_erase->recall_mode = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "ccbsReference", tag, pos, seq_end, &value));
	ccbs_erase->ccbs_reference = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	ASN1_CALL(pos, rose_dec_Address(ctrl, "addressOfB", tag, pos, seq_end,
		&ccbs_erase->address_of_b));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag & ~ASN1_PC_MASK, ASN1_CLASS_APPLICATION | 0);
	ASN1_CALL(pos, rose_dec_Q931ie(ctrl, "q931ie", tag, pos, seq_end,
		&ccbs_erase->q931ie, sizeof(ccbs_erase->q931ie_contents)));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "eraseReason", tag, pos, seq_end, &value));
	ccbs_erase->reason = value;

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the CCBSRemoteUserFree invoke argument parameters.
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
const unsigned char *rose_dec_etsi_CCBSRemoteUserFree_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	struct roseEtsiCCBSRemoteUserFree_ARG *ccbs_remote_user_free;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  CCBSRemoteUserFree %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ccbs_remote_user_free = &args->etsi.CCBSRemoteUserFree;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "recallMode", tag, pos, seq_end, &value));
	ccbs_remote_user_free->recall_mode = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "ccbsReference", tag, pos, seq_end, &value));
	ccbs_remote_user_free->ccbs_reference = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	ASN1_CALL(pos, rose_dec_Address(ctrl, "addressOfB", tag, pos, seq_end,
		&ccbs_remote_user_free->address_of_b));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag & ~ASN1_PC_MASK, ASN1_CLASS_APPLICATION | 0);
	ASN1_CALL(pos, rose_dec_Q931ie(ctrl, "q931ie", tag, pos, seq_end,
		&ccbs_remote_user_free->q931ie, sizeof(ccbs_remote_user_free->q931ie_contents)));

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the CCBSCall invoke argument parameters.
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
const unsigned char *rose_dec_etsi_CCBSCall_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "ccbsReference", tag, pos, end, &value));
	args->etsi.CCBSCall.ccbs_reference = value;

	return pos;
}

/*!
 * \brief Decode the CCBSBFree invoke argument parameters.
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
const unsigned char *rose_dec_etsi_CCBSBFree_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	struct roseEtsiCCBSBFree_ARG *ccbs_b_free;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  CCBSBFree %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ccbs_b_free = &args->etsi.CCBSBFree;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "recallMode", tag, pos, seq_end, &value));
	ccbs_b_free->recall_mode = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "ccbsReference", tag, pos, seq_end, &value));
	ccbs_b_free->ccbs_reference = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	ASN1_CALL(pos, rose_dec_Address(ctrl, "addressOfB", tag, pos, seq_end,
		&ccbs_b_free->address_of_b));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag & ~ASN1_PC_MASK, ASN1_CLASS_APPLICATION | 0);
	ASN1_CALL(pos, rose_dec_Q931ie(ctrl, "q931ie", tag, pos, seq_end,
		&ccbs_b_free->q931ie, sizeof(ccbs_b_free->q931ie_contents)));

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the CCBSStopAlerting invoke argument parameters.
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
const unsigned char *rose_dec_etsi_CCBSStopAlerting_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "ccbsReference", tag, pos, end, &value));
	args->etsi.CCBSStopAlerting.ccbs_reference = value;

	return pos;
}

/*!
 * \brief Decode the CCBSStatusRequest invoke argument parameters.
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
const unsigned char *rose_dec_etsi_CCBSStatusRequest_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	struct roseEtsiCCBSStatusRequest_ARG *ccbs_status_request;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  CCBSStatusRequest %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ccbs_status_request = &args->etsi.CCBSStatusRequest;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "recallMode", tag, pos, seq_end, &value));
	ccbs_status_request->recall_mode = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "ccbsReference", tag, pos, seq_end, &value));
	ccbs_status_request->ccbs_reference = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag & ~ASN1_PC_MASK, ASN1_CLASS_APPLICATION | 0);
	ASN1_CALL(pos, rose_dec_Q931ie(ctrl, "q931ie", tag, pos, seq_end,
		&ccbs_status_request->q931ie, sizeof(ccbs_status_request->q931ie_contents)));

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the CCBSStatusRequest result argument parameters.
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
const unsigned char *rose_dec_etsi_CCBSStatusRequest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_result_args *args)
{
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_BOOLEAN);
	ASN1_CALL(pos, asn1_dec_boolean(ctrl, "free", tag, pos, end, &value));
	args->etsi.CCBSStatusRequest.free = value;

	return pos;
}

/*!
 * \internal
 * \brief Decode the CCBS/CCNR-Request invoke argument parameters.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param tag Component tag that identified this structure.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param ccbs_request Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_etsi_CC_Request_ARG_Backend(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiCCBSRequest_ARG *ccbs_request)
{
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "callLinkageId", tag, pos, end, &value));
	ccbs_request->call_linkage_id = value;

	return pos;
}

/*!
 * \brief Decode the CCBSRequest invoke argument parameters.
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
const unsigned char *rose_dec_etsi_CCBSRequest_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	return rose_dec_etsi_CC_Request_ARG_Backend(ctrl, tag, pos, end,
		&args->etsi.CCBSRequest);
}

/*!
 * \brief Decode the CCNRRequest invoke argument parameters.
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
const unsigned char *rose_dec_etsi_CCNRRequest_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	return rose_dec_etsi_CC_Request_ARG_Backend(ctrl, tag, pos, end,
		&args->etsi.CCNRRequest);
}

/*!
 * \internal
 * \brief Decode the CCBS/CCNR-Request result argument parameters.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this structure.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param ccbs_request Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_etsi_CC_Request_RES_Backend(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiCCBSRequest_RES *ccbs_request)
{
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  CC%sRequest %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "recallMode", tag, pos, seq_end, &value));
	ccbs_request->recall_mode = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "ccbsReference", tag, pos, seq_end, &value));
	ccbs_request->ccbs_reference = value;

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the CCBSRequest result argument parameters.
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
const unsigned char *rose_dec_etsi_CCBSRequest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_result_args *args)
{
	return rose_dec_etsi_CC_Request_RES_Backend(ctrl, "BS", tag, pos, end,
		&args->etsi.CCBSRequest);
}

/*!
 * \brief Decode the CCNRRequest result argument parameters.
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
const unsigned char *rose_dec_etsi_CCNRRequest_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_result_args *args)
{
	return rose_dec_etsi_CC_Request_RES_Backend(ctrl, "NR", tag, pos, end,
		&args->etsi.CCNRRequest);
}

/*!
 * \internal
 * \brief Decode the CCBS/CCNR-Interrogate invoke argument parameters.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this structure.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param ccbs_interrogate Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_etsi_CC_Interrogate_ARG_Backend(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiCCBSInterrogate_ARG *ccbs_interrogate)
{
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  CC%sInterrogate %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the optional components.
	 */
	ccbs_interrogate->ccbs_reference = 0;
	ccbs_interrogate->ccbs_reference_present = 0;
	ccbs_interrogate->a_party_number.length = 0;	/* Assume A party number not present */
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag) {
		case ASN1_TYPE_INTEGER:
			ASN1_CALL(pos, asn1_dec_int(ctrl, "ccbsReference", tag, pos, seq_end,
				&value));
			ccbs_interrogate->ccbs_reference = value;
			ccbs_interrogate->ccbs_reference_present = 1;
			break;
		default:
			ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "partyNumberOfA", tag, pos,
				seq_end, &ccbs_interrogate->a_party_number));
			break;
		}
	}

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the CCBSInterrogate invoke argument parameters.
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
const unsigned char *rose_dec_etsi_CCBSInterrogate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	return rose_dec_etsi_CC_Interrogate_ARG_Backend(ctrl, "BS", tag, pos, end,
		&args->etsi.CCBSInterrogate);
}

/*!
 * \brief Decode the CCNRInterrogate invoke argument parameters.
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
const unsigned char *rose_dec_etsi_CCNRInterrogate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	return rose_dec_etsi_CC_Interrogate_ARG_Backend(ctrl, "NR", tag, pos, end,
		&args->etsi.CCNRInterrogate);
}

/*!
 * \internal
 * \brief Decode the CCBS/CCNR-Interrogate result argument parameters.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this structure.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param ccbs_interrogate Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_etsi_CC_Interrogate_RES_Backend(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiCCBSInterrogate_RES *ccbs_interrogate)
{
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  CC%sInterrogate %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "recallMode", tag, pos, seq_end, &value));
	ccbs_interrogate->recall_mode = value;

	ccbs_interrogate->call_details.num_records = 0;
	if (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
		ASN1_CALL(pos, rose_dec_etsi_CallDetails(ctrl, "callDetails", tag, pos, seq_end,
			&ccbs_interrogate->call_details));
	}

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the CCBSInterrogate result argument parameters.
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
const unsigned char *rose_dec_etsi_CCBSInterrogate_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_result_args *args)
{
	return rose_dec_etsi_CC_Interrogate_RES_Backend(ctrl, "BS", tag, pos, end,
		&args->etsi.CCBSInterrogate);
}

/*!
 * \brief Decode the CCNRInterrogate result argument parameters.
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
const unsigned char *rose_dec_etsi_CCNRInterrogate_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_result_args *args)
{
	return rose_dec_etsi_CC_Interrogate_RES_Backend(ctrl, "NR", tag, pos, end,
		&args->etsi.CCNRInterrogate);
}

/*!
 * \internal
 * \brief Decode the CCBS-T/CCNR-T-Request invoke argument parameters.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this structure.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param ccbs_t_request Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_etsi_CC_T_Request_ARG_Backend(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiCCBS_T_Request_ARG *ccbs_t_request)
{
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	const unsigned char *save_pos;
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  CC%s-T-Request %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	ASN1_CALL(pos, rose_dec_Address(ctrl, "destinationAddress", tag, pos, seq_end,
		&ccbs_t_request->destination));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag & ~ASN1_PC_MASK, ASN1_CLASS_APPLICATION | 0);
	ASN1_CALL(pos, rose_dec_Q931ie(ctrl, "q931ie", tag, pos, seq_end,
		&ccbs_t_request->q931ie, sizeof(ccbs_t_request->q931ie_contents)));

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	ccbs_t_request->retention_supported = 0;	/* DEFAULT retention_supported value (FALSE) */
	ccbs_t_request->presentation_allowed_indicator = 0;
	ccbs_t_request->presentation_allowed_indicator_present = 0;
	ccbs_t_request->originating.number.length = 0;	/* Assume originating party number not present */
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag) {
		case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
			ASN1_CALL(pos, asn1_dec_boolean(ctrl, "retentionSupported", tag, pos,
				seq_end, &value));
			ccbs_t_request->retention_supported = value;
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 2:
			ASN1_CALL(pos, asn1_dec_boolean(ctrl, "presentationAllowedIndicator", tag,
				pos, seq_end, &value));
			ccbs_t_request->presentation_allowed_indicator = value;
			ccbs_t_request->presentation_allowed_indicator_present = 1;
			break;
		case ASN1_TAG_SEQUENCE:
			ASN1_CALL(pos, rose_dec_Address(ctrl, "originatingAddress", tag, pos,
				seq_end, &ccbs_t_request->originating));
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
 * \brief Decode the CCBS_T_Request invoke argument parameters.
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
const unsigned char *rose_dec_etsi_CCBS_T_Request_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	return rose_dec_etsi_CC_T_Request_ARG_Backend(ctrl, "BS", tag, pos, end,
		&args->etsi.CCBS_T_Request);
}

/*!
 * \brief Decode the CCNR_T_Request invoke argument parameters.
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
const unsigned char *rose_dec_etsi_CCNR_T_Request_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	return rose_dec_etsi_CC_T_Request_ARG_Backend(ctrl, "NR", tag, pos, end,
		&args->etsi.CCNR_T_Request);
}

/*!
 * \internal
 * \brief Decode the CCBS-T/CCNR-T-Request result argument parameters.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this structure.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param ccbs_t_request Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_etsi_CC_T_Request_RES_Backend(struct pri *ctrl,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiCCBS_T_Request_RES *ccbs_t_request)
{
	int32_t value;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_BOOLEAN);
	ASN1_CALL(pos, asn1_dec_boolean(ctrl, "retentionSupported", tag, pos, end, &value));
	ccbs_t_request->retention_supported = value;

	return pos;
}

/*!
 * \brief Decode the CCBS_T_Request result argument parameters.
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
const unsigned char *rose_dec_etsi_CCBS_T_Request_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_result_args *args)
{
	return rose_dec_etsi_CC_T_Request_RES_Backend(ctrl, tag, pos, end,
		&args->etsi.CCBS_T_Request);
}

/*!
 * \brief Decode the CCNR_T_Request result argument parameters.
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
const unsigned char *rose_dec_etsi_CCNR_T_Request_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_result_args *args)
{
	return rose_dec_etsi_CC_T_Request_RES_Backend(ctrl, tag, pos, end,
		&args->etsi.CCNR_T_Request);
}

/* ------------------------------------------------------------------- */
/* end rose_etsi_cc.c */
