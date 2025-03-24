# How to setup and work with Matter Controller
https://docs.espressif.com/projects/esp-matter/en/latest/esp32/developing.html#matter-controller
esp-matter/examples/controller
https://github.com/espressif/esp-matter/tree/main/examples/controller
See: 
  - esp-matter_examples_controller.md
  - sdkconfig.defaults

# 1. Build

The sdkconfig file sdkconfig.defaults.otbr is provided to enable the OTBR feature on the controller.
~~~
cd /root/esp-matter/examples/controller
idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults.otbr" set-target esp32s3 build
idf.py -p /dev/ttyACM0 erase-flash flash monitor
~~

# 2. Init and Start the Thread network

Connect the controller to Wi-Fi network with the device console
~~~
matter esp wifi connect {ssid} {password}
matter esp wifi connect MIKE_WIFI_24 *********
matter esp wifi connect MIKE_REDMI_NOTE_13 *********
~~~

Initializing a new Thread network dataset and commit it as the active one
~~~
matter esp ot_cli dataset init new
matter esp ot_cli dataset commit active
~~~

Getting the operational dataset TLV-encoded string. The <dataset_tlvs> will be printed.
~~~
matter esp ot_cli dataset active -x
~~~
<dataset_tlvs>
0e080000000000010000000300000d4a0300001735060004001fffe002086918ac4cec261a970708fd716900bdecc341051061493dd6445a4897f8f07bbcf80022c2030f4f70656e5468726561642d646433340102dd340410abb2f35da1d8108f9278de52b687e99f0c0402a0f7f8
OFFC: 0e08000000000001000000030000144a0300001435060004001fffe002080a9f6e962bfaf9880708fdded0b37370c3f60510b7078e7ab3c4c3624b10968aa9338f2e030f4f70656e5468726561642d613537300102a5700410f88938966f178b3876174a63b8639a220c0402a0f7f8

Starting the Thread network
~~~
matter esp ot_cli ifconfig up
matter esp ot_cli thread start
~~~

# 3. Pair and Control

Pairing the Thread end-device
*Syntax: matter esp controller pairing code-thread <nodeid> <dataset> <payload>*
- <node_id> : randomly chosen value
- <payload> : matter QR code

~~~
matter esp controller pairing ble-thread 1234 <dataset_tlvs> 34970112332
~~~

Control the Thread end-device on the device console (On/Off cluster Toggle command)
~~~
matter esp controller invoke-cmd 1234 1 6 2
~~~

*Example:*
~~~
matter esp controller pairing code-thread 1234 0e08000000000001000000030000174a0300000f35060004001fffe002088fbedf39fa8516870708fd39593d4bdb6a430510ed823d77790b1f415530bbef3678ddb0030f4f70656e5468726561642d3131363601021166041062a21d4edaa262bb779fc2c9923516490c0402a0f7f8 34970112332


> chip-tool interactive start
  >>> pairing code-thread 1234 0e08000000000001000000030000144a0300001435060004001fffe002080a9f6e962bfaf9880708fdded0b37370c3f60510b7078e7ab3c4c3624b10968aa9338f2e030f4f70656e5468726561642d613537300102a5700410f88938966f178b3876174a63b8639a220c0402a0f7f8 34970112332
  >>> pairing code-thread 2345 0e08000000000001000000030000144a0300001435060004001fffe002080a9f6e962bfaf9880708fdded0b37370c3f60510b7078e7ab3c4c3624b10968aa9338f2e030f4f70656e5468726561642d613537300102a5700410f88938966f178b3876174a63b8639a220c0402a0f7f8 34970112332
> matter esp controller pairing code-wifi <node_id> <ssid> <passphrase> <setup_payload>


> matter esp controller read-attr 2345 1 6 0x000

I (4101984) chip[DIS]: Found an existing secure session to [1:0000000000000929]!
I (4101984) chip[DMG]: 0 data version filters provided, 0 not relevant, 0 encoded, 0 skipped due to lack of space
I (4101984) chip[EM]: <<< [E:7501i S:12492 M:165199384] (S) Msg TX from 000000000001B669 to 1:0000000000000929 [E808] [UDP:[FD8A:FD34:7273:1:C318:E218:ADA0:54E2]:5540] --- Type 0001:02 (IM:ReadRequest) (B:51)
I (4101984) chip[EM]: ??1 [E:7501i S:12492 M:165199384] (S) Msg Retransmission to 1:0000000000000929 in 1593ms [State:Idle II:800 AI:800 AT:4000]
Done
> I (4102054) chip[EM]: >>> [E:7501i S:12492 M:247029994 (Ack:165199384)] (S) Msg RX from 1:0000000000000929 [E808] to 000000000001B669 --- Type 0001:05 (IM:ReportData) (B:69)
I (4102064) chip[TOO]: Endpoint: 1 Cluster: 0x0000_0006 Attribute 0x0000_0000 DataVersion: 1171748968
                         ~~~~~~~~~~~
I (4102064) chip[TOO]:   OnOff: TRUE
                         ~~~~~~~~~~~
I (4102064) read_command: read done
I (4102064) chip[EM]: <<< [E:7501i S:12492 M:165199385 (Ack:247029994)] (S) Msg TX from 000000000001B669 to 1:0000000000000929 [E808] [UDP:[FD8A:FD34:7273:1:C318:E218:ADA0:54E2]:5540] --- Type 0000:10 (SecureChannel:StandaloneAck) (B:34)


OFFC#1: matter esp controller pairing code-thread 1234 0e08000000000001000000030000144a0300001435060004001fffe002080a9f6e962bfaf9880708fdded0b37370c3f60510b7078e7ab3c4c3624b10968aa9338f2e030f4f70656e5468726561642d613537300102a5700410f88938966f178b3876174a63b8639a220c0402a0f7f8 34970112332
OFFC#2: matter esp controller pairing code-thread 2345 0e08000000000001000000030000144a0300001435060004001fffe002080a9f6e962bfaf9880708fdded0b37370c3f60510b7078e7ab3c4c3624b10968aa9338f2e030f4f70656e5468726561642d613537300102a5700410f88938966f178b3876174a63b8639a220c0402a0f7f8 34970112332
OFFC#3: matter esp controller pairing code-thread 1 0e08000000000001000000030000144a0300001435060004001fffe002080a9f6e962bfaf9880708fdded0b37370c3f60510b7078e7ab3c4c3624b10968aa9338f2e030f4f70656e5468726561642d613537300102a5700410f88938966f178b3876174a63b8639a220c0402a0f7f8 34970112332


matter esp controller pairing unpair 1234

matter esp controller read-attr 1234 1 6 0x004003
matter esp controller write-attr 1234 1 6 0x4003 "{\"0:U8\": 2}"

matter esp controller read-attr 1234 1 6 0x000
matter esp controller read-attr 2345 1 6 0x000
matter esp controller write-attr 1234 1 6 0x000 "{\"0:U8\": 0}"
matter esp controller write-attr 1234 1 6 0x000 "{\"0:bool\": \"false\"}"
~~~

matter esp controller pairing ble-thread 1515 0e08000000000001000000030000174a0300000f35060004001fffe002088fbedf39fa8516870708fd39593d4bdb6a430510ed823d77790b1f415530bbef3678ddb0030f4f70656e5468726561642d3131363601021166041062a21d4edaa262bb779fc2c9923516490c0402a0f7f8 20202021 3840




Thread End Device:
> matter esp ot_cli factoryreset
> matter esp ot_cli onboardingcodes none
matter onboardingcodes none
