# Shared Data Server — architecture and migration plan

## 1. Decisions

- Patient records are persisted either as files on disk or in MariaDB. PostgreSQL is not a project requirement.
- The selected patient backend is configured by the server, not independently by each client.
- The file backend is a supported production option, not a temporary cache. Patient records must be encrypted at rest, written atomically, protected by restrictive filesystem permissions, and addressed by opaque identifiers rather than PESEL or names.
- MariaDB is an alternative backend. Its implementation must use parameterized statements, transactions, schema migrations, least-privilege credentials, encrypted transport where remote, and explicit backup/restore procedures.
- Measurements, training sessions/results, and game sessions/results are owned by the shared data service. Device acquisition remains a separate responsibility.
- Linux Desktop, Windows Desktop, WebUI, TUI, and Android are peers. They use the same versioned API and must not access the server's storage files or database directly.
- No antenna, electrode, or IR-PWM feature belongs to this application. Those are unrelated legacy artifacts and must not be added to the Biofeedback roadmap.

## 2. Process boundaries

### Data Server

Owns:
- Patient repository and patient identity mapping.
- Measurement result repository and metadata.
- Training plans, sessions, checkpoints, and results.
- Game sessions, scores, and result history.
- Authentication/authorization at resource boundaries.
- Audit events, concurrency/version checks, idempotent writes, backup/restore and data export.

Exposes:
- REST/HTTPS API for CRUD and history queries.
- WebSocket (or Server-Sent Events where simpler) for live status and result notifications.
- A health/status endpoint that reports configured storage backend and readiness without exposing secrets or patient data.

### Device/measurement acquisition

Owns:
- Serial/network communication with Arduino Nano, Raspberry Pi Pico and supported sensors.
- Protocol parsing, calibration, timestamp normalization, validation and signal quality.
- Live sample delivery to the application/data service.

It does not own the patient master record or the durable cross-client result history. A validated sample is associated with a patient/session by the application layer and persisted through the Data Server.

### Clients

Desktop Linux, Desktop Windows, WebUI, TUI and Android:
- Read and write patient, measurement, training and game data through the API.
- Maintain only transient presentation state and explicitly bounded offline queues.
- Display conflict/offline states; never silently overwrite newer records.
- Do not embed database credentials or connect directly to MariaDB.

## 3. Storage strategy

### Patient files backend

Use one opaque patient key per record under the application data directory. Do not put PESEL, name, date of birth or other direct identifiers in filenames or directory names. Encrypt each record with authenticated encryption (AES-256-GCM or an equivalently reviewed AEAD construction); keep keys outside the repository and outside patient files. Write to a permission-restricted temporary file, flush, then atomically replace the target. Validate JSON/schema and record identity on read. Keep audit metadata separate from clinical content.

### MariaDB backend

Implement behind the same repository interface as the file backend. Use migrations and explicit schema versioning. Required logical entities include patients, measurements, training_plans, training_sessions, training_results, game_sessions, game_results, users/roles, audit_events, and idempotency_keys. Store patient identifiers separately from clinical payloads where practical. Enforce foreign keys, indexes, unique constraints and transactions.

### Configuration

The backend is selected in `config/config.json`:
- `storage.patient_backend: "files"` selects local patient files (current default).
- `storage.patient_backend: "mariadb"` selects the intended MariaDB backend.

MariaDB connection settings are grouped under `storage.mariadb`: `host`, `port`, `database`, `username`, `password_env`, `tls` and `connect_timeout_seconds`. The password itself must be supplied through the named environment variable (default: `BIOFEEDBACK_MARIADB_PASSWORD`), never stored in JSON or committed. The configuration manager validates the backend name and required MariaDB connection metadata.

**Implementation status:** the file backend is currently wired into the Desktop application. The MariaDB repository adapter and driver integration are not implemented yet. If `mariadb` is selected today, Desktop startup fails explicitly and does not fall back to files. This prevents accidental split-brain storage. The next storage milestone is the repository contract and MariaDB adapter, followed by shared server integration.

## 4. API contract

Wersjonowanie endpointów: `/api/v1`. Wdrożone kolekcje CRUD:
- `patients`
- `measurements`
- `training-sessions`
- `training-results`
- `game-sessions`
- `game-results`

Każda kolekcja udostępnia `GET /{collection}` i `POST /{collection}`;
rekord udostępnia `GET /{collection}/{id}`, `PUT /{collection}/{id}`
i `DELETE /{collection}/{id}`. Identyfikatory tworzy serwer jako UUID.
Rekord zawiera `id`, `version`, `created_at` i `updated_at`.
Aktualizacja/usunięcie wymagają `expected_version`; niezgodna wersja zwraca
HTTP 409. Ciało rekordu jest ograniczone do 16 MiB.

Do czasu wdrożenia zakresów RBAC wszystkie operacje na kolekcjach wymagają
aktywnej sesji użytkownika z rolą `administrator`. Nie jest to docelowy model
uprawnień klinicznych. Wymagane są dalsze zakresy per rola, pacjent i relacja
terapeuta-pacjent, rejestr audytowy z aktorem i wynikiem operacji oraz klucze
idempotencji. Nie logować danych pacjenta ani sekretów.

