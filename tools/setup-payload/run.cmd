@echo off
set pairingQRCode1=34970112332
set pairingQRCode2=MT:Y.K9042C00KA0648G00

python3 ./SetupPayload.py parse %pairingQRCode1% > result/SetupPayload-parse1.log

python3 ./SetupPayload.py parse %pairingQRCode2% > result/SetupPayload-parse2.log

echo should be  : %pairingQRCode1% > result/SetupPayload-generate.log
echo should be  : %pairingQRCode2% >> result/SetupPayload-generate.log
echo ----------------------------------- >> result/SetupPayload-generate.log
python3 ./SetupPayload.py generate -d 3840 -p 20202021 --vendor-id 65521 --product-id 32768 -cf 0 -dm 2 >> result/SetupPayload-generate.log
