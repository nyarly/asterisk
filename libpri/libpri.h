/*
 * libpri: An implementation of Primary Rate ISDN
 *
 * Written by Mark Spencer <markster@digium.com>
 *
 * Copyright (C) 2001, Digium, Inc.
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

/*
 * NOTE:
 * All new global identifiers that are added to this file MUST be
 * prefixed with PRI_ or pri_ to indicate that they are part of this
 * library and to reduce potential naming conflicts.
 */

#ifndef _LIBPRI_H
#define _LIBPRI_H

/* Node types */
#define PRI_NETWORK		1
#define PRI_CPE			2

/* Debugging */
#define PRI_DEBUG_Q921_RAW		(1 << 0)	/* Show raw HDLC frames */
#define PRI_DEBUG_Q921_DUMP		(1 << 1)	/* Show each interpreted Q.921 frame */
#define PRI_DEBUG_Q921_STATE 	(1 << 2)	/* Debug state machine changes */
#define PRI_DEBUG_CONFIG		(1 << 3) 	/* Display error events on stdout */
#define PRI_DEBUG_Q931_DUMP		(1 << 5)	/* Show interpreted Q.931 frames */
#define PRI_DEBUG_Q931_STATE	(1 << 6)	/* Debug Q.931 state machine changes */
#define	PRI_DEBUG_Q931_ANOMALY 	(1 << 7)	/* Show unexpected events */
#define PRI_DEBUG_APDU			(1 << 8)	/* Debug of APDU components such as ROSE */
#define PRI_DEBUG_AOC			(1 << 9)	/* Debug of Advice of Charge ROSE Messages */
#define PRI_DEBUG_CC			(1 << 10)	/* Debug call-completion. */

#define PRI_DEBUG_ALL			(0xffff)	/* Everything */

/* Switch types */
#define PRI_SWITCH_UNKNOWN 		0
#define PRI_SWITCH_NI2	   		1	/* National ISDN 2 */
#define PRI_SWITCH_DMS100		2	/* DMS 100 */
#define PRI_SWITCH_LUCENT5E		3	/* Lucent 5E */
#define PRI_SWITCH_ATT4ESS		4	/* AT&T 4ESS */
#define PRI_SWITCH_EUROISDN_E1		5	/* Standard EuroISDN (CTR4, ETSI 300-102) */
#define PRI_SWITCH_EUROISDN_T1		6	/* T1 EuroISDN variant (ETSI 300-102) */
#define PRI_SWITCH_NI1			7	/* National ISDN 1 */
#define PRI_SWITCH_GR303_EOC		8	/* GR-303 Embedded Operations Channel */
#define PRI_SWITCH_GR303_TMC		9	/* GR-303 Timeslot Management Channel */
#define PRI_SWITCH_QSIG			10	/* QSIG Switch */
/* Switchtypes 11 - 20 are reserved for internal use */


/* PRI D-Channel Events */
#define PRI_EVENT_DCHAN_UP		 1	/* D-channel is up */
#define PRI_EVENT_DCHAN_DOWN	 2	/* D-channel is down */
#define PRI_EVENT_RESTART		 3	/* B-channel is restarted */
#define PRI_EVENT_CONFIG_ERR 	 4	/* Configuration Error Detected */
#define PRI_EVENT_RING			 5	/* Incoming call (SETUP) */
#define PRI_EVENT_HANGUP		 6	/* Call got hung up (RELEASE/RELEASE_COMPLETE/other) */
#define PRI_EVENT_RINGING		 7	/* Call is ringing (ALERTING) */
#define PRI_EVENT_ANSWER		 8	/* Call has been answered (CONNECT) */
#define PRI_EVENT_HANGUP_ACK	 9	/* Call hangup has been acknowledged */
#define PRI_EVENT_RESTART_ACK	10	/* Restart complete on a given channel (RESTART_ACKNOWLEDGE) */
#define PRI_EVENT_FACNAME		11	/* Caller*ID Name received on Facility (DEPRECATED) */
#define PRI_EVENT_FACILITY		11	/* Facility received (FACILITY) */
#define PRI_EVENT_INFO_RECEIVED 12	/* Additional info (digits) received (INFORMATION) */
#define PRI_EVENT_PROCEEDING	13	/* When we get CALL_PROCEEDING */
#define PRI_EVENT_SETUP_ACK		14	/* When we get SETUP_ACKNOWLEDGE */
#define PRI_EVENT_HANGUP_REQ	15	/* Requesting the higher layer to hangup (DISCONNECT) */
#define PRI_EVENT_NOTIFY		16	/* Notification received (NOTIFY) */
#define PRI_EVENT_PROGRESS		17	/* When we get PROGRESS */
#define PRI_EVENT_KEYPAD_DIGIT	18	/* When we receive during ACTIVE state (INFORMATION) */
#define PRI_EVENT_SERVICE       19	/* SERVICE maintenance message */
#define PRI_EVENT_SERVICE_ACK   20	/* SERVICE maintenance acknowledgement message */
#define PRI_EVENT_HOLD			21	/* HOLD request received */
#define PRI_EVENT_HOLD_ACK		22	/* HOLD_ACKNOWLEDGE received */
#define PRI_EVENT_HOLD_REJ		23	/* HOLD_REJECT received */
#define PRI_EVENT_RETRIEVE		24	/* RETRIEVE request received */
#define PRI_EVENT_RETRIEVE_ACK	25	/* RETRIEVE_ACKNOWLEDGE received */
#define PRI_EVENT_RETRIEVE_REJ	26	/* RETRIEVE_REJECT received */
#define PRI_EVENT_CONNECT_ACK	27	/* CONNECT_ACKNOWLEDGE received */

/* Simple states */
#define PRI_STATE_DOWN		0
#define PRI_STATE_UP		1

#define PRI_PROGRESS_MASK

/* Progress indicator values */
#define PRI_PROG_CALL_NOT_E2E_ISDN						(1 << 0)
#define PRI_PROG_CALLED_NOT_ISDN						(1 << 1)
#define PRI_PROG_CALLER_NOT_ISDN						(1 << 2)
#define PRI_PROG_INBAND_AVAILABLE						(1 << 3)
#define PRI_PROG_DELAY_AT_INTERF						(1 << 4)
#define PRI_PROG_INTERWORKING_WITH_PUBLIC				(1 << 5)
#define PRI_PROG_INTERWORKING_NO_RELEASE				(1 << 6)
#define PRI_PROG_INTERWORKING_NO_RELEASE_PRE_ANSWER		(1 << 7)
#define PRI_PROG_INTERWORKING_NO_RELEASE_POST_ANSWER	(1 << 8)
#define PRI_PROG_CALLER_RETURNED_TO_ISDN					(1 << 9)

/* Numbering plan identifier */
#define PRI_NPI_UNKNOWN					0x0 /*!< Unknown numbering plan */
#define PRI_NPI_E163_E164				0x1 /*!< ISDN/telephony numbering plan (public) */
#define PRI_NPI_X121					0x3 /*!< Data numbering plan */
#define PRI_NPI_F69						0x4 /*!< Telex numbering plan */
#define PRI_NPI_NATIONAL				0x8 /*!< National standard numbering plan */
#define PRI_NPI_PRIVATE					0x9 /*!< Private numbering plan */
#define PRI_NPI_RESERVED				0xF /*!< Reserved for extension */

/* Type of number */
#define PRI_TON_UNKNOWN					0x0
#define PRI_TON_INTERNATIONAL			0x1
#define PRI_TON_NATIONAL				0x2
#define PRI_TON_NET_SPECIFIC			0x3
#define PRI_TON_SUBSCRIBER				0x4
#define PRI_TON_ABBREVIATED				0x6
#define PRI_TON_RESERVED				0x7

/* Redirection reasons */
#define PRI_REDIR_UNKNOWN				0x0
#define PRI_REDIR_FORWARD_ON_BUSY		0x1
#define PRI_REDIR_FORWARD_ON_NO_REPLY	0x2
#define PRI_REDIR_DEFLECTION			0x3
#define PRI_REDIR_DTE_OUT_OF_ORDER		0x9
#define PRI_REDIR_FORWARDED_BY_DTE		0xA
#define PRI_REDIR_UNCONDITIONAL			0xF

/* Dialing plan */
#define PRI_INTERNATIONAL_ISDN		0x11
#define PRI_NATIONAL_ISDN			0x21
#define PRI_LOCAL_ISDN				0x41
#define PRI_PRIVATE					0x49
#define PRI_UNKNOWN					0x0

/* Presentation */
#define PRI_PRES_NUMBER_TYPE				0x03
#define PRI_PRES_USER_NUMBER_UNSCREENED		0x00
#define PRI_PRES_USER_NUMBER_PASSED_SCREEN	0x01
#define PRI_PRES_USER_NUMBER_FAILED_SCREEN	0x02
#define PRI_PRES_NETWORK_NUMBER				0x03

#define PRI_PRES_RESTRICTION				0x60
#define PRI_PRES_ALLOWED					0x00
#define PRI_PRES_RESTRICTED					0x20
#define PRI_PRES_UNAVAILABLE				0x40
#define PRI_PRES_RESERVED					0x60

#define PRES_ALLOWED_USER_NUMBER_NOT_SCREENED \
	(PRI_PRES_ALLOWED | PRI_PRES_USER_NUMBER_UNSCREENED)

#define PRES_ALLOWED_USER_NUMBER_PASSED_SCREEN \
	(PRI_PRES_ALLOWED | PRI_PRES_USER_NUMBER_PASSED_SCREEN)

#define PRES_ALLOWED_USER_NUMBER_FAILED_SCREEN \
	(PRI_PRES_ALLOWED | PRI_PRES_USER_NUMBER_FAILED_SCREEN)

#define PRES_ALLOWED_NETWORK_NUMBER	\
	(PRI_PRES_ALLOWED | PRI_PRES_NETWORK_NUMBER)

#define PRES_PROHIB_USER_NUMBER_NOT_SCREENED \
	(PRI_PRES_RESTRICTED | PRI_PRES_USER_NUMBER_UNSCREENED)

#define PRES_PROHIB_USER_NUMBER_PASSED_SCREEN \
	(PRI_PRES_RESTRICTED | PRI_PRES_USER_NUMBER_PASSED_SCREEN)

#define PRES_PROHIB_USER_NUMBER_FAILED_SCREEN \
	(PRI_PRES_RESTRICTED | PRI_PRES_USER_NUMBER_FAILED_SCREEN)

#define PRES_PROHIB_NETWORK_NUMBER \
	(PRI_PRES_RESTRICTED | PRI_PRES_NETWORK_NUMBER)

#define PRES_NUMBER_NOT_AVAILABLE \
	(PRI_PRES_UNAVAILABLE | PRI_PRES_NETWORK_NUMBER)

