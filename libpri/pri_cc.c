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
 * \brief Call Completion controller
 *
 * \author Richard Mudgett <rmudgett@digium.com>
 */


#include "compat.h"
#include "libpri.h"
#include "pri_internal.h"
#include "pri_facility.h"

#include <stdlib.h>

/* Define CC_SANITY_CHECKS to add some consistency sanity checking. */
//#define CC_SANITY_CHECKS	1
#define CC_SANITY_CHECKS	1// BUGBUG

/*! Maximum times CCBSStatusRequest can have no response before canceling CC. */
#define RAW_STATUS_COUNT_MAX	3

/* ------------------------------------------------------------------- */

/*!
 * \brief Find a cc_record by the PTMP reference_id.
 *
 * \param ctrl D channel controller.
 * \param reference_id CCBS reference ID to look for in cc_record pool.
 *
 * \retval cc_record on success.
 * \retval NULL on error.
 */
struct pri_cc_record *pri_cc_find_by_reference(struct pri *ctrl, unsigned reference_id)
{
	struct pri_cc_record *cc_record;

	for (cc_record = ctrl->cc.pool; cc_record; cc_record = cc_record->next) {
		if (cc_record->ccbs_reference_id == reference_id) {
			/* Found the record */
			break;
		}
	}

	return cc_record;
}

/*!
 * \brief Find a cc_record by the PTMP linkage_id.
 *
 * \param ctrl D channel controller.
 * \param linkage_id Call linkage ID to look for in cc_record pool.
 *
 * \retval cc_record on success.
 * \retval NULL on error.
 */
struct pri_cc_record *pri_cc_find_by_linkage(struct pri *ctrl, unsigned linkage_id)
{
	struct pri_cc_record *cc_record;

	for (cc_record = ctrl->cc.pool; cc_record; cc_record = cc_record->next) {
		if (cc_record->call_linkage_id == linkage_id) {
			/* Found the record */
			break;
		}
	}

	return cc_record;
}

/*!
 * \internal
 * \brief Find a cc_record by the cc_id.
 *
 * \param ctrl D channel controller.
 * \param cc_id ID to look for in cc_record pool.
 *
 * \retval cc_record on success.
 * \retval NULL on error.
 */
static struct pri_cc_record *pri_cc_find_by_id(struct pri *ctrl, long cc_id)
{
	struct pri_cc_record *cc_record;

	for (cc_record = ctrl->cc.pool; cc_record; cc_record = cc_record->next) {
		if (cc_record->record_id == cc_id) {
			/* Found the record */
			break;
		}
	}

	return cc_record;
}

/*!
 * \internal
 * \brief Find the given ie_type in the string of q931 ies.
 *
 * \param ie_type Q.931 ie type to find in q931_ies.
 * \param length Length of the given q931_ies
 * \param q931_ies Given q931_ies
 *
 * \retval found-ie on success.
 * \retval NULL on error.
 */
static const struct q931_ie *pri_cc_find_ie(unsigned ie_type, unsigned length, const unsigned char *q931_ies)
{
	const unsigned char *pos;
	const unsigned char *end;
	const unsigned char *next;
	const struct q931_ie *cur;

	end = q931_ies + length;
	for (pos = q931_ies; pos < end; pos = next) {
		cur = (const struct q931_ie *) pos;
		if (cur->ie & 0x80) {
			/* Single octet ie. */
			next = pos + 1;
		} else {
			/* Variable length ie. */
			next = cur->data + cur->len;
		}
		if (cur->ie == ie_type && next <= end) {
			/* Found the ie and it is within the given q931_ies body. */
			return cur;
		}
	}
	return NULL;
}

/*!
 * \internal
 * \brief Compare the specified ie type in the CC record q931_ies to the given q931_ies.
 *
 * \details
 * The individual q931 ie is compared with memcmp().
 *
 * \param ie_type Q.931 ie type to compare.
 * \param record_ies CC record q931_ies
 * \param length Length of the given q931_ies
 * \param q931_ies Given q931_ies
 *
 * \retval == 0 when record_ies == q931_ies.
 * \retval != 0 when record_ies != q931_ies.
 */
static int pri_cc_cmp_ie(unsigned ie_type, const struct q931_saved_ie_contents *record_ies, unsigned length, const unsigned char *q931_ies)
{
	const struct q931_ie *left;
	const struct q931_ie *right;

	left = pri_cc_find_ie(ie_type, record_ies->length, record_ies->data);
	right = pri_cc_find_ie(ie_type, length, q931_ies);

	if (!left && !right) {
		/* Neither has the requested ie to compare so they match. */
		return 0;
	}
	if (!left || !right) {
		/* One or the other does not have the requested ie to compare. */
		return 1;
	}
	if (left->len != right->len) {
		/* They are not the same length. */
		return 1;
	}
	return memcmp(left->data, right->data, right->len);
}

/*!
 * \internal
 * \brief Compare the CC record q931_ies to the given q931_ies.
 *
 * \note
 * Only the first BC, HLC, and LLC ies in the given q931_ies are compared.
 *
 * \param record_ies CC record q931_ies
 * \param length Length of the given q931_ies
 * \param q931_ies Given q931_ies
 *
 * \retval == 0 when record_ies == q931_ies.
 * \retval != 0 when record_ies != q931_ies.
 */
static int pri_cc_cmp_q931_ies(const struct q931_saved_ie_contents *record_ies, unsigned length, const unsigned char *q931_ies)
{
	return pri_cc_cmp_ie(Q931_BEARER_CAPABILITY, record_ies, length, q931_ies)
		|| pri_cc_cmp_ie(Q931_HIGH_LAYER_COMPAT, record_ies, length, q931_ies)
		|| pri_cc_cmp_ie(Q931_LOW_LAYER_COMPAT, record_ies, length, q931_ies);
}

/*!
 * \brief Find a cc_record by an incoming call addressing data.
 *
 * \param ctrl D channel controller.
 * \param party_a Party A address.
 * \param party_b Party B address.
 * \param length Length of the given q931_ies.
 * \param q931_ies BC, HLC, LLC ies to compare with CC records.
 *
 * \retval cc_record on success.
 * \retval NULL on error.
 */
struct pri_cc_record *pri_cc_find_by_addressing(struct pri *ctrl, const struct q931_party_address *party_a, const struct q931_party_address *party_b, unsigned length, const unsigned char *q931_ies)
{
	struct pri_cc_record *cc_record;
	struct q931_party_address addr_a;
	struct q931_party_address addr_b;

	addr_a = *party_a;
	addr_b = *party_b;
	for (cc_record = ctrl->cc.pool; cc_record; cc_record = cc_record->next) {
		/* Do not compare the number presentation. */
		addr_a.number.presentation = cc_record->party_a.number.presentation;
		addr_b.number.presentation = cc_record->party_b.number.presentation;
		if (!q931_cmp_party_id_to_address(&cc_record->party_a, &addr_a)
			&& !q931_party_address_cmp(&cc_record->party_b, &addr_b)
			&& !pri_cc_cmp_q931_ies(&cc_record->saved_ie_contents, length, q931_ies)) {
			/* Found the record */
			break;
		}
	}

	return cc_record;
}

/*!
 * \internal
 * \brief Allocate a new cc_record reference id.
 *
 * \param ctrl D channel controller.
 *
 * \retval reference_id on success.
 * \retval CC_PTMP_INVALID_ID on error.
 */
static int pri_cc_new_reference_id(struct pri *ctrl)
{
	long reference_id;
	long first_id;

	ctrl->cc.last_reference_id = (ctrl->cc.last_reference_id + 1) & 0x7F;
	reference_id = ctrl->cc.last_reference_id;
	first_id = reference_id;
	while (pri_cc_find_by_reference(ctrl, reference_id)) {
		ctrl->cc.last_reference_id = (ctrl->cc.last_reference_id + 1) & 0x7F;
		reference_id = ctrl->cc.last_reference_id;
		if (reference_id == first_id) {
			/* We probably have a resource leak. */
			pri_error(ctrl, "PTMP call completion reference id exhaustion!\n");
			reference_id = CC_PTMP_INVALID_ID;
			break;
		}
	}

	return reference_id;
}

/*!
 * \internal
 * \brief Allocate a new cc_record linkage id.
 *
 * \param ctrl D channel controller.
 *
 * \retval linkage_id on success.
 * \retval CC_PTMP_INVALID_ID on error.
 */
static int pri_cc_new_linkage_id(struct pri *ctrl)
{
	long linkage_id;
	long first_id;

	ctrl->cc.last_linkage_id = (ctrl->cc.last_linkage_id + 1) & 0x7F;
	linkage_id = ctrl->cc.last_linkage_id;
	first_id = linkage_id;
	while (pri_cc_find_by_linkage(ctrl, linkage_id)) {
		ctrl->cc.last_linkage_id = (ctrl->cc.last_linkage_id + 1) & 0x7F;
		linkage_id = ctrl->cc.last_linkage_id;
		if (linkage_id == first_id) {
			/* We probably have a resource leak. */
			pri_error(ctrl, "PTMP call completion linkage id exhaustion!\n");
			linkage_id = CC_PTMP_INVALID_ID;
			break;
		}
	}

	return linkage_id;
}

/*!
 * \internal
 * \brief Allocate a new cc_record id.
 *
 * \param ctrl D channel controller.
 *
 * \retval cc_id on success.
 * \retval -1 on error.
 */
static long pri_cc_new_id(struct pri *ctrl)
{
	long record_id;
	long first_id;

	record_id = ++ctrl->cc.last_record_id;
	first_id = record_id;
	while (pri_cc_find_by_id(ctrl, record_id)) {
		record_id = ++ctrl->cc.last_record_id;
		if (record_id == first_id) {
			/*
			 * We have a resource leak.
			 * We should never need to allocate 64k records on a D channel.
			 */
			pri_error(ctrl, "Too many call completion records!\n");
			record_id = -1;
			break;
		}
	}

	return record_id;
}

/*!
 * \internal
 * \brief Disassociate the signaling link call from the cc_record.
 *
 * \param cc_record CC record to disassociate from the signaling link call.
 *
 * \return Nothing
 */
static void pri_cc_disassociate_signaling_link(struct pri_cc_record *cc_record)
{
	if (cc_record->signaling) {
		cc_record->signaling->cc.record = NULL;
		cc_record->signaling = NULL;
	}
}

/*!
 * \internal
 * \brief Delete the given call completion record
 *
 * \param ctrl D channel controller.
 * \param doomed Call completion record to destroy
 *
 * \return Nothing
 */
static void pri_cc_delete_record(struct pri *ctrl, struct pri_cc_record *doomed)
{
	struct pri_cc_record **prev;
	struct pri_cc_record *current;

	/* Unlink CC signaling link associations. */
	if (doomed->original_call) {
		doomed->original_call->cc.record = NULL;
		doomed->original_call = NULL;
	}
	pri_cc_disassociate_signaling_link(doomed);

	for (prev = &ctrl->cc.pool, current = ctrl->cc.pool; current;
		prev = &current->next, current = current->next) {
		if (current == doomed) {
			*prev = current->next;
			free(doomed);
			return;
		}
	}

	/* The doomed node is not in the call completion database */
}

/*!
 * \brief Allocate a new cc_record.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 *
 * \retval pointer to new call completion record
 * \retval NULL if failed
 */
struct pri_cc_record *pri_cc_new_record(struct pri *ctrl, q931_call *call)
{
	struct pri_cc_record *cc_record;
	long record_id;

	record_id = pri_cc_new_id(ctrl);
	if (record_id < 0) {
		return NULL;
	}
	cc_record = calloc(1, sizeof(*cc_record));
	if (!cc_record) {
		return NULL;
	}

	/* Initialize the new record */
	cc_record->ctrl = ctrl;
	cc_record->record_id = record_id;
	cc_record->call_linkage_id = CC_PTMP_INVALID_ID;/* So it will never be found this way */
	cc_record->ccbs_reference_id = CC_PTMP_INVALID_ID;/* So it will never be found this way */
	cc_record->party_a = call->cc.party_a;
	cc_record->party_b = call->called;
	cc_record->saved_ie_contents = call->cc.saved_ie_contents;
	cc_record->bc = call->bc;
	cc_record->option.recall_mode = ctrl->cc.option.recall_mode;

	/*
	 * Append the new record to the end of the list so they are in
	 * cronological order for interrogations.
	 */
	if (ctrl->cc.pool) {
		struct pri_cc_record *cur;

		for (cur = ctrl->cc.pool; cur->next; cur = cur->next) {
		}
		cur->next = cc_record;
	} else {
		ctrl->cc.pool = cc_record;
	}

	return cc_record;
}

/*!
 * \internal
 * \brief Encode ETSI PTP call completion event operation message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param operation PTP call completion event operation to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_ptp_cc_operation(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, enum rose_operation operation)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = get_invokeid(ctrl);
	msg.operation = operation;

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode ETSI PTMP call completion available message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param cc_record Call completion record to process event.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_ptmp_cc_available(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, struct pri_cc_record *cc_record)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = get_invokeid(ctrl);
	msg.operation = ROSE_ETSI_CallInfoRetain;

	msg.args.etsi.CallInfoRetain.call_linkage_id = cc_record->call_linkage_id;

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode and queue a cc-available message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode call completion available.
 * \param cc_record Call completion record to process event.
 * \param msgtype Q.931 message type to put facility ie in.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_cc_available_encode(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, int msgtype)
{
	unsigned char buffer[256];
	unsigned char *end;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		if (PTMP_MODE(ctrl)) {
			end =
				enc_etsi_ptmp_cc_available(ctrl, buffer, buffer + sizeof(buffer),
					cc_record);
		} else {
			end =
				enc_etsi_ptp_cc_operation(ctrl, buffer, buffer + sizeof(buffer),
					ROSE_ETSI_CCBS_T_Available);
		}
		break;
	case PRI_SWITCH_QSIG:
		/* Q.SIG does not have a cc-available type message. */
		return 0;
	default:
		return -1;
	}
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, msgtype, buffer, end - buffer, NULL);
}

/*!
 * \internal
 * \brief Encode ETSI PTMP EraseCallLinkageID message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param cc_record Call completion record to process event.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_ptmp_erase_call_linkage(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, struct pri_cc_record *cc_record)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = get_invokeid(ctrl);
	msg.operation = ROSE_ETSI_EraseCallLinkageID;

	msg.args.etsi.EraseCallLinkageID.call_linkage_id = cc_record->call_linkage_id;

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode and queue an EraseCallLinkageID message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode EraseCallLinkageID.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_erase_call_linkage_encode(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record)
{
	unsigned char buffer[256];
	unsigned char *end;

	end =
		enc_etsi_ptmp_erase_call_linkage(ctrl, buffer, buffer + sizeof(buffer),
			cc_record);
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL);
}

/*!
 * \internal
 * \brief Encode and send an EraseCallLinkageID message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode EraseCallLinkageID.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int send_erase_call_linkage_id(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record)
{
	if (rose_erase_call_linkage_encode(ctrl, call, cc_record)
		|| q931_facility(ctrl, call)) {
		pri_message(ctrl,
			"Could not schedule facility message for EraseCallLinkageID.\n");
		return -1;
	}

	return 0;
}

/*!
 * \internal
 * \brief Encode ETSI PTMP CCBSErase message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param cc_record Call completion record to process event.
 * \param reason CCBS Erase reason
 *  normal-unspecified(0), t-CCBS2-timeout(1), t-CCBS3-timeout(2), basic-call-failed(3)
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_ptmp_ccbs_erase(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, struct pri_cc_record *cc_record, int reason)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = get_invokeid(ctrl);
	msg.operation = ROSE_ETSI_CCBSErase;

	if (cc_record->saved_ie_contents.length
		<= sizeof(msg.args.etsi.CCBSErase.q931ie_contents)) {
		/* Saved BC, HLC, and LLC from initial SETUP */
		msg.args.etsi.CCBSErase.q931ie.length = cc_record->saved_ie_contents.length;
		memcpy(msg.args.etsi.CCBSErase.q931ie.contents, cc_record->saved_ie_contents.data,
			cc_record->saved_ie_contents.length);
	} else {
		pri_error(ctrl, "CCBSErase q931 ie contents did not fit.\n");
	}

	q931_copy_address_to_rose(ctrl, &msg.args.etsi.CCBSErase.address_of_b,
		&cc_record->party_b);
	msg.args.etsi.CCBSErase.recall_mode = cc_record->option.recall_mode;
	msg.args.etsi.CCBSErase.ccbs_reference = cc_record->ccbs_reference_id;
	msg.args.etsi.CCBSErase.reason = reason;

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode and queue an CCBSErase message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode CCBSErase.
 * \param cc_record Call completion record to process event.
 * \param reason CCBS Erase reason
 *  normal-unspecified(0), t-CCBS2-timeout(1), t-CCBS3-timeout(2), basic-call-failed(3)
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_ccbs_erase_encode(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, int reason)
{
	unsigned char buffer[256];
	unsigned char *end;

	end =
		enc_etsi_ptmp_ccbs_erase(ctrl, buffer, buffer + sizeof(buffer), cc_record,
			reason);
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL);
}

/*!
 * \internal
 * \brief Encode and send an CCBSErase message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode EraseCallLinkageID.
 * \param cc_record Call completion record to process event.
 * \param reason CCBS Erase reason
 *  normal-unspecified(0), t-CCBS2-timeout(1), t-CCBS3-timeout(2), basic-call-failed(3)
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int send_ccbs_erase(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, int reason)
{
/*
 * XXX May need to add called-party-ie with Party A number in FACILITY message. (CCBSErase)
 * ETSI EN 300-195-1 Section 5.41 MSN interaction.
 */
	if (rose_ccbs_erase_encode(ctrl, call, cc_record, reason)
		|| q931_facility(ctrl, call)) {
		pri_message(ctrl,
			"Could not schedule facility message for CCBSErase.\n");
		return -1;
	}

	return 0;
}

/*!
 * \internal
 * \brief Encode ETSI PTMP CCBSStatusRequest result message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param cc_record Call completion record to process event.
 * \param is_free TRUE if the Party A status is available.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_ptmp_ccbs_status_request_rsp(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, struct pri_cc_record *cc_record, int is_free)
{
	struct rose_msg_result msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = cc_record->response.invoke_id;
	msg.operation = ROSE_ETSI_CCBSStatusRequest;

	msg.args.etsi.CCBSStatusRequest.free = is_free;

	pos = rose_encode_result(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode ETSI PTMP CCBSStatusRequest message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param cc_record Call completion record to process event.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_ptmp_ccbs_status_request(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, struct pri_cc_record *cc_record)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = get_invokeid(ctrl);
	msg.operation = ROSE_ETSI_CCBSStatusRequest;

	if (cc_record->saved_ie_contents.length
		<= sizeof(msg.args.etsi.CCBSStatusRequest.q931ie_contents)) {
		/* Saved BC, HLC, and LLC from initial SETUP */
		msg.args.etsi.CCBSStatusRequest.q931ie.length =
			cc_record->saved_ie_contents.length;
		memcpy(msg.args.etsi.CCBSStatusRequest.q931ie.contents,
			cc_record->saved_ie_contents.data, cc_record->saved_ie_contents.length);
	} else {
		pri_error(ctrl, "CCBSStatusRequest q931 ie contents did not fit.\n");
	}

	msg.args.etsi.CCBSStatusRequest.recall_mode = cc_record->option.recall_mode;
	msg.args.etsi.CCBSStatusRequest.ccbs_reference = cc_record->ccbs_reference_id;

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode ETSI PTMP CCBSRequest/CCNRRequest message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param cc_record Call completion record to process event.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_ptmp_cc_request(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, struct pri_cc_record *cc_record)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = get_invokeid(ctrl);
	msg.operation = cc_record->is_ccnr ? ROSE_ETSI_CCNRRequest : ROSE_ETSI_CCBSRequest;

	msg.args.etsi.CCBSRequest.call_linkage_id = cc_record->call_linkage_id;

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode ETSI PTP CCBS_T_Request/CCNR_T_Request message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param cc_record Call completion record to process event.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_ptp_cc_request(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, struct pri_cc_record *cc_record)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = get_invokeid(ctrl);
	msg.operation = cc_record->is_ccnr
		? ROSE_ETSI_CCNR_T_Request : ROSE_ETSI_CCBS_T_Request;

	if (cc_record->saved_ie_contents.length
		<= sizeof(msg.args.etsi.CCBS_T_Request.q931ie_contents)) {
		/* Saved BC, HLC, and LLC from initial SETUP */
		msg.args.etsi.CCBS_T_Request.q931ie.length = cc_record->saved_ie_contents.length;
		memcpy(msg.args.etsi.CCBS_T_Request.q931ie.contents,
			cc_record->saved_ie_contents.data, cc_record->saved_ie_contents.length);
	} else {
		pri_error(ctrl, "CCBS_T_Request q931 ie contents did not fit.\n");
	}

	q931_copy_address_to_rose(ctrl, &msg.args.etsi.CCBS_T_Request.destination,
		&cc_record->party_b);

	if (cc_record->party_a.number.valid && cc_record->party_a.number.str[0]) {
		q931_copy_id_address_to_rose(ctrl, &msg.args.etsi.CCBS_T_Request.originating,
			&cc_record->party_a);

		msg.args.etsi.CCBS_T_Request.presentation_allowed_indicator_present = 1;
		if ((cc_record->party_a.number.presentation & PRI_PRES_RESTRICTION)
			== PRI_PRES_ALLOWED) {
			msg.args.etsi.CCBS_T_Request.presentation_allowed_indicator = 1;
		}
	}

	//msg.args.etsi.CCBS_T_Request.retention_supported = 0;

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode Q.SIG ccbsRequest/ccnrRequest message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param cc_record Call completion record to process event.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_qsig_cc_request(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, struct pri_cc_record *cc_record)
{
	struct fac_extension_header header;
	struct rose_msg_invoke msg;

	memset(&header, 0, sizeof(header));
	header.nfe_present = 1;
	header.nfe.source_entity = 0;	/* endPINX */
	header.nfe.destination_entity = 0;	/* endPINX */
	header.interpretation_present = 1;
	header.interpretation = 1;	/* clearCallIfAnyInvokePduNotRecognised */
	pos = facility_encode_header(ctrl, pos, end, &header);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = get_invokeid(ctrl);
	msg.operation = cc_record->is_ccnr
		? ROSE_QSIG_CcnrRequest : ROSE_QSIG_CcbsRequest;

	/* Fill in Party B address. */
	q931_copy_number_to_rose(ctrl, &msg.args.qsig.CcbsRequest.number_b,
		&cc_record->party_b.number);
	q931_copy_subaddress_to_rose(ctrl, &msg.args.qsig.CcbsRequest.subaddr_b,
		&cc_record->party_b.subaddress);

	/* Fill in Party A address. */
	q931_copy_presented_number_unscreened_to_rose(ctrl,
		&msg.args.qsig.CcbsRequest.number_a, &cc_record->party_a.number);
	q931_copy_subaddress_to_rose(ctrl, &msg.args.qsig.CcbsRequest.subaddr_a,
		&cc_record->party_a.subaddress);

	/* Fill in service Q.931 ie information. */
	if (cc_record->saved_ie_contents.length
		<= sizeof(msg.args.qsig.CcbsRequest.q931ie_contents)) {
		/* Saved BC, HLC, and LLC from initial SETUP */
		msg.args.qsig.CcbsRequest.q931ie.length = cc_record->saved_ie_contents.length;
		memcpy(msg.args.qsig.CcbsRequest.q931ie.contents,
			cc_record->saved_ie_contents.data, cc_record->saved_ie_contents.length);
	} else {
		pri_error(ctrl, "CcbsRequest q931 ie contents did not fit.\n");
	}

	//msg.args.qsig.CcbsRequest.can_retain_service = 0;

	switch (ctrl->cc.option.signaling_retention_req) {
	case 0:/* Want release signaling link. */
		cc_record->option.retain_signaling_link = 0;

		msg.args.qsig.CcbsRequest.retain_sig_connection_present = 1;
		msg.args.qsig.CcbsRequest.retain_sig_connection = 0;
		break;
	case 1:/* Demand retain signaling link. */
		cc_record->option.retain_signaling_link = 1;

		msg.args.qsig.CcbsRequest.retain_sig_connection_present = 1;
		msg.args.qsig.CcbsRequest.retain_sig_connection = 1;
		break;
	case 2:/* Don't care about signaling link retention. */
	default:
		cc_record->option.retain_signaling_link = 0;
		break;
	}
	if (!cc_record->party_a.number.valid || cc_record->party_a.number.str[0] == '\0') {
		/*
		 * Party A number is not available for the other end to initiate
		 * a signaling link to us.  We must require that the signaling link
		 * be retained.
		 */
		cc_record->option.retain_signaling_link = 1;

		msg.args.qsig.CcbsRequest.retain_sig_connection_present = 1;
		msg.args.qsig.CcbsRequest.retain_sig_connection = 1;
	}

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode Q.SIG ccSuspend/ccResume/ccPathReserve/ccRingout message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param operation Q.SIG call completion event operation to encode.
 * \param interpretation Component interpretation:
 * discardAnyUnrecognisedInvokePdu(0),
 * clearCallIfAnyInvokePduNotRecognised(1),
 * rejectAnyUnrecognisedInvokePdu(2)
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_qsig_cc_extension_event(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, enum rose_operation operation,
	int interpretation)
{
	struct fac_extension_header header;
	struct rose_msg_invoke msg;

	memset(&header, 0, sizeof(header));
	header.nfe_present = 1;
	header.nfe.source_entity = 0;	/* endPINX */
	header.nfe.destination_entity = 0;	/* endPINX */
	header.interpretation_present = 1;
	header.interpretation = interpretation;
	pos = facility_encode_header(ctrl, pos, end, &header);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = get_invokeid(ctrl);
	msg.operation = operation;

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode ETSI PTMP CCBSDeactivate message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param cc_record Call completion record to process event.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_ptmp_cc_deactivate(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, struct pri_cc_record *cc_record)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = get_invokeid(ctrl);
	msg.operation = ROSE_ETSI_CCBSDeactivate;

	msg.args.etsi.CCBSDeactivate.ccbs_reference = cc_record->ccbs_reference_id;

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode and queue an CCBSDeactivate message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode CCBSDeactivate.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_cc_deactivate_encode(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record)
{
	unsigned char buffer[256];
	unsigned char *end;

	end =
		enc_etsi_ptmp_cc_deactivate(ctrl, buffer, buffer + sizeof(buffer), cc_record);
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL);
}

/*!
 * \internal
 * \brief Encode and send an CCBSDeactivate message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode CCBSDeactivate.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int send_cc_deactivate_req(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record)
{
	if (rose_cc_deactivate_encode(ctrl, call, cc_record)
		|| q931_facility(ctrl, call)) {
		pri_message(ctrl,
			"Could not schedule facility message for CCBSDeactivate.\n");
		return -1;
	}

	return 0;
}

/*!
 * \internal
 * \brief Encode ETSI PTMP CCBSBFree message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param cc_record Call completion record to process event.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_ptmp_ccbs_b_free(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, struct pri_cc_record *cc_record)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = get_invokeid(ctrl);
	msg.operation = ROSE_ETSI_CCBSBFree;

	if (cc_record->saved_ie_contents.length
		<= sizeof(msg.args.etsi.CCBSBFree.q931ie_contents)) {
		/* Saved BC, HLC, and LLC from initial SETUP */
		msg.args.etsi.CCBSBFree.q931ie.length = cc_record->saved_ie_contents.length;
		memcpy(msg.args.etsi.CCBSBFree.q931ie.contents, cc_record->saved_ie_contents.data,
			cc_record->saved_ie_contents.length);
	} else {
		pri_error(ctrl, "CCBSBFree q931 ie contents did not fit.\n");
	}

	q931_copy_address_to_rose(ctrl, &msg.args.etsi.CCBSBFree.address_of_b,
		&cc_record->party_b);
	msg.args.etsi.CCBSBFree.recall_mode = cc_record->option.recall_mode;
	msg.args.etsi.CCBSBFree.ccbs_reference = cc_record->ccbs_reference_id;

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode and queue an CCBSBFree message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode CCBSBFree.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_ccbs_b_free_encode(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record)
{
	unsigned char buffer[256];
	unsigned char *end;

	end =
		enc_etsi_ptmp_ccbs_b_free(ctrl, buffer, buffer + sizeof(buffer), cc_record);
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL);
}

