// SPDX-License-Identifier: BSD-3-Clause OR Apache-2.0
#include "aap/aap.h"
#include "aap/aap_serializer.h"

#include "cla/bibe_proto.h"

#include "platform/hal_io.h"

#include "bundle7/create.h"
#include "bundle7/reports.h"

#include "ud3tn/common.h"
#include "ud3tn/config.h"
#include "ud3tn/parser.h"
#include "cbor.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>


size_t bibe_parser_parse(const uint8_t *const buffer,
			 const size_t length,
			 struct bibe_protocol_data_unit *const bpdu)
{
	CborParser parser;
	CborError err;
	CborValue it;
	CborValue report;
	size_t array_length;
	uint64_t transmission_id;
	uint64_t retransmission_time;

	if (bpdu == NULL)
		return CborErrorInternalError;

	err = cbor_parser_init(buffer, length, 0, &parser, &it);

	if (err)
		return err;

	if (!cbor_value_is_array(&it) || !cbor_value_is_length_known(&it))
		return CborErrorIllegalType;

	if (cbor_value_get_array_length(&it, &array_length) != CborNoError)
		return CborErrorUnknownLength;

	// 3 items (transmission id, retransmission time, encapsulated bundle)
	if (array_length < 3)
		return CborErrorTooFewItems;
	else if (array_length > 3)
		return CborErrorTooManyItems;

	if (cbor_value_enter_container(&it, &report))
		return CborErrorInternalError;

	// Transmission ID
	// ---------------
	if (!cbor_value_is_unsigned_integer(&report))
		return CborErrorIllegalType;
	cbor_value_get_uint64(&report, &transmission_id);
	bpdu->transmission_id = transmission_id;
	if (cbor_value_advance_fixed(&report))
		return CborErrorUnexpectedEOF;

	// Retransmission time
	// -------------------
	if (!cbor_value_is_unsigned_integer(&report))
		return CborErrorIllegalType;
	cbor_value_get_uint64(&report, &retransmission_time);
	bpdu->retransmission_time = retransmission_time;
	if (cbor_value_advance_fixed(&report))
		return CborErrorUnexpectedEOF;

	// Encapsulated bundle
	// -------------------
	if (!cbor_value_is_byte_string(&report))
		return CborErrorIllegalType;

	size_t bundle_str_len;
	enum CborError retval;

	cbor_value_get_string_length(&report, &bundle_str_len);
	// Allocate memory for the encapsulated bundle
	bpdu->encapsulated_bundle = malloc(bundle_str_len);
	bpdu->payload_length = bundle_str_len;
	// From the cbor docs:
	//   "The next pointer, if not null, will be updated to point to the next item after
	//    this string. If value points to the last item, then next will be invalid."
	// Since we don't have a next element, we need to pass a null pointer to the function here.
	cbor_value_copy_byte_string(
		&report,
		bpdu->encapsulated_bundle,
		&bundle_str_len,
		NULL
	);
	if (cbor_value_advance(&report)) {
		retval = CborErrorUnexpectedEOF;
		goto fail;
	}

	// Leave BPDU container
	if (!cbor_value_at_end(&report) ||
	    cbor_value_leave_container(&it, &report)) {
		retval = CborErrorInternalError;
		goto fail;
	}

	return 0;

fail:
	free(bpdu->encapsulated_bundle);
	return retval;
}

static void write_to_buffer(
	uint8_t *buffer, const void *data, size_t position, const size_t length)
{
	memcpy(&buffer[position], data, length);
}

struct bibe_header bibe_encode_header(const char *const dest_eid,
				      const size_t payload_len)
{
	struct bibe_header hdr;
	const size_t eid_len = strlen(dest_eid);

	/* Encoding length of the bundle byte string */
	uint8_t *temp_buffer = malloc(sizeof(uint64_t));
	CborEncoder encoder;

	cbor_encoder_init(&encoder, temp_buffer, sizeof(uint64_t), 0);
	cbor_encode_uint(&encoder, (uint64_t)payload_len);
	const size_t bpdu_size = cbor_encoder_get_buffer_size(
		&encoder,
		temp_buffer
	) + 3; // buffer size + 3 for the BPDU data
	temp_buffer[0] |= 0x40; // see bundle7 serializer.c lines 235-239

	/* Encoding the BPDU */
	char *bibe_bytes = malloc(bpdu_size);

	bibe_bytes[0] = 0x83; // 83 (100|00011) -> Array of length 3
	bibe_bytes[1] = 0x00; // 00             -> Integer 0 (transm. ID)
	bibe_bytes[2] = 0x00; // 00             -> Integer 0 (retr. time)
	// bibe_bytes[3 to x] contains the length of the bundle byte string
	// the encapsulated bundle itself will be sent via cla_bibe.c's send_packet_data

	for (size_t i = 3; i < bpdu_size; i++)
		bibe_bytes[i] = temp_buffer[i - 3];

	free(temp_buffer);

	/* Building and encoding the AAP message */
	struct aap_message msg;

	msg.type = AAP_MESSAGE_SENDBIBE;
	msg.eid_length = eid_len;
	// Discard const, the used AAP functions do not modify the EID.
	msg.eid = (char *)dest_eid;
	msg.payload_length = payload_len + bpdu_size;
	msg.payload = NULL;

	hdr.hdr_len = aap_get_serialized_size(&msg) - payload_len;
	hdr.data = malloc(hdr.hdr_len);

	ASSERT(hdr.data);
	ASSERT(hdr.hdr_len != 0);

	aap_serialize_into(hdr.data, &msg, false);
	/* Appending the BPDU to the AAP message */
	write_to_buffer(
		hdr.data,
		bibe_bytes,
		hdr.hdr_len - bpdu_size,
		bpdu_size
	);

	free(bibe_bytes);

	return hdr;
}
