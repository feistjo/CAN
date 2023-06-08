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
        signals = []
        for signal in message.signals:
            signalString = ""
            if signal.byte_order == "big_endian":
                byteOrder = "Endian"
                endian = ", ICANSignal::ByteOrder::kBigEndian"
            else:
                byteOrder = ""
                endian = ""
            signalType = "Signed" if signal.is_signed else "Unsigned"
            signalString = "Make" + byteOrder + signalType + "CANSignal(data_type_placeholder," + str(signal.start) + "," + str(signal.length) + "," + str(signal.scale) + "," + str(signal.offset) + endian + ") " + signal.name + "_Signal{};\n"
            with open(h_file, 'a') as file:
                file.write(signalString)
            signals.append(signal.name + "_Signal")
        rx_message = "CANRXMessage<" + str(len(signals)) + "> " + message.name + "_RX_Message{can_bus, 0x" + format(message.frame_id, 'x') + ", " + ', '.join(signals) + "};\n"
        rx_messages += rx_message
        tx_message = "CANTXMessage<" + str(len(signals)) + "> " + message.name + "_TX_Message{can_bus, 0x" + format(message.frame_id, 'x') + ", " + str(message.length) + ", freq_placeholder, timer_group, " + ', '.join(signals) + "};\n"
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