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
 * \brief Q.SIG ROSE SS-MWI-Operations
 *
 * SS-MWI-Operations ECMA-242 Annex E Table E.1
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
 * \brief Encode the MsgCentreId type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param msg_centre_id
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_qsig_MsgCentreId(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct roseQsigMsgCentreId *msg_centre_id)
{
	unsigned char *seq_len;

	switch (msg_centre_id->type) {
	case 0:	/* integer */
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 0,
			msg_centre_id->u.integer));
		break;
	case 1:	/* partyNumber */
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1);
		ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end, &msg_centre_id->u.number));
		ASN1_CONSTRUCTED_END(seq_len, pos, end);
		break;
	case 2:	/* numericString */
		ASN1_CALL(pos, asn1_enc_string_max(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2,
			msg_centre_id->u.str, sizeof(msg_centre_id->u.str) - 1));
		break;
	default:
		ASN1_ENC_ERROR(ctrl, "Unknown MsgCentreId type");
		return NULL;
	}

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
unsigned char *rose_enc_qsig_MWIActivate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseQsigMWIActivateArg *mwi_activate;
	unsigned char *seq_len;
	unsigned char *explicit_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	mwi_activate = &args->qsig.MWIActivate;
	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
		&mwi_activate->served_user_number));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		mwi_activate->basic_service));
	if (mwi_activate->msg_centre_id_present) {
		ASN1_CALL(pos, rose_enc_qsig_MsgCentreId(ctrl, pos, end,
			&mwi_activate->msg_centre_id));
	}
	if (mwi_activate->number_of_messages_present) {
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 3,
			mwi_activate->number_of_messages));
	}
	if (mwi_activate->originating_number.length) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 4);
		ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
			&mwi_activate->originating_number));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}
	if (mwi_activate->timestamp_present) {
		ASN1_CALL(pos, asn1_enc_string_max(pos, end, ASN1_TYPE_GENERALIZED_TIME,
			mwi_activate->timestamp.str, sizeof(mwi_activate->timestamp.str) - 1));
	}
	if (mwi_activate->priority_present) {
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 5,
			mwi_activate->priority));
	}

	/* No extension to encode */

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
unsigned char *rose_enc_qsig_MWIDeactivate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseQsigMWIDeactivateArg *mwi_deactivate;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	mwi_deactivate = &args->qsig.MWIDeactivate;
	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
		&mwi_deactivate->served_user_number));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		mwi_deactivate->basic_service));
	if (mwi_deactivate->msg_centre_id_present) {
		ASN1_CALL(pos, rose_enc_qsig_MsgCentreId(ctrl, pos, end,
			&mwi_deactivate->msg_centre_id));
	}

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the MWIInterrogate invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_MWIInterrogate_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	const struct roseQsigMWIInterrogateArg *mwi_interrogate;
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	mwi_interrogate = &args->qsig.MWIInterrogate;
	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
		&mwi_interrogate->served_user_number));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		mwi_interrogate->basic_service));
	if (mwi_interrogate->msg_centre_id_present) {
		ASN1_CALL(pos, rose_enc_qsig_MsgCentreId(ctrl, pos, end,
			&mwi_interrogate->msg_centre_id));
	}

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \internal
 * \brief Encode the MWIInterrogateResElt type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param record
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_qsig_MWIInterrogateResElt(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, unsigned tag,
	const struct roseQsigMWIInterrogateResElt *record)
{
	unsigned char *seq_len;
	unsigned char *explicit_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED, record->basic_service));
	if (record->msg_centre_id_present) {
		ASN1_CALL(pos, rose_enc_qsig_MsgCentreId(ctrl, pos, end,
			&record->msg_centre_id));
	}
	if (record->number_of_messages_present) {
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 3,
			record->number_of_messages));
	}
	if (record->originating_number.length) {
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(explicit_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 4);
		ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end,
			&record->originating_number));
		ASN1_CONSTRUCTED_END(explicit_len, pos, end);
	}
	if (record->timestamp_present) {
		ASN1_CALL(pos, asn1_enc_string_max(pos, end, ASN1_TYPE_GENERALIZED_TIME,
			record->timestamp.str, sizeof(record->timestamp.str) - 1));
	}
	if (record->priority_present) {
		ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 5,
			record->priority));
	}

	/* No extension to encode */

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the MWIInterrogate result facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_MWIInterrogate_RES(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_result_args *args)
{
	unsigned index;
	unsigned char *seq_len;
	const struct roseQsigMWIInterrogateRes *mwi_interrogate;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

	mwi_interrogate = &args->qsig.MWIInterrogate;
	for (index = 0; index < mwi_interrogate->num_records; ++index) {
		ASN1_CALL(pos, rose_enc_qsig_MWIInterrogateResElt(ctrl, pos, end,
			ASN1_TAG_SEQUENCE, &mwi_interrogate->list[index]));
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \internal
 * \brief Decode the MsgCentreId argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param msg_centre_id Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_qsig_MsgCentreId(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigMsgCentreId *msg_centre_id)
{
	int32_t value;
	size_t str_len;
	int length;
	int explicit_offset;
	const unsigned char *explicit_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s MsgCentreId\n", name);
	}
	switch (tag) {
	case ASN1_CLASS_CONTEXT_SPECIFIC | 0:
		msg_centre_id->type = 0;	/* integer */
		ASN1_CALL(pos, asn1_dec_int(ctrl, "integer", tag, pos, end, &value));
		msg_centre_id->u.integer = value;
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 1:
		msg_centre_id->type = 1;	/* partyNumber */

		/* Remove EXPLICIT tag */
		if (ctrl->debug & PRI_DEBUG_APDU) {
			pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
		}
		ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
		ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, end);

		ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
		ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "partyNumber", tag, pos, explicit_end,
			&msg_centre_id->u.number));

		ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, end);
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 2:
	case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 2:
		msg_centre_id->type = 2;	/* numericString */
		ASN1_CALL(pos, asn1_dec_string_max(ctrl, "numericString", tag, pos, end,
			sizeof(msg_centre_id->u.str), msg_centre_id->u.str, &str_len));
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	return pos;
}

