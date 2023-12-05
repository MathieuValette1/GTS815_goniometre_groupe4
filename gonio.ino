// Auteur: Mathieu Valette
// Librairies utiles
#include "Wire.h"
#include <MPU6050_light.h>  
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_GC9A01A.h"

// Capteur
MPU6050 mpu(Wire); 
unsigned long timer = 0;

// Ecran
#define TFT_DC  7
#define TFT_CS 10
#define TFT_BL 9

//Couleurs
#define BLACK GC9A01A_BLACK
#define WHITE GC9A01A_WHITE
#define RED GC9A01A_RED
#define GREEN GC9A01A_GREEN
#define BLUE GC9A01A_BLUE

// Dimensions de l'écran. X horizontal, Y vertical
#define X_MID_WIDTH 120
#define Y_MID_HEIGHT 120

#define LED_PIN 13 // LED du capteur A VERIFIER
bool blinkState = false;

Adafruit_GC9A01A tft(TFT_CS, TFT_DC);

// Variable stockant l'angle qui varie, l'angle de catch et l'angle maximum
int angle, angle_catch, angle_max = 0;

// Booléen mis à true si on appuie une fois sur le bouton
bool angle_catch_found = false;
// Booléen mis à true si on appuie une deuxième fois sur le bouton
bool angle_max_found = false;

// pin des boutons, DOIVENT ETRE COHERENT AVEC LE CABLAGE
const int boutonPin = 2; 
const int resetButtonPin = 3;
int buttonState = HIGH;
int resetButtonState = HIGH;


void setup() { 
  // Initialisation
  Serial.begin(9600);
  Wire.begin();
  Serial.print("Début");
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while(status!=0){ Serial.println("Check connection");}

  // Mise à zéro des variables si nécessaire
  angle_catch_found = false;
  angle_max_found = false;
  angle = 0;
  angle_catch = 0;
  angle_max = 0;

  // Allumage de l'écran
  beginDisplay();
  // Calibration du capteur
  calibrate();
  // Arrière plan
  drawBackgroundCircles(WHITE, RED, BLACK);

  // Activation des deux boutons
  pinMode(boutonPin, INPUT_PULLUP); // activation du bouton
  pinMode(resetButtonPin, INPUT_PULLUP); // activiation du bouton reset 
} 

// Boucle principale
void loop() { 
  // Lecture des boutons
  buttonState = digitalRead(boutonPin);
  resetButtonState = digitalRead(resetButtonPin);

  // Si le bouton de RESET est appuyé, on remet à zéro toutes les variables grâce à resetVariablesAndCalibrate()
  if (resetButtonState == LOW){
    resetVariablesAndCalibrate();
  }

  // Si le bouton d'enregistrement est appuyé, on met angle_catch_found à true
  if (buttonState == LOW){
    Serial.print("Click");
    // Si angle_catch_found est déjà true, c'est qu'on a déjà appuyé une fois. on met angle_max_found à true
    // if (angle_catch_found){
    //   angle_max_found = true;
    // }
    // Dans tous les cas, angle_catch_found est true
    angle_catch_found = true;
  }
  // Obtention des mesures
  mpu.update();    
  if((millis()-timer)>50){ // Toutes les 50 ms
  angle = mpu.getAngleZ(); // Obtention de l'angle en degrés
	
  //DEBUG
  //Serial.println("Angle de rotation autour de z: ");Serial.println(angle);
  if (!angle_catch_found){
      // Le bouton n'a jamais été appuyé, donc l'angle de catch n'est pas encore trouvé
      angle_catch = angle;
    }
  if (!angle_max_found){
    // Si le bouton n'a pas été appuyé deux fois, l'angle max n'a pas été trouvé.
    // Si la valeur de l'angle max est supérieure à la valeur précedente, on l'enregistre. Sinon, on ne l'enregistre pas
    if (angle_max < angle){
      angle_max = angle;
    }
  }

  // Affichage des valeurs, d'abord l'angle catch, puis l'angle max
  displayGonioAngle(abs(round(angle_catch)), abs(round(angle_max)));
	timer = millis();  
}
}

void calibrate(){
  // Calibration des capteurs
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000); // Petit délai pour que le capteur soit immobile
  mpu.calcOffsets();
  Serial.println("Done!\n");
}

void beginDisplay(){
  // Allumage de l'écran
  Serial.println("Allumage de l'écran...");
  tft.begin();
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH); // Backlight on
// Ecran de chargement
  drawETSLogo();
  delay(500);
  textIntro();
}

void drawETSLogo() {
  // Affichage du logo ETS
  tft.fillScreen(BLACK);
  tft.fillRect(60, 60, 125, 125, RED);
  tft.fillRect(85, 85, 75, 75, WHITE);
  tft.setCursor(95, 122);
  tft.setTextColor(RED);  tft.setTextSize(3);
  tft.println("E");
  tft.setCursor(115, 115);
  tft.println("T");
  tft.setCursor(135, 108);
  tft.println("S");
}

unsigned long textIntro(){
  //Affichage du texte d'intro
  tft.fillScreen(GC9A01A_BLACK);
  tft.setCursor(50, 75);
  tft.setTextColor(GC9A01A_WHITE);  tft.setTextSize(3);
  tft.println("Bonjour!");
  tft.setCursor(15, 150);
  tft.setTextColor(GC9A01A_WHITE, BLACK);  tft.setTextSize(1);
  tft.println("Prototype de Goniometre CDJLCFM");
  delay(500);  
  tft.fillScreen(GC9A01A_BLACK);
  tft.setCursor(20, 150);
  tft.setTextColor(GC9A01A_WHITE, BLACK);  tft.setTextSize(2);
  tft.println("Initialisation...");
}

void displayGonioAngle(float angle1, float angle2){
  int w = tft.width();
  int h = tft.height();

  uint16_t color_catch = WHITE;
  uint16_t color_max = WHITE;

  if (angle_catch_found){
    color_catch = GREEN;
  }

  if (angle_max_found){
    color_max = GREEN;
  }
  tft.setCursor(55, 70);
  tft.setTextColor(color_catch, BLACK);  tft.setTextSize(2);
  tft.println("Angle catch:");
  tft.setCursor(90, 92);
  tft.println(angle1);

  tft.setCursor(37, 130);
  tft.setTextColor(color_max, BLACK);  tft.setTextSize(2);
  tft.println("Angle maximum:");

  tft.setTextColor(color_max, BLACK);  tft.setTextSize(2);
  tft.setCursor(90, 152);
  tft.println(angle2);
}

void resetVariablesAndCalibrate() {

  textIntro();
  // Réinitialisation des variables
  angle = 0;
  angle_catch = 0;
  angle_max = 0;
  angle_catch_found = false;
  angle_max_found = false;

  // Recalibrage du capteur
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.calcOffsets(); // Gyro et accelero
  Serial.println("Done!\n");
  drawBackgroundCircles(WHITE, RED, BLACK);
  Serial.println(angle_catch);
}

void drawBackgroundCircles(uint16_t color_big, uint16_t color_mid, uint16_t color_little) {
  int radius_big_circle = X_MID_WIDTH;
  int radius_mid_circle = 110; 
  int radius_little_circle = 100; 
  
  tft.fillCircle(X_MID_WIDTH, Y_MID_HEIGHT, radius_big_circle, color_big);
  tft.fillCircle(X_MID_WIDTH, Y_MID_HEIGHT, radius_mid_circle, color_mid);
  tft.fillCircle(X_MID_WIDTH, Y_MID_HEIGHT, radius_little_circle, color_little);
}

