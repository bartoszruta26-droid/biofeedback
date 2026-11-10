#include "tab/PatientTab.hpp"
#include "core/RuntimePaths.hpp"
#include <QFileInfo>
#include <QTextStream>
#include <QDateTime>
#include <QUuid>
#include <QDir>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>

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
    , m_icfButton(nullptr)
    , m_icfHistoryButton(nullptr)
    , m_tableGroup(nullptr)
    , m_patientTable(nullptr)
    , m_actionButtonsLayout(nullptr)
    , m_exportButton(nullptr)
    , m_importButton(nullptr)
    , m_refreshButton(nullptr)
    , m_currentRow(-1)
    , m_isEditMode(false)
    , m_currentPatientPesel("")
    , m_currentPatientDataPath("")
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
    m_mainLayout->setSpacing(15);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);

    setupPatientForm();
    setupPatientTable();
    setupButtons();
}

void PatientTab::setupPatientForm()
{
    m_formGroup = new QGroupBox(tr("Dane Pacjenta"), this);
    m_formGroup->setFont(QFont("Arial", 8, QFont::Bold));
    m_formLayout = new QFormLayout();
    m_formLayout->setSpacing(12);
    m_formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    // Pola formularza
    m_peselEdit = new QLineEdit(this);
    m_peselEdit->setMaxLength(11);
    m_peselEdit->setPlaceholderText(tr("11 cyfr"));
    m_peselEdit->setMinimumHeight(30);
    m_peselEdit->setFont(QFont("Arial", 7));
    
    m_firstNameEdit = new QLineEdit(this);
    m_firstNameEdit->setPlaceholderText(tr("Imię"));
    m_firstNameEdit->setMinimumHeight(30);
    m_firstNameEdit->setFont(QFont("Arial", 7));
    
    m_lastNameEdit = new QLineEdit(this);
    m_lastNameEdit->setPlaceholderText(tr("Nazwisko"));
    m_lastNameEdit->setMinimumHeight(30);
    m_lastNameEdit->setFont(QFont("Arial", 7));
    
    m_birthDateEdit = new QDateEdit(this);
    m_birthDateEdit->setCalendarPopup(true);
    m_birthDateEdit->setDisplayFormat("yyyy-MM-dd");
    m_birthDateEdit->setDate(QDate::currentDate().addYears(-18));
    m_birthDateEdit->setMinimumHeight(30);
    m_birthDateEdit->setFont(QFont("Arial", 7));
    
    m_genderCombo = new QComboBox(this);
    fillGenderCombo(m_genderCombo);
    m_genderCombo->setMinimumHeight(30);
    m_genderCombo->setFont(QFont("Arial", 7));
    
    m_notesEdit = new QTextEdit(this);
    m_notesEdit->setMaximumHeight(80);
    m_notesEdit->setMinimumHeight(60);
    m_notesEdit->setPlaceholderText(tr("Dodatkowe uwagi (opcjonalne)"));
    m_notesEdit->setFont(QFont("Arial", 7));

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
    m_formButtonsLayout->setSpacing(12);

    m_addButton = new QPushButton(tr("Dodaj"), this);
    m_addButton->setToolTip(tr("Dodaj nowego pacjenta"));
    m_addButton->setMinimumHeight(35);
    m_addButton->setFont(QFont("Arial", 7, QFont::Bold));
    
    m_editButton = new QPushButton(tr("Edytuj"), this);
    m_editButton->setToolTip(tr("Edytuj wybranego pacjenta"));
    m_editButton->setEnabled(false);
    m_editButton->setMinimumHeight(35);
    m_editButton->setFont(QFont("Arial", 7));
    
    m_deleteButton = new QPushButton(tr("Usuń"), this);
    m_deleteButton->setToolTip(tr("Usuń wybranego pacjenta"));
    m_deleteButton->setEnabled(false);
    m_deleteButton->setMinimumHeight(35);
    m_deleteButton->setFont(QFont("Arial", 7));
    
    m_clearButton = new QPushButton(tr("Wyczyść"), this);
    m_clearButton->setToolTip(tr("Wyczyść formularz"));
    m_clearButton->setMinimumHeight(35);
    m_clearButton->setFont(QFont("Arial", 7));

    m_formButtonsLayout->addWidget(m_addButton);
    m_formButtonsLayout->addWidget(m_editButton);
    m_formButtonsLayout->addWidget(m_deleteButton);
    m_formButtonsLayout->addWidget(m_clearButton);
    m_icfButton = new QPushButton(tr("Ocena funkcjonalna ICF"), this);
    m_icfButton->setToolTip(tr("Otwórz ustrukturyzowany wywiad stanu funkcjonalnego według ICF"));
    m_icfButton->setMinimumHeight(35);
    m_icfButton->setEnabled(false);
    m_formButtonsLayout->addWidget(m_icfButton);
    m_icfHistoryButton = new QPushButton(tr("Historia ICF"), this);
    m_icfHistoryButton->setToolTip(tr("Przeglądaj wcześniejsze oceny funkcjonalne pacjenta"));
    m_icfHistoryButton->setMinimumHeight(35);
    m_icfHistoryButton->setEnabled(false);
    m_formButtonsLayout->addWidget(m_icfHistoryButton);
    m_formButtonsLayout->addStretch();

    m_mainLayout->addLayout(m_formButtonsLayout);
}

