#define E 659 // Hz
#define C 523 // Hz
#define G 784 // Hz
#define g 392 // Hz
#define R 0 // Rest, play no sound
int song[] = {E, R, E, R, R, E, R, R, C, R, E, R, R, G, R, R, R, R, R, g, R};
int note = 0;
boolean light = false;

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
  taskList[0] = (TCBstruct) {Task1, "LED Blinker", 1, READY, 0, 0};
  taskList[1] = (TCBstruct) {Task2, "Mario Song", 2, READY, 0, 0};
  taskList[2] = (TCBstruct) {schedule_sync, "Sync", 3, READY, 0, 0};
  count = 3;

  // Task 1 set up
  DDRL |= 1 << DDL0; // set pin 49
  PORTL &= ~(1 << PORTL0); // initial off

  // Task 2 set up
  DDRH |= (1 << PH3); // set pin 6

  TCCR4A = 0;
  TCCR4B = 0;
  TCCR4B |= 1 << WGM42; // CTC Mode
  TCCR4A |= 1 << COM4A0; // toggle pin 6
  TCCR4B |= 1 << CS41; // prescaler 8

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


