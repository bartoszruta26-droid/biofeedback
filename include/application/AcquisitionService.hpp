#pragma once

#include "application/MeasurementService.hpp"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>

namespace biofeedback::application {

struct AcquisitionSession {
    std::string id;
    std::string patientId;
    std::string deviceId;
    std::string protocolId;
    bool active{false};
};

// Server-owned coordinator. Device adapters feed ingest(); clients never own
// the acquisition engine or open the physical transport themselves.
class AcquisitionService final {
public:
    explicit AcquisitionService(std::shared_ptr<MeasurementService> measurements);
    AcquisitionService(const AcquisitionService&) = delete;
    AcquisitionService& operator=(const AcquisitionService&) = delete;

    bool startSession(AcquisitionSession session, std::string& error);
    bool stopSession(const std::string& sessionId, std::string& error);
    bool ingest(const std::string& deviceId,
                const measurement::MeasurementInput& input,
                measurement::MeasurementSample& output);
    bool activeSession(AcquisitionSession& output) const;
    std::shared_ptr<MeasurementService> measurementService() const noexcept;

private:
    std::shared_ptr<MeasurementService> m_measurements;
    mutable std::mutex m_mutex;
    std::condition_variable m_ingestIdle;
    std::size_t m_inFlightIngests{0};
    bool m_stopping{false};
    AcquisitionSession m_session;
};

} // namespace biofeedback::application
