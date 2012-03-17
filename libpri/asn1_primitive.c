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
 * \brief ASN.1 BER encode/decode primitives
 *
 * \author Richard Mudgett <rmudgett@digium.com>
 */


#include <stdio.h>
#include <ctype.h>

#include "compat.h"
#include "libpri.h"
#include "pri_internal.h"
#include "asn1.h"

/* ------------------------------------------------------------------- */

/*!
 * \internal
 * \brief Dump the memory contents indicated in printable characters. (Helper function.)
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param start Dump memory starting position.
 * \param end Dump memory ending position. (Not included in dump.)
 *
 * \return Nothing
 */
static void asn1_dump_mem_helper(struct pri *ctrl, const unsigned char *start,
	const unsigned char *end)
{
	pri_message(ctrl, " - \"");
	for (; start < end; ++start) {
		pri_message(ctrl, "%c", (isprint(*start)) ? *start : '~');
	}
	pri_message(ctrl, "\"\n");
}

/*!
 * \internal
 * \brief Dump the memory contents indicated.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param indent Number of spaces to indent for each new memory dump line.
 * \param pos Dump memory starting position.
 * \param length Number of bytes to dump.
 *
 * \return Nothing
 */
static void asn1_dump_mem(struct pri *ctrl, unsigned indent, const unsigned char *pos,
	unsigned length)
{
	const unsigned char *seg_start;
	const unsigned char *end;
	unsigned delimiter;
	unsigned count;

	seg_start = pos;
	end = pos + length;
	if (pos < end) {
		delimiter = '<';
		for (;;) {
			pri_message(ctrl, "%*s", indent, "");
			for (count = 0; count++ < 16 && pos < end;) {
				pri_message(ctrl, "%c%02X", delimiter, *pos++);
				delimiter = (count == 8) ? '-' : ' ';
			}
			if (end <= pos) {
				break;
			}
			asn1_dump_mem_helper(ctrl, seg_start, pos);
			seg_start = pos;
		}
	} else {
		pri_message(ctrl, "%*s<", indent, "");
	}
	pri_message(ctrl, ">");
	asn1_dump_mem_helper(ctrl, seg_start, end);
}

/*!
 * \brief Convert the given tag value to a descriptive string.
 *
 * \param tag Component tag value to convert to a string.
 *
 * \return Converted tag string.
 */
const char *asn1_tag2str(unsigned tag)
{
	static const char *primitives[32] = {
		[ASN1_TYPE_INDEF_TERM] = "Indefinite length terminator",
		[ASN1_TYPE_BOOLEAN] = "Boolean",
		[ASN1_TYPE_INTEGER] = "Integer",
		[ASN1_TYPE_BIT_STRING] = "Bit String",
		[ASN1_TYPE_OCTET_STRING] = "Octet String",
		[ASN1_TYPE_NULL] = "NULL",
		[ASN1_TYPE_OBJECT_IDENTIFIER] = "OID",
		[ASN1_TYPE_OBJECT_DESCRIPTOR] = "Object Descriptor",
		[ASN1_TYPE_EXTERN] = "External",
		[ASN1_TYPE_REAL] = "Real",
		[ASN1_TYPE_ENUMERATED] = "Enumerated",
		[ASN1_TYPE_EMBEDDED_PDV] = "Embedded PDV",
		[ASN1_TYPE_UTF8_STRING] = "UTF8 String",
		[ASN1_TYPE_RELATIVE_OID] = "Relative OID",
		[ASN1_TYPE_SEQUENCE] = "Sequence",
		[ASN1_TYPE_SET] = "Set",
		[ASN1_TYPE_NUMERIC_STRING] = "Numeric String",
		[ASN1_TYPE_PRINTABLE_STRING] = "Printable String",
		[ASN1_TYPE_TELETEX_STRING] = "Teletex String",
		[ASN1_TYPE_VIDEOTEX_STRING] = "Videotex String",
		[ASN1_TYPE_IA5_STRING] = "IA5 String",
		[ASN1_TYPE_UTC_TIME] = "UTC Time",
		[ASN1_TYPE_GENERALIZED_TIME] = "Generalized Time",
		[ASN1_TYPE_GRAPHIC_STRING] = "Graphic String",
		[ASN1_TYPE_VISIBLE_STRING] = "Visible/ISO646 String",
		[ASN1_TYPE_GENERAL_STRING] = "General String",
		[ASN1_TYPE_UNIVERSAL_STRING] = "Universal String",
		[ASN1_TYPE_CHAR_STRING] = "Character String",
		[ASN1_TYPE_BMP_STRING] = "BMP String",
		[ASN1_TYPE_EXTENSION] = "Type Extension",
	};
	static char buf[64];
	const char *description;
	unsigned asn1_constructed;	/*! TRUE if the tag is constructed. */
	unsigned asn1_type;

	asn1_constructed = ((tag & ASN1_PC_MASK) == ASN1_PC_CONSTRUCTED);
	asn1_type = tag & ASN1_TYPE_MASK;

	switch (tag & ASN1_CLASS_MASK) {
	case ASN1_CLASS_UNIVERSAL:
		if (tag == (ASN1_CLASS_UNIVERSAL | ASN1_PC_CONSTRUCTED | ASN1_TYPE_INDEF_TERM)) {
			description = NULL;
		} else {
			description = primitives[asn1_type];
		}
		if (!description) {
			description = "Reserved";
		}
		snprintf(buf, sizeof(buf), "%s%s(%u 0x%02X)", description,
			asn1_constructed ? "/C" : "", tag, tag);
		return buf;
	case ASN1_CLASS_APPLICATION:
		description = "Application";
		break;
	case ASN1_CLASS_CONTEXT_SPECIFIC:
		description = "Context Specific";
		break;
	case ASN1_CLASS_PRIVATE:
		description = "Private";
		break;
	default:
		snprintf(buf, sizeof(buf), "Unknown tag (%u 0x%02X)", tag, tag);
		return buf;
	}
	snprintf(buf, sizeof(buf), "%s%s [%u 0x%02X]", description,
		asn1_constructed ? "/C" : "", asn1_type, asn1_type);
	return buf;
}

