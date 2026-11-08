(() => {
  const pages = Array.from(document.querySelectorAll(".page"));
  const navItems = Array.from(document.querySelectorAll("[data-page]"));
  const breadcrumb = document.getElementById("breadcrumb");
  const sidebar = document.getElementById("sidebar");
  const apiStatus = document.getElementById("apiStatus");
  const pageNames = {
    dashboard: "Pulpit", measurements: "Pomiary", patients: "Pacjenci",
    training: "Trening", devices: "Urządzenia", diagnostics: "Diagnostyka",
    settings: "Ustawienia"
  };

  function openPage(name) {
    if (!pageNames[name]) return;
    pages.forEach((page) => page.classList.toggle("active", page.id === "page-" + name));
    navItems.forEach((item) => {
      const active = item.dataset.page === name;
      item.classList.toggle("active", active);
      if (active) item.setAttribute("aria-current", "page");
      else item.removeAttribute("aria-current");
    });
    breadcrumb.textContent = pageNames[name];
    sidebar.classList.remove("open");
    window.location.hash = name;
  }

  navItems.forEach((item) => item.addEventListener("click", () => openPage(item.dataset.page)));
  document.querySelectorAll("[data-open]").forEach((item) => {
    item.addEventListener("click", () => openPage(item.dataset.open));
  });

  function restorePage() {
    const name = window.location.hash.replace(/^#/, "");
    if (pageNames[name]) {
      pages.forEach((page) => page.classList.toggle("active", page.id === "page-" + name));
      navItems.forEach((item) => {
        const active = item.dataset.page === name;
        item.classList.toggle("active", active);
        if (active) item.setAttribute("aria-current", "page");
      });
      breadcrumb.textContent = pageNames[name];
    }
  }
  window.addEventListener("hashchange", restorePage);
  restorePage();

  const root = document.documentElement;
  function toggleTheme() {
    const next = root.dataset.theme === "light" ? "dark" : "light";
    root.dataset.theme = next;
    try { window.localStorage.setItem("biofeedback-theme", next); } catch (_) { /* storage is optional */ }
  }
  try {
    const saved = window.localStorage.getItem("biofeedback-theme");
    if (saved === "light" || saved === "dark") root.dataset.theme = saved;
  } catch (_) { /* storage is optional */ }

  document.getElementById("themeToggle").addEventListener("click", toggleTheme);
  document.getElementById("themeToggleSettings").addEventListener("click", toggleTheme);
  document.getElementById("mobileMenu").addEventListener("click", () => sidebar.classList.toggle("open"));
  document.addEventListener("keydown", (event) => {
    if (event.key === "Escape") sidebar.classList.remove("open");
  });

  const authGate = document.getElementById("authGate");
  const appShell = document.getElementById("appShell");
  const loginForm = document.getElementById("loginForm");
  const loginMessage = document.getElementById("loginMessage");
  const loginButton = document.getElementById("loginButton");
  const currentUser = document.getElementById("currentUser");
  const api = window.BiofeedbackApi
    ? window.BiofeedbackApi.createApiClient()
    : null;

  function showAuthenticated(user) {
    authGate.hidden = true;
    appShell.hidden = false;
    currentUser.textContent = user.username + " · " + user.role;
    loginMessage.textContent = "";
  }

  loginForm.addEventListener("submit", async (event) => {
    event.preventDefault();
    if (!api) {
      loginMessage.textContent = "Klient API jest niedostępny.";
      return;
    }
    loginButton.disabled = true;
    loginMessage.textContent = "Trwa uwierzytelnianie…";
    try {
      await api.login(
        document.getElementById("loginUsername").value,
        document.getElementById("loginPassword").value
      );
      const user = await api.getCurrentUser();
      showAuthenticated(user);
      document.getElementById("loginPassword").value = "";
    } catch (error) {
      loginMessage.textContent = error.message || "Logowanie nie powiodło się.";
      document.getElementById("loginPassword").value = "";
    } finally {
      loginButton.disabled = false;
    }
  });

  document.getElementById("logoutButton").addEventListener("click", async () => {
    try {
      if (api) await api.logout();
    } catch (_) {
      // Local token is cleared by the client even if the server is unreachable.
    }
    appShell.hidden = true;
    authGate.hidden = false;
    currentUser.textContent = "";
    document.getElementById("loginPassword").value = "";
    loginMessage.textContent = "Sesja została zakończona. Zaloguj się ponownie.";
  });

  async function checkApi() {
    if (!api) {
      apiStatus.textContent = "Klient API niedostępny";
      loginMessage.textContent = "Nie można załadować klienta API.";
      return;
    }
    try {
      const status = await api.getStatus();
      const label = status && status.status ? String(status.status) : "API dostępne";
      apiStatus.textContent = label;
      apiStatus.classList.remove("status-pending");
      apiStatus.classList.add("status-ok");
      document.getElementById("apiSummary").textContent = "API dostępne";
      document.getElementById("apiSummaryDetail").textContent = label;
    } catch (error) {
      apiStatus.textContent = error.status ? "Błąd HTTP " + error.status : "API nieosiągalne";
      apiStatus.title = error.message;
      document.getElementById("apiSummary").textContent = "API niedostępne";
      document.getElementById("apiSummaryDetail").textContent = error.message;
      loginMessage.textContent = "Serwer API jest niedostępny.";
    }
  }
  checkApi();
})();