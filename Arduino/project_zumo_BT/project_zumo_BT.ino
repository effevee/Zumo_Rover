/*
 * Project : Bestuur een Zumo robot chassis met behulp van Bluetooth communicatie
 * 
 * (c)2017 Frank Vergote
 * 
 * Benodigdheden :
 * - Arduino UNO of NANO
 * - 9V batterij om Arduino te voeden
 * - Zumo robot chassis 
 * - 4 AA batterijen om motoren te voeden
 * - 2 mini DC motoren met gearbox
 * - L293D dual H-Bridge chip
 * - Bluetooth module HC-05
 * - Android toestel met Bluetooth App voor besturing
 * 
 * Stuur de DC motoren aan met de L293D dual H-bridge chip. Gebruik een BT module om de signalen
 * van de Android APP op te vangen. Deze worden serieel doorgestuurd naar de Arduino die ze
 * gebruikt om de zumo robot te besturen. 
 * 
 * Besturing commando's van de BT App :
 *            
 *    V = vooruit
 *    L = links
 *    H = stoppen (halt)
 *    R = rechts
 *    A = achteruit
 *    S999 = snelheid (S gevolgd door de snelheid (0-255))
 *    
 * 
 * Pinout van de L293D chip:
 * 
 * Pin# Pin   Functie
 * ------------------------
 * 1    ENA   Motor A enable
 * 2    1A    Motor A sturing 1
 * 3    1Y    Motor A terminal 1
 * 4    GND   Aarde/heatsink
 * 5    GND   Aarde/heatsink
 * 6    2Y    Motor A terminal 2
 * 7    2A    Motor A sturing 2
 * 8    VCC2  Voeding motor
 * 9    ENB   Motor B enable
 * 10   3A    Motor B sturing 1
 * 11   3Y    Motor B terminal 1
 * 12   GND   Aarde/heatsink
 * 13   GND   Aarde/heatsink
 * 14   4Y    Motor B termanal 2
 * 15   4A    Motor B sturing 2
 * 16   VCC1  Voeding chip (5V)
 * 
 * Sturing logica L293D:
 * 
 *  EN  1A  2A  Functie
 *  --------------------------
 *  1   1   0   rechts draaien
 *  1   0   1   links draaien  
 *  1   0   0   stoppen (remmen)
 *  1   1   1   stoppen (remmen)
 *  0   X   X   stoppen (uitlopen)
 *  
 */

// bibliotheken
#include <SoftwareSerial.h>

const int txdPin = 9;
const int rxdPin = 11; 
SoftwareSerial zumoSerial = SoftwareSerial(rxdPin, txdPin);

// constanten
const int motorL_sturing1 = 4;    // pins linker motor
const int motorL_sturing2 = 2;
const int motorL_enable = 3;      // PWM !

const int motorR_sturing1 = 6;    // pins rechter motor
const int motorR_sturing2 = 7;
const int motorR_enable = 5;      // PWM !

const int min_snelheid = 96;      // beginsnelheid PWM 0-255

const char VOORUIT   = 'V';       // commando's besturing
const char ACHTERUIT = 'A';
const char LINKS     = 'L';
const char RECHTS    = 'R';
const char STOPPEN   = 'H';
const char SNELHEID  = 'S';

// variabelen
int motor_snelheid = min_snelheid;
int draaitijd = 0;                // draaitijd bij links/rechts draaien (ms)
                                  // is omgekeerd evenredig met de motorsnelheid
  
void setup() {
  // pinnen H-brug als output
  pinMode(motorL_sturing1, OUTPUT);
  pinMode(motorL_sturing2, OUTPUT);
  pinMode(motorL_enable, OUTPUT);
  pinMode(motorR_sturing1, OUTPUT);
  pinMode(motorR_sturing2, OUTPUT);
  pinMode(motorR_enable, OUTPUT);

  // zumo seriele communicatie starten
  zumoSerial.begin(9600);
  
  // seriele debug console
  Serial.begin(9600);

  // zumo stoppen
  zumo(STOPPEN);
}

void loop() {
  // lees inkomende BT signalen 
  while (zumoSerial.available() > 0) {
    char kar = zumoSerial.read();
    switch(kar) {
      case VOORUIT: 
        Serial.println("VOORUIT");
        zumo(VOORUIT); 
        break;
      case ACHTERUIT: 
        Serial.println("ACHTERUIT"); 
        zumo(ACHTERUIT); 
        break;
      case STOPPEN:
        Serial.println("STOPPEN");
        zumo(STOPPEN); 
        break;
      case LINKS:
        Serial.println("LINKS");
        zumo(LINKS); 
        break;
      case RECHTS:
        Serial.println("RECHTS");
        zumo(RECHTS); 
        break;
      case SNELHEID:
        motor_snelheid = zumoSerial.parseInt();  // lees de snelheid in als integer
        Serial.print("SNELHEID ");
        Serial.println(motor_snelheid);
        zumo(SNELHEID);
        break;        
    }
  }
}

