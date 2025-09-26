<?
use app\models\MainForm;
use app\helpers\SdkConfig;
use app\helpers\Settings;

use yii\widgets\ActiveForm;
use yii\widgets\Pjax;

$flashSizes = [2,4,8,16,32];

Pjax::begin([
	'linkSelector' => false,
	'enablePushState' => false,
]);
$form = ActiveForm::begin([
	'options' =>
		[
			'data' => [
				'pjax' => true
			],
			'id' => 'main-form'
		],
	  'enableAjaxValidation' => false,
  	'enableClientValidation' => false,
    'validateOnBlur' => false,
]);
	
/*
$model = new MainForm();
echo $form->field($model, 'search_word')->textInput([
    'placeholder' => 'Placeholder here...',
    'class' => 'form-control'
  ])->label(false);
*/

?>
<h3>Switchable Sections</h3>
<?
$switchableSections = SdkConfig::getSwitchableSectionsList();
foreach($switchableSections as $switchableSectionName => $switchableSection) {
	$status = $switchableSection['status'];
	$checked = ($status === 1) ? ' checked' : '';
	$id = SdkConfig::$SWITCHABLE_SECTIONS.$switchableSectionName;
	?>
	<label for="<?=$id?>">
		<input type="checkbox" id="<?=$id?>" name="<?=$id?>" value="<?=$status?>"<?=$checked?>/>&nbsp;<?=$switchableSectionName?>
	</label><br/>
	<?
	$status = SdkConfig::checkSectionRealStatus($switchableSectionName);
	//echo "<i>REAL STATUS (sdkconfig) = {$status}</i><br/>";
}

?>

<hr/>

<h3>Sections with custom parameters</h3>
<table cellpadding="5px" style="padding:0;margin:0;">
	<?
	$customSections = SdkConfig::getCustomSectionsList();
	foreach($customSections as $customSectionName => $customSection) {
		?>
		<tr>
			<th colspan="2"><b><?=$customSectionName?></b></th>
		</tr>
		<?
		foreach($customSection['params'] as $paramName => $param) {
			$val = $param['value'];
			$defaultValue = $val;
			if(preg_match("{(CONFIG_ESPTOOLPY_FLASHSIZE_)(\d+)(MB)}si", addslashes($paramName), $m)) {
				?>
				<tr>
					<td colspan="2" class="td-word-break">
						<span class="span-small">
							Would you like to use the flash memory value from the ESP32 chip?<br/>
							Use the <span id="span-goto-get-esp32-info" class="span-like-link">Get ESP32 chip information</span> block and select the appropriate COM port, where<br/>
							the ESP32 chip is connected...
							<br/>
							To check, if the ESP32 has enough flash memory, you can use the <span id="span-goto-partitions-info" class="span-like-link">Partitions Info</span> with<br/>
							the ability to enable/disable OTA blocks.
						</span>
					</td>
				</tr>
				<tr>
					<td width="200px">
						<?
						$paramName = $m[1].'<span id="span-current-flash-size">'.$m[2].'</span>'.$m[3];
						
						$val = $m[2];
						$sdkConfigValue = SdkConfig::checkParameterRealStatus($paramName);
						if(!is_null($sdkConfigValue)) {
							$val = $sdkConfigValue;
							$paramName = $m[1].'<span id="span-current-flash-size">'.$val.'</span>'.$m[3];
						}
						?>
						<select id="select-config-esptoolpy-flashsize" name="CONFIG_ESPTOOLPY_FLASHSIZE">
							<?
							foreach($flashSizes as $flashSize) {
								$selected = ($flashSize == $val) ? ' selected' : '';
								?>
								<option value="<?=$flashSize?>"<?=$selected?>><?=$flashSize?>MB</option>
								<?
							}
							?>
						</select>
						<div id="div-esp32-chip-info">
							<span id="span-esp32-flash-size"></span>
							<button id="button-esp32-set-ram" class="btn btn-primary">Set</button>
						</div>
					</td>
					<td><?=$paramName?></td>
				</tr>
				<tr>
					<td colspan="2" class="td-word-break">
						<?
						echo \Yii::$app->view->renderFile("@app/views/select-com-port.php");
						?>
					</td>
				</tr>
				<tr>
					<td colspan="2" class="td-word-break">
						<?
						echo \Yii::$app->view->renderFile("@app/views/get-partitions-info.php");
						?>
					</td>
				</tr>
				<?
			} else {
				?>
				<tr>
					<td>
						<?
						$sdkConfigValue = SdkConfig::checkParameterRealStatus($paramName);
						if(!is_null($sdkConfigValue)) {
							$val = $sdkConfigValue;
						}
						?>
						<input type="text" value="<?=$val?>" name="<?=$paramName?>" placeholder="<?=$defaultValue?>"/>
					</td>
					<td><?=$paramName?></td>
				</tr>
				<?
			}
		}
		?>
		<tr>
			<td colspan="2">&nbsp;</td>
		</tr>
		<?
	}
	?>
</table>
<button id="button-form-submit" class="btn btn-primary">Save <?=Settings::$_FILE_SDKCONFIG?></button>
<button id="button-partitions-submit" class="btn btn-primary">Save <?=Settings::$_FILE_PARTITIONS?></button>
<?

ActiveForm::end();
Pjax::end();
