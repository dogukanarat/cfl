/**
 * @file test_cfl_builders.c
 * @brief Unit tests for CFL message builder functions
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

/* ========== Request Builder Tests ========== */

void test_cfl_build_request_null_pointer(void)
{
    cfl_error_t err = cfl_build_request(NULL, 0x1234, 100);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_NULL, err);
}

void test_cfl_build_request_basic(void)
{
    cfl_header_t hdr;
    memset(&hdr, 0xFF, sizeof(hdr)); /* Fill with garbage */

    cfl_error_t err = cfl_build_request(&hdr, 0x1234, 42);

    TEST_ASSERT_EQUAL_INT(CFL_OK, err);
    TEST_ASSERT_EQUAL_HEX16(CFL_SYNC_WORD, hdr.sync);
    TEST_ASSERT_EQUAL_UINT8(CFL_VERSION, hdr.version);
    TEST_ASSERT_EQUAL_HEX8(CFL_F_RQST, hdr.flags);
    TEST_ASSERT_EQUAL_HEX16(0x1234, hdr.id);
    TEST_ASSERT_EQUAL_UINT16(42, hdr.seq);
    TEST_ASSERT_EQUAL_UINT16(0, hdr.length);
    TEST_ASSERT_EQUAL_UINT16(0, hdr.crc);
}

void test_cfl_build_request_max_values(void)
{
    cfl_header_t hdr;

    cfl_error_t err = cfl_build_request(&hdr, 0xFFFF, 0xFFFF);

    TEST_ASSERT_EQUAL_INT(CFL_OK, err);
    TEST_ASSERT_EQUAL_HEX16(0xFFFF, hdr.id);
    TEST_ASSERT_EQUAL_UINT16(0xFFFF, hdr.seq);
}

void test_cfl_build_request_zero_values(void)
{
    cfl_header_t hdr;

    cfl_error_t err = cfl_build_request(&hdr, 0, 0);

    TEST_ASSERT_EQUAL_INT(CFL_OK, err);
    TEST_ASSERT_EQUAL_HEX16(0, hdr.id);
    TEST_ASSERT_EQUAL_UINT16(0, hdr.seq);
}

/* ========== Reply Builder Tests ========== */

void test_cfl_build_reply_null_header(void)
{
    cfl_header_t request;
    cfl_build_request(&request, 0x1234, 100);

    cfl_error_t err = cfl_build_reply(NULL, &request);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_NULL, err);
}

void test_cfl_build_reply_null_request(void)
{
    cfl_header_t reply;

    cfl_error_t err = cfl_build_reply(&reply, NULL);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_NULL, err);
}

void test_cfl_build_reply_basic(void)
{
    cfl_header_t request, reply;

    cfl_build_request(&request, 0xABCD, 999);

    cfl_error_t err = cfl_build_reply(&reply, &request);

    TEST_ASSERT_EQUAL_INT(CFL_OK, err);
    TEST_ASSERT_EQUAL_HEX16(CFL_SYNC_WORD, reply.sync);
    TEST_ASSERT_EQUAL_UINT8(CFL_VERSION, reply.version);
    TEST_ASSERT_EQUAL_HEX8(CFL_F_RPLY, reply.flags);
    TEST_ASSERT_EQUAL_HEX16(request.id, reply.id);
    TEST_ASSERT_EQUAL_UINT16(request.seq, reply.seq);
    TEST_ASSERT_EQUAL_UINT16(0, reply.length);
    TEST_ASSERT_EQUAL_UINT16(0, reply.crc);
}

void test_cfl_build_reply_preserves_id_and_seq(void)
{
    cfl_header_t request, reply;

    cfl_build_request(&request, 0x1111, 2222);

    cfl_build_reply(&reply, &request);

    TEST_ASSERT_EQUAL_HEX16(0x1111, reply.id);
    TEST_ASSERT_EQUAL_UINT16(2222, reply.seq);
}

/* ========== ACK Builder Tests ========== */

void test_cfl_build_ack_null_header(void)
{
    cfl_header_t request;
    cfl_build_request(&request, 0x1234, 100);

    cfl_error_t err = cfl_build_ack(NULL, &request);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_NULL, err);
}

void test_cfl_build_ack_null_request(void)
{
    cfl_header_t ack;

    cfl_error_t err = cfl_build_ack(&ack, NULL);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_NULL, err);
}

