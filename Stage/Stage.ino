/*
Dit is het standaard programma om de stage met de controller te kunnen aansturen
Dit is het programma dat in de controller zit wanneer het wordt afgeleverd
Je kunt zelf een ander programma in de arduino uploaden, maar als je het oude programma weer terug wilt kun je dit bestand uploaden
Ook kun je zelf dit bestand aanpassen en gebruiken, maar zorg en dan voor dat je altijd een kopie hebt van het orginele bestand zodat je het niet kwijt raakt
*/

// definieer pin nummers van arduino
// de lijsten zijn zo ingericht: {xas, yas, zas}
const int StepPin[] = {50, 44, 38};  // de pinnen om stappen te maken:
const int DIRPin[] = {48, 42, 36};   // de pinnen om richting aan te geven: als de pin hoog is beweegt de motor naar de positieve kant en als het laag is naar de negatieve kant
const int Sleep[] = {52, 46, 40};    // de pinnen om slaapmodus van de motoren in of uit te schakelen
const int limit0[] = {39, 35, 31};     // de pinnen van de limitswitches aan de negatieve kanten
const int limit1[] = {41, 37, 33};     // de pinnen van de limitswitches aan de positieve kanten

const int leds0[] = { 5, 3, 8 };  // de leds om aan te geven wanneer de motoren in negatieve richtingen gaan
const int leds1[] = { 6, 4, 2 };  // de leds om aan te geven wanneer de motoren in positieve richtingen gaan


#define xLstick A0                             // de pin voor de x richting van de linker joystick
#define yLstick A1                             // de pin voor de y richting van de linker joystick
#define xRstick A2                            // de pin voor de x richting van de rechter joystick (deze wordt in dit programma niet gebruikt)
#define yRstick A7                            // de pin voor de y richting van de rechter joystick (deze wordt gebruikt om de z richting te besturen)
#define LJoystickbutton 41                             // pin voor de linker joystickknop
#define RJoystickbutton 43                              // pin voor de rechter joystickknop
#define DemoLed 12                                      // pin voor led voor demomodus
#define demoButton 47

int stickInputs[] = { 0, 0, 0 };  // om de gemeten inputs van de joysticks op te slaan

unsigned long currentMicros = micros(); // om op te slaan hoeveel microseconden het programma op een bepaald moment duurt
unsigned long previousMicros[] = { 0, 0, 0 };  // om op te slaan hoelang geleden een bepaalde motor voor het laatst een steppuls heeft gekregen
int DemoButtonMillis = millis();

void setup() {
  //Serial.begin(9600); // kan worden ongecommenteerd om communicatie met pc starten voor debugging

  //Leds ground instellen
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
  // de arduino pinnen op de goede pinmodes zetten
  for (int axis = 0; axis < 3; axis++) {
    //pinModes voor de stepper drivers
    pinMode(StepPin[axis], OUTPUT);
    pinMode(DIRPin[axis], OUTPUT);
    pinMode(Sleep[axis], OUTPUT);
    pinMode(limit0[axis], INPUT_PULLUP);
    pinMode(limit1[axis], INPUT_PULLUP);

    //pinModes voor de leds
    pinMode(leds0[axis], OUTPUT);
    pinMode(leds1[axis], OUTPUT);
    pinMode(DemoLed, OUTPUT);

    // startwaarden voor de outputs
    digitalWrite(StepPin[axis], LOW);
    digitalWrite(DIRPin[axis], HIGH);
    digitalWrite(Sleep[axis], LOW);
    digitalWrite(DemoLed, LOW);
    analogWrite(leds0[axis], 0);
    analogWrite(leds1[axis], 0);
  }
  // joystick pinnen als input zetten
  pinMode(xLstick, INPUT);
  pinMode(yLstick, INPUT);
  pinMode(yRstick, INPUT);
  pinMode(RJoystickbutton, INPUT_PULLUP);
  pinMode(LJoystickbutton, INPUT_PULLUP);

  pinMode(demoButton, INPUT_PULLUP);
}

