// defining constants for the pins the buttons are connected to
// there's no specific reason for using the analog pins, I just wanted them grouped away from the other pins
constexpr uint8_t BUTTON_ONE_PIN = A0;
constexpr uint8_t BUTTON_TWO_PIN = A1;
constexpr uint8_t BUTTON_THREE_PIN = A2;
constexpr uint8_t START_GAME_BUTTON_PIN = A3;

// defining constants for the pins the leds are connected to
constexpr uint8_t LED_ONE_PIN = 2;
constexpr uint8_t LED_TWO_PIN = 3;
constexpr uint8_t LED_THREE_PIN = 4;

// constexpr is a C++ keyword (Arduino uses C++, not just C)
// it indicates to the compiler that something can be computed at compile time
// and won't take up any space in the final binary
// I could've just used the const keyword, but this makes my intention more explicit
// I also could've used the preprocessor: #define VAR_NAME val
// I've explicitly chosen NOT to use the preprocessor however, because then you lose
// type safety as the preprocessor only copies and pastes things

// defining the struct types I want to use before any functions
// in normal C/C++ you can put these mostly anywhere, but because of how the Arduino
// compiler transforms .ino files to proper C++ this is required
// this placement specifically remedies when the Arduino compiler writes function
// declarations for your functions and sticks them at the top of the file
struct ReadSequenceResult {
  int level;
  int velocity;
};

// the setup function is the first thing that the gets run on the Arduino
// this is only run a single time, so should be used for one-time setup
void setup() {
  // setting the button pins to act as inputs
  pinMode(BUTTON_ONE_PIN, INPUT);
  pinMode(BUTTON_TWO_PIN, INPUT);
  pinMode(BUTTON_THREE_PIN, INPUT);
  pinMode(START_GAME_BUTTON_PIN, INPUT);

  // setting the led pins to act as outputs
  pinMode(LED_ONE_PIN, OUTPUT);
  pinMode(LED_TWO_PIN, OUTPUT);
  pinMode(LED_THREE_PIN, OUTPUT);

  // ensuring all of the led pins have been set to low
  set_all_leds(LOW);
}

// how many levels the game has
constexpr int MAX_LEVEL = 100;
// how quickly each step of the game will be flashed to the player
constexpr int DEFAULT_VELOCITY = 1000;
constexpr int DEFAULT_LEVEL = 1;

// the loop function is automatically run by the Arduino after the setup() function
// is run. it will be continuously be called forever, so your main program logic begins here
void loop() {
  // where the correct sequence will be stored
  // the { 0 } syntax initializer sets every element to 0
  uint8_t sequence[MAX_LEVEL] = { 0 };
  int velocity = DEFAULT_VELOCITY;
  int level = DEFAULT_LEVEL;

  // I like to avoid using global variables as much as possible, so I've treated the loop()
  // function like it was the standard main in C/C++, and written my own permanent loop inside it
  while (true) {
    // continuously generates a random sequence while we're on level one and the game
    // hasn't been started yet
    if (level == DEFAULT_LEVEL) {
      generate_sequence(sequence);
    }
    // if the start button has been pressed or we're not on the default level, run the following
    if (digitalRead(START_GAME_BUTTON_PIN) == HIGH || level != DEFAULT_LEVEL) {
      // show the correct sequence to the player
      show_sequence(sequence, level, velocity);
      // gets the new level and velocity back after reading a sequence from the player
      ReadSequenceResult res = read_sequence(sequence, level, velocity);
      // don't check for going over the max level limit here, it's handled in right_sequence()
      level = res.level;
      velocity = res.velocity;
    }
  }
}

// conveience function for setting all of the leds to the same value
void set_all_leds(const uint8_t state) {
  digitalWrite(LED_ONE_PIN, state);
  digitalWrite(LED_TWO_PIN, state);
  digitalWrite(LED_THREE_PIN, state);
}

// mutates the given sequence (assumed to be MAX_LEVEL elements long)
void generate_sequence(uint8_t sequence[]) {
  // set a random seed based on how many milliseconds it's been since the program began executing
  // ensures we get as random a sequence as possible as a new sequence is generated repeatedly 
  // before the game starts
  randomSeed(millis());
  for (int i = 0; i < MAX_LEVEL; i += 1) {
    // I've assumed the the three led pins are sequential (2, 3, 4)
    // if you change this then this then generating random numbers like this won't work
    // generates a number between [begin, end), hence the +1 at the end
    sequence[i] = random(LED_ONE_PIN, LED_THREE_PIN + 1);
  }
}

// shows the correct sequence to the player, up to the level they're currently on
void show_sequence(const uint8_t sequence[], const int level, const int velocity) {
  set_all_leds(LOW);
  // for each level, writes the chosen pin to high for velocity milliseconds, then sets
  // it back to low before waiting for a delay of 200 millis
  for (int i = 0; i < level; i += 1) {
    const int pin_to_flash = sequence[i];
    digitalWrite(pin_to_flash, HIGH);
    delay(velocity);
    digitalWrite(pin_to_flash, LOW);
    delay(200);
  }
}

// reads a sequence in from the user
ReadSequenceResult
read_sequence(const uint8_t sequence[], const int level, const int velocity) {
  // for each level
  for (int i = 0; i < level; i += 1) {
    // block here, we're waiting for the user to press a button
    while (true) {
      // gets the led of the button the user has pressed (-1 if they haven't)
      // continue to the next iteration of the while loop if the user hasn't pressed a button
      int chosen_led = check_buttons();
      if (chosen_led < 0) {
        continue;
      }
      // if we're here they've pressed a button, light of the corresponding led for 200 millis
      digitalWrite(chosen_led, HIGH);
      delay(200);
      // if it's the correct led to have chosen based on the generated sequence
      if (chosen_led == sequence[i]) {
        // turn off the led and exit this inner loop, which will continue on to the outer loop
        digitalWrite(chosen_led, LOW);
        break;
      }
      // they picked the wrong button, so go the wrong sequence function and return 
      // the new level and velocity it gives us
      return wrong_sequence();
    }
  }
  // if we've gotten here, the user has pressed all of the correct buttons corresponding
  // to the correct sequence, so we return the result of the right sequence function
  return right_sequence(level, velocity);
}

// called when the user picks an incorrect button in the sequence
ReadSequenceResult wrong_sequence() {
  // blink all three leds three times in rapid succession to indicate failure
  for (int i = 0; i < 3; i += 1) {
    set_all_leds(HIGH);
    delay(250);
    set_all_leds(LOW);
    delay(250);
  }

  // return the default level and velocity, resetting the game
  return ReadSequenceResult{ .level = DEFAULT_LEVEL, .velocity = DEFAULT_VELOCITY };
}

// called when the user enters the entire sequence correctly
ReadSequenceResult right_sequence(const int level, const int velocity) {
  // blink all leds for half a second, then wait a half a second
  set_all_leds(LOW);
  delay(250);
  set_all_leds(HIGH);
  delay(500);
  set_all_leds(LOW);
  delay(500);
  int new_level = level;
  // if by some miracle the user gets to level 100, don't increase the new level
  // any further as we don't have any additional parts of the sequence to give them
  if (level < MAX_LEVEL) {
    new_level += 1;
  }
  // decrease velocity by 50 and return the new level and velocity
  return ReadSequenceResult{ .level = new_level, .velocity = velocity - 50 };
}

// returns -1 to indicate no buttons were pressed
int check_buttons() {
  // checks each button pin and returns the corresponding led pin if it was pressed
  // otherwise returns -1
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