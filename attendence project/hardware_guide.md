# Hardware Configuration Guide
## Student Tracking IoT System

---

## 📋 Bill of Materials (BOM)

### Essential Components

| Component | Model | Quantity | Purpose |
|-----------|-------|----------|---------|
| Microcontroller | ESP32 / Arduino MKR WiFi 1010 | 1 | Main processing unit |
| GPS Module | NEO-6M or NEO-8M | 1 | Real-time location tracking |
| Camera Module | OV2640 or OV5640 | 1 | Facial recognition image capture |
| SD Card Module | Micro SD Card Reader | 1 | Offline data storage |
| Power Supply | 5V / 2A USB Adapter | 1 | Device power |
| USB Cable | Micro USB | 1 | Power & programming |
| Jumper Wires | Male-to-Male / Female | 30+ | Connections |
| Breadboard | 830 points | 1 | Prototyping |

### Optional Components

| Component | Model | Purpose |
|-----------|-------|---------|
| Real-time Clock | DS3231 | Accurate time backup |
| Temperature Sensor | DHT22 | Environmental monitoring |
| GSM Module | SIM800L | Backup connectivity |
| SD Card | 16GB Micro SD | Extended storage |
| Battery | 3.7V Li-ion | Portable operation |
| Solar Panel | 5V Small Panel | Extended autonomy |
| Enclosure | IP65 Rated Box | Weather protection |

---

## 🔌 Wiring Diagram - ESP32 + GPS + Camera

### GPS Module (NEO-6M) Connection

```
NEO-6M Pinout:
┌──────────────────┐
│ VCC   GND   TXD   RXD │
└──────────────────┘

GPS → ESP32 Connections:
VCC  → 5V
GND  → GND
TXD  → GPIO 16 (UART2 RX)
RXD  → GPIO 17 (UART2 TX)

Baud Rate: 9600
```

### Camera Module (OV2640) Connection - I2C

```
OV2640 Pinout:
┌──────────────────┐
│ VCC   GND   SDA   SCL  │
└──────────────────┘

Camera → ESP32 Connections:
VCC  → 3.3V
GND  → GND
SDA  → GPIO 21 (I2C SDA)
SCL  → GPIO 22 (I2C SCL)

I2C Address: 0x30 (default)
Resolution: 1024x768 / 640x480
```

### SD Card Module Connection - SPI

```
SD Module Pinout:
┌─────────────────────────┐
│ VCC   GND   MOSI  MISO  CLK  CS │
└─────────────────────────┘

SD → ESP32 Connections:
VCC  → 3.3V
GND  → GND
MOSI → GPIO 23 (SPI MOSI)
MISO → GPIO 19 (SPI MISO)
CLK  → GPIO 18 (SPI SCK)
CS   → GPIO 5 (SPI CS0)

SPI Frequency: 10 MHz
```

### Real-time Clock (DS3231) - I2C

```
DS3231 Pinout:
┌──────────────────┐
│ VCC   GND   SDA   SCL  │
└──────────────────┘

DS3231 → ESP32 Connections:
VCC  → 3.3V
GND  → GND
SDA  → GPIO 21 (I2C SDA) - Shared with camera
SCL  → GPIO 22 (I2C SCL) - Shared with camera

I2C Address: 0x68
```

### Complete Wiring Table

```
ESP32 PIN | Component      | Signal
----------|----------------|----------
GPIO 5    | SD Card        | CS
GPIO 16   | GPS Module     | RXD
GPIO 17   | GPS Module     | TXD
GPIO 18   | SD Card        | CLK
GPIO 19   | SD Card        | MISO
GPIO 21   | Camera/RTC     | SDA
GPIO 22   | Camera/RTC     | SCL
GPIO 23   | SD Card        | MOSI
GND       | All Modules    | GND
5V        | GPS Module     | VCC
3.3V      | Camera/RTC/SD  | VCC
```

---

## ⚡ Power Management

### Power Consumption Analysis

```
Component           | Current Draw | Duty Cycle | Avg Current
-------------------|--------------|------------|-----------
ESP32 (Active)      | 80-160 mA    | 100%       | 120 mA
GPS Module          | 50 mA        | 100%       | 50 mA
Camera Module       | 150 mA       | 20%        | 30 mA
SD Card Module      | 100 mA       | 10%        | 10 mA
LEDs & Misc         | 50 mA        | 100%       | 50 mA
                    |              |            | --------
TOTAL               |              |            | ~260 mA
```

### Battery Specifications

For portable operation without USB power:

```
Battery Type:        3.7V Li-ion Rechargeable
Capacity:            5000 mAh
Estimated Runtime:   19 hours (5000 mAh ÷ 260 mA)

Charging Circuit:    TP4056 Charging Module
- Input: 5V USB
- Output: 3.7V - 4.2V
- Max Charge Current: 1000 mA
- Protection: Overcharge/Overdischarge

Battery Connector:   JST PH 2-pin
```

