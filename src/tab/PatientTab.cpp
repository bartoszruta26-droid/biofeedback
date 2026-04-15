#include "tab/PatientTab.hpp"
#include <QFileInfo>
#include <QTextStream>
#include <QDateTime>

namespace tab {

PatientTab::PatientTab(QWidget *parent)
    : QWidget(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_formGroup(nullptr)
    , m_formLayout(nullptr)
    , m_peselEdit(nullptr)
    , m_firstNameEdit(nullptr)
    , m_lastNameEdit(nullptr)
    , m_birthDateEdit(nullptr)
    , m_genderCombo(nullptr)
    , m_notesEdit(nullptr)
    , m_formButtonsLayout(nullptr)
    , m_addButton(nullptr)
    , m_editButton(nullptr)
    , m_deleteButton(nullptr)
    , m_clearButton(nullptr)
    , m_tableGroup(nullptr)
    , m_patientTable(nullptr)
    , m_actionButtonsLayout(nullptr)
    , m_exportButton(nullptr)
    , m_importButton(nullptr)
    , m_refreshButton(nullptr)
    , m_currentRow(-1)
    , m_isEditMode(false)
{
    setupUI();
    createConnections();
}

PatientTab::~PatientTab()
{
    // Qt automatycznie czyści widgety z parentem
}

void PatientTab::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);

    setupPatientForm();
    setupPatientTable();
    setupButtons();
}

void PatientTab::setupPatientForm()
{
    m_formGroup = new QGroupBox(tr("Dane Pacjenta"), this);
    m_formLayout = new QFormLayout();
    m_formLayout->setSpacing(8);
    m_formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    // Pola formularza
    m_peselEdit = new QLineEdit(this);
    m_peselEdit->setMaxLength(11);
    m_peselEdit->setPlaceholderText(tr("11 cyfr"));
    
    m_firstNameEdit = new QLineEdit(this);
    m_firstNameEdit->setPlaceholderText(tr("Imię"));
    
    m_lastNameEdit = new QLineEdit(this);
    m_lastNameEdit->setPlaceholderText(tr("Nazwisko"));
    
    m_birthDateEdit = new QDateEdit(this);
    m_birthDateEdit->setCalendarPopup(true);
    m_birthDateEdit->setDisplayFormat("yyyy-MM-dd");
    m_birthDateEdit->setDate(QDate::currentDate().addYears(-18));
    
    m_genderCombo = new QComboBox(this);
    fillGenderCombo(m_genderCombo);
    
    m_notesEdit = new QTextEdit(this);
    m_notesEdit->setMaximumHeight(80);
    m_notesEdit->setPlaceholderText(tr("Dodatkowe uwagi (opcjonalne)"));

    // Dodawanie pól do layoutu
    m_formLayout->addRow(tr("PESEL:"), m_peselEdit);
    m_formLayout->addRow(tr("Imię:"), m_firstNameEdit);
    m_formLayout->addRow(tr("Nazwisko:"), m_lastNameEdit);
    m_formLayout->addRow(tr("Data urodzenia:"), m_birthDateEdit);
    m_formLayout->addRow(tr("Płeć:"), m_genderCombo);
    m_formLayout->addRow(tr("Uwagi:"), m_notesEdit);

    m_formGroup->setLayout(m_formLayout);
    m_mainLayout->addWidget(m_formGroup);

    // Przyciski formularza
    m_formButtonsLayout = new QHBoxLayout();
    m_formButtonsLayout->setSpacing(8);

    m_addButton = new QPushButton(tr("Dodaj"), this);
    m_addButton->setToolTip(tr("Dodaj nowego pacjenta"));
    
    m_editButton = new QPushButton(tr("Edytuj"), this);
    m_editButton->setToolTip(tr("Edytuj wybranego pacjenta"));
    m_editButton->setEnabled(false);
    
    m_deleteButton = new QPushButton(tr("Usuń"), this);
    m_deleteButton->setToolTip(tr("Usuń wybranego pacjenta"));
    m_deleteButton->setEnabled(false);
    
    m_clearButton = new QPushButton(tr("Wyczyść"), this);
    m_clearButton->setToolTip(tr("Wyczyść formularz"));

    m_formButtonsLayout->addWidget(m_addButton);
    m_formButtonsLayout->addWidget(m_editButton);
    m_formButtonsLayout->addWidget(m_deleteButton);
    m_formButtonsLayout->addWidget(m_clearButton);
    m_formButtonsLayout->addStretch();

    m_mainLayout->addLayout(m_formButtonsLayout);
}