// functie die als parameters een bepaalde as(als getal: 0=x, 1=y of 2=z) en een snelheid(als getal tussen -255 en 255) heeft
// de functie veranderd de felheid van de lampjes, checkt of er limitswitches zijn ingedrukt en geeft een steppuls als dat nodig is
// de functie wordt gebruikt met: StepPulse(as, snelheid);
void StepPulse(int axis, int input) {
  if (input > 0) {
    analogWrite(leds1[axis], input);                                                  // zet lampje aan met de goede felheid
    digitalWrite(DIRPin[axis], HIGH);                                                 // zet de motor in de goede richting
    
    currentMicros = micros(); // slaat op hoeveel microseconden er op dit moment zijn verstreken
    //Serial.println(map(input, 0, 255, 50, 5));
    if ((digitalRead(limit1[axis]) == LOW) && (currentMicros > previousMicros[axis] + map(input, 0, 250, 5000, 0)) ) { // checkt of de limitswitch niet is ingeschakelt en of er genoeg tijd is verstreken sinds de laatste steppuls afhankelijk van de snelheid
      // geeft een puls aan de steppin
      digitalWrite(StepPin[axis], HIGH);
      delayMicroseconds(1);
      digitalWrite(StepPin[axis], LOW);

      previousMicros[axis] = micros(); // slaat op wanneer voor het laatst een stap is gezet
    } 
    

  } else if (input < 0) {
    analogWrite(leds0[axis], -input);                                                      // zet lampje aan met de goede felheid
    digitalWrite(DIRPin[axis], LOW);                                                       // zet de motor in de goede richting

    currentMicros = micros(); // slaat op hoeveel microseconden er op dit moment zijn verstreken
    if ( digitalRead(limit0[axis]) == LOW && (currentMicros > previousMicros[axis] + map(input, 0, -250, 5000, 0))) {  // checkt of de limitswitch niet is ingeschakelt en of er genoeg tijd is verstreken sinds de laatste steppuls afhankelijk van de snelheid
      // geeft een puls aan de steppin
      digitalWrite(StepPin[axis], HIGH);
      delayMicroseconds(1);
      digitalWrite(StepPin[axis], LOW);

      previousMicros[axis] = micros(); // slaat op wanneer voor het laatst een stap is gezet
    } 
    
  }
  
}

// de functie wordt gebruikt met: ReadJoysticks();
// als de funtie wordt gebruikt worden de joysticks uitgelezen en is een variabele gezet: stickInputs[x, y, y2/z, x2]
int ReadJoysticks() {  // leest joysticks uit en zet de getallen om in een ander berijk aan getallen
  // meet de pinnen van de joysticks en zet de getallen om in getallen van -255 tot 255
  // als de input dicht bij 0 zit wordt het veranderd naar 0 zodat er niet onjuist beweging wordt gedetecteerd
  stickInputs[0] = map(analogRead(xLstick), 0, 1023, 255, -255);
  if (stickInputs[0] < 5 && stickInputs[0] > -5) { stickInputs[0] = 0; }
  stickInputs[1] = map(analogRead(yLstick), 0, 1023, -255, 255);
  if (stickInputs[1] < 5 && stickInputs[1] > -5) { stickInputs[1] = 0; }
  stickInputs[2] = map(analogRead(yRstick), 0, 1023, 255, -255);
  if (stickInputs[2] < 5 && stickInputs[2] > -5) { stickInputs[2] = 0; }
}

void demo() {
  Serial.println("DemoMode");
  digitalWrite(DemoLed, HIGH);
  int direction[] = {-250, 250, -250}; //richting: -255 is links, 255 is rechts; //slaat op hoe snel en in welke richting de assen bewegen
  digitalWrite(Sleep[0], HIGH);
  digitalWrite(Sleep[1], HIGH);
  digitalWrite(Sleep[2], HIGH);
  while (true) {
    ReadJoysticks();
    

    if (digitalRead(demoButton) == HIGH  && millis > DemoButtonMillis+3000) {DemoButtonMillis = millis(); digitalWrite(DemoLed, LOW); break;} // beindigd de while loop wanneer de demoknop wordt ingedrukt
    if (stickInputs[0] != 0 || stickInputs[1] != 0 || stickInputs[2] != 0) {digitalWrite(DemoLed, LOW); break;} // beindigd de while loop als er met de joysticks wordt bewogen

    for (int axis = 0; axis < 3; axis++) {  // herhaalt de volgende code voor alle assen
      //de richting van de as wordt veranderd als er een limitswitch wordt ingedrukt
      if (digitalRead(limit0[axis]) == LOW) {direction[axis] = 250;}
      if (digitalRead(limit1[axis]) == LOW) {direction[axis] = -250;}

      StepPulse(axis, direction[axis]); //er wordt een stap gezet
      Serial.print(axis); Serial.println(direction[axis]);
    }

  }
}
void loop() {
  if (digitalRead(demoButton) == HIGH && millis > DemoButtonMillis+3000) {DemoButtonMillis = millis(); demo();} // roept demofunctie aan wanneer de demoknop wordt ingedrukt

  
  ReadJoysticks();
  for (int axis = 0; axis < 3; axis++) {  // herhaalt de volgende code voor alle assen
    if (stickInputs[axis] != 0) {         // als de input niet nul is wordt slaapmodus uitgezet en de steppulse functie aangeroepen
      //Serial.print(axis); Serial.println(stickInputs[axis]);
      digitalWrite(Sleep[axis], HIGH);
      StepPulse(axis, stickInputs[axis]);
    } else {  // als de input nul is wordt slaapmodus aangezet en worden de leds van de as uitgezet
      digitalWrite(Sleep[axis], LOW);
      analogWrite(leds0[axis], 0);
      analogWrite(leds1[axis], 0);
    }
  }
}