/**
 * @file test_cfl_validation.c
 * @brief Unit tests for CFL validation functions
 */

#include "unity.h"
#include "cfl/cfl.h"
#include <string.h>

void setUp(void)
{
}

void tearDown(void)
{
}

/* ========== Header Validation Tests ========== */

void test_cfl_header_validate_null_pointer(void)
{
    cfl_error_t err = cfl_header_validate(NULL);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_NULL, err);
}

void test_cfl_header_validate_valid_header(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x1234, CFL_F_RQST);

    cfl_error_t err = cfl_header_validate(&hdr);
    TEST_ASSERT_EQUAL_INT(CFL_OK, err);
}

void test_cfl_header_validate_invalid_sync(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x1234, CFL_F_RQST);
    hdr.sync = 0xBAD;

    cfl_error_t err = cfl_header_validate(&hdr);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_SYNC, err);
}

void test_cfl_header_validate_invalid_version(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x1234, CFL_F_RQST);
    hdr.version = 99;

    cfl_error_t err = cfl_header_validate(&hdr);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_VERSION, err);
}

void test_cfl_header_validate_length_too_large(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x1234, CFL_F_RQST);
    hdr.length = CFL_MAX_PAYLOAD_SIZE + 1;

    cfl_error_t err = cfl_header_validate(&hdr);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_LENGTH, err);
}

void test_cfl_header_validate_max_length(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x1234, CFL_F_RQST);
    hdr.length = CFL_MAX_PAYLOAD_SIZE;

    cfl_error_t err = cfl_header_validate(&hdr);
    TEST_ASSERT_EQUAL_INT(CFL_OK, err);
}

void test_cfl_header_validate_zero_length(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x1234, CFL_F_RQST);
    hdr.length = 0;

    cfl_error_t err = cfl_header_validate(&hdr);
    TEST_ASSERT_EQUAL_INT(CFL_OK, err);
}

/* ========== Message Validation Tests ========== */

void test_cfl_message_validate_null_header(void)
{
    uint8_t payload[] = {0x01, 0x02};
    cfl_error_t err = cfl_message_validate(NULL, payload, sizeof(payload));
    TEST_ASSERT_EQUAL_INT(CFL_ERR_NULL, err);
}

void test_cfl_message_validate_header_only_valid(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x1234, CFL_F_RQST);
    hdr.length = 0;
    cfl_header_compute_crc(&hdr, NULL, 0);

    cfl_error_t err = cfl_message_validate(&hdr, NULL, 0);
    TEST_ASSERT_EQUAL_INT(CFL_OK, err);
}

void test_cfl_message_validate_with_payload_valid(void)
{
    cfl_header_t hdr;
    uint8_t payload[] = {0xAA, 0xBB, 0xCC, 0xDD};

    cfl_header_init(&hdr, 0x5678, CFL_F_PUSH);
    hdr.length = sizeof(payload);
    cfl_header_compute_crc(&hdr, payload, sizeof(payload));

    cfl_error_t err = cfl_message_validate(&hdr, payload, sizeof(payload));
    TEST_ASSERT_EQUAL_INT(CFL_OK, err);
}

void test_cfl_message_validate_length_mismatch(void)
{
    cfl_header_t hdr;
    uint8_t payload[] = {0x01, 0x02, 0x03};

    cfl_header_init(&hdr, 0x1234, CFL_F_RQST);
    hdr.length = 10; /* Mismatch */
    cfl_header_compute_crc(&hdr, payload, sizeof(payload));

    cfl_error_t err = cfl_message_validate(&hdr, payload, sizeof(payload));
    TEST_ASSERT_EQUAL_INT(CFL_ERR_LENGTH, err);
}

void test_cfl_message_validate_null_payload_with_nonzero_length(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x1234, CFL_F_RQST);
    hdr.length = 10;

    cfl_error_t err = cfl_message_validate(&hdr, NULL, 10);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_NULL, err);
}

void test_cfl_message_validate_crc_mismatch(void)
{
    cfl_header_t hdr;
    uint8_t payload[] = {0x01, 0x02, 0x03};

    cfl_header_init(&hdr, 0x1234, CFL_F_RQST);
    hdr.length = sizeof(payload);
    cfl_header_compute_crc(&hdr, payload, sizeof(payload));

    /* Corrupt CRC */
    hdr.crc ^= 0xFFFF;

    cfl_error_t err = cfl_message_validate(&hdr, payload, sizeof(payload));
    TEST_ASSERT_EQUAL_INT(CFL_ERR_CRC, err);
}

void test_cfl_message_validate_corrupted_payload(void)
{
    cfl_header_t hdr;
    uint8_t payload[] = {0x01, 0x02, 0x03, 0x04};

    cfl_header_init(&hdr, 0x1234, CFL_F_RQST);
    hdr.length = sizeof(payload);
    cfl_header_compute_crc(&hdr, payload, sizeof(payload));

    /* Corrupt payload */
    payload[2] ^= 0xFF;

    cfl_error_t err = cfl_message_validate(&hdr, payload, sizeof(payload));
    TEST_ASSERT_EQUAL_INT(CFL_ERR_CRC, err);
}

