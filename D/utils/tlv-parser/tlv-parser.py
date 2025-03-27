#!/bin/env python3

import binascii
import sys

MESHCOP_TLV_TYPE = {
  "CHANNEL": 0,
  "PANID": 1,
  "EXTPANID": 2,
  "NETWORKNAME": 3,
  "PSKC": 4,
  "NETWORKKEY": 5,
  "NETWORK_KEY_SEQUENCE": 6,
  "MESHLOCALPREFIX": 7,
  "STEERING_DATA": 8,
  "BORDER_AGENT_RLOC": 9,
  "COMMISSIONER_ID": 10,
  "COMM_SESSION_ID": 11,
  "SECURITYPOLICY": 12,
  "GET": 13,
  "ACTIVETIMESTAMP": 14,
  "COMMISSIONER_UDP_PORT": 15,
  "STATE": 16,
  "JOINER_DTLS": 17,
  "JOINER_UDP_PORT": 18,
  "JOINER_IID": 19,
  "JOINER_RLOC": 20,
  "JOINER_ROUTER_KEK": 21,
  "PROVISIONING_URL": 32,
  "VENDOR_NAME_TLV": 33,
  "VENDOR_MODEL_TLV": 34,
  "VENDOR_SW_VERSION_TLV": 35,
  "VENDOR_DATA_TLV": 36,
  "VENDOR_STACK_VERSION_TLV": 37,
  "UDP_ENCAPSULATION_TLV": 48,
  "IPV6_ADDRESS_TLV": 49,
  "PENDINGTIMESTAMP": 51,
  "DELAYTIMER": 52,
  "CHANNELMASK": 53,
  "COUNT": 54,
  "PERIOD": 55,
  "SCAN_DURATION": 56,
  "ENERGY_LIST": 57,
  # Seen in a dataset imported through iOS companion app
  "APPLE_TAG_UNKNOWN": 74,
  "DISCOVERYREQUEST": 128,
  "DISCOVERYRESPONSE": 129,
  "JOINERADVERTISEMENT": 241,
}

MESHCOP_TLV_TYPE_NAME = {
  value: name for name, value in MESHCOP_TLV_TYPE.items()
}

def main():
    args = sys.argv[1:]
    data = binascii.a2b_hex(args[0])
    pos = 0
    while pos < len(data):
        tag = data[pos]
        pos += 1
        _len = data[pos]
        pos += 1
        val = data[pos:pos+_len]
        pos += _len
        if tag == 3:
            print("t: %2s (%s), l: %s, v: %s" % (tag, MESHCOP_TLV_TYPE_NAME[tag], _len, val))
        else:
            print("t: %2s (%s), l: %s, v: 0x%s" % (tag,
                                                   MESHCOP_TLV_TYPE_NAME[tag],
                                                   _len, val.hex()))


if __name__ == "__main__":
    main()