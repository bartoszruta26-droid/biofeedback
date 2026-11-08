#include "application/DeviceService.hpp"
#include <utility>
namespace biofeedback::application {
bool DeviceService::registerDevice(DeviceDescriptor descriptor, std::shared_ptr<DeviceAdapter> adapter) {
    if (descriptor.id.empty() || descriptor.name.empty() || descriptor.transport.empty() || !adapter) return false;
    descriptor.state = adapter->isConnected() ? DeviceState::Connected : DeviceState::Disconnected;
    descriptor.lastError.clear();
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_devices.count(descriptor.id) != 0) return false;
        m_devices.emplace(descriptor.id, Entry{descriptor, std::move(adapter)});
    }
    publish(descriptor);
    return true;
}
bool DeviceService::unregisterDevice(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto it = m_devices.find(id);
    if (it == m_devices.end() || it->second.adapter->isConnected()) return false;
    m_devices.erase(it);
    return true;
}
bool DeviceService::connectDevice(const std::string& id) {
    std::shared_ptr<DeviceAdapter> adapter;
    DeviceDescriptor state;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto it = m_devices.find(id);
        if (it == m_devices.end() || it->second.descriptor.state == DeviceState::Connecting) return false;
        if (it->second.adapter->isConnected()) return true;
        it->second.descriptor.state = DeviceState::Connecting;
        it->second.descriptor.lastError.clear();
        adapter = it->second.adapter;
        state = it->second.descriptor;
    }
    publish(state);
    std::string error;
    const bool connected = adapter->connect(error);
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto it = m_devices.find(id);
        if (it == m_devices.end()) { if (connected) adapter->disconnect(); return false; }
        it->second.descriptor.state = connected ? DeviceState::Connected : DeviceState::Error;
        it->second.descriptor.lastError = connected ? std::string() :
            (error.empty() ? "Nie udało się połączyć z urządzeniem." : error);
        state = it->second.descriptor;
    }
    publish(state);
    return connected;
}
bool DeviceService::disconnectDevice(const std::string& id) {
    std::shared_ptr<DeviceAdapter> adapter;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto it = m_devices.find(id);
        if (it == m_devices.end() || it->second.descriptor.state == DeviceState::Connecting) return false;
        adapter = it->second.adapter;
    }
    adapter->disconnect();
    DeviceDescriptor state;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto it = m_devices.find(id);
        if (it == m_devices.end()) return false;
        it->second.descriptor.state = DeviceState::Disconnected;
        it->second.descriptor.lastError.clear();
        state = it->second.descriptor;
    }
    publish(state);
    return true;
}
std::vector<DeviceDescriptor> DeviceService::devices() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<DeviceDescriptor> result;
    result.reserve(m_devices.size());
    for (const auto& item : m_devices) result.push_back(item.second.descriptor);
    return result;
}
bool DeviceService::device(const std::string& id, DeviceDescriptor& output) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto it = m_devices.find(id);
    if (it == m_devices.end()) return false;
    output = it->second.descriptor;
    return true;
}
DeviceService::CallbackToken DeviceService::subscribe(StateCallback callback) {
    if (!callback) return 0;
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto token = m_nextToken++;
    m_callbacks.emplace(token, std::move(callback));
    return token;
}
void DeviceService::unsubscribe(CallbackToken token) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_callbacks.erase(token);
}
void DeviceService::publish(const DeviceDescriptor& descriptor) {
    std::vector<StateCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& item : m_callbacks) callbacks.push_back(item.second);
    }
    for (const auto& callback : callbacks) {
        try { callback(descriptor); } catch (...) {}
    }
}
} // namespace biofeedback::application