/*!
 * \brief Decode the ASN.1 tag value.
 *
 * \param tag_pos ASN.1 tag starting position.
 * \param end End of ASN.1 encoded data buffer.
 * \param tag Decoded tag value returned on success.
 *
 * \retval Next octet after the tag on success.
 * \retval NULL on error.
 */
const unsigned char *asn1_dec_tag(const unsigned char *tag_pos, const unsigned char *end,
	unsigned *tag)
{
	unsigned extended_tag;

	if (end <= tag_pos) {
		return NULL;
	}
	*tag = *tag_pos++;
	if ((*tag & ASN1_TYPE_MASK) == ASN1_TYPE_EXTENSION) {
		/* Extract the extended tag value */
		extended_tag = 0;
		do {
			if (end <= tag_pos) {
				return NULL;
			}
			extended_tag <<= 7;
			extended_tag |= *tag_pos & ~0x80;
		} while (*tag_pos++ & 0x80);
		if (extended_tag && extended_tag < ASN1_TYPE_EXTENSION) {
			/*
			 * The sender did not need to use the extended format.
			 * This is an encoding error on their part, but we will
			 * accept it anyway.
			 *
			 * Note we cannot return a null tag value from this path.
			 * We would misinterpret the indefinite length
			 * terminator.
			 */
			*tag &= ~ASN1_TYPE_MASK;
			*tag |= extended_tag;
		}
	}

	return tag_pos;
}

/*!
 * \brief Decode the length of an ASN.1 component length.
 *
 * \param len_pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param length Decoded length value returned on success. (-1 if indefinite)
 *
 * \retval Next octet after the length on success.
 * \retval NULL on error.
 *
 * \note The decoded length is checked to see if there is enough buffer
 * left for the component body.
 */
const unsigned char *asn1_dec_length(const unsigned char *len_pos,
	const unsigned char *end, int *length)
{
	unsigned length_size;

	if (end <= len_pos) {
		/* Not enough buffer to determine how the length is encoded */
		return NULL;
	}

	if (*len_pos < 0x80) {
		/* Short length encoding */
		*length = *len_pos++;
	} else if (*len_pos == 0x80) {
		/* Indefinite length encoding */
		*length = -1;
		++len_pos;
		if (end < len_pos + ASN1_INDEF_TERM_LEN) {
			/* Not enough buffer for the indefinite length terminator */
			return NULL;
		}
		return len_pos;
	} else {
		/* Long length encoding */
		length_size = *len_pos++ & 0x7f;
		if (length_size == 0x7f) {
			/* Reserved extension encoding that has not been defined. */
			return NULL;
		}
		if (end < len_pos + length_size) {
			/* Not enough buffer for the length value */
			return NULL;
		}
		*length = 0;
		while (length_size--) {
			*length = (*length << 8) | *len_pos++;
		}
	}

	if (end < len_pos + *length) {
		/* Not enough buffer for the component body. */
		return NULL;
	}
	return len_pos;
}

