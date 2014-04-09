/* 
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005/2006, Anthony Minessale II <anthmct@yahoo.com>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Anthony Minessale II <anthmct@yahoo.com>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * 
 * Anthony Minessale II <anthmct@yahoo.com>
 * Michael Jerris <mike@jerris.com>
 * <mkrivushin@yandex.ru> for fsg729 ( which used IPP media libraries )
 * Matteo Brancaleoni <mbrancaleoni@gmail.com>
 *
 * The bcg729 codec itself is not distributed with this module.
 *
 * mod_bcg729.c -- G.729 Codec Module
 *
 */

#include "switch.h"
#include "bcg729/encoder.h"
#include "bcg729/decoder.h"

SWITCH_MODULE_LOAD_FUNCTION(mod_bcg729_load);
SWITCH_MODULE_DEFINITION(mod_bcg729, mod_bcg729_load, NULL, NULL);

struct bcg729_context {
	bcg729DecoderChannelContextStruct *decoder_object;
	bcg729EncoderChannelContextStruct *encoder_object;
};

static switch_status_t switch_bcg729_init(switch_codec_t *codec, switch_codec_flag_t flags, const switch_codec_settings_t *codec_settings)
{
	struct bcg729_context *context = NULL;
	int encoding, decoding;

	encoding = (flags & SWITCH_CODEC_FLAG_ENCODE);
	decoding = (flags & SWITCH_CODEC_FLAG_DECODE);

	if (!(encoding || decoding) || (!(context = switch_core_alloc(codec->memory_pool, sizeof(struct bcg729_context))))) {
		return SWITCH_STATUS_FALSE;
	} else {
        /*
		if (codec->fmtp_in) {
			codec->fmtp_out = switch_core_strdup(codec->memory_pool, codec->fmtp_in);
		}
        */
		codec->fmtp_out = switch_core_strdup(codec->memory_pool, "annexb=no");

		if (encoding) {
            context->encoder_object = initBcg729EncoderChannel();
		}

		if (decoding) {
            context->decoder_object = initBcg729DecoderChannel();
		}

		codec->private_info = context;

		return SWITCH_STATUS_SUCCESS;
	}
}

static switch_status_t switch_bcg729_destroy(switch_codec_t *codec)
{
    struct bcg729_context *context = codec->private_info;

    closeBcg729EncoderChannel(context->encoder_object);
    closeBcg729DecoderChannel(context->decoder_object);

	codec->private_info = NULL;
	return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_bcg729_encode(switch_codec_t *codec,
										  switch_codec_t *other_codec,
										  void *decoded_data,
										  uint32_t decoded_data_len,
										  uint32_t decoded_rate, void *encoded_data, uint32_t *encoded_data_len, uint32_t *encoded_rate,
										  unsigned int *flag)
{
	struct bcg729_context *context = codec->private_info;

	if (!context) {
		return SWITCH_STATUS_FALSE;
	}

	if (decoded_data_len % 160 == 0) {
		uint32_t new_len = 0;
		int16_t *ddp = decoded_data;
		uint8_t *edp = encoded_data;
		int x;
		int loops = (int) decoded_data_len / 160;

		for (x = 0; x < loops && new_len < *encoded_data_len; x++) {
            bcg729Encoder(context->encoder_object, ddp, edp);
			edp += 10;
			ddp += 80;
			new_len += 10;
		}

		if (new_len <= *encoded_data_len) {
			*encoded_data_len = new_len;
		} else {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "buffer overflow!!! %u >= %u\n", new_len, *encoded_data_len);
			return SWITCH_STATUS_FALSE;
		}
	}
	return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_bcg729_decode(switch_codec_t *codec,
										    switch_codec_t *other_codec,
										    void *encoded_data,
										    uint32_t encoded_data_len,
										    uint32_t encoded_rate, 
                                            void *decoded_data, 
                                            uint32_t *decoded_data_len, 
                                            uint32_t *decoded_rate,
										    unsigned int *flag)
{
    struct bcg729_context *context = codec->private_info;
    if (!context) {
        return SWITCH_STATUS_FALSE;
    }

    int framesize;
    int x;
    uint32_t new_len = 0;
    uint8_t *edp = encoded_data;
    int16_t *ddp = decoded_data;

    if (encoded_data_len == 0) {  /* Native PLC interpolation */
        bcg729Decoder(context->decoder_object, NULL, 1, ddp);
	    ddp += 80; 
        decoded_data_len = (uint32_t *) 160;
	    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "g729 zero length frame\n");
        return SWITCH_STATUS_SUCCESS;
    }

    for(x = 0; x < encoded_data_len && new_len < *decoded_data_len; x += framesize) {
        if(encoded_data_len - x < 8)
            framesize = 2;  /* SID */
        else
            framesize = 10; /* regular 729a frame */

        bcg729Decoder(context->decoder_object, edp, 0, ddp);
	    ddp += 80;
	    edp += framesize;
	    new_len += 160;
    }

    if (new_len <= *decoded_data_len) {
        *decoded_data_len = new_len;
    } else {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "buffer overflow!!!\n");
        return SWITCH_STATUS_FALSE;
    }

    return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_LOAD_FUNCTION(mod_bcg729_load)
{
	switch_codec_interface_t *codec_interface;
	int mpf = 10000, spf = 80, bpf = 160, ebpf = 10, count;

	/* connect my internal structure to the blank pointer passed to me */
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	SWITCH_ADD_CODEC(codec_interface, "G.729");
	for (count = 12; count > 0; count--) {
	    switch_core_codec_add_implementation(pool, codec_interface, SWITCH_CODEC_TYPE_AUDIO, 
                                                18, /* the IANA code number */
                                                "G729", /* the IANA code name */
                                                "annexb=no", /* default fmtp to send (can be overridden by the init function) */
                                                8000, /* samples transferred per second */
                                                8000, /* actual samples transferred per second */
                                                8000, /* bits transferred per second */
											    mpf * count, /* number of microseconds per frame */
                                                spf * count, /* number of samples per frame */
                                                bpf * count, /* number of bytes per frame decompressed */
                                                ebpf * count, /* number of bytes per frame compressed */
                                                1, /* number of channels represented */
                                                count * 10, /* number of frames per network packet */
											    switch_bcg729_init, /* function to initialize a codec handle using this implementation */
                                                switch_bcg729_encode, /* function to encode raw data into encoded data */
                                                switch_bcg729_decode, /* function to decode encoded data into raw data */
                                                switch_bcg729_destroy); /* deinitalize a codec handle using this implementation */
	}
	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}

/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4:
 */
