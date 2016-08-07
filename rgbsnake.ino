#define BTN_R 2
#define BTN_G 3
#define BTN_B 4

struct Button {
  Button(uint8_t pin) : pin(pin), state(0), mark(0) { pinMode(pin, INPUT_PULLUP); }
  uint8_t pin;
  uint8_t state;
  uint16_t mark;
  bool handle();
};

struct Channel {
  Channel(Button b, uint8_t pin) : b(b), pin(pin), level(0), state(0), counter(0), mark(0) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }
  Button b;

  uint8_t pin, level;
  volatile uint8_t state, counter;
  uint16_t mark;
  void loop();
  void tick() {
    switch(state) {
      case 0: if(level==0) { return; }; counter = (1<<level-1); state=1; digitalWrite(pin, LOW);
      case 1: if(!counter) state=2; else { counter--; break; }
      case 2: counter = 256-(1<<level); state=3; digitalWrite(pin, HIGH);
      case 3: if(!counter) state=0; else counter--; break;
    }
  }
};

Button btn_red = Button(BTN_R), btn_green = Button(BTN_G), btn_blue = Button(BTN_B);
Channel red(btn_red, 12), blue(btn_blue, 10), green(btn_green, 11);

ISR(TIMER1_COMPA_vect){
  red.tick();
  green.tick();
  blue.tick();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Setup complete.");

  TCCR1A = 0;
  TCCR1B = _BV(WGM12) | _BV(CS10);
  OCR1AL = 255;
  OCR1AH = 0;
  TIMSK1 |= _BV(OCIE1A);
}

void loop() {
  red.loop();
  green.loop();
  blue.loop();
}

bool Button::handle() {
  uint8_t btn = !digitalRead(pin);
  switch(state) {
    case 0:
      if(btn) { state=1; mark = millis(); }
      return 0;
    case 1:
      if(!btn) { state = 0; return 0; }
      if(mark + 100 <= millis()) {
        state = 2;
        return 1;
      }
      return 0;
     case 2:
      if(!btn) { state = 0; }
      return 0;
  }
}

void Channel::loop() {
  uint16_t time = millis();
  if(b.handle()) {
    Serial.println("press");
    level = (level+1)%9;
    Serial.print(" "); Serial.println(level);
  }
}
