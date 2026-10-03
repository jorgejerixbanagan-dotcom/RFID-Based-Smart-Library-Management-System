#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_1 10
#define RST_1 9
#define SS_2 8
#define RST_2 7

#define RED_LED 4
#define BUZZER 5      
#define BUZZER2 6     

LiquidCrystal_I2C lcd(0x27, 16, 2); 

byte BOOK_UID[] = {0xc0, 0x68, 0x91, 0x22};
bool bookBorrowed = false;

MFRC522 rfid1(SS_1, RST_1);
MFRC522 rfid2(SS_2, RST_2);

void setup() {
  Serial.begin(9600);
  SPI.begin();

  pinMode(SS_1, OUTPUT);
  pinMode(SS_2, OUTPUT);
  digitalWrite(SS_1, HIGH);
  digitalWrite(SS_2, HIGH);

  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BUZZER2, OUTPUT);

  lcd.init();
  lcd.backlight();
  resetLCD();

  Serial.println(F("==========================================="));
  Serial.println(F("        SMART LIBRARY SYSTEM READY        "));
  Serial.println(F("==========================================="));

  rfid1.PCD_Init();
  rfid2.PCD_Init();
}

void loop() {
  checkReader(rfid1, SS_1, "SELF-CHECKOUT");
  checkReader(rfid2, SS_2, "EXIT GATE");
  delay(50);
}

void checkReader(MFRC522 &reader, int ssPin, const char* label) {
  digitalWrite(ssPin, LOW);

  if (reader.PICC_IsNewCardPresent() && reader.PICC_ReadCardSerial()) {

    Serial.println();
    Serial.println(F("==========================================="));
    Serial.print(F("[ ")); Serial.print(label); Serial.println(F(" ]"));
    Serial.print(F("UID: "));
    for (byte i = 0; i < reader.uid.size; i++) {
      if (reader.uid.uidByte[i] < 0x10) Serial.print("0");
      Serial.print(reader.uid.uidByte[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    if (strcmp(label, "SELF-CHECKOUT") == 0) {
      if (isBook(reader.uid.uidByte, reader.uid.size)) {
        bookBorrowed = !bookBorrowed;
        if (bookBorrowed) {
          Serial.println(F("Book Status: BORROWED"));
          Serial.println(F("Book successfully borrowed!"));

          lcd.clear();
          lcd.setCursor(0, 0); lcd.print("   SUCCESS!   ");
          lcd.setCursor(0, 1); lcd.print("   BORROWED   ");
          tone(BUZZER2, 1200, 500);

        } else {
          Serial.println(F("Book Status: ON SHELF"));
          Serial.println(F("Book successfully returned!"));

          lcd.clear();
          lcd.setCursor(0, 0); lcd.print("   SUCCESS!   ");
          lcd.setCursor(0, 1); lcd.print("   RETURNED   ");
          tone(BUZZER2, 900, 500);
        }
        
        delay(2000);
        resetLCD();

      } else {
        Serial.println(F("Book Status: UNKNOWN"));

        lcd.clear();
        lcd.setCursor(0,0); lcd.print(" TRY AGAIN! ");
        lcd.setCursor(0,1); lcd.print("  UNKNOWN   ");
        tone(BUZZER2, 600, 500);

        delay(2000);
        resetLCD();
      }
    }
    else if (strcmp(label, "EXIT GATE") == 0) {

      if (isBook(reader.uid.uidByte, reader.uid.size)) {
        Serial.print(F("Book Status: "));
        Serial.println(bookBorrowed ? F("BORROWED") : F("ON SHELF"));

        if (!bookBorrowed) {
          Serial.println(F("ALARM TRIGGERED! Book is ON SHELF"));
          digitalWrite(RED_LED, HIGH); 
          tone(BUZZER, 1000); 
          delay(3000); 
          noTone(BUZZER); 
          digitalWrite(RED_LED, LOW);
        } else {
          Serial.println(F("Clear. Book is BORROWED"));
          noTone(BUZZER);
        }
      } else {
        Serial.println(F("Book Status: UNKNOWN"));
      }
    }

    Serial.println(F("==========================================="));
    reader.PICC_HaltA();
    delay(200);
  }

  digitalWrite(ssPin, HIGH);
}

bool isBook(byte *buffer, byte size) {
  if (size != 4) return false;
  for (byte i = 0; i < size; i++) {
    if (buffer[i] != BOOK_UID[i]) return false;
  }
  return true;
}

void resetLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" SELF-CHECKOUT ");
  lcd.setCursor(0, 1);
  lcd.print("   TAP BOOK   ");
}
