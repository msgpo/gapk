/* HR (GSM 06.20) codec wrapper */

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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include <gsmhr/gsmhr.h>

#include "refsrc/typedefs.h"
#include "refsrc/homing.h"
#include "refsrc/sp_dec.h"
#include "refsrc/sp_enc.h"


#define EXPORT __attribute__((visibility("default")))


struct gsmhr {
	int dec_reset_flg;

	void *bss_base;
	void *bss_save;
	unsigned bss_len;
};

EXPORT struct gsmhr *
gsmhr_init(void)
{
	struct gsmhr *state;
	void *lib;

	lib = dlopen("libgsmhr.so.0", RTLD_NOW | RTLD_NOLOAD);
	if (!lib)
		return NULL;

	state = calloc(1, sizeof(struct gsmhr));
	if (!state)
		return NULL;

	state->dec_reset_flg = 1;

	state->bss_base = dlsym(lib, "__bss_start");
	state->bss_len  = dlsym(lib, "_end") - state->bss_base;
	state->bss_save = malloc(state->bss_len);

	if (!state->bss_save)
	{
		free(state);
		return NULL;
	}

	resetDec();
	resetEnc();

	memcpy(state->bss_save, state->bss_base, state->bss_len);

	return state;
}

EXPORT void
gsmhr_exit(struct gsmhr *state)
{
	free(state->bss_save);
	free(state);
}

EXPORT int
gsmhr_encode(struct gsmhr *state, int16_t *hr_params, const int16_t *pcm)
{
	int enc_reset_flg;
	Shortword pcm_b[F_LEN];

	memcpy(state->bss_base, state->bss_save, state->bss_len);

	memcpy(pcm_b, pcm, F_LEN*sizeof(int16_t));

	enc_reset_flg = encoderHomingFrameTest(pcm_b);

	speechEncoder(pcm_b, hr_params);

	if (enc_reset_flg)
		resetEnc();

	memcpy(state->bss_save, state->bss_base, state->bss_len);

	return 0;
}

EXPORT int
gsmhr_decode(struct gsmhr *state, int16_t *pcm, const int16_t *hr_params)
{
#define WHOLE_FRAME		18
#define TO_FIRST_SUBFRAME	 9

	int dec_reset_flg;
	Shortword hr_params_b[22];

	memcpy(state->bss_base, state->bss_save, state->bss_len);

	memcpy(hr_params_b, hr_params, 22*sizeof(int16_t));

	if (state->dec_reset_flg)
		dec_reset_flg = decoderHomingFrameTest(hr_params_b, TO_FIRST_SUBFRAME);
	else
		dec_reset_flg = 0;

	if (dec_reset_flg && state->dec_reset_flg) {
		int i;
		for (i=0; i<F_LEN; i++)
			pcm[i] = EHF_MASK;
	} else {
		speechDecoder(hr_params_b, pcm);
	}

	if (!state->dec_reset_flg)
		dec_reset_flg = decoderHomingFrameTest(hr_params_b, WHOLE_FRAME);

	if (dec_reset_flg)
		resetDec();

	state->dec_reset_flg = dec_reset_flg;

	memcpy(state->bss_save, state->bss_base, state->bss_len);

	return 0;
}
