// Nama: Mahardika Ramadhana
// NIM: 24/538247/PA/22831
// Tugas Magang GMRT Day 3

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ESP32Servo.h>

// Objek sensor & servo
Adafruit_MPU6050 mpu;
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;

// Pinout
const int pinSrv1 = 13;
const int pinSrv2 = 12;
const int pinSrv3 = 26;
const int pinSrv4 = 27;
const int pinSrv5 = 14;
const int pinPIR = 23;

// Posisi servo (0-180)
const int posAwal = 90; // Posisi 0 derajat (tegak lurus)
const int posKaget = 120; // Posisi servo pas kaget (bebas)

// Variabel global buat timer servo 5 (yaw)
static unsigned long timerYaw = 0;
static bool cekYaw = false; // Flag buat ngecek servo 5 abis muter

void setup() {
  Serial.begin(115200);

  // Mulai MPU6050
  if (!mpu.begin()) {
    Serial.println("MPU6050 ga ketemu");
    while (1) delay(10);
  }
  
  // Atur settingan MPU
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG); // Disesuaikan dari error log
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Pin PIR jadi INPUT
  pinMode(pinPIR, INPUT);

  // Attach semua servo ke pin-nya
  servo1.attach(pinSrv1);
  servo2.attach(pinSrv2);
  servo3.attach(pinSrv3);
  servo4.attach(pinSrv4);
  servo5.attach(pinSrv5);

  // Set semua ke posisi awal (90 derajat)
  servo1.write(posAwal);
  servo2.write(posAwal);
  servo3.write(posAwal);
  servo4.write(posAwal);
  servo5.write(posAwal);
  
  delay(100);
}

void loop() {

  // 1. PRIORITAS: Cek Sensor PIR dulu (Requirement 4)
  if (digitalRead(pinPIR) == HIGH) {
    // Kalo ada gerakan, semua servo "kaget"
    
    // Bergerak serentak ke posisi kaget
    servo1.write(posKaget);
    servo2.write(posKaget);
    servo3.write(posKaget);
    servo4.write(posKaget);
    servo5.write(posKaget);

    // Tahan sebentar
    delay(500); 

    // Kembali serentak ke posisi semula
    servo1.write(posAwal);
    servo2.write(posAwal);
    servo3.write(posAwal);
    servo4.write(posAwal);
    servo5.write(posAwal);

    // Tahan di posisi awal biar ga kaget terus
    delay(1000); 
    
  } else {
    // 2. JIKA AMAN: Baru jalankan logika MPU6050 (Requirement 1, 2, 3)
    
    // Ambil data sensor
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Hitung sudut Roll & Pitch pakai Akselerometer
    // (Rumus 'atan2' dari internet)
    float sudutRoll = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / M_PI;
    float sudutPitch = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180.0 / M_PI;

    // Ambil data Gyro Z (Yaw)
    float gyroYaw = g.gyro.z; // (ini kecepatan rotasi)

    
    // Servo 1 & 2 (Roll) - Melawan Arah
    int posServo12 = posAwal - sudutRoll;
    servo1.write(constrain(posServo12, 0, 180));
    servo2.write(constrain(posServo12, 0, 180));

    // Servo 3 & 4 (Pitch) - Searah
    int posServo34 = posAwal + sudutPitch;
    servo3.write(constrain(posServo34, 0, 180));
    servo4.write(constrain(posServo34, 0, 180));
    
    // Servo 5 (Yaw) - Ikut, tunggu 1 dtk, trus balik
    
    // Cek dulu, lagi muter apa diem?
    // (Harus di-tuning thresholdnya)
    float thresholdYaw = 0.3; 

    if (abs(gyroYaw) > thresholdYaw) {
      // Kalo lagi muter...
      // Map kecepatan gyro ke posisi servo
      // (Angka 5 ini jg tuning)
      int posServo5 = map(gyroYaw, -5, 5, 0, 180);
      servo5.write(constrain(posServo5, 0, 180));

      // Set timer & flag buat nanti
      cekYaw = true;        
      timerYaw = millis();  
      
    } else {
      // Kalo lagi diem...
      
      // Cek, abis gerak ga?
      if (cekYaw == true) {
        // Kalo iya, cek udah 1 detik?
        if (millis() - timerYaw > 1000) {
          // Balikin ke posisi awal
          servo5.write(posAwal);
          cekYaw = false; // Udah selesai
        }
        // Kalo blm 1 detik, ya diem aja (ga ngapa-ngapain)
      }
    }
  }

  // Kasih jeda dikit biar ga overload
  delay(20); 
}