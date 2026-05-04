#ifndef CORE_DEBUGMANAGER_HPP
#define CORE_DEBUGMANAGER_HPP

#include <QObject>
#include <QString>
#include <QMutex>
#include <memory>
#include <functional>

namespace core {

/**
 * @class DebugManager
 * @brief Centralny menedżer wiadomości debugowych działający jako singleton
 * 
 * DebugManager umożliwia:
 * - Rejestrację callbacka do DebugTab
 * - Wysyłanie wiadomości debugowych z dowolnego miejsca w aplikacji
 * - Thread-safe operacje dzięki mutexowi
 * 
 * Tylko DebugTab i command line terminal zbierają dane debugowe.
 * Inne zakładki nie mają własnych terminali debugowych.
 */
class DebugManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Pobiera instancję singletona DebugManager
     * @return Referencja do instancji DebugManager
     */
    static DebugManager& instance();

    /**
     * @brief Rejestruje callback do wysyłania wiadomości debugowych
     * @param callback Funkcja przyjmująca message i type
     */
    void registerDebugCallback(std::function<void(const QString&, const QString&)> callback);

    /**
     * @brief Odłącza zarejestrowany callback
     */
    void unregisterDebugCallback();

    /**
     * @brief Wysyła wiadomość debugową do DebugTab
     * @param message Treść wiadomości
     * @param type Typ wiadomości (INFO, ERROR, WARNING, DATA, SERIAL, ARDUINO)
     */
    void sendDebugMessage(const QString& message, const QString& type = "INFO");

    /**
     * @brief Sprawdza czy callback jest zarejestrowany
     * @return true jeśli callback jest ustawiony
     */
    bool hasCallback() const;

signals:
    /**
     * @brief Sygnał emitowany gdy wiadomość debugowa została wysłana
     * @param message Treść wiadomości
     * @param type Typ wiadomości
     */
    void debugMessageSent(const QString& message, const QString& type);

private:
    /**
     * @brief Konstruktor prywatny dla singletona
     */
    DebugManager();

    /**
     * @brief Destruktor
     */
    ~DebugManager() override;

    // Zapobieganie kopiowaniu
    DebugManager(const DebugManager&) = delete;
    DebugManager& operator=(const DebugManager&) = delete;

    std::function<void(const QString&, const QString&)> m_debugCallback;
    mutable QMutex m_mutex;
};

} // namespace core

#endif // CORE_DEBUGMANAGER_HPP