/* Reverse Charging Indication */
#define PRI_REVERSECHARGE_NONE      -1
#define PRI_REVERSECHARGE_REQUESTED  1

/* Causes for disconnection (See Q.850) */
#define PRI_CAUSE_UNALLOCATED					1	/*!< Called number unassigned. */
#define PRI_CAUSE_NO_ROUTE_TRANSIT_NET			2	/* !Q.SIG */
#define PRI_CAUSE_NO_ROUTE_DESTINATION			3
#define PRI_CAUSE_CHANNEL_UNACCEPTABLE			6
#define PRI_CAUSE_CALL_AWARDED_DELIVERED		7	/* !Q.SIG */
#define PRI_CAUSE_NORMAL_CLEARING				16
#define PRI_CAUSE_USER_BUSY						17
#define PRI_CAUSE_NO_USER_RESPONSE				18
#define PRI_CAUSE_NO_ANSWER						19
#define PRI_CAUSE_CALL_REJECTED					21
#define PRI_CAUSE_NUMBER_CHANGED				22
#define PRI_CAUSE_NONSELECTED_USER_CLEARING		26
#define PRI_CAUSE_DESTINATION_OUT_OF_ORDER		27
#define PRI_CAUSE_INVALID_NUMBER_FORMAT			28
#define PRI_CAUSE_FACILITY_REJECTED				29	/* !Q.SIG */
#define PRI_CAUSE_RESPONSE_TO_STATUS_ENQUIRY	30
#define PRI_CAUSE_NORMAL_UNSPECIFIED			31
#define PRI_CAUSE_NORMAL_CIRCUIT_CONGESTION		34
#define PRI_CAUSE_NETWORK_OUT_OF_ORDER			38	/* !Q.SIG */
#define PRI_CAUSE_NORMAL_TEMPORARY_FAILURE		41
#define PRI_CAUSE_SWITCH_CONGESTION				42	/* !Q.SIG */
#define PRI_CAUSE_ACCESS_INFO_DISCARDED			43	/* !Q.SIG */
#define PRI_CAUSE_REQUESTED_CHAN_UNAVAIL		44
#define PRI_CAUSE_PRE_EMPTED					45	/* !Q.SIG */
#define PRI_CAUSE_RESOURCE_UNAVAIL_UNSPECIFIED	47
#define PRI_CAUSE_FACILITY_NOT_SUBSCRIBED  		50	/* !Q.SIG */
#define PRI_CAUSE_OUTGOING_CALL_BARRED     		52	/* !Q.SIG */
#define PRI_CAUSE_INCOMING_CALL_BARRED     		54	/* !Q.SIG */
#define PRI_CAUSE_BEARERCAPABILITY_NOTAUTH		57
#define PRI_CAUSE_BEARERCAPABILITY_NOTAVAIL     58
#define PRI_CAUSE_SERVICEOROPTION_NOTAVAIL		63	/* Q.SIG */
#define PRI_CAUSE_BEARERCAPABILITY_NOTIMPL		65
#define PRI_CAUSE_CHAN_NOT_IMPLEMENTED     		66	/* !Q.SIG */
#define PRI_CAUSE_FACILITY_NOT_IMPLEMENTED      69	/* !Q.SIG */
#define PRI_CAUSE_INVALID_CALL_REFERENCE		81
#define PRI_CAUSE_IDENTIFIED_CHANNEL_NOTEXIST	82	/* Q.SIG */
#define PRI_CAUSE_INCOMPATIBLE_DESTINATION		88
#define PRI_CAUSE_INVALID_MSG_UNSPECIFIED  		95	/* !Q.SIG */
#define PRI_CAUSE_MANDATORY_IE_MISSING			96
#define PRI_CAUSE_MESSAGE_TYPE_NONEXIST			97
#define PRI_CAUSE_WRONG_MESSAGE					98
#define PRI_CAUSE_IE_NONEXIST					99
#define PRI_CAUSE_INVALID_IE_CONTENTS			100
#define PRI_CAUSE_WRONG_CALL_STATE				101
#define PRI_CAUSE_RECOVERY_ON_TIMER_EXPIRE		102
#define PRI_CAUSE_MANDATORY_IE_LENGTH_ERROR		103	/* !Q.SIG */
#define PRI_CAUSE_PROTOCOL_ERROR				111
#define PRI_CAUSE_INTERWORKING					127	/* !Q.SIG */

/* Transmit capabilities */
#define PRI_TRANS_CAP_SPEECH					0x0
#define PRI_TRANS_CAP_DIGITAL					0x08
#define PRI_TRANS_CAP_RESTRICTED_DIGITAL		0x09
#define PRI_TRANS_CAP_3_1K_AUDIO				0x10
#define PRI_TRANS_CAP_7K_AUDIO					0x11	/* Depriciated ITU Q.931 (05/1998)*/
#define PRI_TRANS_CAP_DIGITAL_W_TONES			0x11
#define PRI_TRANS_CAP_VIDEO						0x18

#define PRI_LAYER_1_ITU_RATE_ADAPT	0x21
#define PRI_LAYER_1_ULAW			0x22
#define PRI_LAYER_1_ALAW			0x23
#define PRI_LAYER_1_G721			0x24
#define PRI_LAYER_1_G722_G725		0x25
#define PRI_LAYER_1_H223_H245		0x26
#define PRI_LAYER_1_NON_ITU_ADAPT	0x27
#define PRI_LAYER_1_V120_RATE_ADAPT	0x28
#define PRI_LAYER_1_X31_RATE_ADAPT	0x29


/* Intermediate rates for V.110 */
#define PRI_INT_RATE_8K			1
#define PRI_INT_RATE_16K		2
#define PRI_INT_RATE_32K		3


/* Rate adaption for bottom 5 bits of rateadaption */
#define PRI_RATE_USER_RATE_MASK		0x1F
#define PRI_RATE_ADAPT_UNSPEC		0x00
#define PRI_RATE_ADAPT_0K6		0x01
#define PRI_RATE_ADAPT_1K2		0x02
#define PRI_RATE_ADAPT_2K4		0x03
#define PRI_RATE_ADAPT_3K6		0x04
#define PRI_RATE_ADAPT_4K8		0x05
#define PRI_RATE_ADAPT_7K2		0x06
#define PRI_RATE_ADAPT_8K		0x07
#define PRI_RATE_ADAPT_9K6		0x08
#define PRI_RATE_ADAPT_14K4		0x09
#define PRI_RATE_ADAPT_16K		0x0A
#define PRI_RATE_ADAPT_19K2		0x0B
#define PRI_RATE_ADAPT_32K		0x0C
#define PRI_RATE_ADAPT_38K4		0x0D
#define PRI_RATE_ADAPT_48K		0x0E
#define PRI_RATE_ADAPT_56K		0x0F
#define PRI_RATE_ADAPT_57K6		0x12
#define PRI_RATE_ADAPT_28K8		0x13
#define PRI_RATE_ADAPT_24K		0x14
#define PRI_RATE_ADAPT_0K1345		0x15
#define PRI_RATE_ADAPT_0K1		0x16
#define PRI_RATE_ADAPT_0K075_1K2	0x17
#define PRI_RATE_ADAPT_1K2_0K075	0x18
#define PRI_RATE_ADAPT_0K05		0x19
#define PRI_RATE_ADAPT_0K075		0x1A
#define PRI_RATE_ADAPT_0K110		0x1B
#define PRI_RATE_ADAPT_0K150		0x1C
#define PRI_RATE_ADAPT_0K200		0x1D
#define PRI_RATE_ADAPT_0K300		0x1E
#define PRI_RATE_ADAPT_12K		0x1F

/* in-band negotiation flag for rateadaption bit 5 */
#define PRI_RATE_ADAPT_NEGOTIATION_POSS	0x20

/* async flag for rateadaption bit 6 */
#define PRI_RATE_ADAPT_ASYNC		0x40

/* Notifications */
#define PRI_NOTIFY_USER_SUSPENDED		0x00	/* User suspended (Q.931) (Call is placed on hold) */
#define PRI_NOTIFY_USER_RESUMED			0x01	/* User resumed (Q.931) (Call is taken off hold) */
#define PRI_NOTIFY_BEARER_CHANGE		0x02	/* Bearer service change (DSS1) */
#define PRI_NOTIFY_ASN1_COMPONENT		0x03	/* ASN.1 encoded component (DSS1) */
#define PRI_NOTIFY_COMPLETION_DELAY		0x04	/* Call completion delay */
#define PRI_NOTIFY_CONF_ESTABLISHED		0x42	/* Conference established */
#define PRI_NOTIFY_CONF_DISCONNECTED		0x43	/* Conference disconnected */
#define PRI_NOTIFY_CONF_PARTY_ADDED		0x44	/* Other party added */
#define PRI_NOTIFY_CONF_ISOLATED		0x45	/* Isolated */
#define PRI_NOTIFY_CONF_REATTACHED		0x46	/* Reattached */
#define PRI_NOTIFY_CONF_OTHER_ISOLATED		0x47	/* Other party isolated */
#define PRI_NOTIFY_CONF_OTHER_REATTACHED	0x48	/* Other party reattached */
#define PRI_NOTIFY_CONF_OTHER_SPLIT		0x49	/* Other party split */
#define PRI_NOTIFY_CONF_OTHER_DISCONNECTED	0x4a	/* Other party disconnected */
#define PRI_NOTIFY_CONF_FLOATING		0x4b	/* Conference floating */
#define PRI_NOTIFY_WAITING_CALL			0x60	/* Call is waiting call */
#define PRI_NOTIFY_DIVERSION_ACTIVATED		0x68	/* Diversion activated (DSS1) (cfu, cfb, cfnr) (EN 300 207-1 Section 7.2.1) */
#define PRI_NOTIFY_TRANSFER_ALERTING		0x69	/* Call transfer, alerting (EN 300 369-1 Section 7.2) */
#define PRI_NOTIFY_TRANSFER_ACTIVE		0x6a	/* Call transfer, active(answered) (EN 300 369-1 Section 7.2) */
#define PRI_NOTIFY_REMOTE_HOLD			0x79	/* Remote hold */
#define PRI_NOTIFY_REMOTE_RETRIEVAL		0x7a	/* Remote retrieval */
#define PRI_NOTIFY_CALL_DIVERTING		0x7b	/* Call is diverting (EN 300 207-1 Section 7.2.1) */

#define PRI_COPY_DIGITS_CALLED_NUMBER

