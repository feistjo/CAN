#pragma once

#include <stdint.h>

#include <array>
#include <chrono>
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
 * @tparam mask This is calculated for you by default
 * @tparam unity_factor This is calculated for you by default
 */
template <typename SignalType,
          uint8_t position,
          uint8_t length,
          int factor,
          int offset,
          bool signed_raw = false,
          uint64_t mask = generate_mask(position, length),
          bool unity_factor = factor == CANTemplateConvertFloat(1)
                              && offset == 0>  // unity_factor is used for increased precision on unity-factor 64-bit
                                               // signals by getting rid of floating point error
class CANSignal : public ICANSignal
{
public:
    CANSignal() {}

    void EncodeSignal(uint64_t *buffer) override
    {
        if (unity_factor)
        {
            *buffer |= (static_cast<uint64_t>(signal_) << position) & mask;
        }
        else
        {
            *buffer |= (static_cast<uint64_t>(((signal_ / CANTemplateGetFloat(factor)) + CANTemplateGetFloat(offset)))
                        << position)
                       & mask;
        }
    }

    void DecodeSignal(uint64_t *buffer) override
    {
        if (unity_factor)
        {
            signal_ = static_cast<SignalType>((*buffer & mask) >> position);
        }
        else
        {
            signal_ = static_cast<SignalType>((((*buffer & mask) >> position) * CANTemplateGetFloat(factor))
                                              + CANTemplateGetFloat(offset));
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
    CANTXMessage(ICAN &can_interface, uint16_t id, uint8_t length, uint32_t period, uint32_t start_time, Ts &...signals)
        : can_interface_{can_interface},
          message_{id, length, std::array<uint8_t, 8>()},
          transmit_timer_{period, EncodeAndSend, VirtualTimer::Type::kRepeating},
          signals_{&signals...}
    {
        static_assert(sizeof...(signals) == num_signals, "Wrong number of signals passed into CANTXMessage.");
        transmit_timer_.Start(start_time);
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
        uint64_t temp_raw{0};
        for (uint8_t i = 0; i < num_signals; i++)
        {
            signals_[i]->EncodeSignal(&temp_raw);
        }
        *reinterpret_cast<uint64_t *>(message_.data_.data()) = temp_raw;
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