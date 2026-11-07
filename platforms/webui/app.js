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

  async function checkApi() {
    if (!window.BiofeedbackApi) {
      apiStatus.textContent = "Klient API niedostępny";
      return;
    }
    const api = window.BiofeedbackApi.createApiClient();
    try {
      const status = await api.getStatus();
      apiStatus.textContent = status && status.status ? String(status.status) : "API dostępne";
      apiStatus.classList.remove("status-pending");
      apiStatus.classList.add("status-ok");
    } catch (error) {
      apiStatus.textContent = error.status ? "Błąd HTTP " + error.status : "API nieosiągalne";
      apiStatus.title = error.message;
    }
  }
  checkApi();
})();
