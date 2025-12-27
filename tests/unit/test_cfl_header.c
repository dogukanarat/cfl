/**
 * @file test_cfl_header.c
 * @brief Unit tests for CFL header manipulation functions
 */

#include "unity.h"
#include "cfl/cfl.h"
#include <string.h>

/* Test setup and teardown */
void setUp(void)
{
}

void tearDown(void)
{
}

/* ========== Header Init Tests ========== */

void test_cfl_header_init_null_pointer(void)
{
    /* Should not crash */
    cfl_header_init(NULL, 0x1234, CFL_F_RQST);
    TEST_PASS();
}

void test_cfl_header_init_basic(void)
{
    cfl_header_t hdr;
    memset(&hdr, 0xFF, sizeof(hdr)); /* Fill with garbage */

    cfl_header_init(&hdr, 0x1234, CFL_F_RQST);

    TEST_ASSERT_EQUAL_HEX16(CFL_SYNC_WORD, hdr.sync);
    TEST_ASSERT_EQUAL_UINT8(CFL_VERSION, hdr.version);
    TEST_ASSERT_EQUAL_HEX8(CFL_F_RQST, hdr.flags);
    TEST_ASSERT_EQUAL_HEX16(0x1234, hdr.id);
    TEST_ASSERT_EQUAL_UINT16(0, hdr.seq);
    TEST_ASSERT_EQUAL_UINT16(0, hdr.length);
    TEST_ASSERT_EQUAL_UINT16(0, hdr.crc);
}

void test_cfl_header_init_with_multiple_flags(void)
{
    cfl_header_t hdr;
    uint8_t flags = CFL_F_RQST | CFL_F_ACK;

    cfl_header_init(&hdr, 0x5678, flags);

    TEST_ASSERT_EQUAL_HEX8(flags, hdr.flags);
    TEST_ASSERT_EQUAL_HEX16(0x5678, hdr.id);
}

/* ========== Sequence Number Tests ========== */

void test_cfl_header_set_seq_null_pointer(void)
{
    cfl_header_set_seq(NULL, 0x1234);
    TEST_PASS();
}

void test_cfl_header_set_seq_basic(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x0001, CFL_F_RQST);

    cfl_header_set_seq(&hdr, 0xABCD);
    TEST_ASSERT_EQUAL_HEX16(0xABCD, hdr.seq);
}

void test_cfl_header_set_seq_max_value(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x0001, CFL_F_RQST);

    cfl_header_set_seq(&hdr, 0xFFFF);
    TEST_ASSERT_EQUAL_HEX16(0xFFFF, hdr.seq);
}

/* ========== Length Tests ========== */

void test_cfl_header_set_length_null_pointer(void)
{
    cfl_header_set_length(NULL, 100);
    TEST_PASS();
}

void test_cfl_header_set_length_basic(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x0001, CFL_F_RQST);

    cfl_header_set_length(&hdr, 256);
    TEST_ASSERT_EQUAL_UINT16(256, hdr.length);
}

void test_cfl_header_set_length_max_payload(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x0001, CFL_F_RQST);

    cfl_header_set_length(&hdr, CFL_MAX_PAYLOAD_SIZE);
    TEST_ASSERT_EQUAL_UINT16(CFL_MAX_PAYLOAD_SIZE, hdr.length);
}

/* ========== CRC Computation Tests ========== */

void test_cfl_header_compute_crc_null_header(void)
{
    uint8_t payload[] = {0x01, 0x02};
    cfl_header_compute_crc(NULL, payload, sizeof(payload));
    TEST_PASS();
}

void test_cfl_header_compute_crc_no_payload(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x0001, CFL_F_RQST);

    cfl_header_compute_crc(&hdr, NULL, 0);
    TEST_ASSERT_NOT_EQUAL(0, hdr.crc);
}

