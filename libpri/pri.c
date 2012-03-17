/*
 * libpri: An implementation of Primary Rate ISDN
 *
 * Written by Mark Spencer <markster@digium.com>
 *
 * Copyright (C) 2001-2005, Digium, Inc.
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

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <stdarg.h>
#include "compat.h"
#include "libpri.h"
#include "pri_internal.h"
#include "pri_facility.h"

#define PRI_BIT(a_bit)		(1UL << (a_bit))
#define PRI_ALL_SWITCHES	0xFFFFFFFF
#define PRI_ETSI_SWITCHES	(PRI_BIT(PRI_SWITCH_EUROISDN_E1) | PRI_BIT(PRI_SWITCH_EUROISDN_T1))

struct pri_timer_table {
	const char *name;
	enum PRI_TIMERS_AND_COUNTERS number;
	unsigned long used_by;
};

/*!
 * \note Sort the timer table entries in the order of the timer name so
 * pri_dump_info_str() can display them in a consistent order.
 */
static const struct pri_timer_table pri_timer[] = {
/* *INDENT-OFF* */
	/* timer name       timer number                used by switches */
	{ "N200",           PRI_TIMER_N200,             PRI_ALL_SWITCHES },
	{ "N201",           PRI_TIMER_N201,             PRI_ALL_SWITCHES },
	{ "N202",           PRI_TIMER_N202,             PRI_ALL_SWITCHES },
	{ "K",              PRI_TIMER_K,                PRI_ALL_SWITCHES },
	{ "T200",           PRI_TIMER_T200,             PRI_ALL_SWITCHES },
	{ "T201",           PRI_TIMER_T201,             PRI_ALL_SWITCHES },
	{ "T202",           PRI_TIMER_T202,             PRI_ALL_SWITCHES },
	{ "T203",           PRI_TIMER_T203,             PRI_ALL_SWITCHES },
	{ "T300",           PRI_TIMER_T300,             PRI_ALL_SWITCHES },
	{ "T301",           PRI_TIMER_T301,             PRI_ALL_SWITCHES },
	{ "T302",           PRI_TIMER_T302,             PRI_ALL_SWITCHES },
	{ "T303",           PRI_TIMER_T303,             PRI_ALL_SWITCHES },
	{ "T304",           PRI_TIMER_T304,             PRI_ALL_SWITCHES },
	{ "T305",           PRI_TIMER_T305,             PRI_ALL_SWITCHES },
	{ "T306",           PRI_TIMER_T306,             PRI_ALL_SWITCHES },
	{ "T307",           PRI_TIMER_T307,             PRI_ALL_SWITCHES },
	{ "T308",           PRI_TIMER_T308,             PRI_ALL_SWITCHES },
	{ "T309",           PRI_TIMER_T309,             PRI_ALL_SWITCHES },
	{ "T310",           PRI_TIMER_T310,             PRI_ALL_SWITCHES },
	{ "T312",           PRI_TIMER_T312,             PRI_ALL_SWITCHES },
	{ "T313",           PRI_TIMER_T313,             PRI_ALL_SWITCHES },
	{ "T314",           PRI_TIMER_T314,             PRI_ALL_SWITCHES },
	{ "T316",           PRI_TIMER_T316,             PRI_ALL_SWITCHES },
	{ "T317",           PRI_TIMER_T317,             PRI_ALL_SWITCHES },
	{ "T318",           PRI_TIMER_T318,             PRI_ALL_SWITCHES },
	{ "T319",           PRI_TIMER_T319,             PRI_ALL_SWITCHES },
	{ "T320",           PRI_TIMER_T320,             PRI_ALL_SWITCHES },
	{ "T321",           PRI_TIMER_T321,             PRI_ALL_SWITCHES },
	{ "T322",           PRI_TIMER_T322,             PRI_ALL_SWITCHES },
	{ "T-HOLD",         PRI_TIMER_T_HOLD,           PRI_ALL_SWITCHES },
	{ "T-RETRIEVE",     PRI_TIMER_T_RETRIEVE,       PRI_ALL_SWITCHES },
	{ "T-RESPONSE",     PRI_TIMER_T_RESPONSE,       PRI_ALL_SWITCHES },
	{ "T-STATUS",       PRI_TIMER_T_STATUS,         PRI_ETSI_SWITCHES },
	{ "T-ACTIVATE",     PRI_TIMER_T_ACTIVATE,       PRI_ETSI_SWITCHES },
	{ "T-DEACTIVATE",   PRI_TIMER_T_DEACTIVATE,     PRI_ETSI_SWITCHES },
	{ "T-INTERROGATE",  PRI_TIMER_T_INTERROGATE,    PRI_ETSI_SWITCHES },
	{ "T-RETENTION",    PRI_TIMER_T_RETENTION,      PRI_ETSI_SWITCHES | PRI_BIT(PRI_SWITCH_QSIG) },
	{ "T-CCBS1",        PRI_TIMER_T_CCBS1,          PRI_ETSI_SWITCHES },
	{ "T-CCBS2",        PRI_TIMER_T_CCBS2,          PRI_ETSI_SWITCHES },
	{ "T-CCBS3",        PRI_TIMER_T_CCBS3,          PRI_ETSI_SWITCHES },
	{ "T-CCBS4",        PRI_TIMER_T_CCBS4,          PRI_ETSI_SWITCHES },
	{ "T-CCBS5",        PRI_TIMER_T_CCBS5,          PRI_ETSI_SWITCHES },
	{ "T-CCBS6",        PRI_TIMER_T_CCBS6,          PRI_ETSI_SWITCHES },
	{ "T-CCNR2",        PRI_TIMER_T_CCNR2,          PRI_ETSI_SWITCHES },
	{ "T-CCNR5",        PRI_TIMER_T_CCNR5,          PRI_ETSI_SWITCHES },
	{ "T-CCNR6",        PRI_TIMER_T_CCNR6,          PRI_ETSI_SWITCHES },
	{ "CC-T1",          PRI_TIMER_QSIG_CC_T1,       PRI_BIT(PRI_SWITCH_QSIG) },
	{ "CCBS-T2",        PRI_TIMER_QSIG_CCBS_T2,     PRI_BIT(PRI_SWITCH_QSIG) },
	{ "CCNR-T2",        PRI_TIMER_QSIG_CCNR_T2,     PRI_BIT(PRI_SWITCH_QSIG) },
	{ "CC-T3",          PRI_TIMER_QSIG_CC_T3,       PRI_BIT(PRI_SWITCH_QSIG) },
#if defined(QSIG_PATH_RESERVATION_SUPPORT)
	{ "CC-T4",          PRI_TIMER_QSIG_CC_T4,       PRI_BIT(PRI_SWITCH_QSIG) },
#endif	/* defined(QSIG_PATH_RESERVATION_SUPPORT) */
/* *INDENT-ON* */
};

char *pri_node2str(int node)
{
	switch(node) {
	case PRI_UNKNOWN:
		return "Unknown node type";
	case PRI_NETWORK:
		return "Network";
	case PRI_CPE:
		return "CPE";
	default:
		return "Invalid value";
	}
}

char *pri_switch2str(int sw)
{
	switch(sw) {
	case PRI_SWITCH_NI2:
		return "National ISDN";
	case PRI_SWITCH_DMS100:
		return "Nortel DMS100";
	case PRI_SWITCH_LUCENT5E:
		return "Lucent 5E";
	case PRI_SWITCH_ATT4ESS:
		return "AT&T 4ESS";
	case PRI_SWITCH_NI1:
		return "National ISDN 1";
	case PRI_SWITCH_EUROISDN_E1:
		return "EuroISDN";
	case PRI_SWITCH_GR303_EOC:
		return "GR303 EOC";
	case PRI_SWITCH_GR303_TMC:
		return "GR303 TMC";
	case PRI_SWITCH_QSIG:
		return "Q.SIG switch";
	default:
		return "Unknown switchtype";
	}
}

