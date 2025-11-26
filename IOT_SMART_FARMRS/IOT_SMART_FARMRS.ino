#include <Arduino.h>              // أساس
#include <Wire.h>                 // I2C
#include <LiquidCrystal_I2C.h>    // شاشة I2C 20x4
#include <DHT.h>                  // حساس DHT22

// ---------------------- Pin configuration (مطابقة لتوصيلاتك) ----------------------
#define PIN_DHT          4    // D4: DHT22
#define PIN_TANK_LED     5    // D5: LED يمثل فتح/غلق الماء (الخزان)
#define PIN_ULTRA_TRIG   3    // D3: TRIG للموجات فوق الصوتية
#define PIN_ULTRA_ECHO   2    // D2: ECHO للموجات فوق الصوتية
#define PIN_SOIL_A       10   // A0: حساس رطوبة التربة (ADC)
#define PIN_I2C_SDA      1    // D1: SDA للشاشة
#define PIN_I2C_SCL      0    // D0: SCL للشاشة
#define PIN_BUZZER       9    // D9: الجرس
#define PIN_LED1         6    // D6: LED1 (خزان فاضي)
#define PIN_LED2         7    // D7: LED2 (تربة < 30%)
#define PIN_LED3         8    // D8: LED3 (كل شيء تمام)

// ---------------------- Sensors & LCD --------------------------
#define DHTTYPE          DHT22
DHT dht(PIN_DHT, DHTTYPE);

// أغلب شاشات I2C عنوانها 0x27، لو ما اشتغلت جرّب 0x3F
#define LCD_ADDR         0x27
LiquidCrystal_I2C lcd(LCD_ADDR, 20, 4);

// ---------------------- Calibration & thresholds ----------------------------
// هندسة الخزان (بالسونار): عدّل حسب المسافات الفعلية عندك
float TANK_HEIGHT_CM     = 30.0; // المسافة من الحساس لسطح الماء عندما "فاضي" (كبيرة)
float FULL_LEVEL_CM      = 5.0;  // المسافة عندما "ممتلئ" (قريبة من الحساس)

// معايرة حساس التربة (ESP32 ADC: مدى 0..4095)
int SOIL_DRY_RAW         = 3500; // قراءة وهو جاف
int SOIL_WET_RAW         = 1200; // قراءة وهو مبلل جدًا

// عتبات التحكم
int SOIL_ON_THRESHOLD    = 30;   // ≤30%: افتح الماء (LED الخزان ON)
int SOIL_OFF_THRESHOLD   = 50;   // ≥50%: اقفل الماء (LED الخزان OFF)
int SOIL_BUZZER_ALARM    = 20;   // <20%: الجرس إنذار

int TANK_EMPTY_PERCENT   = 10;   // ≤10%: اعتبر الخزان فاضي (لإنذار LED1 والجرس)

// توقيت وميض الجرس
unsigned long BUZZER_INTERVAL_MS = 2000;

// ---------------------- State variables ----------------------------
bool valveOn = false;            // حالة "فتح الماء" (يمثلها LED الخزان)
bool buzzerAlarm = false;        // هل الجرس في حالة إنذار
unsigned long lastBuzzerToggle = 0;
bool buzzerState = false;

// ---------------------- Ultrasonic reading ---------------------------
float readUltrasonicDistanceCM() {
  // نبضة TRIG
  digitalWrite(PIN_ULTRA_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_ULTRA_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_ULTRA_TRIG, LOW);

  // زمن الـ ECHO (مع مهلة 30ms)
  unsigned long duration = pulseIn(PIN_ULTRA_ECHO, HIGH, 30000UL);
  if (duration == 0) {
    return NAN; // قراءة غير صالحة
  }
  // تحويل الزمن لمسافة: تقريبًا 58 ميكروثانية لكل سم (ذهاب-عودة)
  float distanceCM = duration / 58.0;
  return distanceCM;
}

// تحويل المسافة لنسبة امتلاء الخزان (0..100%)
int mapTankPercent(float distanceCM) {
  if (isnan(distanceCM)) return -1; // -1 يعني قراءة غير متاحة

  // حصر القراءة بين ممتلئ وفاضي
  float d = constrain(distanceCM, FULL_LEVEL_CM, TANK_HEIGHT_CM);

  // عندما d = FULL_LEVEL_CM → 100%
  // عندما d = TANK_HEIGHT_CM → 0%
  float percent = 100.0 * (TANK_HEIGHT_CM - d) / (TANK_HEIGHT_CM - FULL_LEVEL_CM);
  return (int)round(constrain(percent, 0.0, 100.0));
}

// قراءة رطوبة التربة كنسبة 0..100%
int readSoilPercent() {
  int raw = analogRead(PIN_SOIL_A);              // مدى 0..4095 على ESP32
  int pct = map(raw, SOIL_DRY_RAW, SOIL_WET_RAW, 0, 100); // تحويل إلى %
  pct = constrain(pct, 0, 100);
  return pct;
}