/* Network Specific Facilities (AT&T) */
#define PRI_NSF_NONE                   -1
#define PRI_NSF_SID_PREFERRED          0xB1
#define PRI_NSF_ANI_PREFERRED          0xB2
#define PRI_NSF_SID_ONLY               0xB3
#define PRI_NSF_ANI_ONLY               0xB4
#define PRI_NSF_CALL_ASSOC_TSC         0xB9
#define PRI_NSF_NOTIF_CATSC_CLEARING   0xBA
#define PRI_NSF_OPERATOR               0xB5
#define PRI_NSF_PCCO                   0xB6
#define PRI_NSF_SDN                    0xE1
#define PRI_NSF_TOLL_FREE_MEGACOM      0xE2
#define PRI_NSF_MEGACOM                        0xE3
#define PRI_NSF_ACCUNET                        0xE6
#define PRI_NSF_LONG_DISTANCE_SERVICE  0xE7
#define PRI_NSF_INTERNATIONAL_TOLL_FREE        0xE8
#define PRI_NSF_ATT_MULTIQUEST         0xF0
#define PRI_NSF_CALL_REDIRECTION_SERVICE       0xF7

typedef struct q931_call q931_call;

/* Name character set enumeration values */
#define PRI_CHAR_SET_UNKNOWN				0
#define PRI_CHAR_SET_ISO8859_1				1
#define PRI_CHAR_SET_WITHDRAWN				2
#define PRI_CHAR_SET_ISO8859_2				3
#define PRI_CHAR_SET_ISO8859_3				4
#define PRI_CHAR_SET_ISO8859_4				5
#define PRI_CHAR_SET_ISO8859_5				6
#define PRI_CHAR_SET_ISO8859_7				7
#define PRI_CHAR_SET_ISO10646_BMPSTRING		8
#define PRI_CHAR_SET_ISO10646_UTF_8STRING	9

/*! \brief Q.SIG name information. */
struct pri_party_name {
	/*! \brief TRUE if the name information is valid/present */
	int valid;
	/*!
	 * \brief Q.931 presentation-indicator encoded field
	 * \note Must tollerate the Q.931 screening-indicator field values being present.
	 */
	int presentation;
	/*!
	 * \brief Character set the name is using.
	 * \details
	 * unknown(0),
	 * iso8859-1(1),
	 * enum-value-withdrawn-by-ITU-T(2)
	 * iso8859-2(3),
	 * iso8859-3(4),
	 * iso8859-4(5),
	 * iso8859-5(6),
	 * iso8859-7(7),
	 * iso10646-BmpString(8),
	 * iso10646-utf-8String(9)
	 * \details
	 * Set to iso8859-1(1) if unsure what to use.
	 */
	int char_set;
	/*! \brief Name data with null terminator. */
	char str[64];
};

struct pri_party_number {
	/*! \brief TRUE if the number information is valid/present */
	int valid;
	/*! \brief Q.931 presentation-indicator and screening-indicator encoded fields */
	int presentation;
	/*! \brief Q.931 Type-Of-Number and numbering-plan encoded fields */
	int plan;
	/*! \brief Number data with null terminator. */
	char str[64];
};

/*!
 * \note This structure is a place holder for possible future subaddress support
 * to maintain ABI compatibility.
 */
struct pri_party_subaddress {
	/*! \brief TRUE if the subaddress information is valid/present */
	int valid;
	/*!
	 * \brief Subaddress type.
	 * \details
	 * nsap(0),
	 * user_specified(2)
	 */
	int type;
	/*!
	 * \brief TRUE if odd number of address signals
	 * \note The odd/even indicator is used when the type of subaddress is
	 * user_specified and the coding is BCD.
	 */
	int odd_even_indicator;
	/*! \brief Length of the subaddress data */
	int length;
	/*!
	 * \brief Subaddress data with null terminator.
	 * \note The null terminator is a convenience only since the data could be
	 * BCD/binary and thus have a null byte as part of the contents.
	 */
	unsigned char data[32];
};

/*! \brief Addressing information needed to identify an endpoint in a call. */
struct pri_party_address {
	/*! \brief Subscriber phone number */
	struct pri_party_number number;
	/*! \brief Subscriber subaddress */
	struct pri_party_subaddress subaddress;
};

/*! \brief Information needed to identify an endpoint in a call. */
struct pri_party_id {
	/*! \brief Subscriber name */
	struct pri_party_name name;
	/*! \brief Subscriber phone number */
	struct pri_party_number number;
	/*! \brief Subscriber subaddress */
	struct pri_party_subaddress subaddress;
};

/*! \brief Connected Line/Party information */
struct pri_party_connected_line {
	/*! Connected party ID */
	struct pri_party_id id;
};

/*!
 * \brief Redirecting Line information.
 * \details
 * RDNIS (Redirecting Directory Number Information Service)
 * Where a call diversion or transfer was invoked.
 */
struct pri_party_redirecting {
	/*! Who is redirecting the call (Sent to the party the call is redirected toward) */
	struct pri_party_id from;
	/*! Call is redirecting to a new party (Sent to the caller) */
	struct pri_party_id to;
	/*! Originally called party (in cases of multiple redirects) */
	struct pri_party_id orig_called;
	/*! Number of times the call was redirected */
	int count;
	/*! Original reason for redirect (in cases of multiple redirects) */
	int orig_reason;
	/*! Redirection reason */
	int reason;
};

/*!
 * \brief Information for rerouting/deflecting the call.
 */
struct pri_rerouting_data {
	/*!
	 * \brief Updated caller-id information.
	 * \note The information may have been altered by procedure in the private network.
	 */
	struct pri_party_id caller;
	/*!
	 * \note
	 * deflection.to is the new called number and must always be present.
	 */
	struct pri_party_redirecting deflection;
	/*!
	 * \brief Diverting user subscription option to specify if caller is notified.
	 * \details
	 * noNotification(0),
	 * notificationWithoutDivertedToNr(1),
	 * notificationWithDivertedToNr(2),
	 * notApplicable(3) (Status only.)
	 */
	int subscription_option;
	/*! Invocation ID to use when sending a reply to the call rerouting/deflection request. */
	int invoke_id;
};

/*
 * NOTE:
 * The code surrounded by STATUS_REQUEST_PLACE_HOLDER is not implemented.
 * The STATUS_REQUEST_PLACE_HOLDER code will be made unconditional if support
 * for the messages is ever needed (and gets written).
 */

/* Subcommands derived from supplementary services. */
#define PRI_SUBCMD_REDIRECTING				1	/*!< Redirecting information update */
#define PRI_SUBCMD_CONNECTED_LINE			2	/*!< Connected line information update */
#define PRI_SUBCMD_REROUTING				3	/*!< CallRerouting/CallDeflection received. */
#if defined(STATUS_REQUEST_PLACE_HOLDER)
#define PRI_SUBCMD_STATUS_REQ				4	/*!< Determine the status of the given party. */
#define PRI_SUBCMD_STATUS_REQ_RSP			5	/*!< Status request response */
#endif	/* defined(STATUS_REQUEST_PLACE_HOLDER) */
#define PRI_SUBCMD_CC_AVAILABLE				6	/*!< Indicate that CC is available */
#define PRI_SUBCMD_CC_REQ					7	/*!< CC activation request */
#define PRI_SUBCMD_CC_REQ_RSP				8	/*!< CC activation request response */
#define PRI_SUBCMD_CC_REMOTE_USER_FREE		9	/*!< Indicate that CC party B is available, party A is considered free. */
#define PRI_SUBCMD_CC_B_FREE				10	/*!< Indicate that CC party B is available, party A is considered busy. */
#define PRI_SUBCMD_CC_STATUS_REQ			11	/*!< Request/prod to receive updates of CC party A status */
#define PRI_SUBCMD_CC_STATUS_REQ_RSP		12	/*!< Requested update of CC party A status */
#define PRI_SUBCMD_CC_STATUS				13	/*!< Unsolicited update of CC party A status */
#define PRI_SUBCMD_CC_CALL					14	/*!< Indicate that this call is a CC callback */
#define PRI_SUBCMD_CC_CANCEL				15	/*!< Unsolicited indication that CC is canceled */
#define PRI_SUBCMD_CC_STOP_ALERTING			16	/*!< Indicate that someone else has responed to remote user free */
#define PRI_SUBCMD_TRANSFER_CALL			17	/*!< Request to transfer the specified calls together. */
#define PRI_SUBCMD_AOC_S					18	/*!< Advice Of Charge Start information (Rate list) */
#define PRI_SUBCMD_AOC_D					19	/*!< Advice Of Charge During information */
#define PRI_SUBCMD_AOC_E					20	/*!< Advice Of Charge End information */
#define PRI_SUBCMD_AOC_CHARGING_REQ			21	/*!< Advice Of Charge Request information */
#define PRI_SUBCMD_AOC_CHARGING_REQ_RSP		22	/*!< Advice Of Charge Request Response information */
#define PRI_SUBCMD_MCID_REQ					23	/*!< Malicious Call ID Request */
#define PRI_SUBCMD_MCID_RSP					24	/*!< Malicious Call ID Request response */
#define PRI_SUBCMD_DISPLAY_TEXT				25	/*!< Received display ie text */

#if defined(STATUS_REQUEST_PLACE_HOLDER)
struct pri_subcmd_status_request {
	/*!
	 * \brief Invoke id in case there are multiple outstanding requests.
	 * \note Used to match any responses with the original invoke in case
	 * there are several requests active.
	 */
	int invoke_id;
	/*! \brief Party address requesting status about. */
	struct pri_party_address party;
};
#endif	/* defined(STATUS_REQUEST_PLACE_HOLDER) */

#if defined(STATUS_REQUEST_PLACE_HOLDER)
struct pri_subcmd_status_request_rsp {
	/*!
	 * \brief Request id in case there are multiple outstanding requests.
	 * \note Used to match any responses with the request in case there
	 * are several requests active.
	 */
	int request_id;
	/*!
	 * \brief Response status to the status request.
	 * \details
	 * free(0),
	 * busy(1),
	 * incompatible(2)
	 * timeout(3),
	 */
	int status;
};
#endif	/* defined(STATUS_REQUEST_PLACE_HOLDER) */

struct pri_subcmd_cc_id {
	/*! \brief Call-Completion record id */
	long cc_id;
};

struct pri_subcmd_cc_request {
	/*! \brief Call-Completion record id */
	long cc_id;
	/*!
	 * \brief Mode of call-completion requested.
	 * \details
	 * ccbs(0),
	 * ccnr(1)
	 */
	int mode;
};

struct pri_subcmd_cc_request_rsp {
	/*! \brief Call-Completion record id */
	long cc_id;
	/*!
	 * \brief Status of the requested call-completion activation.
	 * \details
	 * success(0),
	 * timeout(1),
	 * error(2),
	 * reject(3)
	 */
	int status;
	/*!
	 * \brief Failure code that can be converted to a string to further
	 * explain the non-timeout failure.
	 * \note Valid when status is error or reject.
	 * \note Use pri_facility_error2str() to convert the error_code.
	 * \note Use pri_facility_reject2str() to convert the reject_code.
	 */
	int fail_code;
};

struct pri_subcmd_cc_status {
	/*! \brief Call-Completion record id */
	long cc_id;
	/*!
	 * \brief Party A status.
	 * \details
	 * free(0),
	 * busy(1)
	 */
	int status;
};

