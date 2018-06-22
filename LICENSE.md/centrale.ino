/*

  ++ FINESTRA WINDECO ++
  
  
  Progetto per la tesina sperimentale: "WINDECO"
  Autore: WINDECO Development Team
  Licenza: Open-Source

  
	STRUTTURA DATI WINDECO:

  sendData(int mod, int percentScelta, int lum, int command, int co2)
  
	< : Inizio pacchetto dati
	> : Fine pacchetto dati
	
*/


// LIBRERIE //


#include <IRremote.h> // Libreria sensore infrarossi
#include <SoftwareSerial.h> // Libreria Bluetooth
#include <LiquidCrystal.h> // Libreria schermo LCD
#include "CO2Sensor.h" // Libreria sensore di CO2


// DEFINIZIONE PIN SENSORI //


#define pinLum1 A0
#define pinLum2 A1
#define pinLum3 A2
#define pinLum4 A3
#define pinCo2 A4
#define pinInfrarossi 8


// MAPPATURA PULSANTI TELECOMANDO //


#define BUTTON_OK 0xFF02FD
#define BUTTON_UP 0xFF629D
#define BUTTON_DOWN 0xFFA857
#define BUTTON_SX 0xFF22DD
#define BUTTON_DX 0xFFC23D
#define BUTTON_1 0xFF6897
#define BUTTON_2 0xFF9867
#define BUTTON_3 0xFFB04F
#define BUTTON_4 0xFF30CF
#define BUTTON_5 0xFF18E7
#define BUTTON_6 0xFF7A85
#define BUTTON_7 0xFF10EF
#define BUTTON_8 0xFF38C7
#define BUTTON_9 0xFF5AA5
#define BUTTON_0 0XFF4AB5


// DEFINIZIONE VARIABILI STATICHE GLOABLI //


#define CO2_HIGH 700 // Variabile che definisce la soglia massima della CO2 (1000 - 2700 ppm)
//#define TEMPO_CALIBRAZ_CO2 1800000 // Variabile che definisce ogni quanto tempo c'è da calibrare il sensore di CO2 (30 Min)
#define D true // Variabile per stampare le informazioni di Debug


// INIZIALIZZAZIONE DISPOSITIVI //


SoftwareSerial BTSerial(10, 11); // RX | TX
CO2Sensor co2Sensor(pinCo2, 0.99, 100); // Inizializzo il sensore di CO2 in A4
IRrecv infrared(pinInfrarossi); // Inizializzo il ricevitore infrarossi
LiquidCrystal lcd(13, 12, 5, 4, 3, 2);  // Inizializzo il display LCD 16x2 con i relativi Pin
boolean connesso = false; // Flag per la connessione tramite Bluetooth

void setup() {
  
  Serial.begin(9600); // Comunicazione seriale
  
  pinMode(9, OUTPUT);  // This pin will pull the HC-05 pin 34 (key pin) HIGH to switch module to AT mode
  digitalWrite(9, HIGH); // Switcha ad AT MODE

  BTSerial.begin(38400);  // HC-05 baud impostato su 38400
  lcd.begin(16, 2); // Inizializzo l'LCD 16x2
  infrared.enableIRIn(); // Abilita l'input dal ricevitore infrarossi
  co2Sensor.calibrate(); // Calibra subito il sensore di CO2

  pinMode(pinLum1, INPUT); // Input perché legge la luminosità dal sensore
  pinMode(pinLum2, INPUT); // Input perché legge la luminosità dal sensore
  pinMode(pinLum3, INPUT); // Input perché legge la luminosità dal sensore
  pinMode(pinLum4, INPUT); // Input perché legge la luminosità dal sensore
  
  delay(1000);
  BTSerial.print("AT\r\n"); // Imposto il ruolo del dispositivo (1: Master, 0: Slave)
  delay(500);
  BTSerial.print("AT+ROLE=1\r\n"); // Imposto il ruolo del dispositivo (1: Master, 0: Slave)
  delay(500);
  BTSerial.print("AT+CMODE=1\r\n"); //Modalità di connessione
  delay(500);
  BTSerial.print("AT+BIND=21,13,4AC76\r\n"); // Imposta l'indirizzo Bluetooth dello Slave
  delay(500);
  connesso = true;

  digitalWrite(9, LOW);
  lcd.print("Lum. desiderata:"); // 16 caratteri giusti giusti ;)
  lcd.setCursor(0, 1); // Setto il cursore nella colonna 1
  
}