static void pri_default_timers(struct pri *ctrl, int switchtype)
{
	unsigned idx;

	/* Initialize all timers/counters to unsupported/disabled. */
	for (idx = 0; idx < PRI_MAX_TIMERS; ++idx) {
		ctrl->timers[idx] = -1;
	}

	/* Set timer values to standard defaults.  Time is in ms. */
	ctrl->timers[PRI_TIMER_N200] = 3;			/* Max numer of Q.921 retransmissions */
	ctrl->timers[PRI_TIMER_N202] = 3;			/* Max numer of transmissions of the TEI identity request message */

	if (ctrl->bri) {
		ctrl->timers[PRI_TIMER_K] = 1;			/* Max number of outstanding I-frames */
	} else {
		ctrl->timers[PRI_TIMER_K] = 7;			/* Max number of outstanding I-frames */
	}

	ctrl->timers[PRI_TIMER_T200] = 1000;		/* Time between SABME's */
	ctrl->timers[PRI_TIMER_T201] = ctrl->timers[PRI_TIMER_T200];/* Time between TEI Identity Checks (Default same as T200) */
	ctrl->timers[PRI_TIMER_T202] = 10 * 1000;	/* Min time between transmission of TEI Identity request messages */
	ctrl->timers[PRI_TIMER_T203] = 10 * 1000;	/* Max time without exchanging packets */

	ctrl->timers[PRI_TIMER_T303] = 4 * 1000;	/* Length between SETUP retransmissions and timeout */
	ctrl->timers[PRI_TIMER_T305] = 30 * 1000;	/* Wait for DISCONNECT acknowledge */
	ctrl->timers[PRI_TIMER_T308] = 4 * 1000;	/* Wait for RELEASE acknowledge */
	ctrl->timers[PRI_TIMER_T309] = 6 * 1000;	/* Time to wait before clearing calls in case of D-channel transient event.  Q.931 specifies 6-90 seconds */
	ctrl->timers[PRI_TIMER_T312] = (4 + 2) * 1000;/* Supervise broadcast SETUP message call reference retention. T303 + 2 seconds */
	ctrl->timers[PRI_TIMER_T313] = 4 * 1000;	/* Wait for CONNECT acknowledge, CPE side only */

	ctrl->timers[PRI_TIMER_TM20] = 2500;		/* Max time awaiting XID response - Q.921 Appendix IV */
	ctrl->timers[PRI_TIMER_NM20] = 3;			/* Number of XID retransmits - Q.921 Appendix IV */

	ctrl->timers[PRI_TIMER_T_HOLD] = 4 * 1000;	/* Wait for HOLD request response. */
	ctrl->timers[PRI_TIMER_T_RETRIEVE] = 4 * 1000;/* Wait for RETRIEVE request response. */

	ctrl->timers[PRI_TIMER_T_RESPONSE] = 4 * 1000;	/* Maximum time to wait for a typical APDU response. */

	/* ETSI timers */
	ctrl->timers[PRI_TIMER_T_STATUS] = 4 * 1000;	/* Max time to wait for all replies to check for compatible terminals */
	ctrl->timers[PRI_TIMER_T_ACTIVATE] = 10 * 1000;	/* Request supervision timeout. */
	ctrl->timers[PRI_TIMER_T_DEACTIVATE] = 4 * 1000;/* Deactivate supervision timeout. */
	ctrl->timers[PRI_TIMER_T_INTERROGATE] = 4 * 1000;/* Interrogation supervision timeout. */

	/* ETSI call-completion timers */
	ctrl->timers[PRI_TIMER_T_RETENTION] = 30 * 1000;/* Max time to wait for user A to activate call-completion. */
	ctrl->timers[PRI_TIMER_T_CCBS1] = 4 * 1000;		/* T-STATUS timer equivalent for CC user A status. */
	ctrl->timers[PRI_TIMER_T_CCBS2] = 45 * 60 * 1000;/* Max time the CCBS service will be active */
	ctrl->timers[PRI_TIMER_T_CCBS3] = 20 * 1000;	/* Max time to wait for user A to respond to user B availability. */
	ctrl->timers[PRI_TIMER_T_CCBS4] = 5 * 1000;		/* CC user B guard time before sending CC recall indication. */
	ctrl->timers[PRI_TIMER_T_CCBS5] = 60 * 60 * 1000;/* Network B CCBS supervision timeout. */
	ctrl->timers[PRI_TIMER_T_CCBS6] = 60 * 60 * 1000;/* Network A CCBS supervision timeout. */
	ctrl->timers[PRI_TIMER_T_CCNR2] = 180 * 60 * 1000;/* Max time the CCNR service will be active */
	ctrl->timers[PRI_TIMER_T_CCNR5] = 195 * 60 * 1000;/* Network B CCNR supervision timeout. */
	ctrl->timers[PRI_TIMER_T_CCNR6] = 195 * 60 * 1000;/* Network A CCNR supervision timeout. */

	/* Q.SIG call-completion timers */
	ctrl->timers[PRI_TIMER_QSIG_CC_T1] = 30 * 1000;/* CC request supervision timeout. */
	ctrl->timers[PRI_TIMER_QSIG_CCBS_T2] = 60 * 60 * 1000;/* CCBS supervision timeout. */
	ctrl->timers[PRI_TIMER_QSIG_CCNR_T2] = 195 * 60 * 1000;/* CCNR supervision timeout. */
	ctrl->timers[PRI_TIMER_QSIG_CC_T3] = 30 * 1000;/* Max time to wait for user A to respond to user B availability. */
#if defined(QSIG_PATH_RESERVATION_SUPPORT)
	ctrl->timers[PRI_TIMER_QSIG_CC_T4] = 40 * 1000;/* Path reservation supervision timeout. */
#endif	/* defined(QSIG_PATH_RESERVATION_SUPPORT) */

	/* Set any switch specific override default values */
	switch (switchtype) {
	default:
		break;
	}
}

int pri_set_timer(struct pri *ctrl, int timer, int value)
{
	if (!ctrl || timer < 0 || PRI_MAX_TIMERS <= timer || value < 0) {
		return -1;
	}
	ctrl->timers[timer] = value;
	return 0;
}

int pri_get_timer(struct pri *ctrl, int timer)
{
	if (!ctrl || timer < 0 || PRI_MAX_TIMERS <= timer) {
		return -1;
	}
	return ctrl->timers[timer];
}

int pri_set_service_message_support(struct pri *pri, int supportflag)
{
	if (!pri) {
		return -1;
	}
	pri->service_message_support = supportflag ? 1 : 0;
	return 0;
}

int pri_timer2idx(const char *timer_name)
{
	unsigned idx;
	enum PRI_TIMERS_AND_COUNTERS timer_number;

	timer_number = -1;
	for (idx = 0; idx < ARRAY_LEN(pri_timer); ++idx) {
		if (!strcasecmp(timer_name, pri_timer[idx].name)) {
			timer_number = pri_timer[idx].number;
			break;
		}
	}
	return timer_number;
}

static int __pri_read(struct pri *pri, void *buf, int buflen)
{
	int res = read(pri->fd, buf, buflen);
	if (res < 0) {
		if (errno != EAGAIN)
			pri_error(pri, "Read on %d failed: %s\n", pri->fd, strerror(errno));
		return 0;
	}
	return res;
}

static int __pri_write(struct pri *pri, void *buf, int buflen)
{
	int res = write(pri->fd, buf, buflen);
	if (res < 0) {
		if (errno != EAGAIN)
			pri_error(pri, "Write to %d failed: %s\n", pri->fd, strerror(errno));
		return 0;
	}
	return res;
}

/*!
 * \internal
 * \brief Determine the default layer 2 persistence option.
 *
 * \param ctrl D channel controller.
 *
 * \return Default layer 2 persistence option. (legacy behaviour default)
 */
static enum pri_layer2_persistence pri_l2_persistence_option_default(struct pri *ctrl)
{
	enum pri_layer2_persistence persistence;

	if (PTMP_MODE(ctrl)) {
		persistence = PRI_L2_PERSISTENCE_LEAVE_DOWN;
	} else {
		persistence = PRI_L2_PERSISTENCE_KEEP_UP;
	}

	return persistence;
}

/*!
 * \internal
 * \brief Determine the default display text send options.
 *
 * \param ctrl D channel controller.
 *
 * \return Default display text send options. (legacy behaviour defaults)
 */
static unsigned long pri_display_options_send_default(struct pri *ctrl)
{
	unsigned long flags;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_QSIG:
		flags = PRI_DISPLAY_OPTION_BLOCK;
		break;
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		if (ctrl->localtype == PRI_CPE) {
			flags = PRI_DISPLAY_OPTION_BLOCK;
			break;
		}
		flags = PRI_DISPLAY_OPTION_NAME_INITIAL;
		break;
	default:
		flags = PRI_DISPLAY_OPTION_NAME_INITIAL;
		break;
	}
	return flags;
}

/*!
 * \internal
 * \brief Determine the default display text receive options.
 *
 * \param ctrl D channel controller.
 *
 * \return Default display text receive options. (legacy behaviour defaults)
 */
static unsigned long pri_display_options_receive_default(struct pri *ctrl)
{
	unsigned long flags;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_QSIG:
		flags = PRI_DISPLAY_OPTION_BLOCK;
		break;
	default:
		flags = PRI_DISPLAY_OPTION_NAME_INITIAL;
		break;
	}
	return flags;
}

/*!
 * \internal
 * \brief Determine the default date/time send option default.
 *
 * \param ctrl D channel controller.
 *
 * \return Default date/time send option.
 */
static int pri_date_time_send_default(struct pri *ctrl)
{
	int date_time_send;

	if (BRI_NT_PTMP(ctrl)) {
		date_time_send = PRI_DATE_TIME_SEND_DATE_HHMM;
	} else {
		date_time_send = PRI_DATE_TIME_SEND_NO;
	}

	return date_time_send;
}

/*!
 * \brief Destroy the given link.
 *
 * \param link Q.921 link to destroy.
 *
 * \return Nothing
 */
void pri_link_destroy(struct q921_link *link)
{
	if (link) {
		struct q931_call *call;

		call = link->dummy_call;
		if (call) {
			pri_schedule_del(call->pri, call->retranstimer);
			call->retranstimer = 0;
			pri_call_apdu_queue_cleanup(call);
		}
		free(link);
	}
}

/*!
 * \internal
 * \brief Initialize the layer 2 link structure.
 *
 * \param ctrl D channel controller.
 * \param link Q.921 link to initialize.
 * \param sapi SAPI new link is to use.
 * \param tei TEI new link is to use.
 *
 * \note It is assumed that the link has already been memset to zero.
 *
 * \return Nothing
 */
static void pri_link_init(struct pri *ctrl, struct q921_link *link, int sapi, int tei)
{
	link->ctrl = ctrl;
	link->sapi = sapi;
	link->tei = tei;
}

/*!
 * \brief Create a new layer 2 link.
 *
 * \param ctrl D channel controller.
 * \param sapi SAPI new link is to use.
 * \param tei TEI new link is to use.
 *
 * \retval link on success.
 * \retval NULL on error.
 */
struct q921_link *pri_link_new(struct pri *ctrl, int sapi, int tei)
{
	struct link_dummy *dummy_link;
	struct q921_link *link;

	switch (ctrl->switchtype) {
	case PRI_SWITCH_GR303_EOC:
	case PRI_SWITCH_GR303_TMC:
		link = calloc(1, sizeof(*link));
		if (!link) {
			return NULL;
		}
		dummy_link = NULL;
		break;
	default:
		dummy_link = calloc(1, sizeof(*dummy_link));
		if (!dummy_link) {
			return NULL;
		}
		link = &dummy_link->link;
		break;
	}

	pri_link_init(ctrl, link, sapi, tei);
	if (dummy_link) {
		/* Initialize the dummy call reference call record. */
		link->dummy_call = &dummy_link->dummy_call;
		q931_init_call_record(link, link->dummy_call, Q931_DUMMY_CALL_REFERENCE);
	}

	q921_start(link);

	return link;
}

