#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Szkielet aplikacji Biofeedback z podziałem na zakładki,
wczytywaniem danych konfiguracyjnych oraz przygotowaniem czujnika tensometrycznego.
"""

import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import json
import random
from datetime import datetime


class StrainGaugeSensor:
    """Klasa symulująca czujnik tensometryczny."""
    
    def __init__(self):
        self.is_connected = False
        self.is_calibrated = False
        self.zero_offset = 0.0
        self.sensitivity = 1.0
        self.data_callback = None
    
    def connect(self):
        """Symulacja połączenia z czujnikiem."""
        self.is_connected = True
        return True
    
    def disconnect(self):
        """Rozłączenie czujnika."""
        self.is_connected = False
        self.is_calibrated = False
    
    def calibrate(self):
        """Kalibracja czujnika - ustawienie punktu zerowego."""
        if not self.is_connected:
            return False
        self.zero_offset = random.uniform(-0.5, 0.5)
        self.is_calibrated = True
        return True
    
    def read_value(self):
        """Odczyt wartości z czujnika."""
        if not self.is_connected or not self.is_calibrated:
            return None
        base_value = random.gauss(0, 0.1)
        return (base_value - self.zero_offset) * self.sensitivity
    
    def set_sensitivity(self, value):
        """Ustawienie czułości czujnika."""
        self.sensitivity = value


class ConfigManager:
    """Menadżer konfiguracji aplikacji."""
    
    def __init__(self, config_file="config.json"):
        self.config_file = config_file
        self.config = self.load_config()
    
    def load_config(self):
        """Wczytanie konfiguracji z pliku."""
        default_config = {
            "app_name": "Biofeedback App",
            "version": "1.0.0",
            "sensor": {
                "type": "strain_gauge",
                "sampling_rate": 100,
                "sensitivity": 1.0
            },
            "user": {
                "name": "",
                "age": 0
            },
            "thresholds": {
                "min_force": 0.0,
                "max_force": 100.0,
                "target_force": 50.0
            },
            "ui": {
                "theme": "default",
                "language": "pl"
            }
        }
        
        try:
            with open(self.config_file, 'r', encoding='utf-8') as f:
                config = json.load(f)
                for key, value in default_config.items():
                    if key not in config:
                        config[key] = value
                return config
        except FileNotFoundError:
            self.save_config(default_config)
            return default_config
        except json.JSONDecodeError:
            messagebox.showerror("Błąd", "Nieprawidłowy format pliku konfiguracyjnego")
            return default_config
    
    def save_config(self, config=None):
        """Zapis konfiguracji do pliku."""
        if config is None:
            config = self.config
        
        try:
            with open(self.config_file, 'w', encoding='utf-8') as f:
                json.dump(config, f, indent=4, ensure_ascii=False)
            return True
        except Exception as e:
            messagebox.showerror("Błąd", f"Nie udało się zapisać konfiguracji: {str(e)}")
            return False
    
    def get(self, key, default=None):
        """Pobranie wartości z konfiguracji."""
        keys = key.split('.')
        value = self.config
        for k in keys:
            if isinstance(value, dict) and k in value:
                value = value[k]
            else:
                return default
        return value
    
    def set(self, key, value):
        """Ustawienie wartości w konfiguracji."""
        keys = key.split('.')
        config = self.config
        for k in keys[:-1]:
            if k not in config:
                config[k] = {}
            config = config[k]
        config[keys[-1]] = value


class BiofeedbackApp:
    """Główna klasa aplikacji Biofeedback."""
    
    def __init__(self, root):
        self.root = root
        self.root.title("Aplikacja Biofeedback")
        self.root.geometry("900x700")
        
        self.config_manager = ConfigManager()
        self.sensor = StrainGaugeSensor()
        self.is_monitoring = False
        self.monitoring_job = None
        
        self.setup_ui()
        self.load_app_config()
    
    def setup_ui(self):
        """Konfiguracja interfejsu użytkownika."""
        # Pasek menu
        self.create_menu()
        
        # Kontener na zakładki
        self.notebook = ttk.Notebook(self.root)
        self.notebook.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Tworzenie zakładek
        self.create_dashboard_tab()
        self.create_sensor_tab()
        self.create_exercises_tab()
        self.create_history_tab()
        self.create_settings_tab()
        
        # Pasek statusu
        self.status_bar = ttk.Label(
            self.root, 
            text="Gotowy", 
            relief=tk.SUNKEN, 
            anchor=tk.W
        )
        self.status_bar.pack(fill=tk.X, side=tk.BOTTOM)
    
    def create_menu(self):
        """Tworzenie paska menu."""
        menubar = tk.Menu(self.root)
        self.root.config(menu=menubar)
        
        # Menu Plik
        file_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="Plik", menu=file_menu)
        file_menu.add_command(label="Wczytaj konfigurację", command=self.load_config_file)
        file_menu.add_command(label="Zapisz konfigurację", command=self.save_config_file)
        file_menu.add_separator()
        file_menu.add_command(label="Wyjście", command=self.on_closing)
        
        # Menu Pomoc
        help_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="Pomoc", menu=help_menu)
        help_menu.add_command(label="O programie", command=self.show_about)
    
    def create_dashboard_tab(self):
        """Tworzenie zakładki Dashboard."""
        dashboard_frame = ttk.Frame(self.notebook)
        self.notebook.add(dashboard_frame, text="Dashboard")
        
        # Panel głównych informacji
        info_frame = ttk.LabelFrame(dashboard_frame, text="Informacje o sesji")
        info_frame.pack(fill=tk.X, padx=20, pady=10)
        
        ttk.Label(info_frame, text="Użytkownik:").grid(row=0, column=0, sticky=tk.W, padx=10, pady=5)
        self.lbl_user = ttk.Label(info_frame, text="-")
        self.lbl_user.grid(row=0, column=1, sticky=tk.W, padx=10, pady=5)
        
        ttk.Label(info_frame, text="Status czujnika:").grid(row=1, column=0, sticky=tk.W, padx=10, pady=5)
        self.lbl_sensor_status = ttk.Label(info_frame, text="Niepodłączony", foreground="red")
        self.lbl_sensor_status.grid(row=1, column=1, sticky=tk.W, padx=10, pady=5)
        
        # Panel bieżącej wartości
        value_frame = ttk.LabelFrame(dashboard_frame, text="Bieżąca wartość")
        value_frame.pack(fill=tk.BOTH, expand=True, padx=20, pady=10)
        
        self.lbl_current_value = ttk.Label(
            value_frame, 
            text="0.00", 
            font=("Arial", 48),
            justify=tk.CENTER
        )
        self.lbl_current_value.pack(expand=True)
        
        ttk.Label(value_frame, text="jednostek siły", font=("Arial", 12)).pack()
        
        # Przyciski sterujące
        btn_frame = ttk.Frame(dashboard_frame)
        btn_frame.pack(fill=tk.X, padx=20, pady=10)
        
        self.btn_start = ttk.Button(
            btn_frame, 
            text="Rozpocznij pomiar", 
            command=self.toggle_monitoring
        )
        self.btn_start.pack(side=tk.LEFT, padx=5)
        
        self.btn_zero = ttk.Button(
            btn_frame, 
            text="Tara (zerowanie)", 
            command=self.zero_sensor
        )
        self.btn_zero.pack(side=tk.LEFT, padx=5)
    
    def create_sensor_tab(self):
        """Tworzenie zakładki konfiguracji czujnika."""
        sensor_frame = ttk.Frame(self.notebook)
        self.notebook.add(sensor_frame, text="Czujnik")
        
        # Sekcja połączenia
        connection_frame = ttk.LabelFrame(sensor_frame, text="Połączenie")
        connection_frame.pack(fill=tk.X, padx=20, pady=10)
        
        ttk.Label(connection_frame, text="Typ czujnika:").grid(row=0, column=0, sticky=tk.W, padx=10, pady=5)
        ttk.Label(connection_frame, text="Tensometr").grid(row=0, column=1, sticky=tk.W, padx=10, pady=5)
        
        self.btn_connect = ttk.Button(
            connection_frame, 
            text="Podłącz czujnik", 
            command=self.connect_sensor
        )
        self.btn_connect.grid(row=0, column=2, padx=10, pady=5)
        
        self.btn_calibrate = ttk.Button(
            connection_frame, 
            text="Kalibruj", 
            command=self.calibrate_sensor,
            state=tk.DISABLED
        )
        self.btn_calibrate.grid(row=0, column=3, padx=10, pady=5)
        
        # Sekcja ustawień czujnika
        settings_frame = ttk.LabelFrame(sensor_frame, text="Ustawienia czujnika")
        settings_frame.pack(fill=tk.X, padx=20, pady=10)
        
        ttk.Label(settings_frame, text="Czułość:").grid(row=0, column=0, sticky=tk.W, padx=10, pady=5)
        self.scale_sensitivity = ttk.Scale(
            settings_frame, 
            from_=0.1, 
            to=5.0, 
            orient=tk.HORIZONTAL,
            command=self.update_sensitivity
        )
        self.scale_sensitivity.set(1.0)
        self.scale_sensitivity.grid(row=0, column=1, sticky=tk.EW, padx=10, pady=5)
        
        ttk.Label(settings_frame, text="Częstotliwość próbkowania:").grid(row=1, column=0, sticky=tk.W, padx=10, pady=5)
        self.combo_sampling = ttk.Combobox(
            settings_frame, 
            values=[50, 100, 200, 500],
            state="readonly"
        )
        self.combo_sampling.set(100)
        self.combo_sampling.grid(row=1, column=1, sticky=tk.W, padx=10, pady=5)
        
        # Informacje o kalibracji
        calib_frame = ttk.LabelFrame(sensor_frame, text="Status kalibracji")
        calib_frame.pack(fill=tk.X, padx=20, pady=10)
        
        self.lbl_calib_status = ttk.Label(calib_frame, text="Nie skalibrowany", foreground="orange")
        self.lbl_calib_status.pack(padx=10, pady=10)
        
        self.lbl_offset = ttk.Label(calib_frame, text="Offset: 0.00")
        self.lbl_offset.pack(padx=10, pady=5)
    
    def create_exercises_tab(self):
        """Tworzenie zakładki ćwiczeń."""
        exercises_frame = ttk.Frame(self.notebook)
        self.notebook.add(exercises_frame, text="Ćwiczenia")
        
        # Lista ćwiczeń
        list_frame = ttk.LabelFrame(exercises_frame, text="Dostępne ćwiczenia")
        list_frame.pack(fill=tk.BOTH, expand=True, padx=20, pady=10)
        
        columns = ("name", "description", "duration")
        self.exercises_tree = ttk.Treeview(list_frame, columns=columns, show="headings")
        self.exercises_tree.heading("name", text="Nazwa")
        self.exercises_tree.heading("description", text="Opis")
        self.exercises_tree.heading("duration", text="Czas trwania")
        
        self.exercises_tree.column("name", width=150)
        self.exercises_tree.column("description", width=400)
        self.exercises_tree.column("duration", width=100)
        
        # Przykładowe ćwiczenia
        exercises_data = [
            ("Ściskanie statyczne", "Utrzymywanie stałej siły przez określony czas", "30s"),
            ("Cykle ściskania", "Naprzemienne ściskanie i rozluźnianie", "60s"),
            ("Stopniowanie siły", "Płynne zwiększanie i zmniejszanie siły", "45s"),
        ]
        
        for exercise in exercises_data:
            self.exercises_tree.insert("", tk.END, values=exercise)
        
        self.exercises_tree.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Przyciski
        btn_frame = ttk.Frame(exercises_frame)
        btn_frame.pack(fill=tk.X, padx=20, pady=10)
        
        ttk.Button(btn_frame, text="Rozpocznij ćwiczenie", command=self.start_exercise).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text="Dodaj ćwiczenie", command=self.add_exercise).pack(side=tk.LEFT, padx=5)
    
    def create_history_tab(self):
        """Tworzenie zakładki historii."""
        history_frame = ttk.Frame(self.notebook)
        self.notebook.add(history_frame, text="Historia")
        
        # Tabela historii
        table_frame = ttk.LabelFrame(history_frame, text="Poprzednie sesje")
        table_frame.pack(fill=tk.BOTH, expand=True, padx=20, pady=10)
        
        columns = ("date", "exercise", "duration", "avg_force", "max_force")
        self.history_tree = ttk.Treeview(table_frame, columns=columns, show="headings")
        
        for col in columns:
            self.history_tree.heading(col, text=col.replace("_", " ").title())
            self.history_tree.column(col, width=120)
        
        scrollbar = ttk.Scrollbar(table_frame, orient=tk.VERTICAL, command=self.history_tree.yview)
        self.history_tree.configure(yscrollcommand=scrollbar.set)
        
        self.history_tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        # Przyciski
        btn_frame = ttk.Frame(history_frame)
        btn_frame.pack(fill=tk.X, padx=20, pady=10)
        
        ttk.Button(btn_frame, text="Eksportuj dane", command=self.export_history).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text="Wyczyść historię", command=self.clear_history).pack(side=tk.LEFT, padx=5)
    
    def create_settings_tab(self):
        """Tworzenie zakładki ustawień."""
        settings_frame = ttk.Frame(self.notebook)
        self.notebook.add(settings_frame, text="Ustawienia")
        
        # Ustawienia użytkownika
        user_frame = ttk.LabelFrame(settings_frame, text="Dane użytkownika")
        user_frame.pack(fill=tk.X, padx=20, pady=10)
        
        ttk.Label(user_frame, text="Imię i nazwisko:").grid(row=0, column=0, sticky=tk.W, padx=10, pady=5)
        self.entry_user_name = ttk.Entry(user_frame, width=40)
        self.entry_user_name.grid(row=0, column=1, padx=10, pady=5)
        
        ttk.Label(user_frame, text="Wiek:").grid(row=1, column=0, sticky=tk.W, padx=10, pady=5)
        self.entry_user_age = ttk.Entry(user_frame, width=10)
        self.entry_user_age.grid(row=1, column=1, sticky=tk.W, padx=10, pady=5)
        
        # Progi siły
        threshold_frame = ttk.LabelFrame(settings_frame, text="Progi siły")
        threshold_frame.pack(fill=tk.X, padx=20, pady=10)
        
        ttk.Label(threshold_frame, text="Siła minimalna:").grid(row=0, column=0, sticky=tk.W, padx=10, pady=5)
        self.entry_min_force = ttk.Entry(threshold_frame, width=10)
        self.entry_min_force.grid(row=0, column=1, sticky=tk.W, padx=10, pady=5)
        
        ttk.Label(threshold_frame, text="Siła maksymalna:").grid(row=1, column=0, sticky=tk.W, padx=10, pady=5)
        self.entry_max_force = ttk.Entry(threshold_frame, width=10)
        self.entry_max_force.grid(row=1, column=1, sticky=tk.W, padx=10, pady=5)
        
        ttk.Label(threshold_frame, text="Siła docelowa:").grid(row=2, column=0, sticky=tk.W, padx=10, pady=5)
        self.entry_target_force = ttk.Entry(threshold_frame, width=10)
        self.entry_target_force.grid(row=2, column=1, sticky=tk.W, padx=10, pady=5)
        
        # Przyciski zapisu
        btn_frame = ttk.Frame(settings_frame)
        btn_frame.pack(fill=tk.X, padx=20, pady=20)
        
        ttk.Button(btn_frame, text="Zapisz ustawienia", command=self.save_settings).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text="Przywróć domyślne", command=self.reset_settings).pack(side=tk.LEFT, padx=5)
    
    def load_app_config(self):
        """Wczytanie konfiguracji aplikacji i aktualizacja UI."""
        user_name = self.config_manager.get("user.name", "")
        user_age = self.config_manager.get("user.age", 0)
        
        self.entry_user_name.insert(0, user_name)
        self.entry_user_age.insert(0, str(user_age))
        
        thresholds = self.config_manager.get("thresholds", {})
        self.entry_min_force.insert(0, str(thresholds.get("min_force", 0.0)))
        self.entry_max_force.insert(0, str(thresholds.get("max_force", 100.0)))
        self.entry_target_force.insert(0, str(thresholds.get("target_force", 50.0)))
        
        sensor_config = self.config_manager.get("sensor", {})
        self.scale_sensitivity.set(sensor_config.get("sensitivity", 1.0))
        self.combo_sampling.set(str(sensor_config.get("sampling_rate", 100)))
        
        if user_name:
            self.lbl_user.config(text=user_name)
        
        self.update_status("Konfiguracja wczytana")
    
    def connect_sensor(self):
        """Połączenie z czujnikiem."""
        if self.sensor.connect():
            self.btn_connect.config(text="Odłącz czujnik", command=self.disconnect_sensor)
            self.btn_calibrate.config(state=tk.NORMAL)
            self.lbl_sensor_status.config(text="Podłączony", foreground="green")
            self.update_status("Czujnik podłączony")
            messagebox.showinfo("Sukces", "Pomyślnie podłączono czujnik tensometryczny")
        else:
            messagebox.showerror("Błąd", "Nie udało się podłączyć czujnika")
    
    def disconnect_sensor(self):
        """Rozłączenie czujnika."""
        self.stop_monitoring()
        self.sensor.disconnect()
        self.btn_connect.config(text="Podłącz czujnik", command=self.connect_sensor)
        self.btn_calibrate.config(state=tk.DISABLED)
        self.lbl_sensor_status.config(text="Niepodłączony", foreground="red")
        self.lbl_calib_status.config(text="Nie skalibrowany", foreground="orange")
        self.lbl_current_value.config(text="0.00")
        self.update_status("Czujnik odłączony")
    
    def calibrate_sensor(self):
        """Kalibracja czujnika."""
        if self.sensor.calibrate():
            self.lbl_calib_status.config(text="Skalibrowany", foreground="green")
            self.lbl_offset.config(text=f"Offset: {self.sensor.zero_offset:.4f}")
            self.update_status("Czujnik skalibrowany")
            messagebox.showinfo("Sukces", "Pomyślnie skalibrowano czujnik")
        else:
            messagebox.showerror("Błąd", "Nie udało się skalibrować czujnika")
    
    def update_sensitivity(self, value):
        """Aktualizacja czułości czujnika."""
        self.sensor.set_sensitivity(float(value))
        self.update_status(f"Czułość: {value}")
    
    def zero_sensor(self):
        """Zerowanie czujnika (tara)."""
        if self.sensor.is_connected and self.sensor.is_calibrated:
            self.sensor.zero_offset = 0.0
            self.lbl_offset.config(text="Offset: 0.0000")
            self.update_status("Czujnik wyzerowany")
        else:
            messagebox.showwarning("Ostrzeżenie", "Czujnik musi być podłączony i skalibrowany")
    
    def toggle_monitoring(self):
        """Przełączanie monitorowania."""
        if self.is_monitoring:
            self.stop_monitoring()
        else:
            self.start_monitoring()
    
    def start_monitoring(self):
        """Rozpoczęcie monitorowania."""
        if not self.sensor.is_connected or not self.sensor.is_calibrated:
            messagebox.showwarning("Ostrzeżenie", "Czujnik musi być podłączony i skalibrowany")
            return
        
        self.is_monitoring = True
        self.btn_start.config(text="Zatrzymaj pomiar")
        self.update_sensor_value()
        self.update_status("Monitorowanie aktywne")
    
    def stop_monitoring(self):
        """Zatrzymanie monitorowania."""
        self.is_monitoring = False
        if self.monitoring_job:
            self.root.after_cancel(self.monitoring_job)
        self.btn_start.config(text="Rozpocznij pomiar")
        self.update_status("Monitorowanie zatrzymane")
    
    def update_sensor_value(self):
        """Aktualizacja wyświetlanej wartości z czujnika."""
        if self.is_monitoring:
            value = self.sensor.read_value()
            if value is not None:
                self.lbl_current_value.config(text=f"{value:.2f}")
            sampling_rate = int(self.combo_sampling.get())
            self.monitoring_job = self.root.after(int(1000 / sampling_rate), self.update_sensor_value)
    
    def load_config_file(self):
        """Wczytanie pliku konfiguracyjnego."""
        file_path = filedialog.askopenfilename(
            title="Wybierz plik konfiguracyjny",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")]
        )
        if file_path:
            self.config_manager.config_file = file_path
            self.config_manager.config = self.config_manager.load_config()
            self.load_app_config()
            self.update_status(f"Wczytano konfigurację z {file_path}")
    
    def save_config_file(self):
        """Zapis pliku konfiguracyjnego."""
        file_path = filedialog.asksaveasfilename(
            title="Zapisz konfigurację jako",
            defaultextension=".json",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")]
        )
        if file_path:
            self.config_manager.config_file = file_path
            self.save_settings()
            if self.config_manager.save_config():
                self.update_status(f"Zapisano konfigurację do {file_path}")
    
    def save_settings(self):
        """Zapis ustawień do konfiguracji."""
        user_name = self.entry_user_name.get()
        user_age = self.entry_user_age.get()
        
        self.config_manager.set("user.name", user_name)
        self.config_manager.set("user.age", int(user_age) if user_age.isdigit() else 0)
        
        try:
            self.config_manager.set("thresholds.min_force", float(self.entry_min_force.get()))
            self.config_manager.set("thresholds.max_force", float(self.entry_max_force.get()))
            self.config_manager.set("thresholds.target_force", float(self.entry_target_force.get()))
        except ValueError:
            messagebox.showerror("Błąd", "Nieprawidłowe wartości progów siły")
            return
        
        self.config_manager.set("sensor.sensitivity", float(self.scale_sensitivity.get()))
        self.config_manager.set("sensor.sampling_rate", int(self.combo_sampling.get()))
        
        self.config_manager.save_config()
        self.lbl_user.config(text=user_name)
        self.update_status("Zapisano ustawienia")
        messagebox.showinfo("Sukces", "Ustawienia zostały zapisane")
    
    def reset_settings(self):
        """Przywrócenie ustawień domyślnych."""
        if messagebox.askyesno("Potwierdzenie", "Czy na pewno chcesz przywrócić ustawienia domyślne?"):
            self.config_manager.config = self.config_manager.load_config()
            self.load_app_config()
            self.update_status("Przywrócono ustawienia domyślne")
    
    def start_exercise(self):
        """Rozpoczęcie ćwiczenia."""
        selected = self.exercises_tree.selection()
        if selected:
            item = self.exercises_tree.item(selected[0])
            exercise_name = item["values"][0]
            messagebox.showinfo("Ćwiczenie", f"Rozpoczynanie ćwiczenia: {exercise_name}")
        else:
            messagebox.showwarning("Ostrzeżenie", "Wybierz ćwiczenie z listy")
    
    def add_exercise(self):
        """Dodawanie nowego ćwiczenia."""
        messagebox.showinfo("Info", "Funkcja dodawania ćwiczeń w implementacji")
    
    def export_history(self):
        """Eksport historii sesji."""
        messagebox.showinfo("Info", "Funkcja eksportu historii w implementacji")
    
    def clear_history(self):
        """Czyszczenie historii."""
        if messagebox.askyesno("Potwierdzenie", "Czy na pewno chcesz wyczyścić historię?"):
            for item in self.history_tree.get_children():
                self.history_tree.delete(item)
            self.update_status("Wyczyszczono historię")
    
    def update_status(self, message):
        """Aktualizacja paska statusu."""
        self.status_bar.config(text=message)
    
    def show_about(self):
        """Wyświetlenie informacji o programie."""
        about_text = f"""
        Aplikacja Biofeedback
        Wersja: {self.config_manager.get('version', '1.0.0')}
        
        Aplikacja do treningu biofeedback z wykorzystaniem
        czujnika tensometrycznego.
        
        © 2024
        """
        messagebox.showinfo("O programie", about_text)
    
    def on_closing(self):
        """Obsługa zamykania aplikacji."""
        if self.is_monitoring:
            self.stop_monitoring()
        self.sensor.disconnect()
        self.root.destroy()


def main():
    """Funkcja główna."""
    root = tk.Tk()
    app = BiofeedbackApp(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()


if __name__ == "__main__":
    main()
