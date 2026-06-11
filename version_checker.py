import argparse
import sys
import canopen

DEFAULT_SERIAL_PORT = "/dev/cu.usbmodem101"
DEFAULT_BITRATE = 1_000_000
DEFAULT_NODE_ID = 0x7C
DEFAULT_SDO_TIMEOUT_S = 3.0
DEFAULT_SDO_RETRIES = 3

H100A_SW_VERSION = 0x100A


def parse_args():
    p = argparse.ArgumentParser()
    p.add_argument("--serial-port", default=DEFAULT_SERIAL_PORT)
    p.add_argument("--channel", default=None)
    p.add_argument("--bitrate", type=int, default=DEFAULT_BITRATE)
    p.add_argument("--node-id", type=parse_int, default=DEFAULT_NODE_ID)
    p.add_argument("--sdo-timeout", type=float, default=DEFAULT_SDO_TIMEOUT_S)
    p.add_argument("--sdo-retries", type=int, default=DEFAULT_SDO_RETRIES)
    return p.parse_args()


def parse_int(s):
    return int(s, 0)


def create_object_dictionary():
    objdict = canopen.objectdictionary.ObjectDictionary()
    
    var = canopen.objectdictionary.Variable("Manufacturer software version", H100A_SW_VERSION)
    var.data_type = canopen.objectdictionary.VISIBLE_STRING
    objdict.add_object(var)
    
    return objdict


def main():
    args = parse_args()
    channel = args.channel or args.serial_port

    network = canopen.Network()
    
    try:
        network.connect(interface="slcan", channel=channel, bitrate=args.bitrate)
        
        node = network.add_node(args.node_id, create_object_dictionary())
        node.sdo.MAX_RETRIES = args.sdo_retries
        node.sdo.RESPONSE_TIMEOUT = args.sdo_timeout

        version_data = node.sdo[H100A_SW_VERSION].data        
        if isinstance(version_data, bytes):
            version_str = version_data.decode("utf-8", errors="ignore")
        else:
            version_str = str(version_data)            
        print(version_str.strip("\x00 \r\n"))
        
        network.disconnect()
        return 0

    except Exception as e:
        print(f"ERROR: Failed to read firmware version: {e}", file=sys.stderr)
        try:
            network.disconnect()
        except:
            pass
        return 1


if __name__ == "__main__":
    sys.exit(main())
