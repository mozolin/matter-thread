
OTBR
~~~~
> dataset active -x
| 0e080000000000010000000300000f4a0300001235060004001fffe00208dead00beef00cafe0708fd000db800a00000051000112233445566778899aabbccddeeff030e4f70656e5468726561642d455350010212340410104810e2315100afd6bc9215a6bfac530c0402a0f7f8

> networkkey
| ef368a6f80e47d821f3a8d70b17a0ef5

> chip-tool pairing code-thread 1234 hex:0e080000000000010000000300000f35060004001fffe00208dead00beef00cafe0708fd000db800a00000051000112233445566778899aabbccddeeff030e4f70656e5468726561642d455350010212340410104810e2315100afd6bc9215a6bfac530c0402a0f7f8 34970112332

> router table

> chip-tool onoff toggle 1234 1


EndDevice
~~~~~~~~~
> matter esp factoryreset
> matter esp ot_cli state

> matter onboardingcodes none

> matter esp ot_cli dataset set active 0e080000000000010000000300000f4a0300001835060004001fffe002080ad80fc8f565fb430708fd36c110b5afda0e05101450a03ab4223e9cf9907f0f548c1145030e68612d7468726561642d646462640102ddbd0410591801aa6198c76746fdc2fc023c97490c0402a0f7f8
| Wrong!

After TLV-PARSER:
> matter esp ot_cli dataset set active 0e080000000000010000000300000f35060004001fffe00208dead00beef00cafe0708fd000db800a00000051000112233445566778899aabbccddeeff030e4f70656e5468726561642d455350010212340410104810e2315100afd6bc9215a6bfac530c0402a0f7f8
| Success!


> matter esp ot_cli dataset networkkey ef368a6f80e47d821f3a8d70b17a0ef5
> matter esp ot_cli dataset commit active
> matter esp ot_cli ifconfig up
> matter esp ot_cli thread start

> matter esp ot_cli router table

"matter esp attribute set <endpoint_id> <cluster_id> <attribute_id> <value>"
> matter esp attribute set 0x1 0x6 0x0 1
> matter esp attribute set 0x6 0x6 0x0 1
> matter esp attribute set 0x8 0x6 0x0 1

> matter esp attribute get 0x1 0x6 0x0

> matter esp onoff read on-off 1


-----------------------------------------------
- Commissioning window opened
- Commissioning session started
- Commissioning window opened
- Fabric is updated
- Fabric is committed
- Fabric is committed
- Commissioning complete
- Commissioning window closed
- BLE deinitialized and memory reclaimed


RLOC16


0xd400
012345 6 789012345
ROUTER   CHILD
110101 0 000000000


1234



RLOC16:
D400 -> 110101(53) 0 000000000 - OTBR
4000 -> 010000(16) 0 000000000 - ESP32H2 (chip-tool)
3000 -> 001100(12) 0 000000000 - ESP32H2 (dataset)
