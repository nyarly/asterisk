/*
   This file contains all data structures and definitions associated
   with facility message usage and the ROSE components included
   within those messages.

   by Matthew Fredrickson <creslin@digium.com>
   Copyright (C) Digium, Inc. 2004-2005
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

#ifndef _PRI_FACILITY_H
#define _PRI_FACILITY_H
#include "pri_q931.h"
#include "rose.h"

/* Protocol Profile field */
#define Q932_PROTOCOL_MASK			0x1F
#define Q932_PROTOCOL_ROSE			0x11	/* X.219 & X.229 */
#define Q932_PROTOCOL_CMIP			0x12	/* Q.941 */
#define Q932_PROTOCOL_ACSE			0x13	/* X.217 & X.227 */
#define Q932_PROTOCOL_GAT			0x16
#define Q932_PROTOCOL_EXTENSIONS	0x1F

/* Q.952 Divert cause */
#define Q952_DIVERT_REASON_UNKNOWN		0x00
#define Q952_DIVERT_REASON_CFU			0x01
#define Q952_DIVERT_REASON_CFB			0x02
#define Q952_DIVERT_REASON_CFNR			0x03
#define Q952_DIVERT_REASON_CD			0x04
#define Q952_DIVERT_REASON_IMMEDIATE	0x05
/* Q.SIG Divert cause. Listed in ECMA-174 */
#define QSIG_DIVERT_REASON_UNKNOWN		0x00	/* Call forward unknown reason */
#define QSIG_DIVERT_REASON_CFU			0x01	/* Call Forward Unconditional (other reason) */
#define QSIG_DIVERT_REASON_CFB			0x02	/* Call Forward Busy */
#define QSIG_DIVERT_REASON_CFNR			0x03	/* Call Forward No Reply */

/* Q.932 Type of number */
#define Q932_TON_UNKNOWN				0x00
#define Q932_TON_INTERNATIONAL			0x01
#define Q932_TON_NATIONAL				0x02
#define Q932_TON_NET_SPECIFIC			0x03
#define Q932_TON_SUBSCRIBER				0x04
#define Q932_TON_ABBREVIATED			0x06

/* Q.SIG Subscription Option. Listed in ECMA-174 */
#define QSIG_NO_NOTIFICATION						0x00
#define QSIG_NOTIFICATION_WITHOUT_DIVERTED_TO_NR	0x01
#define QSIG_NOTIFICATION_WITH_DIVERTED_TO_NR		0x02

/*! Reasons an APDU callback is called. */
enum APDU_CALLBACK_REASON {
	/*!
	 * \brief Transmit facility ie setup error.  Abort and cleanup.
	 * \note The message may or may not actually get sent.
	 * \note The callback cannot generate an event subcmd.
	 * \note The callback should not send messages.  Out of order messages will result.
	 */
	APDU_CALLBACK_REASON_ERROR,
	/*!
	 * \brief Abort and cleanup.
	 * \note The APDU queue is being destroyed.
	 * \note The callback cannot generate an event subcmd.
	 * \note The callback cannot send messages as the call is likely being destroyed.
	 */
	APDU_CALLBACK_REASON_CLEANUP,
	/*!
	 * \brief Timeout waiting for responses to the message.
	 * \note The callback can generate an event subcmd.
	 * \note The callback can send messages.
	 */
	APDU_CALLBACK_REASON_TIMEOUT,
	/*!
	 * \brief Received a facility response message.
	 * \note The callback can generate an event subcmd.
	 * \note The callback can send messages.
	 */
	APDU_CALLBACK_REASON_MSG_RESULT,
	/*!
	 * \brief Received a facility error message.
	 * \note The callback can generate an event subcmd.
	 * \note The callback can send messages.
	 */
	APDU_CALLBACK_REASON_MSG_ERROR,
	/*!
	 * \brief Received a facility reject message.
	 * \note The callback can generate an event subcmd.
	 * \note The callback can send messages.
	 */
	APDU_CALLBACK_REASON_MSG_REJECT,
};

struct apdu_msg_data {
	/*! Decoded response message contents. */
	union {
		const struct rose_msg_result *result;
		const struct rose_msg_error *error;
		const struct rose_msg_reject *reject;
	} response;
	/*! Q.931 message type the response came in with. */
	int type;
};

union apdu_callback_param {
	void *ptr;
	long value;
	char pad[8];
};

/* So calls to pri_call_apdu_find() will not find an aliased event. */
#define APDU_INVALID_INVOKE_ID  0x10000

#define APDU_TIMEOUT_MSGS_ONLY	-1