/*!
 * \internal
 * \brief Skip to the end of an indefinite length constructed component helper.
 *
 * \param pos ASN.1 tag starting position.
 * \param end End of ASN.1 decoding data buffer.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *asn1_dec_indef_end_fixup_helper(const unsigned char *pos,
	const unsigned char *end)
{
	unsigned tag;
	int length;

	while (pos < end && *pos != ASN1_INDEF_TERM) {
		ASN1_CALL(pos, asn1_dec_tag(pos, end, &tag));
		ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
		if (length < 0) {
			/* Skip over indefinite length sub-component */
			if ((tag & ASN1_PC_MASK) == ASN1_PC_CONSTRUCTED
				|| tag == (ASN1_CLASS_UNIVERSAL | ASN1_PC_PRIMITIVE | ASN1_TYPE_SET)
				|| tag ==
				(ASN1_CLASS_UNIVERSAL | ASN1_PC_PRIMITIVE | ASN1_TYPE_SEQUENCE)) {
				/* This is an ITU encoded indefinite length component. */
				ASN1_CALL(pos, asn1_dec_indef_end_fixup_helper(pos, end));
			} else {
				/* This is a non-ITU encoded indefinite length component. */
				while (pos < end && *pos != ASN1_INDEF_TERM) {
					++pos;
				}
				pos += ASN1_INDEF_TERM_LEN;
			}
		} else {
			/* Skip over defininte length sub-component */
			pos += length;
		}
	}
	if (end < pos + ASN1_INDEF_TERM_LEN) {
		return NULL;
	}

	return pos + ASN1_INDEF_TERM_LEN;
}

/*!
 * \brief Skip to the end of an indefinite length constructed component.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param pos ASN.1 tag starting position.
 * \param end End of ASN.1 decoding data buffer.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *asn1_dec_indef_end_fixup(struct pri *ctrl, const unsigned char *pos,
	const unsigned char *end)
{
	if (pos < end && *pos != ASN1_INDEF_TERM && (ctrl->debug & PRI_DEBUG_APDU)) {
		pri_message(ctrl,
			"  Skipping unused indefinite length constructed component octets!\n");
	}
	return asn1_dec_indef_end_fixup_helper(pos, end);
}

/*!
 * \brief Decode the boolean primitive.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this primitive.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param value Decoded boolean value.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *asn1_dec_boolean(struct pri *ctrl, const char *name, unsigned tag,
	const unsigned char *pos, const unsigned char *end, int32_t *value)
{
	int length;

	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	if (length != 1) {
		/*
		 * The encoding rules say the length can only be one.
		 * It is rediculus to get anything else anyway.
		 */
		return NULL;
	}

	*value = *pos++ ? 1 : 0;

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s %s = %d\n", name, asn1_tag2str(tag), *value);
	}

	return pos;
}

/*!
 * \brief Decode the integer type primitive.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this primitive.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param value Decoded integer type value.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *asn1_dec_int(struct pri *ctrl, const char *name, unsigned tag,
	const unsigned char *pos, const unsigned char *end, int32_t *value)
{
	int length;

	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	if (length <= 0) {
		/*
		 * The encoding rules say the length can not be indefinite.
		 * It cannot be empty for that matter either.
		 */
		return NULL;
	}

#if 1
	/* Read value as signed */
	if (*pos & 0x80) {
		/* The value is negative */
		*value = -1;
	} else {
		*value = 0;
	}
#else
	/* Read value as unsigned */
	*value = 0;
#endif
	while (length--) {
		*value = (*value << 8) | *pos;
		pos++;
	}

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s %s = %d 0x%04X\n", name, asn1_tag2str(tag), *value,
			*value);
	}

	return pos;
}

/*!
 * \brief Decode the null primitive.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this primitive.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *asn1_dec_null(struct pri *ctrl, const char *name, unsigned tag,
	const unsigned char *pos, const unsigned char *end)
{
	int length;

	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	if (length != 0) {
		/*
		 * The encoding rules say the length can only be zero.
		 * It is rediculus to get anything else anyway.
		 */
		return NULL;
	}

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s %s\n", name, asn1_tag2str(tag));
	}

	return pos;
}

