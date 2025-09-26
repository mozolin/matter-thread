$(function() {

	comPortSelectInit();
	flashSizeSelectInit();
	getFlashIdInit();
	gotoGetInfoSectionInit();
	formHandler();
	partitionsTableInit();

});


const statusObj = $('#div-status');
const comPortsListSelectObj = $('#select-com-ports-list');
const esptoolFlashIdButtonObj = $('#button-esptool-flash-id');
const esp32FlashSizeSpanObj = $('#span-esp32-flash-size');
const esp32ChipInfoDivObj = $('#div-esp32-chip-info');
const gotoEsp32ChipInfoSpanObj = $('#span-goto-get-esp32-info');
const cfgEspToolPyFlashSizeSelectObj = $('#select-config-esptoolpy-flashsize');
const currentFlashSizeSpanObj = $('#span-current-flash-size');
const esp32SetRAMButtonObj = $('#button-esp32-set-ram');
const newFlashSizeSpanObj = $('#span-new-flash-size');
const formSubmitButtonObj = $('#button-form-submit');
const otaInputCheckboxObj = $('.checkbox-ota-input');
const partitionsTotalSizeThObj = $('#th-partitions-total-size');
const partitionsSubmitButtonObj = $('#button-partitions-submit');
const gotoPartitionsInfoSpanObj = $('#span-goto-partitions-info');



let comPortSelectedInt = null;
const defSubmitMsg = 'Save '+$_FILE_SDKCONFIG;
const errSubmitMsg = 'Error saving file '+$_FILE_SDKCONFIG;
const switchableSubmitMsg = 'You must select at least one of the switchable sections';
const savedSubmitMsg = 'Saved '+$_FILE_SDKCONFIG+': [NUM-BYTES] bytes';

const defPartitionsMsg = 'Save '+$_FILE_PARTITIONS;
const errPartitionsMsg = 'Error saving file '+$_FILE_PARTITIONS;
const savedPartitionsMsg = 'Saved '+$_FILE_PARTITIONS+': [NUM-BYTES] bytes';

//-- choose COM port
const comPortSelectInit = () => {
	//-- onInit
	setComPortSelected();
	comPortsListSelectObj.change(() => {
  	//-- onChange
  	setComPortSelected();
	});
};


const flashSizeSelectInit = () => {
	partitionsSizeChange();
	//-- choose flash size
	cfgEspToolPyFlashSizeSelectObj.change(() => {
  	setFlashSizeSelected();
  	partitionsSizeChange();
	});
	//-- set new flash size
	esp32SetRAMButtonObj.click((e) => {
  	e.preventDefault();

  	const newFS = $('#span-new-flash-size').text();
  	//-- set for parameter name
  	currentFlashSizeSpanObj.html(newFS);
  	//-- set for SELECT
  	cfgEspToolPyFlashSizeSelectObj.val(newFS);
		partitionsSizeChange();
	});
};


const setComPortSelected = () => {
	const v = comPortsListSelectObj.find(":selected").val();
	if(typeof(v) != 'undefined' && v != null) {
		comPortSelectedInt = parseInt(v);
	}
}

const setFlashSizeSelected = () => {
	const v = cfgEspToolPyFlashSizeSelectObj.find(":selected").val();
	if(typeof(v) != 'undefined' && v != null) {
		currentFlashSizeSpanObj.html(v);
	}
}

