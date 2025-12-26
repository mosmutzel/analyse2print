# Screen Switch Button Konfiguration

## Standard-Konfiguration (GPIO0 - BOOT Button)

Der **BOOT Button** auf dem T-Display-S3-Pro wird standardmäßig zum Umschalten zwischen Main und Debug Screen verwendet.

### Verwendung:
1. **Kurz drücken** = Zwischen MAIN und DEBUG Screen wechseln
2. Beim Start ist der MAIN Screen aktiv
3. Jeder Tastendruck wechselt zum anderen Screen

### Vorteile:
- ✅ Keine externe Hardware erforderlich
- ✅ Button ist bereits auf dem Board vorhanden
- ✅ Zuverlässiges Debouncing implementiert

### Nachteile:
- ⚠️ BOOT Button hat auch andere Funktionen (Firmware Upload)
- ⚠️ Nicht drücken während Upload/Boot-Prozess

---

## Alternative Konfiguration (GPIO16 - Externer Button)

Falls du einen **externen Button** verwenden möchtest, kannst du GPIO16 nutzen.

### Hardware-Anschluss:
```
Button:  GPIO16 ----[Button]---- GND
         (mit internem Pull-up)
```

### Software-Konfiguration:

In **include/display.h** ändern:
```cpp
// Von:
#define SCREEN_SWITCH_PIN 0

// Zu:
#define SCREEN_SWITCH_PIN 16
```

Oder in **platformio.ini** unter `build_flags` hinzufügen:
```ini
-DSCREEN_SWITCH_PIN=16
```

### Vorteile:
- ✅ Dedizierter Button nur für Screen-Wechsel
- ✅ Keine Konflikte mit BOOT-Funktion

### Nachteile:
- ⚠️ Externe Hardware erforderlich (Button + Kabel)
- ⚠️ GPIO16 muss frei sein (nicht anderweitig verwendet)

---

## Debug-Ausgaben

Beim Start zeigt das Serial-Log:
```
Screen switch button initialized on GPIO0, current state: HIGH
```

Alle 5 Sekunden:
```
GPIO0 state: HIGH
```

Bei Tastendruck:
```
GPIO0 state change detected!
GPIO0: Switched to DEBUG screen
```

---

## Verfügbare GPIOs auf T-Display-S3-Pro

### Bereits belegt:
- **GPIO 5, 6**: I2C (PMU SY6970)
- **GPIO 8**: TFT_MISO
- **GPIO 9**: TFT_DC
- **GPIO 17**: TFT_MOSI
- **GPIO 18**: TFT_SCLK
- **GPIO 39**: TFT_CS
- **GPIO 47**: TFT_RST
- **GPIO 48**: TFT_BL (Backlight)

### Frei verfügbar:
- **GPIO 0**: BOOT Button (Standard)
- **GPIO 16**: Frei für externe Anwendungen
- **GPIO 14**: Frei
- **GPIO 21**: Frei

---

## Problemlösung

### Button funktioniert nicht:

1. **Serial Monitor prüfen**:
   ```
   pio device monitor --baud 115200
   ```

2. **Pin-Status prüfen**:
   - Zeigt "GPIO0 state: HIGH" wenn nicht gedrückt?
   - Zeigt "GPIO0 state: LOW" wenn gedrückt?

3. **Bei externem Button (GPIO16)**:
   - Kabel-Verbindung prüfen
   - Button zwischen GPIO16 und GND
   - SCREEN_SWITCH_PIN korrekt gesetzt?

### Screen wechselt nicht:

1. **LVGL prüfen**:
   - Ist displayLoop() im main loop aktiv?
   - Werden beide Screens erstellt (MAIN + DEBUG)?

2. **Debug-Ausgaben prüfen**:
   - "state change detected!" wird angezeigt?
   - "Switched to..." Nachricht erscheint?

---

## Code-Referenz

### display.cpp
```cpp
static void checkButtonAndSwitchScreen() {
  bool reading = digitalRead(BUTTON_PIN);

  // Debouncing mit 50ms Verzögerung
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW) {
      // Toggle zwischen MAIN und DEBUG
      currentScreenIsMain = !currentScreenIsMain;
      loadScreen(currentScreenIsMain ? SCREEN_ID_MAIN : SCREEN_ID_DEBUG);
    }
  }
}
```

### main.cpp loop()
```cpp
void loop() {
  // ... andere Tasks ...

  displayLoop();  // ← Wichtig! Muss im loop aufgerufen werden

  delay(5);
}
```