struct pri_subcmd_cc_cancel {
	/*! \brief Call-Completion record id */
	long cc_id;
	/*!
	 * \brief TRUE if the cc_id is for an agent.
	 * \note This is a convenience value so the upper layer can know which
	 * list it should search for the cc_id.
	 */
	int is_agent;
};

struct pri_subcmd_transfer {
	/*! \brief Opaque call pointer for transfer with other call. */
	q931_call *call_1;
	/*! \brief Opaque call pointer for transfer with other call. */
	q931_call *call_2;
	/*! TRUE if call_1 is on hold. */
	int is_call_1_held;
	/*! TRUE if call_2 is on hold. */
	int is_call_2_held;
	/*! Invocation ID to use when sending a reply to the transfer request. */
	int invoke_id;
};

/*! \brief What is being charged. */
enum PRI_AOC_CHARGED_ITEM {
	PRI_AOC_CHARGED_ITEM_NOT_AVAILABLE,
	PRI_AOC_CHARGED_ITEM_SPECIAL_ARRANGEMENT,
	PRI_AOC_CHARGED_ITEM_BASIC_COMMUNICATION,
	PRI_AOC_CHARGED_ITEM_CALL_ATTEMPT,
	PRI_AOC_CHARGED_ITEM_CALL_SETUP,
	PRI_AOC_CHARGED_ITEM_USER_USER_INFO,
	PRI_AOC_CHARGED_ITEM_SUPPLEMENTARY_SERVICE,
};

/*! \brief Rate method being used. */
enum PRI_AOC_RATE_TYPE {
	PRI_AOC_RATE_TYPE_NOT_AVAILABLE,
	PRI_AOC_RATE_TYPE_FREE,
	PRI_AOC_RATE_TYPE_FREE_FROM_BEGINNING,
	PRI_AOC_RATE_TYPE_DURATION,
	PRI_AOC_RATE_TYPE_FLAT,
	PRI_AOC_RATE_TYPE_VOLUME,
	PRI_AOC_RATE_TYPE_SPECIAL_CODE,
};

enum PRI_AOC_TIME_SCALE {
	PRI_AOC_TIME_SCALE_HUNDREDTH_SECOND,
	PRI_AOC_TIME_SCALE_TENTH_SECOND,
	PRI_AOC_TIME_SCALE_SECOND,
	PRI_AOC_TIME_SCALE_TEN_SECOND,
	PRI_AOC_TIME_SCALE_MINUTE,
	PRI_AOC_TIME_SCALE_HOUR,
	PRI_AOC_TIME_SCALE_DAY,
};

struct pri_aoc_time {
	/*! LengthOfTimeUnit (Not valid if length is zero.) */
	long length;
	/*! \see enum PRI_AOC_TIME_SCALE */
	int scale;
};

enum PRI_AOC_MULTIPLIER {
	PRI_AOC_MULTIPLIER_THOUSANDTH,
	PRI_AOC_MULTIPLIER_HUNDREDTH,
	PRI_AOC_MULTIPLIER_TENTH,
	PRI_AOC_MULTIPLIER_ONE,
	PRI_AOC_MULTIPLIER_TEN,
	PRI_AOC_MULTIPLIER_HUNDRED,
	PRI_AOC_MULTIPLIER_THOUSAND,
};

struct pri_aoc_amount {
	long cost;
	/*! \see enum PRI_AOC_MULTIPLIER */
	int multiplier;
};

struct pri_aoc_duration {
	struct pri_aoc_amount amount;
	struct pri_aoc_time time;
	/*! Not present if the granularity time is zero. */
	struct pri_aoc_time granularity;
	/*!
	 * \brief Charging interval type
	 * \details
	 * continuousCharging(0),
	 * stepFunction(1)
	 */
	int charging_type;
	/*! Name of currency involved.  Null terminated. */
	char currency[10 + 1];
};

struct pri_aoc_flat {
	struct pri_aoc_amount amount;
	/*! Name of currency involved.  Null terminated. */
	char currency[10 + 1];
};

enum PRI_AOC_VOLUME_UNIT {
	PRI_AOC_VOLUME_UNIT_OCTET,
	PRI_AOC_VOLUME_UNIT_SEGMENT,
	PRI_AOC_VOLUME_UNIT_MESSAGE,
};

struct pri_aoc_volume {
	struct pri_aoc_amount amount;
	/*! \see enum PRI_AOC_VOLUME_UNIT */
	int unit;
	/*! Name of currency involved.  Null terminated. */
	char currency[10 + 1];
};

struct pri_aoc_s_element {
	/*!
	 * \brief What is being charged.
	 * \see enum PRI_AOC_CHARGED_ITEM
	 */
	int chargeable;
	/*!
	 * \brief Rate method being used.
	 * \see enum PRI_AOC_RATE_TYPE
	 */
	int rate_type;
	/*! \brief Charge rate being applied. */
	union {
		struct pri_aoc_duration duration;
		struct pri_aoc_flat flat;
		struct pri_aoc_volume volume;
		int special;
	} rate;
};

struct pri_subcmd_aoc_s {
	/*!
	 * \brief Number of items in the rate list.
	 * \note If the list is empty then the charging information is not available.
	 */
	int num_items;
	struct pri_aoc_s_element item[10];
};

enum PRI_AOC_DE_CHARGE {
	PRI_AOC_DE_CHARGE_NOT_AVAILABLE,
	PRI_AOC_DE_CHARGE_FREE,
	PRI_AOC_DE_CHARGE_CURRENCY,
	PRI_AOC_DE_CHARGE_UNITS,
};

struct pri_aoc_recorded_currency {
	struct pri_aoc_amount amount;
	/*! Name of currency involved.  Null terminated. */
	char currency[10 + 1];
};

struct pri_aoc_units_element {
	/*! Number of units recorded. -1 if not available. */
	long number;
	/*! Type of unit recorded. -1 if not available. */
	int type;
};

struct pri_aoc_recorded_units {
	int num_items;
	struct pri_aoc_units_element item[32];
};

enum PRI_AOC_D_BILLING_ID {
	PRI_AOC_D_BILLING_ID_NOT_AVAILABLE,
	PRI_AOC_D_BILLING_ID_NORMAL,
	PRI_AOC_D_BILLING_ID_REVERSE,
	PRI_AOC_D_BILLING_ID_CREDIT_CARD,
};
struct pri_subcmd_aoc_d {
	/*!
	 * \brief What is being charged.
	 * \see enum PRI_AOC_DE_CHARGE
	 */
	int charge;
	/*!
	 * \brief Billing accumulation
	 * \details
	 * subTotal(0),
	 * total(1)
	 */
	int billing_accumulation;
	/*! \see enum PRI_AOC_D_BILLING_ID */
	int billing_id;
	union {
		/*! Recorded currency */
		struct pri_aoc_recorded_currency money;
		/*! Recorded units list */
		struct pri_aoc_recorded_units unit;
	} recorded;
};

enum PRI_AOC_E_BILLING_ID {
	PRI_AOC_E_BILLING_ID_NOT_AVAILABLE,
	PRI_AOC_E_BILLING_ID_NORMAL,
	PRI_AOC_E_BILLING_ID_REVERSE,
	PRI_AOC_E_BILLING_ID_CREDIT_CARD,
	PRI_AOC_E_BILLING_ID_CALL_FORWARDING_UNCONDITIONAL,
	PRI_AOC_E_BILLING_ID_CALL_FORWARDING_BUSY,
	PRI_AOC_E_BILLING_ID_CALL_FORWARDING_NO_REPLY,
	PRI_AOC_E_BILLING_ID_CALL_DEFLECTION,
	PRI_AOC_E_BILLING_ID_CALL_TRANSFER,
};

enum PRI_AOC_E_CHARGING_ASSOCIATION {
	PRI_AOC_E_CHARGING_ASSOCIATION_NOT_AVAILABLE,
	PRI_AOC_E_CHARGING_ASSOCIATION_NUMBER,
	PRI_AOC_E_CHARGING_ASSOCIATION_ID,
};

struct pri_aoc_e_charging_association {
	union {
		/*! Charged number */
		struct pri_party_number number;
		/*! Charge identifier */
		int id;
	} charge;
	/*! \see enum PRI_AOC_E_CHARGING_ASSOCIATION */
	int charging_type;
};

struct pri_subcmd_aoc_e {
	/*!
	 * \brief What is being charged.
	 * \see enum PRI_AOC_DE_CHARGE
	 */
	int charge;
	/*! \see enum PRI_AOC_E_BILLING_ID */
	int billing_id;
	union {
		/*! Recorded currency */
		struct pri_aoc_recorded_currency money;
		/*! Recorded units list */
		struct pri_aoc_recorded_units unit;
	} recorded;
	/*! Charging association. */
	struct pri_aoc_e_charging_association associated;
};

enum PRI_AOC_REQ_RSP {
	/* Error Results */
	PRI_AOC_REQ_RSP_ERROR_NOT_IMPLEMENTED,
	PRI_AOC_REQ_RSP_ERROR_NOT_AVAILABLE,
	PRI_AOC_REQ_RSP_ERROR_TIMEOUT,
	PRI_AOC_REQ_RSP_ERROR_REJECT,
	/* generic error result all other errors are lumped into */
	PRI_AOC_REQ_RSP_ERROR,

	/* AOC Results */
	PRI_AOC_REQ_RSP_CHARGING_INFO_FOLLOWS,
	PRI_AOC_REQ_RSP_CURRENCY_INFO_LIST,
	PRI_AOC_REQ_RSP_SPECIAL_ARR,
};

enum PRI_AOC_REQUEST {
	PRI_AOC_REQUEST_S = (1 << 0),
	PRI_AOC_REQUEST_D = (1 << 1),
	PRI_AOC_REQUEST_E = (1 << 2),
};

struct pri_subcmd_aoc_request_response {
	/*!
	 * \brief aoc_s data from response
	 */
	struct pri_subcmd_aoc_s aoc_s;

	/*!
	 * \brief if the aoc_s msg is present, this will be set
	 */
	int valid_aoc_s;

	/*!
	 * \brief What type of aoc was requested.
	 * \see enum PRI_AOC_REQUEST
	 */
	int charging_request;

	/*!
	 * \brief response to the charging_request
	 * \see enum PRI_AOC_REQ_RSP
	 */
	int charging_response;
};

struct pri_subcmd_aoc_request {
	/*!
	 * \brief What types of aoc are being requested.
	 * \see enum PRI_AOC_REQUEST
	 */
	int charging_request;

	/*!
	 * \brief Value given by the initiating request.
	 */
	 int invoke_id;
};