//-- esptool -p COMx flash-id
const getFlashIdInit = () => {
  esptoolFlashIdButtonObj.click((e) => {
  	e.preventDefault();
  	
  	if(typeof(comPortSelectedInt) != 'undefined' && comPortSelectedInt != null) {
			
			$.ajax({
      	type: 'post',
			  url: '/post/esptool-flash-id',
			  data: {'com-port': comPortSelectedInt},
			  dataType: 'json',
			  success: function(result) {
			    esptoolFlashIdButtonObj.show();
			    if(isValidJSON(result)) {
			      const data = JSON.parse(result);
			      if(typeof(data.port) != 'undefined') {
			      	if(typeof(data.chip) == 'undefined') {
			      		setStatus('The COM'+data.port+' port is invalid!', 'error', 10000);
        			} else {
        				setStatus(data.chip+' in COM'+data.port+' with '+data.ram+'MB flash RAM', 'success');
        				esp32ChipInfoDivObj.show();
        				esp32FlashSizeSpanObj.html(data.chip+': <span id="span-new-flash-size">'+data.ram+'</span>MB');
        				//-- re-init functions
        				//flashSizeSelectInit();
        			}
			      } else {
			      	setStatus('Unknown error!', 'error', 10000);
			      }
			    } else {
			    	//console.error(result);
			    }
			  },
			  error: function(result) {
			    esptoolFlashIdButtonObj.show();
			    //console.error(result);
			  },
			});

			esptoolFlashIdButtonObj.hide();
			setStatus('esptool -p COM'+comPortSelectedInt+' flash-id...');
  	}
  });

  statusObj.click((e) => {
    e.preventDefault();
    
    statusObj.hide();
    //esptoolFlashIdButtonObj.show();
  });
};

const changeClass = (obj, cls) => {
  obj.removeClass();
  if(typeof(cls) != 'undefined') {
		obj.addClass(cls);
	}
};

const setStatus = (txt, cls, tm) => {
  statusObj.html('&nbsp;'+ txt +'&nbsp;').show();
  changeClass(statusObj, cls);
  if(typeof(tm) != 'undefined') {
  	//-- hide status in "tm" ms
		setTimeout(() => {
			//-- clear class
			changeClass(statusObj);
			//-- hide
			statusObj.hide();
	  }, tm);
  }
};

const gotoGetInfoSectionInit = () => {
  gotoEsp32ChipInfoSpanObj.click((e) => {
  	e.preventDefault();

  	//scrollToAnchor('get-esp32-chip-info', 'div', 500, 0);
  	blinkElement('#div-get-esp32-chip-info', {
   	 //color: '#17a2b8',
   	 color: '#666',
	    duration: 3000,
	    blinkSpeed: 300
		});
  });

  gotoPartitionsInfoSpanObj.click((e) => {
  	e.preventDefault();

  	//scrollToAnchor('div-get-partitions-info', 'div', 500, 0);
  	blinkElement('#div-get-partitions-info', {
   	 //color: '#17a2b8',
   	 color: '#666',
	    duration: 3000,
	    blinkSpeed: 300
		});
  });
};

const scrollToAnchor = (aid, tag, time, offset) => {
  if(typeof(offset) == 'undefined') {
  	offset = 0;
  }
  const aTag = $(tag+"[name='"+ aid +"']");
  time = (typeof(time) === 'undefined') ? 2000 : time;
  try {
  	$('html, body').stop().animate({
	    scrollTop: aTag.offset().top - offset
	  }, time);
  } catch(e) {}
};

const formHandler = () => {
	formSubmitButtonObj.click((e) => {
		e.preventDefault();
		
		setSubmitButton('', 'dark', true);
		
		//formSubmitButtonObj.removeClass();
		//formSubmitButtonObj.addClass('btn btn-dark');
		//formSubmitButtonObj.prop('disabled', true);
		
		const formData = {};
		let switchableSectionsFound = false;
		$('form').find('input, textearea, select').each(function() {
	  	if(this.type === 'checkbox') {
	  		formData[this.name] = $(this).prop('checked') ? 1 : 0;
	  		switchableSectionsFound = true;
	  	} else {
	  		formData[this.name] = $(this).val();
	  	}
		});
		if(!switchableSectionsFound) {
			setSubmitButton(switchableSubmitMsg, 'danger');
		  return false;
		}

		$.ajax({
    	type: 'post',
		  url: '/post/form-submit',
		  data: formData,
		  dataType: 'json',
		  success: function(result) {
		  	if(isValidJSON(result)) {
		  		const data = JSON.parse(result);
		  		if(data.status == 'success') {
		      	const msg = savedSubmitMsg.replace('[NUM-BYTES]', data.fsize);
		      	setSubmitButton(msg, 'success');
		      	return true;
		      } else {
		      	if(typeof(data.message) != 'undefined') {
		      		setSubmitButton(data.message, 'danger');
		      		return true;
		      	}
		      }
		    }
		    setSubmitButton(errSubmitMsg, 'danger');
		  },
		  error: function(result) {
		    //console.error(result);
		    setSubmitButton(errSubmitMsg, 'danger');
		  },
		});
	});
	/*
	var inputValue = $('#myInputField').val();
	var checkboxValue = $('#myCheckbox').prop('checked'); // For checked state of checkbox
	var radioValue = $('input[name="myRadioGroup"]:checked').val(); // For checked radio button
	*/
}

