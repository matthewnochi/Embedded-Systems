#include <SevSeg.h> // Include the SevSeg library
SevSeg sevseg;

int counter = 0; // Counter for sevseg
char invertedDisplay[5]; // Buffer

#define E 659 // Hz
#define C 523 // Hz
#define G 784 // Hz
#define g 392 // Hz
#define R 0 // Rest, play no sound
int song[] = {E, R, E, R, R, E, R, R, C, R, E, R, R, G, R, R, R, R, R, g, R};
// int freq[] = {659, 0, 659, 0, 0 , 659, 0, 0, 523, 0, 659, 0, 0, 784, 0, 0, 0, 0, 0, 392, 0};
int freq[] = {659, 659, 659, 659, 659 , 659, 659, 659, 523, 523, 659, 659, 659, 784, 784, 784, 784, 784, 784, 392, 392};
int note = 0;
boolean done = false;
boolean first = true;

enum TaskState {
  READY,
  RUNNING,
  SLEEPING,
  DEAD
};

enum FlagState {
  PENDING,
  DONE
};

volatile FlagState sFlag;

typedef struct {
  void (*task)(); // the function pointer
  char name[20]; // name
  int id; // unqiue id
  int state; // the task state
  int start; // number of times tasks was called
  int delay; // delay
} TCBstruct;

TCBstruct taskList[10];
TCBstruct deadTasks[10];
int count = 0; 
int numDead = 0;

int task_active = 0;

void sleep(int sleeptime){
  taskList[task_active].state = SLEEPING;
  taskList[task_active].delay = sleeptime;
}

void task_self_quit() {
  taskList[task_active].state = DEAD;
  if (numDead < 10) {
    deadTasks[numDead] = taskList[task_active];
    numDead++;
  }

  for (int i = task_active; i < count - 1; i++) {
    taskList[i] = taskList[i + 1];
  }
  
  count--;
  task_active--;
}

void task_start(TCBstruct *task) {
  if (task->state == DEAD) {
    task->state = READY;
  }
  if (count < 10) {
    taskList[count] = *task;
    count++;
  }
}

void setup() {
  Serial.begin(9600);

  // DDS set up
  taskList[0] = (TCBstruct) {Task4, "Combination", 1, READY, 0, 0};
  taskList[1] = (TCBstruct) {schedule_sync, "Sync", 3, READY, 0, 0};
  count = 2;

  // Task 2 set up
  DDRH |= (1 << PH3); // set pin 6

  TCCR4A = 0;
  TCCR4B = 0;
  TCCR4B |= 1 << WGM42; // CTC Mode
  TCCR4A |= 1 << COM4A0; // toggle pin 6
  TCCR4B |= 1 << CS41; // prescaler 8

  // Task 3 set up
  byte numDigits = 4; // Number of digits on the display
  byte digitPins[] = {10, 11, 12, 13}; // Pins controlling each digit
  byte segmentPins[] = {2, 3, 4, 5, 48, 7, 8, 9}; // Pins controlling each segment
  segmentPins[7] = 3;
  bool resistorsOnSegments = true; // Assuming resistors are on the segment pins
  byte hardwareConfig = COMMON_CATHODE; // Type of 7-segment display
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments);
  sevseg.setBrightness(90); // Set brightness level

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
    for (int i = 0; i < count; i++) {
      if (taskList[i].state == READY) {
        taskList[i].state = RUNNING;
        task_active = i;
        taskList[i].start++;
        taskList[i].task();
      }
      if (taskList[i].state == RUNNING) {
        taskList[i].state = READY;
      }
    }
  }
}

void schedule_sync() {
  sFlag = PENDING;
  while (sFlag == PENDING) { }
  for (int i = 0; i < count; i++) {
    if (taskList[i].state == SLEEPING) {
      taskList[i].delay -= 2;
      if (taskList[i].delay <= 0) {
        taskList[i].state = READY;
      }
    }
  } 
  sevseg.refreshDisplay();
}

void Task4() {
  if (note < sizeof(song) / sizeof(song[0])) {
    done = false;
    if (song[note] == 0) {
      TCCR4A &= ~(1 << COM4A0);
    } else {
      TCCR4A |= 1 << COM4A0;
      OCR4A = 2000000 / (2 * song[note]);
    }
    sleep(100);
    counter = freq[note];
    first = true;
  }
  if (note >= sizeof(song) / sizeof(song[0]) && first) {
    counter = 41;
    done = true;
    first = false;
  } 
  note++;
  if (done) {
    counter--;
    sleep(100);
  } 
  if (done && counter <= 0) {
    note = 0;
    done = false;
  }
  snprintf(invertedDisplay, sizeof(invertedDisplay), "%04d", counter); // Zero-pad and convert to string
  reverseString(invertedDisplay);   // Reverse the string
  sevseg.setChars(invertedDisplay); // Display on 7-segment
  return;
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