#include "can_interface.h"
#include "unity.h"

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

void CanSignalTest(void)
{
    CANSignal<uint8_t, 0, 8, CANTemplateConvertFloat(1), 0> test_signal;
    test_signal = 0;
    uint64_t test_buf{0};
    test_signal.EncodeSignal(&test_buf);
    TEST_ASSERT_EQUAL_HEX64(0, test_buf);
    test_signal = 0xFF;
    TEST_ASSERT_EQUAL_HEX8(0xFF, test_signal);
    test_signal.EncodeSignal(&test_buf);
    TEST_ASSERT_EQUAL_HEX64(0xFF, test_buf);

    CANSignal<uint8_t, 8, 8, CANTemplateConvertFloat(1), 0> test_signal_pos_8;
    test_signal_pos_8 = 0;
    test_buf = 0;
    test_signal_pos_8.EncodeSignal(&test_buf);
    TEST_ASSERT_EQUAL_HEX64(0, test_buf);
    test_signal_pos_8 = 0xFF;
    TEST_ASSERT_EQUAL_HEX8(0xFF, test_signal_pos_8);
    test_signal_pos_8.EncodeSignal(&test_buf);
    TEST_ASSERT_EQUAL_HEX64(0xFF00, test_buf);
    test_signal.EncodeSignal(&test_buf);
    TEST_ASSERT_EQUAL_HEX64(0xFFFF, test_buf);

    test_signal.DecodeSignal(&test_buf);
    TEST_ASSERT_EQUAL_HEX8(0xFF, test_signal);
    test_signal_pos_8.DecodeSignal(&test_buf);
    TEST_ASSERT_EQUAL_HEX8(0xFF, test_signal_pos_8);
}

int runUnityTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(CanSignalTest);
    return UNITY_END();
}

int main(void) { return runUnityTests(); }