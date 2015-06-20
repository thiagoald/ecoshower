/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the Uno and
  Leonardo, it is attached to digital pin 13. If you're unsure what
  pin the on-board LED is connected to on your Arduino model, check
  the documentation at http://arduino.cc

  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
 */


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);  // ESCOLHER ALGUM PINO DIGITAL ONDE O 'IN1' DO RELAY VAI SER LIGADO
                        // NÃO PRECISA SER O PINO 13
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(13, HIGH);   // COLOCA 5V NA SAÍDA DO PINO 13 (ABRE A VÁLVULA)
  delay(1000);              // DÁ UM INTERVALO DE 1000 ms (1 s) MANTENDO A VÁLVULA ABERTA
  digitalWrite(13, LOW);    // COLOCA GND NA SAÍDA DO PINO 13 (FECHA A VÁLVULA)
  delay(1000);              // DÁ UM INTERVALO DE 1000 ms (1 s) MANTENDO A VÁLVULA FECHADA
}