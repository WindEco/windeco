/*

  ++ FINESTRA WINDECO ++
	
	
  Progetto per la tesina sperimentale: "WINDECO"
  Autore: WINDECO Development Team
  Licenza: MIT

*/


// LIBRERIE //


#include <SoftwareSerial.h>


// DEFINIZIONE PIN SENSORI //


#define pinLumESopra A0 // Fotoresitenza all'ingresso analogico 0
#define pinLumESotto A1 // Fotoresitenza all'ingresso analogico 1
#define pinPioggia 8 // Sensore di pioggia
#define pinMotore 7 // Pin A del motore 7
#define pinMotorePwm 6 // Pin B del motore 6
#define pinVentola 4 // Pin A della ventola 5
#define pinVentolaPwm 5 // Pin B della ventola 4
SoftwareSerial BTSerial(2, 3); // RX | TX


// DEFINIZIONE VARIABILI STATICHE GLOABALI //


#define CO2_HIGH 700 // Variabile CO2 troppo elevata
#define SENSORE_MAX 1023 // Max (forse 1024)
#define SENSORE_MIN 0 // Min
#define LUM_HIGH 800 // Luminosità che fa scattare il giorno
#define D true // Variabile per stampare le informazioni di Debug


void setup() {
    Serial.begin(9600); // Baud del Monitor Seriale
    BTSerial.begin(38400); // Baud del Bluetooth
    pinMode(9, OUTPUT); // Key 
    digitalWrite(9, HIGH);
    
    pinMode(pinMotore, OUTPUT);  // Pin A del motore
    pinMode(pinMotorePwm, OUTPUT);  // Pin B del motore
    pinMode(pinVentola, OUTPUT); // Pin A ventola ricircolo
    pinMode(pinVentolaPwm, OUTPUT); // Pin B ventola ricircolo
    pinMode(pinLumESopra, INPUT); // Fotoresistenza esterna sopra
    pinMode(pinLumESotto, INPUT); // Fotoresistenza esterna sotto

    //BTSerial.write("AT+UART?");
    //BTSerial.write("AT+UART=38400,0,0\r\n");
    //BTSerial.write("AT+ADDR\r\n");
    digitalWrite(9, LOW);
}

void leggi_bt() { // Scrivo i dati letti dal Bluetooth
  if (BTSerial.available())
    Serial.write(BTSerial.read());
    
  if (Serial.available())
    BTSerial.write(Serial.read());
}

float bytesToInt (byte array[2]) { // Converto i byte in interi
  int l = 0;
  
  l = (int)array[1];
  l += (int)array[0] * 256;
  
  return l;
}



// Variabili temporanee! //



int modAutomaticaTmp; // Forse da mettere = 1
int lumInternaTmp; // Quella inviata dagli altri sensori installati sulla centrale operativa
int percentSceltaTmp = 100; // Quella che ha scelto l'utente dalla centrale operativa
int co2Tmp; // Variabile della CO2

// Variabili vere e proprie! //


int modAutomatica;
int lumInterna; // Quella inviata dagli altri sensori installati sulla centrale operativa
int percentScelta; // Quella che ha scelto l'utente dalla centrale operativa
int lumScelta;
int co2; // Variabile della CO2
int totDataRecv;
int totData;
int irValue;