struct pri_subcmd_mcid_req {
	/*!
	 * \brief Information libpri knows about the malicious caller.
	 * \note For the convenience of the upper layer.  This information
	 * may be incomplete if the upper layer redacted some caller
	 * information because it was restricted.
	 */
	struct pri_party_id originator;
	/*! \brief Information libpri knows about the callee. */
	struct pri_party_id answerer;
};

struct pri_subcmd_mcid_rsp {
	/*!
	 * \brief MCID request response status.
	 * \details
	 * success(0),
	 * timeout(1),
	 * error(2),
	 * reject(3)
	 */
	int status;
	/*!
	 * \brief Failure code that can be converted to a string to further
	 * explain the non-timeout failure.
	 * \note Valid when status is error or reject.
	 * \note Use pri_facility_error2str() to convert the error_code.
	 * \note Use pri_facility_reject2str() to convert the reject_code.
	 */
	int fail_code;
};

struct pri_subcmd_display_txt {
	/*!
	 * \brief Character set the text is using.
	 * \details
	 * unknown(0),
	 * iso8859-1(1),
	 * enum-value-withdrawn-by-ITU-T(2)
	 * iso8859-2(3),
	 * iso8859-3(4),
	 * iso8859-4(5),
	 * iso8859-5(6),
	 * iso8859-7(7),
	 * iso10646-BmpString(8),
	 * iso10646-utf-8String(9)
	 */
	int char_set;
	/*!
	 * \brief Number of octets in the display message.
	 * \note Not including any added null terminator.
	 */
	int length;
	/*!
	 * \brief Display text data.
	 * \note Null terminated on receive.
	 * \note Does not need to be null terminated on send.
	 */
	char text[128];
};

struct pri_subcommand {
	/*! PRI_SUBCMD_xxx defined values */
	int cmd;
	union {
		/*! Reserve room for possible expansion to maintain ABI compatibility. */
		char reserve_space[512];
		struct pri_party_connected_line connected_line;
		struct pri_party_redirecting redirecting;
		struct pri_rerouting_data rerouting;
#if defined(STATUS_REQUEST_PLACE_HOLDER)
		struct pri_subcmd_status_request status_request;
		struct pri_subcmd_status_request_rsp status_request_rsp;
#endif	/* defined(STATUS_REQUEST_PLACE_HOLDER) */
		struct pri_subcmd_cc_id cc_available;
		struct pri_subcmd_cc_request cc_request;
		struct pri_subcmd_cc_request_rsp cc_request_rsp;
		struct pri_subcmd_cc_id cc_remote_user_free;
		struct pri_subcmd_cc_id cc_b_free;
		struct pri_subcmd_cc_id cc_stop_alerting;
		struct pri_subcmd_cc_id cc_status_req;
		struct pri_subcmd_cc_status cc_status_req_rsp;
		struct pri_subcmd_cc_status cc_status;
		struct pri_subcmd_cc_id cc_call;
		struct pri_subcmd_cc_cancel cc_cancel;
		struct pri_subcmd_transfer transfer;
		struct pri_subcmd_aoc_request aoc_request;
		struct pri_subcmd_aoc_request_response aoc_request_response;
		struct pri_subcmd_aoc_s aoc_s;
		struct pri_subcmd_aoc_d aoc_d;
		struct pri_subcmd_aoc_e aoc_e;
		struct pri_subcmd_mcid_req mcid_req;
		struct pri_subcmd_mcid_rsp mcid_rsp;
		struct pri_subcmd_display_txt display;
	} u;
};

/* Max number of subcommands per event message */
#define PRI_MAX_SUBCOMMANDS	8

struct pri_subcommands {
	int counter_subcmd;
	struct pri_subcommand subcmd[PRI_MAX_SUBCOMMANDS];
};


/*
 * Event channel parameter encoding:
 * 3322 2222 2222 1111 1111 1100 0000 0000
 * 1098 7654 3210 9876 5432 1098 7654 3210
 * xxxx xxxx xxxx xEDC BBBBBBBBB AAAAAAAAA
 *
 * Bit field
 * A - B channel
 * B - Span (DS1) (0 - 127)
 * C - DS1 Explicit bit
 * D - D channel (cis_call) bit (status only)
 * E - Call is held bit (status only)
 *
 * B channel values:
 * 0     - No channel (ISDN uses for call waiting feature)
 * 1-127 - B channel #
 * 0xFF  - Any channel (Also if whole channel value is -1 in event)
 */


typedef struct pri_event_generic {
	/* Events with no additional information fall in this category */
	int e;
} pri_event_generic;

typedef struct pri_event_error {
	int e;
	char err[256];
} pri_event_error;

typedef struct pri_event_restart {
	int e;
	int channel;
} pri_event_restart;

typedef struct pri_event_ringing {
	int e;
	int channel;
	int cref;
	int progress;
	int progressmask;
	q931_call *call;
	char useruserinfo[260];		/* User->User info */
	struct pri_subcommands *subcmds;
} pri_event_ringing;

typedef struct pri_event_answer {
	int e;
	int channel;
	int cref;
	int progress;
	int progressmask;
	q931_call *call;
	char useruserinfo[260];		/* User->User info */
	struct pri_subcommands *subcmds;
} pri_event_answer;

/*! Deprecated replaced by struct pri_event_facility. */
typedef struct pri_event_facname {
	int e;
	char callingname[256];
	char callingnum[256];
	int channel;
	int cref;
	q931_call *call;
	int callingpres;			/* Presentation of Calling CallerID */
	int callingplan;			/* Dialing plan of Calling entity */
} pri_event_facname;

struct pri_event_facility {
	int e;
	char callingname[256];		/*!< Deprecated, preserved for struct pri_event_facname compatibility */
	char callingnum[256];		/*!< Deprecated, preserved for struct pri_event_facname compatibility */
	int channel;
	int cref;
	/*!
	 * \brief Master call or normal call.
	 * \note Call pointer known about by upper layer.
	 * \note NULL if dummy call reference.
	 */
	q931_call *call;
	int callingpres;			/*!< Presentation of Calling CallerID (Deprecated, preserved for struct pri_event_facname compatibility) */
	int callingplan;			/*!< Dialing plan of Calling entity (Deprecated, preserved for struct pri_event_facname compatibility) */
	struct pri_subcommands *subcmds;
	q931_call *subcall;			/*!< Subcall to send any reply toward. */
};

#define PRI_CALLINGPLANANI
#define PRI_CALLINGPLANRDNIS
typedef struct pri_event_ring {
	int e;
	int channel;				/* Channel requested */
	int callingpres;			/* Presentation of Calling CallerID */
	int callingplanani;			/* Dialing plan of Calling entity ANI */
	int callingplan;			/* Dialing plan of Calling entity */
	char callingani[256];		/* Calling ANI */
	char callingnum[256];		/* Calling number */
	char callingname[256];		/* Calling name (if provided) */
	int calledplan;				/* Dialing plan of Called number */
	int ani2;                   /* ANI II */
	char callednum[256];		/* Called number */
	char redirectingnum[256];	/* Redirecting number */
	char redirectingname[256];	/* Redirecting name */
	int redirectingreason;		/* Reason for redirect */
	int callingplanrdnis;			/* Dialing plan of Redirecting Number */
	char useruserinfo[260];		/* User->User info */
	int flexible;				/* Are we flexible with our channel selection? */
	int cref;					/* Call Reference Number */
	int ctype;					/* Call type (see PRI_TRANS_CAP_* */
	int layer1;					/* User layer 1 */
	int complete;				/* Have we seen "Complete" i.e. no more number? */
	q931_call *call;			/* Opaque call pointer */
	char callingsubaddr[256];	/* Calling parties subaddress, backwards compatibility */
	int progress;
	int progressmask;
	char origcalledname[256];
	char origcallednum[256];
	int callingplanorigcalled;		/* Dialing plan of Originally Called Number */
	int origredirectingreason;
	int reversecharge;
	struct pri_subcommands *subcmds;
	struct pri_party_id calling;			/* Calling Party's info, initially subaddress' */
	struct pri_party_subaddress called_subaddress;	/* Called party's subaddress */
	char keypad_digits[64];		/* Keypad digits in the SETUP message. */
} pri_event_ring;

typedef struct pri_event_hangup {
	int e;
	int channel;				/* Channel requested */
	int cause;
	int cref;
	q931_call *call;			/* Opaque call pointer of call hanging up. */
	long aoc_units;				/* Advice of Charge number of charged units */
	char useruserinfo[260];		/* User->User info */
	struct pri_subcommands *subcmds;
	/*!
	 * \brief Opaque held call pointer for possible transfer to active call.
	 * \note The call_held and call_active pointers must not be NULL if
	 * transfer held call on disconnect is available.
	 */
	q931_call *call_held;
	/*!
	 * \brief Opaque active call pointer for possible transfer with held call.
	 * \note The call_held and call_active pointers must not be NULL if
	 * transfer held call on disconnect is available.
	 */
	q931_call *call_active;
} pri_event_hangup;

typedef struct pri_event_restart_ack {
	int e;
	int channel;
} pri_event_restart_ack;

#define PRI_PROGRESS_CAUSE
typedef struct pri_event_proceeding {
	int e;
	int channel;
	int cref;
	int progress;
	int progressmask;
	int cause;
	q931_call *call;
	struct pri_subcommands *subcmds;
} pri_event_proceeding;

typedef struct pri_event_setup_ack {
	int e;
	int channel;
	q931_call *call;
	struct pri_subcommands *subcmds;
} pri_event_setup_ack;

typedef struct pri_event_notify {
	int e;
	int channel;
	int info;
	struct pri_subcommands *subcmds;
	q931_call *call;
} pri_event_notify;

typedef struct pri_event_keypad_digit {
	int e;
	int channel;
	q931_call *call;
	char digits[64];
	struct pri_subcommands *subcmds;
} pri_event_keypad_digit;

typedef struct pri_event_service {
	int e;
	int channel;
	int changestatus;
} pri_event_service;

typedef struct pri_event_service_ack {
	int e;
	int channel;
	int changestatus;
} pri_event_service_ack;

struct pri_event_hold {
	int e;
	int channel;
	q931_call *call;
	struct pri_subcommands *subcmds;
};

struct pri_event_hold_ack {
	int e;
	int channel;
	q931_call *call;
	struct pri_subcommands *subcmds;
};

struct pri_event_hold_rej {
	int e;
	int channel;
	q931_call *call;
	int cause;
	struct pri_subcommands *subcmds;
};

struct pri_event_retrieve {
	int e;
	int channel;
	q931_call *call;
	int flexible;				/* Are we flexible with our channel selection? */
	struct pri_subcommands *subcmds;
};

struct pri_event_retrieve_ack {
	int e;
	int channel;
	q931_call *call;
	struct pri_subcommands *subcmds;
};

struct pri_event_retrieve_rej {
	int e;
	int channel;
	q931_call *call;
	int cause;
	struct pri_subcommands *subcmds;
};

