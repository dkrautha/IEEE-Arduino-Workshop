constexpr uint8_t BUTTON_ONE_PIN = A0;
constexpr uint8_t BUTTON_TWO_PIN = A1;
constexpr uint8_t BUTTON_THREE_PIN = A2;
constexpr uint8_t START_GAME_BUTTON_PIN = A3;

constexpr uint8_t LED_ONE_PIN = 2;
constexpr uint8_t LED_TWO_PIN = 3;
constexpr uint8_t LED_THREE_PIN = 4;

struct ReadSequenceResult {
  int level;
  int velocity;
};


void setup() {
  pinMode(BUTTON_ONE_PIN, INPUT);
  pinMode(BUTTON_TWO_PIN, INPUT);
  pinMode(BUTTON_THREE_PIN, INPUT);
  pinMode(START_GAME_BUTTON_PIN, INPUT);

  pinMode(LED_ONE_PIN, OUTPUT);
  pinMode(LED_TWO_PIN, OUTPUT);
  pinMode(LED_THREE_PIN, OUTPUT);

  set_all_leds(LOW);
  Serial.begin(9600);
}

constexpr int MAX_LEVEL = 100;
constexpr int DEFAULT_VELOCITY = 1000;
constexpr int DEFAULT_LEVEL = 1;

void loop() {
  uint8_t sequence[MAX_LEVEL] = { 0 };
  int velocity = DEFAULT_VELOCITY;
  int level = DEFAULT_LEVEL;

  while (true) {
    if (level == DEFAULT_LEVEL) {
      generate_sequence(sequence);
    }
    if (digitalRead(START_GAME_BUTTON_PIN) == HIGH || level != 1) {
      // don't check for going over the max level limit here, it's handled in right_sequence()
      show_sequence(sequence, level, velocity);
      ReadSequenceResult res = read_sequence(sequence, level, velocity);
      level = res.level;
      velocity = res.velocity;
    }
  }
}

void set_all_leds(const uint8_t state) {
  digitalWrite(LED_ONE_PIN, state);
  digitalWrite(LED_TWO_PIN, state);
  digitalWrite(LED_THREE_PIN, state);
}

void generate_sequence(uint8_t sequence[]) {
  randomSeed(millis());
  for (int i = 0; i < MAX_LEVEL; i += 1) {
    sequence[i] = random(LED_ONE_PIN, LED_THREE_PIN + 1);
  }
}

void show_sequence(const uint8_t sequence[], const int level, const int velocity) {
  set_all_leds(LOW);
  for (int i = 0; i < level; i += 1) {
    const int pin_to_flash = sequence[i];
    digitalWrite(pin_to_flash, HIGH);
    delay(velocity);
    digitalWrite(pin_to_flash, LOW);
    delay(200);
  }
}


ReadSequenceResult
read_sequence(const uint8_t sequence[], const int level, const int velocity) {
  for (int i = 0; i < level; i += 1) {
    while (true) {
      int chosen_led = check_buttons();
      if (chosen_led < 0) {
        continue;
      }
      digitalWrite(chosen_led, HIGH);
      delay(200);
      if (chosen_led == sequence[i]) {
        digitalWrite(chosen_led, LOW);
        break;
      }
      return wrong_sequence();
    }
  }
  return right_sequence(level, velocity);
}

ReadSequenceResult wrong_sequence() {
  for (int i = 0; i < 3; i += 1) {
    set_all_leds(HIGH);
    delay(250);
    set_all_leds(LOW);
    delay(250);
  }

  return ReadSequenceResult{ .level = DEFAULT_LEVEL, .velocity = DEFAULT_VELOCITY };
}

ReadSequenceResult right_sequence(const int level, const int velocity) {
  set_all_leds(LOW);
  delay(250);
  set_all_leds(HIGH);
  delay(500);
  set_all_leds(LOW);
  delay(500);
  int new_level = level;
  if (level < MAX_LEVEL) {
    new_level += 1;
  }
  return ReadSequenceResult{ .level = new_level, .velocity = velocity - 50 };
}

// returns -1 to indicate no buttons were pressed
int check_buttons() {
  if (digitalRead(BUTTON_ONE_PIN) == HIGH) {
    return LED_ONE_PIN;
  }
  if (digitalRead(BUTTON_TWO_PIN) == HIGH) {
    return LED_TWO_PIN;
  }
  if (digitalRead(BUTTON_THREE_PIN) == HIGH) {
    return LED_THREE_PIN;
  }
  return -1;
}