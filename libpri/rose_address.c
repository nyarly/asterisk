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
 * \brief ROSE Addressing-Data-Elements
 *
 * Addressing-Data-Elements ETS 300 196-1 D.3
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
 * \brief Encode the public or private network PartyNumber type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param number
 * \param length_of_number
 * \param type_of_number
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
static unsigned char *rose_enc_NetworkPartyNumber(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, unsigned tag, const unsigned char *number,
	size_t length_of_number, u_int8_t type_of_number)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED, type_of_number));
	ASN1_CALL(pos, asn1_enc_string_bin(pos, end, ASN1_TYPE_NUMERIC_STRING, number,
		length_of_number));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the PartyNumber type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param party_number
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_PartyNumber(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct rosePartyNumber *party_number)
{
	switch (party_number->plan) {
	case 0:	/* Unknown PartyNumber */
		ASN1_CALL(pos, asn1_enc_string_bin(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 0,
			party_number->str, party_number->length));
		break;
	case 1:	/* Public PartyNumber */
		ASN1_CALL(pos, rose_enc_NetworkPartyNumber(ctrl, pos, end,
			ASN1_CLASS_CONTEXT_SPECIFIC | 1, party_number->str, party_number->length,
			party_number->ton));
		break;
	case 2:	/* NSAP encoded PartyNumber */
		ASN1_CALL(pos, asn1_enc_string_bin(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2,
			party_number->str, party_number->length));
		break;
	case 3:	/* Data PartyNumber (Not used) */
		ASN1_CALL(pos, asn1_enc_string_bin(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 3,
			party_number->str, party_number->length));
		break;
	case 4:	/* Telex PartyNumber (Not used) */
		ASN1_CALL(pos, asn1_enc_string_bin(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 4,
			party_number->str, party_number->length));
		break;
	case 5:	/* Private PartyNumber */
		ASN1_CALL(pos, rose_enc_NetworkPartyNumber(ctrl, pos, end,
			ASN1_CLASS_CONTEXT_SPECIFIC | 5, party_number->str, party_number->length,
			party_number->ton));
		break;
	case 8:	/* National Standard PartyNumber (Not used) */
		ASN1_CALL(pos, asn1_enc_string_bin(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 8,
			party_number->str, party_number->length));
		break;
	default:
		ASN1_ENC_ERROR(ctrl, "Unknown numbering plan");
		return NULL;
	}

	return pos;
}

/*!
 * \brief Encode the PartySubaddress type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param party_subaddress
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_PartySubaddress(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct rosePartySubaddress *party_subaddress)
{
	unsigned char *seq_len;

	switch (party_subaddress->type) {
	case 0:	/* UserSpecified */
		ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_TAG_SEQUENCE);

		ASN1_CALL(pos, asn1_enc_string_bin(pos, end, ASN1_TYPE_OCTET_STRING,
			party_subaddress->u.user_specified.information, party_subaddress->length));
		if (party_subaddress->u.user_specified.odd_count_present) {
			ASN1_CALL(pos, asn1_enc_boolean(pos, end, ASN1_TYPE_BOOLEAN,
				party_subaddress->u.user_specified.odd_count));
		}

		ASN1_CONSTRUCTED_END(seq_len, pos, end);
		break;
	case 1:	/* NSAP */
		ASN1_CALL(pos, asn1_enc_string_bin(pos, end, ASN1_TYPE_OCTET_STRING,
			party_subaddress->u.nsap, party_subaddress->length));
		break;
	default:
		ASN1_ENC_ERROR(ctrl, "Unknown subaddress type");
		return NULL;
	}

	return pos;
}