void test_cfl_build_ack_basic(void)
{
    cfl_header_t request, ack;

    cfl_build_request(&request, 0x5678, 777);

    cfl_error_t err = cfl_build_ack(&ack, &request);

    TEST_ASSERT_EQUAL_INT(CFL_OK, err);
    TEST_ASSERT_EQUAL_HEX16(CFL_SYNC_WORD, ack.sync);
    TEST_ASSERT_EQUAL_UINT8(CFL_VERSION, ack.version);
    TEST_ASSERT_EQUAL_HEX8(CFL_F_ACK, ack.flags);
    TEST_ASSERT_EQUAL_HEX16(request.id, ack.id);
    TEST_ASSERT_EQUAL_UINT16(request.seq, ack.seq);
}

/* ========== NACK Builder Tests ========== */

void test_cfl_build_nack_null_header(void)
{
    cfl_header_t request;
    cfl_build_request(&request, 0x1234, 100);

    cfl_error_t err = cfl_build_nack(NULL, &request);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_NULL, err);
}

void test_cfl_build_nack_null_request(void)
{
    cfl_header_t nack;

    cfl_error_t err = cfl_build_nack(&nack, NULL);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_NULL, err);
}

void test_cfl_build_nack_basic(void)
{
    cfl_header_t request, nack;

    cfl_build_request(&request, 0x9ABC, 555);

    cfl_error_t err = cfl_build_nack(&nack, &request);

    TEST_ASSERT_EQUAL_INT(CFL_OK, err);
    TEST_ASSERT_EQUAL_HEX16(CFL_SYNC_WORD, nack.sync);
    TEST_ASSERT_EQUAL_UINT8(CFL_VERSION, nack.version);
    TEST_ASSERT_EQUAL_HEX8(CFL_F_NACK, nack.flags);
    TEST_ASSERT_EQUAL_HEX16(request.id, nack.id);
    TEST_ASSERT_EQUAL_UINT16(request.seq, nack.seq);
}

/* ========== Push Builder Tests ========== */

void test_cfl_build_push_null_pointer(void)
{
    cfl_error_t err = cfl_build_push(NULL, 0x1234, 100);
    TEST_ASSERT_EQUAL_INT(CFL_ERR_NULL, err);
}

void test_cfl_build_push_basic(void)
{
    cfl_header_t hdr;

    cfl_error_t err = cfl_build_push(&hdr, 0xDEF0, 123);

    TEST_ASSERT_EQUAL_INT(CFL_OK, err);
    TEST_ASSERT_EQUAL_HEX16(CFL_SYNC_WORD, hdr.sync);
    TEST_ASSERT_EQUAL_UINT8(CFL_VERSION, hdr.version);
    TEST_ASSERT_EQUAL_HEX8(CFL_F_PUSH, hdr.flags);
    TEST_ASSERT_EQUAL_HEX16(0xDEF0, hdr.id);
    TEST_ASSERT_EQUAL_UINT16(123, hdr.seq);
    TEST_ASSERT_EQUAL_UINT16(0, hdr.length);
    TEST_ASSERT_EQUAL_UINT16(0, hdr.crc);
}

/* ========== Integration Tests ========== */

void test_request_reply_flow(void)
{
    cfl_header_t request, reply;
    uint8_t request_payload[] = {0x01, 0x02, 0x03};
    uint8_t reply_payload[] = {0x04, 0x05, 0x06, 0x07};

    /* Build request */
    cfl_build_request(&request, 0x1234, 1);
    request.length = sizeof(request_payload);
    cfl_header_compute_crc(&request, request_payload, sizeof(request_payload));

    /* Validate request */
    cfl_error_t err = cfl_message_validate(&request, request_payload,
                                           sizeof(request_payload));
    TEST_ASSERT_EQUAL_INT(CFL_OK, err);

    /* Build reply */
    cfl_build_reply(&reply, &request);
    reply.length = sizeof(reply_payload);
    cfl_header_compute_crc(&reply, reply_payload, sizeof(reply_payload));

    /* Validate reply */
    err = cfl_message_validate(&reply, reply_payload, sizeof(reply_payload));
    TEST_ASSERT_EQUAL_INT(CFL_OK, err);

    /* Verify reply matches request */
    TEST_ASSERT_EQUAL_HEX16(request.id, reply.id);
    TEST_ASSERT_EQUAL_UINT16(request.seq, reply.seq);
    TEST_ASSERT_TRUE(cfl_header_has_flag(&reply, CFL_F_RPLY));
}