void PatientTab::setupPatientTable()
{
    m_tableGroup = new QGroupBox(tr("Lista Pacjentów"), this);
    
    m_patientTable = new QTableWidget(this);
    m_patientTable->setColumnCount(6);
    m_patientTable->setHorizontalHeaderLabels({
        tr("PESEL"), tr("Imię"), tr("Nazwisko"), 
        tr("Data ur."), tr("Płeć"), tr("Uwagi")
    });
    
    // Konfiguracja tabeli
    m_patientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_patientTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_patientTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_patientTable->setAlternatingRowColors(true);
    m_patientTable->setSortingEnabled(true);
    m_patientTable->horizontalHeader()->setStretchLastSection(true);
    m_patientTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_patientTable->verticalHeader()->setVisible(false);
    
    // Domyślne szerokości kolumn
    m_patientTable->setColumnWidth(0, 120); // PESEL
    m_patientTable->setColumnWidth(1, 100); // Imię
    m_patientTable->setColumnWidth(2, 120); // Nazwisko
    m_patientTable->setColumnWidth(3, 100); // Data ur.
    m_patientTable->setColumnWidth(4, 80);  // Płeć
    // Kolumna 5 (Uwagi) rozciąga się
    
    m_tableGroup->setLayout(new QVBoxLayout());
    m_tableGroup->layout()->addWidget(m_patientTable);
    
    m_mainLayout->addWidget(m_tableGroup);
}

void PatientTab::setupButtons()
{
    m_actionButtonsLayout = new QHBoxLayout();
    m_actionButtonsLayout->setSpacing(8);

    m_exportButton = new QPushButton(tr("Eksportuj CSV"), this);
    m_exportButton->setToolTip(tr("Eksportuj dane pacjentów do pliku CSV"));
    
    m_importButton = new QPushButton(tr("Importuj CSV"), this);
    m_importButton->setToolTip(tr("Importuj dane pacjentów z pliku CSV"));
    
    m_refreshButton = new QPushButton(tr("Odśwież"), this);
    m_refreshButton->setToolTip(tr("Odśwież widok"));

    m_actionButtonsLayout->addWidget(m_exportButton);
    m_actionButtonsLayout->addWidget(m_importButton);
    m_actionButtonsLayout->addWidget(m_refreshButton);
    m_actionButtonsLayout->addStretch();

    m_mainLayout->addLayout(m_actionButtonsLayout);
}

void PatientTab::createConnections()
{
    connect(m_addButton, &QPushButton::clicked, this, &PatientTab::openAddPatientDialog);
    connect(m_editButton, &QPushButton::clicked, this, &PatientTab::editSelectedPatient);
    connect(m_deleteButton, &QPushButton::clicked, this, &PatientTab::deleteSelectedPatient);
    connect(m_clearButton, &QPushButton::clicked, this, [this]() {
        m_peselEdit->clear();
        m_firstNameEdit->clear();
        m_lastNameEdit->clear();
        m_notesEdit->clear();
        m_birthDateEdit->setDate(QDate::currentDate().addYears(-18));
        m_genderCombo->setCurrentIndex(0);
        m_isEditMode = false;
        m_addButton->setEnabled(true);
    });
    
    connect(m_exportButton, &QPushButton::clicked, this, &PatientTab::exportPatients);
    connect(m_importButton, &QPushButton::clicked, this, &PatientTab::importPatients);
    connect(m_refreshButton, &QPushButton::clicked, this, [this]() {
        // Odświeżenie widoku (możliwe rozszerzenie w przyszłości)
    });
    
    connect(m_patientTable, &QTableWidget::itemSelectionChanged, 
            this, &PatientTab::onSelectionChanged);
    connect(m_patientTable, &QTableWidget::cellDoubleClicked,
            this, &PatientTab::onTableRowDoubleClicked);
}

void PatientTab::fillGenderCombo(QComboBox* combo)
{
    combo->clear();
    combo->addItem(tr("Mężczyzna"), "M");
    combo->addItem(tr("Kobieta"), "K");
    combo->addItem(tr("Inna"), "I");
}

bool PatientTab::validatePesel(const QString& pesel)
{
    if (pesel.length() != 11)
        return false;
    
    for (const QChar& c : pesel) {
        if (!c.isDigit())
            return false;
    }
    
    // Prosta walidacja sumy kontrolnej PESEL
    int weights[] = {1, 3, 7, 9, 1, 3, 7, 9, 1, 3};
    int sum = 0;
    
    for (int i = 0; i < 10; ++i) {
        sum += pesel[i].digitValue() * weights[i];
    }
    
    int controlSum = sum % 10;
    int controlDigit = 10 - controlSum;
    if (controlDigit == 10)
        controlDigit = 0;
    
    return controlDigit == pesel[10].digitValue();
}

int PatientTab::findPatientByPesel(const QString& pesel)
{
    for (int row = 0; row < m_patientTable->rowCount(); ++row) {
        if (m_patientTable->item(row, 0)->text() == pesel) {
            return row;
        }
    }
    return -1;
}

