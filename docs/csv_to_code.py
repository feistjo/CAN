import pandas as pd

df = pd.read_csv("full_bus.csv")

def generateSignals():
    for index, signal in df.iterrows():
        signal["ByteOrder"] = "Intel" # assume intel since csv doesn't show byteorder
        signal["Type"] = "Unsigned" # assume signed/unsigned since csv doesn't show type
        signalString = ""
        byteOrder = "Endian" if signal["ByteOrder"] == "Motorola" else ""
        signalType = "Signed" if signal["Type"] == "Signed" else "Unsigned"
        signalString = "Make" + byteOrder + signalType + "CANSignal(dataType," + str(signal["Start Bit"]) + "," + str(signal["Size"]) + "," + str(signal["Factor"]) + "," + str(signal["Offset"]) + ") " + signal["Signal Name"] + "_Signal{};\n"
        with open('can_code.txt', 'a') as file:
            file.write(signalString)

def generateRXMessages():
    num_rows = len(df)
    index = 0
    prevId = None
    signals = []
    while index < num_rows:
        signal_name = df.at[index, 'Signal Name'] + "_Signal"
        currentId = df.at[index, 'Message ID']
        if prevId == currentId:
            # add to message
            signals.append(signal_name)
            # if next id is another address, write
            if currentId != df.at[index+1, 'Message ID']:
                rx_message = "CANRXMessage<" + str(len(signals)) + "> " + df.at[index, "Message Name"] + "_RX_Message{can_bus, 0x" + str(prevId) + ", " + ', '.join(signals) + "};\n"
                with open('can_code.txt', 'a') as file:
                    file.write(rx_message)
        else:
            # new message
            signals = []
            signals.append(signal_name)
        prevId = currentId
        index += 1

def generateTXMessages():
    num_rows = len(df)
    index = 0
    prevId = None
    signals = []
    while index < num_rows:
        signal_name = df.at[index, 'Signal Name'] + "_Signal"
        currentId = df.at[index, 'Message ID']
        if prevId == currentId:
            # add to message
            signals.append(signal_name)
            # if next id is another address, write
            if currentId != df.at[index+1, 'Message ID']:
                # leaving DLC and timer frequency as placeholders
                rx_message = "CANTXMessage<" + str(len(signals)) + "> " + df.at[index, "Message Name"] + "_TX_Message{can_bus, 0x" + str(prevId) + ", DLC, Freq, timer_group, " + ', '.join(signals) + "};\n"
                with open('can_code.txt', 'a') as file:
                    file.write(rx_message)
        else:
            # new message
            signals = []
            signals.append(signal_name)
        prevId = currentId
        index += 1

generateSignals()
generateRXMessages()
generateTXMessages()