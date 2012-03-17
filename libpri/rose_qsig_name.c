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
 * \brief Q.SIG ROSE Name operations and elements
 *
 * Name-Operations ECMA-164 Annex C
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
 * \brief Encode the Q.SIG NameSet type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller
 *   implicitly tags it otherwise.
 * \param name
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_qsig_NameSet(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, unsigned tag, const struct roseQsigName *name)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, asn1_enc_string_bin(pos, end, ASN1_TYPE_OCTET_STRING, name->data,
		name->length));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_INTEGER, name->char_set));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the Q.SIG Name type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param name
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_Name(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct roseQsigName *name)
{
	switch (name->presentation) {
	case 0:	/* optional_name_not_present */
		/* Do not encode anything */
		break;
	case 1:	/* presentation_allowed */
		if (name->char_set == 1) {
			ASN1_CALL(pos, asn1_enc_string_bin(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 0,
				name->data, name->length));
		} else {
			ASN1_CALL(pos, rose_enc_qsig_NameSet(ctrl, pos, end,
				ASN1_CLASS_CONTEXT_SPECIFIC | 1, name));
		}
		break;
	case 2:	/* presentation_restricted */
		if (name->char_set == 1) {
			ASN1_CALL(pos, asn1_enc_string_bin(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2,
				name->data, name->length));
		} else {
			ASN1_CALL(pos, rose_enc_qsig_NameSet(ctrl, pos, end,
				ASN1_CLASS_CONTEXT_SPECIFIC | 3, name));
		}
		break;
	case 3:	/* presentation_restricted_null */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 7));
		break;
	case 4:	/* name_not_available */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 4));
		break;
	default:
		ASN1_ENC_ERROR(ctrl, "Unknown name presentation");
		return NULL;
	}

	return pos;
}

/*!
 * \internal
 * \brief Encode the Q.SIG party-Name invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param party Information to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_qsig_PartyName_ARG_Backend(struct pri *ctrl,
	unsigned char *pos, unsigned char *end, const struct roseQsigPartyName_ARG *party)
{
	return rose_enc_qsig_Name(ctrl, pos, end, &party->name);
}

/*!
 * \brief Encode the Q.SIG CallingName invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_CallingName_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return rose_enc_qsig_PartyName_ARG_Backend(ctrl, pos, end, &args->qsig.CallingName);
}

/*!
 * \brief Encode the Q.SIG CalledName invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_CalledName_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return rose_enc_qsig_PartyName_ARG_Backend(ctrl, pos, end, &args->qsig.CalledName);
}

/*!
 * \brief Encode the Q.SIG ConnectedName invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_ConnectedName_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return rose_enc_qsig_PartyName_ARG_Backend(ctrl, pos, end,
		&args->qsig.ConnectedName);
}

/*!
 * \brief Encode the Q.SIG BusyName invoke facility ie arguments.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param args Arguments to encode in the buffer.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_qsig_BusyName_ARG(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const union rose_msg_invoke_args *args)
{
	return rose_enc_qsig_PartyName_ARG_Backend(ctrl, pos, end, &args->qsig.BusyName);
}

/*!
 * \internal
 * \brief Decode the Q.SIG NameData Name argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param fname Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param name Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_qsig_NameData(struct pri *ctrl, const char *fname,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigName *name)
{
	size_t str_len;

	ASN1_CALL(pos, asn1_dec_string_bin(ctrl, fname, tag, pos, end, sizeof(name->data),
		name->data, &str_len));
	name->length = str_len;

	return pos;
}

/*!
 * \internal
 * \brief Decode the Q.SIG NameSet Name argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param fname Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param name Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_qsig_NameSet(struct pri *ctrl, const char *fname,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigName *name)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s NameSet %s\n", fname, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag & ~ASN1_PC_MASK, ASN1_TYPE_OCTET_STRING);
	ASN1_CALL(pos, rose_dec_qsig_NameData(ctrl, "nameData", tag, pos, seq_end, name));

	if (pos < end && *pos != ASN1_INDEF_TERM) {
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_INTEGER);
		ASN1_CALL(pos, asn1_dec_int(ctrl, "characterSet", tag, pos, seq_end, &value));
		name->char_set = value;
	} else {
		name->char_set = 1;	/* default to iso8859-1 */
	}

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the Q.SIG Name argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param fname Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param name Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *rose_dec_qsig_Name(struct pri *ctrl, const char *fname,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigName *name)
{
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s Name\n", fname);
	}
	name->char_set = 1;	/* default to iso8859-1 */
	switch (tag & ~ASN1_PC_MASK) {
	case ASN1_CLASS_CONTEXT_SPECIFIC | 0:
		name->presentation = 1;	/* presentation_allowed */
		ASN1_CALL(pos, rose_dec_qsig_NameData(ctrl, "namePresentationAllowedSimple", tag,
			pos, end, name));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		/* Must be constructed but we will not check for it for simplicity. */
		name->presentation = 1;	/* presentation_allowed */
		ASN1_CALL(pos, rose_dec_qsig_NameSet(ctrl, "namePresentationAllowedExtended",
			tag, pos, end, name));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 2:
		name->presentation = 2;	/* presentation_restricted */
		ASN1_CALL(pos, rose_dec_qsig_NameData(ctrl, "namePresentationRestrictedSimple",
			tag, pos, end, name));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 3:
		/* Must be constructed but we will not check for it for simplicity. */
		name->presentation = 2;	/* presentation_restricted */
		ASN1_CALL(pos, rose_dec_qsig_NameSet(ctrl, "namePresentationRestrictedExtended",
			tag, pos, end, name));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 4:
		/* Must not be constructed but we will not check for it for simplicity. */
		name->presentation = 4;	/* name_not_available */
		name->length = 0;
		name->data[0] = 0;
		ASN1_CALL(pos, asn1_dec_null(ctrl, "nameNotAvailable", tag, pos, end));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 7:
		/* Must not be constructed but we will not check for it for simplicity. */
		name->presentation = 3;	/* presentation_restricted_null */
		name->length = 0;
		name->data[0] = 0;
		ASN1_CALL(pos, asn1_dec_null(ctrl, "namePresentationRestrictedNull", tag, pos,
			end));
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	return pos;
}

