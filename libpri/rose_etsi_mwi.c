/*
 * libpri: An implementation of Primary Rate ISDN
 *
 * Copyright (C) 2010 Digium, Inc.
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
 * \brief ROSE Message Waiting Indication (MWI) operations
 *
 * Message Waiting Indication (MWI) supplementary service EN 300 745-1
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
 * \brief Encode the MessageID type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param msg_id
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_etsi_message_id(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct roseEtsiMessageID *msg_id)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER, msg_id->reference_number));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED, msg_id->status));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the MWIActivate invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_MWIActivate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiMWIActivate_ARG *mwi_activate;
	unsigned char *seq_len;
	unsigned char *explicit_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	mwi_activate = &args->etsi.MWIActivate;
	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
		&mwi_activate->receiving_user_number));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		mwi_activate->basic_service));
	if (mwi_activate->controlling_user_number.length) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1);
		ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
			&mwi_activate->controlling_user_number));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}
	if (mwi_activate->number_of_messages_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2);
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
			mwi_activate->number_of_messages));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}
	if (mwi_activate->controlling_user_provided_number.length) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 3);
		ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
			&mwi_activate->controlling_user_provided_number));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}
	if (mwi_activate->time_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 4);
		ASN1_CALL(pos, asn1_enc_string_max(pos, end, ASN1_TYPE_GENERALIZED_TIME,
			mwi_activate->time.str, sizeof(mwi_activate->time.str) - 1));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}
	if (mwi_activate->message_id_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 5);
		ASN1_CALL(pos, rose_enc_etsi_message_id(ctrl, pos, end,
			&mwi_activate->message_id));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}
	if (mwi_activate->mode_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 6);
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
			mwi_activate->mode));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the MWIDeactivate invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_MWIDeactivate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiMWIDeactivate_ARG *mwi_deactivate;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	mwi_deactivate = &args->etsi.MWIDeactivate;
	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
		&mwi_deactivate->receiving_user_number));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		mwi_deactivate->basic_service));
	if (mwi_deactivate->controlling_user_number.length) {
		ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
			&mwi_deactivate->controlling_user_number));
	}
	if (mwi_deactivate->mode_present) {
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
			mwi_deactivate->mode));
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the MWIIndicate invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_etsi_MWIIndicate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseEtsiMWIIndicate_ARG *mwi_indicate;
	unsigned char *seq_len;
	unsigned char *explicit_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	mwi_indicate = &args->etsi.MWIIndicate;
	if (mwi_indicate->controlling_user_number.length) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1);
		ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
			&mwi_indicate->controlling_user_number));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}
	if (mwi_indicate->basic_service_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2);
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
			mwi_indicate->basic_service));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}
	if (mwi_indicate->number_of_messages_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 3);
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER,
			mwi_indicate->number_of_messages));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}
	if (mwi_indicate->controlling_user_provided_number.length) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 4);
		ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
			&mwi_indicate->controlling_user_provided_number));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}
	if (mwi_indicate->time_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 5);
		ASN1_CALL(pos, asn1_enc_string_max(pos, end, ASN1_TYPE_GENERALIZED_TIME,
			mwi_indicate->time.str, sizeof(mwi_indicate->time.str) - 1));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}
	if (mwi_indicate->message_id_present) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 6);
		ASN1_CALL(pos, rose_enc_etsi_message_id(ctrl, pos, end,
			&mwi_indicate->message_id));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \internal
 * \brief Decode the MessageID argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param msg_id Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_etsi_message_id(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseEtsiMessageID *msg_id)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s MessageID %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "messageRef", tag, pos, seq_end, &value));
	msg_id->reference_number = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "status", tag, pos, seq_end, &value));
	msg_id->status = value;

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the MWIActivate invoke argument parameters.
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
const unsigned char *rose_dec_etsi_MWIActivate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	int32_t value;
	size_t str_len;
	int length;
	int seq_offset;
	int explicit_offset;
	const unsigned char *explicit_end;
	const unsigned char *seq_end;
	const unsigned char *save_pos;
	struct roseEtsiMWIActivate_ARG *mwi_activate;

	mwi_activate = &args->etsi.MWIActivate;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  MWIActivate %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "receivingUserNr", tag, pos, seq_end,
		&mwi_activate->receiving_user_number));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "basicService", tag, pos, seq_end, &value));
	mwi_activate->basic_service = value;

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	mwi_activate->controlling_user_number.length = 0;
	mwi_activate->number_of_messages_present = 0;
	mwi_activate->controlling_user_provided_number.length = 0;
	mwi_activate->time_present = 0;
	mwi_activate->message_id_present = 0;
	mwi_activate->mode_present = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag) {
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 1:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "controllingUserNr", tag, pos,
				explicit_end, &mwi_activate->controlling_user_number));

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 2:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
			ASN1_CALL(pos, asn1_dec_int(ctrl, "numberOfMessages", tag, pos, explicit_end,
				&value));
			mwi_activate->number_of_messages = value;
			mwi_activate->number_of_messages_present = 1;

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 3:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "controllingUserProvidedNr", tag,
				pos, explicit_end, &mwi_activate->controlling_user_provided_number));

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 4:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CHECK_TAG(ctrl, tag & ~ASN1_PC_MASK, tag, ASN1_TYPE_GENERALIZED_TIME);
			ASN1_CALL(pos, asn1_dec_string_max(ctrl, "time", tag, pos, explicit_end,
				sizeof(mwi_activate->time.str), mwi_activate->time.str, &str_len));
			mwi_activate->time_present = 1;

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 5:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_etsi_message_id(ctrl, "messageId", tag, pos,
				explicit_end, &mwi_activate->message_id));
			mwi_activate->message_id_present = 1;

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 6:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
			ASN1_CALL(pos, asn1_dec_int(ctrl, "mode", tag, pos, explicit_end, &value));
			mwi_activate->mode = value;
			mwi_activate->mode_present = 1;

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
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
 * \brief Decode the MWIDeactivate invoke argument parameters.
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
const unsigned char *rose_dec_etsi_MWIDeactivate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	struct roseEtsiMWIDeactivate_ARG *mwi_deactivate;

	mwi_deactivate = &args->etsi.MWIDeactivate;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  MWIDeactivate %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "receivingUserNr", tag, pos, seq_end,
		&mwi_deactivate->receiving_user_number));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "basicService", tag, pos, seq_end, &value));
	mwi_deactivate->basic_service = value;

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	mwi_deactivate->controlling_user_number.length = 0;
	mwi_deactivate->mode_present = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag) {
		default:
			ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "controllingUserNr", tag, pos,
				seq_end, &mwi_deactivate->controlling_user_number));
			break;
		case ASN1_TYPE_ENUMERATED:
			ASN1_CALL(pos, asn1_dec_int(ctrl, "mode", tag, pos, seq_end, &value));
			mwi_deactivate->mode = value;
			mwi_deactivate->mode_present = 1;
			break;
		}
	}

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the MWIIndicate invoke argument parameters.
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
const unsigned char *rose_dec_etsi_MWIIndicate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end,
	union rose_msg_invoke_args *args)
{
	int32_t value;
	size_t str_len;
	int length;
	int seq_offset;
	int explicit_offset;
	const unsigned char *explicit_end;
	const unsigned char *seq_end;
	const unsigned char *save_pos;
	struct roseEtsiMWIIndicate_ARG *mwi_indicate;

	mwi_indicate = &args->etsi.MWIIndicate;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  MWIIndicate %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	mwi_indicate->controlling_user_number.length = 0;
	mwi_indicate->basic_service_present = 0;
	mwi_indicate->number_of_messages_present = 0;
	mwi_indicate->controlling_user_provided_number.length = 0;
	mwi_indicate->time_present = 0;
	mwi_indicate->message_id_present = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag) {
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 1:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "controllingUserNr", tag, pos,
				explicit_end, &mwi_indicate->controlling_user_number));

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 2:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
			ASN1_CALL(pos, asn1_dec_int(ctrl, "basicService", tag, pos, explicit_end,
				&value));
			mwi_indicate->basic_service = value;
			mwi_indicate->basic_service_present = 1;

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 3:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
			ASN1_CALL(pos, asn1_dec_int(ctrl, "numberOfMessages", tag, pos, explicit_end,
				&value));
			mwi_indicate->number_of_messages = value;
			mwi_indicate->number_of_messages_present = 1;

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 4:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "controllingUserProvidedNr", tag,
				pos, explicit_end, &mwi_indicate->controlling_user_provided_number));

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 5:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CHECK_TAG(ctrl, tag & ~ASN1_PC_MASK, tag, ASN1_TYPE_GENERALIZED_TIME);
			ASN1_CALL(pos, asn1_dec_string_max(ctrl, "time", tag, pos, explicit_end,
				sizeof(mwi_indicate->time.str), mwi_indicate->time.str, &str_len));
			mwi_indicate->time_present = 1;

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 6:
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_etsi_message_id(ctrl, "messageId", tag, pos,
				explicit_end, &mwi_indicate->message_id));
			mwi_indicate->message_id_present = 1;

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
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


/* ------------------------------------------------------------------- */
/* end rose_etsi_mwi.c */
