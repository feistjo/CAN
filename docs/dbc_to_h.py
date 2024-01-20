import cantools
import sys

# This will generate a functional .h file from a dbc compatible with the NFR CAN library
# Note: signal data types should work, but may be incorrect or suboptimal

def get_data_type(signed, length, factor, offset):
    if not isinstance(factor, int):
        if length <= 32:
            return "float"
        else:
            return "double"
    if not signed:
        if length == 1:
            return "bool"
        elif factor < 0:
            return "int" + str(64 if length >= 32 else 32 if length >= 16 else 16 if length >= 8 else 8) + "_t"
        else:
            return "uint" + str(64 if length > 32 else 32 if length > 16 else 16 if length > 8 else 8) + "_t"
    else:
        return "int" + str(64 if length > 32 else 32 if length > 16 else 16 if length > 8 else 8) + "_t"

def fix_var_name_characters(name, prefix = "p"):
    if not name[0].isalpha():
        name = prefix + name
    return '_'.join(str(name).replace('-', ' ').replace('(', ' ').replace(')', ' ').replace('/', ' ').replace('.', ' ').split())

def dbc_to_h(dbc_file, h_file, get_millis):
    # Load the DBC file
    db = cantools.database.load_file(dbc_file)

    # Iterate over the messages and signals in the DBC file
    rx_messages = ""
    tx_messages = ""
    with open(h_file, 'w') as file:
        file.write("// Signals\n")
    for message in db.messages:
        multiplexor_signal = None
        multiplexor_signal_str = ""
        signals = []

        #Trim non-alphabetical starting characters from message and signal names
        message.name = fix_var_name_characters(message.name, "m")

        for signal in message.signals:
            signal.name = fix_var_name_characters(signal.name, "s")

        if message.is_multiplexed():
            #Find the multiplexor
            for signal in message.signals:
                if (signal.is_multiplexer):
                    multiplexor_signal = signal
                    multiplexor_signal_str = signal.name + "_Signal_"
                    break

        for signal in message.signals:
            signalString = ""
            data_type = get_data_type(signal.is_signed, signal.length, signal.scale, signal.offset)
            if (signal.choices != None):
                signalString += "enum class " + signal.name + "_Enum : " + data_type + " {\n"
                for value, name in signal.choices.items():
                    signalString += fix_var_name_characters(str(name), "e") + " = " + str(value) + ",\n"
                signalString += "};\n"
            if signal.byte_order == "big_endian":
                byteOrder = "Endian"
                endian = ", ICANSignal::ByteOrder::kBigEndian"
            else:
                byteOrder = ""
                endian = ""
            signalType = "Signed" if signal.is_signed else "Unsigned"
            signalString += "Make" + byteOrder + signalType + "CANSignal(" + (data_type if signal.choices == None else signal.name + "_Enum") + "," + str(signal.start) + "," + str(signal.length) + "," + str(signal.scale) + ("" if isinstance(signal.scale, int) else "f") + "," + str(signal.offset) + endian + ") " + signal.name + "_Signal_{};\n"
            print(signal.name, str(signal.start), str(signal.length))
            with open(h_file, 'a') as file:
                file.write(signalString)
            if message.is_multiplexed():
                if (signal.is_multiplexer):
                    continue
                
                found = False
                if signal.multiplexer_ids == None:
                    for i in range(len(signals)):
                        if signals[i][0] == "true":
                            signals[i].append(signal.name + "_Signal_")
                            found = True
                            continue
                    if not found:
                        signals.append(["true", "0xFFFFFFFFFFFFFFFFul", signal.name + "_Signal_"])
                else:
                    signal_multiplexer_id = signal.multiplexer_ids[0] if multiplexor_signal.choices == None else (
                        multiplexor_signal.name + "_Enum::" + fix_var_name_characters(str(multiplexor_signal.choices[signal.multiplexer_ids[0]]), "e"))
                    for i in range(len(signals)):
                        if signals[i][1] == signal_multiplexer_id:
                            signals[i].append(signal.name + "_Signal_")
                            found = True
                            continue
                    if not found:
                        signals.append(["false", signal_multiplexer_id, signal.name + "_Signal_"])
            else:
                signals.append(signal.name + "_Signal_")
        print(signals)
        if message.is_multiplexed():
            signal_groups_str = ""
            signal_groups = []
            for i in range(len(signals)):
                signal_groups_str += "MultiplexedSignalGroup<" + str(len(signals[i]) - 2) + ("" if multiplexor_signal.choices == None else ", " + multiplexor_signal.name + "_Enum") + "> " + message.name + "_SignalGroup_" + str(i) + "_{" + ', '.join(str(s) for s in signals[i]) + "};\n"
                signal_groups.append(message.name + "_SignalGroup_" + str(i) + "_")
            #MultiplexedCANTXMessage<2, uint8_t> tx_msg{can, 100, 8, 100, tx_multiplexor, tx_signals_0, tx_signals_1};
            rx_message = signal_groups_str
            data_type_str = (get_data_type(multiplexor_signal.is_signed, multiplexor_signal.length, multiplexor_signal.scale, multiplexor_signal.offset) if multiplexor_signal.choices is None else multiplexor_signal.name + "_Enum")
            rx_message += "MultiplexedCANRXMessage<" + str(len(signal_groups)) + ", " + data_type_str + "> " + message.name + "_RX_Message_{can_bus_, 0x" + format(message.frame_id, 'x') + ", " + ((get_millis + ", ") if get_millis != None else "") + multiplexor_signal_str + ", " + ', '.join(signal_groups) + "};\n"
            tx_message = "MultiplexedCANTXMessage<" + str(len(signal_groups)) + ", 0, " + data_type_str + "> " + message.name + "_TX_Message_{can_bus_, 0x" + format(message.frame_id, 'x') + ", " + ("true, " if message.is_extended_frame else "")  + str(message.length) + ", " + ("0" if message.cycle_time == None else str(message.cycle_time)) + ", timer_group_, std::array<" + data_type_str + ", 0>{}, " + multiplexor_signal_str + ", " + ', '.join(signal_groups) +  "};\n"
        else:
            rx_message = "CANRXMessage<" + str(len(signals)) + "> " + message.name + "_RX_Message_{can_bus_, 0x" + format(message.frame_id, 'x') + ", " + ((get_millis + ", ") if get_millis != None else "") + ', '.join(signals) + "};\n"
            tx_message = "CANTXMessage<" + str(len(signals)) + "> " + message.name + "_TX_Message_{can_bus_, 0x" + format(message.frame_id, 'x') + ", " + ("true, " if message.is_extended_frame else "") + str(message.length) + ", " + ("0" if message.cycle_time == None else str(message.cycle_time)) + ", timer_group_, " + ', '.join(signals) + "};\n"
        rx_messages += rx_message
        tx_messages += tx_message
    with open(h_file, 'a') as file:
        file.write("\n// RX Messages\n")
        file.write(rx_messages)
        file.write("\n// TX Messages\n")
        file.write(tx_messages)

if __name__ == "__main__":
    if len(sys.argv) != 3 and len(sys.argv) != 4:
        print("Usage: python dbc_to_h.py <input_dbc_file> <output_h_file> <optional_get_millis>")
        sys.exit(1)

    dbc_file = sys.argv[1]
    h_file = sys.argv[2]
    get_millis = sys.argv[3] if len(sys.argv) == 4 else None

    dbc_to_h(dbc_file, h_file, get_millis)
    print(f"Converted {dbc_file} to {h_file}")