/*!
 * \brief Encode the Address type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param address
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_Address(struct pri *ctrl, unsigned char *pos, unsigned char *end,
	unsigned tag, const struct roseAddress *address)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end, &address->number));
	if (address->subaddress.length) {
		ASN1_CALL(pos, rose_enc_PartySubaddress(ctrl, pos, end, &address->subaddress));
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the PresentedNumberUnscreened type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param party
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_PresentedNumberUnscreened(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct rosePresentedNumberUnscreened *party)
{
	unsigned char *seq_len;

	switch (party->presentation) {
	case 0:	/* presentationAllowedNumber */
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 0);
		ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end, &party->number));
		ASN1_CONSTRUCTED_END(seq_len, pos, end);
		break;
	case 1:	/* presentationRestricted */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1));
		break;
	case 2:	/* numberNotAvailableDueToInterworking */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2));
		break;
	case 3:	/* presentationRestrictedNumber */
		/* EXPLICIT tag */
		ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 3);
		ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end, &party->number));
		ASN1_CONSTRUCTED_END(seq_len, pos, end);
		break;
	default:
		ASN1_ENC_ERROR(ctrl, "Unknown presentation type");
		return NULL;
	}

	return pos;
}

/*!
 * \brief Encode the NumberScreened type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param screened
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_NumberScreened(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, unsigned tag, const struct roseNumberScreened *screened)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end, &screened->number));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		screened->screening_indicator));

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the PresentedNumberScreened type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param party
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_PresentedNumberScreened(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct rosePresentedNumberScreened *party)
{
	switch (party->presentation) {
	case 0:	/* presentationAllowedNumber */
		ASN1_CALL(pos, rose_enc_NumberScreened(ctrl, pos, end,
			ASN1_CLASS_CONTEXT_SPECIFIC | 0, &party->screened));
		break;
	case 1:	/* presentationRestricted */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1));
		break;
	case 2:	/* numberNotAvailableDueToInterworking */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2));
		break;
	case 3:	/* presentationRestrictedNumber */
		ASN1_CALL(pos, rose_enc_NumberScreened(ctrl, pos, end,
			ASN1_CLASS_CONTEXT_SPECIFIC | 3, &party->screened));
		break;
	default:
		ASN1_ENC_ERROR(ctrl, "Unknown presentation type");
		return NULL;
	}

	return pos;
}

/*!
 * \brief Encode the AddressScreened type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *   The tag should be ASN1_TAG_SEQUENCE unless the caller implicitly
 *   tags it otherwise.
 * \param screened
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_AddressScreened(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, unsigned tag, const struct roseAddressScreened *screened)
{
	unsigned char *seq_len;

	ASN1_CONSTRUCTED_BEGIN(seq_len, pos, end, tag);

	ASN1_CALL(pos, rose_enc_PartyNumber(ctrl, pos, end, &screened->number));
	ASN1_CALL(pos, asn1_enc_int(pos, end, ASN1_TYPE_ENUMERATED,
		screened->screening_indicator));
	if (screened->subaddress.length) {
		ASN1_CALL(pos, rose_enc_PartySubaddress(ctrl, pos, end, &screened->subaddress));
	}

	ASN1_CONSTRUCTED_END(seq_len, pos, end);

	return pos;
}

/*!
 * \brief Encode the PresentedAddressScreened type.
 *
 * \param ctrl D channel controller for diagnostic messages or global options.
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param party
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *rose_enc_PresentedAddressScreened(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct rosePresentedAddressScreened *party)
{
	switch (party->presentation) {
	case 0:	/* presentationAllowedAddress */
		ASN1_CALL(pos, rose_enc_AddressScreened(ctrl, pos, end,
			ASN1_CLASS_CONTEXT_SPECIFIC | 0, &party->screened));
		break;
	case 1:	/* presentationRestricted */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 1));
		break;
	case 2:	/* numberNotAvailableDueToInterworking */
		ASN1_CALL(pos, asn1_enc_null(pos, end, ASN1_CLASS_CONTEXT_SPECIFIC | 2));
		break;
	case 3:	/* presentationRestrictedAddress */
		ASN1_CALL(pos, rose_enc_AddressScreened(ctrl, pos, end,
			ASN1_CLASS_CONTEXT_SPECIFIC | 3, &party->screened));
		break;
	default:
		ASN1_ENC_ERROR(ctrl, "Unknown presentation type");
		return NULL;
	}

	return pos;
}