/*!
 * \internal
 * \brief Encode and send an CCBSBFree message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode CCBSBFree.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int send_ccbs_b_free(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record)
{
/*
 * XXX May need to add called-party-ie with Party A number in FACILITY message. (CCBSBFree)
 * ETSI EN 300-195-1 Section 5.41 MSN interaction.
 */
	if (rose_ccbs_b_free_encode(ctrl, call, cc_record)
		|| q931_facility(ctrl, call)) {
		pri_message(ctrl,
			"Could not schedule facility message for CCBSBFree.\n");
		return -1;
	}

	return 0;
}

/*!
 * \internal
 * \brief Encode ETSI PTMP CCBSRemoteUserFree message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param cc_record Call completion record to process event.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_ptmp_remote_user_free(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, struct pri_cc_record *cc_record)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = get_invokeid(ctrl);
	msg.operation = ROSE_ETSI_CCBSRemoteUserFree;

	if (cc_record->saved_ie_contents.length
		<= sizeof(msg.args.etsi.CCBSRemoteUserFree.q931ie_contents)) {
		/* Saved BC, HLC, and LLC from initial SETUP */
		msg.args.etsi.CCBSRemoteUserFree.q931ie.length =
			cc_record->saved_ie_contents.length;
		memcpy(msg.args.etsi.CCBSRemoteUserFree.q931ie.contents,
			cc_record->saved_ie_contents.data, cc_record->saved_ie_contents.length);
	} else {
		pri_error(ctrl, "CCBSRemoteUserFree q931 ie contents did not fit.\n");
	}

	q931_copy_address_to_rose(ctrl, &msg.args.etsi.CCBSRemoteUserFree.address_of_b,
		&cc_record->party_b);
	msg.args.etsi.CCBSRemoteUserFree.recall_mode = cc_record->option.recall_mode;
	msg.args.etsi.CCBSRemoteUserFree.ccbs_reference = cc_record->ccbs_reference_id;

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode Q.SIG CcOptionalArg for ccCancel/ccExecPossible message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param cc_record Call completion record to process event.
 * \param msgtype Q.931 message type to put facility ie in.
 * \param operation library encoded operation-value
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_qsig_cc_optional_arg(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, struct pri_cc_record *cc_record, int msgtype,
	enum rose_operation operation)
{
	struct fac_extension_header header;
	struct rose_msg_invoke msg;

	memset(&header, 0, sizeof(header));
	header.nfe_present = 1;
	header.nfe.source_entity = 0;	/* endPINX */
	header.nfe.destination_entity = 0;	/* endPINX */
	header.interpretation_present = 1;
	header.interpretation = 1;	/* clearCallIfAnyInvokePduNotRecognised */
	pos = facility_encode_header(ctrl, pos, end, &header);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = get_invokeid(ctrl);
	msg.operation = operation;

	if (cc_record && msgtype == Q931_SETUP) {
		msg.args.qsig.CcCancel.full_arg_present = 1;

		/* Fill in Party A address. */
		q931_copy_number_to_rose(ctrl, &msg.args.qsig.CcCancel.number_a,
			&cc_record->party_a.number);
		q931_copy_subaddress_to_rose(ctrl, &msg.args.qsig.CcCancel.subaddr_a,
			&cc_record->party_a.subaddress);

		/* Fill in Party B address. */
		q931_copy_number_to_rose(ctrl, &msg.args.qsig.CcCancel.number_b,
			&cc_record->party_b.number);
		q931_copy_subaddress_to_rose(ctrl, &msg.args.qsig.CcCancel.subaddr_b,
			&cc_record->party_b.subaddress);

		/* Fill in service Q.931 ie information. */
		if (cc_record->saved_ie_contents.length
			<= sizeof(msg.args.qsig.CcCancel.q931ie_contents)) {
			/* Saved BC, HLC, and LLC from initial SETUP */
			msg.args.qsig.CcCancel.q931ie.length = cc_record->saved_ie_contents.length;
			memcpy(msg.args.qsig.CcCancel.q931ie.contents,
				cc_record->saved_ie_contents.data, cc_record->saved_ie_contents.length);
		} else {
			pri_error(ctrl, "CcOptionalArg q931 ie contents did not fit.\n");
		}
	}

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode and queue a remote user free message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode remote user free message.
 * \param cc_record Call completion record to process event.
 * \param msgtype Q.931 message type to put facility ie in.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_remote_user_free_encode(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, int msgtype)
{
	unsigned char buffer[256];
	unsigned char *end;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		if (PTMP_MODE(ctrl)) {
			end =
				enc_etsi_ptmp_remote_user_free(ctrl, buffer, buffer + sizeof(buffer),
					cc_record);
		} else {
			end =
				enc_etsi_ptp_cc_operation(ctrl, buffer, buffer + sizeof(buffer),
					ROSE_ETSI_CCBS_T_RemoteUserFree);
		}
		break;
	case PRI_SWITCH_QSIG:
		end = enc_qsig_cc_optional_arg(ctrl, buffer, buffer + sizeof(buffer), cc_record,
			msgtype, ROSE_QSIG_CcExecPossible);
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
 * \internal
 * \brief Encode and send a CC facility event in a SETUP message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param cc_record Call completion record to process event.
 * \param encode Function to encode facility to send out in SETUP message.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int pri_cc_send_setup_encode(struct pri *ctrl, struct pri_cc_record *cc_record,
	int (*encode)(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record,
		int msgtype))
{
	struct pri_sr req;
	q931_call *call;

	call = q931_new_call(ctrl);
	if (!call) {
		return -1;
	}

	/* Link the new call as the signaling link. */
	cc_record->signaling = call;
	call->cc.record = cc_record;

	if (encode(ctrl, call, cc_record, Q931_SETUP)) {
		/* Should not happen. */
		q931_destroycall(ctrl, call);
		return -1;
	}

	pri_sr_init(&req);
	if (cc_record->is_agent) {
		q931_party_address_to_id(&req.caller, &cc_record->party_b);
		q931_party_id_to_address(&req.called, &cc_record->party_a);
	} else {
		req.caller = cc_record->party_a;
		req.called = cc_record->party_b;
	}
	//req.cis_auto_disconnect = 0;
	req.cis_call = 1;
	if (q931_setup(ctrl, call, &req)) {
		/* Should not happen. */
		q931_destroycall(ctrl, call);
		return -1;
	}
	return 0;
}

/*!
 * \internal
 * \brief Encode and send an remote user free message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int send_remote_user_free(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	int retval;
	q931_call *call;

/*
 * XXX May need to add called-party-ie with Party A number in FACILITY message. (CCBSRemoteUserFree)
 * ETSI EN 300-195-1 Section 5.41 MSN interaction.
 */
	retval = -1;
	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		call = cc_record->signaling;
		retval = rose_remote_user_free_encode(ctrl, call, cc_record, Q931_FACILITY);
		if (!retval) {
			retval = q931_facility(ctrl, call);
		}
		break;
	case PRI_SWITCH_QSIG:
		/* ccExecPossible could be sent in FACILITY or SETUP. */
		call = cc_record->signaling;
		if (call) {
			retval = rose_remote_user_free_encode(ctrl, call, cc_record, Q931_FACILITY);
			if (!retval) {
				retval = q931_facility(ctrl, call);
			}
		} else {
			retval = pri_cc_send_setup_encode(ctrl, cc_record,
				rose_remote_user_free_encode);
		}
		break;
	default:
		break;
	}
	if (retval) {
		pri_message(ctrl, "Could not schedule message for remote user free.\n");
		return -1;
	}
	return 0;
}

/*!
 * \internal
 * \brief Encode and queue a Q.SIG ccCancel message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode remote user free message.
 * \param cc_record Call completion record to process event.
 * \param msgtype Q.931 message type to put facility ie in.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_cc_cancel(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, int msgtype)
{
	unsigned char buffer[256];
	unsigned char *end;

	end = enc_qsig_cc_optional_arg(ctrl, buffer, buffer + sizeof(buffer), cc_record,
		msgtype, ROSE_QSIG_CcCancel);
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, msgtype, buffer, end - buffer, NULL);
}

/*!
 * \internal
 * \brief Encode and send a Q.SIG ccCancel message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int send_cc_cancel(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	int retval;
	q931_call *call;

	/*
	 * ccCancel could be sent in SETUP or RELEASE.
	 * If ccPathReserve is supported it could also be sent in DISCONNECT.
	 */
	retval = -1;
	call = cc_record->signaling;
	if (call) {
		retval = rose_cc_cancel(ctrl, call, cc_record, Q931_ANY_MESSAGE);
		if (!retval) {
			retval = pri_hangup(ctrl, call, -1);
		}
	} else {
		retval = pri_cc_send_setup_encode(ctrl, cc_record, rose_cc_cancel);
	}
	if (retval) {
		pri_message(ctrl, "Could not schedule message for ccCancel.\n");
		return -1;
	}
	return 0;
}

/*!
 * \internal
 * \brief Encode and queue a CC suspend message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode CC suspend message.
 * \param msgtype Q.931 message type to put facility ie in.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_cc_suspend_encode(struct pri *ctrl, q931_call *call, int msgtype)
{
	unsigned char buffer[256];
	unsigned char *end;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		end =
			enc_etsi_ptp_cc_operation(ctrl, buffer, buffer + sizeof(buffer),
				ROSE_ETSI_CCBS_T_Suspend);
		break;
	case PRI_SWITCH_QSIG:
		end =
			enc_qsig_cc_extension_event(ctrl, buffer, buffer + sizeof(buffer),
				ROSE_QSIG_CcSuspend, 0/* discardAnyUnrecognisedInvokePdu */);
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
 * \internal
 * \brief Encode and send a CC suspend message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int send_cc_suspend(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	int retval;
	q931_call *call;

	retval = -1;
	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		call = cc_record->signaling;
		retval = rose_cc_suspend_encode(ctrl, call, Q931_FACILITY);
		if (!retval) {
			retval = q931_facility(ctrl, call);
		}
		break;
	case PRI_SWITCH_QSIG:
		/*
		 * Suspend is sent in a CONNECT or FACILITY message.
		 * If ccPathReserve is supported, it could also be sent in
		 * RELEASE or DISCONNECT.
		 */
		call = cc_record->signaling;
		if (!call) {
			break;
		}
		retval = rose_cc_suspend_encode(ctrl, call, Q931_ANY_MESSAGE);
		if (!retval) {
			if (call->ourcallstate == Q931_CALL_STATE_ACTIVE) {
				retval = q931_facility(ctrl, call);
			} else {
				retval = q931_connect(ctrl, call, 0, 0);
			}
		}
		break;
	default:
		break;
	}
	if (retval) {
		pri_message(ctrl, "Could not schedule message for CC suspend.\n");
		return -1;
	}
	return 0;
}

/*!
 * \internal
 * \brief Encode and queue a CC resume message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode CC resume message.
 * \param msgtype Q.931 message type to put facility ie in.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_cc_resume_encode(struct pri *ctrl, q931_call *call, int msgtype)
{
	unsigned char buffer[256];
	unsigned char *end;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		end =
			enc_etsi_ptp_cc_operation(ctrl, buffer, buffer + sizeof(buffer),
				ROSE_ETSI_CCBS_T_Resume);
		break;
	case PRI_SWITCH_QSIG:
		end =
			enc_qsig_cc_extension_event(ctrl, buffer, buffer + sizeof(buffer),
				ROSE_QSIG_CcResume, 0/* discardAnyUnrecognisedInvokePdu */);
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
 * \internal
 * \brief Encode and send a CC resume message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int send_cc_resume(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	q931_call *call;

	call = cc_record->signaling;
	if (!call
		|| rose_cc_resume_encode(ctrl, call, Q931_FACILITY)
		|| q931_facility(ctrl, call)) {
		pri_message(ctrl, "Could not schedule message for CC resume.\n");
		return -1;
	}
	return 0;
}

/*!
 * \internal
 * \brief Encode ETSI PTMP CCBSStopAlerting message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param cc_record Call completion record to process event.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_ptmp_ccbs_stop_alerting(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, struct pri_cc_record *cc_record)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = get_invokeid(ctrl);
	msg.operation = ROSE_ETSI_CCBSStopAlerting;

	msg.args.etsi.CCBSStopAlerting.ccbs_reference = cc_record->ccbs_reference_id;

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode and queue an CCBSStopAlerting message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode CCBSStopAlerting.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_ccbs_stop_alerting_encode(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record)
{
	unsigned char buffer[256];
	unsigned char *end;

	end =
		enc_etsi_ptmp_ccbs_stop_alerting(ctrl, buffer, buffer + sizeof(buffer), cc_record);
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL);
}

/*!
 * \internal
 * \brief Encode and send CCBSStopAlerting message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode remote user free.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int send_ccbs_stop_alerting(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record)
{
	if (rose_ccbs_stop_alerting_encode(ctrl, call, cc_record)
		|| q931_facility(ctrl, call)) {
		pri_message(ctrl,
			"Could not schedule facility message for CCBSStopAlerting.\n");
		return -1;
	}

	return 0;
}

/*!
 * \internal
 * \brief Encode ETSI PTMP CCBSCall message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param cc_record Call completion record to process event.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_ptmp_cc_recall(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, struct pri_cc_record *cc_record)
{
	struct rose_msg_invoke msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = get_invokeid(ctrl);
	msg.operation = ROSE_ETSI_CCBSCall;

	msg.args.etsi.CCBSCall.ccbs_reference = cc_record->ccbs_reference_id;

	pos = rose_encode_invoke(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode and queue a cc-recall message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode cc-recall.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_cc_recall_encode(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record)
{
	unsigned char buffer[256];
	unsigned char *end;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		if (PTMP_MODE(ctrl)) {
			end =
				enc_etsi_ptmp_cc_recall(ctrl, buffer, buffer + sizeof(buffer), cc_record);
		} else {
			end =
				enc_etsi_ptp_cc_operation(ctrl, buffer, buffer + sizeof(buffer),
					ROSE_ETSI_CCBS_T_Call);
		}
		break;
	case PRI_SWITCH_QSIG:
		end =
			enc_qsig_cc_extension_event(ctrl, buffer, buffer + sizeof(buffer),
				ROSE_QSIG_CcRingout, 0/* discardAnyUnrecognisedInvokePdu */);
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
 * \brief Copy the cc information into the ETSI ROSE call-information record.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call_information ROSE call-information record to fill in.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void q931_copy_call_information_to_etsi_rose(struct pri *ctrl, struct roseEtsiCallInformation *call_information, const struct pri_cc_record *cc_record)
{
	q931_copy_address_to_rose(ctrl, &call_information->address_of_b, &cc_record->party_b);

	if (cc_record->saved_ie_contents.length
		<= sizeof(call_information->q931ie_contents)) {
		/* Saved BC, HLC, and LLC from initial SETUP */
		call_information->q931ie.length = cc_record->saved_ie_contents.length;
		memcpy(call_information->q931ie.contents, cc_record->saved_ie_contents.data,
			cc_record->saved_ie_contents.length);
	} else {
		pri_error(ctrl, "call-information q931 ie contents did not fit.\n");
	}

	call_information->ccbs_reference = cc_record->ccbs_reference_id;

	q931_copy_subaddress_to_rose(ctrl, &call_information->subaddress_of_a,
		&cc_record->party_a.subaddress);
}

/*!
 * \internal
 * \brief Encode ETSI PTMP specific CCBSInterrogate/CCNRInterrogate result message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param invoke Decoded ROSE invoke message contents.
 * \param cc_record Call completion record to process event.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_ptmp_cc_interrogate_rsp_specific(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const struct rose_msg_invoke *invoke,
	const struct pri_cc_record *cc_record)
{
	struct rose_msg_result msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = invoke->invoke_id;
	msg.operation = invoke->operation;

	msg.args.etsi.CCBSInterrogate.recall_mode = cc_record->option.recall_mode;
	msg.args.etsi.CCBSInterrogate.call_details.num_records = 1;
	q931_copy_call_information_to_etsi_rose(ctrl,
		&msg.args.etsi.CCBSInterrogate.call_details.list[0], cc_record);

	pos = rose_encode_result(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode ETSI PTMP general CCBSInterrogate/CCNRInterrogate result message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param invoke Decoded ROSE invoke message contents.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_etsi_ptmp_cc_interrogate_rsp_general(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const struct rose_msg_invoke *invoke)
{
	struct rose_msg_result msg;
	struct q931_party_number party_a_number;
	const struct pri_cc_record *cc_record;
	unsigned char *new_pos;
	unsigned idx;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = invoke->invoke_id;
	msg.operation = invoke->operation;

	msg.args.etsi.CCBSInterrogate.recall_mode = ctrl->cc.option.recall_mode;

	/* Convert the given party A number. */
	q931_party_number_init(&party_a_number);
	if (invoke->args.etsi.CCBSInterrogate.a_party_number.length) {
		/* The party A number was given. */
		rose_copy_number_to_q931(ctrl, &party_a_number,
			&invoke->args.etsi.CCBSInterrogate.a_party_number);
	}

	/* Build the CallDetails list. */
	idx = 0;
	for (cc_record = ctrl->cc.pool; cc_record; cc_record = cc_record->next) {
		if (cc_record->ccbs_reference_id == CC_PTMP_INVALID_ID
			|| (!cc_record->is_ccnr) != (invoke->operation == ROSE_ETSI_CCBSInterrogate)) {
			/*
			 * Record does not have a reference id yet
			 * or is not for the requested CCBS/CCNR mode.
			 */
			continue;
		}
		if (party_a_number.valid) {
			/* The party A number was given. */
			party_a_number.presentation = cc_record->party_a.number.presentation;
			if (q931_party_number_cmp(&party_a_number, &cc_record->party_a.number)) {
				/* Record party A does not match. */
				continue;
			}
		}

		/* Add call information to the CallDetails list. */
		q931_copy_call_information_to_etsi_rose(ctrl,
			&msg.args.etsi.CCBSInterrogate.call_details.list[idx], cc_record);

		++idx;
		if (ARRAY_LEN(msg.args.etsi.CCBSInterrogate.call_details.list) <= idx) {
			/* List is full. */
			break;
		}
	}
	msg.args.etsi.CCBSInterrogate.call_details.num_records = idx;

	new_pos = rose_encode_result(ctrl, pos, end, &msg);

	/* Reduce the CallDetails list until it fits into the given buffer. */
	while (!new_pos && msg.args.etsi.CCBSInterrogate.call_details.num_records) {
		--msg.args.etsi.CCBSInterrogate.call_details.num_records;
		new_pos = rose_encode_result(ctrl, pos, end, &msg);
	}

	return new_pos;
}