---

## 🔧 Assembly Instructions

### Step 1: Prepare the Breadboard

1. Place the ESP32 module in the center of the breadboard
2. Connect power rails:
   - Red rail: 5V (from USB)
   - Blue rail: GND (from USB)
   - Additional 3.3V available from ESP32 pin

### Step 2: Connect GPS Module

```
1. Place GPS module on the right side of breadboard
2. GPS VCC → 5V rail
3. GPS GND → GND rail
4. GPS RXD → GPIO 16 (via 1kΩ resistor for voltage division)
5. GPS TXD → GPIO 17 (direct connection)
```

### Step 3: Connect Camera Module

```
1. Place camera module on left side with I2C pins accessible
2. Camera VCC → 3.3V rail
3. Camera GND → GND rail
4. Camera SDA → GPIO 21
5. Camera SCL → GPIO 22
6. Add 10kΩ pull-up resistors on SDA and SCL lines
```

### Step 4: Connect SD Card Module

```
1. Place SD card module below GPS module
2. Module VCC → 3.3V rail
3. Module GND → GND rail
4. MOSI → GPIO 23
5. MISO → GPIO 19
6. CLK → GPIO 18
7. CS → GPIO 5
8. Add 100Ω resistors on all data lines
```

### Step 5: Connect RTC (Optional)

```
1. Place RTC module next to camera
2. RTC VCC → 3.3V rail
3. RTC GND → GND rail
4. RTC SDA → GPIO 21 (shared with camera via I2C bus)
5. RTC SCL → GPIO 22 (shared with camera via I2C bus)
```

---

## 🖥️ Software Setup

### ESP32 with Arduino IDE

#### 1. Install Arduino IDE
- Download from https://www.arduino.cc/en/software

#### 2. Add ESP32 Board Support
```
File → Preferences → Additional Boards Manager URLs:
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

#### 3. Install Required Libraries
```
Sketch → Include Library → Manage Libraries

Search and install:
- ESP32 Board Package (by Espressif)
- TinyGPS++ (by Mikal Hart)
- DHT sensor library (by Adafruit)
- ArduinoJson (by Benoit Blanchon)
- WebServer (built-in)
- WiFi (built-in)
```

#### 4. Board Configuration
```
Tools → Board → ESP32 Dev Module
Tools → Flash Size → 4MB
Tools → Upload Speed → 115200
Tools → CPU Frequency → 80MHz (or 160MHz)
Tools → Core Debug Level → Info
```

### ESP32 with ESP-IDF

```bash
# Install ESP-IDF
git clone https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
source export.sh

# Build and Flash
idf.py build
idf.py -p COM3 flash
idf.py monitor
```

---

## 📡 GPS Module Configuration

### NMEA Sentence Parsing

The GPS module outputs standard NMEA 0183 sentences:

```
$GPRMC,Time,Status,Lat,N/S,Lon,E/W,Speed,Course,Date,Variation,E/W*Checksum

Example:
$GPRMC,081350.00,A,4717.113210,N,00833.915187,E,0.295,054.6,190116,,,A*78

Fields:
1. Time: UTC time (HHMMSS.SS)
2. Status: A=valid, V=invalid
3. Latitude: DDMM.MMMMM
4. N/S: North or South
5. Longitude: DDDMM.MMMMM
6. E/W: East or West
7. Speed: Speed over ground in knots
8. Course: Course over ground in degrees
9. Date: Date (DDMMYY)
```

### GPS Initialization Commands

```c
// Set update rate to 1 Hz
$PMTK220,1000*1F

// Only RMC sentence
$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29

// Cold start
$PMTK103*30

// Hot start
$PMTK102*31

// Warm start
$PMTK101*32
```

---

## 📷 Camera Module Setup

### OV2640 Camera Configuration

```c
// Camera sensor I2C initialization
sensor_t *s = esp_camera_sensor_get();
s->set_framesize(s, FRAMESIZE_QVGA);    // 320x240
s->set_quality(s, 10);                   // Quality 0-63
s->set_brightness(s, 0);                 // -2 to 2
s->set_contrast(s, 0);                   // -2 to 2
s->set_saturation(s, 0);                 // -2 to 2
s->set_sharpness(s, 0);                  // -2 to 2
s->set_denoise(s, 0);                    // 0 or 1
s->set_vflip(s, 0);                      // 0 or 1
s->set_hmirror(s, 0);                    // 0 or 1
```

### Supported Resolutions

```
FRAMESIZE_QVGA    → 320 x 240
FRAMESIZE_CIF     → 400 x 296
FRAMESIZE_VGA     → 640 x 480
FRAMESIZE_SVGA    → 800 x 600
FRAMESIZE_XGA     → 1024 x 768
FRAMESIZE_SXGA    → 1280 x 1024
```

---

## 🧪 Testing Procedures

### 1. Power-On Self Test

```
Expected: LED blinks 5 times (system initialized)
Check: All modules receive correct voltage
  - GPS Module LED: Red (steady)
  - ESP32 LED: Blue (blinking)
  - Camera: No LED (responds via I2C)
