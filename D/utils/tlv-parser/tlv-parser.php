<?

$MESHCOP_TLV_TYPE = [
    0 => "CHANNEL",
    1 => "PANID",
    2 => "EXTPANID",
    3 => "NETWORKNAME",
    4 => "PSKC",
    5 => "NETWORKKEY",
    6 => "NETWORK_KEY_SEQUENCE",
    7 => "MESHLOCALPREFIX",
    8 => "STEERING_DATA",
    9 => "BORDER_AGENT_RLOC",
   10 => "COMMISSIONER_ID",
   11 => "COMM_SESSION_ID",
   12 => "SECURITYPOLICY",
   13 => "GET",
   14 => "ACTIVETIMESTAMP",
   15 => "COMMISSIONER_UDP_PORT",
   16 => "STATE",
   17 => "JOINER_DTLS",
   18 => "JOINER_UDP_PORT",
   19 => "JOINER_IID",
   20 => "JOINER_RLOC",
   21 => "JOINER_ROUTER_KEK",
   32 => "PROVISIONING_URL",
   33 => "VENDOR_NAME_TLV",
   34 => "VENDOR_MODEL_TLV",
   35 => "VENDOR_SW_VERSION_TLV",
   36 => "VENDOR_DATA_TLV",
   37 => "VENDOR_STACK_VERSION_TLV",
   48 => "UDP_ENCAPSULATION_TLV",
   49 => "IPV6_ADDRESS_TLV",
   51 => "PENDINGTIMESTAMP",
   52 => "DELAYTIMER",
   53 => "CHANNELMASK",
   54 => "COUNT",
   55 => "PERIOD",
   56 => "SCAN_DURATION",
   57 => "ENERGY_LIST",
   #-- Seen in a dataset imported through iOS/Android companion app
   74 => "APPLE_TAG_UNKNOWN",
  128 => "DISCOVERYREQUEST",
  129 => "DISCOVERYRESPONSE",
  241 => "JOINERADVERTISEMENT",
];


//-- get argv[1]
$data = "";
if(!empty($argv[1])) {
	$data = $argv[1];
}
//-- get argv[2]
$useAppTags = true;
$sVal = strtolower($argv[2]);
if($sVal === 'false' || $sVal === '0') {
	$useAppTags = false;
}

$removedTags = [];
$newLine = "";

$pos = 0;
while($pos < strlen($data)) {
	$tag = hexdec(substr($data, $pos, 2));
	$pos += 2;
	$len = hexdec(substr($data, $pos, 2));
	$pos += 2;
	$val = substr($data, $pos, $len * 2);
  $pos += $len * 2;

  #-- remove this tag
  if(!$useAppTags && $tag > 57) {
    $removedTags[] = $tag;
  }
  
  if($useAppTags || (!$useAppTags && $tag <= 57)) {
    
    $newLine = createLine($newLine, $tag, $len, $val);
    
    if($tag == 3) {
      #-- NetworkName tag
      $val = "'".hex2bin($val)."'";
    } else {
    	$val = '0x'.$val;
    }

    $sTag = str_pad($tag, 3, ' ', STR_PAD_LEFT);
    echo "t: {$sTag} (".$MESHCOP_TLV_TYPE[$tag]."), l: {$len}, v: {$val}\n";
  }
}

echo "\n-----------------------------------------------------------------\n";
echo "was(".strlen($argv[1])."):".$argv[1]."\n";
echo "new(".strlen($newLine)."):".$newLine."\n";

if(count($removedTags) > 0) {
  echo "\nTag(s) removed: ".count($removedTags).":\n";
  foreach($removedTags as $tag) {
  	echo "- ".$MESHCOP_TLV_TYPE[$tag]." ({$tag})\n";
  }
}

if($useAppTags && $newLine === $argv[1]) {
  echo "\nOK!";
}


function createLine($newLine, $tag, $len, $val)
{
	$newLine .= str_pad(dechex($tag), 2, '0', STR_PAD_LEFT);
  $newLine .= str_pad(dechex($len), 2, '0', STR_PAD_LEFT);
  $newLine .= $val;
  
  return $newLine;
}

exit;

/*
MESHCOP_TLV_TYPE_NAME = {
  value: name for name, value in MESHCOP_TLV_TYPE.items()
}



def createLine(newLine, tag, _len, val):
  newLine += str(hex(tag)).replace("0x", "").zfill(2)
  newLine += str(hex(_len)).replace("0x", "").zfill(2)
  newLine += str(binascii.hexlify(val)).replace("b'", "").replace("'", "").zfill(_len * 2)
  return newLine

def main():
  args = sys.argv[1:]
  data = binascii.a2b_hex(args[0])

  print(data)
  
  removedTags = 0
  
  useAppTags = True
  if str(args[1]) == "false" or str(args[1]) == "0":
    useAppTags = False
  pos = 0

  newLine = ""

  while pos < len(data):
    tag = data[pos]
    pos += 1
    _len = data[pos]
    pos += 1
    val = data[pos:pos+_len]
    pos += _len
    
    #-- remove this tag
    if not useAppTags and tag > 57:
      removedTags += 1
    
    if useAppTags or (not useAppTags and tag <= 57):
      #-- NetworkName tag
      if tag == 3:
        newLine = createLine(newLine, tag, _len, val)
        print("t: %2s (%s), l: %s, v: %s" % (tag, MESHCOP_TLV_TYPE_NAME[tag], _len, val))
      #-- other tags
      else:
        newLine = createLine(newLine, tag, _len, val)
        print("t: %2s (%s), l: %s, v: 0x%s" % (tag, MESHCOP_TLV_TYPE_NAME[tag], _len, val.hex()))

  print("\n-----------------------------------------------------------------\n")
  print("was(" + str(len(args[0])) + "):" + args[0])
  print("new(" + str(len(newLine)) + "):" + newLine)

  if removedTags > 0:
    print("\nTag(s) removed: %i" % removedTags)

  if useAppTags and (newLine == args[0]):
    print("\nOK!")

if __name__ == "__main__":
  main()
*/
