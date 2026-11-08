#include "application/AcquisitionService.hpp"

#include <cassert>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <string>

int main() {
    using namespace biofeedback;
    auto engine = std::make_shared<measurement::MeasurementEngine>();
    auto measurementService = std::make_shared<application::MeasurementService>(engine);
    application::AcquisitionService acquisition(measurementService);

    std::string error;
    application::AcquisitionSession invalid;
    assert(!acquisition.startSession(invalid, error));
    assert(error == "INVALID_SESSION");

    application::AcquisitionSession session;
    session.id = "session-1";
    session.patientId = "patient-1";
    session.deviceId = "hx711-1";
    session.protocolId = "strength-test";
    assert(acquisition.startSession(session, error));
    assert(error.empty());
    assert(measurementService->isRunning());

    application::AcquisitionSession active;
    assert(acquisition.activeSession(active));
    assert(active.id == "session-1");
    assert(active.patientId == "patient-1");

    application::AcquisitionSession competing = session;
    competing.id = "session-2";
    assert(!acquisition.startSession(competing, error));
    assert(error == "ACQUISITION_ALREADY_ACTIVE");

    measurement::MeasurementInput input;
    input.hasTimestamp = true;
    input.deviceTimestampMs = 10;
    input.rawValue = 120;
    input.value = 12.0;
    input.sensorValid = true;
    input.unit = "N";
    input.source = "hx711-1";
    measurement::MeasurementSample sample;
    assert(!acquisition.ingest("other-device", input, sample));
    assert(acquisition.ingest("hx711-1", input, sample));
    assert(sample.isValid());

    assert(!acquisition.stopSession("wrong-session", error));
    assert(error == "ACTIVE_SESSION_NOT_FOUND");

    // stopSession must wait for an ingest already admitted to the pipeline.
    std::mutex callbackMutex;
    std::condition_variable callbackCondition;
    bool callbackEntered = false;
    bool releaseCallback = false;
    measurementService->subscribe([&](const measurement::MeasurementSample&) {
        std::unique_lock<std::mutex> lock(callbackMutex);
        callbackEntered = true;
        callbackCondition.notify_all();
        callbackCondition.wait(lock, [&]() { return releaseCallback; });
    });
    std::thread ingestThread([&]() {
        measurement::MeasurementSample concurrentSample;
        assert(acquisition.ingest("hx711-1", input, concurrentSample));
    });
    {
        std::unique_lock<std::mutex> lock(callbackMutex);
        callbackCondition.wait(lock, [&]() { return callbackEntered; });
    }
    std::atomic<bool> stopReturned{false};
    std::thread stopThread([&]() {
        std::string stopError;
        assert(acquisition.stopSession("session-1", stopError));
        stopReturned.store(true, std::memory_order_release);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    assert(!stopReturned.load(std::memory_order_acquire));
    {
        std::lock_guard<std::mutex> lock(callbackMutex);
        releaseCallback = true;
    }
    callbackCondition.notify_all();
    ingestThread.join();
    stopThread.join();
    assert(stopReturned.load(std::memory_order_acquire));

    assert(acquisition.startSession(competing, error)); // session can be restarted after stop
    assert(acquisition.stopSession("session-2", error));
    assert(!measurementService->isRunning());
    assert(!acquisition.activeSession(active));
    assert(!acquisition.ingest("hx711-1", input, sample));
    return 0;
}
