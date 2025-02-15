#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#define TRIG 5
#define ECHO 18

const char* ssid = "ชื่อ wifi"; // เปลี่ยนชื่อ wifi
const char* password = "รหัสผ่าน"; // กำหนดรหัสผ่าน

WebServer server(80);

// อ่านค่าระยะทางจากเซ็นเซอร์ Ultrasonic
long readUltrasonic() {
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(5); 
    digitalWrite(TRIG, LOW);
    long duration = pulseIn(ECHO, HIGH, 30000); // Timeout 30ms
    if (duration == 0) return 0;  // ถ้าไม่ได้รับสัญญาณ ให้คืนค่า 0
    return // <ให้คำนวนค่าเป็น เซนติเมตร ที่นี่> 
}

// สูตรคำนวณระยะทาง
// ระยะทาง (cm) = (เวลาเดินทางของเสียง * ความเร็วเสียง) / 2
// ---เวลาเดินทางของเสียง → วัดจาก pulseIn(ECHO, HIGH); ที่คืนค่าเป็น ไมโครวินาที (μs) (เก็บค่าในตัวแปร duration แล้ว)
// ---ความเร็วเสียงในอากาศ → 343 m/s (ต้องแปลงหน่วยก่อนจาก มิลิวินาที(m/s) เป็น ไมโครวินาที(μs) )

// ฟังก์ชันแสดงหน้าเว็บหลัก
void handleRoot() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="th">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Ultrasonic Sensor</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; background: #f0f8ff; }
        h1 { color: #333; }
        #distance { font-size: 2em; color: #ff4500; }
    </style>
    <script>
        function updateDistance() {
            fetch('/distance')
            .then(response => response.text())
            .then(data => {
                document.getElementById("distance").innerText = data + " cm";
            });
        }
        setInterval(updateDistance, 200); // อัปเดตทุก 200ms
    </script>
</head>
<body>
    <h1>Ultrasonic Distance Sensor</h1>
    <h2>ระยะทาง: <span id="distance">--</span></h2>
</body>
</html>
    )rawliteral";

    server.send(200, "text/html", html);
}

// ฟังก์ชันส่งค่าระยะทางไปยังเว็บ
void handleDistance() {
    long distance = readUltrasonic();
    server.send(200, "text/plain", String(distance));
}

void setup() {
    Serial.begin(115200);

    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);

    IPAddress local_IP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_IP, gateway, subnet);

    Serial.println("WiFi AP Created!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    server.on("/", handleRoot);
    server.on("/distance", handleDistance);

    server.begin();
}

void loop() {
    server.handleClient();
}