// SPDX-License-Identifier: BSD-3-Clause OR Apache-2.0
#include <inttypes.h>
#include <stddef.h>

// -------------------
// Simple BPbis bundle
// -------------------
//
// A CBOR-encoded BPbis bundle with all extension blocks specified in BPbis.
const uint8_t cbor_simple_bundle[] = {
	0x9f, // CBOR indefinite array

	// Primary block
	//
	// [
	//   7,                # Protocol version
	//   131076,           # Bundle Processing Flags:
	//                     #   - REPORT_DELIVERY
	//                     #   - MUST_NOT_BE_FRAGMENTED
	//   0,                # CRC Type (None)
	//   [1, "GS2"],       # Destination EID "dtn:GS2"
	//   [2, [243, 350]],  # Source EID "ipn:243.350"
	//   [1, 0],           # Report-to EID "dtn:none"
	//   [658489863000, 0],# DTN Time "2020-11-12T09:51:03", sequence number
	//   86400             # Lifetime
	// ]
	0x88, 0x07, 0x1a, 0x00, 0x02, 0x00, 0x04, 0x00, 0x82, 0x01, 0x63, 0x47,
	0x53, 0x32, 0x82, 0x02, 0x82, 0x18, 0xf3, 0x19, 0x01, 0x5e, 0x82, 0x01,
	0x00, 0x82, 0x1b, 0x00, 0x00, 0x00, 0x99, 0x51, 0x0d, 0xeb, 0x58, 0x00,
	0x1a, 0x00, 0x01, 0x51, 0x80,

	// Previous Node Block
	//
	// [7, 2, 0, 0, cbor_serialize([1, "GS4"])]
	0x85, 0x06, 0x02, 0x00, 0x00, 0x46,
	0x82, 0x01, 0x63, 0x47, 0x53, 0x34,

	// Hop Count Block
	//
	// [9, 3, 0, 0, cbor_serialize([30, 0])]
	0x85, 0x0a, 0x03, 0x00, 0x00, 0x44,
	0x82, 0x18, 0x1e, 0x00,

	// Bundle Age Block
	//
	// [8, 4, 0, 0, b"\0"]
	0x85, 0x07, 0x04, 0x00, 0x00, 0x41,
	0x00,

	// Payload Block
	//
	// [1, 1, 0, 0, b'Hello world!']
	0x85, 0x01, 0x01, 0x00, 0x00, 0x4c,
	0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x21,

	0xff, // CBOR "STOP"
};
const size_t len_simple_bundle = sizeof(cbor_simple_bundle);

// ------------
// CRC 16 AX.25
// ------------
//
// Bundle with CRC-16 checksum for the primary block
//
//   - no bundle processing flags
//   - no extension blocks
//   - payload block with empty byte string and no block processing flags
//
// CBOR structure:
//   [
//     [7, 0, 1, [1, "GS2"], [1, 0], [1, 0], [0, 0], 86400, b"\x71\x23"],
//     [1, 0, 0, 0, 1, b""]
//   ]
const uint8_t cbor_crc16_primary_block[] = {
	0x9f, 0x89, 0x07, 0x00, 0x01, 0x82, 0x01, 0x63, 0x47, 0x53, 0x32, 0x82,
	0x01, 0x00, 0x82, 0x01, 0x00, 0x82, 0x00, 0x00, 0x1a, 0x00, 0x01, 0x51,
	0x80, 0x42, 0x71, 0x23, 0x85, 0x01, 0x00, 0x00, 0x00, 0x40, 0xff,
};
const size_t len_crc16_primary_block = sizeof(cbor_crc16_primary_block);

