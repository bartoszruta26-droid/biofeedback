#include "core/DebugManager.hpp"
#include <QDebug>

namespace core {

DebugManager& DebugManager::instance()
{
    static DebugManager instance;
    return instance;
}

DebugManager::DebugManager()
    : m_debugCallback(nullptr)
{
}

DebugManager::~DebugManager()
{
    unregisterDebugCallback();
}

void DebugManager::registerDebugCallback(std::function<void(const QString&, const QString&)> callback)
{
    QMutexLocker locker(&m_mutex);
    m_debugCallback = callback;
}

void DebugManager::unregisterDebugCallback()
{
    QMutexLocker locker(&m_mutex);
    m_debugCallback = nullptr;
}

void DebugManager::sendDebugMessage(const QString& message, const QString& type)
{
    QMutexLocker locker(&m_mutex);
    
    // Emituj sygnał dla podłączonych odbiorców
    emit debugMessageSent(message, type);
    
    // Wywołaj callback jeśli jest zarejestrowany (zazwyczaj do DebugTab)
    if (m_debugCallback) {
        m_debugCallback(message, type);
    } else {
        // Jeśli nie ma callbacka, wypisz na stderr jako fallback
        qWarning() << "[DEBUG]" << type << ":" << message;
    }
}

bool DebugManager::hasCallback() const
{
    QMutexLocker locker(&m_mutex);
    return m_debugCallback != nullptr;
}

} // namespace core