struct pri_event_connect_ack {
	int e;
	int channel;
	q931_call *call;
	struct pri_subcommands *subcmds;
};

typedef union {
	int e;
	pri_event_generic gen;		/* Generic view */
	pri_event_restart restart;	/* Restart view */
	pri_event_error	  err;		/* Error view */
	pri_event_facname facname;	/* Caller*ID Name on Facility (Deprecated, use pri_event.facility) */
	pri_event_ring	  ring;		/* Ring */
	pri_event_hangup  hangup;	/* Hang up */
	pri_event_ringing ringing;	/* Ringing */
	pri_event_answer  answer;	/* Answer */
	pri_event_restart_ack restartack;	/* Restart Acknowledge */
	pri_event_proceeding  proceeding;	/* Call proceeding & Progress */
	pri_event_setup_ack   setup_ack;	/* SETUP_ACKNOWLEDGE structure */
	pri_event_notify notify;		/* Notification */
	pri_event_keypad_digit digit;			/* Digits that come during a call */
	pri_event_service service;				/* service message */
	pri_event_service_ack service_ack;		/* service acknowledgement message */
	struct pri_event_facility facility;
	struct pri_event_hold hold;
	struct pri_event_hold_ack hold_ack;
	struct pri_event_hold_rej hold_rej;
	struct pri_event_retrieve retrieve;
	struct pri_event_retrieve_ack retrieve_ack;
	struct pri_event_retrieve_rej retrieve_rej;
	struct pri_event_connect_ack connect_ack;
} pri_event;

struct pri;
struct pri_sr;

#define PRI_IO_FUNCS
/* Type declaration for callbacks to read or write a HDLC frame as below */
typedef int (*pri_io_cb)(struct pri *pri, void *buf, int buflen);

/* Create a D-channel on a given file descriptor.  The file descriptor must be a
   channel operating in HDLC mode with FCS computed by the fd's driver.  Also it
   must be NON-BLOCKING! Frames received on the fd should include FCS.  Nodetype
   must be one of PRI_NETWORK or PRI_CPE.  switchtype should be PRI_SWITCH_* */
struct pri *pri_new(int fd, int nodetype, int switchtype);
struct pri *pri_new_bri(int fd, int ptpmode, int nodetype, int switchtype);

/* Create D-channel just as above with user defined I/O callbacks and data */
struct pri *pri_new_cb(int fd, int nodetype, int switchtype, pri_io_cb io_read, pri_io_cb io_write, void *userdata);

/* Create BRI D-channel just as above with user defined I/O callbacks and data */
struct pri *pri_new_bri_cb(int fd, int ptpmode, int nodetype, int switchtype, pri_io_cb io_read, pri_io_cb io_write, void *userdata);

/* Retrieve the user data associated with the D channel */
void *pri_get_userdata(struct pri *pri);

/* Set the user data associated with the D channel */
void pri_set_userdata(struct pri *pri, void *userdata);

/* Set Network Specific Facility for PRI */
void pri_set_nsf(struct pri *pri, int nsf);

/* Set debug parameters on PRI -- see above debug definitions */
void pri_set_debug(struct pri *pri, int debug);

/* Get debug parameters on PRI -- see above debug definitions */
int pri_get_debug(struct pri *pri);

#define PRI_FACILITY_ENABLE
/* Enable transmission support of Facility IEs on the pri */
void pri_facility_enable(struct pri *pri);

/* Run PRI on the given D-channel, taking care of any events that
   need to be handled.  If block is set, it will block until an event
   occurs which needs to be handled */
pri_event *pri_dchannel_run(struct pri *pri, int block);

/* Check for an outstanding event on the PRI */
pri_event *pri_check_event(struct pri *pri);

/* Give a name to a given event ID */
char *pri_event2str(int id);

/* Give a name to a node type */
char *pri_node2str(int id);

/* Give a name to a switch type */
char *pri_switch2str(int id);

/* Print an event */
void pri_dump_event(struct pri *pri, pri_event *e);

/* Turn presentation into a string */
char *pri_pres2str(int pres);

/* Turn numbering plan into a string */
char *pri_plan2str(int plan);

/* Turn cause into a string */
char *pri_cause2str(int cause);

/*!
 * \brief Convert the given facility error code to a descriptive string.
 *
 * \param facility_error_code Error code to convert to a string.
 *
 * \return Descriptive error string.
 */
const char *pri_facility_error2str(int facility_error_code);

/*!
 * \brief Convert the given facility reject code to a descriptive string.
 *
 * \param facility_reject_code Error code to convert to a string.
 *
 * \return Descriptive reject string.
 */
const char *pri_facility_reject2str(int facility_reject_code);

/* Acknowledge a call and place it on the given channel.  Set info to non-zero if there
   is in-band data available on the channel */
int pri_acknowledge(struct pri *pri, q931_call *call, int channel, int info);

/* Send a digit in overlap mode */
int pri_information(struct pri *pri, q931_call *call, char digit);

#define PRI_KEYPAD_FACILITY_TX
/* Send a keypad facility string of digits */
int pri_keypad_facility(struct pri *pri, q931_call *call, const char *digits);

/* Answer the incomplete(call without called number) call on the given channel.
   Set non-isdn to non-zero if you are not connecting to ISDN equipment */
int pri_need_more_info(struct pri *pri, q931_call *call, int channel, int nonisdn);

/* Answer(CONNECT) the call on the given channel.
   Set non-isdn to non-zero if you are not connecting to ISDN equipment */
int pri_answer(struct pri *pri, q931_call *call, int channel, int nonisdn);

/*!
 * \brief Send the manual CONNECT_ACKNOWLEDGE message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param channel Selected channel to assign to the call waiting call.
 * Zero if do not include the channel id ie in the CONNECT_ACKNOWLEDGE message.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int pri_connect_ack(struct pri *ctrl, q931_call *call, int channel);

/*!
 * \brief Set the manual CONNECT_ACKNOWLEDGE message enable flag.
 *
 * \param ctrl D channel controller.
 * \param enable TRUE to enable manual CONNECT_ACKNOWLEDGE message feature.
 *
 * \return Nothing
 */
void pri_connect_ack_enable(struct pri *ctrl, int enable);

/*!
 * \brief Give connected line information to a call
 * \note Could be used instead of pri_sr_set_caller_party() before calling pri_setup().
 */
int pri_connected_line_update(struct pri *pri, q931_call *call, const struct pri_party_connected_line *connected);

/*!
 * \brief Give redirection information to a call
 * \note Could be used instead of pri_sr_set_redirecting_parties() before calling pri_setup().
 */
int pri_redirecting_update(struct pri *pri, q931_call *call, const struct pri_party_redirecting *redirecting);

/* Set CRV reference for GR-303 calls */


#undef pri_release
#undef pri_disconnect

/* backwards compatibility for those who don't use asterisk with libpri */
#define pri_release(a,b,c) \
	pri_hangup(a,b,c)

#define pri_disconnect(a,b,c) \
	pri_hangup(a,b,c)

/* Hangup a call */
#define PRI_HANGUP
int pri_hangup(struct pri *pri, q931_call *call, int cause);

/*!
 * \brief Set the call hangup fix enable flag.
 *
 * \param ctrl D channel controller.
 * \param enable TRUE to follow Q.931 Section 5.3.2 call hangup better.
 * FALSE for legacy behaviour. (Default FALSE if not called.)
 *
 * \return Nothing
 */
void pri_hangup_fix_enable(struct pri *ctrl, int enable);

#define PRI_DESTROYCALL
void pri_destroycall(struct pri *pri, q931_call *call);

#define PRI_RESTART
int pri_restart(struct pri *pri);

int pri_reset(struct pri *pri, int channel);

/* handle b-channel maintenance messages */
extern int pri_maintenance_service(struct pri *pri, int span, int channel, int changestatus);

/* Create a new call */
q931_call *pri_new_call(struct pri *pri);

/*!
 * \brief Deterimine if the given call control pointer is a dummy call.
 *
 * \retval TRUE if given call is a dummy call.
 * \retval FALSE otherwise.
 */
int pri_is_dummy_call(q931_call *call);

/* Retrieve CRV reference for GR-303 calls.  Returns >0 on success. */
int pri_get_crv(struct pri *pri, q931_call *call, int *callmode);

/* Retrieve CRV reference for GR-303 calls.  CRV must be >0, call mode should be 0 */
int pri_set_crv(struct pri *pri, q931_call *call, int crv, int callmode);

/* How long until you need to poll for a new event */
struct timeval *pri_schedule_next(struct pri *pri);

/* Run any pending schedule events */
extern pri_event *pri_schedule_run(struct pri *pri);
extern pri_event *pri_schedule_run_tv(struct pri *pri, const struct timeval *now);

int pri_call(struct pri *pri, q931_call *c, int transmode, int channel,
    int exclusive, int nonisdn, char *caller, int callerplan, char *callername, int callerpres,
    char *called, int calledplan, int ulayer1);

struct pri_sr *pri_sr_new(void);
void pri_sr_free(struct pri_sr *sr);

int pri_sr_set_channel(struct pri_sr *sr, int channel, int exclusive, int nonisdn);
int pri_sr_set_bearer(struct pri_sr *sr, int transmode, int userl1);
int pri_sr_set_called(struct pri_sr *sr, char *called, int calledplan, int complete);

/*!
 * \brief Set the caller party ID information in the call SETUP record.
 *
 * \param sr New call SETUP record.
 * \param caller Caller party ID information to set.
 *
 * \return Nothing
 */
void pri_sr_set_caller_party(struct pri_sr *sr, const struct pri_party_id *caller);
/*! \note Use pri_sr_set_caller_party() instead to pass more precise caller information. */
int pri_sr_set_caller(struct pri_sr *sr, char *caller, char *callername, int callerplan, int callerpres);

/*!
 * \brief Set the calling subaddress information in the call SETUP record.
 *
 * \param sr New call SETUP record.
 * \param subaddress information to set.
 *
 * \return Nothing
 */
void pri_sr_set_caller_subaddress(struct pri_sr *sr, const struct pri_party_subaddress *subaddress);

/*!
 * \brief Set the called subaddress information in the call SETUP record.
 *
 * \param sr New call SETUP record.
 * \param subaddress information to set.
 *
 * \return Nothing
 */
void pri_sr_set_called_subaddress(struct pri_sr *sr, const struct pri_party_subaddress *subaddress);

/*!
 * \brief Set the redirecting information in the call SETUP record.
 *
 * \param sr New call SETUP record.
 * \param caller Redirecting information to set.
 *
 * \return Nothing
 */
void pri_sr_set_redirecting_parties(struct pri_sr *sr, const struct pri_party_redirecting *redirecting);
/*! \note Use pri_sr_set_redirecting_parties() instead to pass more precise redirecting information. */
int pri_sr_set_redirecting(struct pri_sr *sr, char *num, int plan, int pres, int reason);

