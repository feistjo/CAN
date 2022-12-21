#pragma once

#include <stdint.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <vector>

#include "virtualTimer.h"

class CANMessage
{
public:
    CANMessage(uint16_t id, uint8_t len, std::array<uint8_t, 8> data) : id_{id}, len_{len}, data_{data} {}

    uint16_t id_;
    uint8_t len_;
    std::array<uint8_t, 8> data_;
};

class ICANSignal
{
public:
    enum class ByteOrder
    {
        kBigEndian,
        kLittleEndian
    };
    virtual void EncodeSignal(uint64_t *buffer) = 0;
    virtual void DecodeSignal(uint64_t *buffer) = 0;
};

// Generates a mask of which bits in the message correspond to a specific signal
constexpr uint64_t generate_mask(uint8_t position, uint8_t length)
{
    return 0xFFFFFFFFFFFFFFFFull << (64 - length) >> (64 - (length + position));
}
static constexpr int kCANTemplateFloatDenominator{10000};
constexpr int CANTemplateConvertFloat(float value) { return value * kCANTemplateFloatDenominator; }
constexpr float CANTemplateGetFloat(int value) { return static_cast<float>(value) / kCANTemplateFloatDenominator; }

template <bool signed_raw>
struct GetCANRawType;

template <>
struct GetCANRawType<true>
{
    using type = int64_t;
};

template <>
struct GetCANRawType<false>
{
    using type = uint64_t;
};

/**
 * @brief A class for decoding and encoding CAN signals. Note: only works with little endian signals on a little endian
 * architecture, and you must manually ensure consistency with the DBC
 *
 * @tparam SignalType The type of variable in the application to be encoded/decoded
 * @tparam position The position of the first bit of the signal in the message
 * @tparam length The length of the signal in the message
 * @tparam factor The factor to multiply the raw signal by (gotten using CANTemplateConvertFloat(float value))
 * @tparam offset The offset added to the raw signal (gotten using CANTemplateConvertFloat(float value))
 * @tparam signed_raw Whether or not the signal is signed
 * @tparam byte_order The order of bytes in the signal (big endian or little endian). Do not change this from the
 * default (little endian) if you aren't sure you need to.
 * @tparam mask This is calculated for you by default
 * @tparam unity_factor This is calculated for you by default
 */
template <typename SignalType,
          uint8_t position,
          uint8_t length,
          int factor,
          int offset,
          bool signed_raw = false,
          ICANSignal::ByteOrder byte_order = ICANSignal::ByteOrder::kLittleEndian,
          uint64_t mask = generate_mask(position, length),
          bool unity_factor = factor == CANTemplateConvertFloat(1)
                              && offset == 0>  // unity_factor is used for increased precision on unity-factor 64-bit
                                               // signals by getting rid of floating point error
class CANSignal : public ICANSignal
{
    using underlying_type = typename GetCANRawType<signed_raw>::type;

public:
    CANSignal() {}

    void EncodeSignal(uint64_t *buffer) override
    {
        if (unity_factor)
        {
            if (byte_order == ByteOrder::kLittleEndian)
            {
                *buffer |= (static_cast<underlying_type>(signal_) << position) & mask;
            }
            else
            {
                uint8_t temp_reversed_buffer[8]{0};
                *reinterpret_cast<underlying_type *>(temp_reversed_buffer) |=
                    (static_cast<underlying_type>(signal_) << (64 - (position + length)));
                std::reverse(std::begin(temp_reversed_buffer), std::end(temp_reversed_buffer));
                *buffer |= *reinterpret_cast<underlying_type *>(temp_reversed_buffer) & mask;
            }
        }
        else
        {
            if (byte_order == ByteOrder::kLittleEndian)
            {
                *buffer |= (static_cast<underlying_type>(
                                ((signal_ - CANTemplateGetFloat(offset)) / CANTemplateGetFloat(factor)))
                            << position)
                           & mask;
            }
            else
            {
                uint8_t temp_reversed_buffer[8]{0};
                *reinterpret_cast<underlying_type *>(temp_reversed_buffer) |=
                    (static_cast<underlying_type>(
                         ((signal_ - CANTemplateGetFloat(offset)) / CANTemplateGetFloat(factor)))
                     << (64 - (position + length)));
                std::reverse(std::begin(temp_reversed_buffer), std::end(temp_reversed_buffer));
                *buffer |= *reinterpret_cast<underlying_type *>(temp_reversed_buffer) & mask;
            }
        }
    }

    void DecodeSignal(uint64_t *buffer) override
    {
        if (unity_factor)
        {
            if (byte_order == ByteOrder::kLittleEndian)
            {
                uint8_t temp_buffer[8]{0};
                *reinterpret_cast<underlying_type *>(temp_buffer) = *buffer & mask;
                signal_ = static_cast<SignalType>((*reinterpret_cast<underlying_type *>(temp_buffer)) >> position);
            }
            else
            {
                uint8_t temp_buffer[8]{0};
                *reinterpret_cast<underlying_type *>(temp_buffer) = *buffer & mask;
                std::reverse(std::begin(temp_buffer), std::end(temp_buffer));
                signal_ = static_cast<SignalType>((*reinterpret_cast<underlying_type *>(temp_buffer))
                                                  >> (64 - (position + length)));
            }
        }
        else
        {
            if (byte_order == ByteOrder::kLittleEndian)
            {
                uint8_t temp_buffer[8]{0};
                *reinterpret_cast<underlying_type *>(temp_buffer) = *buffer & mask;
                signal_ = static_cast<SignalType>(
                    (((*reinterpret_cast<underlying_type *>(temp_buffer)) >> position) * CANTemplateGetFloat(factor))
                    + CANTemplateGetFloat(offset));
            }
            else
            {
                uint8_t temp_buffer[8]{0};
                *reinterpret_cast<underlying_type *>(temp_buffer) = *buffer & mask;
                std::reverse(std::begin(temp_buffer), std::end(temp_buffer));
                signal_ = static_cast<SignalType>(
                    (((*reinterpret_cast<underlying_type *>(temp_buffer)) >> (64 - (position + length)))
                     * CANTemplateGetFloat(factor))
                    + CANTemplateGetFloat(offset));
            }
        }
    }

