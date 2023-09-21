constexpr int TIMED_LED_PIN = LED_BUILTIN;  // the number of the LED pin
constexpr int BUTTON_LED_PIN = 8;
constexpr int BUTTON_PIN = 3;

struct LedTimerState {
  unsigned long previous_time;
  int led_state;
};

enum ButtonState {
  Low = LOW,
  High = HIGH,
};

LedTimerState
update_led(LedTimerState state) {
  constexpr long interval = 1000;

  const unsigned long current_time = millis();

  if (current_time - state.previous_time >= interval) {
    // save the last time you blinked the LED
    state.previous_time = current_time;

    // if the LED is off turn it on and vice-versa:
    if (state.led_state == LOW) {
      state.led_state = HIGH;
    } else {
      state.led_state = LOW;
    }

    digitalWrite(TIMED_LED_PIN, state.led_state);
  }

  return state;
}

void setup() {
  // set the digital pin as output:
  pinMode(TIMED_LED_PIN, OUTPUT);
  pinMode(BUTTON_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
}

void loop() {
  LedTimerState timed_led_state = {
    .previous_time = 0,
    .led_state = LOW,
  };

  while (1) {
    timed_led_state = update_led(timed_led_state);

    ButtonState button_state = static_cast<ButtonState>(digitalRead(BUTTON_PIN));
    switch (button_state) {
      case HIGH:
        digitalWrite(BUTTON_LED_PIN, HIGH);
        break;
      case LOW:
        digitalWrite(BUTTON_LED_PIN, LOW);
        break;
    }
  }
}