void PatientTab::setupPatientTable()
{
    m_tableGroup = new QGroupBox(tr("Lista Pacjentów"), this);
    m_tableGroup->setFont(QFont("Arial", 8, QFont::Bold));
    
    m_patientTable = new QTableWidget(this);
    m_patientTable->setColumnCount(6);
    m_patientTable->setHorizontalHeaderLabels({
        tr("PESEL"), tr("Imię"), tr("Nazwisko"), 
        tr("Data ur."), tr("Płeć"), tr("Uwagi")
    });
    m_patientTable->setFont(QFont("Arial", 7));
    
    // Konfiguracja tabeli
    m_patientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_patientTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_patientTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_patientTable->setAlternatingRowColors(true);
    m_patientTable->setSortingEnabled(true);
    m_patientTable->horizontalHeader()->setStretchLastSection(true);
    m_patientTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_patientTable->verticalHeader()->setVisible(false);
    m_patientTable->setMinimumHeight(250);
    
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
    m_actionButtonsLayout->setSpacing(12);

    m_exportButton = new QPushButton(tr("Eksportuj CSV"), this);
    m_exportButton->setToolTip(tr("Eksportuj dane pacjentów do pliku CSV"));
    m_exportButton->setMinimumHeight(35);
    m_exportButton->setFont(QFont("Arial", 7));
    
    m_importButton = new QPushButton(tr("Importuj CSV"), this);
    m_importButton->setToolTip(tr("Importuj dane pacjentów z pliku CSV"));
    m_importButton->setMinimumHeight(35);
    m_importButton->setFont(QFont("Arial", 7));
    
    m_refreshButton = new QPushButton(tr("Odśwież"), this);
    m_refreshButton->setToolTip(tr("Odśwież widok"));
    m_refreshButton->setMinimumHeight(35);
    m_refreshButton->setFont(QFont("Arial", 7));

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
    connect(m_icfButton, &QPushButton::clicked, this, &PatientTab::openIcfAssessmentDialog);
    connect(m_icfHistoryButton, &QPushButton::clicked, this, &PatientTab::openIcfAssessmentHistoryDialog);
}

void PatientTab::setPatientIcfAssessment(const QString& pesel, const QJsonObject& assessment, const QJsonArray& history)
{
    const QString key = pesel.trimmed();
    if (key.isEmpty()) return;
    if (assessment.isEmpty()) m_icfAssessments.remove(key);
    else m_icfAssessments.insert(key, assessment);
    m_icfAssessmentHistories.insert(key, history);
}

