@echo off
set pairingQRCode1=34970112332
set pairingQRCode2=MT:Y.K9042C00KA0648G00

python3 ./SetupPayload.py parse %pairingQRCode1% > SetupPayload-parse1.log

python3 ./SetupPayload.py parse %pairingQRCode2% > SetupPayload-parse2.log

echo should be  : %pairingQRCode1% > SetupPayload-generate.log
echo should be  : %pairingQRCode2% >> SetupPayload-generate.log
echo ----------------------------------- >> SetupPayload-generate.log
python3 ./SetupPayload.py generate -d 3840 -p 20202021 --vendor-id 65521 --product-id 32768 -cf 0 -dm 2 >> SetupPayload-generate.log