/*!
 * \internal
 * \brief Encode and queue a specific CCBSInterrogate/CCNRInterrogate result message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode CCBSInterrogate/CCNRInterrogate response.
 * \param invoke Decoded ROSE invoke message contents.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_cc_interrogate_rsp_specific(struct pri *ctrl, q931_call *call, const struct rose_msg_invoke *invoke, const struct pri_cc_record *cc_record)
{
	unsigned char buffer[256];
	unsigned char *end;

	end =
		enc_etsi_ptmp_cc_interrogate_rsp_specific(ctrl, buffer, buffer + sizeof(buffer),
			invoke, cc_record);
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL);
}

/*!
 * \internal
 * \brief Encode and queue a general CCBSInterrogate/CCNRInterrogate result message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode CCBSInterrogate/CCNRInterrogate response.
 * \param invoke Decoded ROSE invoke message contents.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_cc_interrogate_rsp_general(struct pri *ctrl, q931_call *call, const struct rose_msg_invoke *invoke)
{
	unsigned char buffer[256];
	unsigned char *end;

	end =
		enc_etsi_ptmp_cc_interrogate_rsp_general(ctrl, buffer, buffer + sizeof(buffer),
			invoke);
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL);
}

/*!
 * \brief Respond to the received CCBSInterrogate/CCNRInterrogate invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which the message came.
 * \param invoke Decoded ROSE invoke message contents.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int pri_cc_interrogate_rsp(struct pri *ctrl, q931_call *call, const struct rose_msg_invoke *invoke)
{
	int encode_result;

	if (!ctrl->cc_support) {
		/* Call completion is disabled. */
		return send_facility_error(ctrl, call, invoke->invoke_id,
			ROSE_ERROR_Gen_NotSubscribed);
	}

	if (invoke->args.etsi.CCBSInterrogate.ccbs_reference_present) {
		struct pri_cc_record *cc_record;

		/* Specific CC request interrogation. */
		cc_record = pri_cc_find_by_reference(ctrl,
			invoke->args.etsi.CCBSInterrogate.ccbs_reference);
		if (!cc_record
			|| ((!cc_record->is_ccnr)
				== (invoke->operation == ROSE_ETSI_CCBSInterrogate))) {
			/* Record does not exist or is not for the requested CCBS/CCNR mode. */
			return send_facility_error(ctrl, call, invoke->invoke_id,
				ROSE_ERROR_CCBS_InvalidCCBSReference);
		}
		encode_result = rose_cc_interrogate_rsp_specific(ctrl, call, invoke, cc_record);
	} else {
		/* General CC request interrogation. */
		encode_result = rose_cc_interrogate_rsp_general(ctrl, call, invoke);
	}

	if (encode_result || q931_facility(ctrl, call)) {
		pri_message(ctrl,
			"Could not schedule facility message for cc-interrogate.\n");
		return -1;
	}

	return 0;
}

/*!
 * \brief Respond to the received PTMP CCBSRequest/CCNRRequest invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which the message came.
 * \param invoke Decoded ROSE invoke message contents.
 *
 * \return Nothing
 */
void pri_cc_ptmp_request(struct pri *ctrl, q931_call *call, const struct rose_msg_invoke *invoke)
{
	struct pri_cc_record *cc_record;

	if (!ctrl->cc_support) {
		/* Call completion is disabled. */
		send_facility_error(ctrl, call, invoke->invoke_id,
			ROSE_ERROR_Gen_NotSubscribed);
		return;
	}
	cc_record = pri_cc_find_by_linkage(ctrl,
		invoke->args.etsi.CCBSRequest.call_linkage_id);
	if (!cc_record) {
		send_facility_error(ctrl, call, invoke->invoke_id,
			ROSE_ERROR_CCBS_InvalidCallLinkageID);
		return;
	}
	if (cc_record->state != CC_STATE_AVAILABLE) {
		send_facility_error(ctrl, call, invoke->invoke_id,
			ROSE_ERROR_CCBS_IsAlreadyActivated);
		return;
	}
	cc_record->ccbs_reference_id = pri_cc_new_reference_id(ctrl);
	if (cc_record->ccbs_reference_id == CC_PTMP_INVALID_ID) {
		/* Could not allocate a call reference id. */
		send_facility_error(ctrl, call, invoke->invoke_id,
			ROSE_ERROR_CCBS_OutgoingCCBSQueueFull);
		return;
	}

	/* Save off data to know how to send back any response. */
	cc_record->response.signaling = call;
	cc_record->response.invoke_operation = invoke->operation;
	cc_record->response.invoke_id = invoke->invoke_id;

	/* Set the requested CC mode. */
	cc_record->is_ccnr = (invoke->operation == ROSE_ETSI_CCNRRequest) ? 1 : 0;

	pri_cc_event(ctrl, call, cc_record, CC_EVENT_CC_REQUEST);
}

/*!
 * \brief Respond to the received PTP CCBS_T_Request/CCNR_T_Request invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which the message came.
 * \param msgtype Q.931 message type ie is in.
 * \param invoke Decoded ROSE invoke message contents.
 *
 * \return Nothing
 */
void pri_cc_ptp_request(struct pri *ctrl, q931_call *call, int msgtype, const struct rose_msg_invoke *invoke)
{
	struct pri_cc_record *cc_record;
	struct q931_party_address party_a;
	struct q931_party_address party_b;

	if (msgtype != Q931_REGISTER) {
		/* Ignore CC request message since it did not come in on the correct message. */
		return;
	}
	if (!ctrl->cc_support) {
		/* Call completion is disabled. */
		rose_error_msg_encode(ctrl, call, Q931_ANY_MESSAGE, invoke->invoke_id,
			ROSE_ERROR_Gen_NotSubscribed);
		call->cc.hangup_call = 1;
		return;
	}

	q931_party_address_init(&party_a);
	if (invoke->args.etsi.CCBS_T_Request.originating.number.length) {
		/* The originating number is present. */
		rose_copy_address_to_q931(ctrl, &party_a,
			&invoke->args.etsi.CCBS_T_Request.originating);
	}
	q931_party_address_init(&party_b);
	rose_copy_address_to_q931(ctrl, &party_b,
		&invoke->args.etsi.CCBS_T_Request.destination);
	cc_record = pri_cc_find_by_addressing(ctrl, &party_a, &party_b,
		invoke->args.etsi.CCBS_T_Request.q931ie.length,
		invoke->args.etsi.CCBS_T_Request.q931ie.contents);
	if (!cc_record || cc_record->state != CC_STATE_AVAILABLE) {
		/* Could not find the record or already activated */
		rose_error_msg_encode(ctrl, call, Q931_ANY_MESSAGE, invoke->invoke_id,
			ROSE_ERROR_CCBS_T_ShortTermDenial);
		call->cc.hangup_call = 1;
		return;
	}

	/*
	 * We already have the presentationAllowedIndicator in the cc_record
	 * when we saved the original call information.
	 */
#if 0
	if (invoke->args.etsi.CCBS_T_Request.presentation_allowed_indicator_present) {
		if (invoke->args.etsi.CCBS_T_Request.presentation_allowed_indicator) {
			if (party_a.number.str[0]) {
				party_a.number.presentation =
					PRI_PRES_ALLOWED | PRI_PRES_USER_NUMBER_UNSCREENED;
			} else {
				party_a.number.presentation =
					PRI_PRES_UNAVAILABLE | PRI_PRES_USER_NUMBER_UNSCREENED;
			}
		} else {
			party_a.number.presentation =
				PRI_PRES_RESTRICTED | PRI_PRES_USER_NUMBER_UNSCREENED;
		}
	}
#endif

	/* Link the signaling link to the cc_record. */
	call->cc.record = cc_record;
	cc_record->signaling = call;

	/* Save off data to know how to send back any response. */
	//cc_record->response.signaling = call;
	cc_record->response.invoke_operation = invoke->operation;
	cc_record->response.invoke_id = invoke->invoke_id;

	/* Set the requested CC mode. */
	cc_record->is_ccnr = (invoke->operation == ROSE_ETSI_CCNR_T_Request) ? 1 : 0;

	/* Lets keep this signaling link around for awhile. */
	call->cis_recognized = 1;

	pri_cc_event(ctrl, call, cc_record, CC_EVENT_CC_REQUEST);
}

/*!
 * \brief Respond to the received Q.SIG ccbsRequest/ccnrRequest invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which the message came.
 * \param msgtype Q.931 message type ie is in.
 * \param invoke Decoded ROSE invoke message contents.
 *
 * \return Nothing
 */
void pri_cc_qsig_request(struct pri *ctrl, q931_call *call, int msgtype, const struct rose_msg_invoke *invoke)
{
	struct pri_cc_record *cc_record;
	struct q931_party_address party_a;
	struct q931_party_address party_b;

	if (msgtype != Q931_SETUP) {
		/* Ignore CC request message since it did not come in on the correct message. */
		return;
	}
	if (!ctrl->cc_support) {
		/* Call completion is disabled. */
		rose_error_msg_encode(ctrl, call, Q931_ANY_MESSAGE, invoke->invoke_id,
			ROSE_ERROR_QSIG_LongTermRejection);
		call->cc.hangup_call = 1;
		return;
	}

	/* Extract Party A address. */
	rose_copy_presented_number_unscreened_to_q931(ctrl, &party_a.number,
		&invoke->args.qsig.CcbsRequest.number_a);
	q931_party_subaddress_init(&party_a.subaddress);
	rose_copy_subaddress_to_q931(ctrl, &party_a.subaddress,
		&invoke->args.qsig.CcbsRequest.subaddr_a);

	/* Extract Party B address. */
	q931_party_address_init(&party_b);
	rose_copy_number_to_q931(ctrl, &party_b.number,
		&invoke->args.qsig.CcbsRequest.number_b);
	rose_copy_subaddress_to_q931(ctrl, &party_b.subaddress,
		&invoke->args.qsig.CcbsRequest.subaddr_b);

	cc_record = pri_cc_find_by_addressing(ctrl, &party_a, &party_b,
		invoke->args.qsig.CcbsRequest.q931ie.length,
		invoke->args.qsig.CcbsRequest.q931ie.contents);
	if (!cc_record || cc_record->state != CC_STATE_AVAILABLE) {
		/* Could not find the record or already activated */
		rose_error_msg_encode(ctrl, call, Q931_ANY_MESSAGE, invoke->invoke_id,
			ROSE_ERROR_QSIG_ShortTermRejection);
		call->cc.hangup_call = 1;
		return;
	}

	/* Determine negotiated signaling retention method. */
	if (invoke->args.qsig.CcbsRequest.retain_sig_connection_present) {
		/* We will do what the originator desires. */
		cc_record->option.retain_signaling_link =
			invoke->args.qsig.CcbsRequest.retain_sig_connection;
	} else {
		/* The originator does not care.  Do how we are configured. */
		cc_record->option.retain_signaling_link =
			ctrl->cc.option.signaling_retention_rsp;
	}
	if (!cc_record->party_a.number.valid || cc_record->party_a.number.str[0] == '\0') {
		/*
		 * Party A number is not available for us to initiate
		 * a signaling link.  We must retain the signaling link.
		 */
		cc_record->option.retain_signaling_link = 1;
	}

	/* Link the signaling link to the cc_record. */
	call->cc.record = cc_record;
	cc_record->signaling = call;

	/* Save off data to know how to send back any response. */
	//cc_record->response.signaling = call;
	cc_record->response.invoke_operation = invoke->operation;
	cc_record->response.invoke_id = invoke->invoke_id;

	/* Set the requested CC mode. */
	cc_record->is_ccnr = (invoke->operation == ROSE_QSIG_CcnrRequest) ? 1 : 0;

	/* Lets keep this signaling link around for awhile. */
	call->cis_recognized = 1;

	pri_cc_event(ctrl, call, cc_record, CC_EVENT_CC_REQUEST);
}

/*!
 * \brief Handle the received Q.SIG ccCancel invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which the message came.
 * \param msgtype Q.931 message type ie is in.
 * \param invoke Decoded ROSE invoke message contents.
 *
 * \return Nothing
 */
void pri_cc_qsig_cancel(struct pri *ctrl, q931_call *call, int msgtype, const struct rose_msg_invoke *invoke)
{
	struct pri_cc_record *cc_record;
	struct q931_party_address party_a;
	struct q931_party_address party_b;

	cc_record = call->cc.record;
	if (!cc_record) {
		/* The current call is not associated with the cc_record. */
		if (invoke->args.qsig.CcCancel.full_arg_present) {
			/* Extract Party A address. */
			q931_party_address_init(&party_a);
			rose_copy_number_to_q931(ctrl, &party_a.number,
				&invoke->args.qsig.CcCancel.number_a);
			rose_copy_subaddress_to_q931(ctrl, &party_a.subaddress,
				&invoke->args.qsig.CcCancel.subaddr_a);

			/* Extract Party B address. */
			q931_party_address_init(&party_b);
			rose_copy_number_to_q931(ctrl, &party_b.number,
				&invoke->args.qsig.CcCancel.number_b);
			rose_copy_subaddress_to_q931(ctrl, &party_b.subaddress,
				&invoke->args.qsig.CcCancel.subaddr_b);

			cc_record = pri_cc_find_by_addressing(ctrl, &party_a, &party_b,
				invoke->args.qsig.CcCancel.q931ie.length,
				invoke->args.qsig.CcCancel.q931ie.contents);
		}
		if (!cc_record) {
			/*
			 * Could not find the cc_record
			 * or not enough information to look up a cc_record.
			 */
			if (msgtype == Q931_SETUP) {
				call->cc.hangup_call = 1;
			}
			return;
		}
	}

	if (msgtype == Q931_SETUP && call->cis_call) {
		if (cc_record->signaling) {
			/*
			 * We already have a signaling link.
			 * This could be a collision with our ccExecPossible.
			 * Could this be an alias match?
			 */
			switch (cc_record->state) {
			case CC_STATE_WAIT_CALLBACK:
				if (ctrl->debug & PRI_DEBUG_CC) {
					pri_message(ctrl,
						"-- Collision with our ccExecPossible event call.  Canceling CC.\n");
				}
				break;
			default:
				pri_message(ctrl,
					"-- Warning: Possible Q.SIG CC alias match.  Canceling CC.\n");
				break;
			}
			cc_record->fsm.qsig.msgtype = msgtype;
			pri_cc_event(ctrl, call, cc_record, CC_EVENT_LINK_CANCEL);

			call->cc.hangup_call = 1;
			return;
		}

		/* Link the signaling link to the cc_record. */
		call->cc.record = cc_record;
		cc_record->signaling = call;

		/* Lets keep this signaling link around for awhile. */
		call->cis_recognized = 1;
	}

	cc_record->fsm.qsig.msgtype = msgtype;
	pri_cc_event(ctrl, call, cc_record, CC_EVENT_LINK_CANCEL);
}

/*!
 * \brief Handle the received Q.SIG ccExecPossible invoke message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which the message came.
 * \param msgtype Q.931 message type ie is in.
 * \param invoke Decoded ROSE invoke message contents.
 *
 * \return Nothing
 */
void pri_cc_qsig_exec_possible(struct pri *ctrl, q931_call *call, int msgtype, const struct rose_msg_invoke *invoke)
{
	struct pri_cc_record *cc_record;
	struct q931_party_address party_a;
	struct q931_party_address party_b;

	cc_record = call->cc.record;
	if (!cc_record) {
		/* The current call is not associated with the cc_record. */
		if (invoke->args.qsig.CcExecPossible.full_arg_present) {
			/* Extract Party A address. */
			q931_party_address_init(&party_a);
			rose_copy_number_to_q931(ctrl, &party_a.number,
				&invoke->args.qsig.CcExecPossible.number_a);
			rose_copy_subaddress_to_q931(ctrl, &party_a.subaddress,
				&invoke->args.qsig.CcExecPossible.subaddr_a);

			/* Extract Party B address. */
			q931_party_address_init(&party_b);
			rose_copy_number_to_q931(ctrl, &party_b.number,
				&invoke->args.qsig.CcExecPossible.number_b);
			rose_copy_subaddress_to_q931(ctrl, &party_b.subaddress,
				&invoke->args.qsig.CcExecPossible.subaddr_b);

			cc_record = pri_cc_find_by_addressing(ctrl, &party_a, &party_b,
				invoke->args.qsig.CcExecPossible.q931ie.length,
				invoke->args.qsig.CcExecPossible.q931ie.contents);
		}
		if (!cc_record) {
			/*
			 * Could not find the cc_record
			 * or not enough information to look up a cc_record.
			 */
			rose_cc_cancel(ctrl, call, NULL, Q931_ANY_MESSAGE);
			if (msgtype == Q931_SETUP) {
				call->cc.hangup_call = 1;
			} else {
				/* msgtype should be Q931_FACILITY. */
				pri_hangup(ctrl, call, -1);
			}
			return;
		}
	}

	if (msgtype == Q931_SETUP && call->cis_call) {
		if (cc_record->signaling) {
			/*
			 * We already have a signaling link.  This should not happen.
			 * Could this be an alias match?
			 */
			pri_message(ctrl,
				"-- Warning: Possible Q.SIG CC alias match.  Sending ccCancel back.\n");
			rose_cc_cancel(ctrl, call, NULL, Q931_ANY_MESSAGE);
			call->cc.hangup_call = 1;
			return;
		}

		/* Link the signaling link to the cc_record. */
		call->cc.record = cc_record;
		cc_record->signaling = call;

		/* Lets keep this signaling link around for awhile. */
		call->cis_recognized = 1;
	}

	cc_record->fsm.qsig.msgtype = msgtype;
	pri_cc_event(ctrl, call, cc_record, CC_EVENT_REMOTE_USER_FREE);
}

/*!
 * \brief Convert the given call completion state to a string.
 *
 * \param state CC state to convert to string.
 *
 * \return String version of call completion state.
 */
const char *pri_cc_fsm_state_str(enum CC_STATES state)
{
	const char *str;

	str = "Unknown";
	switch (state) {
	case CC_STATE_IDLE:
		str = "CC_STATE_IDLE";
		break;
	case CC_STATE_PENDING_AVAILABLE:
		str = "CC_STATE_PENDING_AVAILABLE";
		break;
	case CC_STATE_AVAILABLE:
		str = "CC_STATE_AVAILABLE";
		break;
	case CC_STATE_REQUESTED:
		str = "CC_STATE_REQUESTED";
		break;
	case CC_STATE_ACTIVATED:
		str = "CC_STATE_ACTIVATED";
		break;
	case CC_STATE_B_AVAILABLE:
		str = "CC_STATE_B_AVAILABLE";
		break;
	case CC_STATE_SUSPENDED:
		str = "CC_STATE_SUSPENDED";
		break;
	case CC_STATE_WAIT_CALLBACK:
		str = "CC_STATE_WAIT_CALLBACK";
		break;
	case CC_STATE_CALLBACK:
		str = "CC_STATE_CALLBACK";
		break;
	case CC_STATE_WAIT_DESTRUCTION:
		str = "CC_STATE_WAIT_DESTRUCTION";
		break;
	case CC_STATE_NUM:
		/* Not a real state. */
		break;
	}
	return str;
}

/*!
 * \brief Convert the given call completion event to a string.
 *
 * \param event CC event to convert to string.
 *
 * \return String version of call completion event.
 */
const char *pri_cc_fsm_event_str(enum CC_EVENTS event)
{
	const char *str;

	str = "Unknown";
	switch (event) {
	case CC_EVENT_AVAILABLE:
		str = "CC_EVENT_AVAILABLE";
		break;
	case CC_EVENT_CC_REQUEST:
		str = "CC_EVENT_CC_REQUEST";
		break;
	case CC_EVENT_CC_REQUEST_ACCEPT:
		str = "CC_EVENT_CC_REQUEST_ACCEPT";
		break;
	case CC_EVENT_CC_REQUEST_FAIL:
		str = "CC_EVENT_CC_REQUEST_FAIL";
		break;
	case CC_EVENT_REMOTE_USER_FREE:
		str = "CC_EVENT_REMOTE_USER_FREE";
		break;
	case CC_EVENT_B_FREE:
		str = "CC_EVENT_B_FREE";
		break;
	case CC_EVENT_STOP_ALERTING:
		str = "CC_EVENT_STOP_ALERTING";
		break;
	case CC_EVENT_A_STATUS:
		str = "CC_EVENT_A_STATUS";
		break;
	case CC_EVENT_A_FREE:
		str = "CC_EVENT_A_FREE";
		break;
	case CC_EVENT_A_BUSY:
		str = "CC_EVENT_A_BUSY";
		break;
	case CC_EVENT_SUSPEND:
		str = "CC_EVENT_SUSPEND";
		break;
	case CC_EVENT_RESUME:
		str = "CC_EVENT_RESUME";
		break;
	case CC_EVENT_RECALL:
		str = "CC_EVENT_RECALL";
		break;
	case CC_EVENT_LINK_CANCEL:
		str = "CC_EVENT_LINK_CANCEL";
		break;
	case CC_EVENT_CANCEL:
		str = "CC_EVENT_CANCEL";
		break;
	case CC_EVENT_INTERNAL_CLEARING:
		str = "CC_EVENT_INTERNAL_CLEARING";
		break;
	case CC_EVENT_SIGNALING_GONE:
		str = "CC_EVENT_SIGNALING_GONE";
		break;
	case CC_EVENT_HANGUP_SIGNALING:
		str = "CC_EVENT_HANGUP_SIGNALING";
		break;
	case CC_EVENT_MSG_ALERTING:
		str = "CC_EVENT_MSG_ALERTING";
		break;
	case CC_EVENT_MSG_DISCONNECT:
		str = "CC_EVENT_MSG_DISCONNECT";
		break;
	case CC_EVENT_MSG_RELEASE:
		str = "CC_EVENT_MSG_RELEASE";
		break;
	case CC_EVENT_MSG_RELEASE_COMPLETE:
		str = "CC_EVENT_MSG_RELEASE_COMPLETE";
		break;
	case CC_EVENT_TIMEOUT_T_ACTIVATE:
		str = "CC_EVENT_TIMEOUT_T_ACTIVATE";
		break;
#if 0
	case CC_EVENT_TIMEOUT_T_DEACTIVATE:
		str = "CC_EVENT_TIMEOUT_T_DEACTIVATE";
		break;
	case CC_EVENT_TIMEOUT_T_INTERROGATE:
		str = "CC_EVENT_TIMEOUT_T_INTERROGATE";
		break;
#endif
	case CC_EVENT_TIMEOUT_T_RETENTION:
		str = "CC_EVENT_TIMEOUT_T_RETENTION";
		break;
	case CC_EVENT_TIMEOUT_T_CCBS1:
		str = "CC_EVENT_TIMEOUT_T_CCBS1";
		break;
	case CC_EVENT_TIMEOUT_EXTENDED_T_CCBS1:
		str = "CC_EVENT_TIMEOUT_EXTENDED_T_CCBS1";
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		str = "CC_EVENT_TIMEOUT_T_SUPERVISION";
		break;
	case CC_EVENT_TIMEOUT_T_RECALL:
		str = "CC_EVENT_TIMEOUT_T_RECALL";
		break;
	}
	return str;
}

static const char pri_cc_act_header[] = "%ld  CC-Act: %s\n";
#define PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_id)							\
	if ((ctrl)->debug & PRI_DEBUG_CC) {									\
		pri_message((ctrl), pri_cc_act_header, (cc_id), __FUNCTION__);	\
	}

/*!
 * \internal
 * \brief FSM action to mark FSM for destruction.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_set_self_destruct(struct pri *ctrl, struct pri_cc_record *cc_record)
{
#if defined(CC_SANITY_CHECKS)
	struct apdu_event *msg;
#endif	/* defined(CC_SANITY_CHECKS) */

	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);

	/* Abort any pending indirect events. */
	pri_schedule_del(ctrl, cc_record->t_indirect);
	cc_record->t_indirect = 0;

#if defined(CC_SANITY_CHECKS)
	if (cc_record->t_retention) {
		pri_error(ctrl, "T_RETENTION still active");
		pri_schedule_del(ctrl, cc_record->t_retention);
		cc_record->t_retention = 0;
	}
	if (cc_record->t_supervision) {
		pri_error(ctrl, "T_SUPERVISION still active");
		pri_schedule_del(ctrl, cc_record->t_supervision);
		cc_record->t_supervision = 0;
	}
	if (cc_record->t_recall) {
		pri_error(ctrl, "T_RECALL still active");
		pri_schedule_del(ctrl, cc_record->t_recall);
		cc_record->t_recall = 0;
	}
	if (PTMP_MODE(ctrl)) {
		msg = pri_call_apdu_find(cc_record->signaling,
			cc_record->fsm.ptmp.t_ccbs1_invoke_id);
		if (msg) {
			pri_error(ctrl, "T_CCBS1 still active");
			cc_record->fsm.ptmp.t_ccbs1_invoke_id = APDU_INVALID_INVOKE_ID;
			pri_call_apdu_delete(cc_record->signaling, msg);
		}
		if (cc_record->fsm.ptmp.extended_t_ccbs1) {
			pri_error(ctrl, "Extended T_CCBS1 still active");
			pri_schedule_del(ctrl, cc_record->fsm.ptmp.extended_t_ccbs1);
			cc_record->fsm.ptmp.extended_t_ccbs1 = 0;
		}
	}
	if (cc_record->signaling) {
		msg = pri_call_apdu_find(cc_record->signaling, cc_record->t_activate_invoke_id);
		if (msg) {
			pri_error(ctrl, "T_ACTIVATE still active");
			cc_record->t_activate_invoke_id = APDU_INVALID_INVOKE_ID;
			pri_call_apdu_delete(cc_record->signaling, msg);
		}
	}