/*!
 * \brief Decode the Q.SIG MWIActivate invoke argument parameters.
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
const unsigned char *rose_dec_qsig_MWIActivate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;
	size_t str_len;
	int length;
	int seq_offset;
	int explicit_offset;
	const unsigned char *explicit_end;
	const unsigned char *seq_end;
	const unsigned char *save_pos;
	struct roseQsigMWIActivateArg *mwi_activate;

	mwi_activate = &args->qsig.MWIActivate;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  MWIActivateArg %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "servedUserNr", tag, pos, seq_end,
		&mwi_activate->served_user_number));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "basicService", tag, pos, seq_end, &value));
	mwi_activate->basic_service = value;

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	mwi_activate->msg_centre_id_present = 0;
	mwi_activate->number_of_messages_present = 0;
	mwi_activate->originating_number.length = 0;
	mwi_activate->timestamp_present = 0;
	mwi_activate->priority_present = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag & ~ASN1_PC_MASK) {
		case ASN1_CLASS_CONTEXT_SPECIFIC | 0:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 2:
			ASN1_CALL(pos, rose_dec_qsig_MsgCentreId(ctrl, "msgCentreId", tag, pos,
				seq_end, &mwi_activate->msg_centre_id));
			mwi_activate->msg_centre_id_present = 1;
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 3:
			/* Must not be constructed but we will not check for it for simplicity. */
			ASN1_CALL(pos, asn1_dec_int(ctrl, "nbOfMessages", tag, pos, seq_end,
				&value));
			mwi_activate->number_of_messages = value;
			mwi_activate->number_of_messages_present = 1;
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 4:
			/* Must be constructed but we will not check for it for simplicity. */
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "originatingNr", tag, pos,
				explicit_end, &mwi_activate->originating_number));

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_TYPE_GENERALIZED_TIME:
			ASN1_CALL(pos, asn1_dec_string_max(ctrl, "timestamp", tag, pos, end,
				sizeof(mwi_activate->timestamp.str), mwi_activate->timestamp.str,
				&str_len));
			mwi_activate->timestamp_present = 1;
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 5:
			/* Must not be constructed but we will not check for it for simplicity. */
			ASN1_CALL(pos, asn1_dec_int(ctrl, "priority", tag, pos, seq_end, &value));
			mwi_activate->priority = value;
			mwi_activate->priority_present = 1;
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 6:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 7:
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  argumentExt %s\n", asn1_tag2str(tag));
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
 * \brief Decode the Q.SIG MWIDeactivate invoke argument parameters.
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
const unsigned char *rose_dec_qsig_MWIDeactivate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	const unsigned char *save_pos;
	struct roseQsigMWIDeactivateArg *mwi_deactivate;

	mwi_deactivate = &args->qsig.MWIDeactivate;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  MWIDeactivateArg %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "servedUserNr", tag, pos, seq_end,
		&mwi_deactivate->served_user_number));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "basicService", tag, pos, seq_end, &value));
	mwi_deactivate->basic_service = value;

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	mwi_deactivate->msg_centre_id_present = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag & ~ASN1_PC_MASK) {
		case ASN1_CLASS_CONTEXT_SPECIFIC | 0:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 2:
			ASN1_CALL(pos, rose_dec_qsig_MsgCentreId(ctrl, "msgCentreId", tag, pos,
				seq_end, &mwi_deactivate->msg_centre_id));
			mwi_deactivate->msg_centre_id_present = 1;
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 3:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 4:
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  argumentExt %s\n", asn1_tag2str(tag));
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
 * \brief Decode the Q.SIG MWIInterrogate invoke argument parameters.
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
const unsigned char *rose_dec_qsig_MWIInterrogate_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	const unsigned char *save_pos;
	struct roseQsigMWIInterrogateArg *mwi_interrogate;

	mwi_interrogate = &args->qsig.MWIInterrogate;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  MWIInterrogateArg %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "servedUserNr", tag, pos, seq_end,
		&mwi_interrogate->served_user_number));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "basicService", tag, pos, seq_end, &value));
	mwi_interrogate->basic_service = value;

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	mwi_interrogate->msg_centre_id_present = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag & ~ASN1_PC_MASK) {
		case ASN1_CLASS_CONTEXT_SPECIFIC | 0:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 2:
			ASN1_CALL(pos, rose_dec_qsig_MsgCentreId(ctrl, "msgCentreId", tag, pos,
				seq_end, &mwi_interrogate->msg_centre_id));
			mwi_interrogate->msg_centre_id_present = 1;
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 3:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 4:
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  argumentExt %s\n", asn1_tag2str(tag));
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
 * \internal
 * \brief Decode the MWIInterrogateResElt argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param record Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_qsig_MWIInterrogateResElt(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigMWIInterrogateResElt *record)
{
	int32_t value;
	size_t str_len;
	int length;
	int seq_offset;
	int explicit_offset;
	const unsigned char *explicit_end;
	const unsigned char *seq_end;
	const unsigned char *save_pos;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  MWIInterrogateResElt %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "basicService", tag, pos, seq_end, &value));
	record->basic_service = value;

	/*
	 * A sequence specifies an ordered list of component types.
	 * However, for simplicity we are not checking the order of
	 * the remaining optional components.
	 */
	record->msg_centre_id_present = 0;
	record->number_of_messages_present = 0;
	record->originating_number.length = 0;
	record->timestamp_present = 0;
	record->priority_present = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		save_pos = pos;
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		switch (tag & ~ASN1_PC_MASK) {
		case ASN1_CLASS_CONTEXT_SPECIFIC | 0:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 2:
			ASN1_CALL(pos, rose_dec_qsig_MsgCentreId(ctrl, "msgCentreId", tag, pos,
				seq_end, &record->msg_centre_id));
			record->msg_centre_id_present = 1;
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 3:
			/* Must not be constructed but we will not check for it for simplicity. */
			ASN1_CALL(pos, asn1_dec_int(ctrl, "nbOfMessages", tag, pos, seq_end,
				&value));
			record->number_of_messages = value;
			record->number_of_messages_present = 1;
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 4:
			/* Must be constructed but we will not check for it for simplicity. */
			/* Remove EXPLICIT tag */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
			}
			ASN1_CALL(pos, asn1_dec_length(pos, seq_end, &length));
			ASN1_END_SETUP(explicit_end, explicit_offset, length, pos, seq_end);

			ASN1_CALL(pos, asn1_dec_tag(pos, explicit_end, &tag));
			ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "originatingNr", tag, pos,
				explicit_end, &record->originating_number));

			ASN1_END_FIXUP(ctrl, pos, explicit_offset, explicit_end, seq_end);
			break;
		case ASN1_TYPE_GENERALIZED_TIME:
			ASN1_CALL(pos, asn1_dec_string_max(ctrl, "timestamp", tag, pos, end,
				sizeof(record->timestamp.str), record->timestamp.str, &str_len));
			record->timestamp_present = 1;
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 5:
			/* Must not be constructed but we will not check for it for simplicity. */
			ASN1_CALL(pos, asn1_dec_int(ctrl, "priority", tag, pos, seq_end, &value));
			record->priority = value;
			record->priority_present = 1;
			break;
		case ASN1_CLASS_CONTEXT_SPECIFIC | 6:
		case ASN1_CLASS_CONTEXT_SPECIFIC | 7:
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  argumentExt %s\n", asn1_tag2str(tag));
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
 * \brief Decode the Q.SIG MWIInterrogate result argument parameters.
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
const unsigned char *rose_dec_qsig_MWIInterrogate_RES(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_result_args *args)
{
	int length;
	int seq_offset;
	const unsigned char *seq_end;
	struct roseQsigMWIInterrogateRes *mwi_interrogate;

	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  MWIInterrogateRes %s\n", asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	mwi_interrogate = &args->qsig.MWIInterrogate;

	mwi_interrogate->num_records = 0;
	while (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		if (mwi_interrogate->num_records < ARRAY_LEN(mwi_interrogate->list)) {
			ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
			ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TAG_SEQUENCE);
			ASN1_CALL(pos, rose_dec_qsig_MWIInterrogateResElt(ctrl, "listEntry", tag,
				pos, seq_end, &mwi_interrogate->list[mwi_interrogate->num_records]));
			++mwi_interrogate->num_records;
		} else {
			/* Too many records */
			return NULL;
		}
	}

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}


/* ------------------------------------------------------------------- */
/* end rose_qsig_mwi.c */