/*!
 * \internal
 * \brief Destroy the given D channel controller.
 *
 * \param ctrl D channel control to destroy.
 *
 * \return Nothing
 */
static void pri_ctrl_destroy(struct pri *ctrl)
{
	if (ctrl) {
		struct q931_call *call;

		if (ctrl->link.tei == Q921_TEI_GROUP
			&& ctrl->link.sapi == Q921_SAPI_LAYER2_MANAGEMENT
			&& ctrl->localtype == PRI_CPE) {
			/* This dummy call was borrowed from the specific TEI link. */
			call = NULL;
		} else {
			call = ctrl->link.dummy_call;
		}
		if (call) {
			pri_schedule_del(call->pri, call->retranstimer);
			call->retranstimer = 0;
			pri_call_apdu_queue_cleanup(call);
		}
		free(ctrl->msg_line);
		free(ctrl->sched.timer);
		free(ctrl);
	}
}

/*!
 * \internal
 * \brief Create a new D channel control structure.
 *
 * \param fd D channel file descriptor if no callback functions supplied.
 * \param node Switch NET/CPE type
 * \param switchtype ISDN switch type
 * \param rd D channel read callback function
 * \param wr D channel write callback function
 * \param userdata Callback function parameter
 * \param tei TEI new link is to use.
 * \param bri TRUE if interface is BRI
 *
 * \retval ctrl on success.
 * \retval NULL on error.
 */
static struct pri *pri_ctrl_new(int fd, int node, int switchtype, pri_io_cb rd, pri_io_cb wr, void *userdata, int tei, int bri)
{
	int create_dummy_call;
	struct d_ctrl_dummy *dummy_ctrl;
	struct pri *ctrl;

	switch (switchtype) {
	case PRI_SWITCH_GR303_EOC:
	case PRI_SWITCH_GR303_TMC:
		create_dummy_call = 0;
		break;
	default:
		if (bri && node == PRI_CPE && tei == Q921_TEI_GROUP) {
			/*
			 * BRI TE PTMP will not use its own group dummy call record.  It
			 * will use the specific TEI dummy call instead.
			 */
			create_dummy_call = 0;
		} else {
			create_dummy_call = 1;
		}
		break;
	}
	if (create_dummy_call) {
		dummy_ctrl = calloc(1, sizeof(*dummy_ctrl));
		if (!dummy_ctrl) {
			return NULL;
		}
		ctrl = &dummy_ctrl->ctrl;
	} else {
		ctrl = calloc(1, sizeof(*ctrl));
		if (!ctrl) {
			return NULL;
		}
		dummy_ctrl = NULL;
	}
	ctrl->msg_line = calloc(1, sizeof(*ctrl->msg_line));
	if (!ctrl->msg_line) {
		free(ctrl);
		return NULL;
	}

	ctrl->bri = bri;
	ctrl->fd = fd;
	ctrl->read_func = rd;
	ctrl->write_func = wr;
	ctrl->userdata = userdata;
	ctrl->localtype = node;
	ctrl->switchtype = switchtype;
	ctrl->cref = 1;
	ctrl->nsf = PRI_NSF_NONE;
	ctrl->callpool = &ctrl->localpool;
	pri_default_timers(ctrl, switchtype);
	ctrl->q921_rxcount = 0;
	ctrl->q921_txcount = 0;
	ctrl->q931_rxcount = 0;
	ctrl->q931_txcount = 0;

	ctrl->l2_persistence = pri_l2_persistence_option_default(ctrl);
	ctrl->display_flags.send = pri_display_options_send_default(ctrl);
	ctrl->display_flags.receive = pri_display_options_receive_default(ctrl);
	switch (switchtype) {
	case PRI_SWITCH_GR303_EOC:
		ctrl->protodisc = GR303_PROTOCOL_DISCRIMINATOR;
		pri_link_init(ctrl, &ctrl->link, Q921_SAPI_GR303_EOC, Q921_TEI_GR303_EOC_OPS);
		ctrl->link.next = pri_link_new(ctrl, Q921_SAPI_GR303_EOC, Q921_TEI_GR303_EOC_PATH);
		if (!ctrl->link.next) {
			pri_ctrl_destroy(ctrl);
			return NULL;
		}
		break;
	case PRI_SWITCH_GR303_TMC:
		ctrl->protodisc = GR303_PROTOCOL_DISCRIMINATOR;
		pri_link_init(ctrl, &ctrl->link, Q921_SAPI_GR303_TMC_CALLPROC, Q921_TEI_GR303_TMC_CALLPROC);
		ctrl->link.next = pri_link_new(ctrl, Q921_SAPI_GR303_TMC_SWITCHING, Q921_TEI_GR303_TMC_SWITCHING);
		if (!ctrl->link.next) {
			pri_ctrl_destroy(ctrl);
			return NULL;
		}
		break;
	default:
		ctrl->protodisc = Q931_PROTOCOL_DISCRIMINATOR;
		pri_link_init(ctrl, &ctrl->link,
			(tei == Q921_TEI_GROUP) ? Q921_SAPI_LAYER2_MANAGEMENT : Q921_SAPI_CALL_CTRL,
			tei);
		break;
	}
	ctrl->date_time_send = pri_date_time_send_default(ctrl);
	if (dummy_ctrl) {
		/* Initialize the dummy call reference call record. */
		ctrl->link.dummy_call = &dummy_ctrl->dummy_call;
		q931_init_call_record(&ctrl->link, ctrl->link.dummy_call,
			Q931_DUMMY_CALL_REFERENCE);
	}

	if (ctrl->link.tei == Q921_TEI_GROUP && ctrl->link.sapi == Q921_SAPI_LAYER2_MANAGEMENT
		&& ctrl->localtype == PRI_CPE) {
		ctrl->link.next = pri_link_new(ctrl, Q921_SAPI_CALL_CTRL, Q921_TEI_PRI);
		if (!ctrl->link.next) {
			pri_ctrl_destroy(ctrl);
			return NULL;
		}
		/*
		 * Make the group link use the just created specific TEI link
		 * dummy call instead.  It makes no sense for TE PTMP interfaces
		 * to broadcast messages on the dummy call or to broadcast any
		 * messages for that matter.
		 */
		ctrl->link.dummy_call = ctrl->link.next->dummy_call;
	} else {
		q921_start(&ctrl->link);
	}

	return ctrl;
}

void pri_call_set_useruser(q931_call *c, const char *userchars)
{
	/*
	 * There is a slight risk here if c is actually stale.  However,
	 * if it is stale then it is better to catch it here than to
	 * write with it.
	 */
	if (!userchars || !pri_is_call_valid(NULL, c)) {
		return;
	}
	libpri_copy_string(c->useruserinfo, userchars, sizeof(c->useruserinfo));
}

void pri_sr_set_useruser(struct pri_sr *sr, const char *userchars)
{
	sr->useruserinfo = userchars;
}

int pri_restart(struct pri *pri)
{
	/* pri_restart() is no longer needed since the Q.921 rewrite. */
#if 0
	/* Restart Q.921 layer */
	if (pri) {
		q921_reset(pri, 1);
		q921_start(pri, pri->localtype == PRI_CPE);	
	}
#endif
	return 0;
}

struct pri *pri_new(int fd, int nodetype, int switchtype)
{
	return pri_ctrl_new(fd, nodetype, switchtype, __pri_read, __pri_write, NULL, Q921_TEI_PRI, 0);
}

struct pri *pri_new_bri(int fd, int ptpmode, int nodetype, int switchtype)
{
	if (ptpmode)
		return pri_ctrl_new(fd, nodetype, switchtype, __pri_read, __pri_write, NULL, Q921_TEI_PRI, 1);
	else
		return pri_ctrl_new(fd, nodetype, switchtype, __pri_read, __pri_write, NULL, Q921_TEI_GROUP, 1);
}

struct pri *pri_new_cb(int fd, int nodetype, int switchtype, pri_io_cb io_read, pri_io_cb io_write, void *userdata)
{
	if (!io_read)
		io_read = __pri_read;
	if (!io_write)
		io_write = __pri_write;
	return pri_ctrl_new(fd, nodetype, switchtype, io_read, io_write, userdata, Q921_TEI_PRI, 0);
}

struct pri *pri_new_bri_cb(int fd, int ptpmode, int nodetype, int switchtype, pri_io_cb io_read, pri_io_cb io_write, void *userdata)
{
	if (!io_read) {
		io_read = __pri_read;
	}
	if (!io_write) {
		io_write = __pri_write;
	}
	if (ptpmode) {
		return pri_ctrl_new(fd, nodetype, switchtype, io_read, io_write, userdata, Q921_TEI_PRI, 1);
	} else {
		return pri_ctrl_new(fd, nodetype, switchtype, io_read, io_write, userdata, Q921_TEI_GROUP, 1);
	}
}

void *pri_get_userdata(struct pri *pri)
{
	return pri ? pri->userdata : NULL;
}

void pri_set_userdata(struct pri *pri, void *userdata)
{
	if (pri)
		pri->userdata = userdata;
}

void pri_set_nsf(struct pri *pri, int nsf)
{
	if (pri)
		pri->nsf = nsf;
}