void test_cfl_header_compute_crc_with_payload(void)
{
    cfl_header_t hdr;
    uint8_t payload[] = {0xAA, 0xBB, 0xCC, 0xDD};

    cfl_header_init(&hdr, 0x0001, CFL_F_RQST);
    hdr.length = sizeof(payload);

    cfl_header_compute_crc(&hdr, payload, sizeof(payload));
    TEST_ASSERT_NOT_EQUAL(0, hdr.crc);
}

void test_cfl_header_compute_crc_deterministic(void)
{
    cfl_header_t hdr1, hdr2;
    uint8_t payload[] = {0x01, 0x02, 0x03};

    cfl_header_init(&hdr1, 0x1234, CFL_F_RQST);
    hdr1.seq = 100;
    hdr1.length = sizeof(payload);
    cfl_header_compute_crc(&hdr1, payload, sizeof(payload));

    cfl_header_init(&hdr2, 0x1234, CFL_F_RQST);
    hdr2.seq = 100;
    hdr2.length = sizeof(payload);
    cfl_header_compute_crc(&hdr2, payload, sizeof(payload));

    TEST_ASSERT_EQUAL_HEX16(hdr1.crc, hdr2.crc);
}

void test_cfl_header_compute_crc_different_for_different_payload(void)
{
    cfl_header_t hdr1, hdr2;
    uint8_t payload1[] = {0x01, 0x02, 0x03};
    uint8_t payload2[] = {0x01, 0x02, 0x04};

    cfl_header_init(&hdr1, 0x1234, CFL_F_RQST);
    hdr1.length = sizeof(payload1);
    cfl_header_compute_crc(&hdr1, payload1, sizeof(payload1));

    cfl_header_init(&hdr2, 0x1234, CFL_F_RQST);
    hdr2.length = sizeof(payload2);
    cfl_header_compute_crc(&hdr2, payload2, sizeof(payload2));

    TEST_ASSERT_NOT_EQUAL(hdr1.crc, hdr2.crc);
}

/* ========== Flag Manipulation Tests ========== */

void test_cfl_header_set_flags_null_pointer(void)
{
    cfl_header_set_flags(NULL, CFL_F_ACK);
    TEST_PASS();
}

void test_cfl_header_set_flags_basic(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x0001, CFL_F_RQST);

    cfl_header_set_flags(&hdr, CFL_F_ACK);
    TEST_ASSERT_EQUAL_HEX8(CFL_F_RQST | CFL_F_ACK, hdr.flags);
}

void test_cfl_header_set_flags_multiple(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x0001, 0);

    cfl_header_set_flags(&hdr, CFL_F_RQST);
    cfl_header_set_flags(&hdr, CFL_F_ENCRYPTED);
    TEST_ASSERT_EQUAL_HEX8(CFL_F_RQST | CFL_F_ENCRYPTED, hdr.flags);
}

void test_cfl_header_clear_flags_null_pointer(void)
{
    cfl_header_clear_flags(NULL, CFL_F_RQST);
    TEST_PASS();
}

void test_cfl_header_clear_flags_basic(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x0001, CFL_F_RQST | CFL_F_ACK);

    cfl_header_clear_flags(&hdr, CFL_F_RQST);
    TEST_ASSERT_EQUAL_HEX8(CFL_F_ACK, hdr.flags);
}

void test_cfl_header_clear_flags_multiple(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x0001, CFL_F_RQST | CFL_F_ACK | CFL_F_ENCRYPTED);

    cfl_header_clear_flags(&hdr, CFL_F_RQST | CFL_F_ACK);
    TEST_ASSERT_EQUAL_HEX8(CFL_F_ENCRYPTED, hdr.flags);
}

void test_cfl_header_has_flag_null_pointer(void)
{
    uint8_t result = cfl_header_has_flag(NULL, CFL_F_RQST);
    TEST_ASSERT_EQUAL_UINT8(0, result);
}

void test_cfl_header_has_flag_present(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x0001, CFL_F_RQST | CFL_F_ACK);

    TEST_ASSERT_TRUE(cfl_header_has_flag(&hdr, CFL_F_RQST));
    TEST_ASSERT_TRUE(cfl_header_has_flag(&hdr, CFL_F_ACK));
}

