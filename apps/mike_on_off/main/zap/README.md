
# Install ZAP (ZAP-CLI), ZCL Advanced Platform
~~~
git clone --recursive https://github.com/espressif/esp-matter.git
cd esp-matter
./install.sh
source ./export.sh
zap-cli --version
~~~

# Add custom cluster
.../connectedhomeip/src/app/zap-templates/zcl/zcl.json
.../connectedhomeip/src/app/zap-templates/zcl/zcl-with-test-extensions.json

1) add to "xmlRoot": "/root/esp-matter/examples/mike_on_off/main/zap"
2) add to "xmlFile": "mike_on_off-0xFC01.xml"
3) Run zap_regen_all.py:  
~~~
cd ~/esp-matter/connectedhomeip/connectedhomeip
source ./scripts/activate.sh
./scripts/tools/zap_regen_all.py
~~~