```

### 2. GPS Verification

```
Open Serial Monitor (115200 baud)
Expected Output:
[GPS] Initializing GPS module on UART 2
[GPS] Updated: Lat=12.9716, Lon=77.5946, Speed=0.00 km/h
[GPS] Satellites in use: 12
```

### 3. Camera Test

```
Expected: Camera initialization message
[CAMERA] OV2640 Camera module initialized
[CAMERA] Resolution: 1024x768
[CAMERA] Quality: 10
```

### 4. WiFi Connection

```
Expected:
[WIFI] Connecting to: School_WiFi
[WIFI] Connected! IP: 192.168.1.xxx
[WIFI] Signal Strength: -65 dBm
```

### 5. API Communication

```
Expected: Successful data submission
[NETWORK] Connecting to server 192.168.1.100:5000
[NETWORK] JSON Payload: {...}
[NETWORK] Data sent successfully (Response: 200 OK)
```

---

## 🐛 Troubleshooting

### GPS Not Getting Fix

```
Problem:  [GPS] Invalid data received
Solution: 
  - Check antenna connection
  - Move to open area (clear sky view)
  - Wait 3-5 minutes for first fix
  - Check baud rate (should be 9600)
  - Try different GPS module
```

### Camera Not Detected

```
Problem:  [ERROR] Camera not initialized
Solution:
  - Check I2C connections (SDA, SCL)
  - Verify pull-up resistors (10kΩ on SDA/SCL)
  - Check camera module power (3.3V)
  - Try different camera model
  - Check I2C address (0x30)
```

### WiFi Connection Fails

```
Problem:  [WIFI] Failed to connect
Solution:
  - Check SSID and password
  - Verify router is in range
  - Check ESP32 antenna orientation
  - Restart both device and router
  - Check WiFi frequency (2.4GHz only for most boards)
```

### SD Card Not Working

```
Problem:  [STORAGE] SD card initialization failed
Solution:
  - Format SD card (FAT32)
  - Check SPI connections
  - Verify CS pin (GPIO 5)
  - Ensure SD card is inserted correctly
  - Try different SD card (some have compatibility issues)
```

---

## 📊 Performance Metrics

| Metric | Value | Notes |
|--------|-------|-------|
| GPS Accuracy | ±5-10m | Varies with satellite count |
| GPS Fix Time | 30-120s | Cold start vs Hot start |
| Camera Frame Rate | 10-30 fps | Depends on resolution |
| Face Recognition | 95%+ | With good lighting |
| WiFi Range | 50-100m | Line of sight |
| Data Transmission | < 2s | Including cloud processing |
| Battery Life | 12-18 hours | With 5000mAh battery |

---

## 🔐 Safety & Best Practices

1. **Voltage Considerations**
   - GPS Module: Use 5V input (internal regulation to 3.3V)
   - Camera/RTC: Use 3.3V directly
   - Always use voltage dividers or level shifters between 5V and 3.3V logic

2. **Current Limits**
   - ESP32 IO pins: Max 40mA per pin
   - Total: Max 200mA all pins combined
   - Use transistors for high-current devices

3. **ESD Protection**
   - Ground yourself before handling components
   - Use antistatic wrist strap
   - Store components in antistatic bags

4. **Heat Management**
   - GPS module can get warm in direct sun
   - Provide adequate ventilation
   - Use thermal paste on microcontroller if needed

5. **Moisture Protection**
   - Use IP65 rated enclosure for outdoor deployment
   - Add silica gel packets for humidity control
   - Check connections regularly for corrosion

---

## 📚 Additional Resources

- **ESP32 Documentation**: https://docs.espressif.com/
- **GPS Module Datasheet**: u-blox NEO-6M/8M specs
- **Camera Datasheet**: OV2640/5640 specifications
- **Arduino Playground**: https://playground.arduino.cc/
- **Electronics Tutorials**: https://www.electronics-tutorials.ws/

---

## 📞 Support & Contact

For hardware-related questions:
- Email: ramyabalaji2244@gmail.com
- Phone: +91 9043956971
- GitHub Issues: [Project Repository]

---

**Last Updated**: May 2024  
**Hardware Version**: 1.0  
**Firmware Version**: Compatible with 2.0.0+