char *pri_event2str(int id)
{
	unsigned idx;
	struct {
		int id;
		char *name;
	} events[] = {
/* *INDENT-OFF* */
		{ PRI_EVENT_DCHAN_UP,       "PRI_EVENT_DCHAN_UP" },
		{ PRI_EVENT_DCHAN_DOWN,     "PRI_EVENT_DCHAN_DOWN" },
		{ PRI_EVENT_RESTART,        "PRI_EVENT_RESTART" },
		{ PRI_EVENT_CONFIG_ERR,     "PRI_EVENT_CONFIG_ERR" },
		{ PRI_EVENT_RING,           "PRI_EVENT_RING" },
		{ PRI_EVENT_HANGUP,         "PRI_EVENT_HANGUP" },
		{ PRI_EVENT_RINGING,        "PRI_EVENT_RINGING" },
		{ PRI_EVENT_ANSWER,         "PRI_EVENT_ANSWER" },
		{ PRI_EVENT_HANGUP_ACK,     "PRI_EVENT_HANGUP_ACK" },
		{ PRI_EVENT_RESTART_ACK,    "PRI_EVENT_RESTART_ACK" },
		{ PRI_EVENT_FACILITY,       "PRI_EVENT_FACILITY" },
		{ PRI_EVENT_INFO_RECEIVED,  "PRI_EVENT_INFO_RECEIVED" },
		{ PRI_EVENT_PROCEEDING,     "PRI_EVENT_PROCEEDING" },
		{ PRI_EVENT_SETUP_ACK,      "PRI_EVENT_SETUP_ACK" },
		{ PRI_EVENT_HANGUP_REQ,     "PRI_EVENT_HANGUP_REQ" },
		{ PRI_EVENT_NOTIFY,         "PRI_EVENT_NOTIFY" },
		{ PRI_EVENT_PROGRESS,       "PRI_EVENT_PROGRESS" },
		{ PRI_EVENT_KEYPAD_DIGIT,   "PRI_EVENT_KEYPAD_DIGIT" },
		{ PRI_EVENT_SERVICE,        "PRI_EVENT_SERVICE" },
		{ PRI_EVENT_SERVICE_ACK,    "PRI_EVENT_SERVICE_ACK" },
		{ PRI_EVENT_HOLD,           "PRI_EVENT_HOLD" },
		{ PRI_EVENT_HOLD_ACK,       "PRI_EVENT_HOLD_ACK" },
		{ PRI_EVENT_HOLD_REJ,       "PRI_EVENT_HOLD_REJ" },
		{ PRI_EVENT_RETRIEVE,       "PRI_EVENT_RETRIEVE" },
		{ PRI_EVENT_RETRIEVE_ACK,   "PRI_EVENT_RETRIEVE_ACK" },
		{ PRI_EVENT_RETRIEVE_REJ,   "PRI_EVENT_RETRIEVE_REJ" },
		{ PRI_EVENT_CONNECT_ACK,    "PRI_EVENT_CONNECT_ACK" },
/* *INDENT-ON* */
	};

	for (idx = 0; idx < ARRAY_LEN(events); ++idx) {
		if (events[idx].id == id) {
			return events[idx].name;
		}
	}
	return "Unknown Event";
}

pri_event *pri_check_event(struct pri *pri)
{
	char buf[1024];
	int res;
	pri_event *e;
	res = pri->read_func ? pri->read_func(pri, buf, sizeof(buf)) : 0;
	if (!res)
		return NULL;
	/* Receive the q921 packet */
	e = q921_receive(pri, (q921_h *)buf, res);
	return e;
}

static int wait_pri(struct pri *pri)
{	
	struct timeval *tv, real;
	fd_set fds;
	int res;
	FD_ZERO(&fds);
	FD_SET(pri->fd, &fds);
	tv = pri_schedule_next(pri);
	if (tv) {
		gettimeofday(&real, NULL);
		real.tv_sec = tv->tv_sec - real.tv_sec;
		real.tv_usec = tv->tv_usec - real.tv_usec;
		if (real.tv_usec < 0) {
			real.tv_usec += 1000000;
			real.tv_sec -= 1;
		}
		if (real.tv_sec < 0) {
			real.tv_sec = 0;
			real.tv_usec = 0;
		}
	}
	res = select(pri->fd + 1, &fds, NULL, NULL, tv ? &real : tv);
	if (res < 0) 
		return -1;
	return res;
}

pri_event *pri_mkerror(struct pri *pri, char *errstr)
{
	/* Return a configuration error */
	pri->ev.err.e = PRI_EVENT_CONFIG_ERR;
	libpri_copy_string(pri->ev.err.err, errstr, sizeof(pri->ev.err.err));
	return &pri->ev;
}


pri_event *pri_dchannel_run(struct pri *pri, int block)
{
	pri_event *e;
	int res;
	if (!pri)
		return NULL;
	if (block) {
		do {
			e =  NULL;
			res = wait_pri(pri);
			/* Check for error / interruption */
			if (res < 0)
				return NULL;
			if (!res)
				e = pri_schedule_run(pri);
			else
				e = pri_check_event(pri);
		} while(!e);
	} else {
		e = pri_check_event(pri);
		return e;
	}
	return e;
}

void pri_set_debug(struct pri *pri, int debug)
{
	if (!pri)
		return;
	pri->debug = debug;
}

int pri_get_debug(struct pri *pri)
{
	if (!pri)
		return -1;
	return pri->debug;
}

void pri_facility_enable(struct pri *pri)
{
	if (!pri)
		return;
	pri->sendfacility = 1;
}

int pri_acknowledge(struct pri *pri, q931_call *call, int channel, int info)
{
	if (!pri || !pri_is_call_valid(pri, call)) {
		return -1;
	}
	return q931_alerting(pri, call, channel, info);
}

int pri_proceeding(struct pri *pri, q931_call *call, int channel, int info)
{
	if (!pri || !pri_is_call_valid(pri, call)) {
		return -1;
	}
	return q931_call_proceeding(pri, call, channel, info);
}

int pri_progress_with_cause(struct pri *pri, q931_call *call, int channel, int info, int cause)
{
	if (!pri || !pri_is_call_valid(pri, call)) {
		return -1;
	}

	return q931_call_progress_with_cause(pri, call, channel, info, cause);
}

int pri_progress(struct pri *pri, q931_call *call, int channel, int info)
{
	if (!pri || !pri_is_call_valid(pri, call)) {
		return -1;
	}

	return q931_call_progress(pri, call, channel, info);
}

int pri_information(struct pri *pri, q931_call *call, char digit)
{
	if (!pri || !pri_is_call_valid(pri, call)) {
		return -1;
	}
	return q931_information(pri, call, digit);
}

int pri_keypad_facility(struct pri *pri, q931_call *call, const char *digits)
{
	if (!pri || !pri_is_call_valid(pri, call) || !digits || !digits[0]) {
		return -1;
	}

	return q931_keypad_facility(pri, call, digits);
}

int pri_notify(struct pri *pri, q931_call *call, int channel, int info)
{
	if (!pri || !pri_is_call_valid(pri, call)) {
		return -1;
	}
	return q931_notify(pri, call, channel, info);
}

void pri_destroycall(struct pri *pri, q931_call *call)
{
	if (pri && pri_is_call_valid(pri, call)) {
		q931_destroycall(pri, call);
	}
}

int pri_need_more_info(struct pri *pri, q931_call *call, int channel, int nonisdn)
{
	if (!pri || !pri_is_call_valid(pri, call)) {
		return -1;
	}
	return q931_setup_ack(pri, call, channel, nonisdn);
}

int pri_answer(struct pri *pri, q931_call *call, int channel, int nonisdn)
{
	if (!pri || !pri_is_call_valid(pri, call)) {
		return -1;
	}
	return q931_connect(pri, call, channel, nonisdn);
}

int pri_connect_ack(struct pri *ctrl, q931_call *call, int channel)
{
	if (!ctrl || !pri_is_call_valid(ctrl, call)) {
		return -1;
	}
	return q931_connect_acknowledge(ctrl, call, channel);
}

void pri_connect_ack_enable(struct pri *ctrl, int enable)
{
	if (ctrl) {
		ctrl->manual_connect_ack = enable ? 1 : 0;
	}
}

/*!
 * \brief Copy the PRI party name to the Q.931 party name structure.
 *
 * \param q931_name Q.931 party name structure
 * \param pri_name PRI party name structure
 *
 * \return Nothing
 */
void pri_copy_party_name_to_q931(struct q931_party_name *q931_name, const struct pri_party_name *pri_name)
{
	q931_party_name_init(q931_name);
	if (pri_name->valid) {
		q931_name->valid = 1;
		q931_name->presentation = pri_name->presentation;
		q931_name->char_set = pri_name->char_set;
		libpri_copy_string(q931_name->str, pri_name->str, sizeof(q931_name->str));
	}
}

/*!
 * \brief Copy the PRI party number to the Q.931 party number structure.
 *
 * \param q931_number Q.931 party number structure
 * \param pri_number PRI party number structure
 *
 * \return Nothing
 */
void pri_copy_party_number_to_q931(struct q931_party_number *q931_number, const struct pri_party_number *pri_number)
{
	q931_party_number_init(q931_number);
	if (pri_number->valid) {
		q931_number->valid = 1;
		q931_number->presentation = pri_number->presentation;
		q931_number->plan = pri_number->plan;
		libpri_copy_string(q931_number->str, pri_number->str, sizeof(q931_number->str));
	}
}

/*!
 * \brief Copy the PRI party subaddress to the Q.931 party subaddress structure.
 *
 * \param q931_subaddress Q.931 party subaddress structure
 * \param pri_subaddress PRI party subaddress structure
 *
 * \return Nothing
 */
void pri_copy_party_subaddress_to_q931(struct q931_party_subaddress *q931_subaddress, const struct pri_party_subaddress *pri_subaddress)
{
	int length;
	int maxlen = sizeof(q931_subaddress->data) - 1;

	q931_party_subaddress_init(q931_subaddress);

	if (!pri_subaddress->valid) {
		return;
	}

	q931_subaddress->valid = 1;
	q931_subaddress->type = pri_subaddress->type;

	length = pri_subaddress->length;
	if (length > maxlen){
		length = maxlen;
	} else {
		q931_subaddress->odd_even_indicator = pri_subaddress->odd_even_indicator;
	}
	q931_subaddress->length = length;
	memcpy(q931_subaddress->data, pri_subaddress->data, length);
	q931_subaddress->data[length] = '\0';
}

/*!
 * \brief Copy the PRI party id to the Q.931 party id structure.
 *
 * \param q931_id Q.931 party id structure
 * \param pri_id PRI party id structure
 *
 * \return Nothing
 */
