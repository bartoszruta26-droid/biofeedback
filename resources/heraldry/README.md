# Biblioteka znaków heraldycznych

Biblioteka zawiera 14 autorskich, skalowalnych ikon SVG przygotowanych do użycia w aplikacji Biofeedback. Są to współczesne ilustracje inspirowane językiem heraldyki, a nie potwierdzone odwzorowania historycznych herbów ani nadania tytułów czy godności.

## Herb Ciechomskich

Plik `ciechomscy-wez-jablko.svg` przedstawia węża oplatającego i zjadającego jabłko — zgodnie z opisem przekazanym w zleceniu. Projekt ma charakter koncepcyjny; jego zgodność z historycznym, źródłowo potwierdzonym herbem wymaga odrębnej weryfikacji heraldycznej.

## Zawartość

| Plik | Motyw |
|---|---|
| `ciechomscy-wez-jablko.svg` | Wąż i jabłko — Ciechomscy |
| `herb-rodu-klasyczny.svg` | Dąb i korzenie — ród |
| `herb-rodu-lilia.svg` | Lilia — ród |
| `herb-biskupi.svg` | Mitra i pastorał — godność biskupia |
| `herb-arcybiskupi.svg` | Krzyż podwójny — godność arcybiskupia |
| `herb-kardynalski.svg` | Kapelusz i chwosty — godność kardynalska |
| `herb-opacki.svg` | Księga i pastorał — godność opacka |
| `herb-rycerski.svg` | Miecz — motyw rycerski |
| `herb-medyczny.svg` | Laska i wąż — motyw medyczny |
| `herb-naukowy.svg` | Księga i gwiazda — nauka |
| `herb-fundacyjny.svg` | Serce i dłonie — fundacja |
| `herb-misjonarski.svg` | Glob i krzyż — misje |
| `herb-samorzadowy.svg` | Korona i kolumny — godność obywatelska |
| `herb-zaslugi.svg` | Gwiazda i wieniec — zasługi |

## Użycie w Qt

Zasoby są osadzone przez `heraldry.qrc`. Po kompilacji można odwoływać się do nich przez ścieżki:

```cpp
QIcon(QStringLiteral(":/heraldry/herb-biskupi.svg"));
QPixmap(QStringLiteral(":/heraldry/ciechomscy-wez-jablko.svg"));
```

SVG pozostają wektorowe i skalują się bez utraty ostrości. Pliki źródłowe są jednocześnie instalowane i kopiowane wraz z katalogiem `resources/` zgodnie z istniejącymi regułami CMake.