void PatientTab::openIcfAssessmentHistoryDialog()
{
    const QString pesel = getCurrentPatientPesel().trimmed();
    if (pesel.isEmpty()) return;

    const QJsonArray history = m_icfAssessmentHistories.value(pesel);
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Historia ocen funkcjonalnych ICF"));
    dialog.resize(760, 480);
    auto* layout = new QVBoxLayout(&dialog);
    auto* table = new QTableWidget(&dialog);
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({tr("Data oceny"), tr("Ocenione domeny"), tr("Opis / priorytety")});
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    table->setRowCount(history.size());
    for (int row = 0; row < history.size(); ++row) {
        const QJsonObject assessment = history.at(row).toObject();
        const QDateTime date = QDateTime::fromString(
            assessment.value(QStringLiteral("assessed_at")).toString(), Qt::ISODateWithMs);
        const QJsonObject responses = assessment.value(QStringLiteral("responses")).toObject();
        int assessed = 0;
        for (auto it = responses.constBegin(); it != responses.constEnd(); ++it) {
            const QJsonObject response = it.value().toObject();
            const int qualifier = response.contains(QStringLiteral("qualifier"))
                ? response.value(QStringLiteral("qualifier")).toInt(-99)
                : response.value(QStringLiteral("performance")).toInt(-99);
            const int capacity = response.value(QStringLiteral("capacity")).toInt(-99);
            const int extent = response.value(QStringLiteral("extent")).toInt(-99);
            if (qualifier != -99 || capacity != -99 || extent != -99) ++assessed;
        }
        table->setItem(row, 0, new QTableWidgetItem(
            date.isValid() ? date.toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm")) : tr("Brak daty")));
        table->setItem(row, 1, new QTableWidgetItem(QString::number(assessed)));
        table->setItem(row, 2, new QTableWidgetItem(
            assessment.value(QStringLiteral("narrative")).toString()));
    }
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(table);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    buttons->button(QDialogButtonBox::Close)->setText(tr("Zamknij"));
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (history.isEmpty())
        layout->insertWidget(0, new QLabel(tr("Brak zapisanych ocen ICF."), &dialog));
    dialog.exec();
}

void PatientTab::openIcfAssessmentDialog()
{
    const QString pesel = getCurrentPatientPesel().trimmed();
    if (pesel.isEmpty()) {
        QMessageBox::information(this, tr("Ocena ICF"), tr("Najpierw wybierz pacjenta z listy."));
        return;
    }

    struct Question { const char* code; const char* label; int domain; };
    static const Question questions[] = {
        {"b130", "Czy występuje obniżenie energii, napędu lub zwiększona męczliwość?", 0},
        {"b134", "Czy sen jest zaburzony i wpływa na codzienne funkcjonowanie?", 0}, {"b152", "Czy występują trudności w regulacji emocji?", 0},
        {"b280", "Czy pacjent odczuwa ból? Jak często i jak silny?", 0}, {"b710", "Czy zakres lub płynność ruchu stawów są ograniczone?", 0},
        {"b730", "Czy osłabienie siły mięśni utrudnia czynności?", 0}, {"b455", "Czy wysiłek powoduje nadmierną duszność, zmęczenie lub ograniczenie aktywności?", 0},
        {"d410", "Czy pacjent samodzielnie zmienia pozycję: siada, wstaje, kładzie się?", 1},
        {"d415", "Czy pacjent utrzymuje pozycję siedzącą i stojącą bez podparcia?", 1}, {"d430", "Czy podnosi i przenosi przedmioty codziennego użytku?", 1},
        {"d450", "Czy chodzi po domu i poza domem? Jaki dystans pokonuje?", 1}, {"d455", "Czy pokonuje schody, wzniesienia i inne przeszkody?", 1},
        {"d510", "Czy samodzielnie myje całe ciało?", 1}, {"d520", "Czy samodzielnie wykonuje codzienną pielęgnację ciała?", 1},
        {"d540", "Czy samodzielnie zakłada i zdejmuje ubranie oraz obuwie?", 1}, {"d550", "Czy samodzielnie spożywa posiłki?", 1}, {"d560", "Czy samodzielnie przyjmuje napoje?", 1},
        {"d640", "Czy wykonuje typowe prace domowe i obowiązki?", 1}, {"d850", "Czy stan zdrowia ogranicza wykonywanie pracy zarobkowej?", 1},
        {"d920", "Czy uczestniczy w rekreacji, hobby i życiu społecznym?", 1},
        {"e115", "Czy sprzęt i technologie codziennego użytku pomagają, czy utrudniają funkcjonowanie?", 2},
        {"e120", "Czy dostępne pomoce do przemieszczania się są odpowiednie i dostępne?", 2},
        {"e150", "Czy bariery architektoniczne ograniczają dostęp do budynków i usług?", 2},
        {"e310", "Czy rodzina zapewnia potrzebne wsparcie?", 2}, {"e355", "Czy personel ochrony zdrowia zapewnia dostępne i odpowiednie wsparcie?", 2},
        {"e580", "Czy dostęp do świadczeń zdrowotnych wspiera proces leczenia i rehabilitacji?", 2},
        {"s730", "Czy występują zmiany struktury kończyny górnej?", 3},
        {"s750", "Czy występują zmiany struktury kończyny dolnej?", 3},
        {"s760", "Czy występują zmiany struktury tułowia lub kręgosłupa?", 3},
        {"s770", "Czy występują inne zmiany struktur układu mięśniowo-szkieletowego?", 3}
    };

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Ocena stanu funkcjonalnego — ICF"));
    dialog.resize(820, 760);
    auto* root = new QVBoxLayout(&dialog);
    auto* intro = new QLabel(
        tr("Pytania przesiewowo-opisowe oparte na domenach ICF. Oceń funkcjonowanie pacjenta "
           "w typowym okresie ostatnich 30 dni, uwzględniając jego własną perspektywę. "
           "Kwestionariusz wspiera opis kliniczny i nie zastępuje pełnego, standaryzowanego Core Set ICF."),
        &dialog);
    intro->setWordWrap(true);
    root->addWidget(intro);

    auto* scroll = new QScrollArea(&dialog);
    scroll->setWidgetResizable(true);
    auto* content = new QWidget(scroll);
    auto* contentLayout = new QVBoxLayout(content);
    const QJsonObject previous = m_icfAssessments.value(pesel);
    const QJsonObject previousResponses = previous.value(QStringLiteral("responses")).toObject();
    QHash<QString, QComboBox*> qualifiers, capacities, performances;
    QHash<QString, QComboBox*> structureExtent, structureNature, structureLocation;

    auto addDomain = [&](int domain, const QString& title) {
        auto* group = new QGroupBox(title, content);
        auto* form = new QFormLayout(group);
        form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        contentLayout->addWidget(group);
        for (const Question& question : questions) {
            if (question.domain != domain) continue;
            const QString code = QString::fromLatin1(question.code);
            const QString label = QStringLiteral("%1 — %2").arg(code, QString::fromUtf8(question.label));
            const QJsonObject old = previousResponses.value(code).toObject();
            if (domain == 3) {
                auto* row = new QWidget(group);
                auto* rowLayout = new QHBoxLayout(row);
                rowLayout->setContentsMargins(0, 0, 0, 0);
                auto makeQualifier = [&](const QStringList& labels, const QList<int>& values,
                                         const QString& key, int fallback) {
                    auto* combo = new QComboBox(row);
                    for (int i = 0; i < labels.size(); ++i)
                        combo->addItem(labels.at(i), values.at(i));
                    combo->setCurrentIndex(combo->findData(old.value(key).toInt(fallback)));
                    return combo;
                };
                const QStringList extentLabels = {tr("Nie oceniono"), tr("0 — brak"), tr("1 — niewielkie"),
                    tr("2 — umiarkowane"), tr("3 — znaczne"), tr("4 — całkowite"),
                    tr("8 — nieokreślone"), tr("9 — nie dotyczy")};
                const QList<int> extentValues = {-99, 0, 1, 2, 3, 4, 8, 9};
                const QStringList natureLabels = {tr("Nie oceniono"), tr("0 — brak zmiany"),
                    tr("1 — brak części"), tr("2 — część dodatkowa"), tr("3 — nieprawidłowy wymiar"),
                    tr("4 — przerwanie ciągłości"), tr("5 — przemieszczenie"), tr("6 — odchylenie położenia"),
                    tr("7 — zmiana jakościowa"), tr("8 — nieokreślona"), tr("9 — nie dotyczy")};
                const QList<int> natureValues = {-99, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
                const QStringList locationLabels = {tr("Nie oceniono"), tr("0 — więcej niż jeden obszar"),
                    tr("1 — prawa strona"), tr("2 — lewa strona"), tr("3 — obustronnie"),
                    tr("4 — z przodu"), tr("5 — z tyłu"), tr("6 — bliżej tułowia"),
                    tr("7 — dalej od tułowia"), tr("8 — nieokreślona"), tr("9 — nie dotyczy")};
                const QList<int> locationValues = {-99, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
                auto* extent = makeQualifier(extentLabels, extentValues, QStringLiteral("extent"),
                    old.value(QStringLiteral("qualifier")).toInt(-99));
                auto* nature = makeQualifier(natureLabels, natureValues, QStringLiteral("nature"), -99);
                auto* location = makeQualifier(locationLabels, locationValues, QStringLiteral("location"), -99);
                rowLayout->addWidget(new QLabel(tr("Zakres:"), row));
                rowLayout->addWidget(extent, 1);
                rowLayout->addWidget(new QLabel(tr("Charakter:"), row));
                rowLayout->addWidget(nature, 1);
                rowLayout->addWidget(new QLabel(tr("Lokalizacja:"), row));
                rowLayout->addWidget(location, 1);
                form->addRow(label, row);
                structureExtent.insert(code, extent);
                structureNature.insert(code, nature);
                structureLocation.insert(code, location);
            } else if (domain == 1) {
                auto* row = new QWidget(group);
                auto* rowLayout = new QHBoxLayout(row);
                rowLayout->setContentsMargins(0, 0, 0, 0);
                auto* performance = new QComboBox(row);
                auto* capacity = new QComboBox(row);
                const QStringList optionLabels = {
                    tr("Nie oceniono"), tr("0 — brak trudności"), tr("1 — trudność niewielka"),
                    tr("2 — trudność umiarkowana"), tr("3 — trudność znaczna"),
                    tr("4 — trudność całkowita"), tr("8 — nieokreślona"),
                    tr("9 — nie dotyczy")
                };
                const QList<int> optionValues = {-99, 0, 1, 2, 3, 4, 8, 9};
                for (int i = 0; i < optionLabels.size(); ++i) {
                    performance->addItem(optionLabels.at(i), optionValues.at(i));
                    capacity->addItem(optionLabels.at(i), optionValues.at(i));
                }
                performance->setCurrentIndex(performance->findData(
                    old.value(QStringLiteral("performance")).toInt(-99)));
                capacity->setCurrentIndex(capacity->findData(
                    old.value(QStringLiteral("capacity")).toInt(-99)));
                rowLayout->addWidget(new QLabel(tr("Wykonanie:"), row));
                rowLayout->addWidget(performance, 1);
                rowLayout->addWidget(new QLabel(tr("Zdolność:"), row));
                rowLayout->addWidget(capacity, 1);
                form->addRow(label, row);
                performances.insert(code, performance);
                capacities.insert(code, capacity);
            } else {
                auto* combo = new QComboBox(group);
                if (domain == 2) {
                    const QStringList labels = {tr("Nie oceniono"), tr("0 — brak bariery i ułatwienia"), tr("-1 — niewielka bariera"),
                        tr("-2 — umiarkowana bariera"), tr("-3 — znaczna bariera"),
                        tr("-4 — całkowita bariera"), tr("+1 — niewielki ułatwiacz"),
                        tr("+2 — umiarkowany ułatwiacz"), tr("+3 — znaczny ułatwiacz"),
                        tr("+4 — zasadniczy ułatwiacz"), tr("-8 — bariera, stopień nieokreślony"),
                        tr("+8 — ułatwiacz, stopień nieokreślony"), tr("9 — nie dotyczy")};
                    const QList<int> values = {-99, 0, -1, -2, -3, -4, 1, 2, 3, 4, -8, 8, 9};
                    for (int i = 0; i < labels.size(); ++i) combo->addItem(labels.at(i), values.at(i));
                    combo->setCurrentIndex(combo->findData(old.value(QStringLiteral("qualifier")).toInt(-99)));
                } else {
                    const QStringList labels = {tr("Nie oceniono"), tr("0 — brak problemu"), tr("1 — problem niewielki"),
                        tr("2 — problem umiarkowany"), tr("3 — problem znaczny"),
                        tr("4 — problem całkowity"), tr("8 — nieokreślony"), tr("9 — nie dotyczy")};
                    const QList<int> values = {-99, 0, 1, 2, 3, 4, 8, 9};
                    for (int i = 0; i < labels.size(); ++i) combo->addItem(labels.at(i), values.at(i));
                    combo->setCurrentIndex(combo->findData(old.value(QStringLiteral("qualifier")).toInt(-99)));
                }
                form->addRow(label, combo);
                qualifiers.insert(code, combo);
            }
        }
    };
    addDomain(0, tr("Funkcje organizmu — kody b"));
    addDomain(1, tr("Aktywność i uczestniczenie — kody d"));
    addDomain(2, tr("Czynniki środowiskowe — kody e"));
    addDomain(3, tr("Struktury ciała — kody s"));

    auto* notesGroup = new QGroupBox(tr("Kontekst, mocne strony i priorytety pacjenta"), content);
    auto* notesLayout = new QVBoxLayout(notesGroup);
    auto* notes = new QTextEdit(notesGroup);
    notes->setPlaceholderText(tr("Czynniki osobiste (np. wiek, nawyki, motywacja), priorytety, cele, zasoby i oczekiwania pacjenta"));
    notes->setPlainText(previous.value(QStringLiteral("narrative")).toString());
    notes->setMinimumHeight(90);
    notesLayout->addWidget(notes);
    contentLayout->addWidget(notesGroup);
    contentLayout->addStretch(1);
    scroll->setWidget(content);
    root->addWidget(scroll, 1);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    buttons->button(QDialogButtonBox::Save)->setText(tr("Zapisz ocenę"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("Anuluj"));
    root->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) return;

    QJsonObject responses;
    for (auto it = qualifiers.constBegin(); it != qualifiers.constEnd(); ++it)
        responses.insert(it.key(), QJsonObject{{QStringLiteral("qualifier"), it.value()->currentData().toInt()}});
    for (auto it = performances.constBegin(); it != performances.constEnd(); ++it)
        responses.insert(it.key(), QJsonObject{
            {QStringLiteral("performance"), it.value()->currentData().toInt()},
            {QStringLiteral("capacity"), capacities.value(it.key())->currentData().toInt()}
        });
    for (auto it = structureExtent.constBegin(); it != structureExtent.constEnd(); ++it)
        responses.insert(it.key(), QJsonObject{
            {QStringLiteral("extent"), it.value()->currentData().toInt()},
            {QStringLiteral("nature"), structureNature.value(it.key())->currentData().toInt()},
            {QStringLiteral("location"), structureLocation.value(it.key())->currentData().toInt()}
        });

    const QJsonObject assessment{
        {QStringLiteral("id"), QUuid::createUuid().toString(QUuid::WithoutBraces)},
        {QStringLiteral("schema"), QStringLiteral("icf-functional-screening-v1")},
        {QStringLiteral("assessed_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
        {QStringLiteral("reference_period_days"), 30},
        {QStringLiteral("responses"), responses},
        {QStringLiteral("narrative"), notes->toPlainText().trimmed()}
    };
    m_icfAssessments.insert(pesel, assessment);
    emit patientIcfAssessmentChanged(pesel, assessment);
    QMessageBox::information(this, tr("Ocena ICF"),
        tr("Ocena została przekazana do zapisania w kartotece pacjenta."));
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
                           const QString& gender, const QString& notes)
{
    // Preserve the form notes for interactive entry; explicit notes are used by storage reload.
    const QString noteText = notes.isNull() ? m_notesEdit->toPlainText() : notes;
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
    m_patientTable->setItem(row, 5, new QTableWidgetItem(noteText));
    
    if (m_isEditMode) {
        emit patientUpdated(row);
        m_isEditMode = false;
        m_addButton->setText(tr("Dodaj"));
    } else {
        emit patientAdded(pesel, firstName, lastName);
    }
    emit patientRecordChanged(pesel, firstName, lastName, birthDate, gender, noteText);
    
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
            for (int row = 0; row < m_patientTable->rowCount(); ++row)
                emit patientRecordDeleted(m_patientTable->item(row, 0)->text());
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
        const QString deletedPesel = m_patientTable->item(currentRow, 0)->text();
        m_patientTable->removeRow(currentRow);
        emit patientRemoved(currentRow);
        emit patientRecordDeleted(deletedPesel);
        
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
    int currentRow = m_patientTable->currentRow();
    bool hasSelection = currentRow >= 0;
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
    m_icfButton->setEnabled(hasSelection);
    m_icfHistoryButton->setEnabled(hasSelection);
    
    // Emituj sygnał patientSelected gdy wybrano pacjenta
    if (hasSelection) {
        QString pesel = m_patientTable->item(currentRow, 0)->text();
        QString firstName = m_patientTable->item(currentRow, 1)->text();
        QString lastName = m_patientTable->item(currentRow, 2)->text();
        
        // Ustaw aktualnie załadowanego pacjenta
        m_currentPatientPesel = pesel;
        m_currentPatientDataPath = biofeedback::runtime::patientDirectory(pesel);
        
        emit patientSelected(pesel, firstName, lastName);
        emit currentPatientChanged(m_currentPatientPesel, m_currentPatientDataPath);
    } else if (!m_currentPatientPesel.isEmpty()) {
        m_currentPatientPesel.clear();
        m_currentPatientDataPath.clear();
        emit currentPatientChanged(QString(), QString());
    }
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
