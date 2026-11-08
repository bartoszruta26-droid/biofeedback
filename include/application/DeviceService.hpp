#pragma once
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
namespace biofeedback::application {
enum class DeviceState { Disconnected, Connecting, Connected, Error };
struct DeviceDescriptor {
    std::string id, name, transport;
    DeviceState state{DeviceState::Disconnected};
    std::string lastError;
};
class DeviceAdapter {
public:
    virtual ~DeviceAdapter() = default;
    virtual bool connect(std::string& error) = 0;
    virtual void disconnect() noexcept = 0;
    virtual bool isConnected() const noexcept = 0;
};
// Shared device lifecycle boundary. No actuator/electrode/antenna write API.
class DeviceService final {
public:
    using StateCallback = std::function<void(const DeviceDescriptor&)>;
    using CallbackToken = std::uint64_t;
    bool registerDevice(DeviceDescriptor descriptor, std::shared_ptr<DeviceAdapter> adapter);
    bool unregisterDevice(const std::string& id);
    bool connectDevice(const std::string& id);
    bool disconnectDevice(const std::string& id);
    std::vector<DeviceDescriptor> devices() const;
    bool device(const std::string& id, DeviceDescriptor& output) const;
    CallbackToken subscribe(StateCallback callback);
    void unsubscribe(CallbackToken token);
private:
    struct Entry { DeviceDescriptor descriptor; std::shared_ptr<DeviceAdapter> adapter; };
    void publish(const DeviceDescriptor& descriptor);
    mutable std::mutex m_mutex;
    std::map<std::string, Entry> m_devices;
    std::map<CallbackToken, StateCallback> m_callbacks;
    CallbackToken m_nextToken{1};
};
} // namespace biofeedback::application