struct apdu_callback_data {
	/*! APDU invoke id to match with any response messages. (Result/Error/Reject) */
	int invoke_id;
	/*!
	 * \brief Time to wait for responses to APDU in ms.
	 * \note Set to 0 if send the message only.
	 * \note Set to APDU_TIMEOUT_MSGS_ONLY to "timeout" with the message_type list only.
	 */
	int timeout_time;
	/*! Number of Q.931 messages the APDU can "timeout" on. */
	unsigned num_messages;
	/*! Q.931 message list to "timeout" on. */
	int message_type[5];
	/*!
	 * \brief APDU callback function.
	 *
	 * \param reason Reason callback is called.
	 * \param ctrl D channel controller.
	 * \param call Q.931 call leg.
	 * \param apdu APDU queued entry.  Do not change!
	 * \param msg APDU response message data.  (NULL if was not the reason called.)
	 *
	 * \note
	 * A callback must be supplied if the sender cares about any APDU_CALLBACK_REASON.
	 *
	 * \return TRUE if no more responses are expected.
	 */
	int (*callback)(enum APDU_CALLBACK_REASON reason, struct pri *ctrl, struct q931_call *call, struct apdu_event *apdu, const struct apdu_msg_data *msg);
	/*! \brief Sender data for the callback function to identify the particular APDU. */
	union apdu_callback_param user;
};

struct apdu_event {
	/*! Linked list pointer */
	struct apdu_event *next;
	/*! TRUE if this APDU has been sent. */
	int sent;
	/*! What message to send the ADPU in */
	int message;
	/*! Sender supplied information to handle APDU response messages. */
	struct apdu_callback_data response;
	/*! Q.931 call leg.  (Needed for the APDU timeout.) */
	struct q931_call *call;
	/*! Response timeout timer. */
	int timer;
	/*! Length of ADPU */
	int apdu_len;
	/*! ADPU to send */
	unsigned char apdu[255];
};

void rose_copy_number_to_q931(struct pri *ctrl, struct q931_party_number *q931_number, const struct rosePartyNumber *rose_number);
void rose_copy_subaddress_to_q931(struct pri *ctrl, struct q931_party_subaddress *q931_subaddress, const struct rosePartySubaddress *rose_subaddress);
void rose_copy_address_to_q931(struct pri *ctrl, struct q931_party_address *q931_address, const struct roseAddress *rose_address);
void rose_copy_address_to_id_q931(struct pri *ctrl, struct q931_party_id *q931_address, const struct roseAddress *rose_address);
void rose_copy_presented_number_screened_to_q931(struct pri *ctrl, struct q931_party_number *q931_number, const struct rosePresentedNumberScreened *rose_presented);
void rose_copy_presented_number_unscreened_to_q931(struct pri *ctrl, struct q931_party_number *q931_number, const struct rosePresentedNumberUnscreened *rose_presented);
void rose_copy_presented_address_screened_to_id_q931(struct pri *ctrl, struct q931_party_id *q931_address, const struct rosePresentedAddressScreened *rose_presented);
void rose_copy_name_to_q931(struct pri *ctrl, struct q931_party_name *qsig_name, const struct roseQsigName *rose_name);

void q931_copy_number_to_rose(struct pri *ctrl, struct rosePartyNumber *rose_number, const struct q931_party_number *q931_number);
void q931_copy_subaddress_to_rose(struct pri *ctrl, struct rosePartySubaddress *rose_subaddress, const struct q931_party_subaddress *q931_subaddress);
void q931_copy_address_to_rose(struct pri *ctrl, struct roseAddress *rose_address, const struct q931_party_address *q931_address);
void q931_copy_id_address_to_rose(struct pri *ctrl, struct roseAddress *rose_address, const struct q931_party_id *q931_address);
void q931_copy_presented_number_screened_to_rose(struct pri *ctrl, struct rosePresentedNumberScreened *rose_presented, const struct q931_party_number *q931_number);
void q931_copy_presented_number_unscreened_to_rose(struct pri *ctrl, struct rosePresentedNumberUnscreened *rose_presented, const struct q931_party_number *q931_number);
void q931_copy_presented_id_address_screened_to_rose(struct pri *ctrl, struct rosePresentedAddressScreened *rose_presented, const struct q931_party_id *q931_address);
void q931_copy_name_to_rose(struct pri *ctrl, struct roseQsigName *rose_name, const struct q931_party_name *qsig_name);

int rose_error_msg_encode(struct pri *ctrl, q931_call *call, int msgtype, int invoke_id, enum rose_error_code code);
int send_facility_error(struct pri *ctrl, q931_call *call, int invoke_id, enum rose_error_code code);
int rose_result_ok_encode(struct pri *ctrl, q931_call *call, int msgtype, int invoke_id);
int send_facility_result_ok(struct pri *ctrl, q931_call *call, int invoke_id);