/*!
 * \brief Set the keypad digits in the call SETUP record.
 *
 * \param sr New call SETUP record.
 * \param keypad_digits Keypad digits to send.
 *
 * \return Nothing
 */
void pri_sr_set_keypad_digits(struct pri_sr *sr, const char *keypad_digits);

#define PRI_USER_USER_TX
/* Set the user user field.  Warning!  don't send binary data accross this field */
void pri_sr_set_useruser(struct pri_sr *sr, const char *userchars);
void pri_sr_set_reversecharge(struct pri_sr *sr, int requested);

void pri_call_set_useruser(q931_call *sr, const char *userchars);

int pri_setup(struct pri *pri, q931_call *call, struct pri_sr *req);

/*!
 * \brief Set a call as a call indpendent signalling connection (i.e. no bchan)
 * \note Call will automaticlly disconnect after signalling sent.
 */
int pri_sr_set_connection_call_independent(struct pri_sr *req);

/*!
 * \brief Set a call as a call indpendent signalling connection (i.e. no bchan)
 * \note Call will stay connected until explicitly disconnected.
 */
int pri_sr_set_no_channel_call(struct pri_sr *req);

/* Send an MWI indication to a remote location.  If activate is non zero, activates, if zero, deactivates */
int pri_mwi_activate(struct pri *pri, q931_call *c, char *caller, int callerplan, char *callername, int callerpres, char *called, int calledplan);

/* Send an MWI deactivate request to a remote location */
int pri_mwi_deactivate(struct pri *pri, q931_call *c, char *caller, int callerplan, char *callername, int callerpres, char *called, int calledplan);

/*!
 * \brief Send a MWI indication on the specified D channel.
 *
 * \param ctrl D channel controller.
 * \param mailbox Party receiving notification.
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
int pri_mwi_indicate(struct pri *ctrl, const struct pri_party_id *mailbox,
	int basic_service, int num_messages, const struct pri_party_id *caller_id,
	const char *timestamp, int message_reference, int message_status);

/*!
 * \brief Send a MWI indication on the specified D channel. (Take two)
 *
 * \param ctrl D channel controller.
 * \param mailbox Party receiving notification.
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
int pri_mwi_indicate_v2(struct pri *ctrl, const struct pri_party_id *mailbox,
	const struct pri_party_id *vm_id, int basic_service, int num_messages,
	const struct pri_party_id *caller_id, const char *timestamp, int message_reference,
	int message_status);

/* Set service message support flag */
int pri_set_service_message_support(struct pri *pri, int supportflag);

#define PRI_2BCT
/* Attempt to pass the channels back to the NET side if compatable and
 * subscribed.  Sometimes called 2 bchannel transfer (2BCT) */
int pri_channel_bridge(q931_call *call1, q931_call *call2);

/* Override message and error stuff */
#define PRI_NEW_SET_API
void pri_set_message(void (*__pri_error)(struct pri *pri, char *));
void pri_set_error(void (*__pri_error)(struct pri *pri, char *));

/* Set overlap mode */
#define PRI_SET_OVERLAPDIAL
void pri_set_overlapdial(struct pri *pri,int state);

/* QSIG logical channel mapping option, do not skip channel 16 */
#define PRI_SET_CHAN_MAPPING_LOGICAL
void pri_set_chan_mapping_logical(struct pri *pri, int state);

#define PRI_DUMP_INFO_STR
char *pri_dump_info_str(struct pri *pri);

/* Get file descriptor */
int pri_fd(struct pri *pri);

#define PRI_PROGRESS
/* Send progress */
int pri_progress(struct pri *pri, q931_call *c, int channel, int info);

/* Send progress with cause IE */
int pri_progress_with_cause(struct pri *pri, q931_call *c, int channel, int info, int cause);

#define PRI_PROCEEDING_FULL
/* Send call proceeding */
int pri_proceeding(struct pri *pri, q931_call *c, int channel, int info);

/* Enable inband progress when a DISCONNECT is received */
void pri_set_inbanddisconnect(struct pri *pri, unsigned int enable);

/* Enslave a PRI to another, so they share the same call list
   (and maybe some timers) */
void pri_enslave(struct pri *master, struct pri *slave);

/*!
 * \brief  Request AOC on call setup
 *
 * \param call setup struct to set charging request info on
 * \param charging request to set on setup struct
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int pri_sr_set_aoc_charging_request(struct pri_sr *sr, int charging_request);

/*!
 * \brief Send AOC Request Response to a request for AOC-S
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param invoke_id for response message
 * \param aoc_s message for response
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int pri_aoc_s_request_response_send(struct pri *ctrl, q931_call *call, int invoke_id, const struct pri_subcmd_aoc_s *aoc_s);

/*!
 * \brief Send AOC Request Response to a request for AOC-D or AOC-E
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param response in form of enum PRI_AOC_REQ_RSP
 * \param invoke_id for response message
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int pri_aoc_de_request_response_send(struct pri *ctrl, q931_call *call, int response, int invoke_id);

/*!
 * \brief Send AOC-S message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param aoc_s message to send
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int pri_aoc_s_send(struct pri *ctrl, q931_call *c, const struct pri_subcmd_aoc_s *aoc_s);

/*!
 * \brief Send AOC-D message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param aoc_d message to send
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int pri_aoc_d_send(struct pri *ctrl, q931_call *c, const struct pri_subcmd_aoc_d *aoc_d);

/*!
 * \brief Send AOC-E message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param aoc_e message to send
 *
 * \retval 0 on success
 * \retval -1 on failure
 */
int pri_aoc_e_send(struct pri *ctrl, q931_call *c, const struct pri_subcmd_aoc_e *aoc_e);

#define PRI_GR303_SUPPORT
#define PRI_ENSLAVE_SUPPORT
#define PRI_SETUP_CALL
#define PRI_RECEIVE_SUBADDR
#define PRI_REDIRECTING_REASON
#define PRI_AOC_UNITS
#define PRI_ANI

/* Send notification */
int pri_notify(struct pri *pri, q931_call *c, int channel, int info);

int pri_callrerouting_facility(struct pri *pri, q931_call *call, const char *dest, const char* original, const char* reason);

/*!
 * \brief Set the call deflection/rerouting feature enable flag.
 *
 * \param ctrl D channel controller.
 * \param enable TRUE to enable call deflection/rerouting feature.
 *
 * \return Nothing
 */
void pri_reroute_enable(struct pri *ctrl, int enable);

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
int pri_reroute_call(struct pri *ctrl, q931_call *call, const struct pri_party_id *caller, const struct pri_party_redirecting *deflection, int subscription_option);

enum PRI_REROUTING_RSP_CODE {
	/*!
	 * Rerouting invocation accepted and the network provider option
	 * "served user call retention on invocation of diversion"
	 * is "clear call on invocation".
	 */
	PRI_REROUTING_RSP_OK_CLEAR,
	/*!
	 * Rerouting invocation accepted and the network provider option
	 * "served user call retention on invocation of diversion"
	 * is "retain call until alerting begins at the deflected-to user".
	 */
	PRI_REROUTING_RSP_OK_RETAIN,
	PRI_REROUTING_RSP_NOT_SUBSCRIBED,
	PRI_REROUTING_RSP_NOT_AVAILABLE,
	/*! Supplementary service interaction not allowed. */
	PRI_REROUTING_RSP_NOT_ALLOWED,
	PRI_REROUTING_RSP_INVALID_NUMBER,
	/*! Deflection to prohibited number (e.g., operator, police, emergency). */
	PRI_REROUTING_RSP_SPECIAL_SERVICE_NUMBER,
	/*! Deflection to served user number. */
	PRI_REROUTING_RSP_DIVERSION_TO_SELF,
	PRI_REROUTING_RSP_MAX_DIVERSIONS_EXCEEDED,
	PRI_REROUTING_RSP_RESOURCE_UNAVAILABLE,
};

/*!
 * \brief Send the CallRerouteing/CallDeflection response message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param invoke_id Value given by the initiating request.
 * \param code The result to send.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int pri_rerouting_rsp(struct pri *ctrl, q931_call *call, int invoke_id, enum PRI_REROUTING_RSP_CODE code);

/*!
 * \brief Set the call transfer feature enable flag.
 *
 * \param ctrl D channel controller.
 * \param enable TRUE to enable call transfer feature.
 *
 * \return Nothing
 */
void pri_transfer_enable(struct pri *ctrl, int enable);

/*!
 * \brief Send the call transfer response message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg.
 * \param invoke_id Value given by the initiating request.
 * \param is_successful TRUE if the transfer was successful.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int pri_transfer_rsp(struct pri *ctrl, q931_call *call, int invoke_id, int is_successful);

/*!
 * \brief Set the advice of charge events feature enable flag.
 *
 * \param ctrl D channel controller.
 * \param enable TRUE to enable AOC events feature.
 *
 * \return Nothing
 */
void pri_aoc_events_enable(struct pri *ctrl, int enable);

enum pri_layer2_persistence {
	PRI_L2_PERSISTENCE_DEFAULT,
	/*! Immediately bring layer 2 back up if the peer brings layer 2 down. */
	PRI_L2_PERSISTENCE_KEEP_UP,
	/*! Leave layer 2 down if the peer brings layer 2 down. */
	PRI_L2_PERSISTENCE_LEAVE_DOWN,
#if 0	/* Possible future option.  Would need to define how long to idle before dropping. */
	/*! Drop layer 2 on D channel idle. */
	PRI_L2_PERSISTENCE_IDLE_DROP,
#endif
};

/*!
 * \brief Set the layer2 persistence option.
 *
 * \param ctrl D channel controller.
 * \param option Layer 2 persistence to apply.
 *
 * \note
 * Not all values are supported by all modes.
 *
 * \return Nothing
 */
void pri_persistent_layer2_option(struct pri *ctrl, enum pri_layer2_persistence option);

#define PRI_DISPLAY_OPTION_BLOCK		(1 << 0)	/*!< Do not pass display text. */
#define PRI_DISPLAY_OPTION_NAME_INITIAL	(1 << 1)	/*!< Use display in SETUP/CONNECT for name. */
#define PRI_DISPLAY_OPTION_NAME_UPDATE	(1 << 2)	/*!< Use display in FACILITY/NOTIFY for COLP name if appropriate. */
#define PRI_DISPLAY_OPTION_TEXT			(1 << 3)	/*!< Pass arbitrary display text in INFORMATION messages during call. */

/*!
 * \brief Set the display ie send policy options.
 *
 * \param ctrl D channel controller.
 * \param flags Option flags to apply.
 *
 * \note
 * If no flags set then legacy default behaviour.
 *
 * \note
 * Not all options are supported by all switches.
 *
 * \return Nothing
 */
void pri_display_options_send(struct pri *ctrl, unsigned long flags);