// besturing zumo robot chassis
void zumo(char commando) {

  switch(commando) {
       
    case VOORUIT:
      // beide motoren -> sturing 1 laag - sturing 2 hoog
      digitalWrite(motorL_sturing1, LOW);
      digitalWrite(motorL_sturing2, HIGH);
      digitalWrite(motorR_sturing1, LOW);
      digitalWrite(motorR_sturing2, HIGH);
      // beide motoren enablen met motorsnelheid (PWM)
      analogWrite(motorL_enable, motor_snelheid);
      analogWrite(motorR_enable, motor_snelheid);
      // debug info
      Serial.print("Zumo VOORUIT - snelheid ");
      Serial.println(motor_snelheid);
      break;
         
    case LINKS:
      // linker motor  -> sturing 1 hoog - sturing 2 laag
      // rechter motor -> sturing 1 laag - sturing 2 hoog
      digitalWrite(motorL_sturing1, HIGH);
      digitalWrite(motorL_sturing2, LOW);
      digitalWrite(motorR_sturing1, LOW);
      digitalWrite(motorR_sturing2, HIGH);
      // beide motoren enablen met motorsnelheid (PWM)
      analogWrite(motorL_enable, motor_snelheid);
      analogWrite(motorR_enable, motor_snelheid);
      // debug info
      Serial.print("Zumo LINKS - snelheid ");
      Serial.print(motor_snelheid);
      draaitijd = map(motor_snelheid, 0, 255, 1000, 0); // omgekeerd evenredig metmotorsnelheid
      Serial.print(" - tijd ");
      Serial.println(draaitijd);
      // na draaitijd stoppen
      delay(draaitijd);
      zumo(STOPPEN);
      break;
         
    case STOPPEN:
      // beide motoren -> sturing 1 laag - sturing 2 laag
      digitalWrite(motorL_sturing1, LOW);
      digitalWrite(motorL_sturing2, LOW);
      digitalWrite(motorR_sturing1, LOW);
      digitalWrite(motorR_sturing2, LOW);
      // beide motoren -> enable hoog (remmen !) 
      analogWrite(motorL_enable, HIGH);
      analogWrite(motorR_enable, HIGH);
      // debug info
      Serial.println("Zumo STOPPEN");
      break;
    
    case RECHTS:
      // linker motor  -> sturing 1 laag - sturing 2 hoog
      // rechter motor -> sturing 1 hoog - sturing 2 laag
      digitalWrite(motorL_sturing1, LOW);
      digitalWrite(motorL_sturing2, HIGH);
      digitalWrite(motorR_sturing1, HIGH);
      digitalWrite(motorR_sturing2, LOW);
      // beide motoren enablen met motorsnelheid (PWM)
      analogWrite(motorL_enable, motor_snelheid);
      analogWrite(motorR_enable, motor_snelheid);
      // debug info
      Serial.print("Zumo RECHTS - snelheid ");
      Serial.print(motor_snelheid);
      draaitijd = map(motor_snelheid, 0, 255, 1000, 0); // omgekeerd evenredig met motorsnelheid
      Serial.print(" - tijd ");
      Serial.println(draaitijd);
      // na draaitijd stoppen
      delay(draaitijd);
      zumo(STOPPEN);
      break;
         
    case ACHTERUIT:
      // beide motoren -> sturing 1 hoog - sturing 2 laag
      digitalWrite(motorL_sturing1, HIGH);
      digitalWrite(motorL_sturing2, LOW);
      digitalWrite(motorR_sturing1, HIGH);
      digitalWrite(motorR_sturing2, LOW);
      // beide motoren enablen met motorsnelheid (PWM)
      analogWrite(motorL_enable, motor_snelheid);
      analogWrite(motorR_enable, motor_snelheid);
      // debug info
      Serial.print("Zumo ACHTERUIT - snelheid ");
      Serial.println(motor_snelheid);
      break;
         
    case SNELHEID:
      // beide motoren enablen met motorsnelheid (PWM)
      analogWrite(motorL_enable, motor_snelheid);
      analogWrite(motorR_enable, motor_snelheid);
      // debug info
      Serial.print("Zumo SNELHEID ");
      Serial.println(motor_snelheid);
      break;
      
    default:
      // debug info
      Serial.println("Zumo ONGELDIG COMMANDO");
      break;  
      
  }
}

