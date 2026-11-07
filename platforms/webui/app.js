(() => {
  const pages = Array.from(document.querySelectorAll(".page"));
  const navItems = Array.from(document.querySelectorAll("[data-page]"));
  const breadcrumb = document.getElementById("breadcrumb");
  const sidebar = document.getElementById("sidebar");
  const pageNames = {
    dashboard: "Pulpit", measurements: "Pomiary", patients: "Pacjenci",
    training: "Trening", devices: "Urządzenia", diagnostics: "Diagnostyka",
    settings: "Ustawienia"
  };

  function openPage(name) {
    if (!pageNames[name]) return;
    pages.forEach((page) => page.classList.toggle("active", page.id === "page-" + name));
    navItems.forEach((item) => item.classList.toggle("active", item.dataset.page === name));
    breadcrumb.textContent = pageNames[name];
    sidebar.classList.remove("open");
  }

  navItems.forEach((item) => item.addEventListener("click", () => openPage(item.dataset.page)));
  document.querySelectorAll("[data-open]").forEach((item) => {
    item.addEventListener("click", () => openPage(item.dataset.open));
  });

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
})();