void pri_copy_party_id_to_q931(struct q931_party_id *q931_id, const struct pri_party_id *pri_id)
{
	pri_copy_party_name_to_q931(&q931_id->name, &pri_id->name);
	pri_copy_party_number_to_q931(&q931_id->number, &pri_id->number);
	pri_copy_party_subaddress_to_q931(&q931_id->subaddress, &pri_id->subaddress);
}

int pri_connected_line_update(struct pri *ctrl, q931_call *call, const struct pri_party_connected_line *connected)
{
	struct q931_party_id party_id;
	unsigned idx;
	unsigned new_name;
	unsigned new_number;
	unsigned new_subaddress;
	struct q931_call *subcall;

	if (!ctrl || !pri_is_call_valid(ctrl, call)) {
		return -1;
	}

	pri_copy_party_id_to_q931(&party_id, &connected->id);
	q931_party_id_fixup(ctrl, &party_id);

	new_name = q931_party_name_cmp(&party_id.name, &call->local_id.name);
	new_number = q931_party_number_cmp(&party_id.number, &call->local_id.number);
	new_subaddress = party_id.subaddress.valid
		&& q931_party_subaddress_cmp(&party_id.subaddress, &call->local_id.subaddress);

	/* Update the call and all subcalls with new local_id. */
	call->local_id = party_id;
	if (call->outboundbroadcast && call->master_call == call) {
		for (idx = 0; idx < ARRAY_LEN(call->subcalls); ++idx) {
			subcall = call->subcalls[idx];
			if (subcall) {
				subcall->local_id = party_id;
			}
		}
	}

	switch (call->ourcallstate) {
	case Q931_CALL_STATE_CALL_INITIATED:
	case Q931_CALL_STATE_OVERLAP_SENDING:
	case Q931_CALL_STATE_OUTGOING_CALL_PROCEEDING:
	case Q931_CALL_STATE_CALL_DELIVERED:
		/*
		 * The local party transferred to someone else before
		 * the remote end answered.
		 */
		switch (ctrl->switchtype) {
		case PRI_SWITCH_EUROISDN_E1:
		case PRI_SWITCH_EUROISDN_T1:
			if (BRI_NT_PTMP(ctrl)) {
				/*
				 * NT PTMP mode
				 *
				 * We should not send these messages to the network if we are
				 * the CPE side since phones do not transfer calls within
				 * themselves.  Well... If you consider handing the handset to
				 * someone else a transfer then how is the network to know?
				 */
				if (new_number) {
					q931_notify_redirection(ctrl, call, PRI_NOTIFY_TRANSFER_ACTIVE,
						&party_id.name, &party_id.number);
				}
				if (new_subaddress || (party_id.subaddress.valid && new_number)) {
					q931_subaddress_transfer(ctrl, call);
				}
			} else if (PTP_MODE(ctrl)) {
				/* PTP mode */
				if (new_number) {
					/* Immediately send EctInform APDU, callStatus=answered(0) */
					send_call_transfer_complete(ctrl, call, 0);
				}
				if (new_subaddress || (party_id.subaddress.valid && new_number)) {
					q931_subaddress_transfer(ctrl, call);
				}
			}
			break;
		case PRI_SWITCH_QSIG:
			if (new_name || new_number) {
				/* Immediately send CallTransferComplete APDU, callStatus=answered(0) */
				send_call_transfer_complete(ctrl, call, 0);
			}
			if (new_subaddress
				|| (party_id.subaddress.valid && (new_name || new_number))) {
				q931_subaddress_transfer(ctrl, call);
			}
			break;
		default:
			break;
		}
		break;
	case Q931_CALL_STATE_ACTIVE:
		switch (ctrl->switchtype) {
		case PRI_SWITCH_EUROISDN_E1:
		case PRI_SWITCH_EUROISDN_T1:
			if (BRI_NT_PTMP(ctrl)) {
				/*
				 * NT PTMP mode
				 *
				 * We should not send these messages to the network if we are
				 * the CPE side since phones do not transfer calls within
				 * themselves.  Well... If you consider handing the handset to
				 * someone else a transfer then how is the network to know?
				 */
				if (new_number) {
#if defined(USE_NOTIFY_FOR_ECT)
					/*
					 * Some ISDN phones only handle the NOTIFY message that the
					 * EN 300-369 spec says should be sent only if the call has not
					 * connected yet.
					 */
					q931_notify_redirection(ctrl, call, PRI_NOTIFY_TRANSFER_ACTIVE,
						&party_id.name, &party_id.number);
#else
					q931_request_subaddress(ctrl, call, PRI_NOTIFY_TRANSFER_ACTIVE,
						&party_id.name, &party_id.number);
#endif	/* defined(USE_NOTIFY_FOR_ECT) */
				}
				if (new_subaddress || (party_id.subaddress.valid && new_number)) {
					q931_subaddress_transfer(ctrl, call);
				}
			} else if (PTP_MODE(ctrl)) {
				/* PTP mode */
				if (new_number) {
					/* Immediately send EctInform APDU, callStatus=answered(0) */
					send_call_transfer_complete(ctrl, call, 0);
				}
				if (new_subaddress || (party_id.subaddress.valid && new_number)) {
					q931_subaddress_transfer(ctrl, call);
				}
			}
			break;
		case PRI_SWITCH_QSIG:
			if (new_name || new_number) {
				/* Immediately send CallTransferComplete APDU, callStatus=answered(0) */
				send_call_transfer_complete(ctrl, call, 0);
			}
			if (new_subaddress
				|| (party_id.subaddress.valid && (new_name || new_number))) {
				q931_subaddress_transfer(ctrl, call);
			}
			break;
		default:
			break;
		}
		break;
	default:
		/* Just save the data for further developments. */
		break;
	}

	return 0;
}

int pri_redirecting_update(struct pri *ctrl, q931_call *call, const struct pri_party_redirecting *redirecting)
{
	unsigned idx;
	struct q931_call *subcall;

	if (!ctrl || !pri_is_call_valid(ctrl, call)) {
		return -1;
	}

	/* Save redirecting.to information and reason. */
	pri_copy_party_id_to_q931(&call->redirecting.to, &redirecting->to);
	q931_party_id_fixup(ctrl, &call->redirecting.to);
	call->redirecting.reason = redirecting->reason;

	/*
	 * Update all subcalls with new redirecting.to information and reason.
	 * I do not think we will ever have any subcalls when this data is relevant,
	 * but update it just in case.
	 */
	if (call->outboundbroadcast && call->master_call == call) {
		for (idx = 0; idx < ARRAY_LEN(call->subcalls); ++idx) {
			subcall = call->subcalls[idx];
			if (subcall) {
				subcall->redirecting.to = call->redirecting.to;
				subcall->redirecting.reason = redirecting->reason;
			}
		}
	}

	switch (call->ourcallstate) {
	case Q931_CALL_STATE_NULL:
		/* Save the remaining redirecting information before we place a call. */
		pri_copy_party_id_to_q931(&call->redirecting.from, &redirecting->from);
		q931_party_id_fixup(ctrl, &call->redirecting.from);
		pri_copy_party_id_to_q931(&call->redirecting.orig_called, &redirecting->orig_called);
		q931_party_id_fixup(ctrl, &call->redirecting.orig_called);
		call->redirecting.orig_reason = redirecting->orig_reason;
		if (redirecting->count <= 0) {
			if (call->redirecting.from.number.valid) {
				/*
				 * We are redirecting with an unknown count
				 * so assume the count is one.
				 */
				call->redirecting.count = 1;
			} else {
				call->redirecting.count = 0;
			}
		} else if (redirecting->count < PRI_MAX_REDIRECTS) {
			call->redirecting.count = redirecting->count;
		} else {
			call->redirecting.count = PRI_MAX_REDIRECTS;
		}
		break;
	case Q931_CALL_STATE_OVERLAP_RECEIVING:
	case Q931_CALL_STATE_INCOMING_CALL_PROCEEDING:
	case Q931_CALL_STATE_CALL_RECEIVED:
		/* This is an incoming call that has not connected yet. */
		if (!call->redirecting.to.number.valid) {
			/* Not being redirected toward valid number data. Ignore. */
			break;
		}

		switch (ctrl->switchtype) {
		case PRI_SWITCH_EUROISDN_E1:
		case PRI_SWITCH_EUROISDN_T1:
			if (PTMP_MODE(ctrl)) {
				if (NT_MODE(ctrl)) {
					/*
					 * NT PTMP mode
					 *
					 * We should not send these messages to the network if we are
					 * the CPE side since phones do not redirect calls within
					 * themselves.  Well... If you consider someone else picking up
					 * the handset a redirection then how is the network to know?
					 */
					q931_notify_redirection(ctrl, call, PRI_NOTIFY_CALL_DIVERTING, NULL,
						&call->redirecting.to.number);
				}
				break;
			}
			/* PTP mode - same behaviour as Q.SIG */
			/* fall through */
		case PRI_SWITCH_QSIG:
			if (call->redirecting.state != Q931_REDIRECTING_STATE_PENDING_TX_DIV_LEG_3
				|| strcmp(call->redirecting.to.number.str, call->called.number.str) != 0) {
				/* immediately send divertingLegInformation1 APDU */
				if (rose_diverting_leg_information1_encode(ctrl, call)
					|| q931_facility(ctrl, call)) {
					pri_message(ctrl,
						"Could not schedule facility message for divertingLegInfo1\n");
				}
			}
			call->redirecting.state = Q931_REDIRECTING_STATE_IDLE;

			/* immediately send divertingLegInformation3 APDU */
			if (rose_diverting_leg_information3_encode(ctrl, call, Q931_FACILITY)
				|| q931_facility(ctrl, call)) {
				pri_message(ctrl,
					"Could not schedule facility message for divertingLegInfo3\n");
			}
			break;
		default:
			break;
		}
		break;
	default:
		pri_message(ctrl, "Ignored redirecting update because call in state %s(%d).\n",
			q931_call_state_str(call->ourcallstate), call->ourcallstate);
		break;
	}

	return 0;
}

