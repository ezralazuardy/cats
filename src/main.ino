// LIBRARY USAGE
#include <Arduino.h>
#include <Ethernet.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MLX90614.h>
#include <ArduinoHttpClient.h>

// PIN CONFIGURATION
#define LED_RED_PIN 2
#define LED_GREEN_PIN 3
#define IR_PIN 9
#define BUZZER_PIN 8

// MAIN CONFIGURATION
#define SERIAL_BAUD_RATE 9600    // default serial baud rate
#define SCAN_DELAY 1000          // scan delay interval (1 second)
#define SCAN_NETWORK_DELAY 30000 // scanning network delay interval (30 second)
#define ACK_REQUEST_DELAY 60000  // ack request delay (60 second)
#define COOLDOWN_COUNT 3         // cooldown count (after buzz, 3 second interval, 1 second delay per count)

// LCD CONFIGURATION
#define LCD_COLS 16 // LCD pixel width
#define LCD_ROWS 2  // LCD pixel length

// BUZZ CONFIGURATION
#define BUZZ_INTERVAL 800  // buzz interval (0.8 second)
#define BUZZ_FREQUENCY 950 // buzz sound frequency (0.95 second)
#define BUZZ_WARNING 4     // warning buzz amount (4 beep)
#define BUZZ_INVALID 2     // invalid buzz amount (2 beep)
#define BUZZ_VALID 1       // valid buzz amount (1 beep)

// GLOBAL VARIABLE DECLARATION
uint32_t lastScanMillis;
uint32_t lastScanNetworkMillis;
uint32_t lastAckRequestMillis;
uint8_t currentCooldownCounter = COOLDOWN_COUNT;
uint8_t currentAckCounter = 0;
boolean onCooldown = false;

// LIBRARY GLOBAL VARIABLE DECLARATION
#define SERVER_CONNECT_TIMEOUT 5000
#define SERVER_PORT 5000
#define SERVER_ACK_LIMIT 200
const IPAddress serverIp(192, 168, 10, 2);
EthernetClient client;
HttpClient http = HttpClient(client, serverIp, SERVER_PORT);
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, LCD_COLS, LCD_ROWS);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
byte mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// LIBRARY GLOBAL FUNCTION DECLARATION
boolean httpPostRequest(const __FlashStringHelper *path, String body)
{
  Serial.print(F("sending HTTP POST request to "));
  Serial.println(path);
  http.beginRequest();
  http.post(path);
  http.sendHeader(F("Host"), "192.168.10.2:5000");
  http.sendHeader(F("Accept"), F("application/json"));
  http.sendHeader(F("Content-Type"), F("application/json"));
  http.sendHeader(F("Content-Length"), body.length());
  http.sendHeader(F("Connection"), F("close"));
  http.sendHeader(F("User-Agent"), F("Arduino/1.0"));
  http.endRequest();
  http.println(body);
  http.flush();
  boolean response = getServerResponse();
  return response;
}

boolean getServerResponse()
{
  if (http.connected() || http.available())
  {
    Serial.print(F("response code: "));
    Serial.println(http.responseStatusCode());

    /**
     * uncomment code below to view the response body
     * 
     * const String responseBody = http.responseBody();
     * Serial.println(F("response body: "));
     * Serial.println(responseBody);
     */

    return http.responseStatusCode() == 200;
  }
  else
  {
    Serial.println(F("connection failure"));
  }
  return false;
}

void printSingleLCD(const __FlashStringHelper *message1)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message1);
}

void printDoubleLCD(const __FlashStringHelper *message1, const __FlashStringHelper *message2)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message1);
  lcd.setCursor(0, 1);
  lcd.print(message2);
}

void printDoubleLCD(const __FlashStringHelper *message1, const char *message2)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message1);
  lcd.setCursor(0, 1);
  lcd.print(message2);
}

// INITIALIZATION FUNCTION
void initializeLCD()
{
  Serial.println(F("initializing LCD"));
  lcd.init();
  lcd.backlight();
  printSingleLCD(F("Sedang memuat..."));
}

