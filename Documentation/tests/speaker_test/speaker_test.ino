const int NOTE_B = 988;
const int NOTE_E = 1319;
const int PIN_SPEAKER = 8;

int coinNotes[] = { NOTE_B, NOTE_E };
int coinNoteDurations[] = { 125, 400 };
int numCoinNotes = sizeof(coinNotes) / sizeof(coinNotes[0]);

void setup() {
  pinMode(PIN_SPEAKER, OUTPUT);
}

void loop() {
  for (int i = 0; i < numCoinNotes; i++) {
    tone(PIN_SPEAKER, coinNotes[i]);
    delay(coinNoteDurations[i]);
    noTone(8);
  }
  delay(5000);
}
