class OTAUpdater {
  constructor() {
    this.currentFile = null;
    this.form = document.getElementById('ota-form');
    this.fileInput = document.getElementById('firmware-file');
    this.fileInfo = document.getElementById('file-info');
    this.updateBtn = document.getElementById('update-btn');
    this.progressSection = document.getElementById('progress-section');
    this.progressFill = document.getElementById('progress-fill');
    this.progressText = document.getElementById('progress-text');
    this.uploadedSize = document.getElementById('uploaded-size');
    this.totalSize = document.getElementById('total-size');
    this.statusMessage = document.getElementById('status-message');

    this.init();
  }

  init() {
    this.loadDeviceInfo();
    this.setupEventListeners();
    setInterval(() => this.loadDeviceInfo(), 10000);
  }

  async loadDeviceInfo() {
    try {
      console.log('Loading device info from /api/device-info');
      const response = await fetch('/api/device-info');
      
      if (response.ok) {
        const data = await response.json();
        console.log('Device info received:', data);
        this.updateDeviceInfo(data);
      } else {
        console.error('Failed to load device info, status:', response.status);
        this.showStatus('Failed to load device information', 'error');
      }
    } catch (error) {
      console.error('Error loading device info:', error);
      this.showStatus('Error loading device information', 'error');
    }
  }

  updateDeviceInfo(data) {
    //-- Creating a mapping of the data field id to the HTML element id
    /*
    API response:
    {
      "id":"ESP32-Thread-BR",
      "version":"1.0.0-dev",
      "free_heap":179700,
      "project_name":"esp_ot_br",
      "running_partition":"ota_0",
      "update_partition":"ota_1",
      "secure_version":0,
      "sdk_version":"v5.4.1-dirty"
    }
    */
    
    const fieldMapping = {
      'device-id': 'id',
      'fw-version': 'version', 
      'free-heap': 'free_heap',
      'active-partition': 'running_partition',
      'update-partition': 'update_partition',
      'project-name': 'project_name',
      'sdk-version': 'sdk_version'
    };

    //-- Updating each element
    for (const [elementId, dataField] of Object.entries(fieldMapping)) {
      const element = document.getElementById(elementId);
      if (element) {
        if (dataField === 'free_heap' && data[dataField]) {
          element.textContent = `${Math.round(data[dataField] / 1024)} KB`;
        } else {
          element.textContent = data[dataField] || 'N/A';
        }
      } else {
        console.error(`Element #${elementId} not found in HTML`);
      }
    }
  }

  setupEventListeners() {
    this.fileInput.addEventListener('change', (e) => this.handleFileSelect(e));
    this.form.addEventListener('submit', (e) => this.handleSubmit(e));
  }

  //-- Upload via form
  handleFileSelect(event) {
    const file = event.target.files[0];
    if (!file) return;

    //-- Checking the file size (max 2MB)
    if (file.size > 2 * 1024 * 1024) {
      this.showStatus('File too large (max 2MB)', 'error');
      this.fileInput.value = '';
      return;
    }

    //-- Checking the extension
    if (!file.name.endsWith('.bin')) {
      this.showStatus('Please select a .bin file', 'error');
      this.fileInput.value = '';
      return;
    }

    this.currentFile = file;
    this.fileInfo.textContent = `${file.name} (${this.formatFileSize(file.size)})`;
    this.updateBtn.disabled = false;
    this.hideStatus();
  }

  //-- Direct binary upload
  async handleSubmit(event) {
    event.preventDefault();
    
    if (!this.currentFile) {
      this.showStatus('Please select a firmware file', 'error');
      return;
    }

    this.updateBtn.disabled = true;
    this.showProgress();
    this.showStatus('Starting firmware update...', 'info');

    try {
      const formData = new FormData();
      formData.append('firmware', this.currentFile);

      this.showStatus('Uploading firmware...', 'info');
      
      const response = await fetch('/api/ota-update', {
        method: 'POST',
        body: formData,
      });

      const result = await response.json();
      
      if (response.ok) {
        if (result.status === 'success') {
          this.showStatus('✅ ' + result.message, 'success');
          this.progressFill.style.width = '100%';
          this.progressText.textContent = '100%';
          this.uploadedSize.textContent = this.formatFileSize(this.currentFile.size);
          
          this.showStatus('Device will restart shortly...', 'info');
          
          //-- Refreshing device information after reboot
          setTimeout(() => {
            this.loadDeviceInfo();
          }, 10000);
          
        } else {
          this.showStatus('❌ ' + result.message, 'error');
          this.updateBtn.disabled = false;
        }
      } else {
        this.showStatus('❌ Error: ' + result.message, 'error');
        this.updateBtn.disabled = false;
      }
    } catch (error) {
      this.showStatus('❌ Network error: ' + error.message, 'error');
      this.updateBtn.disabled = false;
    }
  }
  
  //-- Direct binary upload
  async handleSubmitDirect(event) {
    event.preventDefault();
    
    if (!this.currentFile) {
      this.showStatus('Please select a firmware file', 'error');
      return;
    }
  
    this.updateBtn.disabled = true;
    this.showProgress();
    this.showStatus('Reading firmware file...', 'info');
  
    try {
      //-- Reading a file as an ArrayBuffer
      const arrayBuffer = await this.currentFile.arrayBuffer();
      
      this.showStatus('Uploading firmware...', 'info');
      
      //-- Sending as raw binary
      const response = await fetch('/api/ota-update', {
        method: 'POST',
        body: arrayBuffer,
        headers: {
          'Content-Type': 'application/octet-stream',
          'X-File-Name': this.currentFile.name
        }
      });
  
      const result = await response.json();
      
      if (response.ok && result.status === 'success') {
        this.showStatus('✅ ' + result.message, 'success');
        this.progressFill.style.width = '100%';
        this.progressText.textContent = '100%';

        //-- Refreshing device information after reboot
        setTimeout(() => {
          this.loadDeviceInfo();
        }, 10000);

      } else {
        this.showStatus('❌ ' + result.message, 'error');
        this.updateBtn.disabled = false;
      }
    } catch (error) {
      this.showStatus('❌ Network error: ' + error.message, 'error');
      this.updateBtn.disabled = false;
    }
  }

  showProgress() {
    this.progressSection.style.display = 'block';
    this.progressFill.style.width = '0%';
    this.progressText.textContent = '0%';
    this.totalSize.textContent = this.formatFileSize(this.currentFile.size);
    this.uploadedSize.textContent = '0 KB';
  }

  showStatus(message, type) {
    this.statusMessage.textContent = message;
    this.statusMessage.className = `status-message ${type}`;
  }

  hideStatus() {
    this.statusMessage.textContent = '';
    this.statusMessage.className = 'status-message';
  }

  formatFileSize(bytes) {
    if (bytes < 1024) return `${bytes} B`;
    if (bytes < 1024 * 1024) return `${(bytes / 1024).toFixed(1)} KB`;
    return `${(bytes / (1024 * 1024)).toFixed(1)} MB`;
  }
}

document.addEventListener('DOMContentLoaded', () => {
  new OTAUpdater();
});
