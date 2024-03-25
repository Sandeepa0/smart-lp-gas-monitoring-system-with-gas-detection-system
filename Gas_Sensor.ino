#define BLYNK_TEMPLATE_ID "TMPL6OW56O2Gd"
#define BLYNK_TEMPLATE_NAME "Gas Detector"
#define BLYNK_AUTH_TOKEN "nN4rgz-63acTpQ_Ba8sx-LWdJFJ-XVV1"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

BlynkTimer timer;

char auth[] = "nN4rgz-63acTpQ_Ba8sx-LWdJFJ-XVV1";
char ssid[] = "Sandeepa 4G";
char pass[] = "1234abcd"; //9CF780D3

#define MQ2 34
#define GREEN 12
#define RED 14
#define BUZZER 13

int MQ2_Val = 0;
boolean state = false;
WidgetLED led(V9);
LiquidCrystal_I2C lcd(0x27, 16, 2);

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
}

void loop()
{
  Blynk.run();
  timer.run();
}

void sendUptime()
{
  MQ2_Val = analogRead(MQ2);

  for (int x = 0; x < 10; x++)
  {
    MQ2_Val = MQ2_Val + analogRead(MQ2);
  }
  MQ2_Val = MQ2_Val / 100;

  Blynk.virtualWrite(V0, MQ2_Val);

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("MQ2 Level:");

  lcd.setCursor(11, 1);
  lcd.print(MQ2_Val);

  lcd.setCursor(15, 1);
  lcd.print("PPM");

  if (MQ2_Val > 100)
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
