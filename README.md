# Mifare NUID Reader for Flipper Zero

A simple Flipper Zero application that scans Mifare Classic NFC cards and displays the NUID (UID) on the screen.

## Features
- Scans Mifare Classic cards
- Displays UID in hexadecimal format
- Visual and haptic feedback when card is detected
- Simple, clean interface

## Installation

### Method 1: Using ufbt (Recommended)
1. Install ufbt if you haven't already:
   ```bash
   pip install ufbt
   ```

2. Create a folder for your app and copy both files into it:
   - `mifare_nuid_reader.c`
   - `application.fam`

3. Build the app:
   ```bash
   ufbt
   ```

4. Install to Flipper Zero (connect via USB):
   ```bash
   ufbt launch
   ```

### Method 2: Using firmware build system
1. Clone the Flipper Zero firmware repository
2. Create a folder: `applications_user/mifare_nuid_reader/`
3. Copy both files into this folder
4. Build the firmware or just the app:
   ```bash
   ./fbt fap_mifare_nuid_reader
   ```
5. Copy the generated `.fap` file to your Flipper Zero's SD card under `apps/NFC/`

## Usage
1. Launch the app from: Apps → NFC → Mifare NUID Reader
2. Place a Mifare Classic card near the Flipper Zero's NFC reader
3. The UID will be displayed on screen
4. The Flipper will vibrate when a card is detected
5. Press Back button to exit

## File Structure
- `mifare_nuid_reader.c` - Main application code
- `application.fam` - Application manifest

## Notes
- This app works with Mifare Classic cards (and most NFC cards that support ISO14443A)
- The UID is displayed in hexadecimal format with spaces between bytes
- Compatible with Flipper Zero firmware version 0.98.0 and later

## Troubleshooting
- If the app doesn't compile, make sure you have the latest firmware SDK
- If no UID appears, ensure your card is properly placed on the NFC reader
- Some cards may take a second to be detected - keep the card steady

## License
Free to use and modify
