package pl.biofeedback.app

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Button
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.NavigationBar
import androidx.compose.material3.NavigationBarItem
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.runtime.rememberCoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Row
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

private val Navy = Color(0xFF17263B)
private val Teal = Color(0xFF087F82)
private val Canvas = Color(0xFFF2F5FA)
private val Muted = Color(0xFF718096)

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            MaterialTheme(colorScheme = lightColorScheme(
                primary = Teal, onPrimary = Color.White,
                background = Canvas, surface = Color.White, onSurface = Navy
            )) { BiofeedbackApp() }
        }
    }
}

private data class AppSection(val label: String, val symbol: String)
private val sections = listOf(
    AppSection("Pulpit", "⌂"), AppSection("Pomiary", "⌁"),
    AppSection("Pacjenci", "♙"), AppSection("Trening", "◈"),
    AppSection("Urządzenia", "⌘")
)

@Composable
private fun BiofeedbackApp() {
    var apiEndpoint by remember { mutableStateOf("") }
    var loginUsername by remember { mutableStateOf("") }
    var loginPassword by remember { mutableStateOf("") }
    var loginError by remember { mutableStateOf("") }
    var isLoggingIn by remember { mutableStateOf(false) }
    var authenticatedUser by remember { mutableStateOf<String?>(null) }
    var apiClient by remember { mutableStateOf<ApiClient?>(null) }
    val coroutineScope = rememberCoroutineScope()

    if (authenticatedUser == null) {
        LoginScreen(
            endpoint = apiEndpoint,
            onEndpointChange = { apiEndpoint = it },
            username = loginUsername,
            onUsernameChange = { loginUsername = it },
            password = loginPassword,
            onPasswordChange = { loginPassword = it },
            error = loginError,
            loading = isLoggingIn,
            onLogin = {
                isLoggingIn = true
                loginError = ""
                coroutineScope.launch {
                    try {
                        val result = withContext(Dispatchers.IO) {
                            val client = ApiClient(apiEndpoint.trim())
                            client.login(loginUsername.trim(), loginPassword)
                            val user = client.currentUser()
                            client to user
                        }
                        apiClient = result.first
                        authenticatedUser = result.second.optString("username") +
                            " · " + result.second.optString("role")
                        loginPassword = ""
                    } catch (error: Exception) {
                        loginError = error.message ?: "Logowanie nie powiodło się."
                        loginPassword = ""
                    } finally {
                        isLoggingIn = false
                    }
                }
            }
        )
        return
    }

    var selected by remember { mutableIntStateOf(0) }
    var moreMenuExpanded by remember { mutableStateOf(false) }
    Scaffold(
        containerColor = Canvas,
        bottomBar = {
            NavigationBar(containerColor = Color.White) {
                sections.forEachIndexed { index, section ->
                    NavigationBarItem(
                        selected = selected == index,
                        onClick = { selected = index },
                        icon = { Text(section.symbol, fontSize = 19.sp) },
                        label = { Text(section.label, fontSize = 10.sp) }
                    )
                }
            }
        }
    ) { padding ->
        LazyColumn(
            modifier = Modifier.fillMaxSize().padding(padding).padding(horizontal = 20.dp),
            verticalArrangement = Arrangement.spacedBy(14.dp)
        ) {
            item {
                Row(
                    modifier = Modifier.fillMaxWidth().padding(top = 22.dp, bottom = 6.dp),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Column {
                        Text("BIOFEEDBACK", color = Teal, fontSize = 10.sp, fontWeight = FontWeight.Bold, letterSpacing = 2.sp)
                        Text(
                            when (selected) {
                                5 -> "Diagnostyka"
                                6 -> "Ustawienia"
                                else -> sections[selected].label
                            },
                            color = Navy, fontSize = 28.sp, fontWeight = FontWeight.Bold
                        )
                        Text(authenticatedUser ?: "Sesja API aktywna", color = Muted, fontSize = 12.sp)
                    }
                    Box {
                        Text("⋮", modifier = Modifier.clickable { moreMenuExpanded = true }.padding(12.dp), color = Navy, fontSize = 24.sp)
                        DropdownMenu(expanded = moreMenuExpanded, onDismissRequest = { moreMenuExpanded = false }) {
                            DropdownMenuItem(text = { Text("Diagnostyka") }, onClick = {
                                selected = 5
                                moreMenuExpanded = false
                            })
                            DropdownMenuItem(text = { Text("Ustawienia") }, onClick = {
                                selected = 6
                                moreMenuExpanded = false
                            })
                            DropdownMenuItem(text = { Text("Wyloguj") }, onClick = {
                                moreMenuExpanded = false
                                coroutineScope.launch {
                                    withContext(Dispatchers.IO) {
                                        try { apiClient?.logout() } catch (_: Exception) { }
                                    }
                                    apiClient = null
                                    authenticatedUser = null
                                    selected = 0
                                }
                            })
                        }
                    }
                }
            }
            item {
                Surface(color = Color(0xFFFFF4DF), shape = RoundedCornerShape(8.dp)) {
                    Text(
                        "TRYB PODGLĄDU · BRAK POŁĄCZENIA Z URZĄDZENIEM",
                        modifier = Modifier.fillMaxWidth().padding(11.dp),
                        color = Color(0xFF91631C), fontSize = 9.sp, fontWeight = FontWeight.Bold
                    )
                }
            }
            when (selected) {
                0 -> {
                    item { MetricCard("Połączenie z czujnikiem", "Rozłączono", "Brak aktywnego urządzenia") }
                    item { MetricCard("Aktywna sesja", "Bezczynny", "Nie rozpoczęto pomiaru") }
                    item { MetricCard("Ostatni odczyt", "— N", "Oczekiwanie na próbkę") }
                    item { EmptyChartCard() }
                }
                1 -> item { ResourceListModule(apiClient, "Sesje pomiarowe", "measurements") }
                2 -> item { PatientsModule(apiClient) }
                3 -> item { TrainingAndGamesModule(apiClient) }
                4 -> item { EmptyModule("Urządzenia", "Brak połączenia. Aplikacja nie wysyła komend sprzętowych w trybie podglądu.") }
                5 -> item { DiagnosticsModule() }
                6 -> item { SettingsModule() }
            }
            item { Spacer(Modifier.height(8.dp)) }
        }
    }
}

