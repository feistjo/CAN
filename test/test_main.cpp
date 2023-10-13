#define NATIVE  // to make can interface increment last receive time
#include <iomanip>
#include <iostream>

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
    TEST_ASSERT_EQUAL_HEX16(0xFF, test_signal);
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

void UnAlignedCanSignalTest(void)
{
    CANSignal<uint16_t, 0, 12, CANTemplateConvertFloat(1), 0> test_signal;
    test_signal = 0;
    uint64_t test_buf{0};
    test_signal.EncodeSignal(&test_buf);
    TEST_ASSERT_EQUAL_HEX64(0, test_buf);
    test_signal = 0xFFF;
    TEST_ASSERT_EQUAL_HEX16(0xFFF, test_signal);
    test_signal.EncodeSignal(&test_buf);
    TEST_ASSERT_EQUAL_HEX64(0xFFF, test_buf);
}

void SignedCanSignalTest(void)
{
    CANSignal<float, 0, 16, CANTemplateConvertFloat(1), 0, true> test_signal;
    test_signal = 0;
    uint64_t test_buf{0};
    test_signal.EncodeSignal(&test_buf);
    TEST_ASSERT_EQUAL_HEX64(0, test_buf);
    test_signal = -1;
    test_signal.EncodeSignal(&test_buf);
    TEST_ASSERT_EQUAL_HEX64(0xFFFF, test_buf);

    test_signal.DecodeSignal(&test_buf);
    TEST_ASSERT_EQUAL_FLOAT(-1, test_signal);
}

void BigEndianCanSignalTest(void)
{
    // CANSignal<uint16_t, 0, 16, CANTemplateConvertFloat(1), 0, false, ICANSignal::ByteOrder::kBigEndian> test_signal;
    MakeEndianUnsignedCANSignal(uint16_t, 8, 16, 1, 0, ICANSignal::ByteOrder::kBigEndian) test_signal;
    test_signal = 0xFF00;
    uint64_t test_buf{0};
    test_signal.EncodeSignal(&test_buf);
    TEST_ASSERT_EQUAL_HEX64(0x00FF, test_buf);
    std::cout << std::hex << test_buf << std::endl;

    test_signal.DecodeSignal(&test_buf);
    TEST_ASSERT_EQUAL_HEX16(0xFF00, test_signal);
}

void TypedCanSignalTest(void)
{
    CANSignal<uint8_t, 0, 8, CANTemplateConvertFloat(1), 0> actual_test_signal;
    ITypedCANSignal<uint8_t>& test_signal = actual_test_signal;
    test_signal = 0;
    uint64_t test_buf{0};
    test_signal.EncodeSignal(&test_buf);
    TEST_ASSERT_EQUAL_HEX64(0, test_buf);
    test_signal = 0xFF;
    TEST_ASSERT_EQUAL_HEX16(0xFF, test_signal);
    test_signal.EncodeSignal(&test_buf);
    TEST_ASSERT_EQUAL_HEX64(0xFF, test_buf);
}

void EnumClassSignalTest(void)
{
    enum class TestEnum : uint8_t
    {
        kT1 = 0,
        kT2 = 0xFF,
        kT3 = 1
    };
    CANSignal<TestEnum, 0, 8, CANTemplateConvertFloat(1), 0> test_signal;
    test_signal = TestEnum::kT1;
    uint64_t test_buf{0};
    test_signal.EncodeSignal(&test_buf);
    TEST_ASSERT_EQUAL_HEX64(0, test_buf);
    test_signal = TestEnum::kT2;
    TEST_ASSERT(test_signal == TestEnum::kT2);
    test_signal.EncodeSignal(&test_buf);
    TEST_ASSERT_EQUAL_HEX64(0xFF, test_buf);
}

void MITMotorBigEndianCANSignalTest(void)
{
    // testing for position working with non-byte-aligned big-endian signals
    MakeEndianUnsignedCANSignal(uint16_t, 8, 16, 1, 0, ICANSignal::ByteOrder::kBigEndian) p_des;
    MakeEndianUnsignedCANSignal(uint16_t, 28, 12, 1, 0, ICANSignal::ByteOrder::kBigEndian) v_des;
    MakeEndianUnsignedCANSignal(uint16_t, 32, 12, 1, 0, ICANSignal::ByteOrder::kBigEndian) kp;
    MakeEndianUnsignedCANSignal(uint16_t, 52, 12, 1, 0, ICANSignal::ByteOrder::kBigEndian) kd;
    MakeEndianUnsignedCANSignal(uint16_t, 56, 12, 1, 0, ICANSignal::ByteOrder::kBigEndian) torque;

    uint8_t msg[8];
    uint64_t* full_msg = reinterpret_cast<uint64_t*>(msg);
    *full_msg = 0;

    p_des = 0x8005;
    v_des = 0x800;
    kp = 0;
    kd = 0xFFF;
    torque = 0x800;
    p_des.EncodeSignal(full_msg);
    v_des.EncodeSignal(full_msg);
    kp.EncodeSignal(full_msg);
    kd.EncodeSignal(full_msg);
    torque.EncodeSignal(full_msg);

    // std::cout << std::hex << *full_msg << std::endl;

    for (int i = 0; i < 8; i++) std::cout << std::hex << std::setfill('0') << std::setw(2) << uint16_t(msg[i]) << " ";
    std::cout << std::endl;
    // printf("%d\n", msg[0]);

    TEST_ASSERT_EQUAL(bswap(uint64_t(0x8005800000fff800)), *full_msg);
}

