#ifndef TAB_PATIENTTAB_HPP
#define TAB_PATIENTTAB_HPP

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QDateEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QScrollArea>

namespace tab {

/**
 * @class PatientTab
 * @brief Widget zakładki do zarządzania danymi pacjentów
 * 
 * PatientTab zapewnia interfejs do:
 * - Dodawania nowych pacjentów
 * - Edycji istniejących danych pacjentów
 * - Przeglądania listy pacjentów w tabeli
 * - Usuwania pacjentów z bazy
 * - Eksportu/importu danych pacjentów
 */
class PatientTab : public QWidget
{
    Q_OBJECT

public:
    explicit PatientTab(QWidget *parent = nullptr);
    ~PatientTab() override;

    /**
     * @brief Dodaje nowego pacjenta do tabeli
     * @param pesel Numer PESEL pacjenta
     * @param firstName Imię pacjenta
     * @param lastName Nazwisko pacjenta
     * @param birthDate Data urodzenia
     * @param gender Płeć pacjenta
     */
    void addPatient(const QString& pesel, const QString& firstName, 
                   const QString& lastName, const QDate& birthDate, 
                   const QString& gender);

    /**
     * @brief Pobiera liczbę pacjentów
     * @return Liczba pacjentów w tabeli
     */
    int patientCount() const;

    /**
     * @brief Czyści wszystkie dane pacjentów
     */
    void clearAllPatients();

    /**
     * @brief Eksportuje dane pacjentów do pliku CSV
     * @param filename Ścieżka do pliku wyjściowego
     * @return true jeśli eksport powiódł się
     */
    bool exportToCSV(const QString& filename);

    /**
     * @brief Importuje dane pacjentów z pliku CSV
     * @param filename Ścieżka do pliku wejściowego
     * @return true jeśli import powiódł się
     */
    bool importFromCSV(const QString& filename);

signals:
    /**
     * @brief Sygnał emitowany gdy dodano nowego pacjenta
     * @param pesel Numer PESEL
     * @param firstName Imię
     * @param lastName Nazwisko
     */
    void patientAdded(const QString& pesel, const QString& firstName, const QString& lastName);

    /**
     * @brief Sygnał emitowany gdy usunięto pacjenta
     * @param row Wiersz usuniętego pacjenta
     */
    void patientRemoved(int row);

    /**
     * @brief Sygnał emitowany gdy zaktualizowano dane pacjenta
     * @param row Wiersz zaktualizowanego pacjenta
     */
    void patientUpdated(int row);

    /**
     * @brief Sygnał emitowany gdy wybrano pacjenta z tabeli
     * @param pesel Numer PESEL wybranego pacjenta
     * @param firstName Imię wybranego pacjenta
     * @param lastName Nazwisko wybranego pacjenta
     */
    void patientSelected(const QString& pesel, const QString& firstName, const QString& lastName);
    
    /**
     * @brief Sygnał emitowany gdy zmieniono aktualnie załadowanego pacjenta
     * @param pesel Numer PESEL pacjenta
     * @param patientDataPath Ścieżka do danych pacjenta
     */
    void currentPatientChanged(const QString& pesel, const QString& patientDataPath);
    
    /**
     * @brief Pobiera aktualnie załadowanego pacjenta PESEL
     * @return PESEL aktualnego pacjenta lub pusty jeśli brak
     */
    QString getCurrentPatientPesel() const;
    
    /**
     * @brief Pobiera ścieżkę do danych aktualnego pacjenta
     * @return Ścieżka do katalogu z danymi pacjenta
     */
    QString getCurrentPatientDataPath() const;

public slots:
    /**
     * @brief Otwiera dialog dodawania nowego pacjenta
     */
    void openAddPatientDialog();

    /**
     * @brief Otwiera dialog edycji wybranego pacjenta
     */
    void editSelectedPatient();

    /**
     * @brief Usuwa wybranego pacjenta
     */
    void deleteSelectedPatient();

    /**
     * @brief Eksportuje dane pacjentów do pliku
     */
    void exportPatients();

    /**
     * @brief Importuje dane pacjentów z pliku
     */
    void importPatients();

    /**
     * @brief Czyści widok tabeli
     */
    void clearTable();

private slots:
    void onSelectionChanged();
    void onTableRowDoubleClicked(int row, int column);

private:
    void setupUI();
    void setupPatientForm();
    void setupPatientTable();
    void setupButtons();
    void createConnections();
    
    void fillGenderCombo(QComboBox* combo);
    bool validatePesel(const QString& pesel);
    int findPatientByPesel(const QString& pesel);

    // Główne widgety
    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    
    // Sekcja formularza
    QGroupBox* m_formGroup;
    QFormLayout* m_formLayout;
    
    QLineEdit* m_peselEdit;
    QLineEdit* m_firstNameEdit;
    QLineEdit* m_lastNameEdit;
    QDateEdit* m_birthDateEdit;
    QComboBox* m_genderCombo;
    QTextEdit* m_notesEdit;
    
    // Przyciski operacji na formularzu
    QHBoxLayout* m_formButtonsLayout;
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_clearButton;
    
    // Tabela pacjentów
    QGroupBox* m_tableGroup;
    QTableWidget* m_patientTable;
    
    // Przyciski akcji
    QHBoxLayout* m_actionButtonsLayout;
    QPushButton* m_exportButton;
    QPushButton* m_importButton;
    QPushButton* m_refreshButton;
    
    // Stan
    int m_currentRow;
    bool m_isEditMode;
    
    // Aktualnie załadowany pacjent
    QString m_currentPatientPesel;
    QString m_currentPatientDataPath;
};

} // namespace tab

#endif // TAB_PATIENTTAB_HPP
