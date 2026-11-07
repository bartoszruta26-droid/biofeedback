#include "gui/LoginDialog.hpp"
#include <QApplication>
#include <QStyle>
#include <QFont>
#include <QTimer>
#include <QInputDialog>
#include <QMessageBox>
#include "core/DebugManager.hpp"

namespace gui {

LoginDialog::LoginDialog(biofeedback::Authentication& auth, QWidget *parent)
    : QDialog(parent)
    , m_authentication(auth)
    , m_titleLabel(nullptr)
    , m_usernameLabel(nullptr)
    , m_passwordLabel(nullptr)
    , m_usernameEdit(nullptr)
    , m_passwordEdit(nullptr)
    , m_confirmPasswordLabel(nullptr)
    , m_confirmPasswordEdit(nullptr)
    , m_loginButton(nullptr)
    , m_cancelButton(nullptr)
    , m_statusLabel(nullptr)
    , m_loginSuccess(false)
    , m_firstRun(false)
    , m_usersLoaded(false)
{
    setWindowTitle(tr("Logowanie - Biofeedback"));
    setFixedSize(420, 430);
    setWindowModality(Qt::ApplicationModal);
    
    setupUI();
    createConnections();
    
    // Existing accounts are never replaced. An empty, valid account store
    // enables a first-run administrator creation flow instead of a weak seed.
    m_usersLoaded = m_authentication.loadUsers();
    m_firstRun = m_usersLoaded && !m_authentication.hasUsers();
    if (m_firstRun) {
        setWindowTitle(tr("Pierwsze uruchomienie - Biofeedback"));
        m_titleLabel->setText(tr("Utwórz konto administratora"));
        m_confirmPasswordLabel->show();
        m_confirmPasswordEdit->show();
        m_loginButton->setText(tr("Utwórz administratora"));
        m_cancelButton->setText(tr("Zamknij"));
        m_statusLabel->setText(tr("Hasło administratora musi mieć co najmniej 12 znaków."));
        m_statusLabel->setStyleSheet("color: #2563eb; font-size: 12px;");
        m_statusLabel->show();
    } else if (!m_usersLoaded) {
        m_loginButton->setEnabled(false);
        m_statusLabel->setText(tr("Nie można odczytać bazy kont. Sprawdź plik w katalogu konfiguracji użytkownika."));
        m_statusLabel->setStyleSheet("color: #e74c3c; font-size: 12px;");
        m_statusLabel->show();
    }
    
    // Debug logging for initialization
    core::DebugManager::instance().sendDebugMessage(
        "LoginDialog initialized",
        core::DebugLevel::DEBUG,
        "LoginDialog::LoginDialog"
    );
}

LoginDialog::~LoginDialog()
{
    core::DebugManager::instance().sendDebugMessage(
        "LoginDialog destroyed",
        core::DebugLevel::DEBUG,
        "LoginDialog::~LoginDialog"
    );
}

void LoginDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // Tytuł
    m_titleLabel = new QLabel(tr("🔐 Logowanie do systemu"));
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(m_titleLabel);
    