/*!
 * \brief Set the display ie receive policy options.
 *
 * \param ctrl D channel controller.
 * \param flags Option flags to apply.
 *
 * \note
 * If no flags set then legacy default behaviour.
 *
 * \note
 * Not all options are supported by all switches.
 *
 * \return Nothing
 */
void pri_display_options_receive(struct pri *ctrl, unsigned long flags);

/*!
 * \brief Send display text during a call.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg
 * \param display Display text to send.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int pri_display_text(struct pri *ctrl, q931_call *call, const struct pri_subcmd_display_txt *display);

/*!
 * \brief Set the call hold feature enable flag.
 *
 * \param ctrl D channel controller.
 * \param enable TRUE to enable call hold feature.
 *
 * \return Nothing
 */
void pri_hold_enable(struct pri *ctrl, int enable);

/*!
 * \brief Send the HOLD message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int pri_hold(struct pri *ctrl, q931_call *call);

/*!
 * \brief Send the HOLD ACKNOWLEDGE message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int pri_hold_ack(struct pri *ctrl, q931_call *call);

/*!
 * \brief Send the HOLD REJECT message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg
 * \param cause Q.931 cause code for rejecting the hold request.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int pri_hold_rej(struct pri *ctrl, q931_call *call, int cause);

/*!
 * \brief Send the RETRIEVE message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg
 * \param channel Encoded channel id to use.  If zero do not send channel id.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int pri_retrieve(struct pri *ctrl, q931_call *call, int channel);

/*!
 * \brief Send the RETRIEVE ACKNOWLEDGE message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg
 * \param channel Encoded channel id to use.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int pri_retrieve_ack(struct pri *ctrl, q931_call *call, int channel);

/*!
 * \brief Send the RETRIEVE REJECT message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg
 * \param cause Q.931 cause code for rejecting the retrieve request.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int pri_retrieve_rej(struct pri *ctrl, q931_call *call, int cause);

#if defined(STATUS_REQUEST_PLACE_HOLDER)
int pri_status_req(struct pri *ctrl, int request_id, const struct pri_sr *req);
void pri_status_req_rsp(struct pri *ctrl, int invoke_id, int status);
#endif	/* defined(STATUS_REQUEST_PLACE_HOLDER) */

/*!
 * \brief Set the Malicious Call ID feature enable flag.
 *
 * \param ctrl D channel controller.
 * \param enable TRUE to enable MCID feature.
 *
 * \return Nothing
 */
void pri_mcid_enable(struct pri *ctrl, int enable);

/*!
 * \brief Send the MCID request message.
 *
 * \param ctrl D channel controller.
 * \param call Q.931 call leg
 *
 * \retval 0 on success.  You should wait for a PRI_SUBCMD_MCID_RSP
 * to continue clearing the call if it was in progress.
 * \retval -1 on error.
 */
int pri_mcid_req_send(struct pri *ctrl, q931_call *call);

/*!
 * \brief Set the call completion feature enable flag.
 *
 * \param ctrl D channel controller.
 * \param enable TRUE to enable call completion feature.
 *
 * \return Nothing
 */
void pri_cc_enable(struct pri *ctrl, int enable);

/*!
 * \brief Set the PTMP NT call completion recall mode.
 *
 * \param ctrl D channel controller.
 * \param mode globalRecall(0), specificRecall(1)
 *
 * \return Nothing
 */
void pri_cc_recall_mode(struct pri *ctrl, int mode);

/*!
 * \brief Set the Q.SIG call completion signaling link retention mode.
 * (Requestor/Initiator/Originator/Party-A)
 *
 * \param ctrl D channel controller.
 * \param signaling_retention release(0), retain(1), do-not-care(2).
 *
 * \return Nothing
 */
void pri_cc_retain_signaling_req(struct pri *ctrl, int signaling_retention);

/*!
 * \brief Set the Q.SIG call completion signaling link retention mode.
 * (Responder/Answerer/Party-B)
 *
 * \param ctrl D channel controller.
 * \param signaling_retention release(0), retain(1).
 *
 * \return Nothing
 */
void pri_cc_retain_signaling_rsp(struct pri *ctrl, int signaling_retention);

long pri_cc_available(struct pri *ctrl, q931_call *call);
int pri_cc_req(struct pri *ctrl, long cc_id, int mode);
int pri_cc_req_rsp(struct pri *ctrl, long cc_id, int status);
void pri_cc_remote_user_free(struct pri *ctrl, long cc_id);
void pri_cc_b_free(struct pri *ctrl, long cc_id);
void pri_cc_stop_alerting(struct pri *ctrl, long cc_id);
void pri_cc_status_req(struct pri *ctrl, long cc_id);
void pri_cc_status_req_rsp(struct pri *ctrl, long cc_id, int status);
void pri_cc_status(struct pri *ctrl, long cc_id, int status);
int pri_cc_call(struct pri *ctrl, long cc_id, q931_call *call, struct pri_sr *req);
void pri_cc_cancel(struct pri *ctrl, long cc_id);

/* Date/time ie send policy option values. */
#define PRI_DATE_TIME_SEND_DEFAULT		0	/*!< Send date/time ie default. */
#define PRI_DATE_TIME_SEND_NO			1	/*!< Send date/time ie never. */
#define PRI_DATE_TIME_SEND_DATE			2	/*!< Send date/time ie date only. */
#define PRI_DATE_TIME_SEND_DATE_HH		3	/*!< Send date/time ie date and hour. */
#define PRI_DATE_TIME_SEND_DATE_HHMM	4	/*!< Send date/time ie date, hour, and minute. */
#define PRI_DATE_TIME_SEND_DATE_HHMMSS	5	/*!< Send date/time ie date, hour, minute, and second. */

/*!
 * \brief Set the date/time ie send policy option.
 *
 * \param ctrl D channel controller.
 * \param option Policy option to set.
 *
 * \note
 * Only valid in NT mode.
 *
 * \return Nothing
 */
void pri_date_time_send_option(struct pri *ctrl, int option);

/* Get/Set PRI Timers  */
#define PRI_GETSET_TIMERS
int pri_set_timer(struct pri *pri, int timer, int value);
int pri_get_timer(struct pri *pri, int timer);
int pri_timer2idx(const char *timer_name);

/*! New configurable timers and counters must be added to the end of the list */
enum PRI_TIMERS_AND_COUNTERS {
	PRI_TIMER_N200,	/*!< Maximum numer of Q.921 retransmissions */
	PRI_TIMER_N201,	/*!< Maximum numer of octets in an information field */
	PRI_TIMER_N202,	/*!< Maximum numer of transmissions of the TEI identity request message */
	PRI_TIMER_K,	/*!< Maximum number of outstanding I-frames */

	PRI_TIMER_T200,	/*!< Time between SABME's */
	PRI_TIMER_T201,	/*!< Minimum time between retransmissions of the TEI Identity check messages */
	PRI_TIMER_T202,	/*!< Minimum time between transmission of TEI Identity request messages */
	PRI_TIMER_T203,	/*!< Maximum time without exchanging packets */

	PRI_TIMER_T300,
	PRI_TIMER_T301,	/*!< Maximum time to respond to an ALERT */
	PRI_TIMER_T302,
	PRI_TIMER_T303,	/*!< Maximum time to wait after sending a SETUP without a response */
	PRI_TIMER_T304,
	PRI_TIMER_T305,	/*!< Wait for DISCONNECT acknowledge */
	PRI_TIMER_T306,
	PRI_TIMER_T307,
	PRI_TIMER_T308,	/*!< Wait for RELEASE acknowledge */
	PRI_TIMER_T309,	/*!< Time active calls can tollerate data link layer being down before clearing. */
	PRI_TIMER_T310,	/*!< Maximum time between receiving a CALL_PROCEEDING and receiving a ALERT/CONNECT/DISCONNECT/PROGRESS */
	PRI_TIMER_T313,	/*!< Wait for CONNECT acknowledge, CPE side only */
	PRI_TIMER_T314,
	PRI_TIMER_T316,	/*!< Maximum time between transmitting a RESTART and receiving a RESTART ACK */
	PRI_TIMER_T317,
	PRI_TIMER_T318,
	PRI_TIMER_T319,
	PRI_TIMER_T320,
	PRI_TIMER_T321,
	PRI_TIMER_T322,

	PRI_TIMER_TM20,	/*!< Maximum time awaiting XID response */
	PRI_TIMER_NM20,	/*!< Number of XID retransmits */

	PRI_TIMER_T_HOLD,	/*!< Maximum time to wait for HOLD request response. */
	PRI_TIMER_T_RETRIEVE,	/*!< Maximum time to wait for RETRIEVE request response. */

	PRI_TIMER_T_RESPONSE,	/*!< Maximum time to wait for a typical APDU response. */

	PRI_TIMER_T_STATUS,		/*!< Max time to wait for all replies to check for compatible terminals */

	PRI_TIMER_T_ACTIVATE,	/*!< Request supervision timeout. */
	PRI_TIMER_T_DEACTIVATE,	/*!< Deactivate supervision timeout. */
	PRI_TIMER_T_INTERROGATE,/*!< Interrogation supervision timeout. */

	/* ETSI call-completion timers */
	PRI_TIMER_T_RETENTION,	/*!< Max time to wait for user A to activate call-completion. */
	PRI_TIMER_T_CCBS1,		/*!< T-STATUS timer equivalent for CC user A status. */
	PRI_TIMER_T_CCBS2,		/*!< Max time the CCBS service will be active */
	PRI_TIMER_T_CCBS3,		/*!< Max time to wait for user A to respond to user B availability. */
	PRI_TIMER_T_CCBS4,		/*!< CC user B guard time before sending CC recall indication. */
	PRI_TIMER_T_CCBS5,		/*!< Network B CCBS supervision timeout. */
	PRI_TIMER_T_CCBS6,		/*!< Network A CCBS supervision timeout. */
	PRI_TIMER_T_CCNR2,		/*!< Max time the CCNR service will be active */
	PRI_TIMER_T_CCNR5,		/*!< Network B CCNR supervision timeout. */
	PRI_TIMER_T_CCNR6,		/*!< Network A CCNR supervision timeout. */

	/* Q.SIG call-completion timers */
	PRI_TIMER_QSIG_CC_T1,	/*!< CC request supervision timeout. */
	PRI_TIMER_QSIG_CCBS_T2,	/*!< CCBS supervision timeout. */
	PRI_TIMER_QSIG_CCNR_T2,	/*!< CCNR supervision timeout. */
	PRI_TIMER_QSIG_CC_T3,	/*!< Max time to wait for user A to respond to user B availability. */
	PRI_TIMER_QSIG_CC_T4,	/*!< Path reservation supervision timeout. */

	PRI_TIMER_T312,			/*!< Supervise broadcast SETUP message call reference retention. */

	/* Must be last in the enum list */
	PRI_MAX_TIMERS
};

/* Get PRI version */
const char *pri_get_version(void);

#endif