Lista kolekcji jest ograniczona do jawnej listy dozwolonych nazw; identyfikatory
nie są ścieżkami ani numerami PESEL. Repozytorium sprawdza format JSON, limit
rozmiaru, wersję rekordu i integralność identyfikatora. Walidacja domenowa
jednostek, timestampów i relacji pacjent/sesja musi być rozszerzana wraz
z integracją producentów danych. WebSocket/SSE dla zdarzeń pozostaje
niezaimplementowany.

## 5. Migration sequence

1. Repair and stabilize the current main branch regression suite. Preserve the existing patient files and CSV import/export behavior.
2. Define repository interfaces and shared domain DTOs for patient, measurement, training and game data. Add contract tests independent of the storage backend.
3. Implement the file-backed patient repository using the existing encrypted file policy; add safe list/get/create/update/delete, atomic writes, version checks and migration/import tests.
4. Implement the Data Server process and inject repositories into the HTTP API. Keep the API loopback-only by default; remote access requires HTTPS/reverse-proxy deployment guidance and explicit configuration.
5. Add measurement, training and game repositories and API routes. Keep device acquisition separate and route validated session results through the application service.
6. Add MariaDB adapter and schema migrations. Run the same repository contract suite against file and MariaDB backends.
7. Migrate Linux Desktop and TUI first, then WebUI and Android, then Windows Desktop. Do not remove local legacy paths until data parity and rollback have been demonstrated.
8. Add multi-client integration tests: create patient on Desktop, edit on Android, read on WebUI; save a measurement in TUI and read on Desktop; save training in Android and read on WebUI; save game result on Desktop and read on TUI.
9. Add Windows CI (MSVC build/test/package), Android instrumentation tests, WebUI-to-server integration tests, backup/restore tests, concurrency tests and security regression tests.

## 6. Acceptance criteria

- One canonical patient record is visible consistently to all authorized clients.
- File and MariaDB patient backends pass identical repository contract tests.
- Patient records are encrypted at rest in file mode; secrets are absent from source, logs and API responses.
- A failed write cannot leave a truncated/corrupt patient record.
- Concurrent edits produce a detectable conflict rather than silent data loss.
- Measurement, training and game results are associated with the correct patient/session and survive server restart.
- Client authorization is tested for allowed and denied resource access.
- Backup restoration reproduces patient and result records and preserves identifiers/relationships.
- CI reports each platform independently; unbuilt or untested platforms are never described as passing.

## 7. Current implementation status

- Dodano `JsonResourceRepository` dla sześciu kolekcji: patients, measurements,
  training-sessions, training-results, game-sessions i game-results.
- Magazyn plikowy używa UUID, atomowego zapisu `QSaveFile`, limitu 16 MiB,
  kontroli wersji oraz uprawnień owner-only. Gdy `encryption.enabled=true`,
  rekordy są szyfrowane AES-256-GCM; brak klucza powoduje odmowę inicjalizacji.
- Desktop i wbudowany serwer HTTP współdzielą tę samą instancję repozytorium.
  Zakładka Pacjenci zapisuje i odczytuje karty; sesje pomiarowe są archiwizowane
  wraz z próbkami, sesje treningowe wraz z wynikami ćwiczeń, a wyniki gier
  trafiają do kolekcji game-sessions i game-results. Stare pliki pacjentów są
  migrowane bez usuwania źródeł migracji.
- API ma CRUD kolekcji, sesje Bearer, kontrolę wersji i tymczasową blokadę
  dostępu do roli administratora. Pełne RBAC, audyt operacji i WebSocket/SSE
  pozostają do wdrożenia.
- WebUI obsługuje CRUD kart pacjentów oraz odczyt archiwów pomiarów,
  treningów i gier. Android obsługuje CRUD kart pacjentów i odczyt tych samych
  archiwów; nie prowadzi jeszcze akwizycji ani tworzenia sesji z czujnika.
- TUI loguje się przez API, odczytuje listę pacjentów i zapisuje sesje oraz
  wyniki treningowe. Nie dołącza jeszcze identyfikatora pacjenta do treningu
  ani nie archiwizuje pomiarów i wyników gier.
- MariaDB pozostaje niewdrożona; wybranie jej jako backendu pacjentów nie
  powoduje fallbacku do plików.

Ten stan nie oznacza jeszcze pełnej synchronizacji pięciu interfejsów.
Wymagane są dalsze integracje WebUI i Androida dla pomiarów/treningów/gier,
powiązanie sesji TUI z pacjentem, archiwizacja gier z TUI, pełne RBAC,
testy wieloklientowe i testy backup/restore. Wyniki CI należy przypisywać wyłącznie
do konkretnego SHA; sam kod repozytorium nie stanowi dowodu poprawności
klinicznej ani kompletności bezpieczeństwa.
