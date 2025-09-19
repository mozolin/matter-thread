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
        setInterval(() => this.loadDeviceInfo(), 5000);
    }

    async loadDeviceInfo() {
        try {
            const response = await fetch('/api/device-info');
            if (response.ok) {
                const data = await response.json();
                this.updateDeviceInfo(data);
            }
        } catch (error) {
            console.error('Failed to load device info:', error);
        }
    }

    updateDeviceInfo(data) {
        document.getElementById('device-id').textContent = data.id;
        document.getElementById('fw-version').textContent = data.version;
        document.getElementById('free-heap').textContent = `${Math.round(data.free_heap / 1024)} KB`;
        document.getElementById('active-partition').textContent = data.project_name || 'ESP-Matter';
    }

    setupEventListeners() {
        this.fileInput.addEventListener('change', (e) => this.handleFileSelect(e));
        this.form.addEventListener('submit', (e) => this.handleSubmit(e));
    }

    handleFileSelect(event) {
        const file = event.target.files[0];
        if (!file) return;

        this.currentFile = file;
        this.fileInfo.textContent = `${file.name} (${this.formatFileSize(file.size)})`;
        this.updateBtn.disabled = false;
        this.hideStatus();
    }

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

            const response = await fetch('/api/ota-update', {
                method: 'POST',
                body: formData,
            });

            const result = await response.json();
            
            if (response.ok) {
                this.showStatus('OTA process started successfully!', 'success');
                this.showStatus('Use Matter OTA or console commands for full update', 'info');
            } else {
                this.showStatus(`Update failed: ${result.message}`, 'error');
                this.updateBtn.disabled = false;
            }
        } catch (error) {
            this.showStatus(`Error: ${error.message}`, 'error');
            this.updateBtn.disabled = false;
        }
    }

    showProgress() {
        this.progressSection.style.display = 'block';
        this.totalSize.textContent = this.formatFileSize(this.currentFile.size);
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

// Initialize when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    new OTAUpdater();
});