void test_cfl_message_validate_corrupted_header_field(void)
{
    cfl_header_t hdr;
    uint8_t payload[] = {0x01, 0x02};

    cfl_header_init(&hdr, 0x1234, CFL_F_RQST);
    hdr.seq = 100;
    hdr.length = sizeof(payload);
    cfl_header_compute_crc(&hdr, payload, sizeof(payload));

    /* Corrupt header field (not CRC) */
    hdr.seq = 999;

    cfl_error_t err = cfl_message_validate(&hdr, payload, sizeof(payload));
    TEST_ASSERT_EQUAL_INT(CFL_ERR_CRC, err);
}

void test_cfl_message_validate_all_flag_combinations(void)
{
    uint8_t flags[] = {
        CFL_F_RQST,
        CFL_F_RPLY,
        CFL_F_PUSH,
        CFL_F_ACK,
        CFL_F_NACK,
        CFL_F_ENCRYPTED,
        CFL_F_COMPRESSED,
        CFL_F_RESERVED,
        CFL_F_RQST | CFL_F_ENCRYPTED,
        CFL_F_RPLY | CFL_F_COMPRESSED,
    };

    for (size_t i = 0; i < sizeof(flags); i++) {
        cfl_header_t hdr;
        cfl_header_init(&hdr, 0x0001, flags[i]);
        hdr.length = 0;
        cfl_header_compute_crc(&hdr, NULL, 0);

        cfl_error_t err = cfl_message_validate(&hdr, NULL, 0);
        TEST_ASSERT_EQUAL_INT(CFL_OK, err);
    }
}

void test_cfl_message_validate_max_payload(void)
{
    cfl_header_t hdr;
    uint8_t payload[CFL_MAX_PAYLOAD_SIZE];
    memset(payload, 0x42, sizeof(payload));

    cfl_header_init(&hdr, 0xFFFF, CFL_F_PUSH);
    hdr.length = sizeof(payload);
    cfl_header_compute_crc(&hdr, payload, sizeof(payload));

    cfl_error_t err = cfl_message_validate(&hdr, payload, sizeof(payload));
    TEST_ASSERT_EQUAL_INT(CFL_OK, err);
}

void test_cfl_message_validate_detects_single_bit_flip_in_payload(void)
{
    cfl_header_t hdr;
    uint8_t payload[] = {0x00, 0x00, 0x00, 0x00};

    cfl_header_init(&hdr, 0x1234, CFL_F_RQST);
    hdr.length = sizeof(payload);
    cfl_header_compute_crc(&hdr, payload, sizeof(payload));

    /* Flip single bit */
    payload[2] ^= 0x01;

    cfl_error_t err = cfl_message_validate(&hdr, payload, sizeof(payload));
    TEST_ASSERT_EQUAL_INT(CFL_ERR_CRC, err);
}

/* ========== Error String Tests ========== */

void test_cfl_error_str_all_codes(void)
{
    TEST_ASSERT_EQUAL_STRING("OK", cfl_error_str(CFL_OK));
    TEST_ASSERT_EQUAL_STRING("Null pointer", cfl_error_str(CFL_ERR_NULL));
    TEST_ASSERT_EQUAL_STRING("Invalid sync word", cfl_error_str(CFL_ERR_SYNC));
    TEST_ASSERT_EQUAL_STRING("Version mismatch", cfl_error_str(CFL_ERR_VERSION));
    TEST_ASSERT_EQUAL_STRING("CRC mismatch", cfl_error_str(CFL_ERR_CRC));
    TEST_ASSERT_EQUAL_STRING("Invalid length", cfl_error_str(CFL_ERR_LENGTH));
    TEST_ASSERT_EQUAL_STRING("Buffer too small", cfl_error_str(CFL_ERR_BUFFER));
    TEST_ASSERT_EQUAL_STRING("Invalid flags", cfl_error_str(CFL_ERR_FLAGS));
}

void test_cfl_error_str_unknown(void)
{
    const char *str = cfl_error_str((cfl_error_t)999);
    TEST_ASSERT_EQUAL_STRING("Unknown error", str);
}

/* Main test runner */
int main(void)
{
    UNITY_BEGIN();

    /* Header validation */
    RUN_TEST(test_cfl_header_validate_null_pointer);
    RUN_TEST(test_cfl_header_validate_valid_header);
    RUN_TEST(test_cfl_header_validate_invalid_sync);
    RUN_TEST(test_cfl_header_validate_invalid_version);
    RUN_TEST(test_cfl_header_validate_length_too_large);
    RUN_TEST(test_cfl_header_validate_max_length);
    RUN_TEST(test_cfl_header_validate_zero_length);

    /* Message validation */
    RUN_TEST(test_cfl_message_validate_null_header);
    RUN_TEST(test_cfl_message_validate_header_only_valid);
    RUN_TEST(test_cfl_message_validate_with_payload_valid);
    RUN_TEST(test_cfl_message_validate_length_mismatch);
    RUN_TEST(test_cfl_message_validate_null_payload_with_nonzero_length);
    RUN_TEST(test_cfl_message_validate_crc_mismatch);
    RUN_TEST(test_cfl_message_validate_corrupted_payload);
    RUN_TEST(test_cfl_message_validate_corrupted_header_field);
    RUN_TEST(test_cfl_message_validate_all_flag_combinations);
    RUN_TEST(test_cfl_message_validate_max_payload);
    RUN_TEST(test_cfl_message_validate_detects_single_bit_flip_in_payload);

    /* Error strings */
    RUN_TEST(test_cfl_error_str_all_codes);
    RUN_TEST(test_cfl_error_str_unknown);

    return UNITY_END();
}