#endif	/* defined(CC_SANITY_CHECKS) */

	cc_record->fsm_complete = 1;
}

/*!
 * \internal
 * \brief FSM action to disassociate the signaling link from the cc_record.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_disassociate_signaling_link(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	pri_cc_disassociate_signaling_link(cc_record);
}

/*!
 * \internal
 * \brief FSM action to send CC available message.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 * \param call Q.931 call leg.
 * \param msgtype Q.931 message type to put facility ie in.
 *
 * \return Nothing
 */
static void pri_cc_act_send_cc_available(struct pri *ctrl, struct pri_cc_record *cc_record, q931_call *call, int msgtype)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	rose_cc_available_encode(ctrl, call, cc_record, msgtype);
}

/*!
 * \internal
 * \brief FSM action to stop the PTMP T_RETENTION timer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_stop_t_retention(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	pri_schedule_del(ctrl, cc_record->t_retention);
	cc_record->t_retention = 0;
}

/*!
 * \internal
 * \brief T_RETENTION timeout callback.
 *
 * \param data CC record pointer.
 *
 * \return Nothing
 */
static void pri_cc_timeout_t_retention(void *data)
{
	struct pri_cc_record *cc_record = data;

	cc_record->t_retention = 0;
	q931_cc_timeout(cc_record->ctrl, cc_record, CC_EVENT_TIMEOUT_T_RETENTION);
}

/*!
 * \internal
 * \brief FSM action to start the PTMP T_RETENTION timer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_start_t_retention(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	if (cc_record->t_retention) {
		pri_error(ctrl, "!! T_RETENTION is already running!");
		pri_schedule_del(ctrl, cc_record->t_retention);
	}
	cc_record->t_retention = pri_schedule_event(ctrl,
		ctrl->timers[PRI_TIMER_T_RETENTION], pri_cc_timeout_t_retention, cc_record);
}

/*!
 * \internal
 * \brief FSM action to stop the PTMP EXTENDED_T_CCBS1 timer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_stop_extended_t_ccbs1(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	pri_schedule_del(ctrl, cc_record->fsm.ptmp.extended_t_ccbs1);
	cc_record->fsm.ptmp.extended_t_ccbs1 = 0;
}

/*!
 * \internal
 * \brief EXTENDED_T_CCBS1 timeout callback.
 *
 * \param data CC record pointer.
 *
 * \return Nothing
 */
static void pri_cc_timeout_extended_t_ccbs1(void *data)
{
	struct pri_cc_record *cc_record = data;

	cc_record->fsm.ptmp.extended_t_ccbs1 = 0;
	q931_cc_timeout(cc_record->ctrl, cc_record, CC_EVENT_TIMEOUT_EXTENDED_T_CCBS1);
}

/*!
 * \internal
 * \brief FSM action to start the PTMP extended T_CCBS1 timer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_start_extended_t_ccbs1(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	if (cc_record->fsm.ptmp.extended_t_ccbs1) {
		pri_error(ctrl, "!! Extended T_CCBS1 is already running!");
		pri_schedule_del(ctrl, cc_record->fsm.ptmp.extended_t_ccbs1);
	}
	/* Timeout is T_CCBS1 + 2 seconds. */
	cc_record->fsm.ptmp.extended_t_ccbs1 = pri_schedule_event(ctrl,
		ctrl->timers[PRI_TIMER_T_CCBS1] + 2000, pri_cc_timeout_extended_t_ccbs1,
		cc_record);
}

/*!
 * \internal
 * \brief FSM action to stop the T_SUPERVISION timer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_stop_t_supervision(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	pri_schedule_del(ctrl, cc_record->t_supervision);
	cc_record->t_supervision = 0;
}

/*!
 * \internal
 * \brief T_SUPERVISION timeout callback.
 *
 * \param data CC record pointer.
 *
 * \return Nothing
 */
static void pri_cc_timeout_t_supervision(void *data)
{
	struct pri_cc_record *cc_record = data;

	cc_record->t_supervision = 0;
	q931_cc_timeout(cc_record->ctrl, cc_record, CC_EVENT_TIMEOUT_T_SUPERVISION);
}

/*!
 * \internal
 * \brief FSM action to start the T_SUPERVISION timer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_start_t_supervision(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	int timer_id;
	int duration;

	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	if (cc_record->t_supervision) {
		pri_error(ctrl, "!! A CC supervision timer is already running!");
		pri_schedule_del(ctrl, cc_record->t_supervision);
	}
	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		if (PTMP_MODE(ctrl)) {
			/* ETSI PTMP mode. */
			timer_id = cc_record->is_ccnr ? PRI_TIMER_T_CCNR2 : PRI_TIMER_T_CCBS2;
		} else if (cc_record->is_agent) {
			/* ETSI PTP mode network B side. */
			timer_id = cc_record->is_ccnr ? PRI_TIMER_T_CCNR5 : PRI_TIMER_T_CCBS5;
		} else {
			/* ETSI PTP mode network A side. */
			timer_id = cc_record->is_ccnr ? PRI_TIMER_T_CCNR6 : PRI_TIMER_T_CCBS6;
		}
		duration = ctrl->timers[timer_id];
		break;
	case PRI_SWITCH_QSIG:
		timer_id = cc_record->is_ccnr ? PRI_TIMER_QSIG_CCNR_T2 : PRI_TIMER_QSIG_CCBS_T2;
		duration = ctrl->timers[timer_id];
		break;
	default:
		/* Timer not defined for this switch type.  Should never happen. */
		pri_error(ctrl, "!! A CC supervision timer is not defined!");
		duration = 0;
		break;
	}
	cc_record->t_supervision = pri_schedule_event(ctrl, duration,
		pri_cc_timeout_t_supervision, cc_record);
}

/*!
 * \internal
 * \brief FSM action to stop the T_RECALL timer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_stop_t_recall(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	pri_schedule_del(ctrl, cc_record->t_recall);
	cc_record->t_recall = 0;
}

/*!
 * \internal
 * \brief T_RECALL timeout callback.
 *
 * \param data CC record pointer.
 *
 * \return Nothing
 */
static void pri_cc_timeout_t_recall(void *data)
{
	struct pri_cc_record *cc_record = data;

	cc_record->t_recall = 0;
	q931_cc_timeout(cc_record->ctrl, cc_record, CC_EVENT_TIMEOUT_T_RECALL);
}

/*!
 * \internal
 * \brief FSM action to start the T_RECALL timer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_start_t_recall(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	int duration;

	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	if (cc_record->t_recall) {
		pri_error(ctrl, "!! T_RECALL is already running!");
		pri_schedule_del(ctrl, cc_record->t_recall);
	}
	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		duration = ctrl->timers[PRI_TIMER_T_CCBS3];
		break;
	case PRI_SWITCH_QSIG:
		duration = ctrl->timers[PRI_TIMER_QSIG_CC_T3];
		break;
	default:
		/* Timer not defined for this switch type.  Should never happen. */
		pri_error(ctrl, "!! A CC recall timer is not defined!");
		duration = 0;
		break;
	}
	cc_record->t_recall = pri_schedule_event(ctrl, duration, pri_cc_timeout_t_recall,
		cc_record);
}

/*!
 * \internal
 * \brief FSM action to send the EraseCallLinkageID message.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_send_erase_call_linkage_id(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	send_erase_call_linkage_id(ctrl, cc_record->signaling, cc_record);
}

/*!
 * \internal
 * \brief FSM action to send the CCBSErase message.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 * \param reason CCBS Erase reason
 *  normal-unspecified(0), t-CCBS2-timeout(1), t-CCBS3-timeout(2), basic-call-failed(3)
 *
 * \return Nothing
 */
static void pri_cc_act_send_ccbs_erase(struct pri *ctrl, struct pri_cc_record *cc_record, int reason)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	send_ccbs_erase(ctrl, cc_record->signaling, cc_record, reason);
}

/*!
 * \internal
 * \brief Find the T_CCBS1 timer/CCBSStatusRequest message.
 *
 * \param cc_record Call completion record to process event.
 *
 * \return Facility message pointer or NULL if not active.
 */
static struct apdu_event *pri_cc_get_t_ccbs1_status(struct pri_cc_record *cc_record)
{
	return pri_call_apdu_find(cc_record->signaling,
		cc_record->fsm.ptmp.t_ccbs1_invoke_id);
}

/*!
 * \internal
 * \brief FSM action to stop the PTMP T_CCBS1 timer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_stop_t_ccbs1(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	struct apdu_event *msg;

	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);

	msg = pri_call_apdu_find(cc_record->signaling, cc_record->fsm.ptmp.t_ccbs1_invoke_id);
	if (msg) {
		cc_record->fsm.ptmp.t_ccbs1_invoke_id = APDU_INVALID_INVOKE_ID;
		pri_call_apdu_delete(cc_record->signaling, msg);
	}
}

/*!
 * \internal
 * \brief CCBSStatusRequest response callback function.
 *
 * \param reason Reason callback is called.
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param apdu APDU queued entry.  Do not change!
 * \param msg APDU response message data.  (NULL if was not the reason called.)
 *
 * \return TRUE if no more responses are expected.
 */
static int pri_cc_ccbs_status_response(enum APDU_CALLBACK_REASON reason, struct pri *ctrl, struct q931_call *call, struct apdu_event *apdu, const struct apdu_msg_data *msg)
{
	struct pri_cc_record *cc_record;

	cc_record = apdu->response.user.ptr;
	switch (reason) {
	case APDU_CALLBACK_REASON_ERROR:
		cc_record->fsm.ptmp.t_ccbs1_invoke_id = APDU_INVALID_INVOKE_ID;
		break;
	case APDU_CALLBACK_REASON_TIMEOUT:
		cc_record->fsm.ptmp.t_ccbs1_invoke_id = APDU_INVALID_INVOKE_ID;
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_TIMEOUT_T_CCBS1);
		break;
	case APDU_CALLBACK_REASON_MSG_RESULT:
		pri_cc_event(ctrl, call, cc_record,
			msg->response.result->args.etsi.CCBSStatusRequest.free
			? CC_EVENT_A_FREE : CC_EVENT_A_BUSY);
		break;
	default:
		break;
	}
	return 0;
}

/*!
 * \internal
 * \brief Encode and queue an CCBSStatusRequest message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode CCBSStatusRequest.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_ccbs_status_request(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record)
{
	unsigned char buffer[256];
	unsigned char *end;
	struct apdu_callback_data response;

	end =
		enc_etsi_ptmp_ccbs_status_request(ctrl, buffer, buffer + sizeof(buffer),
			cc_record);
	if (!end) {
		return -1;
	}

	memset(&response, 0, sizeof(response));
	cc_record->fsm.ptmp.t_ccbs1_invoke_id = ctrl->last_invoke;
	response.invoke_id = ctrl->last_invoke;
	response.timeout_time = ctrl->timers[PRI_TIMER_T_CCBS1];
	response.callback = pri_cc_ccbs_status_response;
	response.user.ptr = cc_record;
	return pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, &response);
}

/*!
 * \internal
 * \brief Encode and send an CCBSStatusRequest message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode CCBSStatusRequest.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int send_ccbs_status_request(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record)
{
/*
 * XXX May need to add called-party-ie with Party A number in FACILITY message. (CCBSStatusRequest)
 * ETSI EN 300-195-1 Section 5.41 MSN interaction.
 */
	if (rose_ccbs_status_request(ctrl, call, cc_record)
		|| q931_facility(ctrl, call)) {
		pri_message(ctrl,
			"Could not schedule facility message for CCBSStatusRequest.\n");
		return -1;
	}

	return 0;
}

/*!
 * \internal
 * \brief FSM action to send the CCBSStatusRequest message.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_send_ccbs_status_request(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	send_ccbs_status_request(ctrl, cc_record->signaling, cc_record);
}

/*!
 * \internal
 * \brief FSM action to stop the PTMP T_ACTIVATE timer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_stop_t_activate(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	struct apdu_event *msg;

	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);

	if (!cc_record->signaling) {
		return;
	}
	msg = pri_call_apdu_find(cc_record->signaling, cc_record->t_activate_invoke_id);
	if (msg) {
		cc_record->t_activate_invoke_id = APDU_INVALID_INVOKE_ID;
		pri_call_apdu_delete(cc_record->signaling, msg);
	}
}

/*!
 * \internal
 * \brief cc-request PTMP response callback function.
 *
 * \param reason Reason callback is called.
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param apdu APDU queued entry.  Do not change!
 * \param msg APDU response message data.  (NULL if was not the reason called.)
 *
 * \return TRUE if no more responses are expected.
 */
static int pri_cc_req_response_ptmp(enum APDU_CALLBACK_REASON reason, struct pri *ctrl, struct q931_call *call, struct apdu_event *apdu, const struct apdu_msg_data *msg)
{
	struct pri_cc_record *cc_record;

	cc_record = apdu->response.user.ptr;

	switch (reason) {
	case APDU_CALLBACK_REASON_ERROR:
		cc_record->t_activate_invoke_id = APDU_INVALID_INVOKE_ID;
		break;
	case APDU_CALLBACK_REASON_TIMEOUT:
		cc_record->t_activate_invoke_id = APDU_INVALID_INVOKE_ID;
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_TIMEOUT_T_ACTIVATE);
		break;
	case APDU_CALLBACK_REASON_MSG_RESULT:
		/*
		 * Since we received this facility, we will not be allocating any
		 * reference and linkage id's.
		 */
		cc_record->ccbs_reference_id =
			msg->response.result->args.etsi.CCBSRequest.ccbs_reference & 0x7F;
		cc_record->option.recall_mode =
			msg->response.result->args.etsi.CCBSRequest.recall_mode;

		pri_cc_event(ctrl, call, cc_record, CC_EVENT_CC_REQUEST_ACCEPT);
		break;
	case APDU_CALLBACK_REASON_MSG_ERROR:
		cc_record->msg.cc_req_rsp.reason = reason;
		cc_record->msg.cc_req_rsp.code = msg->response.error->code;

		pri_cc_event(ctrl, call, cc_record, CC_EVENT_CC_REQUEST_FAIL);
		break;
	case APDU_CALLBACK_REASON_MSG_REJECT:
		cc_record->msg.cc_req_rsp.reason = reason;
		cc_record->msg.cc_req_rsp.code = msg->response.reject->code;

		pri_cc_event(ctrl, call, cc_record, CC_EVENT_CC_REQUEST_FAIL);
		break;
	default:
		break;
	}

	/*
	 * No more reponses are really expected.
	 * However, the FSM will be removing the apdu_event itself instead.
	 */
	return 0;
}

/*!
 * \internal
 * \brief cc-request PTP response callback function.
 *
 * \param reason Reason callback is called.
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param apdu APDU queued entry.  Do not change!
 * \param msg APDU response message data.  (NULL if was not the reason called.)
 *
 * \return TRUE if no more responses are expected.
 */
static int pri_cc_req_response_ptp(enum APDU_CALLBACK_REASON reason, struct pri *ctrl, struct q931_call *call, struct apdu_event *apdu, const struct apdu_msg_data *msg)
{
	struct pri_cc_record *cc_record;

	cc_record = apdu->response.user.ptr;

	switch (reason) {
	case APDU_CALLBACK_REASON_ERROR:
		cc_record->t_activate_invoke_id = APDU_INVALID_INVOKE_ID;
		break;
	case APDU_CALLBACK_REASON_TIMEOUT:
		cc_record->t_activate_invoke_id = APDU_INVALID_INVOKE_ID;
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_TIMEOUT_T_ACTIVATE);
		break;
	case APDU_CALLBACK_REASON_MSG_RESULT:
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_CC_REQUEST_ACCEPT);
		break;
	case APDU_CALLBACK_REASON_MSG_ERROR:
		cc_record->msg.cc_req_rsp.reason = reason;
		cc_record->msg.cc_req_rsp.code = msg->response.error->code;

		pri_cc_event(ctrl, call, cc_record, CC_EVENT_CC_REQUEST_FAIL);
		break;
	case APDU_CALLBACK_REASON_MSG_REJECT:
		cc_record->msg.cc_req_rsp.reason = reason;
		cc_record->msg.cc_req_rsp.code = msg->response.reject->code;

		pri_cc_event(ctrl, call, cc_record, CC_EVENT_CC_REQUEST_FAIL);
		break;
	default:
		break;
	}

	/*
	 * No more reponses are really expected.
	 * However, the FSM will be removing the apdu_event itself instead.
	 */
	return 0;
}

/*!
 * \internal
 * \brief cc-request Q.SIG response callback function.
 *
 * \param reason Reason callback is called.
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param apdu APDU queued entry.  Do not change!
 * \param msg APDU response message data.  (NULL if was not the reason called.)
 *
 * \return TRUE if no more responses are expected.
 */
static int pri_cc_req_response_qsig(enum APDU_CALLBACK_REASON reason, struct pri *ctrl, struct q931_call *call, struct apdu_event *apdu, const struct apdu_msg_data *msg)
{
	struct pri_cc_record *cc_record;

	cc_record = apdu->response.user.ptr;

	switch (reason) {
	case APDU_CALLBACK_REASON_ERROR:
		cc_record->t_activate_invoke_id = APDU_INVALID_INVOKE_ID;
		break;
	case APDU_CALLBACK_REASON_TIMEOUT:
		cc_record->t_activate_invoke_id = APDU_INVALID_INVOKE_ID;
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_TIMEOUT_T_ACTIVATE);
		break;
	case APDU_CALLBACK_REASON_MSG_RESULT:
		if (!cc_record->option.retain_signaling_link) {
			/* We were ambivalent about the signaling link retention option. */
			if (msg->type == Q931_CONNECT) {
				/* The far end elected to retain the signaling link. */
				cc_record->option.retain_signaling_link = 1;
			}
		}

		cc_record->fsm.qsig.msgtype = msg->type;
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_CC_REQUEST_ACCEPT);
		break;
	case APDU_CALLBACK_REASON_MSG_ERROR:
		cc_record->msg.cc_req_rsp.reason = reason;
		cc_record->msg.cc_req_rsp.code = msg->response.error->code;

		cc_record->fsm.qsig.msgtype = msg->type;
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_CC_REQUEST_FAIL);
		break;
	case APDU_CALLBACK_REASON_MSG_REJECT:
		cc_record->msg.cc_req_rsp.reason = reason;
		cc_record->msg.cc_req_rsp.code = msg->response.reject->code;

		cc_record->fsm.qsig.msgtype = msg->type;
		pri_cc_event(ctrl, call, cc_record, CC_EVENT_CC_REQUEST_FAIL);
		break;
	default:
		break;
	}

	/*
	 * No more reponses are really expected.
	 * However, the FSM will be removing the apdu_event itself instead.
	 */
	return 0;
}

/*!
 * \internal
 * \brief Encode and queue a cc-request message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode cc-request.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_cc_request(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record)
{
	unsigned char buffer[256];
	unsigned char *end;
	struct apdu_callback_data response;
	int msgtype;

	memset(&response, 0, sizeof(response));

	switch (ctrl->switchtype) {
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		if (PTMP_MODE(ctrl)) {
			end =
				enc_etsi_ptmp_cc_request(ctrl, buffer, buffer + sizeof(buffer),
					cc_record);
			msgtype = Q931_FACILITY;
			response.callback = pri_cc_req_response_ptmp;
		} else {
			end =
				enc_etsi_ptp_cc_request(ctrl, buffer, buffer + sizeof(buffer), cc_record);
			msgtype = Q931_REGISTER;
			response.callback = pri_cc_req_response_ptp;
		}
		response.timeout_time = ctrl->timers[PRI_TIMER_T_ACTIVATE];
		break;
	case PRI_SWITCH_QSIG:
		end = enc_qsig_cc_request(ctrl, buffer, buffer + sizeof(buffer), cc_record);
		msgtype = Q931_SETUP;
		response.callback = pri_cc_req_response_qsig;
		response.timeout_time = ctrl->timers[PRI_TIMER_QSIG_CC_T1];
		break;
	default:
		return -1;
	}
	if (!end) {
		return -1;
	}

	response.user.ptr = cc_record;
	response.invoke_id = ctrl->last_invoke;
	cc_record->t_activate_invoke_id = ctrl->last_invoke;
	return pri_call_apdu_queue(call, msgtype, buffer, end - buffer, &response);
}

/*!
 * \internal
 * \brief FSM action to queue the cc-request message.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 * \param call Call leg from which to encode cc-request.
 *
 * \return Nothing
 */
static void pri_cc_act_queue_cc_request(struct pri *ctrl, struct pri_cc_record *cc_record, q931_call *call)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	if (rose_cc_request(ctrl, call, cc_record)) {
		pri_message(ctrl, "Could not queue message for cc-request.\n");
	}
}

/*!
 * \internal
 * \brief FSM action to send the CCBSDeactivate message.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_send_cc_deactivate_req(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	send_cc_deactivate_req(ctrl, cc_record->signaling, cc_record);
}

/*!
 * \internal
 * \brief FSM action to send the CCBSBFree message.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_send_ccbs_b_free(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	send_ccbs_b_free(ctrl, cc_record->signaling, cc_record);
}

/*!
 * \internal
 * \brief FSM action to send the remote user free message.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_send_remote_user_free(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	send_remote_user_free(ctrl, cc_record);
}

/*!
 * \internal
 * \brief FSM action to send the CALL_PROCEEDING message on the signaling link.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_send_call_proceeding(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	pri_proceeding(ctrl, cc_record->signaling, 0, 0);
}

/*!
 * \internal
 * \brief FSM action to send the CC suspend message.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_send_cc_suspend(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	send_cc_suspend(ctrl, cc_record);
}

/*!
 * \internal
 * \brief FSM action to send the CC resume message.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_send_cc_resume(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	send_cc_resume(ctrl, cc_record);
}

/*!
 * \internal
 * \brief FSM action to send the ccCancel message.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_send_cc_cancel(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	send_cc_cancel(ctrl, cc_record);
}

/*!
 * \internal
 * \brief FSM action to send the CCBSStopAlerting message.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_send_ccbs_stop_alerting(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	send_ccbs_stop_alerting(ctrl, cc_record->signaling, cc_record);
}

/*!
 * \internal
 * \brief FSM action to release the call linkage id.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_release_link_id(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	cc_record->call_linkage_id = CC_PTMP_INVALID_ID;
}

/*!
 * \internal
 * \brief FSM action to set the Q.SIG retain-signaling-link option.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_set_retain_signaling_link(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	cc_record->option.retain_signaling_link = 1;
}

/*!
 * \internal
 * \brief FSM action to reset the raw A status request no response count.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_raw_status_count_reset(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	cc_record->fsm.ptmp.party_a_status_count = 0;
}

/*!
 * \internal
 * \brief FSM action to increment the raw A status request no response count.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_raw_status_count_increment(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	++cc_record->fsm.ptmp.party_a_status_count;
}

/*!
 * \internal
 * \brief FSM action to reset raw A status.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_reset_raw_a_status(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	cc_record->fsm.ptmp.party_a_status_acc = CC_PARTY_A_AVAILABILITY_INVALID;
}

/*!
 * \internal
 * \brief FSM action to add raw A status with busy.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_add_raw_a_status_busy(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	if (cc_record->fsm.ptmp.party_a_status_acc != CC_PARTY_A_AVAILABILITY_FREE) {
		cc_record->fsm.ptmp.party_a_status_acc = CC_PARTY_A_AVAILABILITY_BUSY;
	}
}

/*!
 * \internal
 * \brief FSM action to set raw A status to free.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_set_raw_a_status_free(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	cc_record->fsm.ptmp.party_a_status_acc = CC_PARTY_A_AVAILABILITY_FREE;
}

/*!
 * \internal
 * \brief Fill in the status response party A status update event.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_fill_status_rsp_a(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record)
{
	struct pri_subcommand *subcmd;

	if (cc_record->fsm.ptmp.party_a_status_acc == CC_PARTY_A_AVAILABILITY_INVALID) {
		/* Accumulated party A status is invalid so don't pass it up. */
		return;
	}

	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		return;
	}

	subcmd->cmd = PRI_SUBCMD_CC_STATUS_REQ_RSP;
	subcmd->u.cc_status_req_rsp.cc_id =  cc_record->record_id;
	subcmd->u.cc_status_req_rsp.status =
		(cc_record->fsm.ptmp.party_a_status_acc == CC_PARTY_A_AVAILABILITY_FREE)
		? 0 /* free */ : 1 /* busy */;
}

