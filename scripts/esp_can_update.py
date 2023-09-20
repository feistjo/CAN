import requests
import sys
import hashlib
import time
import math
from os.path import basename

Import("env")

try:
    import can
    import cantools
    from tqdm import tqdm
except ImportError:
    env.Execute("$PYTHONEXE -m pip install can")
    env.Execute("$PYTHONEXE -m pip install cantools")
    env.Execute("$PYTHONEXE -m pip install tqdm")
    import can
    import cantools
    from tqdm import tqdm

try:
    import configparser
except ImportError:
    import ConfigParser as configparser
# project_config = configparser.ConfigParser()
# project_config.read("platformio.ini")
# can_update_config = {k: v for k, v in project_config.items("can_update")}
can_update_config = env.GetProjectConfig().items("can_update", as_dict=True)
db = cantools.database.load_file("esp_can_update.dbc")


def on_upload(source, target, env):
    firmware_path = str(source[0])

    with open(firmware_path, "rb") as firmware:
        firmware_bytes = firmware.read()
        md5 = hashlib.md5(firmware_bytes).digest()
        print(hashlib.md5(firmware_bytes).hexdigest())
        firmware.seek(0)

        data_message = db.get_message_by_name("update_data_message")
        data_message.frame_id = int(can_update_config.get("update_message_id"), 0)
        progress_message = db.get_message_by_name("update_progress_message")
        progress_message.frame_id = data_message.frame_id + 1

        can_bus = can.interface.Bus(
            "can0",
            bustype="socketcan",
            bitrate=int(can_update_config.get("update_baud")),
        )
        if can_bus is None:
            env.Execute(
                "sudo ip link set up can0 type can bitrate "
                + can_update_config.get("update_baud")
            )
            can_bus = can.interface.Bus(
                "can0",
                bustype="socketcan",
                bitrate=int(can_update_config.get("update_baud")),
            )

        try:
            data_message_data = data_message.encode(
                {
                    "message_type": 2,
                    "update_md5": md5[i * 4]
                    + (md5[(i * 4) + 1] << 8)
                    + (md5[(i * 4) + 2] << 16)
                    + (md5[(i * 4) + 3] << 24),
                    "update_md5_idx": i,
                }
            )
            can_bus.send(
                can.Message(
                    arbitration_id=data_message.frame_id, data=data_message_data
                )
            )
        except:
            env.Execute(
                "sudo ip link set up can0 type can bitrate "
                + can_update_config.get("update_baud")
            )
            can_bus = can.interface.Bus(
                "can0",
                bustype="socketcan",
                bitrate=int(can_update_config.get("update_baud")),
            )

        received_md5 = False
        while not received_md5:
            for i in range(4):
                data_message_data = data_message.encode(
                    {
                        "message_type": 2,
                        "update_md5": md5[i * 4]
                        + (md5[(i * 4) + 1] << 8)
                        + (md5[(i * 4) + 2] << 16)
                        + (md5[(i * 4) + 3] << 24),
                        "update_md5_idx": i,
                    }
                )
                can_bus.send(
                    can.Message(
                        arbitration_id=data_message.frame_id, data=data_message_data
                    )
                )
                time.sleep(0.02)
            msg = can_bus.recv(0.1)
            if (msg != None) and msg.arbitration_id == progress_message.frame_id:
                received_progress_msg = db.decode_message(
                    "update_progress_message", msg.data
                )
                if received_progress_msg["received_md5"]:
                    received_md5 = True
            while msg is not None:
                msg = can_bus.recv(0.00001)

        data_message_data = data_message.encode(
            {"message_type": 0, "update_length": len(firmware_bytes)}
        )
        received_len = False

        while not received_len:
            can_bus.send(
                can.Message(
                    arbitration_id=data_message.frame_id, data=data_message_data
                )
            )
            time.sleep(0.01)
            msg = can_bus.recv(0.05)
            if (msg != None) and msg.arbitration_id == progress_message.frame_id:
                received_progress_msg = db.decode_message(
                    "update_progress_message", msg.data
                )
                if received_progress_msg["received_len"]:
                    received_len = True
            while msg is not None:
                msg = can_bus.recv(0.00001)

        bar = tqdm(
            desc="Upload Progress",
            total=len(firmware_bytes),
            dynamic_ncols=True,
            unit="B",
            unit_scale=True,
        )

        for i in range(math.floor((len(firmware_bytes) + 3) / 4)):
            current_bytes_written = False
            while not current_bytes_written:
                data_message_data = data_message.encode(
                    {
                        "message_type": 1,
                        "data_block_index": i,
                        "update_data": firmware_bytes[i * 4]
                        + (firmware_bytes[(i * 4) + 1] << 8)
                        + (firmware_bytes[(i * 4) + 2] << 16)
                        + (firmware_bytes[(i * 4) + 3] << 24),
                    }
                )
                can_bus.send(
                    can.Message(
                        arbitration_id=data_message.frame_id, data=data_message_data
                    )
                )

                msg = can_bus.recv(0.1)
                while msg is not None:
                    if msg.arbitration_id == progress_message.frame_id:
                        received_progress_msg = db.decode_message(
                            "update_progress_message", msg.data
                        )
                        # print(db.decode_message("update_progress_message", data_message_data))
                        # print(i)
                        if (
                            received_progress_msg["written"] == True
                            and received_progress_msg["update_block_idx"] == i
                        ) or (received_progress_msg["update_block_idx"] == i + 1):
                            current_bytes_written = True
                    msg = can_bus.recv(0.00001)

            if i % 1024 == 0:
                bar.update(4096)
                # time.sleep(0.1)

        can_bus.shutdown()

try:
    if env.GetProjectOption("upload_can") == "y":
        DefaultEnvironment().Replace(UPLOADCMD=on_upload)
except:
    pass
