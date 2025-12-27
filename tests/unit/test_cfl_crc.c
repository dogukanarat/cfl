/**
 * @file test_cfl_crc.c
 * @brief Unit tests for CFL CRC-16-CCITT implementation
 */

#include "unity.h"
#include "cfl/cfl.h"
#include <string.h>

/* Test setup and teardown */
void setUp(void)
{
    /* Called before each test */
}

void tearDown(void)
{
    /* Called after each test */
}

/* Test cfl_crc16 with NULL pointer */
void test_cfl_crc16_null_pointer(void)
{
    uint16_t crc = cfl_crc16(NULL, 10);
    TEST_ASSERT_EQUAL_HEX16(CFL_CRC_INIT, crc);
}

/* Test cfl_crc16 with zero length */
void test_cfl_crc16_zero_length(void)
{
    uint8_t data[] = {0x01, 0x02, 0x03};
    uint16_t crc = cfl_crc16(data, 0);
    TEST_ASSERT_EQUAL_HEX16(CFL_CRC_INIT, crc);
}

/* Test cfl_crc16 with known data */
void test_cfl_crc16_known_data(void)
{
    /* Test with simple data pattern */
    uint8_t data1[] = {0x00};
    uint16_t crc1 = cfl_crc16(data1, sizeof(data1));
    TEST_ASSERT_NOT_EQUAL(CFL_CRC_INIT, crc1);

    /* Test with another pattern */
    uint8_t data2[] = {0xFF};
    uint16_t crc2 = cfl_crc16(data2, sizeof(data2));
    TEST_ASSERT_NOT_EQUAL(CFL_CRC_INIT, crc2);
    TEST_ASSERT_NOT_EQUAL(crc1, crc2);
}

/* Test cfl_crc16 with "123456789" - standard CRC test vector */
void test_cfl_crc16_test_vector(void)
{
    /* CRC-16-CCITT of "123456789" should be 0x29B1 */
    uint8_t data[] = "123456789";
    uint16_t crc = cfl_crc16(data, 9); /* Don't include null terminator */
    TEST_ASSERT_EQUAL_HEX16(0x29B1, crc);
}

/* Test cfl_crc16 determinism */
void test_cfl_crc16_deterministic(void)
{
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint16_t crc1 = cfl_crc16(data, sizeof(data));
    uint16_t crc2 = cfl_crc16(data, sizeof(data));
    TEST_ASSERT_EQUAL_HEX16(crc1, crc2);
}

/* Test cfl_crc16 with different data produces different CRC */
void test_cfl_crc16_different_data(void)
{
    uint8_t data1[] = {0x01, 0x02, 0x03};
    uint8_t data2[] = {0x01, 0x02, 0x04};
    uint16_t crc1 = cfl_crc16(data1, sizeof(data1));
    uint16_t crc2 = cfl_crc16(data2, sizeof(data2));
    TEST_ASSERT_NOT_EQUAL(crc1, crc2);
}

/* Test cfl_crc16 with maximum payload size */
void test_cfl_crc16_max_payload(void)
{
    uint8_t data[CFL_MAX_PAYLOAD_SIZE];
    memset(data, 0xAA, sizeof(data));
    uint16_t crc = cfl_crc16(data, sizeof(data));
    TEST_ASSERT_NOT_EQUAL(CFL_CRC_INIT, crc);
}

/* Test cfl_crc16_continue with NULL pointer */
void test_cfl_crc16_continue_null_pointer(void)
{
    uint16_t crc = cfl_crc16_continue(0x1234, NULL, 10);
    TEST_ASSERT_EQUAL_HEX16(0x1234, crc);
}

/* Test cfl_crc16_continue matches single call */
void test_cfl_crc16_continue_matches_single(void)
{
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};

    /* Compute CRC in one call */
    uint16_t crc_single = cfl_crc16(data, sizeof(data));

    /* Compute CRC in multiple calls */
    uint16_t crc_continue = CFL_CRC_INIT;
    crc_continue = cfl_crc16_continue(crc_continue, data, 3);
    crc_continue = cfl_crc16_continue(crc_continue, data + 3, 3);

    TEST_ASSERT_EQUAL_HEX16(crc_single, crc_continue);
}

/* Test cfl_crc16_continue with header and payload */
void test_cfl_crc16_continue_header_payload(void)
{
    cfl_header_t hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.sync = CFL_SYNC_WORD;
    hdr.version = CFL_VERSION;
    hdr.id = 0x1234;

    uint8_t payload[] = {0xAA, 0xBB, 0xCC, 0xDD};

    /* Compute CRC over header (excluding CRC field) + payload */
    uint16_t crc = cfl_crc16((uint8_t *)&hdr, offsetof(cfl_header_t, crc));
    crc = cfl_crc16_continue(crc, payload, sizeof(payload));

    TEST_ASSERT_NOT_EQUAL(CFL_CRC_INIT, crc);
}

/* Test CRC detects single bit error */
void test_cfl_crc16_detects_bit_error(void)
{
    uint8_t data1[] = {0x00, 0x00, 0x00, 0x00};
    uint8_t data2[] = {0x01, 0x00, 0x00, 0x00}; /* Single bit flipped */

    uint16_t crc1 = cfl_crc16(data1, sizeof(data1));
    uint16_t crc2 = cfl_crc16(data2, sizeof(data2));

    TEST_ASSERT_NOT_EQUAL(crc1, crc2);
}

/* Test CRC detects byte swap */
void test_cfl_crc16_detects_byte_swap(void)
{
    uint8_t data1[] = {0x12, 0x34};
    uint8_t data2[] = {0x34, 0x12};

    uint16_t crc1 = cfl_crc16(data1, sizeof(data1));
    uint16_t crc2 = cfl_crc16(data2, sizeof(data2));

    TEST_ASSERT_NOT_EQUAL(crc1, crc2);
}

/* Main test runner */
int main(void)
{
    UNITY_BEGIN();

    /* CRC basic tests */
    RUN_TEST(test_cfl_crc16_null_pointer);
    RUN_TEST(test_cfl_crc16_zero_length);
    RUN_TEST(test_cfl_crc16_known_data);
    RUN_TEST(test_cfl_crc16_test_vector);
    RUN_TEST(test_cfl_crc16_deterministic);
    RUN_TEST(test_cfl_crc16_different_data);
    RUN_TEST(test_cfl_crc16_max_payload);

    /* CRC continue tests */
    RUN_TEST(test_cfl_crc16_continue_null_pointer);
    RUN_TEST(test_cfl_crc16_continue_matches_single);
    RUN_TEST(test_cfl_crc16_continue_header_payload);

    /* Error detection tests */
    RUN_TEST(test_cfl_crc16_detects_bit_error);
    RUN_TEST(test_cfl_crc16_detects_byte_swap);

    return UNITY_END();
}
