import os
import sys
import time
from pathlib import Path

import canopen

SERIAL_PORT = "/dev/cu.usbmodem1101"
CHANNEL = SERIAL_PORT
BITRATE = 100_000
NODE_ID = 0x2A
BIN_PATH = Path("build-sysbuild/template/zephyr/zephyr.signed.bin")

BLOCK_TRANSFER = False
DOWNLOAD_BUFFER_SIZE = 1024
STATUS_TIMEOUT_S = 30.0
BOOTUP_TIMEOUT_S = 20.0
SDO_TIMEOUT_S = 1.0
SDO_RETRIES = 1
CONFIRM_IMAGE = True

H1F50_PROGRAM_DATA = 0x1F50
H1F51_PROGRAM_CTRL = 0x1F51
H1F56_PROGRAM_SWID = 0x1F56
H1F57_FLASH_STATUS = 0x1F57

PROGRAM_CTRL_STOP = 0x00
PROGRAM_CTRL_START = 0x01
PROGRAM_CTRL_CLEAR = 0x03
PROGRAM_CTRL_ZEPHYR_CONFIRM = 0x80


def create_object_dictionary():
    objdict = canopen.objectdictionary.ObjectDictionary()

    arr = canopen.objectdictionary.Array("Program data", H1F50_PROGRAM_DATA)
    var = canopen.objectdictionary.Variable("", H1F50_PROGRAM_DATA, subindex=1)
    var.data_type = canopen.objectdictionary.DOMAIN
    arr.add_member(var)
    objdict.add_object(arr)

    arr = canopen.objectdictionary.Array("Program control", H1F51_PROGRAM_CTRL)
    var = canopen.objectdictionary.Variable("", H1F51_PROGRAM_CTRL, subindex=1)
    var.data_type = canopen.objectdictionary.UNSIGNED8
    arr.add_member(var)
    objdict.add_object(arr)

    arr = canopen.objectdictionary.Array("Program software ID", H1F56_PROGRAM_SWID)
    var = canopen.objectdictionary.Variable("", H1F56_PROGRAM_SWID, subindex=1)
    var.data_type = canopen.objectdictionary.UNSIGNED32
    arr.add_member(var)
    objdict.add_object(arr)

    arr = canopen.objectdictionary.Array("Flash status", H1F57_FLASH_STATUS)
    var = canopen.objectdictionary.Variable("", H1F57_FLASH_STATUS, subindex=1)
    var.data_type = canopen.objectdictionary.UNSIGNED32
    arr.add_member(var)
    objdict.add_object(arr)

    return objdict


def wait_flash_status_ok(flash_sdo, timeout_s):
    end = time.time() + timeout_s
    status = int(flash_sdo.raw)

    while status != 0 and time.time() < end:
        time.sleep(0.1)
        status = int(flash_sdo.raw)

    return status


def main():
    if not BIN_PATH.is_file():
        print(f"Binary not found: {BIN_PATH}")
        return 1

    size = os.path.getsize(BIN_PATH)

    network = canopen.Network()
    network.connect(interface="slcan", channel=CHANNEL, bitrate=BITRATE)

    node = network.add_node(NODE_ID, create_object_dictionary())
    node.sdo.MAX_RETRIES = SDO_RETRIES
    node.sdo.RESPONSE_TIMEOUT = SDO_TIMEOUT_S

    data_sdo = node.sdo[H1F50_PROGRAM_DATA][1]
    ctrl_sdo = node.sdo[H1F51_PROGRAM_CTRL][1]
    swid_sdo = node.sdo[H1F56_PROGRAM_SWID][1]
    flash_sdo = node.sdo[H1F57_FLASH_STATUS][1]

    print(f"Software ID: 0x{int(swid_sdo.raw):08X}")

    node.nmt.state = "PRE-OPERATIONAL"
    time.sleep(0.5)

    ctrl_sdo.raw = PROGRAM_CTRL_STOP
    ctrl_sdo.raw = PROGRAM_CTRL_CLEAR

    status = wait_flash_status_ok(flash_sdo, STATUS_TIMEOUT_S)
    if status != 0:
        print(f"CLEAR failed, flash status=0x{status:08X}")
        network.disconnect()
        return 2

    infile = open(BIN_PATH, "rb")
    outfile = data_sdo.open(
        "wb",
        buffering=DOWNLOAD_BUFFER_SIZE,
        size=size,
        block_transfer=BLOCK_TRANSFER,
    )

    while True:
        chunk = infile.read(DOWNLOAD_BUFFER_SIZE // 2)
        if not chunk:
            break
        outfile.write(chunk)

    infile.close()
    outfile.close()

    status = wait_flash_status_ok(flash_sdo, STATUS_TIMEOUT_S)
    if status != 0:
        print(f"Download failed, flash status=0x{status:08X}")
        network.disconnect()
        return 3

    print(f"Software ID after download: 0x{int(swid_sdo.raw):08X}")

    ctrl_sdo.raw = PROGRAM_CTRL_START
    node.nmt.wait_for_bootup(timeout=BOOTUP_TIMEOUT_S)

    print(f"Software ID after reboot: 0x{int(swid_sdo.raw):08X}")

    if CONFIRM_IMAGE:
        node.nmt.state = "PRE-OPERATIONAL"
        time.sleep(0.5)
        ctrl_sdo.raw = PROGRAM_CTRL_ZEPHYR_CONFIRM

    network.disconnect()
    return 0


if __name__ == "__main__":
    sys.exit(main())