void initializeLED()
{
  Serial.println(F("initializing LED"));
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);
}

void initializeIR()
{
  Serial.println(F("initializing IR"));
  pinMode(IR_PIN, INPUT);
}

void initializeMLX()
{
  Serial.println(F("initializing MLX"));
  mlx.begin();
}

void initializeBuzzer()
{
  Serial.println(F("initializing Buzzer"));
  pinMode(BUZZER_PIN, OUTPUT);
}

void initializeEthernet()
{
  Serial.println(F("initializing Ethernet"));
  http.setTimeout(SERVER_CONNECT_TIMEOUT);
  client.setTimeout(SERVER_CONNECT_TIMEOUT);
  Ethernet.begin(mac);
  if (Ethernet.hardwareStatus() == EthernetNoHardware)
  {
    Serial.println(F("ethernet shield is not detected, please install it and reset device"));
    noNetworkDeviceException();
  }
  else
    Ethernet.begin(mac, IPAddress(192, 168, 10, 3), IPAddress(8, 8, 8, 8), IPAddress(192, 168, 10, 1), IPAddress(255, 255, 255, 0));
}

// MAIN FUNCTION
void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println(F("booting..."));
  initializeLCD();
  initializeLED();
  initializeIR();
  initializeMLX();
  initializeBuzzer();
  initializeEthernet();
  Serial.println(F("booting done!"));
}

void loop()
{
  uint32_t currentMillis = millis();
  if (currentMillis - lastScanMillis > SCAN_DELAY)
  {
    lastScanMillis = currentMillis;
    scanNetwork(currentMillis);
    if (onCooldown)
    {
      if (currentCooldownCounter <= 0)
      {
        resetCooldown();
      }
      else
      {
        showCooldown();
      }
    }
    else if (!digitalRead(IR_PIN))
    {
      performDetection();
    }
    else
    {
      requestAck(currentMillis);
    }
  }
}

void scanNetwork(const uint32_t currentMillis)
{
  if (currentMillis - lastScanNetworkMillis > SCAN_NETWORK_DELAY)
  {
    lastScanNetworkMillis = currentMillis;
    Serial.println(F("scanning network"));
    client.connect(serverIp, SERVER_PORT);
    if (client.connected())
    {
      networkDetected();
    }
    else
    {
      noNetworkException();
    }
    client.stop();
  }
}

void networkDetected()
{
  Serial.print(F("connected to "));
  Serial.print(serverIp);
  Serial.print(F(":"));
  Serial.print(SERVER_PORT);
  Serial.print(F(" with ip "));
  Serial.println(Ethernet.localIP());
  printDoubleLCD(F("Menunggu"), F("pemindaian..."));
  digitalWrite(LED_RED_PIN, LOW);
  resetAck();
}

void noNetworkException()
{
  Serial.print(F("can't connect to "));
  Serial.print(serverIp);
  Serial.print(F(":"));
  Serial.print(SERVER_PORT);
  Serial.print(F(" with ip "));
  Serial.println(Ethernet.localIP());
  printDoubleLCD(F("Jaringan tidak"), F("tersedia!"));
  digitalWrite(LED_RED_PIN, HIGH);
}

void noNetworkDeviceException()
{
  printDoubleLCD(F("Jaringan"), F("tidak tersedia!"));
  digitalWrite(LED_RED_PIN, HIGH);
  while (true)
    delay(SCAN_DELAY);
}

