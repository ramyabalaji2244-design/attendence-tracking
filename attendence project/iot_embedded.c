/**
 * Student Tracking & Attendance System - IoT Embedded Code
 * 
 * Hardware Requirements:
 * - ESP32 / Arduino with WiFi capability
 * - GPS Module (NEO-6M / Neo-8M)
 * - Camera Module (OV2640 / OV5640)
 * - SD Card Module (for local storage)
 * - Real-time Clock (DS3231)
 * 
 * Features:
 * - Face Recognition for Attendance
 * - GPS Tracking for Real-time Location
 * - Data Transmission to Cloud Server
 * - Offline Data Storage
 * - Power Management
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// ==================== Configuration ====================
#define DEVICE_ID "DEVICE_001"
#define STUDENT_ID "STU_12345"
#define SCHOOL_ID "SCHOOL_001"
#define API_SERVER "192.168.1.100"
#define API_PORT 5000
#define GPS_BAUD_RATE 9600
#define CAMERA_RESOLUTION "1024x768"
#define UPDATE_INTERVAL 30  // seconds
#define OFFLINE_STORAGE_SIZE 1000  // records

// ==================== Data Structures ====================
typedef struct {
    double latitude;
    double longitude;
    float accuracy;
    float speed;
    time_t timestamp;
    int satellites;
    float hdop;
} GPSData;

typedef struct {
    char image_data[50000];  // Base64 encoded image
    size_t data_size;
    time_t timestamp;
    float confidence;
    char student_id[20];
    char status[20];  // "present" or "absent"
} FaceRecognitionData;

typedef struct {
    GPSData gps;
    FaceRecognitionData face_data;
    time_t timestamp;
    int signal_strength;
    float battery_level;
    char device_status[20];
} SensorData;

typedef struct {
    SensorData data;
    char sync_status[20];  // "synced" or "pending"
} StorageRecord;

typedef struct {
    StorageRecord records[OFFLINE_STORAGE_SIZE];
    int record_count;
    int write_index;
} OfflineStorage;

// ==================== Global Variables ====================
OfflineStorage offline_storage;
GPSData current_gps;
FaceRecognitionData face_recognition_data;
int wifi_connected = 0;
float battery_level = 100.0;
int signal_strength = -65;  // dBm

// ==================== GPS Functions ====================

/**
 * Parse NMEA format GPS data from serial input
 * Example: $GPRMC,081350.00,A,4717.113210,N,00833.915187,E,0.295,054.6,190116,,,A*78
 */
int parse_gprmc_sentence(char *sentence, GPSData *gps_data) {
    char tokens[12][50];
    int token_count = 0;
    char temp[strlen(sentence)];
    strcpy(temp, sentence);

    // Split by comma
    char *token = strtok(temp, ",");
    while (token != NULL && token_count < 12) {
        strcpy(tokens[token_count], token);
        token_count++;
        token = strtok(NULL, ",");
    }

    if (token_count < 10) {
        printf("[GPS] Invalid NMEA sentence format\n");
        return 0;
    }

    // Parse time
    char *time_str = tokens[1];
    if (strlen(time_str) >= 6) {
        gps_data->timestamp = time(NULL);
    }

    // Parse status
    char *status = tokens[2];
    if (status[0] != 'A') {
        printf("[GPS] GPS data invalid (status: %s)\n", status);
        return 0;
    }

    // Parse latitude
    char *lat_str = tokens[3];
    char *lat_dir = tokens[4];
    double lat_deg = strtod(lat_str, NULL);
    gps_data->latitude = (lat_deg / 100.0);
    if (lat_dir[0] == 'S') {
        gps_data->latitude *= -1;
    }

    // Parse longitude
    char *lon_str = tokens[5];
    char *lon_dir = tokens[6];
    double lon_deg = strtod(lon_str, NULL);
    gps_data->longitude = (lon_deg / 100.0);
    if (lon_dir[0] == 'W') {
        gps_data->longitude *= -1;
    }

    // Parse speed
    gps_data->speed = strtof(tokens[7], NULL) * 1.852;  // Convert knots to km/h

    // Parse date
    // tokens[9] contains date in DDMMYY format

    printf("[GPS] Updated: Lat=%.6f, Lon=%.6f, Speed=%.2f km/h\n",
           gps_data->latitude, gps_data->longitude, gps_data->speed);

    gps_data->accuracy = 5.0;
    gps_data->satellites = 10;
    gps_data->hdop = 1.5;

    return 1;
}

/**
 * Initialize GPS module
 */
