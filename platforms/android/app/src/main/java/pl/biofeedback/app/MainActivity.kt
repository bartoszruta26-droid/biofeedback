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
                1 -> item { EmptyModule("Pomiary", "Sesje i wykresy pojawią się po integracji z MeasurementEngine.") }
                2 -> item { EmptyModule("Pacjenci", "Repozytorium kart nie jest podłączone. Aplikacja nie tworzy fikcyjnych rekordów.") }
                3 -> item { EmptyModule("Trening", "Moduł oczekuje na wspólne usługi treningowe i sesyjne.") }
                4 -> item { EmptyModule("Urządzenia", "Brak połączenia. Aplikacja nie wysyła komend sprzętowych w trybie podglądu.") }
                5 -> item { DiagnosticsModule() }
                6 -> item { SettingsModule() }
            }
            item { Spacer(Modifier.height(8.dp)) }
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