/*!
 * \internal
 * \brief Pass up party A status response to upper layer (indirectly).
 *
 * \param data CC record pointer.
 *
 * \return Nothing
 */
static void pri_cc_indirect_status_rsp_a(void *data)
{
	struct pri_cc_record *cc_record = data;

	cc_record->t_indirect = 0;
	q931_cc_indirect(cc_record->ctrl, cc_record, pri_cc_fill_status_rsp_a);
}

/*!
 * \internal
 * \brief FSM action to pass up party A status response to upper layer (indirectly).
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 *
 * \note
 * Warning:  Must not use this action with pri_cc_act_set_self_destruct() in the
 * same event.
 */
static void pri_cc_act_pass_up_status_rsp_a_indirect(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	if (cc_record->fsm.ptmp.party_a_status_acc != CC_PARTY_A_AVAILABILITY_INVALID) {
		/* Accumulated party A status is not invalid so pass it up. */
		if (cc_record->t_indirect) {
			pri_error(ctrl, "!! An indirect action is already active!");
			pri_schedule_del(ctrl, cc_record->t_indirect);
		}
		cc_record->t_indirect = pri_schedule_event(ctrl, 0, pri_cc_indirect_status_rsp_a,
			cc_record);
	}
}

/*!
 * \internal
 * \brief FSM action to pass up party A status response to upper layer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_pass_up_status_rsp_a(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	pri_cc_fill_status_rsp_a(ctrl, cc_record->signaling, cc_record);
}

/*!
 * \internal
 * \brief FSM action to promote raw A status.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_promote_raw_a_status(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	cc_record->party_a_status = cc_record->fsm.ptmp.party_a_status_acc;
}

/*!
 * \internal
 * \brief FSM action to reset A status.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_reset_a_status(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	cc_record->party_a_status = CC_PARTY_A_AVAILABILITY_INVALID;
}

/*!
 * \internal
 * \brief FSM action to set A status as busy.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_set_a_status_busy(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	cc_record->party_a_status = CC_PARTY_A_AVAILABILITY_BUSY;
}

/*!
 * \internal
 * \brief FSM action to set A status as free.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_set_a_status_free(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	cc_record->party_a_status = CC_PARTY_A_AVAILABILITY_FREE;
}

/*!
 * \internal
 * \brief Fill in the party A status update event.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_fill_status_a(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record)
{
	struct pri_subcommand *subcmd;

	if (cc_record->party_a_status == CC_PARTY_A_AVAILABILITY_INVALID) {
		/* Party A status is invalid so don't pass it up. */
		return;
	}

	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		return;
	}

	subcmd->cmd = PRI_SUBCMD_CC_STATUS;
	subcmd->u.cc_status.cc_id =  cc_record->record_id;
	subcmd->u.cc_status.status =
		(cc_record->party_a_status == CC_PARTY_A_AVAILABILITY_FREE)
		? 0 /* free */ : 1 /* busy */;
}

/*!
 * \internal
 * \brief Pass up party A status to upper layer (indirectly).
 *
 * \param data CC record pointer.
 *
 * \return Nothing
 */
static void pri_cc_indirect_status_a(void *data)
{
	struct pri_cc_record *cc_record = data;

	cc_record->t_indirect = 0;
	q931_cc_indirect(cc_record->ctrl, cc_record, pri_cc_fill_status_a);
}

/*!
 * \internal
 * \brief FSM action to pass up party A status to upper layer (indirectly).
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 *
 * \note
 * Warning:  Must not use this action with pri_cc_act_set_self_destruct() in the
 * same event.
 */
static void pri_cc_act_pass_up_a_status_indirect(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	if (cc_record->party_a_status != CC_PARTY_A_AVAILABILITY_INVALID) {
		/* Party A status is not invalid so pass it up. */
		if (cc_record->t_indirect) {
			pri_error(ctrl, "!! An indirect action is already active!");
			pri_schedule_del(ctrl, cc_record->t_indirect);
		}
		cc_record->t_indirect = pri_schedule_event(ctrl, 0, pri_cc_indirect_status_a,
			cc_record);
	}
}

/*!
 * \internal
 * \brief FSM action to pass up party A status to upper layer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_pass_up_a_status(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	pri_cc_fill_status_a(ctrl, cc_record->signaling, cc_record);
}

/*!
 * \internal
 * \brief FSM action to pass up CC request (CCBS/CCNR) to upper layer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_pass_up_cc_request(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	struct pri_subcommand *subcmd;

	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);

	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		return;
	}

	subcmd->cmd = PRI_SUBCMD_CC_REQ;
	subcmd->u.cc_request.cc_id =  cc_record->record_id;
	subcmd->u.cc_request.mode = cc_record->is_ccnr ? 1 /* ccnr */ : 0 /* ccbs */;
}

/*!
 * \internal
 * \brief FSM action to pass up CC cancel to upper layer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_pass_up_cc_cancel(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	struct pri_subcommand *subcmd;

	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);

	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		return;
	}

	subcmd->cmd = PRI_SUBCMD_CC_CANCEL;
	subcmd->u.cc_cancel.cc_id =  cc_record->record_id;
	subcmd->u.cc_cancel.is_agent = cc_record->is_agent;
}

/*!
 * \internal
 * \brief FSM action to pass up CC call to upper layer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_pass_up_cc_call(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	struct pri_subcommand *subcmd;

	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);

	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		return;
	}

	subcmd->cmd = PRI_SUBCMD_CC_CALL;
	subcmd->u.cc_call.cc_id = cc_record->record_id;
}

/*!
 * \internal
 * \brief FSM action to pass up CC available to upper layer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_pass_up_cc_available(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	struct pri_subcommand *subcmd;

	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);

	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		return;
	}

	subcmd->cmd = PRI_SUBCMD_CC_AVAILABLE;
	subcmd->u.cc_available.cc_id = cc_record->record_id;
}

/*!
 * \internal
 * \brief FSM action to pass up CC request response is success to upper layer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_pass_up_cc_req_rsp_success(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	struct pri_subcommand *subcmd;

	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);

	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		return;
	}

	subcmd->cmd = PRI_SUBCMD_CC_REQ_RSP;
	subcmd->u.cc_request_rsp.cc_id = cc_record->record_id;
	subcmd->u.cc_request_rsp.status = 0;/* success */
	subcmd->u.cc_request_rsp.fail_code = 0;
}

/*!
 * \internal
 * \brief FSM action to pass up CC request response is failed to upper layer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_pass_up_cc_req_rsp_fail(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	struct pri_subcommand *subcmd;

	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);

	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		return;
	}

	subcmd->cmd = PRI_SUBCMD_CC_REQ_RSP;
	subcmd->u.cc_request_rsp.cc_id = cc_record->record_id;
	subcmd->u.cc_request_rsp.status =
		(cc_record->msg.cc_req_rsp.reason == APDU_CALLBACK_REASON_MSG_ERROR)
		? 2 /* error */ : 3 /* reject */;
	subcmd->u.cc_request_rsp.fail_code = cc_record->msg.cc_req_rsp.code;
}

/*!
 * \internal
 * \brief FSM action to pass up CC request response is timeout to upper layer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_pass_up_cc_req_rsp_timeout(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	struct pri_subcommand *subcmd;

	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);

	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		return;
	}

	subcmd->cmd = PRI_SUBCMD_CC_REQ_RSP;
	subcmd->u.cc_request_rsp.cc_id = cc_record->record_id;
	subcmd->u.cc_request_rsp.status = 1;/* timeout */
	subcmd->u.cc_request_rsp.fail_code = 0;
}

/*!
 * \internal
 * \brief FSM action to pass up CC B free to upper layer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_pass_up_b_free(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	struct pri_subcommand *subcmd;

	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);

	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		return;
	}

	subcmd->cmd = PRI_SUBCMD_CC_B_FREE;
	subcmd->u.cc_b_free.cc_id =  cc_record->record_id;
}

/*!
 * \internal
 * \brief FSM action to pass up CC remote user free to upper layer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_pass_up_remote_user_free(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	struct pri_subcommand *subcmd;

	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);

	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		return;
	}

	subcmd->cmd = PRI_SUBCMD_CC_REMOTE_USER_FREE;
	subcmd->u.cc_remote_user_free.cc_id =  cc_record->record_id;
}

/*!
 * \internal
 * \brief FSM action to pass up stop alerting to upper layer.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_pass_up_stop_alerting(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	struct pri_subcommand *subcmd;

	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);

	subcmd = q931_alloc_subcommand(ctrl);
	if (!subcmd) {
		return;
	}

	subcmd->cmd = PRI_SUBCMD_CC_STOP_ALERTING;
	subcmd->u.cc_stop_alerting.cc_id =  cc_record->record_id;
}

/*!
 * \internal
 * \brief FSM action to send error response to recall attempt.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 * \param code Error code to put in error message response.
 *
 * \return Nothing
 */
static void pri_cc_act_send_error_recall(struct pri *ctrl, struct pri_cc_record *cc_record, enum rose_error_code code)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	rose_error_msg_encode(ctrl, cc_record->response.signaling, Q931_ANY_MESSAGE,
		cc_record->response.invoke_id, code);
}

/*!
 * \internal
 * \brief FSM action to queue CC recall marker.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 * \param call Q.931 call leg.
 *
 * \return Nothing
 */
static void pri_cc_act_queue_setup_recall(struct pri *ctrl, struct pri_cc_record *cc_record, q931_call *call)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	rose_cc_recall_encode(ctrl, call, cc_record);
}

/*!
 * \internal
 * \brief FSM action to request the call be hung up.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 * \param call Q.931 call leg.
 *
 * \return Nothing
 */
static void pri_cc_act_set_call_to_hangup(struct pri *ctrl, struct pri_cc_record *cc_record, q931_call *call)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	call->cc.hangup_call = 1;
}

/*!
 * \internal
 * \brief Post the CC_EVENT_HANGUP_SIGNALING event (timeout action).
 *
 * \param data CC record pointer.
 *
 * \return Nothing
 */
static void pri_cc_post_hangup_signaling(void *data)
{
	struct pri_cc_record *cc_record = data;

	cc_record->t_indirect = 0;
	q931_cc_timeout(cc_record->ctrl, cc_record, CC_EVENT_HANGUP_SIGNALING);
}

/*!
 * \internal
 * \brief FSM action to post the CC_EVENT_HANGUP_SIGNALING event indirectly.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_post_hangup_signaling(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	if (cc_record->t_indirect) {
		pri_error(ctrl, "!! An indirect action is already active!");
		pri_schedule_del(ctrl, cc_record->t_indirect);
	}
	cc_record->t_indirect = pri_schedule_event(ctrl, 0, pri_cc_post_hangup_signaling,
		cc_record);
}

/*!
 * \internal
 * \brief FSM action to immediately hangup the signaling link.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_hangup_signaling_link(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	PRI_CC_ACT_DEBUG_OUTPUT(ctrl, cc_record->record_id);
	pri_hangup(ctrl, cc_record->signaling, -1);
}

/*!
 * \internal
 * \brief FSM action to set original call data into recall call.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 *
 * \return Nothing
 */
static void pri_cc_act_set_original_call_parameters(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record)
{
	call->called = cc_record->party_b;
	call->remote_id = cc_record->party_a;
	call->cc.saved_ie_contents = cc_record->saved_ie_contents;
	call->bc = cc_record->bc;
}

