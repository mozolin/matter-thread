$(function() {

	comPortSelectInit();
	getFlashIdInit();

});


const statusObj = $('#div-status');
const comPortsListObj = $('#select-com-ports-list');
let comPortSelectedInt = null;
const esptoolFlashIdObj = $('#button-esptool-flash-id');


//-- choose COM port
const comPortSelectInit = () => {
	//-- onInit
	setComPortSelected();
	comPortsListObj.change(() => {
  	//-- onChange
  	setComPortSelected();
	});
};

const setComPortSelected = () => {
	const v = comPortsListObj.find(":selected").val();
	//console.log(v);
	if(typeof(v) != 'undefined' && v != null) {
		comPortSelectedInt = parseInt(v);
		//console.log(comPortSelectedInt);
	}
}

//-- esptool -p COMx flash-id
const getFlashIdInit = () => {
  esptoolFlashIdObj.click(() => {
  	if(typeof(comPortSelectedInt) != 'undefined' && comPortSelectedInt != null) {
			
			$.ajax({
      	type: 'post',
			  url: '/post/esptool-flash-id',
			  data: {'com-port': comPortSelectedInt},
			  dataType: 'json',
			  success: function(result) {
			    esptoolFlashIdObj.show();
			    const data = JSON.parse(result);
			    if(typeof(data.port) != 'undefined') {
			    	if(typeof(data.chip) == 'undefined') {
			    		setStatus('The COM'+data.port+' port is invalid!', 10000, 'error');
        		} else {
        			setStatus('Successfully!', 10000, 'success');
        		}
			    } else {
			    	setStatus('Unknown error!', 10000, 'error');
			    }
			    
			  },
			  error: function(result) {
			    esptoolFlashIdObj.show();
			    //console.error(result);
			  },
			});

			esptoolFlashIdObj.hide();
			setStatus('esptool -p COM'+comPortSelectedInt+' flash-id...');
  	}
  });

  statusObj.click(() => {
    statusObj.hide();
    //esptoolFlashIdObj.show();
  });
};

const changeClass = (obj, cls) => {
  obj.removeClass();
  if(typeof(cls) != 'undefined') {
		obj.addClass(cls);
	}
};

const setStatus = (txt, tm, cls) => {
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