void performDetection()
{
  Serial.println(F("object detected"));
  printSingleLCD(F("Memeriksa..."));
  digitalWrite(LED_RED_PIN, HIGH);
  tone(BUZZER_PIN, BUZZ_FREQUENCY);
  const float temp1 = mlx.readObjectTempC();
  delay(BUZZ_INTERVAL);
  digitalWrite(LED_RED_PIN, LOW);
  noTone(BUZZER_PIN);
  delay(200);
  if (digitalRead(IR_PIN))
  {
    Serial.println(F("detection failure!"));
    printDoubleLCD(F("Pemeriksaan"), F("gagal!"));
    delay(200);
    for (uint8_t i = 0; i < BUZZ_INVALID; i++)
    {
      tone(BUZZER_PIN, BUZZ_FREQUENCY);
      digitalWrite(LED_RED_PIN, HIGH);
      delay(200);
      noTone(BUZZER_PIN);
      digitalWrite(LED_RED_PIN, LOW);
      delay(200);
    }
  }
  else
  {
    const float temp2 = mlx.readObjectTempC();
    const float finalTemp = (temp1 + temp2) / 2;
    Serial.println(F("detection success"));
    Serial.print(F("object temperature: "));
    Serial.print(finalTemp);
    Serial.println(F(" C"));
    char degree[16];
    dtostrf(finalTemp, 4, 2, degree);
    strcat(degree, " Celcius");
    printDoubleLCD(F("Suhu anda "), degree);
    if (finalTemp < 38.0)
    {
      Serial.println(F("temperature is normal"));
      for (uint8_t i = 0; i < BUZZ_VALID; i++)
      {
        tone(BUZZER_PIN, BUZZ_FREQUENCY);
        digitalWrite(LED_GREEN_PIN, HIGH);
        delay(200);
        noTone(BUZZER_PIN);
        digitalWrite(LED_GREEN_PIN, LOW);
        delay(200);
      }
      delay(1800);
      printSingleLCD(F("Suhu anda normal"));
    }
    else
    {
      Serial.println(F("temperature is too high!"));
      for (uint8_t i = 0; i < BUZZ_WARNING; i++)
      {
        tone(BUZZER_PIN, BUZZ_FREQUENCY);
        digitalWrite(LED_RED_PIN, HIGH);
        delay(200);
        noTone(BUZZER_PIN);
        digitalWrite(LED_RED_PIN, LOW);
        delay(200);
      }
      delay(1600);
      printDoubleLCD(F("Suhu anda"), F("terlalu tinggi!"));
    }
    saveDetection(finalTemp);
    delay(500);
  }
  startCooldown();
}

void startCooldown()
{
  onCooldown = true;
}

void showCooldown()
{
  char countdown[8];
  sprintf(countdown, "%d", currentCooldownCounter--);
  strcat(countdown, " detik");
  char message[15] = "dalam ";
  strcat(message, countdown);
  printDoubleLCD(F("Memulai lagi"), message);
}

void resetCooldown()
{
  onCooldown = false;
  currentCooldownCounter = COOLDOWN_COUNT;
  printDoubleLCD(F("Menunggu"), F("pemindaian..."));
}

void resetAck()
{
  if (currentAckCounter == 0 || currentAckCounter >= SERVER_ACK_LIMIT)
  {
    httpPostRequest(F("http://192.168.10.2:5000/api/ack/reset"), String("{\"key\":\"$2y$10$er7IYh/BIPBOuUkGbuTcMu6ffQAYte53IvH05syDhcECawV8gICoC\"}"));
    lastAckRequestMillis = millis();
    currentAckCounter = 0;
  }
}

void requestAck(const uint32_t currentMillis)
{
  if (currentMillis - lastAckRequestMillis > ACK_REQUEST_DELAY)
  {
    httpPostRequest(F("http://192.168.10.2:5000/api/ack"), String("{\"key\":\"$2y$10$er7IYh/BIPBOuUkGbuTcMu6ffQAYte53IvH05syDhcECawV8gICoC\"}"));
    lastAckRequestMillis = currentMillis;
    currentAckCounter++;
  }
}

void saveDetection(const float temperature)
{
  String body = String("{\"key\":\"$2y$10$er7IYh/BIPBOuUkGbuTcMu6ffQAYte53IvH05syDhcECawV8gICoC\",\"temperature\":") + String(temperature) + String("}");
  httpPostRequest(F("http://192.168.10.2:5000/api/detection/add"), body);
}