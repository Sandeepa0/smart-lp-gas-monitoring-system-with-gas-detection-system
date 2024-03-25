#define BLYNK_TEMPLATE_ID "TMPL6OW56O2Gd"
#define BLYNK_TEMPLATE_NAME "Gas Detector"
#define BLYNK_AUTH_TOKEN "nN4rgz-63acTpQ_Ba8sx-LWdJFJ-XVV1"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <Preferences.h>

BlynkTimer timer;

char auth[] = "nN4rgz-63acTpQ_Ba8sx-LWdJFJ-XVV1";
char ssid[] = "s"; //Dialog 4G 465
char pass[] = "12345678"; //9CF780D3

#define MQ2 34
#define GREEN 12
#define RED 14
#define BUZZER 13
#define LOADCELL_DOUT_PIN 5
#define LOADCELL_SCK_PIN  18
#define BTN_MODE_PIN 15
#define weight_of_object_for_calibration 181

int MQ2_Val = 0;
boolean state = false;
WidgetLED led(V9);
LiquidCrystal_I2C lcd(0x27, 16, 2);

int buttonPressCount = 0;
HX711 LOADCELL_HX711;
Preferences preferences;

int weight_In_g;
float weight_In_kg;
int last_weight_In_kg;
float weight_In_oz;
float LOAD_CALIBRATION_FACTOR;


void scale_Tare();
void loadcell_Calibration();
void sendUptime();

void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  pinMode(MQ2, INPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  timer.setInterval(1000L, sendUptime);

  lcd.init();
  lcd.backlight();

  LOADCELL_HX711.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale_Tare();  // Tare the scale initially

  preferences.begin("CF", false);
  LOAD_CALIBRATION_FACTOR = preferences.getFloat("CFVal", 0);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibration Val:");
  lcd.setCursor(0, 1);
  lcd.print(LOAD_CALIBRATION_FACTOR);
  delay(1500);

  LOADCELL_HX711.set_scale(LOAD_CALIBRATION_FACTOR);
  delay(100);
  LOADCELL_HX711.tare();
  delay(100);
}

void loop()
{
  Blynk.run();
  timer.run();

  if (LOADCELL_HX711.wait_ready_timeout(100))
{
  weight_In_g = LOADCELL_HX711.get_units(2);

  if (last_weight_In_kg != weight_In_g)
  {
    lcd.setCursor(10, 0);
    lcd.print(" ");
    lcd.setCursor(10, 0);
    lcd.print(" ");
  }
  lcd.setCursor(0, 0);
  lcd.print("Gas Level:");
  String check_isNegative;

  byte x_pos_g;
  check_isNegative = String(weight_In_g);

  if (abs(weight_In_g) >= 0 && abs(weight_In_g) <= 9)
  {
    x_pos_g = 10;
  }
  else if (abs(weight_In_g) >= 10 && abs(weight_In_g) <= 99)
  {
    x_pos_g = 10;
  }
  else if (abs(weight_In_g) >= 100 && abs(weight_In_g) <= 999)
  {
    x_pos_g = 10;
  }
  else if (abs(weight_In_g) >= 1000 && abs(weight_In_g) <= 5000)
  {
    x_pos_g = 10;
  }

  if (check_isNegative[0] == '-')
    x_pos_g = x_pos_g - 1;

  lcd.setCursor(x_pos_g, 0);

  // Convert grams to kilograms
  float weight_In_kg = weight_In_g / 1000.0;
  float weight_in_pre = weight_In_kg;
  float weight_in_pre = ((weight_In_kg - 8.5)/5) * 100;
  lcd.print(weight_in_pre, 2); // Display weight in kg with 3 decimal places

  lcd.setCursor(16, 0);
  lcd.print("%");
  Blynk.logEvent("level");
  Blynk.virtualWrite(V5, weight_in_pre);

  if (weight_in_pre < 20){
  Blynk.logEvent("gas_level");
  }
  check_isNegative = "   ";

  }
  else
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error...");
    lcd.setCursor(0, 1);
    lcd.print("HX711 not found.");
    while (1)
    {
      delay(1000);
    }
  }

  if (digitalRead(BTN_MODE_PIN) == LOW)
  {
    byte btn_mode_cnt = 0;

    while (digitalRead(BTN_MODE_PIN) == LOW)
    {
      btn_mode_cnt++;
      if (btn_mode_cnt > 3)
      {
        loadcell_Calibration();
        break;
      }
      delay(1000);
    }

    while (digitalRead(BTN_MODE_PIN) == HIGH)
    {
      if (btn_mode_cnt < 3)
        scale_Tare();
      break;
    }
  }
}