    // Separator
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line);
    
    // Nazwa użytkownika
    m_usernameLabel = new QLabel(tr("Nazwa użytkownika:"));
    m_usernameLabel->setStyleSheet("font-weight: bold; color: #34495e;");
    mainLayout->addWidget(m_usernameLabel);
    
    m_usernameEdit = new QLineEdit();
    m_usernameEdit->setPlaceholderText(tr("Wprowadź nazwę użytkownika"));
    m_usernameEdit->setMinimumHeight(35);
    m_usernameEdit->setStyleSheet(
        "QLineEdit { "
        "   border: 2px solid #bdc3c7; "
        "   border-radius: 5px; "
        "   padding: 5px 10px; "
        "   font-size: 14px; "
        "} "
        "QLineEdit:focus { "
        "   border-color: #3498db; "
        "}"
    );
    mainLayout->addWidget(m_usernameEdit);
    
    // Hasło
    m_passwordLabel = new QLabel(tr("Hasło:"));
    m_passwordLabel->setStyleSheet("font-weight: bold; color: #34495e;");
    mainLayout->addWidget(m_passwordLabel);
    
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setPlaceholderText(tr("Wprowadź hasło"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMinimumHeight(35);
    m_passwordEdit->setStyleSheet(
        "QLineEdit { "
        "   border: 2px solid #bdc3c7; "
        "   border-radius: 5px; "
        "   padding: 5px 10px; "
        "   font-size: 14px; "
        "} "
        "QLineEdit:focus { "
        "   border-color: #3498db; "
        "}"
    );
    mainLayout->addWidget(m_passwordEdit);

    m_confirmPasswordLabel = new QLabel(tr("Powtórz hasło administratora:"));
    m_confirmPasswordLabel->setStyleSheet("font-weight: bold; color: #34495e;");
    m_confirmPasswordLabel->hide();
    mainLayout->addWidget(m_confirmPasswordLabel);

    m_confirmPasswordEdit = new QLineEdit();
    m_confirmPasswordEdit->setPlaceholderText(tr("Wpisz hasło ponownie"));
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    m_confirmPasswordEdit->setMinimumHeight(35);
    m_confirmPasswordEdit->setStyleSheet(
        "QLineEdit { border: 2px solid #bdc3c7; border-radius: 5px; padding: 5px 10px; font-size: 14px; }"
        "QLineEdit:focus { border-color: #3498db; }");
    m_confirmPasswordEdit->hide();
    mainLayout->addWidget(m_confirmPasswordEdit);

    // Status
    m_statusLabel = new QLabel();
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: #e74c3c; font-size: 12px;");
    m_statusLabel->hide();
    mainLayout->addWidget(m_statusLabel);
    
    // Przyciski
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    m_cancelButton = new QPushButton(tr("Anuluj"));
    m_cancelButton->setMinimumHeight(40);
    m_cancelButton->setStyleSheet(
        "QPushButton { "
        "   background-color: #95a5a6; "
        "   color: white; "
        "   border: none; "
        "   border-radius: 5px; "
        "   font-size: 14px; "
        "   font-weight: bold; "
        "} "
        "QPushButton:hover { "
        "   background-color: #7f8c8d; "
        "} "
        "QPushButton:pressed { "
        "   background-color: #6c7a7b; "
        "}"
    );
    buttonLayout->addWidget(m_cancelButton);
    
    m_loginButton = new QPushButton(tr("Zaloguj"));
    m_loginButton->setMinimumHeight(40);
    m_loginButton->setDefault(true);
    m_loginButton->setStyleSheet(
        "QPushButton { "
        "   background-color: #3498db; "
        "   color: white; "
        "   border: none; "
        "   border-radius: 5px; "
        "   font-size: 14px; "
        "   font-weight: bold; "
        "} "
        "QPushButton:hover { "
        "   background-color: #2980b9; "
        "} "
        "QPushButton:pressed { "
        "   background-color: #2471a3; "
        "} "
        "QPushButton:disabled { "
        "   background-color: #bdc3c7; "
        "}"
    );
    buttonLayout->addWidget(m_loginButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Domyślne skupienie na polu nazwy użytkownika
    m_usernameEdit->setFocus();
}

void LoginDialog::createConnections()
{
    connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginButtonClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &LoginDialog::onCancelButtonClicked);
    connect(m_usernameEdit, &QLineEdit::textChanged, this, &LoginDialog::onUsernameTextChanged);
    connect(m_passwordEdit, &QLineEdit::textChanged, this, &LoginDialog::onPasswordTextChanged);
    connect(m_confirmPasswordEdit, &QLineEdit::textChanged, this, &LoginDialog::onPasswordTextChanged);

    // Enter zatwierdza logowanie
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginButtonClicked);
}

