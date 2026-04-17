#ifndef GUI_LOGINDIALOG_HPP
#define GUI_LOGINDIALOG_HPP

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QString>
#include <QTimer>
#include "core/Authentication.hpp"

namespace gui {

/**
 * @class LoginDialog
 * @brief Dialog logowania do aplikacji Biofeedback
 * 
 * LoginDialog zapewnia graficzny interfejs do:
 * - Wprowadzania nazwy użytkownika i hasła
 * - Weryfikacji danych logowania
 * - Wyświetlania komunikatów o błędach
 */
class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(biofeedback::Authentication& auth, QWidget *parent = nullptr);
    ~LoginDialog() override;

    /**
     * @brief Pobiera nazwę zalogowanego użytkownika
     * @return Nazwa użytkownika
     */
    QString getUsername() const { return m_username; }

    /**
     * @brief Pobiera rolę zalogowanego użytkownika
     * @return Rola użytkownika
     */
    QString getRole() const { return m_role; }

    /**
     * @brief Sprawdza czy logowanie powiodło się
     * @return true jeśli użytkownik został pomyślnie zalogowany
     */
    bool loginSuccess() const { return m_loginSuccess; }

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onLoginButtonClicked();
    void onCancelButtonClicked();
    void onUsernameTextChanged(const QString& text);
    void onPasswordTextChanged(const QString& text);

private:
    void setupUI();
    void createConnections();
    bool attemptLogin(const std::string& username, const std::string& password);

    // Widgety
    QLabel* m_titleLabel;
    QLabel* m_usernameLabel;
    QLabel* m_passwordLabel;
    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QPushButton* m_loginButton;
    QPushButton* m_cancelButton;
    QLabel* m_statusLabel;

    // Referencja do obiektu Authentication
    biofeedback::Authentication& m_authentication;

    // Stan logowania
    QString m_username;
    QString m_role;
    bool m_loginSuccess;
};

} // namespace gui

#endif // GUI_LOGINDIALOG_HPP