/*!
 * \brief Decode the object identifier primitive.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this primitive.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param oid Decoded OID type value.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *asn1_dec_oid(struct pri *ctrl, const char *name, unsigned tag,
	const unsigned char *pos, const unsigned char *end, struct asn1_oid *oid)
{
	int length;
	unsigned num_values;
	unsigned value;
	unsigned delimiter;

	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	if (length < 0) {
		/*
		 * The encoding rules say the length can not be indefinite.
		 */
		return NULL;
	}

	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "  %s %s =", name, asn1_tag2str(tag));
	}
	delimiter = ' ';
	num_values = 0;
	while (length) {
		value = 0;
		for (;;) {
			--length;
			value = (value << 7) | (*pos & 0x7F);
			if (!(*pos++ & 0x80)) {
				/* Last octet in the OID subidentifier value */
				if (num_values < ARRAY_LEN(oid->value)) {
					oid->value[num_values] = value;
					if (ctrl->debug & PRI_DEBUG_APDU) {
						pri_message(ctrl, "%c%u", delimiter, value);
					}
					delimiter = '.';
				} else {
					/* Too many OID subidentifier values */
					delimiter = '~';
					if (ctrl->debug & PRI_DEBUG_APDU) {
						pri_message(ctrl, "%c%u", delimiter, value);
					}
				}
				++num_values;
				break;
			}
			if (!length) {
				oid->num_values = 0;
				if (ctrl->debug & PRI_DEBUG_APDU) {
					pri_message(ctrl, "\n"
						"    Last OID subidentifier value not terminated!\n");
				}
				return NULL;
			}
		}
	}
	if (ctrl->debug & PRI_DEBUG_APDU) {
		pri_message(ctrl, "\n");
	}

	if (num_values <= ARRAY_LEN(oid->value)) {
		oid->num_values = num_values;
		return pos;
	} else {
		/* Need to increase the size of the OID subidentifier list. */
		oid->num_values = 0;
		if (ctrl->debug & PRI_DEBUG_APDU) {
			pri_message(ctrl, "    Too many OID values!\n");
		}
		return NULL;
	}
}

/*!
 * \brief Decode a binary string primitive.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this primitive.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param buf_size Size of the supplied string buffer. (Must be nonzero)
 * \param str Where to put the decoded string.
 * \param str_len Length of the decoded string.
 *
 * \note The string will be null terminated just in case.
 * The buffer needs to have enough room for a null terminator.
 * \note The parse will fail if the parsed string is too large for
 * the supplied buffer.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *asn1_dec_string_bin(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end, size_t buf_size,
	unsigned char *str, size_t *str_len)
{
	int length;
	size_t sub_buf_size;
	size_t sub_str_len;
	unsigned char *sub_str;

	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	if (length < 0) {
		/* This is an indefinite length string */
		if (ctrl->debug & PRI_DEBUG_APDU) {
			pri_message(ctrl, "  %s %s = Indefinite length string\n", name,
				asn1_tag2str(tag));
		}
		if ((tag & ASN1_PC_MASK) == ASN1_PC_CONSTRUCTED) {
			/*
			 * This is an ITU encoded indefinite length string
			 * and could contain null.
			 */

			/* Ensure that an empty string is null terminated. */
			*str = 0;

			/* Collect all substrings into the original string buffer. */
			*str_len = 0;
			sub_str = str;
			sub_buf_size = buf_size;
			for (;;) {
				ASN1_CALL(pos, asn1_dec_tag(pos, end, &tag));
				if (tag == ASN1_INDEF_TERM) {
					/* End-of-contents octets */
					break;
				}

				/* Append the substring to the accumulated indefinite string. */
				ASN1_CALL(pos, asn1_dec_string_bin(ctrl, name, tag, pos, end,
					sub_buf_size, sub_str, &sub_str_len));

				sub_buf_size -= sub_str_len;
				sub_str += sub_str_len;
				*str_len += sub_str_len;
			}
		} else {
			/*
			 * This is a non-ITU encoded indefinite length string
			 * and must not contain null's.
			 */
			for (length = 0;; ++length) {
				if (end <= pos + length) {
					/* Not enough buffer left */
					return NULL;
				}
				if (pos[length] == 0) {
					/* Found End-of-contents octets */
					break;
				}
			}

			if (buf_size - 1 < length) {
				/* The destination buffer is not large enough for the data */
				if (ctrl->debug & PRI_DEBUG_APDU) {
					pri_message(ctrl, "    String buffer not large enough!\n");
				}
				return NULL;
			}

			/* Extract the string and null terminate it. */
			memcpy(str, pos, length);
			str[length] = 0;
			*str_len = length;

			pos += length + 1;
		}
		if (end <= pos) {
			/* Not enough buffer left for End-of-contents octets */
			return NULL;
		}
		if (*pos++ != 0) {
			/* We actually did not find the End-of-contents octets. */
			return NULL;
		}
		if (ctrl->debug & PRI_DEBUG_APDU) {
			/* Dump the collected string buffer contents. */
			pri_message(ctrl, "    Completed string =\n");
			asn1_dump_mem(ctrl, 6, str, *str_len);
		}
	} else {
		/* This is a definite length string */
		if (buf_size - 1 < length) {
			/* The destination buffer is not large enough for the data */
			if (ctrl->debug & PRI_DEBUG_APDU) {
				pri_message(ctrl, "  %s %s = Buffer not large enough!\n", name,
					asn1_tag2str(tag));
			}
			return NULL;
		}

		/* Extract the string and null terminate it. */
		memcpy(str, pos, length);
		str[length] = 0;
		*str_len = length;

		pos += length;

		if (ctrl->debug & PRI_DEBUG_APDU) {
			/* Dump the collected string buffer contents. */
			pri_message(ctrl, "  %s %s =\n", name, asn1_tag2str(tag));
			asn1_dump_mem(ctrl, 4, str, *str_len);
		}
	}

	return pos;
}

