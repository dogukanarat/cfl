/**
 * @file test_cfl_serialization.c
 * @brief Unit tests for CFL serialization/deserialization
 */

#include "unity.h"
#include "cfl/cfl.h"
#include <string.h>

static uint8_t test_buffer[CFL_HEADER_SIZE + CFL_MAX_PAYLOAD_SIZE];

void setUp(void)
{
    memset(test_buffer, 0, sizeof(test_buffer));
}

void tearDown(void)
{
}

/* ========== Serialization Tests ========== */

void test_cfl_serialize_null_header(void)
{
    uint8_t payload[] = {0x01, 0x02};
    uint16_t len = cfl_serialize(NULL, payload, sizeof(payload),
                                  test_buffer, sizeof(test_buffer));
    TEST_ASSERT_EQUAL_UINT16(0, len);
}

void test_cfl_serialize_null_buffer(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x0001, CFL_F_RQST);

    uint16_t len = cfl_serialize(&hdr, NULL, 0, NULL, 100);
    TEST_ASSERT_EQUAL_UINT16(0, len);
}

void test_cfl_serialize_buffer_too_small(void)
{
    cfl_header_t hdr;
    uint8_t payload[] = {0x01, 0x02, 0x03};

    cfl_header_init(&hdr, 0x0001, CFL_F_RQST);

    /* Buffer smaller than required */
    uint8_t small_buffer[5];
    uint16_t len = cfl_serialize(&hdr, payload, sizeof(payload),
                                  small_buffer, sizeof(small_buffer));
    TEST_ASSERT_EQUAL_UINT16(0, len);
}

void test_cfl_serialize_header_only(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x1234, CFL_F_RQST);
    hdr.seq = 100;
    cfl_header_compute_crc(&hdr, NULL, 0);

    uint16_t len = cfl_serialize(&hdr, NULL, 0,
                                  test_buffer, sizeof(test_buffer));

    TEST_ASSERT_EQUAL_UINT16(CFL_HEADER_SIZE, len);

    /* Verify header was copied correctly */
    cfl_header_t *serialized = (cfl_header_t *)test_buffer;
    TEST_ASSERT_EQUAL_HEX16(hdr.sync, serialized->sync);
    TEST_ASSERT_EQUAL_UINT8(hdr.version, serialized->version);
    TEST_ASSERT_EQUAL_HEX8(hdr.flags, serialized->flags);
    TEST_ASSERT_EQUAL_HEX16(hdr.id, serialized->id);
    TEST_ASSERT_EQUAL_UINT16(hdr.seq, serialized->seq);
    TEST_ASSERT_EQUAL_UINT16(hdr.length, serialized->length);
    TEST_ASSERT_EQUAL_HEX16(hdr.crc, serialized->crc);
}

void test_cfl_serialize_with_payload(void)
{
    cfl_header_t hdr;
    uint8_t payload[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};

    cfl_header_init(&hdr, 0x5678, CFL_F_PUSH);
    hdr.length = sizeof(payload);
    cfl_header_compute_crc(&hdr, payload, sizeof(payload));

    uint16_t len = cfl_serialize(&hdr, payload, sizeof(payload),
                                  test_buffer, sizeof(test_buffer));

    TEST_ASSERT_EQUAL_UINT16(CFL_HEADER_SIZE + sizeof(payload), len);

    /* Verify header */
    cfl_header_t *serialized_hdr = (cfl_header_t *)test_buffer;
    TEST_ASSERT_EQUAL_HEX16(hdr.sync, serialized_hdr->sync);

    /* Verify payload */
    uint8_t *serialized_payload = test_buffer + CFL_HEADER_SIZE;
    TEST_ASSERT_EQUAL_UINT8_ARRAY(payload, serialized_payload, sizeof(payload));
}

