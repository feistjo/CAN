import cantools
import sys

def dbc_to_h(dbc_file, h_file):
    # Load the DBC file
    db = cantools.database.load_file(dbc_file)

    # Iterate over the messages and signals in the DBC file
    rx_messages = ""
    tx_messages = ""
    with open(h_file, 'w') as file:
        file.write("// Signals\n")
    for message in db.messages:
        multiplexor_signal = ""
        signals = []

        for signal in message.signals:
            signalString = ""
            if (signal.choices != None):
                signalString += "enum class " + signal.name + "_Enum : data_type_placeholder {\n"
                for value, name in signal.choices.items():
                    signalString += '_'.join(str(name).replace('-', ' ').replace('(', ' ').replace(')', ' ').split()) + " = " + str(value) + ",\n"
                signalString += "};\n"
            if signal.byte_order == "big_endian":
                byteOrder = "Endian"
                endian = ", ICANSignal::ByteOrder::kBigEndian"
            else:
                byteOrder = ""
                endian = ""
            signalType = "Signed" if signal.is_signed else "Unsigned"
            signalString += "Make" + byteOrder + signalType + "CANSignal(" + ("data_type_placeholder" if signal.choices == None else signal.name + "_Enum") + "," + str(signal.start) + "," + str(signal.length) + "," + str(signal.scale) + ("" if isinstance(signal.scale, int) else "f") + "," + str(signal.offset) + endian + ") " + signal.name + "_Signal{};\n"
            with open(h_file, 'a') as file:
                file.write(signalString)
            if message.is_multiplexed():
                if (signal.is_multiplexer):
                    multiplexor_signal = signal.name + "_Signal_"
                    continue
                
                found = False
                for i in range(len(signals)):
                    if signals[i][0] == signal.multiplexer_ids[0]:
                        signals[i].append(signal.name + "_Signal_")
                        found = True
                        continue
                if not found:
                    signals.append([signal.multiplexer_ids[0], signal.name + "_Signal_"])
            else:
                signals.append(signal.name + "_Signal_")
        print(signals)
        if message.is_multiplexed():
            signal_groups_str = ""
            signal_groups = []
            for i in range(len(signals)):
                signal_groups_str += "MultiplexedSignalGroup<" + str(len(signals[i]) - 1) + "> " + message.name + "_SignalGroup_" + str(i) + "_{" + ', '.join(str(s) for s in signals[i]) + "};\n"
                signal_groups.append(message.name + "_SignalGroup_" + str(i) + "_")
            #MultiplexedCANTXMessage<2, uint8_t> tx_msg{can, 100, 8, 100, tx_multiplexor, tx_signals_0, tx_signals_1};
            rx_message = signal_groups_str
            rx_message += "MultiplexedCANRXMessage<" + str(len(signal_groups)) + ", " + "data_type_placeholder" + "> " + message.name + "_RX_Message_{can_bus, 0x" + format(message.frame_id, 'x') + ", " + multiplexor_signal + ", " + ', '.join(signal_groups) + "};\n"
            tx_message = "MultiplexedCANTXMessage<" + str(len(signal_groups)) + ", " + "data_type_placeholder" + "> " + message.name + "_TX_Message_{can_bus, 0x" + format(message.frame_id, 'x') + ", " + ("true, " if message.is_extended_frame else "")  + str(message.length) + ", freq_placeholder, timer_group, " + multiplexor_signal + ", " + ', '.join(signal_groups) +  "};\n"
        else:
            rx_message = "CANRXMessage<" + str(len(signals)) + "> " + message.name + "_RX_Message_{can_bus, 0x" + format(message.frame_id, 'x') + ", " + ', '.join(signals) + "};\n"
            tx_message = "CANTXMessage<" + str(len(signals)) + "> " + message.name + "_TX_Message_{can_bus, 0x" + format(message.frame_id, 'x') + ", " + ("true, " if message.is_extended_frame else "") + str(message.length) + ", freq_placeholder, timer_group, " + ', '.join(signals) + "};\n"
        rx_messages += rx_message
        tx_messages += tx_message
    with open(h_file, 'a') as file:
        file.write("\n// RX Messages\n")
        file.write(rx_messages)
        file.write("\n// TX Messages\n")
        file.write(tx_messages)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python dbc_to_h.py <input_dbc_file> <output_h_file>")
        sys.exit(1)

    dbc_file = sys.argv[1]
    h_file = sys.argv[2]

    dbc_to_h(dbc_file, h_file)
    print(f"Converted {dbc_file} to {h_file}")