/*!
 * \brief Decode a string that can be truncated to a maximum length primitive.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param name Field name
 * \param tag Component tag that identified this primitive.
 * \param pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 decoding data buffer.
 * \param buf_size Size of the supplied string buffer. (Must be nonzero)
 * \param str Where to put the decoded null terminated string.
 * \param str_len Length of the decoded string.
 * (computed for convenience since you could just do a strlen())
 *
 * \note The parsed string will be truncated if the string buffer
 * cannot contain it.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
const unsigned char *asn1_dec_string_max(struct pri *ctrl, const char *name,
	unsigned tag, const unsigned char *pos, const unsigned char *end, size_t buf_size,
	unsigned char *str, size_t *str_len)
{
	int length;
	size_t str_length;
	size_t sub_buf_size;
	size_t sub_str_len;
	unsigned char *sub_str;

	ASN1_CALL(pos, asn1_dec_length(pos, end, &length));
	if (length < 0) {
		/* This is an indefinite length string */
		if (ctrl->debug & PRI_DEBUG_APDU) {
			pri_message(ctrl, "  %s %s = Indefinite length string\n", name,
				asn1_tag2str(tag));
		}
		if ((tag & ASN1_PC_MASK) == ASN1_PC_CONSTRUCTED) {
			/* This is an ITU encoded indefinite length string. */

			/* Ensure that an empty string is null terminated. */
			*str = 0;

			/* Collect all substrings into the original string buffer. */
			*str_len = 0;
			sub_str = str;
			sub_buf_size = buf_size;
			for (;;) {
				ASN1_CALL(pos, asn1_dec_tag(pos, end, &tag));
				if (tag == ASN1_INDEF_TERM) {
					/* End-of-contents octets */
					break;
				}

				/* Append the substring to the accumulated indefinite string. */
				ASN1_CALL(pos, asn1_dec_string_max(ctrl, name, tag, pos, end,
					sub_buf_size, sub_str, &sub_str_len));

				sub_buf_size -= sub_str_len;
				sub_str += sub_str_len;
				*str_len += sub_str_len;
			}
		} else {
			/* This is a non-ITU encoded indefinite length string. */
			for (length = 0;; ++length) {
				if (end <= pos + length) {
					/* Not enough buffer left */
					return NULL;
				}
				if (pos[length] == 0) {
					/* Found End-of-contents octets */
					break;
				}
			}

			/* Extract the string, truncate if necessary, and terminate it. */
			str_length = (buf_size - 1 < length) ? buf_size - 1 : length;
			memcpy(str, pos, str_length);
			str[str_length] = 0;
			*str_len = str_length;

			pos += length + 1;
		}
		if (end <= pos) {
			/* Not enough buffer left for End-of-contents octets */
			return NULL;
		}
		if (*pos++ != 0) {
			/* We actually did not find the End-of-contents octets. */
			return NULL;
		}
		if (ctrl->debug & PRI_DEBUG_APDU) {
			pri_message(ctrl, "    Completed string = \"%s\"\n", str);
		}
	} else {
		/*
		 * This is a definite length string
		 *
		 * Extract the string, truncate if necessary, and terminate it.
		 */
		str_length = (buf_size - 1 < length) ? buf_size - 1 : length;
		memcpy(str, pos, str_length);
		str[str_length] = 0;
		*str_len = str_length;

		pos += length;

		if (ctrl->debug & PRI_DEBUG_APDU) {
			pri_message(ctrl, "  %s %s = \"%s\"\n", name, asn1_tag2str(tag), str);
		}
	}

	return pos;
}