void test_cfl_serialize_max_payload(void)
{
    cfl_header_t hdr;
    uint8_t payload[CFL_MAX_PAYLOAD_SIZE];
    memset(payload, 0x55, sizeof(payload));

    cfl_header_init(&hdr, 0x0001, CFL_F_RQST);
    hdr.length = sizeof(payload);
    cfl_header_compute_crc(&hdr, payload, sizeof(payload));

    uint16_t len = cfl_serialize(&hdr, payload, sizeof(payload),
                                  test_buffer, sizeof(test_buffer));

    TEST_ASSERT_EQUAL_UINT16(CFL_HEADER_SIZE + CFL_MAX_PAYLOAD_SIZE, len);
}

/* ========== Deserialization Tests ========== */

void test_cfl_deserialize_null_buffer(void)
{
    cfl_header_t hdr;
    const uint8_t *payload;
    uint16_t payload_len;

    cfl_error_t err = cfl_deserialize(NULL, 100, &hdr, &payload, &payload_len);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_NULL, err);
}

void test_cfl_deserialize_null_header(void)
{
    const uint8_t *payload;
    uint16_t payload_len;

    cfl_error_t err = cfl_deserialize(test_buffer, sizeof(test_buffer),
                                      NULL, &payload, &payload_len);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_NULL, err);
}

void test_cfl_deserialize_buffer_too_small(void)
{
    cfl_header_t hdr;
    const uint8_t *payload;
    uint16_t payload_len;

    /* Buffer smaller than header */
    uint8_t small_buffer[5];
    cfl_error_t err = cfl_deserialize(small_buffer, sizeof(small_buffer),
                                      &hdr, &payload, &payload_len);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_BUFFER, err);
}

void test_cfl_deserialize_header_only(void)
{
    /* First serialize a header */
    cfl_header_t hdr_out;
    cfl_header_init(&hdr_out, 0xABCD, CFL_F_RPLY);
    hdr_out.seq = 999;
    cfl_header_compute_crc(&hdr_out, NULL, 0);

    uint16_t ser_len = cfl_serialize(&hdr_out, NULL, 0,
                                      test_buffer, sizeof(test_buffer));
    TEST_ASSERT_EQUAL_UINT16(CFL_HEADER_SIZE, ser_len);

    /* Now deserialize */
    cfl_header_t hdr_in;
    const uint8_t *payload;
    uint16_t payload_len;

    cfl_error_t err = cfl_deserialize(test_buffer, ser_len,
                                      &hdr_in, &payload, &payload_len);
    TEST_ASSERT_EQUAL_INT(CFL_OK, err);
    TEST_ASSERT_NULL(payload);
    TEST_ASSERT_EQUAL_UINT16(0, payload_len);

    /* Verify header fields */
    TEST_ASSERT_EQUAL_HEX16(hdr_out.sync, hdr_in.sync);
    TEST_ASSERT_EQUAL_UINT8(hdr_out.version, hdr_in.version);
    TEST_ASSERT_EQUAL_HEX8(hdr_out.flags, hdr_in.flags);
    TEST_ASSERT_EQUAL_HEX16(hdr_out.id, hdr_in.id);
    TEST_ASSERT_EQUAL_UINT16(hdr_out.seq, hdr_in.seq);
    TEST_ASSERT_EQUAL_UINT16(hdr_out.length, hdr_in.length);
    TEST_ASSERT_EQUAL_HEX16(hdr_out.crc, hdr_in.crc);
}