/*!
 * \internal
 * \brief Decode the NumberDigits PartyNumber argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param party_number Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_NumberDigits(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct rosePartyNumber *party_number)
{
	size_t str_len;

	ASN1_CALL(pos, asn1_dec_string_max(ctrl, name, tag, pos, end,
		sizeof(party_number->str), party_number->str, &str_len));
	party_number->length = str_len;

	return pos;
}

/*!
 * \internal
 * \brief Decode the NSAP PartyNumber argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param party_number Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_NSAPPartyNumber(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct rosePartyNumber *party_number)
{
	size_t str_len;

	ASN1_CALL(pos, asn1_dec_string_bin(ctrl, name, tag, pos, end,
		sizeof(party_number->str), party_number->str, &str_len));
	party_number->length = str_len;

	return pos;
}

/*!
 * \internal
 * \brief Decode the public or private network PartyNumber argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param party_number Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_NetworkPartyNumber(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct rosePartyNumber *party_number)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "typeOfNumber", tag, pos, seq_end, &value));
	party_number->ton = value;

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag & ~ASN1_PC_MASK, ASN1_TYPE_NUMERIC_STRING);
	ASN1_CALL(pos, rose_dec_NumberDigits(ctrl, "numberDigits", tag, pos, seq_end,
		party_number));

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the PartyNumber argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param party_number Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *rose_dec_PartyNumber(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct rosePartyNumber *party_number)
{
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s PartyNumber\n", name);
	}
	party_number->ton = 0;	/* unknown */
	switch (tag & ~ASN1_PC_MASK) {
	case ASN1_CLASS_CONTEXT_SPECIFIC | 0:
		party_number->plan = 0;	/* Unknown PartyNumber */
		ASN1_CALL(pos, rose_dec_NumberDigits(ctrl, "unknownPartyNumber", tag, pos, end,
			party_number));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		/* Must be constructed but we will not check for it for simplicity. */
		party_number->plan = 1;	/* Public PartyNumber */
		ASN1_CALL(pos, rose_dec_NetworkPartyNumber(ctrl, "publicPartyNumber", tag, pos,
			end, party_number));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 2:
		party_number->plan = 2;	/* NSAP encoded PartyNumber */
		ASN1_CALL(pos, rose_dec_NSAPPartyNumber(ctrl, "nsapEncodedPartyNumber", tag, pos,
			end, party_number));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 3:
		party_number->plan = 3;	/* Data PartyNumber (Not used) */
		ASN1_CALL(pos, rose_dec_NumberDigits(ctrl, "dataPartyNumber", tag, pos, end,
			party_number));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 4:
		party_number->plan = 4;	/* Telex PartyNumber (Not used) */
		ASN1_CALL(pos, rose_dec_NumberDigits(ctrl, "telexPartyNumber", tag, pos, end,
			party_number));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 5:
		/* Must be constructed but we will not check for it for simplicity. */
		party_number->plan = 5;	/* Private PartyNumber */
		ASN1_CALL(pos, rose_dec_NetworkPartyNumber(ctrl, "privatePartyNumber", tag, pos,
			end, party_number));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 8:
		party_number->plan = 8;	/* National Standard PartyNumber (Not used) */
		ASN1_CALL(pos, rose_dec_NumberDigits(ctrl, "nationalStandardPartyNumber", tag,
			pos, end, party_number));
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	return pos;
}

