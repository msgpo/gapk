/* Formats handling */

/*
 * This file is part of gapk (GSM Audio Pocket Knife).
 *
 * gapk is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gapk is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gapk.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdint.h>

enum osmo_gapk_format_type {
	FMT_INVALID = 0,

	/* Classic .amr container */
	FMT_AMR_EFR,

	/* Classic .gsm file for FR */
	FMT_GSM,

	/* 3GPP Reference HR vocodec files */
	FMT_HR_REF_DEC,
	FMT_HR_REF_ENC,

	/* Racal 6103E TCH recordings */
	FMT_RACAL_HR,
	FMT_RACAL_FR,
	FMT_RACAL_EFR,

	/* Raw PCM */
	FMT_RAWPCM_S16LE,

	/* Texas Instrument calypso/locosto buffer format */
	FMT_TI_HR,
	FMT_TI_FR,
	FMT_TI_EFR,

	/* AMR encoded data, variable length */
	FMT_AMR_OPENCORE,
	FMT_RTP_AMR,

	FMT_RTP_EFR,

	/* HR in RTP according to ETSI TS 101 318 */
	FMT_RTP_HR_ETSI,
	/* HR in RTP according to IETF RFC5993 */
	FMT_RTP_HR_IETF,

	_FMT_MAX,
};

/* Need to import here because of enum interdep */
#include <osmocom/gapk/codecs.h>

/*! call-back for actual format conversion function
 *  \param[out] dst caller-allocated buffer for output data
 *  \param[in] src input data
 *  \param[in] src_len length of input data \a src
 *  \returns number of output bytes written to \a dst; negative on error */
typedef int (*osmo_gapk_fmt_conv_cb_t)(uint8_t *dst,
	const uint8_t *src, unsigned int src_len);

struct osmo_gapk_format_desc {
	enum osmo_gapk_format_type type;
	enum osmo_gapk_codec_type codec_type;
	const char *description;
	const char *name;

	/*! Length of frames in this format (as opposed to canonical) */
	unsigned int frame_len;

	/* Format conversation function pointers */
	osmo_gapk_fmt_conv_cb_t conv_from_canon;
	osmo_gapk_fmt_conv_cb_t conv_to_canon;

	/*! Length of a (global) header at start of file */
	unsigned int header_len;
	/*! Exact match for (global) header at start of file */
	const uint8_t *header;
};

const struct osmo_gapk_format_desc *
osmo_gapk_fmt_get_from_type(enum osmo_gapk_format_type type);
const struct osmo_gapk_format_desc *
osmo_gapk_fmt_get_from_name(const char *name);