@Composable
private fun ResourceListModule(client: ApiClient?, title: String, collection: String) {
    var items by remember { mutableStateOf<org.json.JSONArray?>(null) }
    var errorMessage by remember { mutableStateOf("") }
    var busy by remember { mutableStateOf(false) }
    val scope = rememberCoroutineScope()

    fun refresh() {
        if (client == null) {
            errorMessage = "Brak klienta API."
            return
        }
        scope.launch {
            busy = true
            errorMessage = ""
            try {
                val response = withContext(Dispatchers.IO) { client.listResources(collection) }
                items = response.optJSONArray("items")
            } catch (error: Exception) {
                errorMessage = error.message ?: "Nie można pobrać danych z serwera."
            } finally {
                busy = false
            }
        }
    }

    Column(verticalArrangement = Arrangement.spacedBy(10.dp)) {
        Text(title, color = Navy, fontWeight = FontWeight.Bold)
        Text("Dane odczytywane ze wspólnego repozytorium serwera.", color = Muted, fontSize = 12.sp)
        Button(onClick = { refresh() }, enabled = !busy && client != null) { Text("Odśwież") }
        if (busy) CircularProgressIndicator()
        if (errorMessage.isNotBlank()) Text(errorMessage, color = Color(0xFFB42318), fontSize = 12.sp)
        Text("Rekordy: ${items?.length() ?: 0}", color = Navy, fontWeight = FontWeight.SemiBold)
        val records = items
        if (records != null) {
            for (index in 0 until records.length()) {
                val record = records.optJSONObject(index) ?: continue
                val label = listOf("exercise_name", "game_id", "display_name", "status")
                    .firstNotNullOfOrNull { key -> record.optString(key).takeIf { it.isNotBlank() } }
                    ?: record.optString("id")
                val detail = buildList {
                    if (record.has("started_at")) add("Start: ${record.optString("started_at")}")
                    if (record.has("sample_count")) add("Próbki: ${record.optInt("sample_count")}")
                    if (record.has("score")) add("Wynik: ${record.optInt("score")}")
                    add("Wersja: ${record.optInt("version")}")
                }.joinToString(" • ")
                Card(colors = CardDefaults.cardColors(containerColor = Color.White)) {
                    Column(Modifier.fillMaxWidth().padding(12.dp), verticalArrangement = Arrangement.spacedBy(4.dp)) {
                        Text(label, color = Navy, fontWeight = FontWeight.Bold)
                        Text(detail, color = Muted, fontSize = 11.sp)
                    }
                }
            }
        }
    }
}