void test_request_ack_flow(void)
{
    cfl_header_t request, ack;

    /* Build request */
    cfl_build_request(&request, 0xABCD, 42);
    cfl_header_compute_crc(&request, NULL, 0);

    /* Build ACK */
    cfl_build_ack(&ack, &request);
    cfl_header_compute_crc(&ack, NULL, 0);

    /* Validate both */
    TEST_ASSERT_EQUAL_INT(CFL_OK, cfl_message_validate(&request, NULL, 0));
    TEST_ASSERT_EQUAL_INT(CFL_OK, cfl_message_validate(&ack, NULL, 0));

    /* Verify ACK matches request */
    TEST_ASSERT_EQUAL_HEX16(request.id, ack.id);
    TEST_ASSERT_EQUAL_UINT16(request.seq, ack.seq);
    TEST_ASSERT_TRUE(cfl_header_has_flag(&ack, CFL_F_ACK));
}

void test_request_nack_flow(void)
{
    cfl_header_t request, nack;

    /* Build request */
    cfl_build_request(&request, 0x5555, 999);
    cfl_header_compute_crc(&request, NULL, 0);

    /* Build NACK */
    cfl_build_nack(&nack, &request);
    cfl_header_compute_crc(&nack, NULL, 0);

    /* Validate both */
    TEST_ASSERT_EQUAL_INT(CFL_OK, cfl_message_validate(&request, NULL, 0));
    TEST_ASSERT_EQUAL_INT(CFL_OK, cfl_message_validate(&nack, NULL, 0));

    /* Verify NACK matches request */
    TEST_ASSERT_EQUAL_HEX16(request.id, nack.id);
    TEST_ASSERT_EQUAL_UINT16(request.seq, nack.seq);
    TEST_ASSERT_TRUE(cfl_header_has_flag(&nack, CFL_F_NACK));
}

void test_push_message_flow(void)
{
    cfl_header_t push;
    uint8_t payload[] = {0xDE, 0xAD, 0xBE, 0xEF};

    /* Build push */
    cfl_build_push(&push, 0x8888, 0);
    push.length = sizeof(payload);
    cfl_header_compute_crc(&push, payload, sizeof(payload));

    /* Validate */
    cfl_error_t err = cfl_message_validate(&push, payload, sizeof(payload));
    TEST_ASSERT_EQUAL_INT(CFL_OK, err);
    TEST_ASSERT_TRUE(cfl_header_has_flag(&push, CFL_F_PUSH));
}

void test_builder_output_validates(void)
{
    cfl_header_t hdrs[5];

    /* Build various message types */
    cfl_build_request(&hdrs[0], 0x0001, 1);
    cfl_build_push(&hdrs[1], 0x0002, 2);

    cfl_header_t temp_req;
    cfl_build_request(&temp_req, 0x0003, 3);
    cfl_build_reply(&hdrs[2], &temp_req);
    cfl_build_ack(&hdrs[3], &temp_req);
    cfl_build_nack(&hdrs[4], &temp_req);

    /* All should validate */
    for (int i = 0; i < 5; i++) {
        cfl_error_t err = cfl_header_validate(&hdrs[i]);
        TEST_ASSERT_EQUAL_INT(CFL_OK, err);
    }
}

/* Main test runner */
int main(void)
{
    UNITY_BEGIN();

    /* Request builder */
    RUN_TEST(test_cfl_build_request_null_pointer);
    RUN_TEST(test_cfl_build_request_basic);
    RUN_TEST(test_cfl_build_request_max_values);
    RUN_TEST(test_cfl_build_request_zero_values);

    /* Reply builder */
    RUN_TEST(test_cfl_build_reply_null_header);
    RUN_TEST(test_cfl_build_reply_null_request);
    RUN_TEST(test_cfl_build_reply_basic);
    RUN_TEST(test_cfl_build_reply_preserves_id_and_seq);

    /* ACK builder */
    RUN_TEST(test_cfl_build_ack_null_header);
    RUN_TEST(test_cfl_build_ack_null_request);
    RUN_TEST(test_cfl_build_ack_basic);

    /* NACK builder */
    RUN_TEST(test_cfl_build_nack_null_header);
    RUN_TEST(test_cfl_build_nack_null_request);
    RUN_TEST(test_cfl_build_nack_basic);

    /* Push builder */
    RUN_TEST(test_cfl_build_push_null_pointer);
    RUN_TEST(test_cfl_build_push_basic);

    /* Integration tests */
    RUN_TEST(test_request_reply_flow);
    RUN_TEST(test_request_ack_flow);
    RUN_TEST(test_request_nack_flow);
    RUN_TEST(test_push_message_flow);
    RUN_TEST(test_builder_output_validates);

    return UNITY_END();
}