/*!
 * \internal
 * \brief Recursive ASN.1 buffer decoding dump helper.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param pos ASN.1 tag starting position.
 * \param end End of ASN.1 decoding data buffer.
 * \param level Indentation level to use.
 * \param indefinite_term TRUE if to stop on an indefinite length terminator.
 *
 * \retval Start of the next ASN.1 component on success.
 * \retval NULL on error.
 */
static const unsigned char *asn1_dump_helper(struct pri *ctrl, const unsigned char *pos,
	const unsigned char *end, unsigned level, unsigned indefinite_term)
{
	unsigned delimiter;
	unsigned tag;
	int length;
	const unsigned char *len_pos;

	while (pos < end && (!indefinite_term || *pos != ASN1_INDEF_TERM)) {
		/* Decode the tag */
		pri_message(ctrl, "%*s", 2 * level, "");
		len_pos = asn1_dec_tag(pos, end, &tag);
		if (!len_pos) {
			pri_message(ctrl, "Invalid tag encoding!\n");
			return NULL;
		}

		/* Dump the tag contents. */
		pri_message(ctrl, "%s ", asn1_tag2str(tag));
		delimiter = '<';
		while (pos < len_pos) {
			pri_message(ctrl, "%c%02X", delimiter, *pos);
			delimiter = ' ';
			++pos;
		}
		pri_message(ctrl, "> ");

		/* Decode the length */
		pos = asn1_dec_length(len_pos, end, &length);
		if (!pos) {
			pri_message(ctrl, "Invalid length encoding!\n");
			return NULL;
		}

		/* Dump the length contents. */
		if (length < 0) {
			pri_message(ctrl, "Indefinite length ");
		} else {
			pri_message(ctrl, "Len:%d ", length);
		}
		delimiter = '<';
		while (len_pos < pos) {
			pri_message(ctrl, "%c%02X", delimiter, *len_pos);
			delimiter = ' ';
			++len_pos;
		}
		pri_message(ctrl, ">\n");

		/* Dump the body contents */
		++level;
		if (length < 0) {
			/* Indefinite length */
			if ((tag & ASN1_PC_MASK) == ASN1_PC_CONSTRUCTED) {
				/* This is an ITU encoded indefinite length component. */
				ASN1_CALL(pos, asn1_dump_helper(ctrl, pos, end, level, 1));
			} else if (tag == (ASN1_CLASS_UNIVERSAL | ASN1_PC_PRIMITIVE | ASN1_TYPE_SET)
				|| tag ==
				(ASN1_CLASS_UNIVERSAL | ASN1_PC_PRIMITIVE | ASN1_TYPE_SEQUENCE)) {
				pri_message(ctrl, "%*sThis tag must always be constructed!\n", 2 * level,
					"");
				/* Assume tag was constructed to keep going */
				ASN1_CALL(pos, asn1_dump_helper(ctrl, pos, end, level, 1));
			} else {
				/* This is a non-ITU encoded indefinite length component. */
				pri_message(ctrl, "%*sNon-ITU indefininte length component.\n",
					2 * level, "");
				length = 0;
				while (pos + length < end && pos[length] != ASN1_INDEF_TERM) {
					++length;
				}
				if (length) {
					asn1_dump_mem(ctrl, 2 * level, pos, length);
					pos += length;
				}
			}
			--level;
			if (end < pos + ASN1_INDEF_TERM_LEN) {
				pri_message(ctrl, "%*sNot enough room for the End-of-contents octets!\n",
					2 * level, "");
				pos = end;
			} else {
				pri_message(ctrl, "%*sEnd-of-contents <%02X %02X>%s\n", 2 * level, "",
					pos[0], pos[1], (pos[1] != 0) ? " Invalid!" : "");
				pos += ASN1_INDEF_TERM_LEN;
			}
		} else {
			/* Defininte length */
			if ((tag & ASN1_PC_MASK) == ASN1_PC_CONSTRUCTED) {
				/* Dump constructed contents */
				ASN1_CALL(pos, asn1_dump_helper(ctrl, pos, pos + length, level, 0));
			} else if (tag == (ASN1_CLASS_UNIVERSAL | ASN1_PC_PRIMITIVE | ASN1_TYPE_SET)
				|| tag ==
				(ASN1_CLASS_UNIVERSAL | ASN1_PC_PRIMITIVE | ASN1_TYPE_SEQUENCE)) {
				/* Assume tag was constructed to keep going */
				pri_message(ctrl, "%*sThis tag must always be constructed!\n", 2 * level,
					"");
				ASN1_CALL(pos, asn1_dump_helper(ctrl, pos, pos + length, level, 0));
			} else if (0 < length) {
				/* Dump primitive contents. */
				asn1_dump_mem(ctrl, 2 * level, pos, length);
				pos += length;
			}
			--level;
		}

#if 0
		pri_message(ctrl, "%*sEnd\n", 2 * level, "");
#endif
	}

	return pos;
}