    SignalType &value_ref() { return signal_; }

    void operator=(const SignalType &signal) { signal_ = signal; }

    operator SignalType() const { return signal_; }

private:
    SignalType signal_;
};

class ICANTXMessage
{
public:
    virtual uint16_t GetID() = 0;
    virtual VirtualTimer &GetTransmitTimer() = 0;
    virtual void EncodeSignals() = 0;
};

class ICANRXMessage
{
public:
    virtual uint16_t GetID() = 0;
    virtual void DecodeSignals(CANMessage message) = 0;
};

class ICAN
{
public:
    enum class BaudRate
    {
        kBaud1M = 1000000,
        kBaud500K = 500000,
        kBaud250K = 250000,
        kBaud125k = 125000
    };

    virtual void Initialize(BaudRate baud);

    virtual bool SendMessage(CANMessage &msg) = 0;

    virtual void RegisterRXMessage(ICANRXMessage &msg) = 0;

    virtual void Tick() = 0;
};

/**
 * @brief A class for storing signals in a message that sends every period
 */
template <size_t num_signals>
class CANTXMessage : public ICANTXMessage
{
public:
    template <typename... Ts>
    /**
     * @brief Construct a new CANTXMessage object
     *
     * @param can_interface The ICAN object the message will be transmitted on
     * @param id The ID of the CAN message
     * @param length The length in bytes of the message
     * @param period The transmit period in ms of the message
     * @param start_time The time in ms to start transmitting the message
     * @param signals The ICANSignals contained in the message
     */
    CANTXMessage(ICAN &can_interface, uint16_t id, uint8_t length, uint32_t period, Ts &...signals)
        : can_interface_{can_interface},
          message_{id, length, std::array<uint8_t, 8>()},
          transmit_timer_{period, [this]() { this->EncodeAndSend(); }, VirtualTimer::Type::kRepeating},
          signals_{&signals...}
    {
        static_assert(sizeof...(signals) == num_signals, "Wrong number of signals passed into CANTXMessage.");
    }

    template <typename... Ts>
    /**
     * @brief Construct a new CANTXMessage object and automatically adds it to a VirtualTimerGroup
     *
     * @param can_interface The ICAN object the message will be transmitted on
     * @param id The ID of the CAN message
     * @param length The length in bytes of the message
     * @param period The transmit period in ms of the message
     * @param start_time The time in ms to start transmitting the message
     * @param timer_group A timer group to add the transmit timer to
     * @param signals The ICANSignals contained in the message
     */
    CANTXMessage(ICAN &can_interface,
                 uint16_t id,
                 uint8_t length,
                 uint32_t period,
                 VirtualTimerGroup &timer_group,
                 Ts &...signals)
        : CANTXMessage(can_interface, id, length, period, signals...)
    {
        timer_group.AddTimer(transmit_timer_);
    }

    void EncodeAndSend()
    {
        EncodeSignals();
        can_interface_.SendMessage(message_);
    }

    uint16_t GetID() { return message_.id_; }

    VirtualTimer &GetTransmitTimer() { return transmit_timer_; }

private:
    ICAN &can_interface_;
    CANMessage message_;
    VirtualTimer transmit_timer_;
    std::array<ICANSignal *, num_signals> signals_;

    void EncodeSignals()
    {
        uint8_t temp_raw[8]{0};
        for (uint8_t i = 0; i < num_signals; i++)
        {
            signals_[i]->EncodeSignal(reinterpret_cast<uint64_t *>(temp_raw));
        }
        std::copy(std::begin(temp_raw), std::end(temp_raw), message_.data_.begin());
    }
};

/**
 * @brief A class for storing signals that get updated every time a matching message is received
 */
template <size_t num_signals>
class CANRXMessage : public ICANRXMessage
{
public:
    template <typename... Ts>
    CANRXMessage(ICAN &can_interface, uint16_t id, Ts &...signals)
        : can_interface_{can_interface}, id_{id}, signals_{&signals...}
    {
        static_assert(sizeof...(signals) == num_signals, "Wrong number of signals passed into CANRXMessage.");
        can_interface_.RegisterRXMessage(*this);
    }

    uint16_t GetID() { return id_; }

    void DecodeSignals(CANMessage message)
    {
        uint64_t temp_raw = *reinterpret_cast<uint64_t *>(message.data_.data());
        for (uint8_t i = 0; i < num_signals; i++)
        {
            signals_[i]->DecodeSignal(&temp_raw);
        }
    }

private:
    ICAN &can_interface_;
    uint16_t id_;
    std::array<ICANSignal *, num_signals> signals_;

    uint64_t raw_message;
};