const isValidJSON = (str) => {
  try {
    JSON.parse(str);
    return true;
  } catch (e) {}
  return false;
}

const setSubmitButton = (txt, cls, disabled) => {
  formSubmitButtonObj.removeClass();
  formSubmitButtonObj.html((txt !== '') ? txt : defSubmitMsg);
	formSubmitButtonObj.addClass('btn btn-'+cls);
	if(typeof(disabled) == 'undefined' || !disabled) {
		formSubmitButtonObj.prop('disabled', false);
	}
  if(typeof(tm) != 'undefined') {
  	//-- hide status in "tm" ms
		setTimeout(() => {
			//changeClass(statusObj);
	  }, 5000);
  }
};

const setPartitionsButton = (txt, cls, disabled) => {
  partitionsSubmitButtonObj.removeClass();
  partitionsSubmitButtonObj.html((txt !== '') ? txt : defPartitionsMsg);
	partitionsSubmitButtonObj.addClass('btn btn-'+cls);
	if(typeof(disabled) == 'undefined' || !disabled) {
		partitionsSubmitButtonObj.prop('disabled', false);
	}
  if(typeof(tm) != 'undefined') {
  	//-- hide status in "tm" ms
		setTimeout(() => {
			//changeClass(statusObj);
	  }, 5000);
  }
};


const partitionsTableInit = () => {
  partitionsSubmitButtonObj.click((e) => {
		e.preventDefault();

		const formData = {};
		
		otaInputCheckboxObj.each(function() {
			const obj = $(this);
			const checked = obj.prop('checked');

			if(this.type === 'checkbox') {
	  		formData[this.name] = $(this).prop('checked') ? 1 : 0;
	  	}
		});

		$.ajax({
    	type: 'post',
		  url: '/post/partitions-submit',
		  data: formData,
		  dataType: 'json',
		  success: function(result) {
		  	if(isValidJSON(result)) {
		  		const data = JSON.parse(result);
		  		if(data.status == 'success') {
		      	const msg = savedPartitionsMsg.replace('[NUM-BYTES]', data.fsize);
		      	setPartitionsButton(msg, 'success');
		      	return true;
		      } else {
		      	if(typeof(data.message) != 'undefined') {
		      		setPartitionsButton(data.message, 'danger');
		      		return true;
		      	}
		      }
		    }
		    setPartitionsButton(errPartitionsMsg, 'danger');
		  },
		  error: function(result) {
		    //console.error(result);
		    setPartitionsButton(errPartitionsMsg, 'danger');
		  },
		});
  });
  
  otaInputCheckboxObj.change((e) => {
  	e.preventDefault();
		
		let totalSize = parseInt(partitionsTotalSizeThObj.text());
		
		const obj = $(e.target);
		const val = parseInt(obj.val());
		const checked = obj.prop('checked');
		//-- get its TR
		const papa = obj.parent().parent();
		papa.removeClass();
		if(checked) {
			papa.addClass('checked');
			totalSize += val;
		} else {
			papa.addClass('unchecked');
			totalSize -= val;
		}

		partitionsTotalSizeThObj.text(totalSize);
		//console.log(sizeInMB);
		partitionsSizeChange();

  });
};

const partitionsSizeChange = () => {
	const totalSize = parseInt(partitionsTotalSizeThObj.text());
	const sizeInMB = totalSize / 1024 / 1024;
	const sizeRAM = cfgEspToolPyFlashSizeSelectObj.val();

	partitionsTotalSizeThObj.removeClass();
	if(sizeInMB > sizeRAM) {
		partitionsTotalSizeThObj.addClass('error');
	} else {
		partitionsTotalSizeThObj.addClass('success');
	}
};
