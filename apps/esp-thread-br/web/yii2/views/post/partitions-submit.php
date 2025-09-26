<?
use app\helpers\PartitionsCsv;

$result = [];

$fsize = PartitionsCsv::savePartitionsCsv($data);
if($fsize > 0) {
	$result['status'] = 'success';
	$result['fsize'] = $fsize;
} else {
	$result['status'] = 'error';
	$result['message'] = 'Error occured!';
}

echo json_encode($result);
