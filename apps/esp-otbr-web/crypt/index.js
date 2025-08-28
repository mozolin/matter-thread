var INPUT_PASSWORD = "";

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
  $('#loginOverlay').css('display', 'flex');
  $('#mainContent, main, section').hide();
}

function showContent()
{
  $('#loginOverlay').hide();
  $('#mainContent, main, section').show();
}

const originalFetch = window.fetch;
window.fetch = function(...args) {
  return originalFetch.apply(this, args).then(response => {
    if(response.status === 401) {
      showLogin();
      throw new Error('Not authorized');
    }
    return response;
  });
};

function handleLogin(event)
{
  event.preventDefault();
  
  const username = document.getElementById('username').value;
  const password = document.getElementById('password').value;

  const MD5_USERNAME = '8341b873bab1f5fd6fe76eca518fe957';
  const MD5_PASSWORD = 'a50eb12c544980ec3b73261b2a6bfd1c';
  
  if(CryptoJS.MD5(username).toString() == MD5_USERNAME && CryptoJS.MD5(password).toString() == MD5_PASSWORD) {
    localStorage.setItem('authenticated', 'true');
    showContent();
  } else {
    document.getElementById('loginError').style.display = 'block';
  }
}

function checkAuth() {
  return localStorage.getItem('authenticated') === 'true';
}
