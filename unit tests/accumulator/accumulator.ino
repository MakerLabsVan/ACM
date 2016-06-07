/*
  ACCUMULATOR

  Circuit:
    Pull-up resistor between ground and node with control signal and digital pin 2.
  
  Input:
    Control signal to Pin 2.

  Output:
    Elapsed time that control signal was HIGH for.

  Operation:
    Button state signal is read continuously. Timer activates by a LOW to HIGH signal,
    deactivates by a HIGH to LOW signal, and prints elapsed time upon deactivation.

  Notes:
    Millis() will overflow at 50 days, resulting in an error in elapsed time calculation.
    LED functionality added for debugging purposes has been commented out.
  
*/

const int ledPin =  13;      // the number of the LED pin
const int buttonPin = 2;     // the number of the pushbutton pin
int buttonState = 0;
int previous_state = 0;
unsigned long start_time = 0;
unsigned long elapsed_time = 0;

void setup() {
  //initialize serial communication and botton pin
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
}

void loop() {
  // read state
  buttonState = digitalRead(buttonPin);
  digitalWrite(ledPin, LOW);
  
  // check for new button press
  if (previous_state == LOW && buttonState == HIGH) {
    start_time = millis();
    previous_state = buttonState;
    //digitalWrite(ledPin, HIGH);
    } 
    
  // check for button release
  else if (previous_state == HIGH && buttonState == LOW) {
    elapsed_time = (millis() - start_time)/1000;
    
    //ignore extraneous zeros (due to bouncing?) and print
    if (elapsed_time != 0) Serial.println(elapsed_time);
    
    previous_state = buttonState;
  }
  
  // no event
  else {
      //digitalWrite(ledPin, LOW);
      previous_state = buttonState;;
    }
}