int lum = 0; // Variabile luminosità
void leggi_lum() { // Legge la luminosità
  lum = (int)(analogRead(pinLum1) + analogRead(pinLum2) + analogRead(pinLum3) + analogRead(pinLum4))/4;
}

// unsigned long tempoCo2 = millis(); // Variabile del tempo per la calibrazione della co2
int co2; // Variabile CO2 (PRIMA ERA 800)
void leggi_co2() { // Legge il sensore di CO2

  co2 = co2Sensor.read(); // Lettura della co2
  
  //TODO CALIBRAZIONE SENSORE CO2
  
  if(co2 > CO2_HIGH) { // Se la CO2 è alta
    if(D) {
      Serial.print("** CO2 troppo alta! **");
      Serial.println("");
    }
  }
}

int irCmd = 0; // Valore letto dal sensore infrarossi inizialmente nullo
String numLcd = "";
int numLcdFinal = 100; // Percentuale della luminosità voluta dall'utente da inviare tramite Bluetooth
int modAutomatica = 1; // Indica se la modalità automatica è attivata
void leggi_ir() { // Per leggere i segnali del telecomando infrarossi
  
  decode_results segnale; // Per sapere il segnale inviato al sensore infrarossi

  if(infrared.decode(&segnale)) { // this checks to see if a code has been received

    Serial.println(segnale.value, HEX); // prints the hex value a a button press
  
    if(segnale.value == BUTTON_UP) {
      if(modAutomatica == 0) irCmd = 1; // 1 = su
    }
    if(segnale.value == BUTTON_DOWN) {
      if(modAutomatica == 0) irCmd = 2; // 2 = giù
    }
    if(segnale.value == BUTTON_SX) {
      if(modAutomatica == 0) modAutomatica = 1;
    }
    if(segnale.value == BUTTON_DX) {
      if(modAutomatica == 1) modAutomatica = 0;
    }
    if(segnale.value == BUTTON_1) {
      numLcd += 1;
      lcd.print(1, DEC);
    }
    if(segnale.value == BUTTON_2) {
      numLcd += 2;
      lcd.print(2, DEC);
    }
    if(segnale.value == BUTTON_3) {
      numLcd += 3;
      lcd.print(3, DEC);
    }
    if(segnale.value == BUTTON_4) {
      numLcd += 4;
      lcd.print(4, DEC);
    }
    if(segnale.value == BUTTON_5) {
      numLcd += 5;
      lcd.print(5, DEC);
    }
    if(segnale.value == BUTTON_6) {
      numLcd += 6;
      lcd.print(6, DEC);
    }
    if(segnale.value == BUTTON_7) {
      numLcd += 7;
      lcd.print(7, DEC);
    }
    if(segnale.value == BUTTON_8) {
      numLcd += 8;
      lcd.print(8, DEC);
    }
    if(segnale.value == BUTTON_9) {
      numLcd += 9;
      lcd.print(9, DEC);
    }
    if(segnale.value == BUTTON_0) {
      numLcd += 0;
      lcd.print(0, DEC);
    }
    if(segnale.value == BUTTON_OK) {
      numLcdFinal = numLcd.toInt(); // Converto la mia stringa in Intero, così da poterlo inviare
      
      if(numLcdFinal >= 100) numLcdFinal = 100; // Così non va in Overflow
      sendData(modAutomatica, numLcdFinal, lum, irCmd, co2); // Invio l'intero blocco di dati
      numLcd = ""; // Ripulisco la stringa
      lcd.clear();
      lcd.print("Inviata! " + String(numLcdFinal));
      delay(3000); // Blocco per 3 secondi
      lcd.clear();
      lcd.print("Lum. desiderata:");
      lcd.setCursor(0, 1);
    }
    infrared.resume(); // Riceve il prossimo segnale
  }
}