int gps_init(int uart_port) {
    printf("[GPS] Initializing GPS module on UART %d\n", uart_port);
    
    // Set baud rate
    // uart_set_baudrate(uart_port, GPS_BAUD_RATE);
    
    // Send configuration commands
    // uart_write_bytes(uart_port, "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n");  // RMC only
    // uart_write_bytes(uart_port, "$PMTK220,1000*1F\r\n");  // Update rate 1Hz
    
    printf("[GPS] GPS module initialized\n");
    return 1;
}

/**
 * Read GPS data from serial buffer
 */
int gps_read_data(GPSData *gps_data) {
    // In real implementation, this reads from UART buffer
    static int count = 0;
    
    // Simulate GPS data variation
    if (count % 2 == 0) {
        gps_data->latitude = 12.9716 + (rand() % 100) / 100000.0;
        gps_data->longitude = 77.5946 + (rand() % 100) / 100000.0;
    } else {
        gps_data->latitude = 12.9352 + (rand() % 100) / 100000.0;
        gps_data->longitude = 77.6245 + (rand() % 100) / 100000.0;
    }
    
    gps_data->speed = (rand() % 60) + 0.5;
    gps_data->accuracy = (rand() % 10) + 2.0;
    gps_data->satellites = (rand() % 4) + 8;
    gps_data->hdop = (rand() % 20) / 10.0 + 0.8;
    gps_data->timestamp = time(NULL);
    
    count++;
    return 1;
}

// ==================== Camera & Face Recognition Functions ====================

/**
 * Capture image from camera module
 */
int camera_capture_image(unsigned char *image_buffer, size_t *buffer_size) {
    printf("[CAMERA] Capturing image...\n");
    
    // In real implementation:
    // 1. Initialize camera interface (I2C/SPI)
    // 2. Set resolution and quality
    // 3. Trigger capture
    // 4. Read image data
    // 5. Compress to JPEG
    
    // Simulated image data (placeholder)
    *buffer_size = 2048;
    for (int i = 0; i < 2048; i++) {
        image_buffer[i] = (unsigned char)(rand() % 256);
    }
    
    printf("[CAMERA] Image captured (%zu bytes)\n", *buffer_size);
    return 1;
}

/**
 * Convert binary image data to Base64
 */
void binary_to_base64(unsigned char *binary, size_t bin_size,
                      char *base64_buffer, size_t buffer_size) {
    const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    int base64_index = 0;
    
    for (size_t i = 0; i < bin_size; i += 3) {
        unsigned char byte1 = binary[i];
        unsigned char byte2 = (i + 1 < bin_size) ? binary[i + 1] : 0;
        unsigned char byte3 = (i + 2 < bin_size) ? binary[i + 2] : 0;
        
        unsigned int b = (byte1 << 16) | (byte2 << 8) | byte3;
        
        base64_buffer[base64_index++] = base64_chars[(b >> 18) & 0x3F];
        base64_buffer[base64_index++] = base64_chars[(b >> 12) & 0x3F];
        base64_buffer[base64_index++] = (i + 1 < bin_size) ? base64_chars[(b >> 6) & 0x3F] : '=';
        base64_buffer[base64_index++] = (i + 2 < bin_size) ? base64_chars[b & 0x3F] : '=';
        
        if (base64_index >= buffer_size - 4) break;
    }
    
    base64_buffer[base64_index] = '\0';
}

/**
 * Process face recognition
 */
int face_recognition_process(FaceRecognitionData *face_data) {
    printf("[FACE_REC] Processing face recognition...\n");
    
    unsigned char image_buffer[50000];
    size_t image_size = 0;
    
    // Capture image
    if (!camera_capture_image(image_buffer, &image_size)) {
        printf("[FACE_REC] Image capture failed\n");
        return 0;
    }
    
    // Convert to Base64
    binary_to_base64(image_buffer, image_size, face_data->image_data, 50000);
    face_data->data_size = strlen(face_data->image_data);
    
    // In real implementation, this would:
    // 1. Send image to local ML model (TensorFlow Lite)
    // 2. Get face embedding
    // 3. Compare with stored face database
    // 4. Get confidence score
    
    // Simulated face recognition result
    face_data->confidence = 0.95 + (rand() % 5) / 100.0;  // 95-99%
    face_data->timestamp = time(NULL);
    strcpy(face_data->student_id, STUDENT_ID);
    strcpy(face_data->status, "present");
    
    printf("[FACE_REC] Face recognized - ID: %s, Confidence: %.2f%%\n",
           face_data->student_id, face_data->confidence * 100);
    
    return 1;
}

// ==================== Power Management ====================

/**
 * Read battery level from ADC
 */
float read_battery_level() {
    // In real implementation: read from ADC connected to battery
    // ADC range: 0-3.3V (0-4095 counts on ESP32)
    // Assuming voltage divider: V = (ADC_reading / 4095) * 3.3 * 2
    
    // Simulate battery level
    static int counter = 0;
    battery_level = 100.0 - (counter * 0.1);  // Decrease by 0.1% every update
    if (battery_level < 20.0) {
        battery_level = 100.0;  // Reset for simulation
        counter = 0;
    }
    counter++;
    
    return battery_level;
}

