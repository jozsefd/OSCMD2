/**
 * PKCS15 emulation layer for cards with eSign and QES application
 *
 * Copyright (C) 2018, Jozsef Dojcsak
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "common/compat_strlcpy.h"
#include "internal.h"
#include "pkcs15.h"
#include "cardctl.h"
#include "log.h"

typedef struct cdata_st {
	const char *label;
	int	    authority;
	const char *path;
	const char *id;
	int         obj_flags;
} cdata, *pcdata;

typedef struct pdata_st {
	const char *id;
	const char *label;
	const char *path;
	int         ref;
	int         type;
	unsigned int maxlen;
	unsigned int minlen;
	unsigned int storedlen;
	int         flags;	
	int         tries_left;
	const char  pad_char;
	int         obj_flags;
} pindata, *ppindata; 

typedef struct prdata_st {
	const char *id;
	const char *label;
	unsigned int modulus_len;
	int         usage;
	const char *path;
	int         ref;
	const char *auth_id;
	int         obj_flags;
} prdata, *pprdata;

typedef struct container_st {
	const char *id;
	const pcdata certdata;
	const ppindata pindata;
	const pprdata prdata;
} container;

#define USAGE_NONREP	SC_PKCS15_PRKEY_USAGE_NONREPUDIATION
#define USAGE_KE	SC_PKCS15_PRKEY_USAGE_ENCRYPT | \
			SC_PKCS15_PRKEY_USAGE_DECRYPT | \
			SC_PKCS15_PRKEY_USAGE_WRAP    | \
			SC_PKCS15_PRKEY_USAGE_UNWRAP
#define USAGE_AUT	SC_PKCS15_PRKEY_USAGE_ENCRYPT | \
			SC_PKCS15_PRKEY_USAGE_DECRYPT | \
			SC_PKCS15_PRKEY_USAGE_WRAP    | \
			SC_PKCS15_PRKEY_USAGE_UNWRAP  | \
			SC_PKCS15_PRKEY_USAGE_SIGN


static int get_cert_len(sc_card_t *card, sc_path_t *path)
{
	int r;
	u8  buf[8];

	r = sc_select_file(card, path, NULL);
	if (r < 0)
		return 0;
	r = sc_read_binary(card, 0, buf, sizeof(buf), 0);
	if (r < 0)	
		return 0;
	if (buf[0] != 0x30 || buf[1] != 0x82)
		return 0;
	path->index = 0;
	path->count = ((((size_t) buf[2]) << 8) | buf[3]) + 4;
	return 1;
} 

static int detect_qes_c1(sc_pkcs15_card_t *p15card) {
	// known object on card
	const cdata auth_cert = { "C.CH.AUT", 0, "3F00060843F1", "1", 0 };
	const cdata encr_cert = { "C.CH.ENC", 0, "3F0006084301", "2", 0 };
	const cdata sign_cert = { "C.CH.QES", 0, "3F0006044301", "3", 0 };
	const cdata auth_root_cert = { "C.RootCA_Auth", 1, "3F00060843F0", "4", 0 };
	const cdata encr_root_cert = { "C.RootCA_Enc", 1, "3F0006084300", "5", 0 };
	const cdata sign_root_cert = { "C.RootCA_QES", 1, "3F0006044300", "6", 0 };
	const prdata auth_key = { "1", "PrK.CH.AUT", 2048, USAGE_AUT, "3F0006080F01", 0x81, "1", SC_PKCS15_CO_FLAG_PRIVATE };
	const prdata encr_key = { "2", "PrK.CH.ENC", 2048, USAGE_KE, "3F0006080F02", 0x83, "1", SC_PKCS15_CO_FLAG_PRIVATE };
	const prdata sign_key = { "3", "PrK.CH.QES", 2048, SC_PKCS15_PRKEY_USAGE_SIGN | SC_PKCS15_PRKEY_USAGE_NONREPUDIATION, "3F0006040F01", 0x84, "2", SC_PKCS15_CO_FLAG_PRIVATE };
	const prdata sign3072_key = { "3", "PrK.CH.QES", 3072, SC_PKCS15_PRKEY_USAGE_SIGN | SC_PKCS15_PRKEY_USAGE_NONREPUDIATION, "3F0006040F01", 0x84, "2", SC_PKCS15_CO_FLAG_PRIVATE };
	const pindata auth_pin = { "1", "Auth.PIN", "3F00", 0x01, SC_PKCS15_PIN_TYPE_UTF8, 16, 6, 0,
		SC_PKCS15_PIN_FLAG_INITIALIZED | SC_PKCS15_PIN_FLAG_CASE_SENSITIVE,
		-1, 0x00, SC_PKCS15_CO_FLAG_MODIFIABLE | SC_PKCS15_CO_FLAG_PRIVATE };
	const pindata sign_pin = { "2", "Sign.PIN", "3F000604", 0x81,  SC_PKCS15_PIN_TYPE_UTF8, 16, 6, 0,
		// denoting with SC_PKCS15_PIN_FLAG_CONFIDENTIALITY_PROTECTED that only class-2 readers are supported!
		SC_PKCS15_PIN_FLAG_INITIALIZED | SC_PKCS15_PIN_FLAG_CASE_SENSITIVE | SC_PKCS15_PIN_FLAG_CONFIDENTIALITY_PROTECTED | SC_PKCS15_PIN_FLAG_LOCAL,
		-1, 0x00, SC_PKCS15_CO_FLAG_MODIFIABLE | SC_PKCS15_CO_FLAG_PRIVATE };

	int    r, i;
	char   buf[256];
	sc_path_t path;
	sc_file_t *file = NULL;
	sc_card_t *card = p15card->card;
	sc_serial_number_t serial;

	/* get serial number */
	r = sc_card_ctl(card, SC_CARDCTL_GET_SERIALNR, &serial);
	if (r != SC_SUCCESS) return SC_ERROR_INTERNAL;

	r = sc_bin_to_hex(serial.value, serial.len, buf, sizeof(buf), 0);
	if (r != SC_SUCCESS) return SC_ERROR_INTERNAL;

	if (p15card->tokeninfo->serial_number) free(p15card->tokeninfo->serial_number);
	p15card->tokeninfo->serial_number = (char*)malloc(strlen(buf) + 1);
	if (!p15card->tokeninfo->serial_number) return SC_ERROR_INTERNAL;
	strcpy(p15card->tokeninfo->serial_number, buf);

	/* the manufacturer ID, in this case Giesecke & Devrient GmbH */
	if (p15card->tokeninfo->manufacturer_id) free(p15card->tokeninfo->manufacturer_id);
	p15card->tokeninfo->manufacturer_id = strdup("GuD");

	const container containers[] = {
		{ "1", &auth_cert, &auth_pin, &auth_key },
		{ "2", &encr_cert, &auth_pin, &encr_key },
		{ "3", &sign_cert, &sign_pin, (card->type == SC_CARD_TYPE_STARCOS_V3_5 ? &sign3072_key : &sign_key) },
		{ "4", &auth_root_cert, 0, 0 },
		{ "5", &encr_root_cert, 0, 0 },
		{ "6", &sign_root_cert, 0, 0 },
	};

	ppindata installed_pins[2];
	int installed_pin_count = 0;

	/* enumerate containers */
	for( i=0; i<sizeof(containers)/sizeof(container); i++) {
		struct sc_pkcs15_cert_info cert_info;
		struct sc_pkcs15_object    cert_obj;

		memset(&cert_info, 0, sizeof(cert_info));
		memset(&cert_obj,  0, sizeof(cert_obj));

		sc_pkcs15_format_id(containers[i].id, &cert_info.id);
		cert_info.authority = containers[i].certdata->authority;
		sc_format_path(containers[i].certdata->path, &cert_info.path);
		if (!get_cert_len(card, &cert_info.path))
			/* skip errors */
			continue;

		strlcpy(cert_obj.label, containers[i].certdata->label, sizeof(cert_obj.label));
		cert_obj.flags = containers[i].certdata->obj_flags;

		r = sc_pkcs15emu_add_x509_cert(p15card, &cert_obj, &cert_info);
		if (r < 0)
			return SC_ERROR_INTERNAL;

		if (containers[i].pindata != 0) {
			// check if pin is installed.
			int j;
			int is_pin_installed = 0;
			for (int j = 0; j < installed_pin_count; j++) {
				if (installed_pins[j] == containers[i].pindata) {
					is_pin_installed = 1;
					break;
				}
			}

			if (!is_pin_installed) {
				struct sc_pkcs15_auth_info pin_info;
				struct sc_pkcs15_object   pin_obj;
				int reference = containers[i].pindata->ref;

				installed_pins[installed_pin_count++] = containers[i].pindata;

				if (reference == 0x01 && card->type == SC_CARD_TYPE_STARCOS_V3_5) {
					//fix: 3.5 card uses PIN-ID 06 for Auth-PIN!
					reference = 0x06;
				}

				memset(&pin_info, 0, sizeof(pin_info));
				memset(&pin_obj, 0, sizeof(pin_obj));

				sc_pkcs15_format_id(containers[i].pindata->id, &pin_info.auth_id);
				pin_info.auth_type = SC_PKCS15_PIN_AUTH_TYPE_PIN;
				pin_info.attrs.pin.reference = reference;
				pin_info.attrs.pin.flags = containers[i].pindata->flags;
				pin_info.attrs.pin.type = containers[i].pindata->type;
				pin_info.attrs.pin.min_length = containers[i].pindata->minlen;
				pin_info.attrs.pin.stored_length = containers[i].pindata->storedlen;
				pin_info.attrs.pin.max_length = containers[i].pindata->maxlen;
				pin_info.attrs.pin.pad_char = containers[i].pindata->pad_char;
				if (containers[i].pindata->path != NULL) sc_format_path(containers[i].pindata->path, &pin_info.path);
				pin_info.tries_left = -1;

				strlcpy(pin_obj.label, containers[i].pindata->label, sizeof(pin_obj.label));
				pin_obj.flags = containers[i].pindata->obj_flags;

				r = sc_pkcs15emu_add_pin_obj(p15card, &pin_obj, &pin_info);
				if (r < 0)
					return SC_ERROR_INTERNAL;
			}
		}

		if (containers[i].prdata != 0) {
			struct sc_pkcs15_prkey_info prkey_info;
			struct sc_pkcs15_object     prkey_obj;
			int modulus_len = containers[i].prdata->modulus_len;
			memset(&prkey_info, 0, sizeof(prkey_info));
			memset(&prkey_obj, 0, sizeof(prkey_obj));

			sc_pkcs15_format_id(containers[i].id, &prkey_info.id);
			prkey_info.usage = containers[i].prdata->usage;
			prkey_info.native = 1;
			prkey_info.key_reference = containers[i].prdata->ref;
			prkey_info.modulus_length = modulus_len;
			sc_format_path(containers[i].prdata->path, &prkey_info.path);

			strlcpy(prkey_obj.label, containers[i].prdata->label, sizeof(prkey_obj.label));
			prkey_obj.flags = containers[i].prdata->obj_flags;
			if (containers[i].prdata->auth_id)
				sc_pkcs15_format_id(containers[i].prdata->auth_id, &prkey_obj.auth_id);

			r = sc_pkcs15emu_add_rsa_prkey(p15card, &prkey_obj, &prkey_info);
			if (r < 0)
				return SC_ERROR_INTERNAL;
		}
	}
	
	return SC_SUCCESS;
}

int sc_pkcs15emu_esign_qes_init_ex(
	sc_pkcs15_card_t   *p15card,
	struct sc_aid	   *aid,
	sc_pkcs15emu_opt_t *opts
){
	sc_card_t         *card = p15card->card;
	sc_context_t      *ctx = p15card->card->ctx;
	sc_serial_number_t serialnr;
	char               serial[30];
	int i, r;

	/* check if we have the correct card OS unless SC_PKCS15EMU_FLAGS_NO_CHECK */
	i=(opts && (opts->flags & SC_PKCS15EMU_FLAGS_NO_CHECK));
	if ( !i && 
		card->type!=SC_CARD_TYPE_STARCOS_V3_4 &&
		card->type!=SC_CARD_TYPE_STARCOS_V3_5
	   ) return SC_ERROR_WRONG_CARD;

	return detect_qes_c1(p15card);
}