@Composable
private fun TrainingAndGamesModule(client: ApiClient?) {
    var trainingSessions by remember { mutableStateOf<org.json.JSONArray?>(null) }
    var trainingResults by remember { mutableStateOf<org.json.JSONArray?>(null) }
    var gameResults by remember { mutableStateOf<org.json.JSONArray?>(null) }
    var errorMessage by remember { mutableStateOf("") }
    var busy by remember { mutableStateOf(false) }
    val scope = rememberCoroutineScope()

    fun refresh() {
        if (client == null) {
            errorMessage = "Brak klienta API."
            return
        }
        scope.launch {
            busy = true
            errorMessage = ""
            try {
                val loaded = withContext(Dispatchers.IO) {
                    Triple(client.getTrainingSessions(), client.getTrainingResults(), client.getGameResults())
                }
                trainingSessions = loaded.first.optJSONArray("items")
                trainingResults = loaded.second.optJSONArray("items")
                gameResults = loaded.third.optJSONArray("items")
            } catch (error: Exception) {
                errorMessage = error.message ?: "Nie można pobrać historii treningu."
            } finally {
                busy = false
            }
        }
    }

    @Composable
    fun section(label: String, records: org.json.JSONArray?) {
        Text("$label: ${records?.length() ?: 0}", color = Navy, fontWeight = FontWeight.SemiBold)
        if (records != null) {
            for (index in 0 until records.length()) {
                val record = records.optJSONObject(index) ?: continue
                val title = record.optString("exercise_name").ifBlank {
                    record.optString("game_id").ifBlank { record.optString("id") }
                }
                val score = if (record.has("score")) " • wynik: ${record.optInt("score")}" else ""
                Card(colors = CardDefaults.cardColors(containerColor = Color.White)) {
                    Text(title + score, Modifier.fillMaxWidth().padding(12.dp), color = Navy)
                }
            }
        }
    }

    Column(verticalArrangement = Arrangement.spacedBy(10.dp)) {
        Text("Trening i gry", color = Navy, fontWeight = FontWeight.Bold)
        Text("Historia pobierana z repozytorium wspólnego dla klientów.", color = Muted, fontSize = 12.sp)
        Button(onClick = { refresh() }, enabled = !busy && client != null) { Text("Odśwież historię") }
        if (busy) CircularProgressIndicator()
        if (errorMessage.isNotBlank()) Text(errorMessage, color = Color(0xFFB42318), fontSize = 12.sp)
        section("Sesje treningowe", trainingSessions)
        section("Wyniki treningów", trainingResults)
        section("Wyniki gier", gameResults)
    }
}