void LoginDialog::onLoginButtonClicked()
{
    try {
        std::string username = m_usernameEdit->text().trimmed().toStdString();
        std::string password = m_passwordEdit->text().toStdString();

        if (!m_usersLoaded) {
            m_statusLabel->setText(tr("Baza kont jest niedostępna. Logowanie i tworzenie kont są zablokowane."));
            m_statusLabel->show();
            return;
        }

        // Security: Do not log raw username - use redacted form or just indicate attempt
        core::DebugManager::instance().sendDebugMessage(
            QString("Login attempt for user: [REDACTED] (length: %1)")
                .arg(QString::number(username.length())),
            core::DebugLevel::INFO,
            "LoginDialog::onLoginButtonClicked"
        );
        
        if (username.empty()) {
            m_statusLabel->setText(tr("⚠ Nazwa użytkownika nie może być pusta"));
            m_statusLabel->show();
            m_usernameEdit->setFocus();
            
            core::DebugManager::instance().sendDebugMessage(
                "Login failed: empty username",
                core::DebugLevel::WARNING,
                "LoginDialog::onLoginButtonClicked"
            );
            return;
        }
        
        if (password.empty()) {
            m_statusLabel->setText(tr("⚠ Hasło nie może być puste"));
            m_statusLabel->show();
            m_passwordEdit->setFocus();
            
            core::DebugManager::instance().sendDebugMessage(
                "Login failed: empty password",
                core::DebugLevel::WARNING,
                "LoginDialog::onLoginButtonClicked"
            );
            return;
        }
        
        if (m_firstRun) {
            const QString passwordText = m_passwordEdit->text();
            if (passwordText.size() < 12) {
                m_statusLabel->setText(tr("Hasło musi mieć co najmniej 12 znaków."));
                m_statusLabel->show();
                m_passwordEdit->setFocus();
                return;
            }
            if (passwordText != m_confirmPasswordEdit->text()) {
                m_statusLabel->setText(tr("Wprowadzone hasła są różne."));
                m_statusLabel->show();
                m_confirmPasswordEdit->clear();
                m_confirmPasswordEdit->setFocus();
                return;
            }
            m_loginButton->setEnabled(false);
            m_cancelButton->setEnabled(false);
            m_statusLabel->setText(tr("Tworzenie konta administratora..."));
            m_statusLabel->setStyleSheet("color: #2563eb; font-size: 12px;");
            m_statusLabel->show();
            QApplication::processEvents();
            if (!m_authentication.createInitialAdministrator(username, password)) {
                m_statusLabel->setText(tr("Nie udało się utworzyć konta. Sprawdź uprawnienia katalogu danych."));
                m_statusLabel->setStyleSheet("color: #e74c3c; font-size: 12px;");
                m_loginButton->setEnabled(true);
                m_cancelButton->setEnabled(true);
                return;
            }
            m_firstRun = false;
        }

        // Blokada przycisków podczas logowania
        m_loginButton->setEnabled(false);
        m_cancelButton->setEnabled(false);
        m_statusLabel->setText(tr("⏳ Logowanie..."));
        m_statusLabel->setStyleSheet("color: #f39c12; font-size: 12px;");
        m_statusLabel->show();
        QApplication::processEvents();
        
        if (attemptLogin(username, password)) {
            if (m_authentication.requiresPasswordChange()) {
                bool accepted=false;
                const QString first=QInputDialog::getText(this,tr("Wymagana zmiana hasła"),tr("Nowe hasło (minimum 12 znaków):"),QLineEdit::Password,QString(),&accepted);
                if(!accepted){m_authentication.logout();m_loginButton->setEnabled(true);m_cancelButton->setEnabled(true);return;}
                bool confirmed=false;
                const QString second=QInputDialog::getText(this,tr("Potwierdzenie hasła"),tr("Powtórz nowe hasło:"),QLineEdit::Password,QString(),&confirmed);
                if(!confirmed||first!=second||first.size()<12||!m_authentication.changePassword(first.toStdString())){
                    m_authentication.logout();QMessageBox::warning(this,tr("Zmiana hasła"),tr("Zmiana hasła nie powiodła się. Wymagane jest hasło o długości co najmniej 12 znaków."));
                    m_loginButton->setEnabled(true);m_cancelButton->setEnabled(true);return;
                }
            }
            m_statusLabel->setText(tr("✓ Zalogowano pomyślnie!"));
            m_statusLabel->setStyleSheet("color: #27ae60; font-size: 12px;");
            m_loginSuccess = true;
            m_username = QString::fromStdString(username);
            m_role = QString::fromStdString(m_authentication.getCurrentUserRole());
            
            // Security: Do not log username even on success - only log role
            core::DebugManager::instance().sendDebugMessage(
                QString("Login successful for user: [REDACTED], role: %1")
                    .arg(m_role),
                core::DebugLevel::INFO,
                "LoginDialog::onLoginButtonClicked"
            );
            
            // Krótka pauza przed zamknięciem
            QTimer::singleShot(500, this, &QDialog::accept);
        } else {
            m_statusLabel->setText(tr("✗ Błędna nazwa użytkownika lub hasło"));
            m_statusLabel->setStyleSheet("color: #e74c3c; font-size: 12px;");
            m_passwordEdit->clear();
            m_passwordEdit->setFocus();
            m_loginButton->setEnabled(true);
            m_cancelButton->setEnabled(true);
            
            // Security: Do not log username on failure - just indicate failed attempt
            core::DebugManager::instance().sendDebugMessage(
                "Login failed for user: [REDACTED] - invalid credentials",
                core::DebugLevel::WARNING,
                "LoginDialog::onLoginButtonClicked"
            );
        }
    } catch (const std::exception& e) {
        m_statusLabel->setText(tr("✗ Wystąpił błąd podczas logowania"));
        m_statusLabel->setStyleSheet("color: #e74c3c; font-size: 12px;");
        m_loginButton->setEnabled(true);
        m_cancelButton->setEnabled(true);
        
        core::DebugManager::instance().sendDebugMessage(
            QString("Exception during login: %1").arg(e.what()),
            core::DebugLevel::ERROR,
            "LoginDialog::onLoginButtonClicked"
        );
    } catch (...) {
        m_statusLabel->setText(tr("✗ Wystąpił nieznany błąd podczas logowania"));
        m_statusLabel->setStyleSheet("color: #e74c3c; font-size: 12px;");
        m_loginButton->setEnabled(true);
        m_cancelButton->setEnabled(true);
        
        core::DebugManager::instance().sendDebugMessage(
            "Unknown exception during login",
            core::DebugLevel::ERROR,
            "LoginDialog::onLoginButtonClicked"
        );
    }
}

