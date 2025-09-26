<?
use app\helpers\PartitionsCsv;
use app\helpers\Settings;

$data = PartitionsCsv::getPartitionsCsvInfo();
?>

<div id="div-get-partitions-info" name="div-get-partitions-info">
	<b>Partitions Info</b><br/>
	<div id="div-over-partitions">

		<table cellpadding="5px" style="padding:0;margin:0;" id="table-partitions">
			<tr>
			  <th>&nbsp;</th>
			  <th>name</th>
			  <th>type</th>
			  <th>subtype</th>
			  <th>size</th>
			  <th>bytes</th>
			</tr>
			<?
			$idx = 0;
			foreach($data['table'] as $row) {
				$cls = ($idx % 2 === 0) ? 'tr1' : 'tr2';
				$cls = 'tr1';
				$isOTA = preg_match("{ota_\d+}si", $row['name']);
				
				$firstCol = '&nbsp;';
				if($isOTA) {
					$checked = ' checked';
					if(substr($row['name'], 0, 1) === '#') {
						$checked = '';
						$row['name'] = substr($row['name'], 1);
						$cls .= ' unchecked';
					} else {
						$cls .= ' checked';
					}
					$firstCol = "<input type=\"checkbox\" class=\"checkbox-ota-input\" name=\"{$row['name']}\" value=\"{$row['sizeBytes']}\"{$checked}/>";
				}
				?>
				<tr class="<?=$cls?>">
				  <td><?=$firstCol?></td>
				  <td><?=$row['name']?></td>
				  <td class="tdc"><?=$row['type']?></td>
				  <td class="tdc"><?=$row['subtype']?></td>
				  <td class="tdr"><?=$row['size']?></td>
				  <td class="tdr"><?=$row['sizeBytes']?></td>
				</tr>
				<?
				$idx++;
			}
			?>
			<tr>
				<th colspan="5">Total size:</th>
				<th class="tdr" id="th-partitions-total-size"><?=$data['total-size']?></th>
			</tr>
		</table>
	</div>
</div>