/**
 * Read WiFi signal strength
 */
int read_signal_strength() {
    // Returns RSSI in dBm (typical range: -100 to -30 dBm)
    // Simulate signal strength
    signal_strength = -65 + (rand() % 20) - 10;
    return signal_strength;
}

// ==================== Data Transmission Functions ====================

/**
 * Create JSON payload for API request
 */
int create_sensor_data_json(SensorData *sensor_data, char *json_buffer, size_t buffer_size) {
    int written = snprintf(json_buffer, buffer_size,
        "{"
        "\"device_id\":\"%s\","
        "\"student_id\":\"%s\","
        "\"school_id\":\"%s\","
        "\"timestamp\":%ld,"
        "\"gps\":{"
        "\"latitude\":%.6f,"
        "\"longitude\":%.6f,"
        "\"accuracy\":%.2f,"
        "\"speed\":%.2f,"
        "\"satellites\":%d,"
        "\"hdop\":%.2f"
        "},"
        "\"face_recognition\":{"
        "\"status\":\"%s\","
        "\"confidence\":%.4f,"
        "\"image_size\":%zu"
        "},"
        "\"device_status\":{"
        "\"battery\":%.1f,"
        "\"signal_strength\":%d,"
        "\"status\":\"%s\""
        "}"
        "}",
        DEVICE_ID,
        STUDENT_ID,
        SCHOOL_ID,
        sensor_data->timestamp,
        sensor_data->gps.latitude,
        sensor_data->gps.longitude,
        sensor_data->gps.accuracy,
        sensor_data->gps.speed,
        sensor_data->gps.satellites,
        sensor_data->gps.hdop,
        sensor_data->face_data.status,
        sensor_data->face_data.confidence,
        sensor_data->face_data.data_size,
        sensor_data->battery_level,
        sensor_data->signal_strength,
        sensor_data->device_status
    );

    return written;
}

/**
 * Send data to cloud server via HTTP
 */
int send_data_to_server(SensorData *sensor_data) {
    printf("[NETWORK] Connecting to server %s:%d...\n", API_SERVER, API_PORT);
    
    if (!wifi_connected) {
        printf("[NETWORK] WiFi not connected\n");
        return 0;
    }

    char json_buffer[2000];
    int json_size = create_sensor_data_json(sensor_data, json_buffer, sizeof(json_buffer));
    
    printf("[NETWORK] JSON Payload size: %d bytes\n", json_size);
    printf("[NETWORK] Payload:\n%s\n", json_buffer);

    // In real implementation, use HTTP client library:
    // esp_http_client_config_t config = {
    //     .url = "http://192.168.1.100:5000/api/attendance/submit",
    //     .method = HTTP_METHOD_POST,
    // };
    // esp_http_client_handle_t client = esp_http_client_init(&config);
    // esp_http_client_set_post_field(client, json_buffer, json_size);
    // esp_http_client_perform(client);

    printf("[NETWORK] Data sent successfully\n");
    return 1;
}

// ==================== Offline Storage Functions ====================

/**
 * Store data in offline storage when WiFi is unavailable
 */
int store_offline(SensorData *sensor_data) {
    if (offline_storage.record_count >= OFFLINE_STORAGE_SIZE) {
        printf("[STORAGE] Offline storage full, overwriting old records\n");
        offline_storage.write_index = 0;
    }

    StorageRecord record;
    record.data = *sensor_data;
    strcpy(record.sync_status, "pending");

    offline_storage.records[offline_storage.write_index] = record;
    offline_storage.write_index = (offline_storage.write_index + 1) % OFFLINE_STORAGE_SIZE;
    offline_storage.record_count++;

    printf("[STORAGE] Data stored offline (Records: %d)\n", offline_storage.record_count);
    return 1;
}

/**
 * Sync offline data when WiFi is back
 */
int sync_offline_data() {
    printf("[STORAGE] Syncing offline data (%d records)...\n", offline_storage.record_count);

    for (int i = 0; i < offline_storage.record_count; i++) {
        StorageRecord *record = &offline_storage.records[i];
        
        if (strcmp(record->sync_status, "pending") == 0) {
            if (send_data_to_server(&record->data)) {
                strcpy(record->sync_status, "synced");
                printf("[STORAGE] Record %d synced\n", i);
            } else {
                printf("[STORAGE] Record %d sync failed\n", i);
                return 0;
            }
        }
    }

    offline_storage.record_count = 0;
    printf("[STORAGE] Offline sync complete\n");
    return 1;
}

// ==================== Main Sensor Loop ====================

/**
 * Collect sensor data and send to server
 */