void test_cfl_deserialize_with_payload(void)
{
    /* Serialize */
    cfl_header_t hdr_out;
    uint8_t payload_out[] = {0x11, 0x22, 0x33, 0x44, 0x55};

    cfl_header_init(&hdr_out, 0x1111, CFL_F_RQST);
    hdr_out.length = sizeof(payload_out);
    cfl_header_compute_crc(&hdr_out, payload_out, sizeof(payload_out));

    uint16_t ser_len = cfl_serialize(&hdr_out, payload_out, sizeof(payload_out),
                                      test_buffer, sizeof(test_buffer));

    /* Deserialize */
    cfl_header_t hdr_in;
    const uint8_t *payload_in;
    uint16_t payload_len;

    cfl_error_t err = cfl_deserialize(test_buffer, ser_len,
                                      &hdr_in, &payload_in, &payload_len);
    TEST_ASSERT_EQUAL_INT(CFL_OK, err);
    TEST_ASSERT_NOT_NULL(payload_in);
    TEST_ASSERT_EQUAL_UINT16(sizeof(payload_out), payload_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(payload_out, payload_in, sizeof(payload_out));
}

void test_cfl_deserialize_invalid_sync(void)
{
    cfl_header_t hdr_out;
    cfl_header_init(&hdr_out, 0x0001, CFL_F_RQST);
    hdr_out.sync = 0xBAD; /* Invalid sync word */

    cfl_serialize(&hdr_out, NULL, 0, test_buffer, sizeof(test_buffer));

    cfl_header_t hdr_in;
    const uint8_t *payload;
    uint16_t payload_len;

    cfl_error_t err = cfl_deserialize(test_buffer, CFL_HEADER_SIZE,
                                      &hdr_in, &payload, &payload_len);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_SYNC, err);
}

void test_cfl_deserialize_invalid_version(void)
{
    cfl_header_t hdr_out;
    cfl_header_init(&hdr_out, 0x0001, CFL_F_RQST);
    hdr_out.version = 99; /* Invalid version */

    cfl_serialize(&hdr_out, NULL, 0, test_buffer, sizeof(test_buffer));

    cfl_header_t hdr_in;
    const uint8_t *payload;
    uint16_t payload_len;

    cfl_error_t err = cfl_deserialize(test_buffer, CFL_HEADER_SIZE,
                                      &hdr_in, &payload, &payload_len);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_VERSION, err);
}

void test_cfl_deserialize_payload_too_large(void)
{
    cfl_header_t hdr_out;
    cfl_header_init(&hdr_out, 0x0001, CFL_F_RQST);
    hdr_out.length = CFL_MAX_PAYLOAD_SIZE + 1; /* Too large */

    cfl_serialize(&hdr_out, NULL, 0, test_buffer, sizeof(test_buffer));

    cfl_header_t hdr_in;
    const uint8_t *payload;
    uint16_t payload_len;

    cfl_error_t err = cfl_deserialize(test_buffer, CFL_HEADER_SIZE,
                                      &hdr_in, &payload, &payload_len);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_LENGTH, err);
}

void test_cfl_deserialize_buffer_missing_payload(void)
{
    /* Serialize message with payload */
    cfl_header_t hdr_out;
    uint8_t payload[] = {0x01, 0x02, 0x03};

    cfl_header_init(&hdr_out, 0x0001, CFL_F_RQST);
    hdr_out.length = sizeof(payload);

    cfl_serialize(&hdr_out, payload, sizeof(payload),
                  test_buffer, sizeof(test_buffer));

    /* Try to deserialize with buffer that's too small for payload */
    cfl_header_t hdr_in;
    const uint8_t *payload_in;
    uint16_t payload_len;

    cfl_error_t err = cfl_deserialize(test_buffer, CFL_HEADER_SIZE,
                                      &hdr_in, &payload_in, &payload_len);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_BUFFER, err);
}

/* ========== Round-trip Tests ========== */

