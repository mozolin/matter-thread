<?
require("PhpHtmlCssJsMinifier.php");

$minList = [
	"../index.html" => "../index.min.html",
	"../static/restful.js" => "../static/restful.min.js",
	"../static/style.css" => "../static/style.min.css",
];

$minify = new PhpHtmlCssJsMinifier();

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
			$result = $minify->minify_js($contents);
			$flagReady = true;
		}
		if($flagReady) {
			file_put_contents($fileDst, $result);
			if(file_exists($fileDst)) {
				$pathInfo = pathinfo($fileDst);
				echo $pathInfo["basename"].": ".filesize($fileSrc)."/".filesize($fileDst)."\n";
			}
		}
	}
}
