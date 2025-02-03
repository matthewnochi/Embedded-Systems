#include <SevSeg.h> // Include the SevSeg library
SevSeg sevseg;

int counter = 0; // Counter variable
char invertedDisplay[5]; // Buffer to store the inverted number as a string

#define E 659 // Hz
#define C 523 // Hz
#define G 784 // Hz
#define g 392 // Hz
#define R 0   // Rest, play no sound
int song[] = {E, R, E, R, R, E, R, R, C, R, E, R, R, G, R, R, R, R, R, g, R};
int note = 0;
boolean light = false;

enum TaskState {
  READY,
  RUNNING,
  SLEEPING
};

enum FlagState {
  PENDING,
  DONE
};

int delays[10] = {0};
int task_state[10] = {READY, READY, READY};
void (*tasks[10])() = {NULL};
int task_active = 0;

volatile FlagState sFlag;

void sleep(int sleeptime) {
  delays[task_active] = sleeptime; // Convert ms to ticks
  task_state[task_active] = SLEEPING;
}

void setup() {
  Serial.begin(9600);

  // SevSeg setup
  byte numDigits = 4; // Number of digits on the display
  byte digitPins[] = {10, 11, 12, 13}; // Pins controlling each digit
  byte segmentPins[] = {2, 3, 4, 5, 48, 7, 8, 9}; // Pins controlling each segment
  bool resistorsOnSegments = true; // Assuming resistors are on the segments
  byte hardwareConfig = COMMON_CATHODE; // Type of 7-segment display
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments);
  sevseg.setBrightness(90); // Set brightness level

  // SRRI setup
  tasks[0] = Task1;
  tasks[1] = Task2;
  tasks[2] = schedule_sync;
  tasks[3] = Task3;
  tasks[4] = NULL;

  // Task 1 setup
  DDRL |= 1 << DDL0; // set pin 49
  PORTL &= ~(1 << PORTL0); // initial off

  // Task 2 setup
  DDRH |= (1 << PH3); // set pin 6

  TCCR4A = 0;
  TCCR4B = 0;
  TCCR4B |= 1 << WGM42; // CTC Mode
  TCCR4A |= 1 << COM4A0; // toggle pin 6
  TCCR4B |= 1 << CS41;   // prescaler 8
  
  // Interrupt Timer Setup
  TCCR3A = 0;
  TCCR3B = 0;
  TCCR3B |= 1 << WGM32;
  TCCR3B |= 1 << CS31; // prescaler 8
  OCR3A = 4000;  // sets frequency to 2ms
  TIMSK3 |= (1 << OCIE3A); // Timer 3 Compare Match A interrupt

  Serial.println("Setup Complete.");
}

ISR(TIMER3_COMPA_vect) {
  sFlag = DONE;  // Update scheduler flag
}

void loop() {
  while (1) {
    for (task_active = 0; task_active < 10; task_active++) {
      if (tasks[task_active] == NULL) { // End of task list
        break;
      }

      if (task_state[task_active] == READY) {
        task_state[task_active] = RUNNING;
        (*tasks[task_active])();
        if (task_state[task_active] == RUNNING) {
          task_state[task_active] = READY;
        }
      }
    }
    sevseg.refreshDisplay(); // Refresh the display
  }
}

void schedule_sync() {
  sFlag = PENDING;
  while (sFlag == PENDING) { }
  for (int i = 0; i < 10; i++) {
    if (task_state[i] == SLEEPING) {
      delays[i] -= 2;
      if (delays[i] <= 0) {
        task_state[i] = READY;
      }
    }
  } 
}

void Task1() {
  if (light) {
    PORTL &= ~(1 << PORTL0); // Turn off the LED
    light = false;
    sleep(750); // Sleep for 750ms
  } else {
    PORTL |= 1 << PORTL0; // Turn on the LED
    light = true;
    sleep(250); // Sleep for 250ms
  }
  return;
}

void Task2() {
  if (note < sizeof(song) / sizeof(song[0])) {
    if (song[note] == 0) {
      TCCR4A &= ~(1 << COM4A0);
    } else {
      TCCR4A |= 1 << COM4A0;
      OCR4A = 2000000 / (2 * song[note]);
    }
    sleep(100);
  }
  if (note >= sizeof(song) / sizeof(song[0])) {
    TCCR4A &= ~(1 << COM4A0);
    note = 0;
    sleep(4000);
  } else {
    note++;
  }
  return;
}

void Task3() {
  counter = (counter + 1) % 10000;

  snprintf(invertedDisplay, sizeof(invertedDisplay), "%04d", counter); // Zero-pad and convert to string
  reverseString(invertedDisplay);   // Reverse the string
  sevseg.setChars(invertedDisplay); // Display on 7-segment

  sleep(100); // Sync with scheduler
}

// Helper function to reverse a string for Task 3
void reverseString(char* str) {
  int len = strlen(str);
  for (int i = 0; i < len / 2; i++) {
    char temp = str[i];
    str[i] = str[len - i - 1];
    str[len - i - 1] = temp;
  }
}