/*!
 * \internal
 * \brief Decode the User PartySubaddress argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param party_subaddress Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_UserSubaddress(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct rosePartySubaddress *party_subaddress)
{
	size_t str_len;
	int32_t odd_count;
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	party_subaddress->type = 0;	/* UserSpecified */

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s UserSpecified %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	/* SubaddressInformation */
	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag & ~ASN1_PC_MASK, ASN1_TYPE_OCTET_STRING);
	ASN1_CALL(pos, asn1_dec_string_bin(ctrl, "subaddressInformation", tag, pos, seq_end,
		sizeof(party_subaddress->u.user_specified.information),
		party_subaddress->u.user_specified.information, &str_len));
	party_subaddress->length = str_len;

	if (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		/*
		 * The optional odd count indicator must be present since there
		 * is something left.
		 */
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_BOOLEAN);
		ASN1_CALL(pos, asn1_dec_boolean(ctrl, "oddCount", tag, pos, seq_end,
			&odd_count));
		party_subaddress->u.user_specified.odd_count = odd_count;
		party_subaddress->u.user_specified.odd_count_present = 1;
	} else {
		party_subaddress->u.user_specified.odd_count = 0;
		party_subaddress->u.user_specified.odd_count_present = 0;
	}

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \internal
 * \brief Decode the NSAP PartySubaddress argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param party_subaddress Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *rose_dec_NSAPSubaddress(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct rosePartySubaddress *party_subaddress)
{
	size_t str_len;

	party_subaddress->type = 1;	/* NSAP */

	ASN1_CALL(pos, asn1_dec_string_bin(ctrl, name, tag, pos, end,
		sizeof(party_subaddress->u.nsap), party_subaddress->u.nsap, &str_len));
	party_subaddress->length = str_len;

	return pos;
}

/*!
 * \brief Decode the PartySubaddress argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param party_subaddress Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *rose_dec_PartySubaddress(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct rosePartySubaddress *party_subaddress)
{
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s PartySubaddress\n", name);
	}
	switch (tag) {
	case ASN1_TAG_SEQUENCE:
		ASN1_CALL(pos, rose_dec_UserSubaddress(ctrl, "user", tag, pos, end,
			party_subaddress));
		break;
	case ASN1_TYPE_OCTET_STRING:
	case ASN1_TYPE_OCTET_STRING | ASN1_PC_CONSTRUCTED:
		ASN1_CALL(pos, rose_dec_NSAPSubaddress(ctrl, "nsap", tag, pos, end,
			party_subaddress));
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	return pos;
}

/*!
 * \brief Decode the Address argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param address Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *rose_dec_Address(struct pri *ctrl, const char *name, unsigned tag,
	const unsigned char *pos, const unsigned char *end, struct roseAddress *address)
{
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s Address %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "partyNumber", tag, pos, seq_end,
		&address->number));

	if (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		/* The optional subaddress must be present since there is something left. */
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CALL(pos, rose_dec_PartySubaddress(ctrl, "partySubaddress", tag, pos,
			seq_end, &address->subaddress));
	} else {
		address->subaddress.length = 0;	/* Subaddress not present */
	}

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the PresentedNumberUnscreened argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param party Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *rose_dec_PresentedNumberUnscreened(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct rosePresentedNumberUnscreened *party)
{
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s PresentedNumberUnscreened\n", name);
	}
	switch (tag) {
	case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 0:
		if (ctrl->debug & PRI_DEBUG_APDU) {
			pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
		}
		ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
		ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		party->presentation = 0;	/* presentationAllowedNumber */
		ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "presentationAllowedNumber", tag, pos,
			seq_end, &party->number));

		ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		party->presentation = 1;	/* presentationRestricted */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "presentationRestricted", tag, pos, end));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 2:
		party->presentation = 2;	/* numberNotAvailableDueToInterworking */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "numberNotAvailableDueToInterworking", tag,
			pos, end));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 3:
		if (ctrl->debug & PRI_DEBUG_APDU) {
			pri_message(ctrl, "  Explicit %s\n", asn1_tag2str(tag));
		}
		ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
		ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		party->presentation = 3;	/* presentationRestrictedNumber */
		ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "presentationRestrictedNumber", tag,
			pos, seq_end, &party->number));

		ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	return pos;
}

