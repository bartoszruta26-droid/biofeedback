/**
 * Biofeedback WebUI API client.
 *
 * Same-origin by default. This module deliberately contains no simulated data:
 * unavailable endpoints reject with ApiError and callers must render that state.
 */
(function (root) {
  "use strict";

  class ApiError extends Error {
    constructor(message, status, payload) {
      super(message);
      this.name = "ApiError";
      this.status = status || 0;
      this.payload = payload || null;
    }
  }

  function createApiClient(options) {
    const apiBaseMeta = root.document && root.document.querySelector
      ? root.document.querySelector('meta[name="biofeedback-api-base"]')
      : null;
    const config = Object.assign({
      baseUrl: apiBaseMeta ? apiBaseMeta.content : "",
      fetchImpl: root.fetch ? root.fetch.bind(root) : null,
      WebSocketImpl: root.WebSocket || null,
      requestTimeoutMs: 8000,
      accessToken: null
    }, options || {});

    function url(path) {
      return config.baseUrl.replace(/\/$/, "") + path;
    }

    async function request(path, init) {
      if (!config.fetchImpl) throw new ApiError("Fetch API is unavailable", 0);
      const controller = typeof AbortController !== "undefined" ? new AbortController() : null;
      const timeout = controller ? setTimeout(() => controller.abort(), config.requestTimeoutMs) : null;
      const requestInit = Object.assign({}, init || {});
      requestInit.headers = Object.assign({ Accept: "application/json" }, requestInit.headers || {});
      if (config.accessToken && path !== "/api/v1/auth/login") {
        requestInit.headers.Authorization = "Bearer " + config.accessToken;
      }
      if (controller) requestInit.signal = controller.signal;
      try {
        const response = await config.fetchImpl(url(path), requestInit);
        const text = await response.text();
        let payload = null;
        if (text) {
          try { payload = JSON.parse(text); }
          catch (_) { throw new ApiError("API returned invalid JSON", response.status); }
        }
        if (!response.ok) {
          if (response.status === 401 && path !== "/api/v1/auth/login") config.accessToken = null;
          const message = payload && payload.message ? payload.message : "API request failed";
          throw new ApiError(message, response.status, payload);
        }
        return payload;
      } catch (error) {
        if (error instanceof ApiError) throw error;
        const message = error && error.name === "AbortError" ? "API request timed out" : "API is unavailable";
        throw new ApiError(message, 0, { cause: error && error.message });
      } finally {
        if (timeout !== null) clearTimeout(timeout);
      }
    }

    function jsonRequest(method, body) {
      return { method, headers: { Accept: "application/json", "Content-Type": "application/json" }, body: JSON.stringify(body || {}) };
    }

    const resourceCollections = new Set([
      "patients", "measurements", "training-sessions", "training-results",
      "game-sessions", "game-results"
    ]);
    function resourcePath(collection, id) {
      if (!resourceCollections.has(collection)) throw new ApiError("Unsupported resource collection", 400);
      const base = "/api/v1/" + collection;
      return id === undefined || id === null
        ? base
        : base + "/" + encodeURIComponent(String(id));
    }
    function listResources(collection) {
      return request(resourcePath(collection));
    }
    function getResource(collection, id) {
      return request(resourcePath(collection, id));
    }
    function createResource(collection, record) {
      return request(resourcePath(collection), jsonRequest("POST", record));
    }
    function updateResource(collection, id, expectedVersion, record) {
      return request(resourcePath(collection, id), jsonRequest("PUT", {
        expected_version: expectedVersion, record
      }));
    }
    function deleteResource(collection, id, expectedVersion) {
      return request(resourcePath(collection, id), jsonRequest("DELETE", {
        expected_version: expectedVersion
      }));
    }

    function subscribe(path, onMessage, onState) {
      if (!config.WebSocketImpl) {
        const error = new ApiError("WebSocket is unavailable", 0);
        if (onState) onState({ state: "unavailable", error });
        return { close: function () {} };
      }
      const base = config.baseUrl || (root.location ? root.location.origin : "");
      const socketUrl = new URL(path, base || "http://localhost");
      socketUrl.protocol = socketUrl.protocol === "https:" ? "wss:" : "ws:";
      const socket = new config.WebSocketImpl(socketUrl.toString());
      socket.addEventListener("open", () => onState && onState({ state: "connected" }));
      socket.addEventListener("message", (event) => {
        try { onMessage(JSON.parse(event.data)); }
        catch (error) { onState && onState({ state: "invalid-message", error }); }
      });
      socket.addEventListener("error", () => onState && onState({ state: "error" }));
      socket.addEventListener("close", (event) => onState && onState({ state: "disconnected", code: event.code }));
      return { close: () => socket.close() };
    }

    return Object.freeze({
      request,
      login: async (username, password) => {
        config.accessToken = null;
        const result = await request("/api/v1/auth/login", jsonRequest("POST", { username, password }));
        if (!result || typeof result.access_token !== "string" || !/^[0-9a-fA-F]{64}$/.test(result.access_token)) {
          throw new ApiError("API login response did not contain an access token", 0, result);
        }
        config.accessToken = result.access_token;
        return result;
      },
      logout: async () => {
        try { return await request("/api/v1/auth/logout", jsonRequest("POST")); }
        finally { config.accessToken = null; }
      },
      getCurrentUser: () => request("/api/v1/auth/me"),
      getStatus: () => request("/api/v1/status"),
      getDevices: () => request("/api/v1/devices"),
      connectDevice: (id) => request("/api/v1/devices/" + encodeURIComponent(id) + "/connect", jsonRequest("POST")),
      disconnectDevice: (id) => request("/api/v1/devices/" + encodeURIComponent(id) + "/disconnect", jsonRequest("POST")),
      listResources,
      getResource,
      createResource,
      updateResource,
      deleteResource,
      getSessions: () => listResources("measurements"),
      createSession: (data) => createResource("measurements", data),
      getPatients: () => listResources("patients"),
      getPatient: (id) => getResource("patients", id),
      createPatient: (record) => createResource("patients", record),
      updatePatient: (id, version, record) => updateResource("patients", id, version, record),
      deletePatient: (id, version) => deleteResource("patients", id, version),
      getTrainingSessions: () => listResources("training-sessions"),
      createTrainingSession: (record) => createResource("training-sessions", record),
      getTrainingResults: () => listResources("training-results"),
      createTrainingResult: (record) => createResource("training-results", record),
      getGameSessions: () => listResources("game-sessions"),
      createGameSession: (record) => createResource("game-sessions", record),
      getGameResults: () => listResources("game-results"),
      createGameResult: (record) => createResource("game-results", record),
      subscribeMeasurements: (onMessage, onState) => subscribe("/ws/measurements", onMessage, onState),
      subscribeEvents: (onMessage, onState) => subscribe("/ws/events", onMessage, onState)
    });
  }

  const exported = { ApiError, createApiClient };
  if (typeof module !== "undefined" && module.exports) module.exports = exported;
  root.BiofeedbackApi = exported;
})(typeof globalThis !== "undefined" ? globalThis : this);
