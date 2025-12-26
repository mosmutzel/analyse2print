# PMU (Power Management Unit) Konfiguration

## Hardware: SY6970 Chip auf LilyGo T-Display-S3-Pro

### Aktuelle Konfiguration (mit Batterie + USB Analyzer)

```cpp
PMU.enableOTG();        // USB Host für Analyzer aktiviert
PMU.enableADCMeasure(); // Batteriespannungs-Messung aktiv
PMU.enableCharge();     // Batterieladung aktiviert
```

## Wichtige Hinweise

### 1. OTG-Modus (USB Host)
- **MUSS aktiviert sein** für den USB Analyzer Betrieb
- Der ESP32-S3 fungiert als USB Host und liest den Analyzer aus
- OTG stellt 5V Spannung für den Analyzer bereit

### 2. Batterieladung
- **Aktiviert** wenn Batterie angeschlossen ist
- Standard-Ladestrom: ~1A (kann angepasst werden)
- PMU lädt automatisch wenn USB-C Kabel angeschlossen ist

### 3. ADC Messung
- Überwacht Batteriespannung
- Überwacht USB-Spannung
- Kann für Batterie-Anzeige verwendet werden

## Pin-Belegung

### I2C für PMU
- **SDA**: GPIO 5
- **SCL**: GPIO 6
- **I2C-Adresse**: 0x6B (SY6970_SLAVE_ADDRESS)

### Display Backlight
- **BL**: GPIO 48
- **Steuerung**: Digital HIGH/LOW (V1.1 verwendet konstante Stromquelle)

## Batterie-Management

### Empfehlungen
1. **Mit Batterie**: Aktuelle Konfiguration verwenden
2. **Ohne Batterie**:
   - `PMU.disableCharge()` verwenden
   - OTG trotzdem aktiviert lassen für Analyzer

### Ladestrom anpassen (optional)
```cpp
// Ladestrom auf 500mA reduzieren (schonender für Batterie)
PMU.setChargerConstantCurr(500);
```

## Stromverbrauch

### Typische Werte
- **Display an**: ~150-200mA
- **BLE aktiv**: ~50mA
- **USB Analyzer**: ~100-150mA
- **Gesamt**: ~300-400mA während Betrieb

### Batterie-Laufzeit (geschätzt)
Mit 3000mAh Batterie: ~7-10 Stunden Dauerbetrieb

## Troubleshooting

### Display zeigt nichts
1. PMU-Initialisierung prüfen (Serial Monitor)
2. Backlight-Pin prüfen (GPIO 48)
3. I2C-Verbindung zum PMU testen

### USB Analyzer funktioniert nicht
1. OTG-Modus aktiviert? (`PMU.enableOTG()`)
2. USB-C Kabel korrekt angeschlossen?
3. Analyzer-Stromversorgung ausreichend?

### Batterie lädt nicht
1. Ist Ladung aktiviert? (`PMU.enableCharge()`)
2. USB-C Netzteil mit mindestens 1A verwenden
3. Batterie-Anschluss überprüfen

## Versionsunterschiede

### V1.0 vs V1.1
- **V1.0**: PWM Backlight-Steuerung
- **V1.1**: Konstante Stromquelle für Backlight (stabiler)
- Markierung: "V1.1" neben USB-C Buchse

## Serial Debug Ausgaben

```
PMU initialized successfully
OTG enabled for USB Analyzer
Battery charging enabled
Display initialized
```

Falls PMU-Init fehlschlägt:
```
Warning: PMU initialization failed - display may not work correctly
```

## Weitere Funktionen (optional)

### Batteriespannung auslesen
```cpp
uint16_t vbat = PMU.getBattVoltage();  // in mV
Serial.printf("Battery: %d mV\n", vbat);
```

### USB-Spannung prüfen
```cpp
bool usbPresent = PMU.isVbusIn();
Serial.printf("USB connected: %s\n", usbPresent ? "Yes" : "No");
```

### Ladestrom auslesen
```cpp
uint16_t current = PMU.getChargeCurrent();  // in mA
Serial.printf("Charge current: %d mA\n", current);
```
