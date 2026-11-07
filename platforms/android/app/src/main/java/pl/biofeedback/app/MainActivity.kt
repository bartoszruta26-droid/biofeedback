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
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
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
    var selected by remember { mutableIntStateOf(0) }
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
                Column(Modifier.padding(top = 22.dp, bottom = 6.dp)) {
                    Text("BIOFEEDBACK", color = Teal, fontSize = 10.sp, fontWeight = FontWeight.Bold, letterSpacing = 2.sp)
                    Text(sections[selected].label, color = Navy, fontSize = 28.sp, fontWeight = FontWeight.Bold)
                    Text("Mobilny panel systemu terapeutycznego", color = Muted, fontSize = 13.sp)
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
            }
            item { Spacer(Modifier.height(8.dp)) }
        }
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