@Composable
private fun PatientsModule(client: ApiClient?) {
    var firstName by remember { mutableStateOf("") }
    var lastName by remember { mutableStateOf("") }
    var patientReference by remember { mutableStateOf("") }
    var medicalHistory by remember { mutableStateOf("") }
    var editingId by remember { mutableStateOf<String?>(null) }
    var editingVersion by remember { mutableIntStateOf(0) }
    var patients by remember { mutableStateOf<org.json.JSONObject?>(null) }
    var errorMessage by remember { mutableStateOf("") }
    var busy by remember { mutableStateOf(false) }
    val scope = rememberCoroutineScope()

    fun refresh() {
        if (client == null) {
            errorMessage = "Brak klienta API."
            return
        }
        scope.launch {
            busy = true
            errorMessage = ""
            try {
                patients = withContext(Dispatchers.IO) { client.getPatients() }
            } catch (error: Exception) {
                errorMessage = error.message ?: "Nie można pobrać kart pacjentów."
            } finally {
                busy = false
            }
        }
    }

    fun savePatient() {
        if (client == null || firstName.isBlank() || lastName.isBlank() || patientReference.isBlank()) {
            errorMessage = "Podaj imię, nazwisko i identyfikator pacjenta oraz sprawdź połączenie API."
            return
        }
        val payload = org.json.JSONObject()
            .put("firstName", firstName.trim())
            .put("lastName", lastName.trim())
            .put("pesel", patientReference.trim())
            .put("medicalHistory", medicalHistory)
        if (editingId != null) {
            val existingItems = patients?.optJSONArray("items")
            if (existingItems != null) {
                for (index in 0 until existingItems.length()) {
                    val existing = existingItems.optJSONObject(index) ?: continue
                    if (existing.optString("id") == editingId) {
                        payload.put("birthDate", existing.optString("birthDate"))
                        payload.put("gender", existing.optString("gender"))
                        break
                    }
                }
            }
        }
        scope.launch {
            busy = true
            errorMessage = ""
            try {
                withContext(Dispatchers.IO) {
                    val id = editingId
                    if (id == null) client.createPatient(payload)
                    else client.updatePatient(id, editingVersion, payload)
                }
                editingId = null
                editingVersion = 0
                firstName = ""
                lastName = ""
                patientReference = ""
                medicalHistory = ""
                patients = withContext(Dispatchers.IO) { client.getPatients() }
            } catch (error: Exception) {
                errorMessage = error.message ?: "Nie udało się zapisać karty."
            } finally {
                busy = false
            }
        }
    }

    Column(verticalArrangement = Arrangement.spacedBy(10.dp)) {
        Text("Wspólne repozytorium pacjentów", color = Navy, fontWeight = FontWeight.Bold)
        Text("Dane są pobierane i zapisywane przez serwer Biofeedback.", color = Muted, fontSize = 12.sp)
        OutlinedTextField(firstName, { firstName = it }, label = { Text("Imię") }, modifier = Modifier.fillMaxWidth())
        OutlinedTextField(lastName, { lastName = it }, label = { Text("Nazwisko") }, modifier = Modifier.fillMaxWidth())
        OutlinedTextField(patientReference, { patientReference = it }, label = { Text("PESEL / identyfikator") }, modifier = Modifier.fillMaxWidth())
        OutlinedTextField(medicalHistory, { medicalHistory = it }, label = { Text("Uwagi medyczne") }, modifier = Modifier.fillMaxWidth())
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            Button(onClick = { savePatient() }, enabled = !busy) {
                Text(if (editingId == null) "Dodaj kartę" else "Zapisz zmiany")
            }
            Button(onClick = { refresh() }, enabled = !busy && client != null) {
                Text("Odśwież")
            }
            if (editingId != null) {
                Button(onClick = {
                    editingId = null
                    editingVersion = 0
                    firstName = ""
                    lastName = ""
                    patientReference = ""
                    medicalHistory = ""
                }, enabled = !busy) { Text("Anuluj") }
            }
        }
        if (busy) CircularProgressIndicator()
        if (errorMessage.isNotBlank()) Text(errorMessage, color = Color(0xFFB42318), fontSize = 12.sp)

        val items = patients?.optJSONArray("items")
        Text("Liczba kart: ${items?.length() ?: 0}", color = Navy, fontWeight = FontWeight.SemiBold)
        if (items != null) {
            for (index in 0 until items.length()) {
                val patient = items.optJSONObject(index) ?: continue
                val id = patient.optString("id")
                val version = patient.optInt("version", 0)
                Card(colors = CardDefaults.cardColors(containerColor = Color.White)) {
                    Column(Modifier.fillMaxWidth().padding(12.dp), verticalArrangement = Arrangement.spacedBy(6.dp)) {
                        Text(
                            patient.optString("firstName") + " " + patient.optString("lastName"),
                            color = Navy, fontWeight = FontWeight.Bold
                        )
                        Text("Wersja rekordu: $version", color = Muted, fontSize = 11.sp)
                        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                            Button(onClick = {
                                editingId = id
                                editingVersion = version
                                firstName = patient.optString("firstName")
                                lastName = patient.optString("lastName")
                                patientReference = patient.optString("pesel")
                                medicalHistory = patient.optString("medicalHistory")
                            }, enabled = !busy) { Text("Edytuj") }
                            Button(onClick = {
                                scope.launch {
                                    busy = true
                                    errorMessage = ""
                                    try {
                                        withContext(Dispatchers.IO) { client?.deletePatient(id, version) }
                                        patients = withContext(Dispatchers.IO) { client?.getPatients() }
                                    } catch (error: Exception) {
                                        errorMessage = error.message ?: "Nie udało się usunąć karty."
                                    } finally {
                                        busy = false
                                    }
                                }
                            }, enabled = !busy && client != null) { Text("Usuń") }
                        }
                    }
                }
            }
        }
    }
}