#if defined(STATUS_REQUEST_PLACE_HOLDER)
/*!
 * \brief Poll/ping for the status of any "called" party.
 *
 * \param ctrl D channel controller.
 * \param request_id The upper layer's ID number to match with the response in case
 *		there are several requests at the same time.
 * \param req Setup request for "called" party to determine the status.
 *
 * \note
 * There could be one or more PRI_SUBCMD_STATUS_REQ_RSP to the status request
 * depending upon how many endpoints respond to the request.
 * (This includes the timeout termination response.)
 * \note
 * Could be used to poll for the status of call-completion party B.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int pri_status_req(struct pri *ctrl, int request_id, const struct pri_sr *req)
{
	return -1;
}
#endif	/* defined(STATUS_REQUEST_PLACE_HOLDER) */

#if defined(STATUS_REQUEST_PLACE_HOLDER)
/*!
 * \brief Response to a poll/ping request for status of any "called" party by libpri.
 *
 * \param ctrl D channel controller.
 * \param invoke_id ID given by libpri when it requested the party status.
 * \param status free(0)/busy(1)/incompatible(2)
 *
 * \note
 * There could be zero, one, or more responses to the original
 * status request depending upon how many endpoints respond to the request.
 * \note
 * Could be used to poll for the status of call-completion party B.
 *
 * \return Nothing
 */
void pri_status_req_rsp(struct pri *ctrl, int invoke_id, int status)
{
}
#endif	/* defined(STATUS_REQUEST_PLACE_HOLDER) */

#if 0
/* deprecated routines, use pri_hangup */
int pri_release(struct pri *pri, q931_call *call, int cause)
{
	if (!pri || !pri_is_call_valid(pri, call)) {
		return -1;
	}
	return q931_release(pri, call, cause);
}

int pri_disconnect(struct pri *pri, q931_call *call, int cause)
{
	if (!pri || !pri_is_call_valid(pri, call)) {
		return -1;
	}
	return q931_disconnect(pri, call, cause);
}
#endif

int pri_channel_bridge(q931_call *call1, q931_call *call2)
{
	struct q931_call *winner;

	/*
	 * There is a slight risk here if call1 or call2 is actually
	 * stale.  However, if they are stale then it is better to catch
	 * it here than to write with these pointers.
	 */
	if (!pri_is_call_valid(NULL, call1) || !pri_is_call_valid(NULL, call2)) {
		return -1;
	}

	winner = q931_find_winning_call(call1);
	if (!winner) {
		/* Cannot transfer: Call 1 does not have a winner yet. */
		return -1;
	}
	call1 = winner;

	winner = q931_find_winning_call(call2);
	if (!winner) {
		/* Cannot transfer: Call 2 does not have a winner yet. */
		return -1;
	}
	call2 = winner;

	/* Check to see if we're on the same PRI */
	if (call1->pri != call2->pri) {
		return -1;
	}

	/* Check for bearer capability */
	if (call1->bc.transcapability != call2->bc.transcapability)
		return -1;

	switch (call1->pri->switchtype) {
	case PRI_SWITCH_NI2:
	case PRI_SWITCH_LUCENT5E:
	case PRI_SWITCH_ATT4ESS:
		if (eect_initiate_transfer(call1->pri, call1, call2)) {
			return -1;
		}
		break;
	case PRI_SWITCH_DMS100:
		if (rlt_initiate_transfer(call1->pri, call1, call2)) {
			return -1;
		}
		break;
	case PRI_SWITCH_QSIG:
		call1->bridged_call = call2;
		call2->bridged_call = call1;
		if (anfpr_initiate_transfer(call1->pri, call1, call2)) {
			return -1;
		}
		break;
	case PRI_SWITCH_EUROISDN_E1:
	case PRI_SWITCH_EUROISDN_T1:
		if (etsi_initiate_transfer(call1->pri, call1, call2)) {
			return -1;
		}
		break;
	default:
		return -1;
	}
	return 0;
}

void pri_hangup_fix_enable(struct pri *ctrl, int enable)
{
	if (ctrl) {
		ctrl->hangup_fix_enabled = enable ? 1 : 0;
	}
}

int pri_hangup(struct pri *pri, q931_call *call, int cause)
{
	if (!pri || !pri_is_call_valid(pri, call)) {
		return -1;
	}
	if (cause == -1)
		/* normal clear cause */
		cause = PRI_CAUSE_NORMAL_CLEARING;
	return q931_hangup(pri, call, cause);
}

int pri_reset(struct pri *pri, int channel)
{
	if (!pri)
		return -1;
	return q931_restart(pri, channel);
}

int pri_maintenance_service(struct pri *pri, int span, int channel, int changestatus)
{
	if (!pri) {
		return -1;
	}
	return maintenance_service(pri, span, channel, changestatus);
}

q931_call *pri_new_call(struct pri *pri)
{
	if (!pri)
		return NULL;
	return q931_new_call(pri);
}

int pri_is_dummy_call(q931_call *call)
{
	if (!call) {
		return 0;
	}
	return q931_is_dummy_call(call);
}

void pri_dump_event(struct pri *pri, pri_event *e)
{
	if (!pri || !e)
		return;
	pri_message(pri, "Event type: %s (%d)\n", pri_event2str(e->gen.e), e->gen.e);
}

void pri_sr_init(struct pri_sr *req)
{
	memset(req, 0, sizeof(struct pri_sr));
	q931_party_redirecting_init(&req->redirecting);
	q931_party_id_init(&req->caller);
	q931_party_address_init(&req->called);
	req->reversecharge = PRI_REVERSECHARGE_NONE;
}

int pri_sr_set_connection_call_independent(struct pri_sr *req)
{
	if (!req)
		return -1;

	req->cis_call = 1; /* have to set cis_call for all those pesky IEs we need to setup */
	req->cis_auto_disconnect = 1;
	return 0;
}

int pri_sr_set_no_channel_call(struct pri_sr *req)
{
	if (!req) {
		return -1;
	}

	req->cis_call = 1;
	return 0;
}

/* Don't call any other pri functions on this */
int pri_mwi_activate(struct pri *pri, q931_call *c, char *caller, int callerplan, char *callername, int callerpres, char *called,
					int calledplan)
{
	struct pri_sr req;

	if (!pri || !pri_is_call_valid(pri, c)) {
		return -1;
	}

	pri_sr_init(&req);
	pri_sr_set_connection_call_independent(&req);
	pri_sr_set_caller(&req, caller, callername, callerplan, callerpres);
	pri_sr_set_called(&req, called, calledplan, 0);

	if (mwi_message_send(pri, c, &req, 1) < 0) {
		pri_message(pri, "Unable to send MWI activate message\n");
		return -1;
	}
	/* Do more stuff when we figure out that the CISC stuff works */
	return q931_setup(pri, c, &req);
}

int pri_mwi_deactivate(struct pri *pri, q931_call *c, char *caller, int callerplan, char *callername, int callerpres, char *called,
					int calledplan)
{
	struct pri_sr req;

	if (!pri || !pri_is_call_valid(pri, c)) {
		return -1;
	}

	pri_sr_init(&req);
	pri_sr_set_connection_call_independent(&req);
	pri_sr_set_caller(&req, caller, callername, callerplan, callerpres);
	pri_sr_set_called(&req, called, calledplan, 0);

	if(mwi_message_send(pri, c, &req, 0) < 0) {
		pri_message(pri, "Unable to send MWI deactivate message\n");
		return -1;
	}

	return q931_setup(pri, c, &req);
}
	
int pri_setup(struct pri *pri, q931_call *c, struct pri_sr *req)
{
	if (!pri || !pri_is_call_valid(pri, c)) {
		return -1;
	}

	return q931_setup(pri, c, req);
}

int pri_call(struct pri *pri, q931_call *c, int transmode, int channel, int exclusive, 
					int nonisdn, char *caller, int callerplan, char *callername, int callerpres, char *called,
					int calledplan, int ulayer1)
{
	struct pri_sr req;

	if (!pri || !pri_is_call_valid(pri, c)) {
		return -1;
	}

	pri_sr_init(&req);
	pri_sr_set_caller(&req, caller, callername, callerplan, callerpres);
	pri_sr_set_called(&req, called, calledplan, 0);
	req.transmode = transmode;
	req.channel = channel;
	req.exclusive = exclusive;
	req.nonisdn =  nonisdn;
	req.userl1 = ulayer1;
	return q931_setup(pri, c, &req);
}	

static void (*__pri_error)(struct pri *pri, char *stuff);
static void (*__pri_message)(struct pri *pri, char *stuff);

void pri_set_message(void (*func)(struct pri *pri, char *stuff))
{
	__pri_message = func;
}

void pri_set_error(void (*func)(struct pri *pri, char *stuff))
{
	__pri_error = func;
}

static void pri_old_message(struct pri *ctrl, const char *fmt, va_list *ap)
{
	char tmp[1024];

	vsnprintf(tmp, sizeof(tmp), fmt, *ap);
	if (__pri_message)
		__pri_message(ctrl, tmp);
	else
		fputs(tmp, stdout);
}

void pri_message(struct pri *ctrl, const char *fmt, ...)
{
	int added_length;
	va_list ap;

	if (!ctrl || !ctrl->msg_line) {
		/* Just have to do it the old way. */
		va_start(ap, fmt);
		pri_old_message(ctrl, fmt, &ap);
		va_end(ap);
		return;
	}

	va_start(ap, fmt);
	added_length = vsnprintf(ctrl->msg_line->str + ctrl->msg_line->length,
		sizeof(ctrl->msg_line->str) - ctrl->msg_line->length, fmt, ap);
	va_end(ap);
	if (added_length < 0
		|| sizeof(ctrl->msg_line->str) <= ctrl->msg_line->length + added_length) {
		static char truncated_output[] =
			"v-- Error building output or output was truncated. (Next line) --v\n";

		/*
		 * This clause should never need to run because the
		 * output line accumulation buffer is quite large.
		 */

		/* vsnprintf() error or output string was truncated. */
		if (__pri_message) {
			__pri_message(ctrl, truncated_output);
		} else {
			fputs(truncated_output, stdout);
		}

		/* Add a terminating '\n' to force a flush of the line. */
		ctrl->msg_line->length = strlen(ctrl->msg_line->str);
		if (ctrl->msg_line->length) {
			ctrl->msg_line->str[ctrl->msg_line->length - 1] = '\n';
		} else {
			ctrl->msg_line->str[0] = '\n';
			ctrl->msg_line->str[1] = '\0';
		}
	} else {
		ctrl->msg_line->length += added_length;
	}

	if (ctrl->msg_line->length
		&& ctrl->msg_line->str[ctrl->msg_line->length - 1] == '\n') {
		/* The accumulated output line was terminated so send it out. */
		ctrl->msg_line->length = 0;
		if (__pri_message) {
			__pri_message(ctrl, ctrl->msg_line->str);
		} else {
			fputs(ctrl->msg_line->str, stdout);
		}
	}
}