void scrivi_bt() { // Scrive i segnali del Bluetooth
  if(Serial.available())
    BTSerial.write(Serial.read());
    
  if(BTSerial.available())
    Serial.write(BTSerial.read());
}

float bytesToInt (byte array[4]) { // Converto byte in intero
  int l = 0;
  
  l =  (int)array[3];
  l += (int)array[2] * 256;
  l += (int)array[1] * 256 * 2;
  l += (int)array[0] * 256 * 3;
    
  //float f = (float)l / 10000; // Solo per convertirlo in float
  return l;
}

byte *intToBytes(int n) { // Converto una variabile intera in bytes
  
  byte *buf = (byte*)malloc(sizeof(int)); // Alloco dinamicamente un array di Byte
  buf[0] = (n >> 8) & 0xFF; // Byte 1: i primi 8 bit sono il primo Byte 
  buf[1] = n & 0xFF; // Byte 2: gli ultimi int sono l'altro Byte
  return buf;
}


float getArrayFloat(byte array[32]) { // 32 byte
	float f[8]; // Array di float
	for(int i = 0; i < 32; i += 4){
		byte tmp[4];
	}
	
}

int totData = 0;
void sendData(int mod, int percentScelta, int lum, int command, int co2){
  
    BTSerial.write('<'); // Carattere iniziale
    byte *bytes_send = intToBytes(mod); // Valore che indica se è attiva la modalità automatica
    BTSerial.write(bytes_send[0]);
    BTSerial.write(bytes_send[1]);
    
    bytes_send = intToBytes(percentScelta); // Valore scelto dall'utente della luminosità (percentuale)
    BTSerial.write(bytes_send[0]);
    BTSerial.write(bytes_send[1]);
    
    bytes_send = intToBytes(lum); // Luminosità interna
    BTSerial.write(bytes_send[0]);
    BTSerial.write(bytes_send[1]);
    
    bytes_send = intToBytes(command); // Comando infrarossi
    BTSerial.write(bytes_send[0]);
    BTSerial.write(bytes_send[1]);
    
    bytes_send = intToBytes(co2); // CO2
    BTSerial.write(bytes_send[0]);
    BTSerial.write(bytes_send[1]);
    
    totData = mod + percentScelta + lum + command + co2;
    bytes_send = intToBytes(totData); // Per verificare che i dati sono stati inviati correttamente
    BTSerial.write(bytes_send[0]);
    BTSerial.write(bytes_send[1]);
    BTSerial.write('>'); // Carattere terminatore 
}

long oldTime = 0;
void loop() {
  
  leggi_ir(); // Legge il sensore infrarossi
  leggi_lum(); // Legge la luminosità
  leggi_co2(); // Da aggiustare, causa problemi... (forse non più)
  //scrivi_bt(); // Per debug

  if(connesso && (( (long)millis() - 1000) > oldTime)){ // Se è connesso ed è passato 1 secondo (invia ogni secondo i dati)
    oldTime = millis();
    sendData(modAutomatica, numLcdFinal, lum, irCmd, co2); // sendData(int mod, int percentScelta, int lum, int command, int co2)
  }
  if(D) { // Informazioni di Debug
    
      Serial.print("Comando infrarossi: ");
      Serial.println(irCmd);

      Serial.print("Percentuale lum utente: ");
      Serial.println(numLcdFinal);
      
      Serial.print("Luminosità media: ");
      Serial.println(lum);
      
      Serial.print("CO2 rilevata: ");
      Serial.println(co2);
      Serial.println("");
      
    }
}