int collect_and_send_sensor_data() {
    printf("\n========== Sensor Data Collection Cycle ==========\n");
    printf("[MAIN] Collecting sensor data...\n");

    SensorData sensor_data;
    sensor_data.timestamp = time(NULL);

    // Read GPS data
    printf("[MAIN] Reading GPS data...\n");
    if (!gps_read_data(&sensor_data.gps)) {
        printf("[MAIN] GPS read failed\n");
        return 0;
    }

    // Process face recognition
    printf("[MAIN] Processing face recognition...\n");
    if (!face_recognition_process(&sensor_data.face_data)) {
        printf("[MAIN] Face recognition failed\n");
        return 0;
    }

    // Read power status
    sensor_data.battery_level = read_battery_level();
    sensor_data.signal_strength = read_signal_strength();
    strcpy(sensor_data.device_status, "active");

    // Try to send data
    printf("[MAIN] Attempting to send data...\n");
    if (wifi_connected) {
        if (!send_data_to_server(&sensor_data)) {
            printf("[MAIN] Send failed, storing offline\n");
            store_offline(&sensor_data);
        }
    } else {
        printf("[MAIN] WiFi not connected, storing offline\n");
        store_offline(&sensor_data);
    }

    printf("================================================\n");
    return 1;
}

// ==================== WiFi Connection Functions ====================

/**
 * WiFi event handler
 */
void wifi_event_handler(int event_type) {
    if (event_type == 1) {  // WIFI_EVENT_STA_CONNECTED
        printf("[WIFI] Connected to WiFi network\n");
        wifi_connected = 1;
        
        // Sync offline data when connection is restored
        if (offline_storage.record_count > 0) {
            sync_offline_data();
        }
    } else if (event_type == 0) {  // WIFI_EVENT_STA_DISCONNECTED
        printf("[WIFI] Disconnected from WiFi network\n");
        wifi_connected = 0;
    }
}

/**
 * Initialize WiFi connection
 */
int wifi_init(const char *ssid, const char *password) {
    printf("[WIFI] Initializing WiFi...\n");
    printf("[WIFI] SSID: %s\n", ssid);

    // In real implementation:
    // esp_wifi_init(&cfg);
    // esp_wifi_set_mode(WIFI_MODE_STA);
    // esp_wifi_sta_get_ap_info(&ap_info);

    wifi_connected = 1;  // Simulate connection for demo
    printf("[WIFI] WiFi initialized successfully\n");
    
    return 1;
}

// ==================== System Initialization ====================

/**
 * Initialize all hardware modules
 */
int system_init() {
    printf("\n========== System Initialization ==========\n");
    printf("[INIT] Device ID: %s\n", DEVICE_ID);
    printf("[INIT] Student ID: %s\n", STUDENT_ID);
    printf("[INIT] School ID: %s\n", SCHOOL_ID);

    // Initialize offline storage
    offline_storage.record_count = 0;
    offline_storage.write_index = 0;

    // Initialize GPS
    if (!gps_init(0)) {
        printf("[INIT] GPS initialization failed\n");
        return 0;
    }

    // Initialize WiFi
    if (!wifi_init("School_WiFi", "password123")) {
        printf("[INIT] WiFi initialization failed\n");
        return 0;
    }

    printf("[INIT] System initialized successfully\n");
    printf("==========================================\n");
    
    return 1;
}

// ==================== Main Function ====================

int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("  Student Tracking IoT System v1.0\n");
    printf("========================================\n\n");

    // Initialize system
    if (!system_init()) {
        printf("[ERROR] System initialization failed\n");
        return 1;
    }

    // Main loop - collect and send data every UPDATE_INTERVAL seconds
    int cycle_count = 0;
    while (1) {
        cycle_count++;
        printf("\n[MAIN] Cycle %d at %s", cycle_count, asctime(localtime(&(time_t){time(NULL)}))));

        // Collect and send sensor data
        collect_and_send_sensor_data();

        // Sleep for UPDATE_INTERVAL seconds
        printf("[MAIN] Sleeping for %d seconds...\n", UPDATE_INTERVAL);
        // In real implementation: sleep(UPDATE_INTERVAL) or vTaskDelay()

        // For demo, just run a few cycles
        if (cycle_count >= 5) {
            printf("\n[MAIN] Demo complete\n");
            break;
        }

        // Small delay for demo
        for (int i = 0; i < 1000000000; i++) {
            asm("nop");  // Prevent compiler optimization
        }
    }

    printf("\n========== System Shutdown ==========\n");
    printf("[SHUTDOWN] Syncing any remaining data...\n");
    if (offline_storage.record_count > 0) {
        sync_offline_data();
    }
    printf("[SHUTDOWN] System shutdown complete\n");

    return 0;
}