/* Queues an MWI apdu on a the given call */
int mwi_message_send(struct pri *pri, q931_call *call, struct pri_sr *req, int activate);

/* starts a 2BCT */
int eect_initiate_transfer(struct pri *pri, q931_call *c1, q931_call *c2);

int rlt_initiate_transfer(struct pri *pri, q931_call *c1, q931_call *c2);

/* starts a QSIG Path Replacement */
int anfpr_initiate_transfer(struct pri *pri, q931_call *c1, q931_call *c2);

int etsi_initiate_transfer(struct pri *ctrl, q931_call *call_1, q931_call *call_2);

int qsig_cf_callrerouting(struct pri *pri, q931_call *c, const char* dest, const char* original, const char* reason);
int send_reroute_request(struct pri *ctrl, q931_call *call, const struct q931_party_id *caller, const struct q931_party_redirecting *deflection, int subscription_option);

int send_call_transfer_complete(struct pri *pri, q931_call *call, int call_status);
int rose_request_subaddress_encode(struct pri *ctrl, struct q931_call *call);
int send_subaddress_transfer(struct pri *ctrl, struct q931_call *call);

int rose_diverting_leg_information1_encode(struct pri *pri, q931_call *call);
int rose_diverting_leg_information3_encode(struct pri *pri, q931_call *call, int messagetype);

int rose_connected_name_encode(struct pri *pri, q931_call *call, int messagetype);
int rose_called_name_encode(struct pri *pri, q931_call *call, int messagetype);

int pri_call_apdu_queue(q931_call *call, int messagetype, const unsigned char *apdu, int apdu_len, struct apdu_callback_data *response);
void pri_call_apdu_queue_cleanup(q931_call *call);
struct apdu_event *pri_call_apdu_find(struct q931_call *call, int invoke_id);
int pri_call_apdu_extract(struct q931_call *call, struct apdu_event *extract);
void pri_call_apdu_delete(struct q931_call *call, struct apdu_event *doomed);

/* Adds the "standard" APDUs to a call */
int pri_call_add_standard_apdus(struct pri *pri, q931_call *call);

void asn1_dump(struct pri *ctrl, const unsigned char *start_asn1, const unsigned char *end);

void rose_handle_invoke(struct pri *ctrl, q931_call *call, int msgtype, q931_ie *ie, const struct fac_extension_header *header, const struct rose_msg_invoke *invoke);
void rose_handle_result(struct pri *ctrl, q931_call *call, int msgtype, q931_ie *ie, const struct fac_extension_header *header, const struct rose_msg_result *result);
void rose_handle_error(struct pri *ctrl, q931_call *call, int msgtype, q931_ie *ie, const struct fac_extension_header *header, const struct rose_msg_error *error);
void rose_handle_reject(struct pri *ctrl, q931_call *call, int msgtype, q931_ie *ie, const struct fac_extension_header *header, const struct rose_msg_reject *reject);

int pri_cc_interrogate_rsp(struct pri *ctrl, q931_call *call, const struct rose_msg_invoke *invoke);
void pri_cc_ptmp_request(struct pri *ctrl, q931_call *call, const struct rose_msg_invoke *invoke);
void pri_cc_ptp_request(struct pri *ctrl, q931_call *call, int msgtype, const struct rose_msg_invoke *invoke);
void pri_cc_qsig_request(struct pri *ctrl, q931_call *call, int msgtype, const struct rose_msg_invoke *invoke);
void pri_cc_qsig_cancel(struct pri *ctrl, q931_call *call, int msgtype, const struct rose_msg_invoke *invoke);
void pri_cc_qsig_exec_possible(struct pri *ctrl, q931_call *call, int msgtype, const struct rose_msg_invoke *invoke);

int aoc_charging_request_send(struct pri *ctrl, q931_call *c, enum PRI_AOC_REQUEST aoc_request_flag);
void aoc_etsi_aoc_request(struct pri *ctrl, q931_call *call, const struct rose_msg_invoke *invoke);
void aoc_etsi_aoc_s_currency(struct pri *ctrl, const struct rose_msg_invoke *invoke);
void aoc_etsi_aoc_s_special_arrangement(struct pri *ctrl, const struct rose_msg_invoke *invoke);
void aoc_etsi_aoc_d_currency(struct pri *ctrl, const struct rose_msg_invoke *invoke);
void aoc_etsi_aoc_d_charging_unit(struct pri *ctrl, const struct rose_msg_invoke *invoke);
void aoc_etsi_aoc_e_currency(struct pri *ctrl, q931_call *call, const struct rose_msg_invoke *invoke);
void aoc_etsi_aoc_e_charging_unit(struct pri *ctrl, q931_call *call, const struct rose_msg_invoke *invoke);

#endif /* _PRI_FACILITY_H */
