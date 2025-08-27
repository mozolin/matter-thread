<?
//-- the best with Html and Css
require("PhpHtmlCssJsMinifier.php");
//-- the best with Js
require("class.JavaScriptPacker.php");;

$minList = [
  "frontend/index.html"        => "frontend/index.min.html",
  "frontend/static/restful.js" => "frontend/static/restful.min.js",
  "frontend/static/style.css"  => "frontend/static/style.min.css",
];

$minify = new PhpHtmlCssJsMinifier();

echo str_repeat("-", 45)."\n";
echo "| File name         |    Source |  Minified |\n";
echo str_repeat("-", 45)."\n";

foreach($minList as $fileSrc => $fileDst) {
  if(file_exists($fileDst)) {
    @unlink($fileDst);
  }
  if(file_exists($fileSrc)) {
    $contents = file_get_contents($fileSrc);
    $pathInfo = pathinfo($fileSrc);
    
    $flagReady = false;
    if($pathInfo["extension"] === "html") {
      $result = $minify->minify_html($contents);
      $flagReady = true;
      
    }
    if($pathInfo["extension"] === "css") {
      $result = $minify->minify_css($contents);
      $flagReady = true;
    }
    if($pathInfo["extension"] === "js") {
      $result = (new JavaScriptPacker($contents, 'Normal', true, false))->pack();
      $flagReady = true;
    }
    if($flagReady) {
      file_put_contents($fileDst, $result);
      if(file_exists($fileDst)) {
        $pathInfo = pathinfo($fileDst);
        
        $fn = str_pad($pathInfo["basename"], 17, " ", STR_PAD_RIGHT);
        $size1 = str_pad(filesize($fileSrc), 9, " ", STR_PAD_LEFT);
        $size2 = str_pad(filesize($fileDst), 9, " ", STR_PAD_LEFT);
        
        echo "| ".$fn." | ".$size1." | ".$size2." |\n";
      }
    }
  }
}
echo str_repeat("-", 45)."\n";