// تحكم فتح/غلق الماء بناءً على رطوبة التربة
void updateValveBySoil(int soilPct) {
  if (!valveOn && soilPct <= SOIL_ON_THRESHOLD) {
    valveOn = true;   // افتح الماء (LED الخزان يشتغل)
  } else if (valveOn && soilPct >= SOIL_OFF_THRESHOLD) {
    valveOn = false;  // اقفل الماء (LED الخزان يطفى)
  }
  digitalWrite(PIN_TANK_LED, valveOn ? HIGH : LOW);
}

// منطق عمل الجرس (وميض كل 2 ثانية وقت الإنذار)
void updateBuzzer(int tankPct, int soilPct) {
  // إنذار لو الخزان فاضي أو التربة أقل من 20%
  buzzerAlarm = (tankPct >= 0 && tankPct <= TANK_EMPTY_PERCENT) || (soilPct < SOIL_BUZZER_ALARM);

  if (buzzerAlarm) {
    unsigned long now = millis();
    if (now - lastBuzzerToggle >= BUZZER_INTERVAL_MS) {
      buzzerState = !buzzerState;                          // تبديل الحالة
      digitalWrite(PIN_BUZZER, buzzerState ? HIGH : LOW);  // وميض
      lastBuzzerToggle = now;
    }
  } else {
    buzzerState = false;
    digitalWrite(PIN_BUZZER, LOW); // إيقاف الجرس خارج الإنذار
  }
}

// تحديث لمبات الحالة الثلاثة
void updateStatusLEDs(int tankPct, int soilPct) {
  // LED1: الخزان فاضي
  bool tankEmpty = (tankPct >= 0 && tankPct <= TANK_EMPTY_PERCENT);
  digitalWrite(PIN_LED1, tankEmpty ? HIGH : LOW);

  // LED2: التربة أقل من 30%
  bool soilLow = (soilPct < SOIL_ON_THRESHOLD);
  digitalWrite(PIN_LED2, soilLow ? HIGH : LOW);

  // LED3: كل شيء تمام (التربة ≥50% والخزان ليس فاضي)
  bool allGood = (soilPct >= SOIL_OFF_THRESHOLD) && !tankEmpty;
  digitalWrite(PIN_LED3, allGood ? HIGH : LOW);
}

// عرض البيانات على شاشة LCD
void updateLCD(float tempC, float humPct, int tankPct, int soilPct) {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  if (isnan(tempC)) lcd.print("--");
  else { lcd.print(tempC, 1); lcd.print(" C"); }

  lcd.setCursor(0, 1);
  lcd.print("Hum : ");
  if (isnan(humPct)) lcd.print("--");
  else { lcd.print(humPct, 1); lcd.print(" %"); }

  lcd.setCursor(0, 2);
  lcd.print("Tank: ");
  if (tankPct < 0) lcd.print("--");
  else { lcd.print(tankPct); lcd.print(" %"); }

  lcd.setCursor(0, 3);
  lcd.print("Soil: ");
  lcd.print(soilPct);
  lcd.print(" %");

  // مؤشرات حالة سريعة على يمين السطور
  if (tankPct >= 0 && tankPct <= TANK_EMPTY_PERCENT) {
    lcd.setCursor(12, 2); lcd.print("EMPTY");
  }
  if (soilPct <= SOIL_ON_THRESHOLD) {
    lcd.setCursor(12, 3); lcd.print("WATER");
  } else if (soilPct >= SOIL_OFF_THRESHOLD) {
    lcd.setCursor(12, 3); lcd.print("OK   ");
  }
}

void setup() {
  // تهيئة I2C على الأرجل المخصصة
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  // تهيئة الشاشة وDHT
  lcd.init();
  lcd.backlight();
  dht.begin();

  // اتجاهات الأرجل
  pinMode(PIN_TANK_LED, OUTPUT);
  pinMode(PIN_ULTRA_TRIG, OUTPUT);
  pinMode(PIN_ULTRA_ECHO, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_LED3, OUTPUT);

  // حالات ابتدائية
  digitalWrite(PIN_TANK_LED, LOW);
  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_LED1, LOW);
  digitalWrite(PIN_LED2, LOW);
  digitalWrite(PIN_LED3, LOW);

  // شاشة ترحيب
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Smart Farmers");
  lcd.setCursor(0, 1); lcd.print("Init sensors...");
  delay(1500);
}

void loop() {
  // قراءات DHT22
  float hum = dht.readHumidity();
  float temp = dht.readTemperature(); // بالسيليزي

  // قراءة السونار → نسبة الخزان
  float distCM = readUltrasonicDistanceCM();
  int tankPct = mapTankPercent(distCM);

  // قراءة رطوبة التربة
  int soilPct = readSoilPercent();

  // تحكم فتح/غلق الماء
  updateValveBySoil(soilPct);

  // منطق الجرس
  updateBuzzer(tankPct, soilPct);

  // لمبات الحالة
  updateStatusLEDs(tankPct, soilPct);

  // تحديث الشاشة
  updateLCD(temp, hum, tankPct, soilPct);

  // معدل التحديث
  delay(500);
}