void pri_error(struct pri *pri, const char *fmt, ...)
{
	char tmp[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(tmp, sizeof(tmp), fmt, ap);
	va_end(ap);
	if (__pri_error)
		__pri_error(pri, tmp);
	else
		fputs(tmp, stderr);
}

/* Set overlap mode */
void pri_set_overlapdial(struct pri *pri,int state)
{
	if (pri) {
		pri->overlapdial = state ? 1 : 0;
	}
}

void pri_set_chan_mapping_logical(struct pri *pri, int state)
{
	if (pri && pri->switchtype == PRI_SWITCH_QSIG) {
		pri->chan_mapping_logical = state ? 1 : 0;
	}
}

void pri_set_inbanddisconnect(struct pri *pri, unsigned int enable)
{
	if (pri) {
		pri->acceptinbanddisconnect = (enable != 0);
	}
}

int pri_fd(struct pri *pri)
{
	return pri->fd;
}

/*!
 * \internal
 * \brief Append snprintf output to the given buffer.
 *
 * \param buf Buffer currently filling.
 * \param buf_used Offset into buffer where to put new stuff.
 * \param buf_size Actual buffer size of buf.
 * \param format printf format string.
 *
 * \return Total buffer space used.
 */
static size_t pri_snprintf(char *buf, size_t buf_used, size_t buf_size, const char *format, ...) __attribute__((format(printf, 4, 5)));
static size_t pri_snprintf(char *buf, size_t buf_used, size_t buf_size, const char *format, ...)
{
	va_list args;

	if (buf_used < buf_size) {
		va_start(args, format);
		buf_used += vsnprintf(buf + buf_used, buf_size - buf_used, format, args);
		va_end(args);
	}
	if (buf_size < buf_used) {
		buf_used = buf_size + 1;
	}
	return buf_used;
}

char *pri_dump_info_str(struct pri *ctrl)
{
	char *buf;
	size_t buf_size;
	size_t used;
	struct q921_frame *f;
	struct q921_link *link;
	struct pri_cc_record *cc_record;
	struct q931_call *call;
	unsigned num_calls;
	unsigned num_globals;
	unsigned q921outstanding;
	unsigned idx;
	unsigned long switch_bit;

	if (!ctrl) {
		return NULL;
	}

	buf_size = 4096;	/* This should be bigger than we will ever need. */
	buf = malloc(buf_size);
	if (!buf) {
		return NULL;
	}

	/* Might be nice to format these a little better */
	used = 0;
	used = pri_snprintf(buf, used, buf_size, "Switchtype: %s\n",
		pri_switch2str(ctrl->switchtype));
	used = pri_snprintf(buf, used, buf_size, "Type: %s%s%s\n",
		ctrl->bri ? "BRI " : "",
		pri_node2str(ctrl->localtype),
		PTMP_MODE(ctrl) ? " PTMP" : "");
	used = pri_snprintf(buf, used, buf_size, "Remote type: %s\n",
		pri_node2str(ctrl->remotetype));
	used = pri_snprintf(buf, used, buf_size, "Overlap Dial: %d\n", ctrl->overlapdial);
	used = pri_snprintf(buf, used, buf_size, "Logical Channel Mapping: %d\n",
		ctrl->chan_mapping_logical);
	used = pri_snprintf(buf, used, buf_size, "Timer and counter settings:\n");
	switch_bit = PRI_BIT(ctrl->switchtype);
	for (idx = 0; idx < ARRAY_LEN(pri_timer); ++idx) {
		if (pri_timer[idx].used_by & switch_bit) {
			enum PRI_TIMERS_AND_COUNTERS tmr;

			tmr = pri_timer[idx].number;
			if (0 <= ctrl->timers[tmr]) {
				used = pri_snprintf(buf, used, buf_size, "  %s: %d\n",
					pri_timer[idx].name, ctrl->timers[tmr]);
			}
		}
	}

	/* Remember that Q921 Counters include Q931 packets (and any retransmissions) */
	used = pri_snprintf(buf, used, buf_size, "Q931 RX: %d\n", ctrl->q931_rxcount);
	used = pri_snprintf(buf, used, buf_size, "Q931 TX: %d\n", ctrl->q931_txcount);
	used = pri_snprintf(buf, used, buf_size, "Q921 RX: %d\n", ctrl->q921_rxcount);
	used = pri_snprintf(buf, used, buf_size, "Q921 TX: %d\n", ctrl->q921_txcount);
	for (link = &ctrl->link; link; link = link->next) {
		q921outstanding = 0;
		for (f = link->tx_queue; f; f = f->next) {
			++q921outstanding;
		}
		used = pri_snprintf(buf, used, buf_size, "Q921 Outstanding: %u (TEI=%d)\n",
			q921outstanding, link->tei);
	}

	/* Count the call records in existance.  Useful to check for unreleased calls. */
	num_calls = 0;
	num_globals = 0;
	for (call = *ctrl->callpool; call; call = call->next) {
		if (!(call->cr & ~Q931_CALL_REFERENCE_FLAG)) {
			++num_globals;
			continue;
		}
		++num_calls;
		if (call->outboundbroadcast) {
			used = pri_snprintf(buf, used, buf_size,
				"Master call subcall count: %d\n", q931_get_subcall_count(call));
		}
	}
	used = pri_snprintf(buf, used, buf_size, "Total active-calls:%u global:%u\n",
		num_calls, num_globals);

	/*
	 * List simplified call completion records.
	 *
	 * This should be last in the output because it could overflow
	 * the buffer.
	 */
	used = pri_snprintf(buf, used, buf_size, "CC records:\n");
	for (cc_record = ctrl->cc.pool; cc_record; cc_record = cc_record->next) {
		used = pri_snprintf(buf, used, buf_size,
			"  %ld A:%s B:%s state:%s\n", cc_record->record_id,
			cc_record->party_a.number.valid ? cc_record->party_a.number.str : "",
			cc_record->party_b.number.valid ? cc_record->party_b.number.str : "",
			pri_cc_fsm_state_str(cc_record->state));
	}

	if (buf_size < used) {
		pri_message(ctrl,
			"pri_dump_info_str(): Produced output exceeded buffer capacity. (Truncated)\n");
	}
	return buf;
}

int pri_get_crv(struct pri *pri, q931_call *call, int *callmode)
{
	if (!pri || !pri_is_call_valid(pri, call)) {
		return -1;
	}
	return q931_call_getcrv(pri, call, callmode);
}

int pri_set_crv(struct pri *pri, q931_call *call, int crv, int callmode)
{
	if (!pri || !pri_is_call_valid(pri, call)) {
		return -1;
	}
	return q931_call_setcrv(pri, call, crv, callmode);
}

void pri_enslave(struct pri *master, struct pri *slave)
{
	if (!master || !slave) {
		return;
	}

	if (slave->master) {
		struct pri *swp;

		/* The slave already has a master */
		if (master->master || master->slave) {
			/* The new master has a master or it already has slaves. */
			return;
		}

		/* Swap master and slave. */
		swp = master;
		master = slave;
		slave = swp;
	}

	/*
	 * To have some support for dynamic interfaces, the master NFAS
	 * D channel control structure will always exist even if it is
	 * abandoned/deleted by the upper layer.  The master/slave
	 * pointers ensure that the correct master will be used.
	 */

	master = PRI_NFAS_MASTER(master);
	master->nfas = 1;
	slave->nfas = 1;
	slave->callpool = &master->localpool;

	/* Link the slave to the master on the end of the master's list. */
	slave->master = master;
	slave->slave = NULL;
	for (; master->slave; master = master->slave) {
	}
	master->slave = slave;
}

struct pri_sr *pri_sr_new(void)
{
	struct pri_sr *req;
	req = malloc(sizeof(*req));
	if (req) 
		pri_sr_init(req);
	return req;
}

void pri_sr_free(struct pri_sr *sr)
{
	free(sr);
}

int pri_sr_set_channel(struct pri_sr *sr, int channel, int exclusive, int nonisdn)
{
	sr->channel = channel;
	sr->exclusive = exclusive;
	sr->nonisdn = nonisdn;
	return 0;
}

int pri_sr_set_bearer(struct pri_sr *sr, int transmode, int userl1)
{
	sr->transmode = transmode;
	sr->userl1 = userl1;
	return 0;
}

int pri_sr_set_called(struct pri_sr *sr, char *called, int calledplan, int numcomplete)
{
	q931_party_address_init(&sr->called);
	if (called) {
		sr->called.number.valid = 1;
		sr->called.number.plan = calledplan;
		libpri_copy_string(sr->called.number.str, called, sizeof(sr->called.number.str));
	}
	sr->numcomplete = numcomplete;
	return 0;
}

void pri_sr_set_called_subaddress(struct pri_sr *sr, const struct pri_party_subaddress *subaddress)
{
	pri_copy_party_subaddress_to_q931(&sr->called.subaddress, subaddress);
}

int pri_sr_set_caller(struct pri_sr *sr, char *caller, char *callername, int callerplan, int callerpres)
{
	q931_party_id_init(&sr->caller);
	if (caller) {
		sr->caller.number.valid = 1;
		sr->caller.number.presentation = callerpres;
		sr->caller.number.plan = callerplan;
		libpri_copy_string(sr->caller.number.str, caller, sizeof(sr->caller.number.str));

		if (callername) {
			sr->caller.name.valid = 1;
			sr->caller.name.presentation = callerpres;
			sr->caller.name.char_set = PRI_CHAR_SET_ISO8859_1;
			libpri_copy_string(sr->caller.name.str, callername,
				sizeof(sr->caller.name.str));
		}
	}
	return 0;
}

void pri_sr_set_caller_subaddress(struct pri_sr *sr, const struct pri_party_subaddress *subaddress)
{
	pri_copy_party_subaddress_to_q931(&sr->caller.subaddress, subaddress);
}

void pri_sr_set_caller_party(struct pri_sr *sr, const struct pri_party_id *caller)
{
	pri_copy_party_id_to_q931(&sr->caller, caller);
}

int pri_sr_set_redirecting(struct pri_sr *sr, char *num, int plan, int pres, int reason)
{
	q931_party_redirecting_init(&sr->redirecting);
	if (num && num[0]) {
		sr->redirecting.from.number.valid = 1;
		sr->redirecting.from.number.presentation = pres;
		sr->redirecting.from.number.plan = plan;
		libpri_copy_string(sr->redirecting.from.number.str, num,
			sizeof(sr->redirecting.from.number.str));

		sr->redirecting.count = 1;
		sr->redirecting.reason = reason;
	}
	return 0;
}

void pri_sr_set_redirecting_parties(struct pri_sr *sr, const struct pri_party_redirecting *redirecting)
{
	pri_copy_party_id_to_q931(&sr->redirecting.from, &redirecting->from);
	pri_copy_party_id_to_q931(&sr->redirecting.to, &redirecting->to);
	pri_copy_party_id_to_q931(&sr->redirecting.orig_called, &redirecting->orig_called);
	sr->redirecting.orig_reason = redirecting->orig_reason;
	sr->redirecting.reason = redirecting->reason;
	if (redirecting->count <= 0) {
		if (sr->redirecting.from.number.valid) {
			/*
			 * We are redirecting with an unknown count
			 * so assume the count is one.
			 */
			sr->redirecting.count = 1;
		} else {
			sr->redirecting.count = 0;
		}
	} else if (redirecting->count < PRI_MAX_REDIRECTS) {
		sr->redirecting.count = redirecting->count;
	} else {
		sr->redirecting.count = PRI_MAX_REDIRECTS;
	}
}

void pri_sr_set_reversecharge(struct pri_sr *sr, int requested)
{
	sr->reversecharge = requested;
}

void pri_sr_set_keypad_digits(struct pri_sr *sr, const char *keypad_digits)
{
	sr->keypad_digits = keypad_digits;
}

void pri_transfer_enable(struct pri *ctrl, int enable)
{
	if (ctrl) {
		ctrl->transfer_support = enable ? 1 : 0;
	}
}

void pri_hold_enable(struct pri *ctrl, int enable)
{
	if (ctrl) {
		ctrl->hold_support = enable ? 1 : 0;
	}
}

int pri_hold(struct pri *ctrl, q931_call *call)
{
	if (!ctrl || !pri_is_call_valid(ctrl, call)) {
		return -1;
	}
	return q931_send_hold(ctrl, call);
}

int pri_hold_ack(struct pri *ctrl, q931_call *call)
{
	if (!ctrl || !pri_is_call_valid(ctrl, call)) {
		return -1;
	}
	return q931_send_hold_ack(ctrl, call);
}

int pri_hold_rej(struct pri *ctrl, q931_call *call, int cause)
{
	if (!ctrl || !pri_is_call_valid(ctrl, call)) {
		return -1;
	}
	return q931_send_hold_rej(ctrl, call, cause);
}

int pri_retrieve(struct pri *ctrl, q931_call *call, int channel)
{
	if (!ctrl || !pri_is_call_valid(ctrl, call)) {
		return -1;
	}
	return q931_send_retrieve(ctrl, call, channel);
}

int pri_retrieve_ack(struct pri *ctrl, q931_call *call, int channel)
{
	if (!ctrl || !pri_is_call_valid(ctrl, call)) {
		return -1;
	}
	return q931_send_retrieve_ack(ctrl, call, channel);
}

int pri_retrieve_rej(struct pri *ctrl, q931_call *call, int cause)
{
	if (!ctrl || !pri_is_call_valid(ctrl, call)) {
		return -1;
	}
	return q931_send_retrieve_rej(ctrl, call, cause);
}

int pri_callrerouting_facility(struct pri *pri, q931_call *call, const char *dest, const char* original, const char* reason)
{
	if (!pri || !pri_is_call_valid(pri, call) || !dest) {
		return -1;
	}

	return qsig_cf_callrerouting(pri, call, dest, original, reason);
}

void pri_reroute_enable(struct pri *ctrl, int enable)
{
	if (ctrl) {
		ctrl->deflection_support = enable ? 1 : 0;
	}
}

int pri_reroute_call(struct pri *ctrl, q931_call *call, const struct pri_party_id *caller, const struct pri_party_redirecting *deflection, int subscription_option)
{
	const struct q931_party_id *caller_id;
	struct q931_party_id local_caller;
	struct q931_party_redirecting reroute;

	if (!ctrl || !pri_is_call_valid(ctrl, call) || !deflection) {
		return -1;
	}

	if (caller) {
		/* Convert the caller update information. */
		pri_copy_party_id_to_q931(&local_caller, caller);
		q931_party_id_fixup(ctrl, &local_caller);
		caller_id = &local_caller;
	} else {
		caller_id = NULL;
	}

	/* Convert the deflection information. */
	q931_party_redirecting_init(&reroute);
	pri_copy_party_id_to_q931(&reroute.from, &deflection->from);
	q931_party_id_fixup(ctrl, &reroute.from);
	pri_copy_party_id_to_q931(&reroute.to, &deflection->to);
	q931_party_id_fixup(ctrl, &reroute.to);
	pri_copy_party_id_to_q931(&reroute.orig_called, &deflection->orig_called);
	q931_party_id_fixup(ctrl, &reroute.orig_called);
	reroute.reason = deflection->reason;
	reroute.orig_reason = deflection->orig_reason;
	if (deflection->count <= 0) {
		/*
		 * We are deflecting with an unknown count
		 * so assume the count is one.
		 */
		reroute.count = 1;
	} else if (deflection->count < PRI_MAX_REDIRECTS) {
		reroute.count = deflection->count;
	} else {
		reroute.count = PRI_MAX_REDIRECTS;
	}

	return send_reroute_request(ctrl, call, caller_id, &reroute, subscription_option);
}

void pri_cc_enable(struct pri *ctrl, int enable)
{
	if (ctrl) {
		ctrl->cc_support = enable ? 1 : 0;
	}
}

void pri_cc_recall_mode(struct pri *ctrl, int mode)
{
	if (ctrl) {
		ctrl->cc.option.recall_mode = mode ? 1 : 0;
	}
}

void pri_cc_retain_signaling_req(struct pri *ctrl, int signaling_retention)
{
	if (ctrl && 0 <= signaling_retention && signaling_retention < 3) {
		ctrl->cc.option.signaling_retention_req = signaling_retention;
	}
}

void pri_cc_retain_signaling_rsp(struct pri *ctrl, int signaling_retention)
{
	if (ctrl) {
		ctrl->cc.option.signaling_retention_rsp = signaling_retention ? 1 : 0;
	}
}

void pri_persistent_layer2_option(struct pri *ctrl, enum pri_layer2_persistence option)
{
	if (!ctrl) {
		return;
	}
	if (PTMP_MODE(ctrl)) {
		switch (option) {
		case PRI_L2_PERSISTENCE_DEFAULT:
			ctrl->l2_persistence = pri_l2_persistence_option_default(ctrl);
			break;
		case PRI_L2_PERSISTENCE_KEEP_UP:
		case PRI_L2_PERSISTENCE_LEAVE_DOWN:
			ctrl->l2_persistence = option;
			break;
		}
		if (ctrl->l2_persistence == PRI_L2_PERSISTENCE_KEEP_UP) {
			q921_bring_layer2_up(ctrl);
		}
	}
}

void pri_display_options_send(struct pri *ctrl, unsigned long flags)
{
	if (!ctrl) {
		return;
	}
	if (!flags) {
		flags = pri_display_options_send_default(ctrl);
	}
	ctrl->display_flags.send = flags;
}

void pri_display_options_receive(struct pri *ctrl, unsigned long flags)
{
	if (!ctrl) {
		return;
	}
	if (!flags) {
		flags = pri_display_options_receive_default(ctrl);
	}
	ctrl->display_flags.receive = flags;
}

int pri_display_text(struct pri *ctrl, q931_call *call, const struct pri_subcmd_display_txt *display)
{
	if (!ctrl || !display || display->length <= 0
		|| sizeof(display->text) < display->length || !pri_is_call_valid(ctrl, call)) {
		/* Parameter sanity checks failed. */
		return -1;
	}
	return q931_display_text(ctrl, call, display);
}

void pri_date_time_send_option(struct pri *ctrl, int option)
{
	if (!ctrl) {
		return;
	}
	switch (option) {
	case PRI_DATE_TIME_SEND_DEFAULT:
		ctrl->date_time_send = pri_date_time_send_default(ctrl);
		break;
	default:
	case PRI_DATE_TIME_SEND_NO:
		ctrl->date_time_send = PRI_DATE_TIME_SEND_NO;
		break;
	case PRI_DATE_TIME_SEND_DATE:
	case PRI_DATE_TIME_SEND_DATE_HH:
	case PRI_DATE_TIME_SEND_DATE_HHMM:
	case PRI_DATE_TIME_SEND_DATE_HHMMSS:
		if (NT_MODE(ctrl)) {
			/* Only networks may send date/time ie. */
			ctrl->date_time_send = option;
		} else {
			ctrl->date_time_send = PRI_DATE_TIME_SEND_NO;
		}
		break;
	}
}