void test_cfl_header_has_flag_absent(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x0001, CFL_F_RQST);

    TEST_ASSERT_FALSE(cfl_header_has_flag(&hdr, CFL_F_ACK));
    TEST_ASSERT_FALSE(cfl_header_has_flag(&hdr, CFL_F_NACK));
}

void test_cfl_header_has_flag_all_flags(void)
{
    cfl_header_t hdr;
    cfl_header_init(&hdr, 0x0001, 0);

    /* Test each flag individually */
    cfl_header_set_flags(&hdr, CFL_F_RQST);
    TEST_ASSERT_TRUE(cfl_header_has_flag(&hdr, CFL_F_RQST));

    cfl_header_init(&hdr, 0x0001, CFL_F_RPLY);
    TEST_ASSERT_TRUE(cfl_header_has_flag(&hdr, CFL_F_RPLY));

    cfl_header_init(&hdr, 0x0001, CFL_F_PUSH);
    TEST_ASSERT_TRUE(cfl_header_has_flag(&hdr, CFL_F_PUSH));

    cfl_header_init(&hdr, 0x0001, CFL_F_ACK);
    TEST_ASSERT_TRUE(cfl_header_has_flag(&hdr, CFL_F_ACK));

    cfl_header_init(&hdr, 0x0001, CFL_F_NACK);
    TEST_ASSERT_TRUE(cfl_header_has_flag(&hdr, CFL_F_NACK));

    cfl_header_init(&hdr, 0x0001, CFL_F_ENCRYPTED);
    TEST_ASSERT_TRUE(cfl_header_has_flag(&hdr, CFL_F_ENCRYPTED));

    cfl_header_init(&hdr, 0x0001, CFL_F_COMPRESSED);
    TEST_ASSERT_TRUE(cfl_header_has_flag(&hdr, CFL_F_COMPRESSED));

    cfl_header_init(&hdr, 0x0001, CFL_F_RESERVED);
    TEST_ASSERT_TRUE(cfl_header_has_flag(&hdr, CFL_F_RESERVED));
}

/* Main test runner */
int main(void)
{
    UNITY_BEGIN();

    /* Header init tests */
    RUN_TEST(test_cfl_header_init_null_pointer);
    RUN_TEST(test_cfl_header_init_basic);
    RUN_TEST(test_cfl_header_init_with_multiple_flags);

    /* Sequence tests */
    RUN_TEST(test_cfl_header_set_seq_null_pointer);
    RUN_TEST(test_cfl_header_set_seq_basic);
    RUN_TEST(test_cfl_header_set_seq_max_value);

    /* Length tests */
    RUN_TEST(test_cfl_header_set_length_null_pointer);
    RUN_TEST(test_cfl_header_set_length_basic);
    RUN_TEST(test_cfl_header_set_length_max_payload);

    /* CRC tests */
    RUN_TEST(test_cfl_header_compute_crc_null_header);
    RUN_TEST(test_cfl_header_compute_crc_no_payload);
    RUN_TEST(test_cfl_header_compute_crc_with_payload);
    RUN_TEST(test_cfl_header_compute_crc_deterministic);
    RUN_TEST(test_cfl_header_compute_crc_different_for_different_payload);

    /* Flag manipulation tests */
    RUN_TEST(test_cfl_header_set_flags_null_pointer);
    RUN_TEST(test_cfl_header_set_flags_basic);
    RUN_TEST(test_cfl_header_set_flags_multiple);
    RUN_TEST(test_cfl_header_clear_flags_null_pointer);
    RUN_TEST(test_cfl_header_clear_flags_basic);
    RUN_TEST(test_cfl_header_clear_flags_multiple);
    RUN_TEST(test_cfl_header_has_flag_null_pointer);
    RUN_TEST(test_cfl_header_has_flag_present);
    RUN_TEST(test_cfl_header_has_flag_absent);
    RUN_TEST(test_cfl_header_has_flag_all_flags);

    return UNITY_END();
}
