# Analyse2Print Firmware

Firmware für den LilyGo T-Display-S3-Pro Gas-Analyzer mit Niimbot B1 Drucker.

## Features

- **Gas-Analyse**: O2 und He Messung via USB Analyzer
- **Automatischer Druck**: Labels mit MOD/END Berechnung
- **WiFi Manager**: Netzwerke scannen, verbinden und speichern
- **WiFi Auto-Connect**: Automatische Verbindung zu gespeicherten Netzwerken beim Start
- **Namen-Speicher**: Taucher-Namen für Labels speichern
- **OTA Update**: Firmware-Update über WiFi

## Hardware
- Lilygo T-Display-S3-Pro (https://lilygo.cc/products/t-display-s3-pro?variant=43111690141877)
- Niimbot B1 Label Printer (https://niimbots.com/de/products/niimbot-b1-etikettendrucker-mit-klebeband?variant=45661628039404)
- Divesoft Gas Analyzer (https://www.divesoft.com/en/products/analyzers)
- USB C to USB C cable (Solo) or USB B to USB C cable (HE/O2)
- Niimbot Labels 50mm x 30mm (https://niimbots.com/de/collections/beschriftungsband-fur-b21/products/2-rolls-label-tape-for-b21-b1-b3s)

## Lizenzierung

Jedes Gerät benötigt einen individuellen Lizenzschlüssel basierend auf der eindeutigen Geräte-ID.


## Firmware Update

1. Verbinde mit einem WiFi-Netzwerk
2. Gehe zu Einstellungen > Info
3. Klicke auf "Update prüfen"
4. Falls eine neue Version verfügbar ist, wird sie automatisch installiert

## Manuelles Update

Die aktuelle Firmware-Datei: [firmware.bin](firmware/firmware.bin)

### Via USB:
```bash
esptool.py --chip esp32s3 --port /dev/ttyUSB0 write_flash 0x10000 firmware.bin
```

## Version

Aktuelle Version: 1.3.0

Siehe [version.json](firmware/version.json) für Details.

## Changelog

- **v1.3.0**: WiFi Auto-Connect zu gespeicherten Netzwerken beim Start
- **v1.2.0**: Lizenz-Manager
- **v1.1.0**: WiFi Manager, OTA Update Check
- **v1.0.0**: Initial Release
