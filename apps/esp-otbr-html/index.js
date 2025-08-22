
var INPUT_PASSWORD = "";

// Проверяем авторизацию при загрузке
document.addEventListener('DOMContentLoaded', function()
{
  if(!checkAuth()) {
    showLogin();
  } else {
    showContent();
  }
});

function showLogin()
{
  document.getElementById('loginOverlay').style.display = 'flex';
  document.getElementById('mainContent').classList.add('hidden');
}

function showContent()
{
  document.getElementById('loginOverlay').style.display = 'none';
  document.getElementById('mainContent').classList.remove('hidden');
}

// Добавляем проверку авторизации ко всем AJAX запросам
const originalFetch = window.fetch;
window.fetch = function(...args) {
  return originalFetch.apply(this, args).then(response => {
    if(response.status === 401) {
      // Не авторизован - показываем форму логина
      showLogin();
      throw new Error('Not authorized');
    }
    return response;
  });
};

function handleLogin(event)
{
  event.preventDefault();
  
  const password = document.getElementById('password').value;

  //console.warn(INPUT_PASSWORD, ' -> ', password, 'md5=', CryptoJS.MD5(password).toString());

  //-- mySecurePassword123
  const INPUT_PASSWORD = '88d20abeabe807c001b6ed342b261978';
  
  //password = "mySecurePassword123";
  var hashedPassword = CryptoJS.MD5(password);
  var hashedPasswordHex = hashedPassword.toString();

  //console.log("Original Password:", password);
  //console.log("MD5 Hash (Hex):", hashedPasswordHex);

  if(CryptoJS.MD5(password).toString() == INPUT_PASSWORD) {
    console.warn('CORRECT!');
    localStorage.setItem('authenticated', 'true');
    showContent();
  } else {
    document.getElementById('loginError').style.display = 'block';
  }
}

function checkAuth() {
  return localStorage.getItem('authenticated') === 'true';
}

/*
document.getElementById('fileInput').addEventListener('change', function(event)
{
  const file = event.target.files[0];
  if(file) {
    const reader = new FileReader();
    reader.onload = function(e) {
      INPUT_PASSWORD = e.target.result;
      //console.log(e.target.result); // Content of the file
    };
    reader.readAsText(file); // Or readAsDataURL, readAsArrayBuffer
  }
});
*/