void OperatorsTest(void)
{
    MakeSignedCANSignal(float, 0, 32, 1, 0) test_signal;
    test_signal = 0;
    TEST_ASSERT_EQUAL_FLOAT(0, test_signal);
    test_signal += 10;
    TEST_ASSERT_EQUAL_FLOAT(10, test_signal);
    test_signal -= 5;
    TEST_ASSERT_EQUAL_FLOAT(5, test_signal);
    test_signal *= 2;
    TEST_ASSERT_EQUAL_FLOAT(10, test_signal);
    test_signal /= 2;
    TEST_ASSERT_EQUAL_FLOAT(5, test_signal);
}

void MultiplexedCANMessageTest(void)
{
    MakeUnsignedCANSignal(uint8_t, 0, 8, 1, 0) tx_multiplexor;
    MakeSignedCANSignal(int32_t, 8, 32, 1, 0) tx_signal_0_0;
    MakeSignedCANSignal(int16_t, 40, 16, 1, 0) tx_signal_0_1;
    MultiplexedSignalGroup<2> tx_signals_0{0, tx_signal_0_0, tx_signal_0_1};
    MakeSignedCANSignal(int32_t, 8, 32, 1, 0) tx_signal_1_0;
    MakeSignedCANSignal(int16_t, 40, 16, 1, 0) tx_signal_1_1;
    MultiplexedSignalGroup<2> tx_signals_1{1, tx_signal_1_0, tx_signal_1_1};

    MakeUnsignedCANSignal(uint8_t, 0, 8, 1, 0) rx_multiplexor;
    MakeSignedCANSignal(int32_t, 8, 32, 1, 0) rx_signal_0_0;
    MakeSignedCANSignal(int16_t, 40, 16, 1, 0) rx_signal_0_1;
    MultiplexedSignalGroup<2> rx_signals_0{0, rx_signal_0_0, rx_signal_0_1};
    MakeSignedCANSignal(int32_t, 8, 32, 1, 0) rx_signal_1_0;
    MakeSignedCANSignal(int16_t, 40, 16, 1, 0) rx_signal_1_1;
    MultiplexedSignalGroup<2> rx_signals_1{1, rx_signal_1_0, rx_signal_1_1};

    MockCAN can{};

    MultiplexedCANTXMessage<2, 2, uint8_t> tx_msg{
        can, 100, 8, 100, std::array<uint8_t, 2>{0, 1}, tx_multiplexor, tx_signals_0, tx_signals_1};
    MultiplexedCANRXMessage<2, uint8_t> rx_msg{
        can, 100, []() { return 0; }, rx_multiplexor, rx_signals_0, rx_signals_1};

    tx_signal_0_0 = 1;
    tx_signal_0_1 = 2;
    tx_signal_1_0 = 3;
    tx_signal_1_1 = 4;

    tx_msg.EncodeAndSend();
    TEST_ASSERT(tx_multiplexor == 0);

    rx_msg.DecodeSignals(can.last_message);
    TEST_ASSERT(rx_multiplexor == 0);
    TEST_ASSERT(rx_signal_0_0 == 1);
    TEST_ASSERT(rx_signal_0_1 == 2);
    TEST_ASSERT(rx_signal_1_0 == 0);
    TEST_ASSERT(rx_signal_1_1 == 0);

    tx_signal_0_0 = 0;
    tx_signal_0_1 = 0;

    tx_msg.EncodeAndSend();
    TEST_ASSERT(tx_multiplexor == 1);

    rx_msg.DecodeSignals(can.last_message);
    TEST_ASSERT(rx_multiplexor == 1);
    TEST_ASSERT(rx_signal_0_0 == 1);
    TEST_ASSERT(rx_signal_0_1 == 2);
    TEST_ASSERT(rx_signal_1_0 == 3);
    TEST_ASSERT(rx_signal_1_1 == 4);

    tx_msg.EncodeAndSend();
    TEST_ASSERT(tx_multiplexor == 0);
}

int runUnityTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(CanSignalTest);
    RUN_TEST(UnAlignedCanSignalTest);
    RUN_TEST(BigEndianCanSignalTest);
    RUN_TEST(SignedCanSignalTest);
    RUN_TEST(TypedCanSignalTest);
    RUN_TEST(EnumClassSignalTest);
    RUN_TEST(MITMotorBigEndianCANSignalTest);
    RUN_TEST(OperatorsTest);
    RUN_TEST(MultiplexedCANMessageTest);
    return UNITY_END();
}

int main(void) { return runUnityTests(); }