void test_cfl_serialize_deserialize_roundtrip(void)
{
    cfl_header_t hdr_out, hdr_in;
    uint8_t payload_out[] = {0xDE, 0xAD, 0xBE, 0xEF};
    const uint8_t *payload_in;
    uint16_t payload_len;

    /* Serialize */
    cfl_header_init(&hdr_out, 0x9999, CFL_F_PUSH);
    hdr_out.seq = 42;
    hdr_out.length = sizeof(payload_out);
    cfl_header_compute_crc(&hdr_out, payload_out, sizeof(payload_out));

    uint16_t ser_len = cfl_serialize(&hdr_out, payload_out, sizeof(payload_out),
                                      test_buffer, sizeof(test_buffer));

    /* Deserialize */
    cfl_error_t err = cfl_deserialize(test_buffer, ser_len,
                                      &hdr_in, &payload_in, &payload_len);

    /* Verify */
    TEST_ASSERT_EQUAL_INT(CFL_OK, err);
    TEST_ASSERT_EQUAL_HEX16(hdr_out.sync, hdr_in.sync);
    TEST_ASSERT_EQUAL_UINT8(hdr_out.version, hdr_in.version);
    TEST_ASSERT_EQUAL_HEX8(hdr_out.flags, hdr_in.flags);
    TEST_ASSERT_EQUAL_HEX16(hdr_out.id, hdr_in.id);
    TEST_ASSERT_EQUAL_UINT16(hdr_out.seq, hdr_in.seq);
    TEST_ASSERT_EQUAL_UINT16(hdr_out.length, hdr_in.length);
    TEST_ASSERT_EQUAL_HEX16(hdr_out.crc, hdr_in.crc);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(payload_out, payload_in, sizeof(payload_out));
}

void test_cfl_serialize_deserialize_max_payload(void)
{
    cfl_header_t hdr_out, hdr_in;
    uint8_t payload_out[CFL_MAX_PAYLOAD_SIZE];
    const uint8_t *payload_in;
    uint16_t payload_len;

    /* Fill with pattern */
    for (uint16_t i = 0; i < sizeof(payload_out); i++) {
        payload_out[i] = (uint8_t)(i & 0xFF);
    }

    /* Serialize */
    cfl_header_init(&hdr_out, 0xFFFF, CFL_F_RQST);
    hdr_out.seq = 0xFFFF;
    hdr_out.length = sizeof(payload_out);
    cfl_header_compute_crc(&hdr_out, payload_out, sizeof(payload_out));

    uint16_t ser_len = cfl_serialize(&hdr_out, payload_out, sizeof(payload_out),
                                      test_buffer, sizeof(test_buffer));

    /* Deserialize */
    cfl_error_t err = cfl_deserialize(test_buffer, ser_len,
                                      &hdr_in, &payload_in, &payload_len);

    /* Verify */
    TEST_ASSERT_EQUAL_INT(CFL_OK, err);
    TEST_ASSERT_EQUAL_UINT16(CFL_MAX_PAYLOAD_SIZE, payload_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(payload_out, payload_in, CFL_MAX_PAYLOAD_SIZE);
}

/* Main test runner */
int main(void)
{
    UNITY_BEGIN();

    /* Serialization tests */
    RUN_TEST(test_cfl_serialize_null_header);
    RUN_TEST(test_cfl_serialize_null_buffer);
    RUN_TEST(test_cfl_serialize_buffer_too_small);
    RUN_TEST(test_cfl_serialize_header_only);
    RUN_TEST(test_cfl_serialize_with_payload);
    RUN_TEST(test_cfl_serialize_max_payload);

    /* Deserialization tests */
    RUN_TEST(test_cfl_deserialize_null_buffer);
    RUN_TEST(test_cfl_deserialize_null_header);
    RUN_TEST(test_cfl_deserialize_buffer_too_small);
    RUN_TEST(test_cfl_deserialize_header_only);
    RUN_TEST(test_cfl_deserialize_with_payload);
    RUN_TEST(test_cfl_deserialize_invalid_sync);
    RUN_TEST(test_cfl_deserialize_invalid_version);
    RUN_TEST(test_cfl_deserialize_payload_too_large);
    RUN_TEST(test_cfl_deserialize_buffer_missing_payload);

    /* Round-trip tests */
    RUN_TEST(test_cfl_serialize_deserialize_roundtrip);
    RUN_TEST(test_cfl_serialize_deserialize_max_payload);

    return UNITY_END();
}
