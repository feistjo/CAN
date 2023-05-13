import cantools
import csv
import sys

def dbc_to_csv(dbc_file, csv_file):
    # Load the DBC file
    db = cantools.database.load_file(dbc_file)

    # Prepare the CSV file
    with open(csv_file, mode='w', newline='') as csvfile:
        csv_writer = csv.writer(csvfile)

        # Write the header row
        header = ['Message ID', 'Message Name', 'Sender', 'Signal Name', 'Start Bit', 'Size', 'Factor', 'Offset', 'Min', 'Max', 'Unit', 'Cycle Time']
        csv_writer.writerow(header)

        # Iterate over the messages and signals in the DBC file
        for message in db.messages:
            for signal in message.signals:
                row = [
                    format(message.frame_id, 'x'),
                    message.name,
                    message.senders,
                    signal.name,
                    signal.start,
                    signal.length,
                    signal.scale,
                    signal.offset,
                    signal.minimum,
                    signal.maximum,
                    signal.unit,
                    message.cycle_time
                ]
                csv_writer.writerow(row)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python dbc_to_csv.py <input_dbc_file> <output_csv_file>")
        sys.exit(1)

    dbc_file = sys.argv[1]
    csv_file = sys.argv[2]

    dbc_to_csv(dbc_file, csv_file)
    print(f"Converted {dbc_file} to {csv_file}")
