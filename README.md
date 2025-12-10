# DetectWifiHack

DetectWifiHack is an ESP32-based Wi‑Fi sniffer that listens in promiscuous mode to detect common Wi‑Fi attack patterns and alerts via a buzzer. It currently focuses on:

- Deauthentication frames (often used in deauth/handshake‑capture attacks)
- Association/Authentication bursts that may correlate with certain attack workflows (experimental)

The code is written for ESP‑IDF and is intended to be built either with Espressif’s `idf.py` or through CLion’s ESP‑IDF CMake integration.


### Features (current)
- Promiscuous‑mode sniffer using `esp_wifi_set_promiscuous()`
- Logs basic metadata (source MAC, channel) with `ESP_LOGI`
- Audible alert via a buzzer on detection events

Note: Detection is heuristic and limited. False positives are possible. Use logs to investigate events.


### Hardware requirements
- ESP32 DevKit board (ESP32‑WROOM or ESP32‑WROVER class)
- 3.3 V active buzzer or piezo buzzer module
- Optional: NPN transistor + resistor if your buzzer draws more current than a GPIO can supply
- USB cable for power/programming


### Wiring (ESP32 ↔ buzzer)
The example firmware uses GPIO 2 for the alarm output:

- Connect buzzer positive (+) to `GPIO2`
- Connect buzzer negative (–) to `GND`

Recommendations and notes:
- If using a raw piezo or a buzzer that needs more current, place a small transistor (e.g., 2N2222) between the GPIO and the buzzer, with a base resistor (~1 kΩ), and power the buzzer from 3.3 V. Always connect grounds.
- GPIO2 is a bootstrapping pin on many ESP32 boards. Some buzzer modules may pull the line and cause boot issues. If you see boot failures, move the buzzer to a different pin and update `BUZZER_PIN` in `main/main.c` accordingly.
- Avoid 5 V buzzers directly on a GPIO.


### Project layout
- `main/main.c` — sniffer and detection logic (promiscuous RX callback + alarm)
- `CMakeLists.txt` — ESP‑IDF CMake project definition
- `autoexec.bat` — convenience script to activate ESP‑IDF v5.1.2 environment on Windows
- `sdkconfig` — project configuration


### How it works (high level)
1. Initializes NVS and the default event loop
2. Initializes Wi‑Fi in `WIFI_MODE_NULL` (no STA/AP)
3. Enables promiscuous mode and registers a RX callback
4. In the callback, inspects the IEEE 802.11 frame control bytes to flag potential attacks
5. On a match, logs details and sounds the buzzer briefly

Notes on detection:
- Deauthentication frames have Frame Control type/subtype `0xC0 0x00`. The code flags these as likely deauth events.
- The sample also checks for `0x30 0x00` frames (association request). Bursts may correlate with some attack setups, but this is experimental and likely noisy. Use logs to refine.


## Development environment

This project targets ESP‑IDF 5.1.x on Windows (see `autoexec.bat`). You can also use Linux/macOS with equivalent steps.

### Espressif setup (Windows)
1. Install ESP‑IDF 5.1.x using the official installer or `idf-installer`. The paths referenced here assume:
   - `D:\Espressif\frameworks\esp-idf-v5.1.2`
   - `D:\Espressif\python_env\idf5.1_py3.12_env`
2. Open a Developer PowerShell or CMD.
3. Run the environment scripts (you can use the provided helper):
   - `autoexec.bat` (in this repo) calls:
     - `D:\Espressif\python_env\idf5.1_py3.12_env\Scripts\activate.bat`
     - `D:\Espressif\frameworks\esp-idf-v5.1.2\export.bat`
   - Alternatively, run the above two commands manually to activate the Python venv and export ESP‑IDF environment variables.

Verify your installation:
```
idf.py --version
python --version
```


### Building and flashing with idf.py
1. Connect the ESP32 board. Note the COM port (e.g., `COM5`) in Windows Device Manager.
2. From a terminal with ESP‑IDF environment active (see above):
```
idf.py set-target esp32
idf.py build
idf.py -p COM5 flash monitor
```
Replace `COM5` with your actual port. Press `Ctrl+]` to exit the monitor.


### Building inside CLion (MinGW toolchain)
This repository includes a CLion profile named “Debug”. To build from CLion:
- Ensure your ESP‑IDF environment is exported before starting CLion (run `autoexec.bat` in the same shell, then start CLion from that shell), or configure CLion’s ESP‑IDF toolchain integration.
- Build the `wifi_hack_detect` target.

From the command line using CLion’s active CMake profile (example):
```
cmake --build cmake-build-debug --target wifi_hack_detect
```
Flashing/monitoring is typically done with `idf.py`; CLion can be configured to call it as an External Tool or via ESP‑IDF plugin support.


## Configuration
- `sdkconfig` controls Wi‑Fi, logging level, and other IDF options.
- Promiscuous capture behavior may be constrained by regulatory domain and chip/SDK limitations. ESP32 promiscuous mode does not provide full 802.11 monitor capabilities and some management fields may be truncated.
- Channel hopping is not implemented in this minimal example. You can add a timer to iterate channels with `esp_wifi_set_channel()` for broader coverage (note: this can increase false positives and CPU use).


## Troubleshooting
- Board does not boot after wiring the buzzer to GPIO2:
  - Unplug the buzzer and try again. If it boots, move the alarm to a different GPIO and update `BUZZER_PIN` in `main.c`.
- Build errors about missing ESP‑IDF or Python:
  - Ensure you have run `export.bat` and activated the matching Python environment as in `autoexec.bat`.
- Flashing fails on Windows:
  - Close any open serial monitor, select the correct COM port, and check that your USB cable supports data.
- No detections in logs:
  - Increase logging level (`menuconfig` → Component config → Log output). Ensure you’re on a busy Wi‑Fi channel, or implement channel hopping.


## Legal and ethical notice
Promiscuous Wi‑Fi capture is regulated in many jurisdictions. Only capture RF traffic that you are legally permitted to observe, and only on your own networks or with explicit authorization. This tool is for defensive research and educational purposes. You are responsible for compliance with applicable laws.


## License
This project is released under the terms of the license in `LICENSE`.


## Roadmap (ideas)
- Implement configurable channel hopping
- Add filtering and counters per MAC/channel
- Persist events to NVS with timestamps
- Optional MQTT/Serial reporting instead of or in addition to the buzzer