// Bundle with CRC-16 checksum for the payload block
//
//   - no bundle processing flags
//   - no extension blocks
//   - payload block with "Hello world" byte string and CRC 16 checksum and no
//     block processing flags
//
// CBOR structure:
//   [
//     [7, 0, 0, [1, "GS2"], [1, 0], [1, 0], [0, 0], 86400],
//     [1, 0, 0, 1, b"Hello world!", b"\x60\xd7"]
//   ]
const uint8_t cbor_crc16_payload_block[] = {
	0x9f, 0x88, 0x07, 0x00, 0x00, 0x82, 0x01, 0x63, 0x47, 0x53, 0x32, 0x82,
	0x01, 0x00, 0x82, 0x01, 0x00, 0x82, 0x00, 0x00, 0x1a, 0x00, 0x01, 0x51,
	0x80, 0x86, 0x01, 0x00, 0x00, 0x01, 0x4c, 0x48, 0x65, 0x6c, 0x6c, 0x6f,
	0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x21, 0x42, 0x60, 0xd7, 0xff,
};
const size_t len_crc16_payload_block = sizeof(cbor_crc16_payload_block);


// ------------------------------
// CRC 32 Castagnoli Verification
// ------------------------------
//
// Bundle with CRC-32-C checksum for the primary block
//
//   - no bundle processing flags
//   - no extension blocks
//   - payload block with empty byte string and no block processing flags
//
// CBOR structure:
//   [
//     [
//       7, 0, 2, [1, "GS2"], [1, 0], [1, 0], [0, 0], 86400,
//       b"\x95\x42\xde\xfd"
//     ],
//     [1, 0, 0, 0, 1, b""]
//   ]
const uint8_t cbor_crc32_primary_block[] = {
	0x9f, 0x89, 0x07, 0x00, 0x02, 0x82, 0x01, 0x63, 0x47, 0x53, 0x32, 0x82,
	0x01, 0x00, 0x82, 0x01, 0x00, 0x82, 0x00, 0x00, 0x1a, 0x00, 0x01, 0x51,
	0x80, 0x44, 0x95, 0x42, 0xde, 0xfd, 0x85, 0x01, 0x00, 0x00, 0x00, 0x40,
	0xff,
};

const size_t len_crc32_primary_block = sizeof(cbor_crc32_primary_block);

// Bundle with CRC-32 checksum for the payload block
//
//   - no bundle processing flags
//   - no extension blocks
//   - payload block with "Hello world" byte string and CRC 32 checksum and no
//     block processing flags
//
// CBOR structure:
//   [
//     [7, 0, 0, [1, "GS2"], [1, 0], [1, 0], [0, 0], 86400],
//     [1, 0, 0, 2, b"Hello world!", b"\xc3\xae\xc5\x52"]
//   ]
const uint8_t cbor_crc32_payload_block[] = {
	0x9f, 0x88, 0x07, 0x00, 0x00, 0x82, 0x01, 0x63, 0x47, 0x53, 0x32, 0x82,
	0x01, 0x00, 0x82, 0x01, 0x00, 0x82, 0x00, 0x00, 0x1a, 0x00, 0x01, 0x51,
	0x80, 0x86, 0x01, 0x00, 0x00, 0x02, 0x4c, 0x48, 0x65, 0x6c, 0x6c, 0x6f,
	0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x21, 0x44, 0xc3, 0xae, 0xc5, 0x52,
	0xff,
};

const size_t len_crc32_payload_block = sizeof(cbor_crc32_payload_block);

// Bundle with invalid CRC-16 X.25 checksum for the primary block
//
//   - no bundle processing flags
//   - no extension blocks
//   - payload block with empty byte string and no block processing flags
//
// CBOR structure:
//   [
//     [7, 0, 1, [1, "GS2"], [1, 0], [1, 0], [0, 0], 86400, b"\x00\x00"],
//     [1, 0, 0, 0, b""]
//   ]
const uint8_t cbor_invalid_crc16[] = {
	0x9f, 0x89, 0x07, 0x00, 0x01, 0x82, 0x01, 0x63, 0x47, 0x53, 0x32, 0x82,
	0x01, 0x00, 0x82, 0x01, 0x00, 0x82, 0x00, 0x00, 0x1a, 0x00, 0x01, 0x51,
	0x80, 0x42, 0x00, 0x00, 0x85, 0x01, 0x00, 0x00, 0x00, 0x40, 0xff,
};
const size_t len_invalid_crc16 = sizeof(cbor_invalid_crc16);