void PatientTab::addPatient(const QString& pesel, const QString& firstName,
                           const QString& lastName, const QDate& birthDate,
                           const QString& gender)
{
    // Sprawdzenie czy PESEL już istnieje
    int existingRow = findPatientByPesel(pesel);
    if (existingRow >= 0 && !m_isEditMode) {
        QMessageBox::warning(this, tr("Błąd"), 
                            tr("Pacjent z tym numerem PESEL już istnieje."));
        return;
    }
    
    int row;
    if (m_isEditMode && existingRow >= 0) {
        row = existingRow;
    } else {
        row = m_patientTable->rowCount();
        m_patientTable->insertRow(row);
    }
    
    m_patientTable->setItem(row, 0, new QTableWidgetItem(pesel));
    m_patientTable->setItem(row, 1, new QTableWidgetItem(firstName));
    m_patientTable->setItem(row, 2, new QTableWidgetItem(lastName));
    m_patientTable->setItem(row, 3, new QTableWidgetItem(birthDate.toString("yyyy-MM-dd")));
    m_patientTable->setItem(row, 4, new QTableWidgetItem(gender));
    
    if (m_isEditMode) {
        emit patientUpdated(row);
        m_isEditMode = false;
        m_addButton->setText(tr("Dodaj"));
    } else {
        emit patientAdded(pesel, firstName, lastName);
    }
    
    // Wyczyszczenie formularza po dodaniu
    if (!m_isEditMode) {
        m_peselEdit->clear();
        m_firstNameEdit->clear();
        m_lastNameEdit->clear();
        m_notesEdit->clear();
        m_birthDateEdit->setDate(QDate::currentDate().addYears(-18));
        m_genderCombo->setCurrentIndex(0);
    }
}

int PatientTab::patientCount() const
{
    return m_patientTable->rowCount();
}

void PatientTab::clearAllPatients()
{
    if (m_patientTable->rowCount() > 0) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, tr("Potwierdzenie"),
            tr("Czy na pewno chcesz usunąć wszystkich pacjentów?"),
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            m_patientTable->setRowCount(0);
        }
    }
}

bool PatientTab::exportToCSV(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out.setCodec("UTF-8");
    
    // Nagłówek
    out << "PESEL,Imie,Nazwisko,Data_Urodzenia,Plec,Uwagi\n";
    
    // Dane
    for (int row = 0; row < m_patientTable->rowCount(); ++row) {
        QString pesel = m_patientTable->item(row, 0)->text();
        QString firstName = m_patientTable->item(row, 1)->text();
        QString lastName = m_patientTable->item(row, 2)->text();
        QString birthDate = m_patientTable->item(row, 3)->text();
        QString gender = m_patientTable->item(row, 4)->text();
        QString notes = m_patientTable->item(row, 5) ? 
                       m_patientTable->item(row, 5)->text() : "";
        
        // Escapowanie przecinków w danych
        notes.replace("\"", "\"\"");
        
        out << pesel << "," << firstName << "," << lastName << ","
            << birthDate << "," << gender << ",\"" << notes << "\"\n";
    }
    
    file.close();
    return true;
}

bool PatientTab::importFromCSV(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream in(&file);
    in.setCodec("UTF-8");
    
    // Pominięcie nagłówka
    QString line = in.readLine();
    
    int importedCount = 0;
    
    while (!in.atEnd()) {
        line = in.readLine();
        if (line.isEmpty())
            continue;
        
        // Proste parsowanie CSV (zakłada brak przecinków w polach)
        QStringList fields = line.split(',');
        
        if (fields.size() >= 5) {
            QString pesel = fields[0].trimmed();
            QString firstName = fields[1].trimmed();
            QString lastName = fields[2].trimmed();
            QDate birthDate = QDate::fromString(fields[3].trimmed(), "yyyy-MM-dd");
            QString gender = fields[4].trimmed();
            
            if (!pesel.isEmpty() && !firstName.isEmpty() && !lastName.isEmpty()) {
                addPatient(pesel, firstName, lastName, birthDate, gender);
                importedCount++;
            }
        }
    }
    
    file.close();
    return importedCount > 0;
}

