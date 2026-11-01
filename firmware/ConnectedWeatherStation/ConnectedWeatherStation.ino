// ==================================================== Date and time :
#include <NTPClient.h>
#include <WiFiUdp.h>
const long utcOffsetInSeconds = -14400; // 60 * 60 * (-4)

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient now(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// ==================================================== Button :
#include <PinButton.h>
PinButton button(0); // D3

// ==================================================== Buzzer :
#define buzzer 14 // D5

bool buzz = false;
bool isBuzzing = false;

unsigned long currentMillis;
unsigned long previousMillis = 0;
const long interval = 300;

// ==================================================== Lights :
#define d0 16
#define d6 12
#define d7 13

// ==================================================== Screen :
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ==================================================== Sensor :
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

bool sens = false;

// ==================================================== Server :
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "MyWifi.h"
#include "PageSettings.h"

#define PORT 80

const char* ssid = STASSID;
const char* password = STAPSK;

String page;
String content;
bool showingIP = false;

ESP8266WebServer server(PORT);

// ==================================================== Game settings :
#define MAX 7

bool game = false;
bool isOver = false; // is game over
bool gameZero = true;

int guess = 0;
int randomNumber;
int previousRandomNumber = -1;
int score;

// ==================================================== Setup :
void setup() {
  Serial.begin(74880);

  // ------------------------------------------------ Lights :
  pinMode(d0, OUTPUT);
  pinMode(d6, OUTPUT);
  pinMode(d7, OUTPUT);

  // ------------------------------------------------ Buzzer :
  pinMode(buzzer, OUTPUT);

  // ------------------------------------------------ Screen :
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.clearDisplay();
  display.display();
  delay(1000);
  
  
  // ------------------------------------------------ Sensor :
  unsigned status;
  status = bme.begin(0x76);

  // ------------------------------------------------ Server :
  setupConnection();

  // ------------------------------------------------ Page : HOME
  server.on("/", []() {
    content = "";

    content += "<h2>TP 1 Objets Conectes</h2><p>Erica Santini de Lima</p><br>";
    content += "<p><b>Commandes Ecran Principal :</b></p>";
    content += "<p>";
    content += "Actualiser les informations du senseur : single click<br>";
    content += "Voir les informations du serveur : double click<br>";
    content += "Retourner aux informations du senseur : single click ou double click";
    content += "</p>";
    content += "<p><b>Commandes Jeux :</b></p>";
    content += "<p>";
    content += "Commencer le jeu : long click<br>";
    content += "Confirmer ta reponse : long click<br>";
    content += "Changer ta reponse : single click<br>";
    content += "Quitter le jeu : long click";
    content += "</p>";
    
    page = htmlTop + content + htmlBottom;
    server.send(200, "text/html", page);});

  // ------------------------------------------------ Page : SCORE
  server.on("/score", []() {
    content = "";

    if(gameZero == true){
      content += "<h2>Score inexistant</h2><p>Tu n'as pas encore commence un match</p>";
    }
    else if(isOver == true && sens == true){
      content += "<h2>Game Over</h2><p>Ton dernier score etait " + (String)score + "</p>";
    }
    else if(game == true){
      content += "<h2>Ton score est</h2>";
      content += "<h1>" + (String)score + "</h1>"; 
    }
    else{
      content += "<p>Tu n'as pas de score<br>Il faut initialiser le jeu</p>";
    }

    page = htmlTop + content + htmlBottom;
    server.send(200, "text/html", page);});

  // ------------------------------------------------ Page : CHEAT
  server.on("/cheat", []() {
    content = "";

    if(gameZero == true){
      content += "<h2>Cheat non disponible</h2><p>Tu n'as pas encore commence un match</p>";
    }
    else if(isOver == true && sens == true){
      content += "<h2>Game Over</h2><p>Il faut reinitialiser le jeu</p>";
    }
    else if(game == true){
      content += "<h2>La bonne reponse est</h2>";
      content += "<h1>" + (String)randomNumber + "</h1>";  
    }
    else{
      content += "<p>Il faut initialiser le jeu</p>";
    }

    page = htmlTop + content + htmlBottom;
    server.send(200, "text/html", page);});

  // ------------------------------------------------ Page : SENSOR
  server.on("/sensor", []() {
    content = "";

    content += "<h2>Senseur BME280</h2>";
    content += getTemperature();
  
    page = htmlTop + content + htmlBottom;
    server.send(200, "text/html", page);});

  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

// ==================================================== Main loop :
void loop() {
  button.update();
  
  server.handleClient();
  MDNS.update();

  if(sens == false && game == false){
    sens = true;
    showTemperature();
  }

  if(buzz == true && isBuzzing == false){
    tone(buzzer, 200);
    buzz = false;
    isBuzzing = true;
    previousMillis = millis();
  }

  if(isBuzzing == true){
    currentMillis = millis();
    
    if(currentMillis - previousMillis >= interval){
    
      Serial.println(currentMillis - previousMillis);
      Serial.println(interval);
      noTone(buzzer);
      
      isBuzzing = false;
    }  
  }
  
  
  // ................................................ Long click :
  if (button.isLongClick()) {
    if(game == false){
      previousRandomNumber = -1;
      gameZero = false;
      game = true;
      isOver = false;
      score = 0;
      Serial.println("Game is now TRUE / ON");
      newRound();
    }
    else{
      Serial.println("Verifying...");
      verifyAnswer();
    }
  }

  // ................................................ Double click :
  if (button.isDoubleClick() && game == false){
    showingIP = screenIP(!showingIP);
  }
  

  // ................................................ Single click :
  if (button.isSingleClick()) {
    if(showingIP == true){
      showingIP = false;
    }
    
    if(isOver == true){
      sens = false;
    }
    if(game == false){
      Serial.println("Single click, game is off");
      showTemperature();
    }
    else{
      if(guess == MAX){
        guess = 1;
      }
      else{
        guess++;  
      }
      showNumber(guess);
      
      Serial.println("Single click, game is on, guess++");
      Serial.print("Guess: ");
      Serial.println(guess);
    }
  }
}

// ==================================================== Side functions :
void newRound(){
  Serial.println("New round");
  guess = 0;
  showNumber(guess);
  randomNumber = random(1,8);
  while(randomNumber == previousRandomNumber){
    randomNumber = random(1,8);
  }
  previousRandomNumber = randomNumber;
  lightsOn(randomNumber);
  Serial.print("Lights on - number ");
  Serial.println(randomNumber);
}

void verifyAnswer(){
  Serial.println("Function verifyAnswer");

  if(guess == randomNumber){
      Serial.println("Right answer");
      score++;
      newRound();
  }
  else{
    Serial.println("Wrong answer");
    Serial.println("Lights off, Game off");
    buzz = true;
    game = false;
    gameOver(score);
  }
}

void showNumber(int n){
  display.clearDisplay();
  display.setTextSize(5);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(50, 15);
  if(n == 0){
    display.print("?");
  }
  else{
    display.print(n);
  }
  display.display();
}

void lightsOn(int i){
  if((i % 8) > 3) { digitalWrite(d0, HIGH);} else { digitalWrite(d0, LOW);}
  if((i % 4) > 1) { digitalWrite(d6, HIGH);} else { digitalWrite(d6, LOW);}
  if((i % 2) > 0) { digitalWrite(d7, HIGH);} else { digitalWrite(d7, LOW);}
}

void gameOver(int score){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println("Game Over");
  display.setTextSize(1);
  display.println();
  display.print("Score: ");
  display.print(score);
  display.display();
  isOver = true;

  digitalWrite(d0, LOW);
  digitalWrite(d6, LOW);
  digitalWrite(d7, LOW);
}

void showTemperature(){
  now.update();

  String r1 = "Temp. = " + (String)bme.readTemperature() + " C";
  String r2 = "Press. = " + (String)(bme.readPressure() / 100.0F) + " hPa";
  String r3 = "Altitude = " + (String)bme.readAltitude(SEALEVELPRESSURE_HPA) + " m";
  String r4 = "Humidity = " + (String)bme.readHumidity() + " %";

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0,6);
  
  display.println(r1);
  display.println(r2);
  display.println(r3);
  display.println(r4);

  display.setCursor(0,55);

  if(now.getHours() < 10){
    display.print("0");
  }
  display.print(now.getHours());
  display.print(":");
  if(now.getMinutes() < 10){
    display.print("0");
  }
  display.print(now.getMinutes());
  display.print(":");
  if(now.getSeconds() < 10){
    display.print("0");
  }
  display.println(now.getSeconds());
  
  display.display();
  
  return;
}

String getTemperature(){
    String r = "";

    r += "Temperature = " + (String)bme.readTemperature() + " Celsius<br>";
    r += "Pressure = " + (String)(bme.readPressure() / 100.0F) + " hPa<br>";
    r += "Approx. Altitude = " + (String)bme.readAltitude(SEALEVELPRESSURE_HPA) + " m<br>";
    r += "Humidity = " + (String)bme.readHumidity() + " %";

    return r;
}

void handleNotFound() {
  server.send(200, "text/html", errorPage);
}


void setupConnection(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,10);
  display.print("Loading TP ");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
}

bool screenIP(bool b){  
  if(b == true){
    display.clearDisplay();
    display.setCursor(0,10);
    display.println("Web server");
    display.println();
    display.print("IP   : ");
    display.println(WiFi.localIP());
    display.print("Port : ");
    display.println(PORT);
    display.display();  
  }
  else{
    showTemperature();
  }

  return b;
}