void recvData() { // Funzione per ricevere i dati tramite Bluetooth
  
  char c = BTSerial.read();
  
  if(c == '<') { // Se viene letto il carattere iniziale
    while(BTSerial.available() < 12);
    byte b[2];
    
    b[0] = BTSerial.read();
    b[1] = BTSerial.read();
    modAutomaticaTmp = bytesToInt(b); // Mod Automatica

    b[0] = BTSerial.read();
    b[1] = BTSerial.read();
    percentSceltaTmp = bytesToInt(b); // Percentuale luminosità

    b[0] = BTSerial.read();
    b[1] = BTSerial.read();
    lumInternaTmp = bytesToInt(b); // Luminosità interna letta dalla centrale
    
    b[0] = BTSerial.read();
    b[1] = BTSerial.read();
    irValue = bytesToInt(b); // Valore telecomando infrarossi
    
    b[0] = BTSerial.read();
    b[1] = BTSerial.read();
    co2 = bytesToInt(b); // Valore CO2

    b[0] = BTSerial.read();
    b[1] = BTSerial.read();
    totDataRecv = bytesToInt(b); // Somma dei dati ricevuti

    totData = modAutomaticaTmp + percentSceltaTmp + lumInternaTmp + irValue + co2Tmp;

    if(totData == totDataRecv) { // Controllo di eventuali errori nella comunicazione Bluetooth
        modAutomatica = modAutomaticaTmp;
        lumInterna = lumInternaTmp;
        percentScelta = percentSceltaTmp;
        co2 = co2Tmp;
        irRecv(irValue);
        
        lumScelta = (lumInterna * percentScelta) / 100; // Calcolo luminosità effettiva voluta dall'utente (lo calcolo solo se è giusta la ricezione)

        if(D) {
          Serial.print("\n********* DATI CORRETTI *********\n\n");
          
          Serial.print("Effettiva Mod Automatica: ");
          Serial.println(modAutomatica);
          
          Serial.print("Effettiva Percentuale lum utente: ");
          Serial.println(percentScelta);
    
          Serial.print("Effettiva Luminosità effettiva scelta: ");
          Serial.println(lumScelta);
          
          Serial.print("Effettiva Lum interna: ");
          Serial.println(lumInterna);
          
          Serial.print("Effettiva CO2: ");
          Serial.println(co2);
          
          Serial.print("Effettivo comando infrarossi: ");
          Serial.println(irValue);
    
          Serial.println("** RICEZIONE DATI PERFETTAAAAAAA! **");
     
          Serial.print("\n****************\n\n");
        }
        
    }else if(D) {
      Serial.print("\n********* DATI ERRATI *********\n\n");
      
      Serial.print("Mod Automatica ricevuta: ");
      Serial.println(modAutomaticaTmp);
      
      Serial.print("Percentuale lum utente ricevuta: ");
      Serial.println(percentSceltaTmp);

      //Serial.print("Luminosità effettiva scelta ricevuta: ");
      //Serial.println(lumSceltaTmp);
      
      Serial.print("Lum interna ricevuta: ");
      Serial.println(lumInternaTmp);
      
      Serial.print("CO2 ricevuta: ");
      Serial.println(co2Tmp);
      
      Serial.print("Comando infrarossi ricevuto: ");
      Serial.println(irValue);

      Serial.println("** RICEZIONE DATI ERRATAAAAAAA! **");
 
      Serial.print("\n****************\n\n");
    }
  }
}


void irRecv(int irCmd){ // In base ai comandi dati dall'utente via IR, decido cosa fare
  if(irCmd == 0) return; // Se ricevo un comando nullo esco dalla funzione
  
  if(irCmd == 1 && !modAutomatica) { // SALE
    vai_su(); // Va su la veneziana
    if(D) Serial.println("LA VENEZIANA DOVREBBE SALIRE!");
  }
  
  if(irCmd == 2 && !modAutomatica) { // SCENDE
    vai_giu(); // Va su la veneziana
    if(D) Serial.println("LA VENEZIANA DOVREBBE SCENDERE!");
  }
  
}

boolean firstLaunch = true; // Serve all'inizio per togliere il delay e quindi acquisire un valore iniziale della luminosità
boolean giorno = true; // Variabile booleana di giorno/notte
int lumE = 0; // Luminosità media ultimi 5/10 min
int pCamp = 0; // Primo campionamento luminosità
unsigned long ut = 0; // ms ultimo campionamento
int ppCamp = 0; // Posizione primo campionamento
int sCamp[10]; // Secondo campionamento lum
int psCamp = 0; // Indice secondo campionamento lum
void leggi_gn() { // Legge giorno e notte in base alla media dei sensori di lum esterni
  
  unsigned long t = millis(); // Timer
  int camp = 4000; // Campiona ogni 4s

  if(giorno) // Se è giorno
    camp = 2000;
  if(firstLaunch) // Se è la prima volta che viene avviato il programma, il campionamento avverà ogni 1ms (per testing)
    camp = 1;
   
  if(t - ut > camp) {
    pCamp += analogRead(pinLumESopra); // Lettura luminosità interna sensore alto
    pCamp += analogRead(pinLumESotto); // Lettura luminosità interna sensore sotto
    ppCamp += 2;

    if(ppCamp >= 30) { // Se l'array con il primo campionamento è pieno
      // Esegue la media del primo campionamento
      float media = pCamp; 
      media /= ppCamp;
      sCamp[psCamp++] = (int)(media); // Inserisce i dati nel secondo campionamento
      pCamp = 0; //Azzero la somma degli elementi
      ppCamp = 0; // Azzera l'index (ricomincia da capo, perché Arduino non supporta troppi valori)
    }

    if(psCamp >= 10){ // Se ha acquisito tutti i campionamenti del secondo array
      psCamp = 0; // Azzera l'index del secondo campionamento

      // Esegue la media del secondo campionamento
      float media = 0;
      for(int i = 0; i < 10; i++) 
        media += sCamp[i];
        
      media /= 10; // Esegue la media vera e propria
      
      if(firstLaunch) // Controlla se la finestra è appena stata installata e annulla il ciclo di inizializzazione della luminosità
        firstLaunch = false;
        
      lumE = (int)media; // Inserisce il dato pubblico della luminosità esterna
    }

    if(lumE > LUM_HIGH || firstLaunch) // Se la luminosità è alta o è appena stata installata la finestra imposta giorno = true
      giorno = true; // GIORNO RILEVATO!
    else
      giorno = false; // NOTTE RILEVATA!
    
    ut = t; // Tempo in cui è stato eseguito l'ultimo campionamento
  }
  
}