void LoginDialog::onCancelButtonClicked()
{
    core::DebugManager::instance().sendDebugMessage(
        "Login cancelled by user",
        core::DebugLevel::INFO,
        "LoginDialog::onCancelButtonClicked"
    );
    m_loginSuccess = false;
    reject();
}

void LoginDialog::onUsernameTextChanged(const QString& text)
{
    m_statusLabel->hide();
    const bool confirmationReady = !m_firstRun || !m_confirmPasswordEdit->text().isEmpty();
    m_loginButton->setEnabled(m_usersLoaded && !text.trimmed().isEmpty() &&
                              !m_passwordEdit->text().isEmpty() && confirmationReady);
}

void LoginDialog::onPasswordTextChanged(const QString& text)
{
    m_statusLabel->hide();
    const bool confirmationReady = !m_firstRun || !m_confirmPasswordEdit->text().isEmpty();
    m_loginButton->setEnabled(m_usersLoaded && !text.isEmpty() &&
                              !m_usernameEdit->text().trimmed().isEmpty() && confirmationReady);
}

bool LoginDialog::attemptLogin(const std::string& username, const std::string& password)
{
    try {
        core::DebugManager::instance().sendDebugMessage(
            "Attempting authentication via Authentication module",
            core::DebugLevel::DEBUG,
            "LoginDialog::attemptLogin"
        );
        bool result = m_authentication.login(username, password);
        
        if (result) {
            core::DebugManager::instance().sendDebugMessage(
                "Authentication successful",
                core::DebugLevel::INFO,
                "LoginDialog::attemptLogin"
            );
        } else {
            core::DebugManager::instance().sendDebugMessage(
                "Authentication failed - invalid credentials",
                core::DebugLevel::WARNING,
                "LoginDialog::attemptLogin"
            );
        }
        
        return result;
    } catch (const std::exception& e) {
        core::DebugManager::instance().sendDebugMessage(
            QString("Exception during authentication: %1").arg(e.what()),
            core::DebugLevel::ERROR,
            "LoginDialog::attemptLogin"
        );
        return false;
    } catch (...) {
        core::DebugManager::instance().sendDebugMessage(
            "Unknown exception during authentication",
            core::DebugLevel::ERROR,
            "LoginDialog::attemptLogin"
        );
        return false;
    }
}

void LoginDialog::closeEvent(QCloseEvent *event)
{
    // Zapobiegaj zamknięciu okna przez X - wymuś użycie przycisku Anuluj
    core::DebugManager::instance().sendDebugMessage(
        "Login dialog closed via window close button",
        core::DebugLevel::INFO,
        "LoginDialog::closeEvent"
    );
    m_loginSuccess = false;
    QDialog::closeEvent(event);
}

} // namespace gui