/*!
 * \brief Decode the NumberScreened argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param screened Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *rose_dec_NumberScreened(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseNumberScreened *screened)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s NumberScreened %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "partyNumber", tag, pos, seq_end,
		&screened->number));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "screeningIndicator", tag, pos, seq_end, &value));
	screened->screening_indicator = value;

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the PresentedNumberScreened argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param party Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *rose_dec_PresentedNumberScreened(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct rosePresentedNumberScreened *party)
{
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s PresentedNumberScreened\n", name);
	}
	switch (tag) {
	case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 0:
		party->presentation = 0;	/* presentationAllowedNumber */
		ASN1_CALL(pos, rose_dec_NumberScreened(ctrl, "presentationAllowedNumber", tag,
			pos, end, &party->screened));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		party->presentation = 1;	/* presentationRestricted */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "presentationRestricted", tag, pos, end));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 2:
		party->presentation = 2;	/* numberNotAvailableDueToInterworking */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "numberNotAvailableDueToInterworking", tag,
			pos, end));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 3:
		party->presentation = 3;	/* presentationRestrictedNumber */
		ASN1_CALL(pos, rose_dec_NumberScreened(ctrl, "presentationRestrictedNumber", tag,
			pos, end, &party->screened));
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	return pos;
}

/*!
 * \brief Decode the AddressScreened argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param screened Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *rose_dec_AddressScreened(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct roseAddressScreened *screened)
{
	int32_t value;
	int length;
	int seq_offset;
	const unsigned char *seq_end;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s AddressScreened %s\n", name, asn1_tag2str(tag));
	}
	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	ASN1_END_SETUP(seq_end, seq_offset, length, pos, end);

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CALL(pos, rose_dec_PartyNumber(ctrl, "partyNumber", tag, pos, seq_end,
		&screened->number));

	ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
	ASN1_CHECK_TAG(ctrl, tag, tag, ASN1_TYPE_ENUMERATED);
	ASN1_CALL(pos, asn1_dec_int(ctrl, "screeningIndicator", tag, pos, seq_end, &value));
	screened->screening_indicator = value;

	if (pos < seq_end && *pos != ASN1_INDEF_TERM) {
		/* The optional subaddress must be present since there is something left. */
		ASN1_CALL(pos, asn1_dec_tag(pos, seq_end, &tag));
		ASN1_CALL(pos, rose_dec_PartySubaddress(ctrl, "partySubaddress", tag, pos,
			seq_end, &screened->subaddress));
	} else {
		screened->subaddress.length = 0;	/* Subaddress not present */
	}

	ASN1_END_FIXUP(ctrl, pos, seq_offset, seq_end, end);

	return pos;
}

/*!
 * \brief Decode the PresentedAddressScreened argument parameters.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this production.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param party Parameter storage to fill.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *rose_dec_PresentedAddressScreened(struct pri *ctrl,
	const char *name, unsigned tag, const unsigned char *pos, const unsigned char *end,
	struct rosePresentedAddressScreened *party)
{
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s PresentedAddressScreened\n", name);
	}
	switch (tag) {
	case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 0:
		party->presentation = 0;	/* presentationAllowedAddress */
		ASN1_CALL(pos, rose_dec_AddressScreened(ctrl, "presentationAllowedAddress", tag,
			pos, end, &party->screened));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 1:
		party->presentation = 1;	/* presentationRestricted */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "presentationRestricted", tag, pos, end));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | 2:
		party->presentation = 2;	/* numberNotAvailableDueToInterworking */
		ASN1_CALL(pos, asn1_dec_null(ctrl, "numberNotAvailableDueToInterworking", tag,
			pos, end));
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC | ASN1_PC_CONSTRUCTED | 3:
		party->presentation = 3;	/* presentationRestrictedAddress */
		ASN1_CALL(pos, rose_dec_AddressScreened(ctrl, "presentationRestrictedAddress",
			tag, pos, end, &party->screened));
		break;
	default:
		ASN1_DID_NOT_EXPECT_TAG(ctrl, tag);
		return NULL;
	}

	return pos;
}

/* ------------------------------------------------------------------- */
/* end rose_address.c */
