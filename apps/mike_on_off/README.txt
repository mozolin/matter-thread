
OTBR
~~~~
> dataset active -x
| 0e080000000000010000000300000f4a0300001835060004001fffe002080ad80fc8f565fb430708fd36c110b5afda0e05101450a03ab4223e9cf9907f0f548c1145030e68612d7468726561642d646462640102ddbd0410591801aa6198c76746fdc2fc023c97490c0402a0f7f8

> networkkey
| 1450a03ab4223e9cf9907f0f548c1145

> chip-tool pairing code-thread 1234 hex:0e080000000000010000000300000f4a0300001835060004001fffe002080ad80fc8f565fb430708fd36c110b5afda0e05101450a03ab4223e9cf9907f0f548c1145030e68612d7468726561642d646462640102ddbd0410591801aa6198c76746fdc2fc023c97490c0402a0f7f8 34970112332

> router table

> chip-tool onoff toggle 1234 1


EndDevice
~~~~~~~~~
> matter esp factoryreset
> matter esp ot_cli state

> matter onboardingcodes none

> matter esp ot_cli dataset set active 0e080000000000010000000300000f4a0300001835060004001fffe002080ad80fc8f565fb430708fd36c110b5afda0e05101450a03ab4223e9cf9907f0f548c1145030e68612d7468726561642d646462640102ddbd0410591801aa6198c76746fdc2fc023c97490c0402a0f7f8
| Wrong!

> matter esp ot_cli dataset networkkey 1450a03ab4223e9cf9907f0f548c1145
> matter esp ot_cli dataset commit active
> matter esp ot_cli ifconfig up
> matter esp ot_cli thread start

> matter esp ot_cli router table

"matter esp attribute set <endpoint_id> <cluster_id> <attribute_id> <value>"
> matter esp attribute set 0x1 0x6 0x0 1
> matter esp attribute get 0x1 0x6 0x0

> matter esp onoff read on-off 1
