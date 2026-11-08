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
    refreshSharedData();
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
    clearResourceViews();
    setMessage("patientMessage", "");
    setMessage("measurementMessage", "");
    setMessage("trainingMessage", "");
    document.getElementById("loginPassword").value = "";
    loginMessage.textContent = "Sesja została zakończona. Zaloguj się ponownie.";
  });

  function setMessage(id, message) {
    const element = document.getElementById(id);
    if (element) element.textContent = message || "";
  }

  function clearResourceViews() {
    ["patientRecords", "measurementRecords", "trainingSessions", "trainingResults",
     "gameSessions", "gameResults"].forEach((id) => {
      const element = document.getElementById(id);
      if (element) element.replaceChildren();
    });
  }

  function renderSimpleCollection(containerId, items, titleFor, detailFor) {
    const container = document.getElementById(containerId);
    if (!container) return;
    container.replaceChildren();
    if (!items.length) {
      const empty = document.createElement("p");
      empty.className = "muted";
      empty.textContent = "Brak zapisanych rekordów.";
      container.append(empty);
      return;
    }
    items.forEach((record) => {
      const card = document.createElement("article");
      card.className = "resource-card";
      const content = document.createElement("div");
      const title = document.createElement("h3");
      title.textContent = titleFor(record);
      const detail = document.createElement("p");
      detail.textContent = detailFor(record);
      content.append(title, detail);
      card.append(content);
      container.append(card);
    });
  }

  async function refreshPatients() {
    try {
      const response = await api.getPatients();
      const container = document.getElementById("patientRecords");
      if (!container) return;
      container.replaceChildren();
      const items = Array.isArray(response.items) ? response.items : [];
      if (!items.length) {
        const empty = document.createElement("p");
        empty.className = "muted";
        empty.textContent = "Brak kart pacjentów.";
        container.append(empty);
      }
      items.forEach((patient) => {
        const card = document.createElement("article");
        card.className = "resource-card";
        const content = document.createElement("div");
        const title = document.createElement("h3");
        title.textContent = [patient.firstName, patient.lastName].filter(Boolean).join(" ") || "Pacjent";
        const detail = document.createElement("p");
        detail.textContent = "Wersja " + patient.version + " · ID " + patient.id;
        content.append(title, detail);

        const actions = document.createElement("div");
        actions.className = "resource-actions";
        const edit = document.createElement("button");
        edit.type = "button";
        edit.textContent = "Edytuj";
        edit.addEventListener("click", async () => {
          const firstName = window.prompt("Imię", patient.firstName || "");
          if (firstName === null) return;
          const lastName = window.prompt("Nazwisko", patient.lastName || "");
          if (lastName === null) return;
          const medicalHistory = window.prompt("Wywiad / uwagi", patient.medicalHistory || "");
          if (medicalHistory === null) return;
          try {
            await api.updatePatient(patient.id, patient.version, {
              firstName, lastName, pesel: patient.pesel || "", medicalHistory,
              birthDate: patient.birthDate || "", gender: patient.gender || ""
            });
            await refreshPatients();
            setMessage("patientMessage", "Karta została zaktualizowana.");
          } catch (error) {
            setMessage("patientMessage", error.message || "Nie udało się zaktualizować karty.");
          }
        });
        const remove = document.createElement("button");
        remove.type = "button";
        remove.className = "danger";
        remove.textContent = "Usuń";
        remove.addEventListener("click", async () => {
          if (!window.confirm("Usunąć tę kartę pacjenta?")) return;
          try {
            await api.deletePatient(patient.id, patient.version);
            await refreshPatients();
            setMessage("patientMessage", "Karta została usunięta.");
          } catch (error) {
            setMessage("patientMessage", error.message || "Nie udało się usunąć karty.");
          }
        });
        actions.append(edit, remove);
        card.append(content, actions);
        container.append(card);
      });
      setMessage("patientMessage", "Pobrano kart: " + items.length);
      populatePatientSelectors();
    } catch (error) {
      setMessage("patientMessage", error.message || "Nie można pobrać kart pacjentów.");
    }
  }

  async function refreshMeasurements() {
    try {
      const response = await api.listResources("measurements");
      const items = Array.isArray(response.items) ? response.items : [];
      renderSimpleCollection("measurementRecords", items,
        (item) => item.started_at || item.session_id || item.id,
        (item) => {
          const parts = [];
          if (item.status) parts.push("Status: " + item.status);
          if (item.sample_count !== undefined) parts.push("Próbki: " + item.sample_count);
          if (item.series) parts.push("Serie: " + item.series.length);
          if (item.version) parts.push("Wersja: " + item.version);
          return parts.join(" · ");
        });
      setMessage("measurementMessage", "Pobrano sesji: " + items.length);
    } catch (error) {
      setMessage("measurementMessage", error.message || "Nie można pobrać pomiarów.");
    }
  }

  async function refreshTraining() {
    try {
      const [sessions, results, gameSessions, gameResults] = await Promise.all([
        api.getTrainingSessions(), api.getTrainingResults(),
        api.getGameSessions(), api.getGameResults()
      ]);
      const list = (response) => Array.isArray(response.items) ? response.items : [];
      renderSimpleCollection("trainingSessions", list(sessions),
        (item) => item.started_at || item.session_id || item.id,
        (item) => "Status: " + (item.status || "—") + " · Ćwiczenia: " +
          (Array.isArray(item.exercise_results) ? item.exercise_results.length : "—"));
      renderSimpleCollection("trainingResults", list(results),
        (item) => item.exercise_name || item.id,
        (item) => "Powtórzenia: " + (item.completed_repetitions ?? "—") +
          " · Maks. siła: " + (item.maximum_peak_force_n ?? "—") + " N");
      renderSimpleCollection("gameSessions", list(gameSessions),
        (item) => item.game_id || item.id,
        (item) => "Wynik: " + (item.score ?? "—") + " · Status: " + (item.status || "—"));
      renderSimpleCollection("gameResults", list(gameResults),
        (item) => item.game_id || item.id,
        (item) => "Wynik: " + (item.score ?? "—") + " · Sesja treningowa: " +
          (item.training_session_id || "—"));
      setMessage("trainingMessage", "Historia została odświeżona.");
    } catch (error) {
      setMessage("trainingMessage", error.message || "Nie można pobrać historii treningu i gier.");
    }
  }

  function refreshSharedData() {
    if (!api) return;
    refreshPatients();
    populatePatientSelectors();
    refreshMeasurements();
    refreshTraining();
  }

  const patientForm = document.getElementById("patientForm");
  if (patientForm) {
    patientForm.addEventListener("submit", async (event) => {
      event.preventDefault();
      const button = document.getElementById("savePatientButton");
      button.disabled = true;
      try {
        await api.createPatient({
          firstName: document.getElementById("patientFirstName").value.trim(),
          lastName: document.getElementById("patientLastName").value.trim(),
          pesel: document.getElementById("patientPesel").value.trim(),
          medicalHistory: document.getElementById("patientMedicalHistory").value
        });
        patientForm.reset();
        await refreshPatients();
        setMessage("patientMessage", "Karta została utworzona.");
      } catch (error) {
        setMessage("patientMessage", error.message || "Nie udało się utworzyć karty.");
      } finally {
        button.disabled = false;
      }
    });
  }

  async function populatePatientSelectors() {
    const selectors = ["trainingPatient", "gamePatient"]
      .map((id) => document.getElementById(id)).filter(Boolean);
    if (!selectors.length || !api) return;
    try {
      const response = await api.getPatients();
      const patients = Array.isArray(response.items) ? response.items : [];
      selectors.forEach((select) => {
        const selected = select.value;
        select.replaceChildren(new Option("Wybierz pacjenta", ""));
        patients.forEach((patient) => {
          const label = [patient.firstName, patient.lastName].filter(Boolean).join(" ") || "Pacjent";
          select.add(new Option(label + " · " + patient.id, patient.id));
        });
        if (patients.some((patient) => patient.id === selected)) select.value = selected;
      });
    } catch (error) {
      setMessage("trainingMessage", error.message || "Nie można pobrać listy pacjentów.");
    }
  }

  const trainingEntryForm = document.getElementById("trainingEntryForm");
  trainingEntryForm?.addEventListener("submit", async (event) => {
    event.preventDefault();
    const button = document.getElementById("saveTrainingEntry");
    button.disabled = true;
    try {
      const patientId = document.getElementById("trainingPatient").value;
      const exercise = document.getElementById("trainingExercise").value.trim();
      const series = Number(document.getElementById("trainingSeries").value);
      const repetitions = Number(document.getElementById("trainingRepetitions").value);
      const targetRaw = document.getElementById("trainingTargetForce").value;
      const targetForce = targetRaw === "" ? null : Number(targetRaw);
      if (!patientId || !exercise || !Number.isInteger(series) || !Number.isInteger(repetitions) ||
          series < 0 || repetitions < 0 || (targetForce !== null && (!Number.isFinite(targetForce) || targetForce < 0))) {
        throw new Error("Sprawdź pacjenta oraz wartości treningu.");
      }
      const startedAt = new Date().toISOString();
      const session = await api.createTrainingSession({
        patient_id: patientId, started_at: startedAt, status: "completed",
        source_client: "webui", source_mode: "manual_entry",
        exercise_results: [{ exercise_name: exercise, completed_series: series,
          completed_repetitions: repetitions, target_force_n: targetForce }]
      });
      await api.createTrainingResult({
        training_session_id: session.id, patient_id: patientId,
        exercise_name: exercise, completed_series: series,
        completed_repetitions: repetitions, target_force_n: targetForce,
        source_client: "webui", source_mode: "manual_entry"
      });
      trainingEntryForm.reset();
      await refreshTraining();
      setMessage("trainingMessage", "Sesja i wynik treningu zostały zapisane na serwerze.");
    } catch (error) {
      setMessage("trainingMessage", error.message || "Nie udało się zapisać treningu.");
    } finally {
      button.disabled = false;
    }
  });

  const gameEntryForm = document.getElementById("gameEntryForm");
  gameEntryForm?.addEventListener("submit", async (event) => {
    event.preventDefault();
    const button = document.getElementById("saveGameEntry");
    button.disabled = true;
    try {
      const patientId = document.getElementById("gamePatient").value;
      const gameId = document.getElementById("gameIdentifier").value.trim();
      const exerciseName = document.getElementById("gameExerciseName").value.trim();
      const score = Number(document.getElementById("gameScore").value);
      if (!patientId || !gameId || !Number.isSafeInteger(score) || score < 0) {
        throw new Error("Sprawdź pacjenta, identyfikator gry i wynik.");
      }
      const session = await api.createGameSession({
        game_id: gameId, patient_id: patientId, score,
        status: "completed", started_at: new Date().toISOString(),
        source_client: "webui", source_mode: "manual_entry"
      });
      await api.createGameResult({
        game_id: gameId, patient_id: patientId,
        game_session_id: session.id, score,
        exercise_name: exerciseName || gameId,
        source_client: "webui", source_mode: "manual_entry"
      });
      gameEntryForm.reset();
      await refreshTraining();
      setMessage("trainingMessage", "Sesja i wynik gry zostały zapisane na serwerze.");
    } catch (error) {
      setMessage("trainingMessage", error.message || "Nie udało się zapisać wyniku gry.");
    } finally {
      button.disabled = false;
    }
  });

  document.getElementById("refreshPatients")?.addEventListener("click", refreshPatients);
  document.getElementById("refreshMeasurements")?.addEventListener("click", refreshMeasurements);
  document.getElementById("refreshTraining")?.addEventListener("click", refreshTraining);

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