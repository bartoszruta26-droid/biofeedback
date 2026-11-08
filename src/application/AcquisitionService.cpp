#include <stdexcept>
#include "application/AcquisitionService.hpp"

#include <utility>

namespace biofeedback::application {

AcquisitionService::AcquisitionService(std::shared_ptr<MeasurementService> measurements)
    : m_measurements(std::move(measurements)) {
    if (!m_measurements) {
        throw std::invalid_argument("AcquisitionService requires MeasurementService");
    }
}

bool AcquisitionService::startSession(AcquisitionSession session, std::string& error) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (session.id.empty() || session.patientId.empty() || session.deviceId.empty()) {
        error = "INVALID_SESSION";
        return false;
    }
    if (m_session.active || m_stopping) {
        error = "ACQUISITION_ALREADY_ACTIVE";
        return false;
    }
    session.active = true;
    m_session = std::move(session);
    m_measurements->clearHistory();
    m_measurements->start();
    error.clear();
    return true;
}

bool AcquisitionService::stopSession(const std::string& sessionId, std::string& error) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_session.active || sessionId.empty() || m_session.id != sessionId || m_stopping) {
        error = "ACTIVE_SESSION_NOT_FOUND";
        return false;
    }
    // Close the ingest gate first, then drain callbacks already admitted. This
    // prevents a sample from being accepted after stopSession() has returned.
    m_stopping = true;
    m_session.active = false;
    m_ingestIdle.wait(lock, [this]() { return m_inFlightIngests == 0; });
    m_measurements->stop();
    m_session.active = false;
    m_stopping = false;
    error.clear();
    return true;
}

bool AcquisitionService::ingest(const std::string& deviceId,
                                const measurement::MeasurementInput& input,
                                measurement::MeasurementSample& output) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_session.active || m_stopping || deviceId.empty() || deviceId != m_session.deviceId)
            return false;
        ++m_inFlightIngests;
    }

    const bool accepted = m_measurements->ingest(input, output);
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        --m_inFlightIngests;
        if (m_inFlightIngests == 0) m_ingestIdle.notify_all();
    }
    return accepted;
}

bool AcquisitionService::activeSession(AcquisitionSession& output) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_session.active) return false;
    output = m_session;
    return true;
}

std::shared_ptr<MeasurementService> AcquisitionService::measurementService() const noexcept {
    return m_measurements;
}

} // namespace biofeedback::application