/*!
 * \brief Dump the given ASN.1 buffer contents.
 *
 * \param ctrl D channel controller for any diagnostic messages.
 * \param start_asn1 First octet in the ASN.1 buffer. (ASN.1 tag starting position)
 * \param end One beyond the last octet in the ASN.1 buffer.
 *
 * \return Nothing
 */
void asn1_dump(struct pri *ctrl, const unsigned char *start_asn1,
	const unsigned char *end)
{
	pri_message(ctrl, "ASN.1 dump\n");
	if (start_asn1) {
		asn1_dump_helper(ctrl, start_asn1, end, 1, 0);
	}
	pri_message(ctrl, "ASN.1 end\n");
}

/*!
 * \brief Encode the length of an ASN.1 component body of predetermined size.
 *
 * \param len_pos Starting position of the ASN.1 component length.
 * \param end End of ASN.1 encoding data buffer.
 * \param length Predetermined component body length.
 *
 * \note The encoding buffer does not need to be checked after calling.
 * It is already checked to have the requested room.
 *
 * \retval Next octet after the length on success.
 * \retval NULL on error.
 */
unsigned char *asn1_enc_length(unsigned char *len_pos, unsigned char *end, size_t length)
{
	u_int32_t body_length;		/* Length of component contents */
	u_int32_t value;
	u_int32_t test_mask;
	unsigned length_size;		/* Length of the length encoding */

	body_length = length;

	/* Determine length encoding length */
	if (body_length < 128) {
		length_size = 1;
	} else {
		/* Find most significant octet of 32 bit integer that carries meaning. */
		test_mask = 0xFF000000;
		for (length_size = 4; --length_size;) {
			if (body_length & test_mask) {
				/*
				 * Found the first 8 bits of a multiple octet length that
				 * is not all zeroes.
				 */
				break;
			}
			test_mask >>= 8;
		}
		length_size += 1 + 1;
	}

	if (end < len_pos + length_size + body_length) {
		/* No room for the length and component body in the buffer */
		return NULL;
	}

	/* Encode the component body length */
	if (length_size == 1) {
		*len_pos++ = body_length;
	} else {
		*len_pos++ = 0x80 | --length_size;
		while (length_size--) {
			value = body_length;
			value >>= (8 * length_size);
			*len_pos++ = value & 0xFF;
		}
	}

	return len_pos;
}

/*!
 * \brief Encode the length of an already encoded ASN.1 component.
 *
 * \param len_pos Starting position of the ASN.1 component length.
 * \param component_end Next octet after the component body.
 * \param end End of ASN.1 encoding data buffer.
 *
 * \note The total component size could increase or decrease.
 * \note The component length field must have been initialized with
 * ASN1_LEN_INIT() or ASN1_CONSTRUCTED_BEGIN().
 *
 * \retval Next octet after the component body on success.
 * \retval NULL on error.
 */
unsigned char *asn1_enc_length_fixup(unsigned char *len_pos,
	unsigned char *component_end, unsigned char *end)
{
	u_int32_t body_length;		/* Length of component contents */
	u_int32_t value;
	u_int32_t test_mask;
	unsigned length_size;		/* Length of the length encoding */

	if (component_end < len_pos + *len_pos) {
		/* Sanity check */
		return NULL;
	}

	body_length = component_end - len_pos - *len_pos;

	/* Determine length encoding length */
	if (body_length < 128) {
		length_size = 1;
	} else {
		/* Find most significant octet of 32 bit integer that carries meaning. */
		test_mask = 0xFF000000;
		for (length_size = 4; --length_size;) {
			if (body_length & test_mask) {
				/*
				 * Found the first 8 bits of a multiple octet length that
				 * is not all zeroes.
				 */
				break;
			}
			test_mask >>= 8;
		}
		length_size += 1 + 1;
	}

	component_end = len_pos + length_size + body_length;
	if (end < component_end) {
		/* No room for the component in the buffer */
		return NULL;
	}
	if (length_size != *len_pos) {
		/* Must shift the component body */
		memmove(len_pos + length_size, len_pos + *len_pos, body_length);
	}

	/* Encode the component body length */
	if (length_size == 1) {
		*len_pos = body_length;
	} else {
		*len_pos++ = 0x80 | --length_size;
		while (length_size--) {
			value = body_length;
			value >>= (8 * length_size);
			*len_pos++ = value & 0xFF;
		}
	}

	return component_end;
}

