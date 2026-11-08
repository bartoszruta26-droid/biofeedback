"use strict";

const test = require("node:test");
const assert = require("node:assert/strict");
const { createApiClient, ApiError } = require("./api-client.js");

function response(status, payload) {
  return {
    ok: status >= 200 && status < 300,
    status,
    text: async () => payload === undefined ? "" : JSON.stringify(payload)
  };
}

test("status and device endpoints use the v1 API contract", async () => {
  const calls = [];
  const api = createApiClient({
    fetchImpl: async (url, init) => {
      calls.push({ url, init });
      return response(200, { ok: true });
    }
  });
  assert.deepEqual(await api.getStatus(), { ok: true });
  assert.deepEqual(await api.getDevices(), { ok: true });
  assert.equal(calls[0].url, "/api/v1/status");
  assert.equal(calls[1].url, "/api/v1/devices");
});

test("device identifiers are URL encoded and commands are POST", async () => {
  let call;
  const api = createApiClient({
    fetchImpl: async (url, init) => {
      call = { url, init };
      return response(202, { accepted: true });
    }
  });
  await api.connectDevice("sensor / 1");
  assert.equal(call.url, "/api/v1/devices/sensor%20%2F%201/connect");
  assert.equal(call.init.method, "POST");
});

test("HTTP errors preserve status and payload", async () => {
  const api = createApiClient({
    fetchImpl: async () => response(503, { message: "service unavailable" })
  });
  await assert.rejects(api.getStatus(), (error) => {
    assert.ok(error instanceof ApiError);
    assert.equal(error.status, 503);
    assert.equal(error.message, "service unavailable");
    return true;
  });
});

test("invalid JSON is reported as an API error", async () => {
  const api = createApiClient({
    fetchImpl: async () => ({ ok: true, status: 200, text: async () => "not-json" })
  });
  await assert.rejects(api.getStatus(), (error) => error instanceof ApiError && error.message === "API returned invalid JSON");
});

test("network failures are normalized without inventing a successful state", async () => {
  const api = createApiClient({
    fetchImpl: async () => { throw new Error("connection refused"); }
  });
  await assert.rejects(api.getStatus(), (error) => error instanceof ApiError && error.status === 0 && error.message === "API is unavailable");
});

test("measurement WebSocket uses secure transport on HTTPS origins", () => {
  let socketUrl;
  class FakeWebSocket {
    constructor(value) { socketUrl = value; this.listeners = {}; }
    addEventListener(name, callback) { this.listeners[name] = callback; }
    close() {}
  }
  const api = createApiClient({
    baseUrl: "https://biofeedback.example",
    WebSocketImpl: FakeWebSocket
  });
  const subscription = api.subscribeMeasurements(() => {});
  assert.equal(socketUrl, "wss://biofeedback.example/ws/measurements");
  subscription.close();
});

test("login stores bearer token in memory and protected calls attach it", async () => {
  const calls = [];
  const api = createApiClient({
    fetchImpl: async (url, init) => {
      calls.push({ url, init });
      if (url === "/api/v1/auth/login") return response(200, { access_token: "a".repeat(64), token_type: "Bearer" });
      if (url === "/api/v1/auth/me") {
        return init.headers.Authorization
          ? response(200, { username: "operator", role: "clinician" })
          : response(401, { message: "authentication required" });
      }
      if (url === "/api/v1/auth/logout") return response(200, { ok: true });
      return response(404, { message: "not found" });
    }
  });

  await api.login("operator", "example-password");
  assert.equal(calls[0].init.headers.Authorization, undefined);
  assert.equal(calls[0].init.method, "POST");
  assert.deepEqual(await api.getCurrentUser(), { username: "operator", role: "clinician" });
  assert.equal(calls[1].init.headers.Authorization, "Bearer " + "a".repeat(64));
  await api.logout();
  assert.equal(calls[2].init.headers.Authorization, "Bearer " + "a".repeat(64));
  await assert.rejects(api.getCurrentUser(), (error) => error instanceof ApiError && error.status === 401);
  assert.equal(calls[3].init.headers.Authorization, undefined);
});

test("login rejects malformed bearer tokens", async () => {
  const api = createApiClient({ fetchImpl: async () => response(200, { access_token: "g".repeat(64) }) });
  await assert.rejects(api.login("operator", "example-password"), (error) =>
    error instanceof ApiError && error.message === "API login response did not contain an access token");
});

test("login rejects a successful response without an access token", async () => {
  const api = createApiClient({ fetchImpl: async () => response(200, { ok: true }) });
  await assert.rejects(api.login("operator", "example-password"), (error) =>
    error instanceof ApiError && error.message === "API login response did not contain an access token");
});
