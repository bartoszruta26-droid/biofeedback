#!/bin/bash

# Konfiguracja - EDYTUJ TE WARTOŚCI
GITHUB_USER="TWOJA_NAZWA_UZYTKOWNIKA"  # Wpisz swoją nazwę użytkownika GitHub
REPO_NAME="hx711-arduino"              # Wpisz nazwę repozytorium (musi istnieć jako puste repo na GH)
BRANCH_NAME="main"

# Sprawdź czy plik źródłowy istnieje
if [ ! -f "hx711_advanced.ino" ]; then
    echo "BŁĄD: Nie znaleziono pliku hx711_advanced.ino w bieżącym katalogu."
    exit 1
fi

echo "Konfigurowanie Git dla repozytorium: https://github.com/$GITHUB_USER/$REPO_NAME"

# Inicjalizacja git (jeśli jeszcze nie zrobione)
if [ ! -d ".git" ]; then
    git init
fi

# Dodanie wszystkich plików
git add .

# Commit zmian
git commit -m "Initial commit: HX711 High-Speed Logger (80Hz, Binary Protocol, Watchdog)"

# Dodanie zdalnego repozytorium (usunięcie starego jeśli istnieje)
git remote remove origin 2>/dev/null
git remote add origin "https://github.com/$GITHUB_USER/$REPO_NAME.git"

# Pobranie najnowszych zmian (na wypadek gdyby repo nie było puste)
git fetch origin
git branch -M $BRANCH_NAME

echo ""
echo "---------------------------------------------------------"
echo "Gotowe do wysłania!"
echo "Wymagany jest Token Dostępu (Personal Access Token) z uprawnieniem 'repo'."
echo "Gdy zostaniesz poproszony o hasło, wklej swój token."
echo "---------------------------------------------------------"
echo ""

# Wypchnięcie kodu
git push -u origin $BRANCH_NAME

if [ $? -eq 0 ]; then
    echo ""
    echo "SUKCES! Kod został wysłany na GitHub."
    echo "Adres: https://github.com/$GITHUB_USER/$REPO_NAME"
else
    echo ""
    echo "Wystąpił błąd podczas wysyłania. Sprawdź poprawność tokena i nazwy repozytorium."
fi