/*!
 * \internal
 * \brief Decode the Q.SIG party-Name invoke argument parameters.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param name Field name
 * \param tag Component tag that identified this structure.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param party Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_qsig_PartyName_ARG_Backend(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseQsigPartyName_ARG *party)
{
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	if (tag == ASN1_TAG_SEQUENCE) {
		if (ctrl->debug & PRI_DEBUG_APDU) {
			pri_message(ctrl, "  %s %s\n", name, asn1_tag2str(tag));
		}
		ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
		ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CALL(pos, rose_dec_qsig_Name(ctrl, "name", tag, pos, seq_end,
			&party->name));

		/* Fixup will skip over any OPTIONAL manufacturer extension information */
		ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);
	} else {
		ASN1_CALL(pos, rose_dec_qsig_Name(ctrl, name, tag, pos, end, &party->name));
	}

	return pos;
}

/*!
 * \brief Decode the Q.SIG CallingName invoke argument parameters.
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
const unsigned char *rose_dec_qsig_CallingName_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	return rose_dec_qsig_PartyName_ARG_Backend(ctrl, "callingName", tag, pos, end,
		&args->qsig.CallingName);
}

/*!
 * \brief Decode the Q.SIG CalledName invoke argument parameters.
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
const unsigned char *rose_dec_qsig_CalledName_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	return rose_dec_qsig_PartyName_ARG_Backend(ctrl, "calledName", tag, pos, end,
		&args->qsig.CalledName);
}

/*!
 * \brief Decode the Q.SIG ConnectedName invoke argument parameters.
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
const unsigned char *rose_dec_qsig_ConnectedName_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	return rose_dec_qsig_PartyName_ARG_Backend(ctrl, "connectedName", tag, pos, end,
		&args->qsig.ConnectedName);
}

/*!
 * \brief Decode the Q.SIG BusyName invoke argument parameters.
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
const unsigned char *rose_dec_qsig_BusyName_ARG(struct pri *ctrl, unsigned tag,
	const unsigned char *pos, const unsigned char *end, union rose_msg_invoke_args *args)
{
	return rose_dec_qsig_PartyName_ARG_Backend(ctrl, "busyName", tag, pos, end,
		&args->qsig.BusyName);
}

/* ------------------------------------------------------------------- */
/* end rose_qsig_name.c */