void PatientTab::openAddPatientDialog()
{
    m_isEditMode = false;
    m_addButton->setText(tr("Dodaj"));
    
    // Walidacja danych
    QString pesel = m_peselEdit->text().trimmed();
    QString firstName = m_firstNameEdit->text().trimmed();
    QString lastName = m_lastNameEdit->text().trimmed();
    QDate birthDate = m_birthDateEdit->date();
    QString gender = m_genderCombo->currentData().toString();
    
    if (pesel.isEmpty()) {
        QMessageBox::warning(this, tr("Błąd"), tr("Numer PESEL jest wymagany."));
        m_peselEdit->setFocus();
        return;
    }
    
    if (!validatePesel(pesel)) {
        QMessageBox::warning(this, tr("Błąd"), 
                            tr("Nieprawidłowy numer PESEL. Sprawdź poprawność."));
        m_peselEdit->setFocus();
        return;
    }
    
    if (firstName.isEmpty()) {
        QMessageBox::warning(this, tr("Błąd"), tr("Imię jest wymagane."));
        m_firstNameEdit->setFocus();
        return;
    }
    
    if (lastName.isEmpty()) {
        QMessageBox::warning(this, tr("Błąd"), tr("Nazwisko jest wymagane."));
        m_lastNameEdit->setFocus();
        return;
    }
    
    addPatient(pesel, firstName, lastName, birthDate, gender);
}

void PatientTab::editSelectedPatient()
{
    int currentRow = m_patientTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::information(this, tr("Informacja"), 
                                tr("Wybierz pacjenta z listy do edycji."));
        return;
    }
    
    // Wczytanie danych do formularza
    m_peselEdit->setText(m_patientTable->item(currentRow, 0)->text());
    m_firstNameEdit->setText(m_patientTable->item(currentRow, 1)->text());
    m_lastNameEdit->setText(m_patientTable->item(currentRow, 2)->text());
    
    QDate birthDate = QDate::fromString(m_patientTable->item(currentRow, 3)->text(), "yyyy-MM-dd");
    if (birthDate.isValid()) {
        m_birthDateEdit->setDate(birthDate);
    }
    
    QString gender = m_patientTable->item(currentRow, 4)->text();
    int genderIndex = m_genderCombo->findData(gender);
    if (genderIndex >= 0) {
        m_genderCombo->setCurrentIndex(genderIndex);
    }
    
    if (m_patientTable->item(currentRow, 5)) {
        m_notesEdit->setText(m_patientTable->item(currentRow, 5)->text());
    }
    
    m_isEditMode = true;
    m_addButton->setText(tr("Zapisz"));
    m_currentRow = currentRow;
    
    m_peselEdit->setFocus();
}

void PatientTab::deleteSelectedPatient()
{
    int currentRow = m_patientTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::information(this, tr("Informacja"), 
                                tr("Wybierz pacjenta z listy do usunięcia."));
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Potwierdzenie"),
        tr("Czy na pewno chcesz usunąć wybranego pacjenta?"),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        m_patientTable->removeRow(currentRow);
        emit patientRemoved(currentRow);
        
        // Wyczyszczenie formularza
        m_peselEdit->clear();
        m_firstNameEdit->clear();
        m_lastNameEdit->clear();
        m_notesEdit->clear();
    }
}

void PatientTab::exportPatients()
{
    if (m_patientTable->rowCount() == 0) {
        QMessageBox::information(this, tr("Informacja"), 
                                tr("Brak pacjentów do eksportu."));
        return;
    }
    
    QString defaultName = QString("pacjenci_%1.csv").arg(
        QDate::currentDate().toString("yyyy-MM-dd"));
    
    QString filename = QFileDialog::getSaveFileName(
        this, tr("Eksportuj dane pacjentów"), 
        defaultName,
        tr("Pliki CSV (*.csv);;Wszystkie pliki (*)"));
    
    if (filename.isEmpty())
        return;
    
    if (!QFileInfo(filename).suffix().toLower().isEmpty() && 
        !filename.endsWith(".csv", Qt::CaseInsensitive)) {
        filename += ".csv";
    }
    
    if (exportToCSV(filename)) {
        QMessageBox::information(this, tr("Sukces"), 
                                tr("Dane pacjentów zostały wyeksportowane."));
    } else {
        QMessageBox::critical(this, tr("Błąd"), 
                             tr("Nie udało się wyeksportować danych."));
    }
}

void PatientTab::importPatients()
{
    QString filename = QFileDialog::getOpenFileName(
        this, tr("Importuj dane pacjentów"), 
        QString(),
        tr("Pliki CSV (*.csv);;Wszystkie pliki (*)"));
    
    if (filename.isEmpty())
        return;
    
    if (importFromCSV(filename)) {
        QMessageBox::information(this, tr("Sukces"), 
                                tr("Dane pacjentów zostały zaimportowane."));
    } else {
        QMessageBox::critical(this, tr("Błąd"), 
                             tr("Nie udało się zaimportować danych."));
    }
}

void PatientTab::clearTable()
{
    clearAllPatients();
}

void PatientTab::onSelectionChanged()
{
    bool hasSelection = m_patientTable->currentRow() >= 0;
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
}

void PatientTab::onTableRowDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    
    if (row >= 0) {
        m_patientTable->selectRow(row);
        editSelectedPatient();
    }
}

} // namespace tab
