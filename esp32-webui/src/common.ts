
export async function loadNavbar() {
    const navbarContainer = document.getElementById('navbar');
    if (navbarContainer) {
      const response = await fetch('nav.html');
      navbarContainer.innerHTML = await response.text();
  
      const currentPath = window.location.pathname;
      document.querySelectorAll('.navbar .tab').forEach((tab) => {
        const a = tab as HTMLAnchorElement;
        if (a.href.endsWith(currentPath)) {
          a.classList.add('active');
        }
      });
    }
  }
  