boolean piove = false; // Variabile per verificare se piove oppure no
void leggi_pioggia() {
  
  int valPioggia = digitalRead(pinPioggia); // Legge 1 oppure 0
  
  if(valPioggia == 1) // Se sta piovendo
    if(!piove) piove = true;
  else
    if(piove) piove = false;
  
}

void vai_su() { // Per farla salire
  digitalWrite(pinMotore, HIGH); // Forse da invertire
  digitalWrite(pinMotorePwm, 255);
}

void vai_giu() { // Per farla scendere 
  digitalWrite(pinMotore, LOW); // Forse da invertire
  digitalWrite(pinMotorePwm, 255);
}

void fermati() { // Per farla fermare
  //digitalWrite(pinMotore, HIGH);
  digitalWrite(pinMotorePwm, 0);
}

boolean ventola = false;
void ventola_on() { // Attivazione della ventola
  if(ventola) return; // Se la ventola è accesa allora esco
  digitalWrite(pinVentola, HIGH); // Forse va invertito
  digitalWrite(pinVentolaPwm, 255);
  ventola = true;
}

void ventola_off() { // Disattivazione della ventola
  if(!ventola) return; // Se la ventola è spenta allora esco
  //digitalWrite(pinVentola, LOW);
  digitalWrite(pinVentolaPwm, 0);
  ventola = false;
}

long oldTime = 0;
float lumEsterna = (analogRead(pinLumESopra) + analogRead(pinLumESotto))/2; // Luminosità esterna
void loop() {
  
  recvData(); // Ricevimento dati dal Bluetooth
  leggi_pioggia(); // Legge se piove oppure no
  leggi_gn(); // Legge se è giorno oppure notte

  if(co2 > CO2_HIGH) 
    ventola_on(); // Se la CO2 è alta la ventola del ricircolo si accende
  else
    ventola_off(); // Altrimenti spengo la ventola
    
  // deve chiedere al bt la lum interna, quella richiesta dall'utente, e dopo se dentro c'è più luminosità di quella richiesta dall'utente e fuori c'è più lumonosità che quella interna, chiude la persiana
  if((modAutomatica && giorno && lumInterna > lumScelta && lumEsterna > lumInterna) || piove) vai_giu(); // Se è attiva la modAuto ed è notte e dentro c'è più luminosità di quella richiesta dall'utente e fuori c'è più lumonosità che quella interna oppure piove, chiude la persiana
  
  if(modAutomatica && !giorno) vai_giu(); // Se modAuto è accesa ed è notte, viene giù la veneziana

  if((modAutomatica&& giorno && lumInterna < lumScelta && lumInterna < lumEsterna) || !piove) vai_su(); // Se modAuto è attiva e è giorno e quando la lumInterna è minore di quella desiderata e la lumEsterna è maggiore di quella interna
 
  if(D && (( (long)millis() - 1000) > oldTime)) { // Scrive le informazioni di Debug
    oldTime = millis();

    Serial.print("\n ----------------------- \n\n");
    
    Serial.print("Media luminosità esterna: ");
    Serial.println(lumEsterna);
    
    Serial.print("Giorno: ");
    if(giorno) Serial.println("SI");
    else Serial.println("NO");
    
    Serial.print("Piove: ");
    if(piove) Serial.println("SI");
    else Serial.println("NO");

    Serial.print("\n ----------------------- \n\n");

  }

}
