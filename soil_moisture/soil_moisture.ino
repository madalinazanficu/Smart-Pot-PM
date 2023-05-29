#include <LiquidCrystal_I2C.h>
#include "matrix_animation.h"
#define sensor_read A0
#define relay_write 7

// Variabile pentru timer - overflow normal 860
const int overflows = 10;
int timer_counter = 0;
bool check_plant = false;

// Creez obiectul ecran de 16 caractere pe 2 linii la adresa 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {

  // Initializare comunicatie seriala
  Serial.begin(9600);

  // Setare pini de comunicare cu alte module
  pinMode(relay_write, OUTPUT);
  digitalWrite(relay_write, LOW);

  // Set-up for Timer
  timer_setup();

  // Set-up for the LCD
  lcd.init();

  // Set-up for Matrix
  matrix_setup();
}


void matrix_setup() {
  FastLED.addLeds<WS2812B, PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(100); //Number 0-255
  FastLED.clear();
}

void timer_setup() {
  // Dezactiveaza intreruperi la nivel global
  cli();
  // Setează Timer1 pentru modul de funcționare CTC
  TCCR1A = 0;                       

  // Setează prescaler la 1024
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);

  // Setează valoarea OCR1A la valoarea maxima posibila pe 16 biti 
  OCR1A = 65535;

  // 3 secunde cu prescaler 1024
  OCR1B = 49151;

  // Activare întrerupere TIMER1_COMPA
  TIMSK1 |= (1 << OCIE1A);

  // Activare întreruperi la nivel global
  sei();
}


/*
  Implementare rutină de tratare a întreruperii TIMER1_COMPA
  Fiecare rutina este apelata cand TCNT0 ajunge la valoarea maxima pe 16 biti
  Acest lucru nu permite numararea pana la 3600 de secunde, astfel avem nevoie de 860 astfel de numarari
  Pentru a verifica starea plantei
*/
ISR(TIMER1_COMPA_vect) {
 
  timer_counter++;
  if (timer_counter == overflows) {
    check_plant = true;
    timer_counter = 0;
  }
}

void show_on_lcd(int humidity_level) {
  String moisture = String(humidity_level) + "%";
  lcd.clear();
  lcd.backlight();

  if (humidity_level < 65) {
    lcd.setCursor(1,0);
    lcd.print("Soil Moisture:");
    lcd.setCursor(7, 1);
    lcd.print(moisture);

  } else if (humidity_level >= 60 && humidity_level < 80) {
    lcd.setCursor(1,0);
    lcd.print("Your plant has");
    lcd.setCursor(0, 1);
    lcd.print("enough water " + moisture);

  } else {
    lcd.setCursor(1,0);
    lcd.print("Too much water!");
    lcd.setCursor(7, 1);
    lcd.print(moisture);

  }
}

/* 
  Pompa este controlata de un releu ce lucreaza in logica pozitiva
  Se comanda releu pe pinul digital 7.
  Daca nivelul de umiditate al solului este mai mic decat valoarea de threshold.
*/
void command_relay(int humidity_level) {
  int threshold = 80;

  if (humidity_level < threshold) {
    digitalWrite(relay_write, HIGH);
    delay(3000);
    digitalWrite(relay_write, LOW);
  }
}


/* 
  Comunicatia cu modulul ESP este realizata cu ajutorul interfetei seriale (USART).
*/
void communication_ESP(int humidity_level) {
  Serial.println(String(humidity_level));
}


void loop() {
  int soil_moisture = 0;
  int humidity_level = 0;

  // Cand timerul a ajuns la o ora sa iau masuratori
  if (check_plant == true) {

    // Se citeste valoarea de pe senzor (valoare intre 0 - 1024)
    soil_moisture = analogRead(sensor_read);
    
    // Se calculeaza procentul de umiditate in logica inversa
    humidity_level = map(soil_moisture, 1024, 0, 0, 100);

    check_plant = false;
    show_on_lcd(humidity_level);
    command_relay(humidity_level);
    communication_ESP(humidity_level);
  }

  matrix_animation();
}