/*!
 * \brief Encode the boolean primitive.
 *
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 * \param value Component value to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *asn1_enc_boolean(unsigned char *pos, unsigned char *end, unsigned tag,
	int32_t value)
{
	if (end < pos + 3) {
		/* No room for the component in the buffer */
		return NULL;
	}

	/* Encode component */
	*pos++ = tag;
	*pos++ = 1;
	*pos++ = value ? 1 : 0;

	return pos;
}

/*!
 * \brief Encode the integer type primitive.
 *
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 * \param value Component value to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *asn1_enc_int(unsigned char *pos, unsigned char *end, unsigned tag,
	int32_t value)
{
	unsigned count;
	u_int32_t test_mask;
	u_int32_t val;

	/* Find most significant octet of 32 bit integer that carries meaning. */
	test_mask = 0xFF800000;
	val = (u_int32_t) value;
	for (count = 4; --count;) {
		if ((val & test_mask) != test_mask && (val & test_mask) != 0) {
			/*
			 * The first 9 bits of a multiple octet integer is not
			 * all ones or zeroes.
			 */
			break;
		}
		test_mask >>= 8;
	}

	if (end < pos + 3 + count) {
		/* No room for the component in the buffer */
		return NULL;
	}

	/* Encode component */
	*pos++ = tag;
	*pos++ = count + 1;
	do {
		val = (u_int32_t) value;
		val >>= (8 * count);
		*pos++ = val & 0xFF;
	} while (count--);

	return pos;
}

/*!
 * \brief Encode the null type primitive.
 *
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *asn1_enc_null(unsigned char *pos, unsigned char *end, unsigned tag)
{
	if (end < pos + 2) {
		/* No room for the component in the buffer */
		return NULL;
	}

	/* Encode component */
	*pos++ = tag;
	*pos++ = 0;

	return pos;
}

/*!
 * \brief Encode the object identifier (OID) primitive.
 *
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 * \param oid Component value to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *asn1_enc_oid(unsigned char *pos, unsigned char *end, unsigned tag,
	const struct asn1_oid *oid)
{
	unsigned char *len_pos;
	unsigned num_values;
	unsigned count;
	u_int32_t value;

	if (end < pos + 2) {
		/* No room for the component tag and length in the buffer */
		return NULL;
	}

	*pos++ = tag;
	len_pos = pos++;

	/* For all OID subidentifer values */
	for (num_values = 0; num_values < oid->num_values; ++num_values) {
		/*
		 * Count the number of 7 bit chunks that are needed
		 * to encode the integer.
		 */
		value = oid->value[num_values] >> 7;
		for (count = 0; value; ++count) {
			/* There are bits still set */
			value >>= 7;
		}

		if (end < pos + count + 1) {
			/* No room for the component body in the buffer */
			return NULL;
		}

		/* Store OID subidentifier value */
		do {
			value = oid->value[num_values];
			value >>= (7 * count);
			*pos++ = (value & 0x7F) | (count ? 0x80 : 0);
		} while (count--);
	}

	/* length */
	*len_pos = pos - len_pos - 1;

	return pos;
}

/*!
 * \brief Encode the binary string type primitive.
 *
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 * \param str Binary string to encode.
 * \param str_len Length of binary string to encode.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *asn1_enc_string_bin(unsigned char *pos, unsigned char *end, unsigned tag,
	const unsigned char *str, size_t str_len)
{
	if (end < pos + 1) {
		/* No room for the component tag in the buffer */
		return NULL;
	}

	/* Encode component */
	*pos++ = tag;
	ASN1_CALL(pos, asn1_enc_length(pos, end, str_len));
	memcpy(pos, str, str_len);

	return pos + str_len;
}

/*!
 * \brief Encode a string that can be truncated to a maximum length primitive.
 *
 * \param pos Starting position to encode ASN.1 component.
 * \param end End of ASN.1 encoding data buffer.
 * \param tag Component tag to identify the encoded component.
 * \param str Null terminated string to encode.
 * \param max_len Maximum length of string to encode.
 *
 * \note The string will be truncated if it is too long.
 *
 * \retval Start of the next ASN.1 component to encode on success.
 * \retval NULL on error.
 */
unsigned char *asn1_enc_string_max(unsigned char *pos, unsigned char *end, unsigned tag,
	const unsigned char *str, size_t max_len)
{
	size_t str_len;

	str_len = strlen((char *) str);
	if (max_len < str_len) {
		str_len = max_len;
	}
	return asn1_enc_string_bin(pos, end, tag, str, str_len);
}

/* ------------------------------------------------------------------- */
/* end asn1_primitive.c */