@Composable
private fun LoginScreen(
    endpoint: String,
    onEndpointChange: (String) -> Unit,
    username: String,
    onUsernameChange: (String) -> Unit,
    password: String,
    onPasswordChange: (String) -> Unit,
    error: String,
    loading: Boolean,
    onLogin: () -> Unit
) {
    Column(
        modifier = Modifier.fillMaxSize().background(Canvas).padding(24.dp),
        verticalArrangement = Arrangement.Center
    ) {
        Text("BIOFEEDBACK", color = Teal, fontSize = 11.sp, fontWeight = FontWeight.Bold, letterSpacing = 2.sp)
        Spacer(Modifier.height(10.dp))
        Text("Logowanie", color = Navy, fontSize = 28.sp, fontWeight = FontWeight.Bold)
        Spacer(Modifier.height(8.dp))
        Text("Połącz się z serwerem API przez szyfrowane HTTPS.", color = Muted, fontSize = 13.sp)
        Spacer(Modifier.height(22.dp))
        OutlinedTextField(
            value = endpoint,
            onValueChange = onEndpointChange,
            modifier = Modifier.fillMaxWidth(),
            label = { Text("Adres HTTPS API") },
            placeholder = { Text("https://serwer.example") },
            singleLine = true
        )
        Spacer(Modifier.height(10.dp))
        OutlinedTextField(
            value = username,
            onValueChange = onUsernameChange,
            modifier = Modifier.fillMaxWidth(),
            label = { Text("Użytkownik") },
            singleLine = true
        )
        Spacer(Modifier.height(10.dp))
        OutlinedTextField(
            value = password,
            onValueChange = onPasswordChange,
            modifier = Modifier.fillMaxWidth(),
            label = { Text("Hasło") },
            visualTransformation = PasswordVisualTransformation(),
            singleLine = true
        )
        Spacer(Modifier.height(16.dp))
        Button(
            onClick = onLogin,
            enabled = !loading && endpoint.startsWith("https://") && username.isNotBlank() && password.isNotEmpty(),
            modifier = Modifier.fillMaxWidth()
        ) {
            if (loading) CircularProgressIndicator()
            else Text("Zaloguj")
        }
        if (error.isNotBlank()) {
            Spacer(Modifier.height(12.dp))
            Text(error, color = Color(0xFFB42318), fontSize = 12.sp)
        }
        Spacer(Modifier.height(12.dp))
        Text("Token sesji pozostaje w pamięci aplikacji i znika po wylogowaniu lub zamknięciu procesu.", color = Muted, fontSize = 11.sp)
    }
}

@Composable
private fun MetricCard(title: String, value: String, detail: String) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(14.dp),
        colors = CardDefaults.cardColors(containerColor = Color.White),
        elevation = CardDefaults.cardElevation(defaultElevation = 1.dp)
    ) {
        Column(Modifier.padding(18.dp)) {
            Text(title, color = Muted, fontSize = 12.sp, fontWeight = FontWeight.Medium)
            Spacer(Modifier.height(8.dp))
            Text(value, color = Navy, fontSize = 22.sp, fontWeight = FontWeight.Bold)
            Text(detail, color = Muted, fontSize = 11.sp)
        }
    }
}

@Composable
private fun EmptyChartCard() {
    Card(
        modifier = Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(14.dp),
        colors = CardDefaults.cardColors(containerColor = Color.White)
    ) {
        Column(Modifier.padding(18.dp)) {
            Text("Przebieg pomiaru", color = Navy, fontSize = 15.sp, fontWeight = FontWeight.Bold)
            Spacer(Modifier.height(12.dp))
            Box(
                modifier = Modifier.fillMaxWidth().height(150.dp).background(Canvas, RoundedCornerShape(10.dp)),
                contentAlignment = Alignment.Center
            ) { Text("Oczekiwanie na dane z czujnika", color = Muted, fontSize = 12.sp) }
        }
    }
}

@Composable
private fun EmptyModule(title: String, description: String) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(14.dp),
        colors = CardDefaults.cardColors(containerColor = Color.White)
    ) {
        Column(Modifier.padding(22.dp), horizontalAlignment = Alignment.Start) {
            Text(title, color = Navy, fontSize = 17.sp, fontWeight = FontWeight.Bold)
            Spacer(Modifier.height(8.dp))
            Text(description, color = Muted, fontSize = 12.sp)
        }
    }
}

@Composable
private fun DiagnosticsModule() {
    Card(
        modifier = Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(14.dp),
        colors = CardDefaults.cardColors(containerColor = Color.White)
    ) {
        Column(Modifier.padding(20.dp), verticalArrangement = Arrangement.spacedBy(14.dp)) {
            Text("Stan usług", color = Navy, fontSize = 17.sp, fontWeight = FontWeight.Bold)
            DiagnosticRow("Interfejs Android", "Aktywny", true)
            DiagnosticRow("Serwer API", "Niepodłączony", false)
            DiagnosticRow("Strumień pomiarowy", "Niepodłączony", false)
            DiagnosticRow("Adapter sprzętowy", "Niepodłączony", false)
        }
    }
}

@Composable
private fun DiagnosticRow(label: String, state: String, active: Boolean) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(label, color = Navy, fontSize = 12.sp)
        Text(state, color = if (active) Teal else Muted, fontSize = 11.sp, fontWeight = FontWeight.Medium)
    }
}

@Composable
private fun SettingsModule() {
    EmptyModule(
        "Ustawienia aplikacji",
        "Wersja rozwojowa. Konfiguracja serwera API, kont użytkowników i parametrów urządzenia zostanie udostępniona po wdrożeniu wspólnej warstwy usług."
    )
}