void scale_Tare()
{
  lcd.clear();
  delay(500);
  lcd.setCursor(0, 0);
  lcd.print("Mode: Tare");
  delay(1000);

  if (LOADCELL_HX711.is_ready())
  {
    LOADCELL_HX711.tare();
    delay(500);
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error...");
    lcd.setCursor(0, 1);
    lcd.print("HX711 not found.");
    while (1)
    {
      delay(1000);
    }
  }
}

void loadcell_Calibration()
{
  lcd.clear();
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Mode: Calbiration");
  delay(1500);

  for (byte i = 0; i < 2; i++)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Do not place");
    lcd.setCursor(0, 1);
    lcd.print("any object");
    lcd.setCursor(0, 2);
    lcd.print("on the scale...");
    delay(1500);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Please wait...");
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Start Calibration...");
  delay(1000);

  if (LOADCELL_HX711.is_ready())
  {
    LOADCELL_HX711.set_scale();
    delay(100);
    LOADCELL_HX711.tare();
    delay(100);

    for (byte i = 0; i < 2; i++)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Now place an object");
      lcd.setCursor(0, 1);
      lcd.print("on the scale");
      delay(1500);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Weighing ");
      delay(1500);
    }

    lcd.clear();
    delay(1000);

    long sensor_Reading_Results = 0;

    for (byte i = 0; i < 5; i++)
    {
      sensor_Reading_Results = LOADCELL_HX711.get_units(10);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sensor Reading ");
      lcd.print(i + 1);
      lcd.setCursor(0, 1);
      lcd.print(sensor_Reading_Results);
      delay(1000);
    }

    lcd.clear();
    delay(1000);

    float CALIBRATION_FACTOR = sensor_Reading_Results / weight_of_object_for_calibration;

    preferences.putFloat("CFVal", CALIBRATION_FACTOR);
    delay(100);

    LOAD_CALIBRATION_FACTOR = preferences.getFloat("CFVal", 0);
    delay(100);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Calibration Val:");
    lcd.setCursor(0, 1);
    lcd.print(LOAD_CALIBRATION_FACTOR);
    delay(1500);

    LOADCELL_HX711.set_scale(LOAD_CALIBRATION_FACTOR);
    delay(100);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Calibration Complete");
    delay(1500);

    lcd.clear();
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error...");
    lcd.setCursor(0, 1);
    lcd.print("HX711 not found...");
    while (1)
    {
      delay(1000);
    }
  }
}

void sendUptime()
{
  MQ2_Val = analogRead(MQ2);

  for (int x = 0; x < 10; x++)
  {
    MQ2_Val = MQ2_Val + analogRead(MQ2);
  }
  MQ2_Val = MQ2_Val/50;
  

  Blynk.virtualWrite(V0, MQ2_Val);

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("PPM Level:");

  lcd.setCursor(11, 1);
  lcd.print(MQ2_Val);

  lcd.setCursor(15, 1);
  lcd.print("PPM");

  if (MQ2_Val > 150)
  {
    Blynk.logEvent("gas_leakage");
    Serial.println("Gas Detected!");
    digitalWrite(GREEN, LOW);
    digitalWrite(RED, HIGH);
    digitalWrite(BUZZER, HIGH);
    led.on();

    lcd.setCursor(0, 2);
    lcd.print("Gas Leak Detected!");
  }
  else
  {
    digitalWrite(GREEN, HIGH);
    digitalWrite(RED, LOW);
    digitalWrite(BUZZER, LOW);
    led.off();

    lcd.setCursor(0, 2);
    lcd.print("No Gas Leak!");
  }

  Serial.print("MQ2 Level: ");
  Serial.println(MQ2_Val);
}