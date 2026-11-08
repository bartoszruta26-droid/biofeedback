#include "measurement/MeasurementEngine.hpp"

#include <cassert>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cmath>
#include <limits>
#include <mutex>
#include <thread>
#include <stdexcept>

using namespace biofeedback::measurement;

int main() {
    MeasurementEngine engine({-100.0, 100.0, 2});
    int callbackCount = 0;
    const auto token = engine.subscribe([&callbackCount](const MeasurementSample&) {
        ++callbackCount;
    });
    assert(token != 0);

    MeasurementInput input;
    input.deviceTimestampMs = 1234;
    input.hasTimestamp = true;
    input.rawValue = 250;
    input.value = 2500.0;
    input.sensorValid = true;
    input.unit = "g";
    input.source = "sensor-test";

    const auto grams = engine.ingest(input);
    assert(grams.isValid());
    assert(grams.unit == "kg");
    assert(std::abs(grams.value - 2.5) < 1e-9);
    assert(grams.timestampUnixMs > 0);
    assert(grams.sequence == 1);

    MeasurementInput wrappedClockInput = input;
    wrappedClockInput.deviceTimestampMs = 0;
    const auto wrappedClockSample = engine.ingest(wrappedClockInput);
    assert(wrappedClockSample.isValid()); // uint32 device-clock wrap is not a missing timestamp
    assert(wrappedClockSample.deviceTimestampMs == 0);

    input.unit = "mm";
    input.value = 25.0;
    const auto millimetres = engine.ingest(input);
    assert(millimetres.isValid());
    assert(millimetres.unit == "m");
    assert(std::abs(millimetres.value - 0.025) < 1e-9);

    input.unit = "not-a-unit";
    assert(engine.ingest(input).quality == Quality::InvalidUnit);
    input.unit = "N";
    input.source.clear();
    assert(engine.ingest(input).quality == Quality::InvalidSource);
    input.source = "sensor-test";
    input.sensorValid = false;
    assert(engine.ingest(input).quality == Quality::SensorRejected);
    input.sensorValid = true;
    input.hasTimestamp = false;
    assert(engine.ingest(input).quality == Quality::MissingTimestamp);
    input.hasTimestamp = true;
    input.deviceTimestampMs = 1235;
    input.value = std::numeric_limits<double>::infinity();
    assert(engine.ingest(input).quality == Quality::NonFiniteValue);
    input.value = 101.0;
    assert(engine.ingest(input).quality == Quality::OutsideConfiguredRange);

    assert(callbackCount == 9);
    const auto history = engine.history();
    assert(history.size() == 2);
    assert(history.front().sequence == 8);
    assert(history.back().sequence == 9);

    engine.unsubscribe(token);
    engine.ingest(input);
    assert(callbackCount == 9);
    engine.clearHistory();
    assert(engine.history().empty());

    MeasurementEngine concurrentEngine;
    std::mutex callbackMutex;
    std::condition_variable callbackEntered;
    std::condition_variable releaseCallback;
    bool entered = false;
    bool release = false;
    std::atomic<bool> unsubscribeReturned{false};
    const auto concurrentToken = concurrentEngine.subscribe(
        [&](const MeasurementSample&) {
            std::unique_lock<std::mutex> lock(callbackMutex);
            entered = true;
            callbackEntered.notify_one();
            releaseCallback.wait(lock, [&] { return release; });
        });
    MeasurementInput concurrentInput;
    concurrentInput.deviceTimestampMs = 10;
    concurrentInput.hasTimestamp = true;
    concurrentInput.value = 1.0;
    concurrentInput.sensorValid = true;
    concurrentInput.unit = "N";
    concurrentInput.source = "unsubscribe-regression";
    std::thread producer([&] { concurrentEngine.ingest(concurrentInput); });
    {
        std::unique_lock<std::mutex> lock(callbackMutex);
        callbackEntered.wait(lock, [&] { return entered; });
    }
    std::thread remover([&] {
        concurrentEngine.unsubscribe(concurrentToken);
        unsubscribeReturned.store(true, std::memory_order_release);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    assert(!unsubscribeReturned.load(std::memory_order_acquire));
    {
        std::lock_guard<std::mutex> lock(callbackMutex);
        release = true;
    }
    releaseCallback.notify_one();
    producer.join();
    remover.join();
    assert(unsubscribeReturned.load(std::memory_order_acquire));
    concurrentEngine.ingest(concurrentInput);
    assert(concurrentEngine.history().size() == 2);

    // A recovered journal resumes sequence assignment without reusing old IDs.
    MeasurementEngineConfig recoveredConfig;
    recoveredConfig.initialSequence = 301;
    MeasurementEngine recoveredEngine(recoveredConfig);
    const auto recoveredSample = recoveredEngine.ingest(concurrentInput);
    assert(recoveredSample.sequence == 301);

    // The JSON/API sequence domain is signed 64-bit; exhaustion must fail closed.
    MeasurementEngineConfig finalSequenceConfig;
    finalSequenceConfig.initialSequence =
        static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());
    MeasurementEngine finalSequenceEngine(finalSequenceConfig);
    const auto finalSequenceSample = finalSequenceEngine.ingest(concurrentInput);
    assert(finalSequenceSample.sequence ==
           static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()));
    MeasurementSample exhaustedOutput;
    assert(!finalSequenceEngine.ingest(concurrentInput, exhaustedOutput));
    assert(exhaustedOutput.sequence == 0);
    assert(finalSequenceEngine.history().size() == 1);

    bool invalidInitialSequenceRejected = false;
    try {
        MeasurementEngineConfig invalidConfig;
        invalidConfig.initialSequence = 0;
        MeasurementEngine invalidEngine(invalidConfig);
    } catch (const std::invalid_argument&) {
        invalidInitialSequenceRejected = true;
    }
    assert(invalidInitialSequenceRejected);

    // Persistence is a pre-publication gate: failed commits consume no sequence,
    // enter no in-memory history and are never delivered to subscribers.
    MeasurementEngine commitEngine;
    int commitPublications = 0;
    commitEngine.subscribe([&](const MeasurementSample&) { ++commitPublications; });
    bool allowCommit = false;
    commitEngine.setCommitHook([&](const MeasurementSample& candidate) {
        assert(candidate.sequence == 1);
        return allowCommit;
    });
    MeasurementSample committedSample;
    assert(!commitEngine.ingest(concurrentInput, committedSample));
    assert(committedSample.sequence == 0);
    assert(commitEngine.history().empty());
    assert(commitPublications == 0);
    allowCommit = true;
    assert(commitEngine.ingest(concurrentInput, committedSample));
    assert(committedSample.sequence == 1);
    assert(commitEngine.history().size() == 1);
    assert(commitPublications == 1);
    return 0;
}