/*!
 * \internal
 * \brief CC FSM PTMP agent CC_STATE_IDLE.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptmp_agent_idle(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_AVAILABLE:
		cc_record->state = CC_STATE_PENDING_AVAILABLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTMP agent CC_STATE_PENDING_AVAILABLE.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptmp_agent_pend_avail(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_MSG_ALERTING:
		pri_cc_act_send_cc_available(ctrl, cc_record, call, Q931_ALERTING);
		cc_record->state = CC_STATE_AVAILABLE;
		break;
	case CC_EVENT_MSG_DISCONNECT:
		pri_cc_act_send_cc_available(ctrl, cc_record, call, Q931_DISCONNECT);
		cc_record->state = CC_STATE_AVAILABLE;
		break;
	case CC_EVENT_INTERNAL_CLEARING:
		pri_cc_act_release_link_id(ctrl, cc_record);
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_release_link_id(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTMP agent CC_STATE_AVAILABLE.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptmp_agent_avail(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_MSG_RELEASE:
	case CC_EVENT_MSG_RELEASE_COMPLETE:
		pri_cc_act_stop_t_retention(ctrl, cc_record);
		pri_cc_act_start_t_retention(ctrl, cc_record);
		break;
	case CC_EVENT_CC_REQUEST:
		pri_cc_act_pass_up_cc_request(ctrl, cc_record);
		pri_cc_act_stop_t_retention(ctrl, cc_record);
		cc_record->state = CC_STATE_REQUESTED;
		break;
	case CC_EVENT_INTERNAL_CLEARING:
		pri_cc_act_stop_t_retention(ctrl, cc_record);
		pri_cc_act_start_t_retention(ctrl, cc_record);
		break;
	case CC_EVENT_TIMEOUT_T_RETENTION:
		pri_cc_act_send_erase_call_linkage_id(ctrl, cc_record);
		pri_cc_act_release_link_id(ctrl, cc_record);
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_retention(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_send_erase_call_linkage_id(ctrl, cc_record);
		pri_cc_act_release_link_id(ctrl, cc_record);
		pri_cc_act_stop_t_retention(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTMP agent CC_STATE_REQUESTED.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptmp_agent_req(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_CC_REQUEST_ACCEPT:
		pri_cc_act_send_erase_call_linkage_id(ctrl, cc_record);
		pri_cc_act_release_link_id(ctrl, cc_record);
		pri_cc_act_start_t_supervision(ctrl, cc_record);
		pri_cc_act_reset_a_status(ctrl, cc_record);
		pri_cc_act_raw_status_count_reset(ctrl, cc_record);
		cc_record->state = CC_STATE_ACTIVATED;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_send_erase_call_linkage_id(ctrl, cc_record);
		pri_cc_act_release_link_id(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTMP agent CC_STATE_ACTIVATED.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptmp_agent_activated(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_RECALL:
		pri_cc_act_send_error_recall(ctrl, cc_record, ROSE_ERROR_CCBS_NotReadyForCall);
		pri_cc_act_set_call_to_hangup(ctrl, cc_record, call);
		break;
	case CC_EVENT_B_FREE:
		pri_cc_act_send_ccbs_b_free(ctrl, cc_record);
		break;
	case CC_EVENT_REMOTE_USER_FREE:
		switch (cc_record->party_a_status) {
		case CC_PARTY_A_AVAILABILITY_INVALID:
			if (!pri_cc_get_t_ccbs1_status(cc_record)) {
				pri_cc_act_reset_raw_a_status(ctrl, cc_record);
				pri_cc_act_send_ccbs_status_request(ctrl, cc_record);
				//pri_cc_act_start_t_ccbs1(ctrl, cc_record);
			}
			cc_record->state = CC_STATE_B_AVAILABLE;
			break;
		case CC_PARTY_A_AVAILABILITY_BUSY:
			pri_cc_act_pass_up_a_status_indirect(ctrl, cc_record);
			pri_cc_act_send_ccbs_b_free(ctrl, cc_record);
			if (!pri_cc_get_t_ccbs1_status(cc_record)) {
				pri_cc_act_reset_raw_a_status(ctrl, cc_record);
				pri_cc_act_send_ccbs_status_request(ctrl, cc_record);
				//pri_cc_act_start_t_ccbs1(ctrl, cc_record);
			}
			cc_record->state = CC_STATE_SUSPENDED;
			break;
		case CC_PARTY_A_AVAILABILITY_FREE:
			//pri_cc_act_pass_up_a_status_indirect(ctrl, cc_record);
			pri_cc_act_send_remote_user_free(ctrl, cc_record);
			pri_cc_act_stop_t_ccbs1(ctrl, cc_record);
			pri_cc_act_stop_extended_t_ccbs1(ctrl, cc_record);
			pri_cc_act_start_t_recall(ctrl, cc_record);
			cc_record->state = CC_STATE_WAIT_CALLBACK;
			break;
		default:
			break;
		}
		break;
	case CC_EVENT_A_STATUS:
		if (pri_cc_get_t_ccbs1_status(cc_record)) {
			pri_cc_act_pass_up_status_rsp_a_indirect(ctrl, cc_record);
		} else {
			pri_cc_act_reset_a_status(ctrl, cc_record);
			pri_cc_act_reset_raw_a_status(ctrl, cc_record);
			pri_cc_act_send_ccbs_status_request(ctrl, cc_record);
			//pri_cc_act_start_t_ccbs1(ctrl, cc_record);
			pri_cc_act_stop_extended_t_ccbs1(ctrl, cc_record);
			pri_cc_act_start_extended_t_ccbs1(ctrl, cc_record);
		}
		break;
	case CC_EVENT_A_FREE:
		pri_cc_act_raw_status_count_reset(ctrl, cc_record);
		pri_cc_act_set_raw_a_status_free(ctrl, cc_record);
		pri_cc_act_promote_raw_a_status(ctrl, cc_record);
		pri_cc_act_pass_up_a_status(ctrl, cc_record);
		pri_cc_act_stop_t_ccbs1(ctrl, cc_record);
		break;
	case CC_EVENT_A_BUSY:
		pri_cc_act_add_raw_a_status_busy(ctrl, cc_record);
		pri_cc_act_pass_up_status_rsp_a(ctrl, cc_record);
		break;
	case CC_EVENT_TIMEOUT_T_CCBS1:
		pri_cc_act_promote_raw_a_status(ctrl, cc_record);
		if (cc_record->party_a_status != CC_PARTY_A_AVAILABILITY_INVALID) {
			/* Only received User A busy. */
			pri_cc_act_raw_status_count_reset(ctrl, cc_record);
		} else {
			/* Did not get any responses. */
			pri_cc_act_raw_status_count_increment(ctrl, cc_record);
			if (cc_record->fsm.ptmp.party_a_status_count >= RAW_STATUS_COUNT_MAX) {
				/* User A no longer present. */
				pri_cc_act_send_ccbs_erase(ctrl, cc_record, 0 /* normal-unspecified */);
				pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
				pri_cc_act_stop_t_ccbs1(ctrl, cc_record);
				pri_cc_act_stop_extended_t_ccbs1(ctrl, cc_record);
				pri_cc_act_stop_t_supervision(ctrl, cc_record);
				pri_cc_act_set_self_destruct(ctrl, cc_record);
				cc_record->state = CC_STATE_IDLE;
			}
		}
		break;
	case CC_EVENT_TIMEOUT_EXTENDED_T_CCBS1:
		pri_cc_act_reset_a_status(ctrl, cc_record);
		pri_cc_act_raw_status_count_reset(ctrl, cc_record);
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_send_ccbs_erase(ctrl, cc_record, 1 /* t-CCBS2-timeout */);
		pri_cc_act_stop_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_extended_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_LINK_CANCEL:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_send_ccbs_erase(ctrl, cc_record, 0 /* normal-unspecified */);
		pri_cc_act_stop_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_extended_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_send_ccbs_erase(ctrl, cc_record, 0 /* normal-unspecified */);
		pri_cc_act_stop_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_extended_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTMP agent CC_STATE_B_AVAILABLE.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptmp_agent_b_avail(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_RECALL:
		pri_cc_act_send_error_recall(ctrl, cc_record, ROSE_ERROR_CCBS_NotReadyForCall);
		pri_cc_act_set_call_to_hangup(ctrl, cc_record, call);
		break;
	case CC_EVENT_A_STATUS:
		pri_cc_act_stop_extended_t_ccbs1(ctrl, cc_record);
		pri_cc_act_start_extended_t_ccbs1(ctrl, cc_record);
		pri_cc_act_pass_up_status_rsp_a_indirect(ctrl, cc_record);
		break;
	case CC_EVENT_A_FREE:
		pri_cc_act_send_remote_user_free(ctrl, cc_record);
		pri_cc_act_set_raw_a_status_free(ctrl, cc_record);
		//pri_cc_act_promote_raw_a_status(ctrl, cc_record);
		//pri_cc_act_pass_up_a_status(ctrl, cc_record);
		if (cc_record->fsm.ptmp.extended_t_ccbs1) {
			pri_cc_act_pass_up_status_rsp_a(ctrl, cc_record);
		}
		pri_cc_act_stop_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_extended_t_ccbs1(ctrl, cc_record);
		pri_cc_act_start_t_recall(ctrl, cc_record);
		cc_record->state = CC_STATE_WAIT_CALLBACK;
		break;
	case CC_EVENT_A_BUSY:
		pri_cc_act_add_raw_a_status_busy(ctrl, cc_record);
		if (cc_record->fsm.ptmp.extended_t_ccbs1) {
			pri_cc_act_pass_up_status_rsp_a(ctrl, cc_record);
		}
		break;
	case CC_EVENT_TIMEOUT_T_CCBS1:
		if (cc_record->fsm.ptmp.party_a_status_acc != CC_PARTY_A_AVAILABILITY_INVALID) {
			/* Only received User A busy. */
			pri_cc_act_raw_status_count_reset(ctrl, cc_record);
			pri_cc_act_send_ccbs_b_free(ctrl, cc_record);
			pri_cc_act_promote_raw_a_status(ctrl, cc_record);
			pri_cc_act_pass_up_a_status(ctrl, cc_record);
			/* Optimization due to flattening. */
			//if (!pri_cc_get_t_ccbs1_status(cc_record))
			{
				pri_cc_act_reset_raw_a_status(ctrl, cc_record);
				pri_cc_act_send_ccbs_status_request(ctrl, cc_record);
				//pri_cc_act_start_t_ccbs1(ctrl, cc_record);
			}
			cc_record->state = CC_STATE_SUSPENDED;
		} else {
			/* Did not get any responses. */
			pri_cc_act_raw_status_count_increment(ctrl, cc_record);
			if (cc_record->fsm.ptmp.party_a_status_count >= RAW_STATUS_COUNT_MAX) {
				/* User A no longer present. */
				pri_cc_act_send_ccbs_erase(ctrl, cc_record, 0 /* normal-unspecified */);
				pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
				pri_cc_act_stop_t_ccbs1(ctrl, cc_record);
				pri_cc_act_stop_extended_t_ccbs1(ctrl, cc_record);
				pri_cc_act_stop_t_supervision(ctrl, cc_record);
				pri_cc_act_set_self_destruct(ctrl, cc_record);
				cc_record->state = CC_STATE_IDLE;
				break;
			}
			//pri_cc_act_reset_raw_a_status(ctrl, cc_record);
			pri_cc_act_send_ccbs_status_request(ctrl, cc_record);
			//pri_cc_act_start_t_ccbs1(ctrl, cc_record);
		}
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_send_ccbs_erase(ctrl, cc_record, 1 /* t-CCBS2-timeout */);
		pri_cc_act_stop_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_extended_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_LINK_CANCEL:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_send_ccbs_erase(ctrl, cc_record, 0 /* normal-unspecified */);
		pri_cc_act_stop_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_extended_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_send_ccbs_erase(ctrl, cc_record, 0 /* normal-unspecified */);
		pri_cc_act_stop_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_extended_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTMP agent CC_STATE_SUSPENDED.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptmp_agent_suspended(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_RECALL:
		pri_cc_act_send_error_recall(ctrl, cc_record, ROSE_ERROR_CCBS_NotReadyForCall);
		pri_cc_act_set_call_to_hangup(ctrl, cc_record, call);
		break;
	case CC_EVENT_A_STATUS:
		pri_cc_act_stop_extended_t_ccbs1(ctrl, cc_record);
		pri_cc_act_start_extended_t_ccbs1(ctrl, cc_record);
		pri_cc_act_pass_up_status_rsp_a_indirect(ctrl, cc_record);
		break;
	case CC_EVENT_A_FREE:
		pri_cc_act_set_raw_a_status_free(ctrl, cc_record);
		pri_cc_act_promote_raw_a_status(ctrl, cc_record);
		pri_cc_act_pass_up_a_status(ctrl, cc_record);
		if (cc_record->fsm.ptmp.extended_t_ccbs1) {
			pri_cc_act_pass_up_status_rsp_a(ctrl, cc_record);
		}
		pri_cc_act_stop_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_extended_t_ccbs1(ctrl, cc_record);
		pri_cc_act_reset_a_status(ctrl, cc_record);
		pri_cc_act_raw_status_count_reset(ctrl, cc_record);
		cc_record->state = CC_STATE_ACTIVATED;
		break;
	case CC_EVENT_A_BUSY:
		pri_cc_act_add_raw_a_status_busy(ctrl, cc_record);
		if (cc_record->fsm.ptmp.extended_t_ccbs1) {
			pri_cc_act_pass_up_status_rsp_a(ctrl, cc_record);
		}
		break;
	case CC_EVENT_TIMEOUT_T_CCBS1:
		if (cc_record->fsm.ptmp.party_a_status_acc != CC_PARTY_A_AVAILABILITY_INVALID) {
			/* Only received User A busy. */
			pri_cc_act_raw_status_count_reset(ctrl, cc_record);
		} else {
			/* Did not get any responses. */
			pri_cc_act_raw_status_count_increment(ctrl, cc_record);
			if (cc_record->fsm.ptmp.party_a_status_count >= RAW_STATUS_COUNT_MAX) {
				/* User A no longer present. */
				pri_cc_act_send_ccbs_erase(ctrl, cc_record, 0 /* normal-unspecified */);
				pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
				pri_cc_act_stop_t_ccbs1(ctrl, cc_record);
				pri_cc_act_stop_extended_t_ccbs1(ctrl, cc_record);
				pri_cc_act_stop_t_supervision(ctrl, cc_record);
				pri_cc_act_set_self_destruct(ctrl, cc_record);
				cc_record->state = CC_STATE_IDLE;
				break;
			}
		}
		pri_cc_act_reset_raw_a_status(ctrl, cc_record);
		pri_cc_act_send_ccbs_status_request(ctrl, cc_record);
		//pri_cc_act_start_t_ccbs1(ctrl, cc_record);
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_send_ccbs_erase(ctrl, cc_record, 1 /* t-CCBS2-timeout */);
		pri_cc_act_stop_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_extended_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_LINK_CANCEL:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_send_ccbs_erase(ctrl, cc_record, 0 /* normal-unspecified */);
		pri_cc_act_stop_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_extended_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_send_ccbs_erase(ctrl, cc_record, 0 /* normal-unspecified */);
		pri_cc_act_stop_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_extended_t_ccbs1(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTMP agent CC_STATE_WAIT_CALLBACK.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptmp_agent_wait_callback(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_TIMEOUT_T_RECALL:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_send_ccbs_erase(ctrl, cc_record, 2 /* t-CCBS3-timeout */);
		pri_cc_act_stop_t_recall(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_STOP_ALERTING:
		/*
		 * If an earlier link can send us this event then we
		 * really should be configured for globalRecall like
		 * the earlier link.
		 */
		if (cc_record->option.recall_mode == 0 /* globalRecall */) {
			pri_cc_act_send_ccbs_stop_alerting(ctrl, cc_record);
		}
		pri_cc_act_stop_t_recall(ctrl, cc_record);
		pri_cc_act_reset_a_status(ctrl, cc_record);
		pri_cc_act_raw_status_count_reset(ctrl, cc_record);
		cc_record->state = CC_STATE_ACTIVATED;
		break;
	case CC_EVENT_RECALL:
		pri_cc_act_pass_up_cc_call(ctrl, cc_record);
		pri_cc_act_set_original_call_parameters(ctrl, call, cc_record);
		if (cc_record->option.recall_mode == 0 /* globalRecall */) {
			pri_cc_act_send_ccbs_stop_alerting(ctrl, cc_record);
		}
		pri_cc_act_stop_t_recall(ctrl, cc_record);
		cc_record->state = CC_STATE_CALLBACK;
		break;
	case CC_EVENT_A_STATUS:
		pri_cc_act_set_raw_a_status_free(ctrl, cc_record);
		pri_cc_act_pass_up_status_rsp_a_indirect(ctrl, cc_record);
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_send_ccbs_erase(ctrl, cc_record, 1 /* t-CCBS2-timeout */);
		pri_cc_act_stop_t_recall(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_LINK_CANCEL:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_send_ccbs_erase(ctrl, cc_record, 0 /* normal-unspecified */);
		pri_cc_act_stop_t_recall(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_send_ccbs_erase(ctrl, cc_record, 0 /* normal-unspecified */);
		pri_cc_act_stop_t_recall(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTMP agent CC_STATE_CALLBACK.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptmp_agent_callback(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_RECALL:
		pri_cc_act_send_error_recall(ctrl, cc_record, ROSE_ERROR_CCBS_AlreadyAccepted);
		pri_cc_act_set_call_to_hangup(ctrl, cc_record, call);
		break;
	case CC_EVENT_A_STATUS:
		pri_cc_act_set_raw_a_status_free(ctrl, cc_record);
		pri_cc_act_pass_up_status_rsp_a_indirect(ctrl, cc_record);
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_send_ccbs_erase(ctrl, cc_record, 1 /* t-CCBS2-timeout */);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_LINK_CANCEL:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_send_ccbs_erase(ctrl, cc_record, 0 /* normal-unspecified */);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_send_ccbs_erase(ctrl, cc_record, 0 /* normal-unspecified */);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTMP monitor CC_STATE_IDLE.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptmp_monitor_idle(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_AVAILABLE:
		/*
		 * Before event is posted:
		 *   Received CallInfoRetain
		 *   Created cc_record
		 *   Saved CallLinkageID
		 */
		pri_cc_act_pass_up_cc_available(ctrl, cc_record);
		cc_record->state = CC_STATE_AVAILABLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTMP monitor CC_STATE_AVAILABLE.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptmp_monitor_avail(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	/*
	 * The upper layer is responsible for canceling the CC available
	 * offering as a safeguard in case the network cable is disconnected.
	 * The timer should be set much longer than the network T_RETENTION
	 * timer so normally the CC records will be cleaned up by network
	 * activity.
	 */
	switch (event) {
	case CC_EVENT_CC_REQUEST:
		/* cc_record->is_ccnr is set before event posted. */
		pri_cc_act_queue_cc_request(ctrl, cc_record, call);
		//pri_cc_act_start_t_activate(ctrl, cc_record);
		cc_record->state = CC_STATE_REQUESTED;
		break;
	case CC_EVENT_TIMEOUT_T_RETENTION:
		/*
		 * Received EraseCallLinkageID
		 * T_RETENTION expired on the network side so we will pretend
		 * that it expired on our side.
		 */
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTMP monitor CC_STATE_REQUESTED.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptmp_monitor_req(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_CC_REQUEST_ACCEPT:
		/*
		 * Before event is posted:
		 *   Received CCBSRequest/CCNRRequest response
		 *   Saved CCBSReference
		 */
		pri_cc_act_release_link_id(ctrl, cc_record);
		pri_cc_act_pass_up_cc_req_rsp_success(ctrl, cc_record);
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		/*
		 * Start T_CCBS2 or T_CCNR2 depending upon CC mode.
		 * For PTMP TE mode these timers are not defined.  However,
		 * we will use them anyway to protect our resources from leaks
		 * caused by the network cable being disconnected.  These
		 * timers should be set much longer than the network
		 * so normally the CC records will be cleaned up by network
		 * activity.
		 */
		pri_cc_act_start_t_supervision(ctrl, cc_record);
		cc_record->state = CC_STATE_ACTIVATED;
		break;
	case CC_EVENT_CC_REQUEST_FAIL:
		pri_cc_act_pass_up_cc_req_rsp_fail(ctrl, cc_record);
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_TIMEOUT_T_ACTIVATE:
		pri_cc_act_pass_up_cc_req_rsp_timeout(ctrl, cc_record);
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_LINK_CANCEL:
		/* Received CCBSErase */
		/* Claim it was a timeout */
		pri_cc_act_pass_up_cc_req_rsp_timeout(ctrl, cc_record);
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		cc_record->state = CC_STATE_WAIT_DESTRUCTION;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTMP monitor CC_STATE_WAIT_DESTRUCTION.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptmp_monitor_wait_destruction(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	/* We were in the middle of a cc-request when we were asked to cancel. */
	switch (event) {
	case CC_EVENT_CC_REQUEST_ACCEPT:
		/*
		 * Before event is posted:
		 *   Received CCBSRequest/CCNRRequest response
		 *   Saved CCBSReference
		 */
		pri_cc_act_send_cc_deactivate_req(ctrl, cc_record);
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CC_REQUEST_FAIL:
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_TIMEOUT_T_ACTIVATE:
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_LINK_CANCEL:
		/* Received CCBSErase */
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTMP monitor CC_STATE_ACTIVATED.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptmp_monitor_activated(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_B_FREE:
		/* Received CCBSBFree */
		pri_cc_act_pass_up_b_free(ctrl, cc_record);
		break;
	case CC_EVENT_REMOTE_USER_FREE:
		/* Received CCBSRemoteUserFree */
		pri_cc_act_pass_up_remote_user_free(ctrl, cc_record);
		cc_record->state = CC_STATE_WAIT_CALLBACK;
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_send_cc_deactivate_req(ctrl, cc_record);
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_LINK_CANCEL:
		/* Received CCBSErase */
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_send_cc_deactivate_req(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTMP monitor CC_STATE_WAIT_CALLBACK.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptmp_monitor_wait_callback(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_STOP_ALERTING:
		pri_cc_act_pass_up_stop_alerting(ctrl, cc_record);
		cc_record->state = CC_STATE_ACTIVATED;
		break;
	case CC_EVENT_RECALL:
		/* The original call parameters have already been set. */
		pri_cc_act_queue_setup_recall(ctrl, cc_record, call);
		cc_record->state = CC_STATE_CALLBACK;
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_send_cc_deactivate_req(ctrl, cc_record);
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_LINK_CANCEL:
		/* Received CCBSErase */
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_send_cc_deactivate_req(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTMP monitor CC_STATE_CALLBACK.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptmp_monitor_callback(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	/*
	 * We are waiting for the CC records to be torn down because
	 * CC is complete.
	 * This state is mainly to block CC_EVENT_STOP_ALERTING since
	 * we are the one doing the CC recall so we do not need to stop
	 * alerting.
	 */
	switch (event) {
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_send_cc_deactivate_req(ctrl, cc_record);
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_LINK_CANCEL:
		/* Received CCBSErase */
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_send_cc_deactivate_req(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTP agent CC_STATE_IDLE.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptp_agent_idle(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_AVAILABLE:
		cc_record->state = CC_STATE_PENDING_AVAILABLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTP agent CC_STATE_PENDING_AVAILABLE.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptp_agent_pend_avail(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_MSG_ALERTING:
		pri_cc_act_send_cc_available(ctrl, cc_record, call, Q931_ALERTING);
		cc_record->state = CC_STATE_AVAILABLE;
		break;
	case CC_EVENT_MSG_DISCONNECT:
		pri_cc_act_send_cc_available(ctrl, cc_record, call, Q931_DISCONNECT);
		cc_record->state = CC_STATE_AVAILABLE;
		break;
	case CC_EVENT_INTERNAL_CLEARING:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTP agent CC_STATE_AVAILABLE.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptp_agent_avail(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	/*
	 * For PTP mode the T_RETENTION timer is not defined.  However,
	 * we will use it anyway in this state to protect our resources
	 * from leaks caused by user A not requesting CC.  This timer
	 * should be set much longer than the PTMP network link to
	 * allow for variations in user A's CC offer timer.
	 */
	switch (event) {
	case CC_EVENT_MSG_RELEASE:
	case CC_EVENT_MSG_RELEASE_COMPLETE:
		pri_cc_act_stop_t_retention(ctrl, cc_record);
		pri_cc_act_start_t_retention(ctrl, cc_record);
		break;
	case CC_EVENT_CC_REQUEST:
		pri_cc_act_pass_up_cc_request(ctrl, cc_record);
		pri_cc_act_stop_t_retention(ctrl, cc_record);
		cc_record->state = CC_STATE_REQUESTED;
		break;
	case CC_EVENT_INTERNAL_CLEARING:
		pri_cc_act_stop_t_retention(ctrl, cc_record);
		pri_cc_act_start_t_retention(ctrl, cc_record);
		break;
	case CC_EVENT_TIMEOUT_T_RETENTION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_retention(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_stop_t_retention(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTP agent CC_STATE_REQUESTED.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptp_agent_req(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_CC_REQUEST_ACCEPT:
		/* Start T_CCBS5/T_CCNR5 depending upon CC mode. */
		pri_cc_act_start_t_supervision(ctrl, cc_record);
		pri_cc_act_reset_a_status(ctrl, cc_record);
		cc_record->state = CC_STATE_ACTIVATED;
		break;
	case CC_EVENT_SIGNALING_GONE:
		/* Signaling link cleared. */
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTP agent CC_STATE_ACTIVATED.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptp_agent_activated(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_REMOTE_USER_FREE:
		pri_cc_act_pass_up_a_status_indirect(ctrl, cc_record);
		if (cc_record->party_a_status == CC_PARTY_A_AVAILABILITY_BUSY) {
			cc_record->state = CC_STATE_SUSPENDED;
		} else {
			pri_cc_act_send_remote_user_free(ctrl, cc_record);
			cc_record->state = CC_STATE_WAIT_CALLBACK;
		}
		break;
	case CC_EVENT_SUSPEND:
		/* Received CCBS_T_Suspend */
		pri_cc_act_set_a_status_busy(ctrl, cc_record);
		break;
	case CC_EVENT_RESUME:
		/* Received CCBS_T_Resume */
		pri_cc_act_reset_a_status(ctrl, cc_record);
		break;
	case CC_EVENT_RECALL:
		/* Received CCBS_T_Call */
		pri_cc_act_pass_up_cc_call(ctrl, cc_record);
		pri_cc_act_set_original_call_parameters(ctrl, call, cc_record);
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_SIGNALING_GONE:
		/* Signaling link cleared. */
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTP agent CC_STATE_WAIT_CALLBACK.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptp_agent_wait_callback(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_SUSPEND:
		/* Received CCBS_T_Suspend */
		pri_cc_act_set_a_status_busy(ctrl, cc_record);
		pri_cc_act_pass_up_a_status(ctrl, cc_record);
		cc_record->state = CC_STATE_SUSPENDED;
		break;
	case CC_EVENT_RECALL:
		/* Received CCBS_T_Call */
		pri_cc_act_pass_up_cc_call(ctrl, cc_record);
		pri_cc_act_set_original_call_parameters(ctrl, call, cc_record);
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_SIGNALING_GONE:
		/* Signaling link cleared. */
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTP agent CC_STATE_SUSPENDED.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptp_agent_suspended(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_RESUME:
		/* Received CCBS_T_Resume */
		pri_cc_act_set_a_status_free(ctrl, cc_record);
		pri_cc_act_pass_up_a_status(ctrl, cc_record);
		pri_cc_act_reset_a_status(ctrl, cc_record);
		cc_record->state = CC_STATE_ACTIVATED;
		break;
	case CC_EVENT_RECALL:
		/* Received CCBS_T_Call */
		pri_cc_act_pass_up_cc_call(ctrl, cc_record);
		pri_cc_act_set_original_call_parameters(ctrl, call, cc_record);
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_SIGNALING_GONE:
		/* Signaling link cleared. */
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTP monitor CC_STATE_IDLE.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptp_monitor_idle(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_AVAILABLE:
		/* Received CCBS-T-Aailable */
		pri_cc_act_pass_up_cc_available(ctrl, cc_record);
		cc_record->state = CC_STATE_AVAILABLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTP monitor CC_STATE_AVAILABLE.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptp_monitor_avail(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	/* The upper layer is responsible for canceling the CC available offering. */
	switch (event) {
	case CC_EVENT_CC_REQUEST:
		/*
		 * Before event is posted:
		 *   cc_record->is_ccnr is set.
		 *   The signaling connection call record is created.
		 */
		pri_cc_act_queue_cc_request(ctrl, cc_record, call);
		/*
		 * For PTP mode the T_ACTIVATE timer is not defined.  However,
		 * we will use it to protect our resources from leaks caused
		 * by the network cable being disconnected.
		 * This timer should be set longer than normal so the
		 * CC records will normally be cleaned up by network activity.
		 */
		//pri_cc_act_start_t_activate(ctrl, cc_record);
		cc_record->state = CC_STATE_REQUESTED;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTP monitor CC_STATE_REQUESTED.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptp_monitor_req(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_CC_REQUEST_ACCEPT:
		/*
		 * Received CCBS-T-Request/CCNR-T-Request response
		 * Before event is posted:
		 *   Negotiated CC retention setting saved
		 */
		pri_cc_act_pass_up_cc_req_rsp_success(ctrl, cc_record);
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		/* Start T_CCBS6/T_CCNR6 depending upon CC mode. */
		pri_cc_act_start_t_supervision(ctrl, cc_record);
		pri_cc_act_reset_a_status(ctrl, cc_record);
		cc_record->state = CC_STATE_ACTIVATED;
		break;
	case CC_EVENT_CC_REQUEST_FAIL:
		pri_cc_act_pass_up_cc_req_rsp_fail(ctrl, cc_record);
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		/*
		 * If this request fail comes in with the RELEASE_COMPLETE
		 * message then the post action will never get a chance to
		 * run.  It will be aborted because the CC_EVENT_SIGNALING_GONE
		 * will be processed first.
		 */
		pri_cc_act_post_hangup_signaling(ctrl, cc_record);
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		cc_record->state = CC_STATE_WAIT_DESTRUCTION;
		break;
	case CC_EVENT_TIMEOUT_T_ACTIVATE:
		pri_cc_act_pass_up_cc_req_rsp_timeout(ctrl, cc_record);
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_SIGNALING_GONE:
		/* Claim it was a timeout */
		pri_cc_act_pass_up_cc_req_rsp_timeout(ctrl, cc_record);
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTP monitor CC_STATE_WAIT_DESTRUCTION.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptp_monitor_wait_destruction(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	/*
	 * Delayed disconnect of the signaling link to allow subcmd events
	 * from the signaling link to be passed up.
	 */
	switch (event) {
	case CC_EVENT_SIGNALING_GONE:
		/* Signaling link cleared. */
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_HANGUP_SIGNALING:
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTP monitor CC_STATE_ACTIVATED.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptp_monitor_activated(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_REMOTE_USER_FREE:
		/* Received CCBS_T_RemoteUserFree */
		pri_cc_act_pass_up_remote_user_free(ctrl, cc_record);
		if (cc_record->party_a_status == CC_PARTY_A_AVAILABILITY_BUSY) {
			pri_cc_act_send_cc_suspend(ctrl, cc_record);
			cc_record->state = CC_STATE_SUSPENDED;
		} else {
			cc_record->state = CC_STATE_WAIT_CALLBACK;
		}
		break;
	case CC_EVENT_SUSPEND:
		pri_cc_act_set_a_status_busy(ctrl, cc_record);
		break;
	case CC_EVENT_RESUME:
		pri_cc_act_reset_a_status(ctrl, cc_record);
		break;
	case CC_EVENT_RECALL:
		/* The original call parameters have already been set. */
		pri_cc_act_queue_setup_recall(ctrl, cc_record, call);
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_SIGNALING_GONE:
		/* Signaling link cleared. */
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTP monitor CC_STATE_WAIT_CALLBACK.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptp_monitor_wait_callback(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_SUSPEND:
		pri_cc_act_send_cc_suspend(ctrl, cc_record);
		cc_record->state = CC_STATE_SUSPENDED;
		break;
	case CC_EVENT_RECALL:
		/* The original call parameters have already been set. */
		pri_cc_act_queue_setup_recall(ctrl, cc_record, call);
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_SIGNALING_GONE:
		/* Signaling link cleared. */
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM PTP monitor CC_STATE_SUSPENDED.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_ptp_monitor_suspended(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_RESUME:
		pri_cc_act_send_cc_resume(ctrl, cc_record);
		pri_cc_act_reset_a_status(ctrl, cc_record);
		cc_record->state = CC_STATE_ACTIVATED;
		break;
	case CC_EVENT_RECALL:
		/* The original call parameters have already been set. */
		pri_cc_act_queue_setup_recall(ctrl, cc_record, call);
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_SIGNALING_GONE:
		/* Signaling link cleared. */
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM Q.SIG agent CC_STATE_IDLE.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_qsig_agent_idle(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_AVAILABLE:
		cc_record->state = CC_STATE_AVAILABLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM Q.SIG agent CC_STATE_AVAILABLE.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_qsig_agent_avail(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	/*
	 * For Q.SIG mode the T_RETENTION timer is not defined.  However,
	 * we will use it anyway in this state to protect our resources
	 * from leaks caused by user A not requesting CC.  This timer
	 * should be set much longer than the PTMP network link to
	 * allow for variations in user A's CC offer timer.
	 */
	switch (event) {
	case CC_EVENT_MSG_RELEASE:
	case CC_EVENT_MSG_RELEASE_COMPLETE:
		pri_cc_act_stop_t_retention(ctrl, cc_record);
		pri_cc_act_start_t_retention(ctrl, cc_record);
		break;
	case CC_EVENT_CC_REQUEST:
		pri_cc_act_pass_up_cc_request(ctrl, cc_record);
		/* Send Q931_CALL_PROCEEDING message on signaling link. */
		pri_cc_act_send_call_proceeding(ctrl, cc_record);
		pri_cc_act_stop_t_retention(ctrl, cc_record);
		cc_record->state = CC_STATE_REQUESTED;
		break;
	case CC_EVENT_INTERNAL_CLEARING:
		pri_cc_act_stop_t_retention(ctrl, cc_record);
		pri_cc_act_start_t_retention(ctrl, cc_record);
		break;
	case CC_EVENT_TIMEOUT_T_RETENTION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_retention(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_stop_t_retention(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM Q.SIG agent CC_STATE_REQUESTED.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_qsig_agent_req(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_CC_REQUEST_ACCEPT:
		/* Start QSIG_CCBS_T2/QSIG_CCNR_T2 depending upon CC mode. */
		pri_cc_act_start_t_supervision(ctrl, cc_record);
		cc_record->state = CC_STATE_ACTIVATED;
		break;
	case CC_EVENT_SIGNALING_GONE:
		/* Signaling link cleared. */
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM Q.SIG agent CC_STATE_WAIT_DESTRUCTION.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_qsig_agent_wait_destruction(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	/*
	 * Delayed disconnect of the signaling link to allow subcmd events
	 * from the signaling link to be passed up.
	 */
	switch (event) {
	case CC_EVENT_SIGNALING_GONE:
		/* Signaling link cleared. */
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_HANGUP_SIGNALING:
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM Q.SIG agent CC_STATE_ACTIVATED.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_qsig_agent_activated(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_REMOTE_USER_FREE:
		/* Send ccExecPossible in FACILITY or SETUP. */
		pri_cc_act_send_remote_user_free(ctrl, cc_record);
		cc_record->state = CC_STATE_WAIT_CALLBACK;
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_send_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_SIGNALING_GONE:
		/* Signaling link cleared. */
		pri_cc_act_disassociate_signaling_link(ctrl, cc_record);
		break;
	case CC_EVENT_LINK_CANCEL:
		/* Received ccCancel */
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_post_hangup_signaling(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		cc_record->state = CC_STATE_WAIT_DESTRUCTION;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_send_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM Q.SIG agent CC_STATE_WAIT_CALLBACK.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_qsig_agent_wait_callback(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_SUSPEND:
		/* Received ccSuspend */
		pri_cc_act_set_a_status_busy(ctrl, cc_record);
		pri_cc_act_pass_up_a_status(ctrl, cc_record);
		cc_record->state = CC_STATE_SUSPENDED;
		break;
	case CC_EVENT_RECALL:
		/* Received ccRingout */
		pri_cc_act_pass_up_cc_call(ctrl, cc_record);
		pri_cc_act_set_original_call_parameters(ctrl, call, cc_record);
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_send_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_SIGNALING_GONE:
		/* Signaling link cleared. */
		pri_cc_act_disassociate_signaling_link(ctrl, cc_record);
		break;
	case CC_EVENT_LINK_CANCEL:
		/* Received ccCancel */
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_post_hangup_signaling(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		cc_record->state = CC_STATE_WAIT_DESTRUCTION;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_send_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM Q.SIG agent CC_STATE_SUSPENDED.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_qsig_agent_suspended(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_RESUME:
		/* Received ccResume */
		pri_cc_act_set_a_status_free(ctrl, cc_record);
		pri_cc_act_pass_up_a_status(ctrl, cc_record);
		cc_record->state = CC_STATE_ACTIVATED;
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_send_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_SIGNALING_GONE:
		/* Signaling link cleared. */
		pri_cc_act_disassociate_signaling_link(ctrl, cc_record);
		break;
	case CC_EVENT_LINK_CANCEL:
		/* Received ccCancel */
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_post_hangup_signaling(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		cc_record->state = CC_STATE_WAIT_DESTRUCTION;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_send_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM Q.SIG monitor CC_STATE_IDLE.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_qsig_monitor_idle(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_AVAILABLE:
		/*
		 * LibPRI will determine if CC will be offered based upon
		 * if it is even possible.
		 * Essentially:
		 * 1) The call must not have been redirected in this link's
		 * setup.
		 * 2) Received an ALERTING or received a
		 * DISCONNECT(busy/congestion).
		 */
		pri_cc_act_pass_up_cc_available(ctrl, cc_record);
		cc_record->state = CC_STATE_AVAILABLE;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM Q.SIG monitor CC_STATE_AVAILABLE.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_qsig_monitor_avail(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	/* The upper layer is responsible for canceling the CC available offering. */
	switch (event) {
	case CC_EVENT_CC_REQUEST:
		/*
		 * Before event is posted:
		 *   cc_record->is_ccnr is set.
		 *   The signaling connection call record is created.
		 */
		pri_cc_act_queue_cc_request(ctrl, cc_record, call);
		/* Start QSIG_CC_T1. */
		//pri_cc_act_start_t_activate(ctrl, cc_record);
		cc_record->state = CC_STATE_REQUESTED;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM Q.SIG monitor CC_STATE_REQUESTED.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_qsig_monitor_req(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_CC_REQUEST_ACCEPT:
		/*
		 * Received ccbsRequest/ccnrRequest response
		 * Before event is posted:
		 *   Negotiated CC retention setting saved
		 *   Negotiated signaling link retention setting saved
		 */
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		if (cc_record->fsm.qsig.msgtype == Q931_RELEASE) {
			pri_cc_act_disassociate_signaling_link(ctrl, cc_record);
			if (cc_record->option.retain_signaling_link) {
				/*
				 * The far end did not honor the
				 * signaling link retention requirement.
				 * ECMA-186 Section 6.5.2.2.1
				 */
				pri_cc_act_pass_up_cc_req_rsp_timeout(ctrl, cc_record);
				pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
				pri_cc_act_send_cc_cancel(ctrl, cc_record);
				pri_cc_act_set_self_destruct(ctrl, cc_record);
				cc_record->state = CC_STATE_IDLE;
				break;
			}
		}
		pri_cc_act_pass_up_cc_req_rsp_success(ctrl, cc_record);
		/* Start QSIG_CCBS_T2/QSIG_CCNR_T2 depending upon CC mode. */
		pri_cc_act_start_t_supervision(ctrl, cc_record);
		pri_cc_act_reset_a_status(ctrl, cc_record);
		cc_record->state = CC_STATE_ACTIVATED;
		break;
	case CC_EVENT_CC_REQUEST_FAIL:
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		pri_cc_act_pass_up_cc_req_rsp_fail(ctrl, cc_record);
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		/*
		 * If this request fail comes in with the RELEASE message
		 * then the post action will never get a chance to run.
		 * It will be aborted because the CC_EVENT_SIGNALING_GONE
		 * will be processed first.
		 */
		pri_cc_act_post_hangup_signaling(ctrl, cc_record);
		cc_record->state = CC_STATE_WAIT_DESTRUCTION;
		break;
	case CC_EVENT_TIMEOUT_T_ACTIVATE:
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		pri_cc_act_pass_up_cc_req_rsp_timeout(ctrl, cc_record);
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_SIGNALING_GONE:
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		/* Claim it was a timeout */
		pri_cc_act_pass_up_cc_req_rsp_timeout(ctrl, cc_record);
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CANCEL:
		cc_record->state = CC_STATE_WAIT_DESTRUCTION;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM Q.SIG monitor CC_STATE_WAIT_DESTRUCTION.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_qsig_monitor_wait_destruction(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	/*
	 * Delayed disconnect of the signaling link to allow subcmd events
	 * from the signaling link to be passed up.
	 */
	switch (event) {
	case CC_EVENT_CC_REQUEST_ACCEPT:
		/*
		 * Received ccbsRequest/ccnrRequest response
		 * Before event is posted:
		 *   Negotiated CC retention setting saved
		 *   Negotiated signaling link retention setting saved
		 */
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		if (cc_record->fsm.qsig.msgtype == Q931_RELEASE) {
			pri_cc_act_disassociate_signaling_link(ctrl, cc_record);
		}
		pri_cc_act_send_cc_cancel(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_CC_REQUEST_FAIL:
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		/*
		 * If this request fail comes in with the RELEASE message
		 * then the post action will never get a chance to run.
		 * It will be aborted because the CC_EVENT_SIGNALING_GONE
		 * will be processed first.
		 */
		pri_cc_act_post_hangup_signaling(ctrl, cc_record);
		break;
	case CC_EVENT_TIMEOUT_T_ACTIVATE:
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_SIGNALING_GONE:
		/* Signaling link cleared. */
		pri_cc_act_stop_t_activate(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_HANGUP_SIGNALING:
		//pri_cc_act_stop_t_activate(ctrl, cc_record);
		pri_cc_act_hangup_signaling_link(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM Q.SIG monitor CC_STATE_ACTIVATED.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_qsig_monitor_activated(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_REMOTE_USER_FREE:
		/* Received ccExecPossible */
		pri_cc_act_pass_up_remote_user_free(ctrl, cc_record);
		/*
		 * ECMA-186 Section 6.5.2.1.7
		 * Implied switch to retain-signaling-link.
		 */
		pri_cc_act_set_retain_signaling_link(ctrl, cc_record);
		if (cc_record->fsm.qsig.msgtype == Q931_SETUP) {
			/* Send Q931_CALL_PROCEEDING message on signaling link. */
			pri_cc_act_send_call_proceeding(ctrl, cc_record);
		}
		if (cc_record->party_a_status == CC_PARTY_A_AVAILABILITY_BUSY) {
			/*
			 * The ccSuspend will be sent in a FACILITY or CONNECT
			 * message depending upon the CIS call state.
			 */
			pri_cc_act_send_cc_suspend(ctrl, cc_record);
			cc_record->state = CC_STATE_SUSPENDED;
		} else {
			/* Start QSIG_CC_T3 */
			pri_cc_act_start_t_recall(ctrl, cc_record);
			cc_record->state = CC_STATE_WAIT_CALLBACK;
		}
		break;
	case CC_EVENT_SUSPEND:
		pri_cc_act_set_a_status_busy(ctrl, cc_record);
		break;
	case CC_EVENT_RESUME:
		pri_cc_act_reset_a_status(ctrl, cc_record);
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_send_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_SIGNALING_GONE:
		/* Signaling link cleared. */
		pri_cc_act_disassociate_signaling_link(ctrl, cc_record);
		break;
	case CC_EVENT_LINK_CANCEL:
		/* Received ccCancel */
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_post_hangup_signaling(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		cc_record->state = CC_STATE_WAIT_DESTRUCTION;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_send_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM Q.SIG monitor CC_STATE_WAIT_CALLBACK.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_qsig_monitor_wait_callback(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_RECALL:
		/* The original call parameters have already been set. */
		pri_cc_act_queue_setup_recall(ctrl, cc_record, call);
		pri_cc_act_stop_t_recall(ctrl, cc_record);
		cc_record->state = CC_STATE_CALLBACK;
		break;
	case CC_EVENT_SUSPEND:
		pri_cc_act_stop_t_recall(ctrl, cc_record);
		/*
		 * The ccSuspend will be sent in a FACILITY or CONNECT
		 * message depending upon the CIS call state.
		 */
		pri_cc_act_send_cc_suspend(ctrl, cc_record);
		cc_record->state = CC_STATE_SUSPENDED;
		break;
	case CC_EVENT_TIMEOUT_T_RECALL:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_send_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_recall(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_send_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_recall(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_SIGNALING_GONE:
		/* Signaling link cleared. */
		pri_cc_act_disassociate_signaling_link(ctrl, cc_record);
		break;
	case CC_EVENT_LINK_CANCEL:
		/* Received ccCancel */
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_post_hangup_signaling(ctrl, cc_record);
		pri_cc_act_stop_t_recall(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		cc_record->state = CC_STATE_WAIT_DESTRUCTION;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_send_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_recall(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM Q.SIG monitor CC_STATE_CALLBACK.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_qsig_monitor_callback(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_send_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_SIGNALING_GONE:
		/* Signaling link cleared. */
		pri_cc_act_disassociate_signaling_link(ctrl, cc_record);
		break;
	case CC_EVENT_LINK_CANCEL:
		/* Received ccCancel */
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_post_hangup_signaling(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		cc_record->state = CC_STATE_WAIT_DESTRUCTION;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_send_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM Q.SIG monitor CC_STATE_SUSPENDED.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
static void pri_cc_fsm_qsig_monitor_suspended(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	switch (event) {
	case CC_EVENT_RESUME:
		pri_cc_act_send_cc_resume(ctrl, cc_record);
		pri_cc_act_reset_a_status(ctrl, cc_record);
		cc_record->state = CC_STATE_ACTIVATED;
		break;
	case CC_EVENT_TIMEOUT_T_SUPERVISION:
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_send_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	case CC_EVENT_SIGNALING_GONE:
		/* Signaling link cleared. */
		pri_cc_act_disassociate_signaling_link(ctrl, cc_record);
		break;
	case CC_EVENT_LINK_CANCEL:
		/* Received ccCancel */
		pri_cc_act_pass_up_cc_cancel(ctrl, cc_record);
		pri_cc_act_post_hangup_signaling(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		cc_record->state = CC_STATE_WAIT_DESTRUCTION;
		break;
	case CC_EVENT_CANCEL:
		pri_cc_act_send_cc_cancel(ctrl, cc_record);
		pri_cc_act_stop_t_supervision(ctrl, cc_record);
		pri_cc_act_set_self_destruct(ctrl, cc_record);
		cc_record->state = CC_STATE_IDLE;
		break;
	default:
		break;
	}
}

/*!
 * \internal
 * \brief CC FSM state function type.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \return Nothing
 */
typedef void (*pri_cc_fsm_state)(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event);

/*! CC FSM PTMP agent state table. */
static const pri_cc_fsm_state pri_cc_fsm_ptmp_agent[CC_STATE_NUM] = {
/* *INDENT-OFF* */
	[CC_STATE_IDLE] = pri_cc_fsm_ptmp_agent_idle,
	[CC_STATE_PENDING_AVAILABLE] = pri_cc_fsm_ptmp_agent_pend_avail,
	[CC_STATE_AVAILABLE] = pri_cc_fsm_ptmp_agent_avail,
	[CC_STATE_REQUESTED] = pri_cc_fsm_ptmp_agent_req,
	[CC_STATE_ACTIVATED] = pri_cc_fsm_ptmp_agent_activated,
	[CC_STATE_B_AVAILABLE] = pri_cc_fsm_ptmp_agent_b_avail,
	[CC_STATE_SUSPENDED] = pri_cc_fsm_ptmp_agent_suspended,
	[CC_STATE_WAIT_CALLBACK] = pri_cc_fsm_ptmp_agent_wait_callback,
	[CC_STATE_CALLBACK] = pri_cc_fsm_ptmp_agent_callback,
/* *INDENT-ON* */
};

/*! CC FSM PTMP monitor state table. */
static const pri_cc_fsm_state pri_cc_fsm_ptmp_monitor[CC_STATE_NUM] = {
/* *INDENT-OFF* */
	[CC_STATE_IDLE] = pri_cc_fsm_ptmp_monitor_idle,
	[CC_STATE_AVAILABLE] = pri_cc_fsm_ptmp_monitor_avail,
	[CC_STATE_REQUESTED] = pri_cc_fsm_ptmp_monitor_req,
	[CC_STATE_WAIT_DESTRUCTION] = pri_cc_fsm_ptmp_monitor_wait_destruction,
	[CC_STATE_ACTIVATED] = pri_cc_fsm_ptmp_monitor_activated,
	[CC_STATE_WAIT_CALLBACK] = pri_cc_fsm_ptmp_monitor_wait_callback,
	[CC_STATE_CALLBACK] = pri_cc_fsm_ptmp_monitor_callback,
/* *INDENT-ON* */
};

/*! CC FSM PTP agent state table. */
static const pri_cc_fsm_state pri_cc_fsm_ptp_agent[CC_STATE_NUM] = {
/* *INDENT-OFF* */
	[CC_STATE_IDLE] = pri_cc_fsm_ptp_agent_idle,
	[CC_STATE_PENDING_AVAILABLE] = pri_cc_fsm_ptp_agent_pend_avail,
	[CC_STATE_AVAILABLE] = pri_cc_fsm_ptp_agent_avail,
	[CC_STATE_REQUESTED] = pri_cc_fsm_ptp_agent_req,
	[CC_STATE_ACTIVATED] = pri_cc_fsm_ptp_agent_activated,
	[CC_STATE_WAIT_CALLBACK] = pri_cc_fsm_ptp_agent_wait_callback,
	[CC_STATE_SUSPENDED] = pri_cc_fsm_ptp_agent_suspended,
/* *INDENT-ON* */
};

/*! CC FSM PTP monitor state table. */
static const pri_cc_fsm_state pri_cc_fsm_ptp_monitor[CC_STATE_NUM] = {
/* *INDENT-OFF* */
	[CC_STATE_IDLE] = pri_cc_fsm_ptp_monitor_idle,
	[CC_STATE_AVAILABLE] = pri_cc_fsm_ptp_monitor_avail,
	[CC_STATE_REQUESTED] = pri_cc_fsm_ptp_monitor_req,
	[CC_STATE_WAIT_DESTRUCTION] = pri_cc_fsm_ptp_monitor_wait_destruction,
	[CC_STATE_ACTIVATED] = pri_cc_fsm_ptp_monitor_activated,
	[CC_STATE_WAIT_CALLBACK] = pri_cc_fsm_ptp_monitor_wait_callback,
	[CC_STATE_SUSPENDED] = pri_cc_fsm_ptp_monitor_suspended,
/* *INDENT-ON* */
};

/*! CC FSM Q.SIG agent state table. */
static const pri_cc_fsm_state pri_cc_fsm_qsig_agent[CC_STATE_NUM] = {
/* *INDENT-OFF* */
	[CC_STATE_IDLE] = pri_cc_fsm_qsig_agent_idle,
	[CC_STATE_AVAILABLE] = pri_cc_fsm_qsig_agent_avail,
	[CC_STATE_REQUESTED] = pri_cc_fsm_qsig_agent_req,
	[CC_STATE_WAIT_DESTRUCTION] = pri_cc_fsm_qsig_agent_wait_destruction,
	[CC_STATE_ACTIVATED] = pri_cc_fsm_qsig_agent_activated,
	[CC_STATE_WAIT_CALLBACK] = pri_cc_fsm_qsig_agent_wait_callback,
	[CC_STATE_SUSPENDED] = pri_cc_fsm_qsig_agent_suspended,
/* *INDENT-ON* */
};

/*! CC FSM Q.SIG monitor state table. */
static const pri_cc_fsm_state pri_cc_fsm_qsig_monitor[CC_STATE_NUM] = {
/* *INDENT-OFF* */
	[CC_STATE_IDLE] = pri_cc_fsm_qsig_monitor_idle,
	[CC_STATE_AVAILABLE] = pri_cc_fsm_qsig_monitor_avail,
	[CC_STATE_REQUESTED] = pri_cc_fsm_qsig_monitor_req,
	[CC_STATE_WAIT_DESTRUCTION] = pri_cc_fsm_qsig_monitor_wait_destruction,
	[CC_STATE_ACTIVATED] = pri_cc_fsm_qsig_monitor_activated,
	[CC_STATE_WAIT_CALLBACK] = pri_cc_fsm_qsig_monitor_wait_callback,
	[CC_STATE_CALLBACK] = pri_cc_fsm_qsig_monitor_callback,
	[CC_STATE_SUSPENDED] = pri_cc_fsm_qsig_monitor_suspended,
/* *INDENT-ON* */
};

/*!
 * \brief Send an event to the cc state machine.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * (May be NULL if it is supposed to be the signaling connection
 * for Q.SIG or PTP and it is not established yet.)
 * \param cc_record Call completion record to process event.
 * \param event Event to process.
 *
 * \retval nonzero if cc record destroyed because FSM completed.
 */
int pri_cc_event(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, enum CC_EVENTS event)
{
	const pri_cc_fsm_state *cc_fsm;
	enum CC_STATES orig_state;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_QSIG:
		if (cc_record->is_agent) {
			cc_fsm = pri_cc_fsm_qsig_agent;
		} else {
			cc_fsm = pri_cc_fsm_qsig_monitor;
		}
		break;
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		if (PTMP_MODE(ctrl)) {
			if (cc_record->is_agent) {
				cc_fsm = pri_cc_fsm_ptmp_agent;
			} else {
				cc_fsm = pri_cc_fsm_ptmp_monitor;
			}
		} else {
			if (cc_record->is_agent) {
				cc_fsm = pri_cc_fsm_ptp_agent;
			} else {
				cc_fsm = pri_cc_fsm_ptp_monitor;
			}
		}
		break;
	default:
		/* CC not supported on this switch type. */
		cc_fsm = NULL;
		break;
	}

	if (!cc_fsm) {
		/* No FSM available. */
		pri_cc_delete_record(ctrl, cc_record);
		return 1;
	}
	orig_state = cc_record->state;
	if (ctrl->debug & PRI_DEBUG_CC) {
		pri_message(ctrl, "%ld CC-Event: %s in state %s\n", cc_record->record_id,
			pri_cc_fsm_event_str(event), pri_cc_fsm_state_str(orig_state));
	}
	if (orig_state < CC_STATE_IDLE || CC_STATE_NUM <= orig_state || !cc_fsm[orig_state]) {
		/* Programming error: State not implemented. */
		pri_error(ctrl, "!! CC state not implemented: %s(%d)\n",
			pri_cc_fsm_state_str(orig_state), orig_state);
		return 0;
	}
	/* Execute the state. */
	cc_fsm[orig_state](ctrl, call, cc_record, event);
	if (ctrl->debug & PRI_DEBUG_CC) {
		pri_message(ctrl, "%ld  CC-Next-State: %s\n", cc_record->record_id,
			(orig_state == cc_record->state)
			? "$" : pri_cc_fsm_state_str(cc_record->state));
	}
	if (cc_record->fsm_complete) {
		pri_cc_delete_record(ctrl, cc_record);
		return 1;
	} else {
		return 0;
	}
}

/*!
 * \brief Indicate to the far end that CCBS/CCNR is available.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 *
 * \details
 * The CC available indication will go out with the next
 * DISCONNECT(busy/congested)/ALERTING message.
 *
 * \retval cc_id on success for subsequent reference.
 * \retval -1 on error.
 */
long pri_cc_available(struct pri *ctrl, q931_call *call)
{
	struct pri_cc_record *cc_record;
	long cc_id;

	if (!ctrl || !pri_is_call_valid(ctrl, call)) {
		return -1;
	}
	if (call->cc.record) {
		/* This call is already associated with call completion. */
		return -1;
	}

	cc_record = NULL;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_QSIG:
		cc_record = pri_cc_new_record(ctrl, call);
		if (!cc_record) {
			break;
		}

		/*
		 * Q.SIG has no message to send when CC is available.
		 * Q.SIG assumes CC is always available and is denied when
		 * requested if CC is not possible or allowed.
		 */
		cc_record->original_call = call;
		cc_record->is_agent = 1;
		break;
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		if (PTMP_MODE(ctrl)) {
			int linkage_id;

			if (!BRI_NT_PTMP(ctrl)) {
				/*
				 * No CC agent protocol defined for this mode.
				 * i.e.,  A device acting like a phone cannot be a CC agent.
				 */
				break;
			}

			linkage_id = pri_cc_new_linkage_id(ctrl);
			if (linkage_id == CC_PTMP_INVALID_ID) {
				break;
			}
			cc_record = pri_cc_new_record(ctrl, call);
			if (!cc_record) {
				break;
			}
			cc_record->call_linkage_id = linkage_id;
			cc_record->signaling = ctrl->link.dummy_call;
		} else {
			cc_record = pri_cc_new_record(ctrl, call);
			if (!cc_record) {
				break;
			}
		}
		cc_record->original_call = call;
		cc_record->is_agent = 1;
		break;
	default:
		break;
	}

	call->cc.record = cc_record;
	if (cc_record && !pri_cc_event(ctrl, call, cc_record, CC_EVENT_AVAILABLE)) {
		cc_id = cc_record->record_id;
	} else {
		cc_id = -1;
	}
	return cc_id;
}

/*!
 * \brief Determine if CC is available for Q.SIG outgoing call.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 *
 * \return Nothing
 */
void pri_cc_qsig_determine_available(struct pri *ctrl, q931_call *call)
{
	struct pri_cc_record *cc_record;

	if (!call->cc.originated || call->cc.initially_redirected) {
		/*
		 * The call is not suitable for us to consider CC:
		 * The call was not originated by us.
		 * The call was originally redirected.
		 */
		return;
	}

	if (!ctrl->cc_support) {
		/*
		 * Blocking the cc-available event effectively
		 * disables call completion for outgoing calls.
		 */
		return;
	}
	if (call->cc.record) {
		/* Already made available. */
		return;
	}
	cc_record = pri_cc_new_record(ctrl, call);
	if (!cc_record) {
		return;
	}
	cc_record->original_call = call;
	call->cc.record = cc_record;
	pri_cc_event(ctrl, call, cc_record, CC_EVENT_AVAILABLE);
}

/*!
 * \brief Request to activate CC.
 *
 * \param ctrl D channel controller.
 * \param cc_id CC record ID to activate.
 * \param mode Which CC mode to use CCBS(0)/CCNR(1)
 *
 * \note
 * Will always get a reply from libpri.  libpri will start a timer to guarantee
 * that a reply will be passed back to the upper layer.
 * \note
 * If you cancel with pri_cc_cancel() you are indicating that you do not need
 * the request reply and the cc_id will no longer be valid anyway.
 * \note
 * Allow for the possibility that the reply may come in before this
 * function returns.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int pri_cc_req(struct pri *ctrl, long cc_id, int mode)
{
	struct pri_sr req;
	q931_call *call;
	struct pri_cc_record *cc_record;

	if (!ctrl) {
		return -1;
	}
	cc_record = pri_cc_find_by_id(ctrl, cc_id);
	if (!cc_record) {
		return -1;
	}
	if (cc_record->is_agent || cc_record->state != CC_STATE_AVAILABLE) {
		/* CC is an agent or already requested. */
		return -1;
	}

	/* Set the requested CC mode. */
	cc_record->is_ccnr = mode ? 1 : 0;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_QSIG:
		if (cc_record->signaling) {
			/* We should not have a signaling link at this point. */
			return -1;
		}
		call = q931_new_call(ctrl);
		if (!call) {
			return -1;
		}

		/* Link the new call as the signaling link. */
		cc_record->signaling = call;
		call->cc.record = cc_record;

		if (pri_cc_event(ctrl, call, cc_record, CC_EVENT_CC_REQUEST)) {
			/* Should not happen. */
			q931_destroycall(ctrl, call);
			break;
		}

		pri_sr_init(&req);
		req.caller = cc_record->party_a;
		req.called = cc_record->party_b;
		//req.cis_auto_disconnect = 0;
		req.cis_call = 1;
		if (q931_setup(ctrl, call, &req)) {
			/* Should not happen. */
			q931_destroycall(ctrl, call);
			return -1;
		}
		break;
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		if (PTMP_MODE(ctrl)) {
			/* ETSI PTMP */
			if (pri_cc_event(ctrl, cc_record->signaling, cc_record, CC_EVENT_CC_REQUEST)) {
				/* Should not happen. */
				break;
			}
			q931_facility(ctrl, cc_record->signaling);
			break;
		}

		/* ETSI PTP */
		if (cc_record->signaling) {
			/* We should not have a signaling link at this point. */
			return -1;
		}
		call = q931_new_call(ctrl);
		if (!call) {
			return -1;
		}

		cc_record->signaling = call;
		call->cc.record = cc_record;
		if (pri_cc_event(ctrl, call, cc_record, CC_EVENT_CC_REQUEST)) {
			/* Should not happen. */
			q931_destroycall(ctrl, call);
			break;
		}

		if (q931_register(ctrl, call)) {
			/* Should not happen. */
			q931_destroycall(ctrl, call);
			return -1;
		}
		break;
	default:
		return -1;
	}

	return 0;
}

/*!
 * \internal
 * \brief Encode a PTMP cc-request reply message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param operation CCBS/CCNR operation code.
 * \param invoke_id Invoke id to put in error message response.
 * \param recall_mode Configured PTMP recall mode.
 * \param reference_id Active CC reference id.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_cc_etsi_ptmp_req_rsp(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, enum rose_operation operation, int invoke_id, int recall_mode,
	int reference_id)
{
	struct rose_msg_result msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = invoke_id;
	msg.operation = operation;

	/* CCBS/CCNR reply */
	msg.args.etsi.CCBSRequest.recall_mode = recall_mode;
	msg.args.etsi.CCBSRequest.ccbs_reference = reference_id;

	pos = rose_encode_result(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode and queue PTMP a cc-request reply message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param msgtype Q.931 message type to put facility ie in.
 * \param operation CCBS/CCNR operation code.
 * \param invoke_id Invoke id to put in error message response.
 * \param recall_mode Configured PTMP recall mode.
 * \param reference_id Active CC reference id.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_cc_etsi_ptmp_req_rsp_encode(struct pri *ctrl, q931_call *call, int msgtype, enum rose_operation operation, int invoke_id, int recall_mode, int reference_id)
{
	unsigned char buffer[256];
	unsigned char *end;

	end = enc_cc_etsi_ptmp_req_rsp(ctrl, buffer, buffer + sizeof(buffer), operation,
		invoke_id, recall_mode, reference_id);
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, msgtype, buffer, end - buffer, NULL);
}

/*!
 * \internal
 * \brief Send the CC activation request result PTMP.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param operation CCBS/CCNR operation code.
 * \param invoke_id Invoke id to put in error message response.
 * \param recall_mode Configured PTMP recall mode.
 * \param reference_id Active CC reference id.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int send_cc_etsi_ptmp_req_rsp(struct pri *ctrl, q931_call *call, enum rose_operation operation, int invoke_id, int recall_mode, int reference_id)
{
	if (rose_cc_etsi_ptmp_req_rsp_encode(ctrl, call, Q931_FACILITY, operation, invoke_id,
		recall_mode, reference_id)
		|| q931_facility(ctrl, call)) {
		pri_message(ctrl, "Could not schedule CC request result message.\n");
		return -1;
	}

	return 0;
}

/*!
 * \internal
 * \brief Response to an incoming CC activation request PTMP.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 * \param status success(0)/timeout(1)/
 *		short_term_denial(2)/long_term_denial(3)/not_subscribed(4)/queue_full(5)
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int pri_cc_req_rsp_ptmp(struct pri *ctrl, struct pri_cc_record *cc_record, int status)
{
	int fail;

	switch (cc_record->response.invoke_operation) {
	case ROSE_ETSI_CCBSRequest:
	case ROSE_ETSI_CCNRRequest:
		break;
	default:
		/* We no longer know how to send the response.  Should not happen. */
		return -1;
	}

	fail = 0;
	if (status) {
		enum rose_error_code code;

		switch (status) {
		default:
		case 1:/* timeout */
		case 2:/* short_term_denial */
			code = ROSE_ERROR_CCBS_ShortTermDenial;
			break;
		case 3:/* long_term_denial */
			code = ROSE_ERROR_CCBS_LongTermDenial;
			break;
		case 4:/* not_subscribed */
			code = ROSE_ERROR_Gen_NotSubscribed;
			break;
		case 5:/* queue_full */
			code = ROSE_ERROR_CCBS_OutgoingCCBSQueueFull;
			break;
		}
		send_facility_error(ctrl, cc_record->response.signaling,
			cc_record->response.invoke_id, code);
		pri_cc_event(ctrl, cc_record->response.signaling, cc_record,
			CC_EVENT_CANCEL);
	} else {
		/* Successful CC activation. */
		if (send_cc_etsi_ptmp_req_rsp(ctrl, cc_record->response.signaling,
			cc_record->response.invoke_operation, cc_record->response.invoke_id,
			cc_record->option.recall_mode, cc_record->ccbs_reference_id)) {
			fail = -1;
		}
		pri_cc_event(ctrl, cc_record->response.signaling, cc_record,
			CC_EVENT_CC_REQUEST_ACCEPT);
	}
	return fail;
}

/*!
 * \internal
 * \brief Encode a PTP cc-request reply message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param cc_record Call completion record to process event.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_cc_etsi_ptp_req_rsp(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, struct pri_cc_record *cc_record)
{
	struct rose_msg_result msg;

	pos = facility_encode_header(ctrl, pos, end, NULL);
	if (!pos) {
		return NULL;
	}

	memset(&msg, 0, sizeof(msg));
	msg.invoke_id = cc_record->response.invoke_id;
	msg.operation = cc_record->response.invoke_operation;

	/* CCBS/CCNR reply */
	//msg.args.etsi.CCBS_T_Request.retention_supported = 0;

	pos = rose_encode_result(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode and queue PTP a cc-request reply message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_cc_etsi_ptp_req_rsp_encode(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record)
{
	unsigned char buffer[256];
	unsigned char *end;

	end = enc_cc_etsi_ptp_req_rsp(ctrl, buffer, buffer + sizeof(buffer), cc_record);
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL);
}

/*!
 * \internal
 * \brief Send the CC activation request result PTP.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int send_cc_etsi_ptp_req_rsp(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	if (rose_cc_etsi_ptp_req_rsp_encode(ctrl, cc_record->signaling, cc_record)
		|| q931_facility(ctrl, cc_record->signaling)) {
		pri_message(ctrl, "Could not schedule CC request result message.\n");
		return -1;
	}

	return 0;
}

/*!
 * \internal
 * \brief Response to an incoming CC activation request PTP.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 * \param status success(0)/timeout(1)/
 *		short_term_denial(2)/long_term_denial(3)/not_subscribed(4)/queue_full(5)
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int pri_cc_req_rsp_ptp(struct pri *ctrl, struct pri_cc_record *cc_record, int status)
{
	int fail;

	switch (cc_record->response.invoke_operation) {
	case ROSE_ETSI_CCBS_T_Request:
	case ROSE_ETSI_CCNR_T_Request:
		break;
	default:
		/* We no longer know how to send the response.  Should not happen. */
		return -1;
	}
	if (!cc_record->signaling) {
		return -1;
	}

	fail = 0;
	if (status) {
		enum rose_error_code code;

		switch (status) {
		default:
		case 1:/* timeout */
		case 5:/* queue_full */
		case 2:/* short_term_denial */
			code = ROSE_ERROR_CCBS_T_ShortTermDenial;
			break;
		case 3:/* long_term_denial */
			code = ROSE_ERROR_CCBS_T_LongTermDenial;
			break;
		case 4:/* not_subscribed */
			code = ROSE_ERROR_Gen_NotSubscribed;
			break;
		}
		rose_error_msg_encode(ctrl, cc_record->signaling, Q931_ANY_MESSAGE,
			cc_record->response.invoke_id, code);
		pri_cc_event(ctrl, cc_record->signaling, cc_record, CC_EVENT_CANCEL);
	} else {
		/* Successful CC activation. */
		if (send_cc_etsi_ptp_req_rsp(ctrl, cc_record)) {
			fail = -1;
		}
		pri_cc_event(ctrl, cc_record->signaling, cc_record, CC_EVENT_CC_REQUEST_ACCEPT);
	}
	return fail;
}

/*!
 * \internal
 * \brief Encode a Q.SIG cc-request reply message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode the facility ie contents.
 * \param end End of facility ie contents encoding data buffer.
 * \param cc_record Call completion record to process event.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *enc_cc_qsig_req_rsp(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, struct pri_cc_record *cc_record)
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
	msg.invoke_id = cc_record->response.invoke_id;
	msg.operation = cc_record->response.invoke_operation;

	/* CCBS/CCNR reply */

	/* We do not support ccPathReserve */
	msg.args.qsig.CcbsRequest.no_path_reservation = 1;
	//msg.args.qsig.CcbsRequest.retain_service = 0;

	pos = rose_encode_result(ctrl, pos, end, &msg);

	return pos;
}

/*!
 * \internal
 * \brief Encode and queue Q.SIG a cc-request reply message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_cc_qsig_req_rsp_encode(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record)
{
	unsigned char buffer[256];
	unsigned char *end;

	end = enc_cc_qsig_req_rsp(ctrl, buffer, buffer + sizeof(buffer), cc_record);
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, Q931_ANY_MESSAGE, buffer, end - buffer, NULL);
}

/*!
 * \internal
 * \brief Send the CC activation request result Q.SIG.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int send_cc_qsig_req_rsp(struct pri *ctrl, struct pri_cc_record *cc_record)
{
	struct q931_call *call;
	int retval;

	/* The cc-request response goes out on either a CONNECT or RELEASE message. */
	call = cc_record->signaling;
	retval = rose_cc_qsig_req_rsp_encode(ctrl, call, cc_record);
	if (!retval) {
		if (cc_record->option.retain_signaling_link) {
			retval = q931_connect(ctrl, call, 0, 0);
		} else {
			pri_cc_disassociate_signaling_link(cc_record);
			retval = pri_hangup(ctrl, call, -1);
		}
	}
	if (retval) {
		pri_message(ctrl, "Could not schedule CC request result message.\n");
		return -1;
	}

	return 0;
}

/*!
 * \internal
 * \brief Response to an incoming CC activation request Q.SIG.
 *
 * \param ctrl D channel controller.
 * \param cc_record Call completion record to process event.
 * \param status success(0)/timeout(1)/
 *		short_term_denial(2)/long_term_denial(3)/not_subscribed(4)/queue_full(5)
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int pri_cc_req_rsp_qsig(struct pri *ctrl, struct pri_cc_record *cc_record, int status)
{
	int fail;

	switch (cc_record->response.invoke_operation) {
	case ROSE_QSIG_CcbsRequest:
	case ROSE_QSIG_CcnrRequest:
		break;
	default:
		/* We no longer know how to send the response.  Should not happen. */
		return -1;
	}
	if (!cc_record->signaling) {
		return -1;
	}

	fail = 0;
	if (status) {
		enum rose_error_code code;

		switch (status) {
		default:
		case 1:/* timeout */
		case 5:/* queue_full */
		case 2:/* short_term_denial */
			code = ROSE_ERROR_QSIG_ShortTermRejection;
			break;
		case 4:/* not_subscribed */
		case 3:/* long_term_denial */
			code = ROSE_ERROR_QSIG_LongTermRejection;
			break;
		}
		rose_error_msg_encode(ctrl, cc_record->signaling, Q931_ANY_MESSAGE,
			cc_record->response.invoke_id, code);
		pri_cc_event(ctrl, cc_record->signaling, cc_record, CC_EVENT_CANCEL);
	} else {
		/* Successful CC activation. */
		if (send_cc_qsig_req_rsp(ctrl, cc_record)) {
			fail = -1;
		}
		pri_cc_event(ctrl, cc_record->signaling, cc_record, CC_EVENT_CC_REQUEST_ACCEPT);
	}
	return fail;
}

/*!
 * \brief Response to an incoming CC activation request.
 *
 * \param ctrl D channel controller.
 * \param cc_id CC record ID to activate.
 * \param status success(0)/timeout(1)/
 *      short_term_denial(2)/long_term_denial(3)/not_subscribed(4)/queue_full(5)
 *
 * \note
 * If the given status was failure, then the cc_id is no longer valid.
 * \note
 * The caller should cancel CC if error is returned.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int pri_cc_req_rsp(struct pri *ctrl, long cc_id, int status)
{
	struct pri_cc_record *cc_record;
	int fail;

	if (!ctrl) {
		return -1;
	}
	cc_record = pri_cc_find_by_id(ctrl, cc_id);
	if (!cc_record) {
		return -1;
	}
	if (!cc_record->is_agent) {
		/* CC is a monitor and does not send this response event. */
		return -1;
	}

	fail = -1;
	switch (ctrl->switchtype) {
	case PRI_SWITCH_QSIG:
		if (!pri_cc_req_rsp_qsig(ctrl, cc_record, status)) {
			fail = 0;
		}
		break;
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		if (PTMP_MODE(ctrl)) {
			if (!pri_cc_req_rsp_ptmp(ctrl, cc_record, status)) {
				fail = 0;
			}
		} else {
			if (!pri_cc_req_rsp_ptp(ctrl, cc_record, status)) {
				fail = 0;
			}
		}
		break;
	default:
		break;
	}
	return fail;
}

/*!
 * \brief Indicate that the remote user (Party B) is free to call.
 * The upper layer considers Party A is free.
 *
 * \param ctrl D channel controller.
 * \param cc_id CC record ID to activate.
 *
 * \return Nothing
 */
void pri_cc_remote_user_free(struct pri *ctrl, long cc_id)
{
	struct pri_cc_record *cc_record;

	if (!ctrl) {
		return;
	}
	cc_record = pri_cc_find_by_id(ctrl, cc_id);
	if (!cc_record) {
		return;
	}
	if (!cc_record->is_agent) {
		/* CC is a monitor and does not send this event. */
		return;
	}

	pri_cc_event(ctrl, cc_record->signaling, cc_record, CC_EVENT_REMOTE_USER_FREE);
}

/*!
 * \brief Indicate that the remote user (Party B) is free to call.
 * However, the upper layer considers Party A is busy.
 *
 * \details
 * Party B is free, but Party A is considered busy for some reason.
 * This is mainly due to the upper layer experiencing congestion.
 * The upper layer will be monitoring Party A until it considers
 * Party A free again.
 *
 * \param ctrl D channel controller.
 * \param cc_id CC record ID to activate.
 *
 * \return Nothing
 */
void pri_cc_b_free(struct pri *ctrl, long cc_id)
{
	struct pri_cc_record *cc_record;

	if (!ctrl) {
		return;
	}
	cc_record = pri_cc_find_by_id(ctrl, cc_id);
	if (!cc_record) {
		return;
	}
	if (!cc_record->is_agent) {
		/* CC is a monitor and does not send this event. */
		return;
	}

	pri_cc_event(ctrl, cc_record->signaling, cc_record, CC_EVENT_B_FREE);
}

/*!
 * \brief Indicate that some other Party A has responed to the CC recall.
 *
 * \param ctrl D channel controller.
 * \param cc_id CC record ID to activate.
 *
 * \return Nothing
 */
void pri_cc_stop_alerting(struct pri *ctrl, long cc_id)
{
	struct pri_cc_record *cc_record;

	if (!ctrl) {
		return;
	}
	cc_record = pri_cc_find_by_id(ctrl, cc_id);
	if (!cc_record) {
		return;
	}
	if (!cc_record->is_agent) {
		/* CC is a monitor and does not send this event. */
		return;
	}

	pri_cc_event(ctrl, cc_record->signaling, cc_record, CC_EVENT_STOP_ALERTING);
}

/*!
 * \brief Poll/Ping for the status of CC party A.
 *
 * \param ctrl D channel controller.
 * \param cc_id CC record ID to activate.
 *
 * \note
 * There could be zero, one, or more PRI_SUBCMD_CC_STATUS_REQ_RSP responses to
 * the status request depending upon how many endpoints respond to the request.
 * \note
 * This is expected to be called only if there are two PTMP links between
 * party A and the network.  (e.g., A --> * --> PSTN)
 *
 * \return Nothing
 */
void pri_cc_status_req(struct pri *ctrl, long cc_id)
{
	struct pri_cc_record *cc_record;

	if (!ctrl) {
		return;
	}
	cc_record = pri_cc_find_by_id(ctrl, cc_id);
	if (!cc_record) {
		return;
	}
	if (!cc_record->is_agent) {
		/* CC is a monitor and does not send this event. */
		return;
	}

	pri_cc_event(ctrl, cc_record->signaling, cc_record, CC_EVENT_A_STATUS);
}

/*!
 * \internal
 * \brief Encode and queue an CCBSStatusRequest result message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode CCBSStatusRequest.
 * \param cc_record Call completion record to process event.
 * \param is_free TRUE if the Party A status is available.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int rose_ccbs_status_request_rsp(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, int is_free)
{
	unsigned char buffer[256];
	unsigned char *end;

	end =
		enc_etsi_ptmp_ccbs_status_request_rsp(ctrl, buffer, buffer + sizeof(buffer),
			cc_record, is_free);
	if (!end) {
		return -1;
	}

	return pri_call_apdu_queue(call, Q931_FACILITY, buffer, end - buffer, NULL);
}

/*!
 * \internal
 * \brief Encode and send an CCBSStatusRequest result message.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param call Call leg from which to encode CCBSStatusRequest.
 * \param cc_record Call completion record to process event.
 * \param is_free TRUE if the Party A status is available.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
static int send_ccbs_status_request_rsp(struct pri *ctrl, q931_call *call, struct pri_cc_record *cc_record, int is_free)
{
	if (rose_ccbs_status_request_rsp(ctrl, call, cc_record, is_free)
		|| q931_facility(ctrl, call)) {
		pri_message(ctrl,
			"Could not schedule facility message for CCBSStatusRequest result.\n");
		return -1;
	}

	return 0;
}

/*!
 * \brief Update the busy status of CC party A.
 *
 * \param ctrl D channel controller.
 * \param cc_id CC record ID to activate.
 * \param status Updated party A status free(0)/busy(1)
 *
 * \note
 * This is expected to be called only if there are two PTMP links between
 * party A and the network.  (e.g., A --> * --> PSTN)
 *
 * \return Nothing
 */
void pri_cc_status_req_rsp(struct pri *ctrl, long cc_id, int status)
{
	struct pri_cc_record *cc_record;

	if (!ctrl) {
		return;
	}
	cc_record = pri_cc_find_by_id(ctrl, cc_id);
	if (!cc_record) {
		return;
	}
	if (cc_record->is_agent) {
		/* CC is an agent and does not send this response event. */
		return;
	}

	switch (ctrl->switchtype) {
	case PRI_SWITCH_QSIG:
		/* Does not apply. */
		break;
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		if (PTMP_MODE(ctrl)) {
			if (cc_record->response.invoke_operation != ROSE_ETSI_CCBSStatusRequest) {
				/* We no longer know how to send the response. */
				break;
			}
			send_ccbs_status_request_rsp(ctrl, cc_record->signaling, cc_record,
				status ? 0 /* busy */ : 1 /* free */);
		}
		break;
	default:
		break;
	}
}

/*!
 * \brief Update the busy status of CC party A.
 *
 * \param ctrl D channel controller.
 * \param cc_id CC record ID to activate.
 * \param status Updated party A status free(0)/busy(1)
 *
 * \note
 * Party A status is used to suspend/resume monitoring party B.
 *
 * \return Nothing
 */
void pri_cc_status(struct pri *ctrl, long cc_id, int status)
{
	struct pri_cc_record *cc_record;

	if (!ctrl) {
		return;
	}
	cc_record = pri_cc_find_by_id(ctrl, cc_id);
	if (!cc_record) {
		return;
	}
	if (cc_record->is_agent) {
		/* CC is an agent and does not send this event. */
		return;
	}

	pri_cc_event(ctrl, cc_record->signaling, cc_record,
		status ? CC_EVENT_SUSPEND : CC_EVENT_RESUME);
}

/*!
 * \brief Initiate the CC callback call.
 *
 * \param ctrl D channel controller.
 * \param cc_id CC record ID to activate.
 * \param call Q.931 call leg.
 * \param req SETUP request parameters.  Parameters saved by CC will override.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int pri_cc_call(struct pri *ctrl, long cc_id, q931_call *call, struct pri_sr *req)
{
	struct pri_cc_record *cc_record;

	if (!ctrl || !pri_is_call_valid(ctrl, call) || !req) {
		return -1;
	}
	cc_record = pri_cc_find_by_id(ctrl, cc_id);
	if (!cc_record) {
		return -1;
	}
	if (cc_record->is_agent) {
		/* CC is an agent and does not initiate callbacks. */
		return -1;
	}

	/* Override parameters for sending recall. */
	req->caller = cc_record->party_a;
	req->called = cc_record->party_b;
	req->transmode = cc_record->bc.transcapability;
	req->userl1 = cc_record->bc.userl1;

	/*
	 * The caller is allowed to send different user-user information.
	 *
	 * It makes no sense for the caller to supply redirecting information
	 * but we'll allow it to pass anyway.
	 */
	//q931_party_redirecting_init(&req->redirecting);

	/* Add switch specific recall APDU to call. */
	pri_cc_event(ctrl, call, cc_record, CC_EVENT_RECALL);

	if (q931_setup(ctrl, call, req)) {
		return -1;
	}
	return 0;
}

/*!
 * \brief Unsolicited indication that CC is cancelled.
 *
 * \param ctrl D channel controller.
 * \param cc_id CC record ID to deactivate.
 *
 * \return Nothing.  The cc_id is no longer valid.
 */
void pri_cc_cancel(struct pri *ctrl, long cc_id)
{
	struct pri_cc_record *cc_record;

	if (!ctrl) {
		return;
	}
	cc_record = pri_cc_find_by_id(ctrl, cc_id);
	if (!cc_record) {
		return;
	}
	pri_cc_event(ctrl, cc_record->signaling, cc_record, CC_EVENT_CANCEL);
}

/* ------------------------------------------------------------------- */
/* end pri_cc.c */
