#include "LedControl.h"
#include "queue.h"
/*
 pin 12 is connected to the DataIn 
 pin 11 is connected to LOAD(CS)
 pin 10 is connected to the CLK 
 We have only a single MAX72XX.
 */
LedControl lc = LedControl(12,10,11,1);

struct Point {
  char x;
  char y;
  Point() {
    x = y = 0;
  }
  Point(char X, char Y) {
    x = X;
    y = Y;
  }
};

Point apple = Point();

Point dirs[] = { Point(1,0), Point(0,-1), Point(-1,0), Point(0,1) };
enum DIRS { RIGHT, UP, LEFT, DOWN } dir;

byte generalDir = DOWN;
byte width = 8;
byte height = 8;
Queue<Point> snake(width * height);


void initScreen() {
  lc.shutdown(0,false);
  lc.setIntensity(0, 8);
}
void clearScreen() {
  lc.clearDisplay(0);
}
void setPixel(byte row, byte col, bool val) {
  lc.setLed(0, row, col, val);
}

void initSnake() {
  snake.enqueue(Point(0,0));
  setPixel(0, 0, true);
  generateApple();
  dir = RIGHT;
  generalDir = DOWN;
}

void resetSnake() {
  while(!snake.isEmpty()) {
    snake.dequeue();
  }
  clearScreen();
  initSnake();
}

bool generateApple() {
  if (snake.itemCount() >= width*height) {
    return false;
  }
  do {
    apple.x = random(width);
    apple.y = random(height);
  } while (isInSnake(apple.x, apple.y));
  setPixel(apple.y, apple.x, true);
  return true;
}

bool isSnakeOnApple() {
  Point *point = &snake.tail->item;
  return point->x == apple.x && point->y == apple.y;
}

bool isInSnake(byte x, byte y) {
  // should use a hash table here
  Queue<Point>::Node* node = snake.head;
  while (node != nullptr) {
    Point *point = &node->item;
    if (x == point->x && y == point->y) {
      return true;
    }
    node = node->next;
  }
  return false;
}


void chooseDir() {
  Point *point = &snake.tail->item;
  byte x = point->x;
  byte y = point->y;

  bool appleAtEdge = (apple.x == 0 || apple.x == width - 1);

  if (y == 0) {
    generalDir = DOWN;
  } else if (y == height - 1) {
    generalDir = UP;
  }
  
  if (generalDir == UP) {
    if (x == 0) {
      if (y == 0 || (!(y & 1) && !appleAtEdge && (apple.y == y || apple.y == y-1))) {
        dir = RIGHT;
      } else {
        dir = UP;
      }
    } else if (!(y & 1) && !((apple.y == y || apple.y == y-1) && !appleAtEdge && apple.x > x)) {
      dir = UP;
    } else {
      dir = dir == RIGHT ? RIGHT : LEFT;
    }
  } else {
    if (x == width - 1) {
      if (y == height - 1 || ((y & 1) && !appleAtEdge && (apple.y == y || apple.y == y+1))) {
        dir = LEFT;
      } else {
        dir = DOWN;
      }
    } else if ((y & 1) && !((apple.y == y || apple.y == y+1) && !appleAtEdge && apple.x < x)) {
      dir = DOWN;
    } else {
      dir = dir == LEFT ? LEFT : RIGHT;
    }
  }
}

bool moveForward() {
  Point *point = &snake.tail->item;
  
  byte x = point->x + dirs[dir].x;
  byte y = point->y + dirs[dir].y;

  if (!(0 <= x && x < width && 0 <= y && y < height && !isInSnake(x, y))) {
    return false;
  }
  snake.enqueue(Point(x, y));

  return true;
}

void removeTail() {
  Point *point = &snake.tail->item;
  setPixel(point->y, point->x, true);
  point = &snake.head->item;
  setPixel(point->y, point->x, false);
  snake.dequeue();
}

void setup() {
  // random
  randomSeed(analogRead(0));
  
  initScreen();

  initSnake();

  delay(1000);
}

void loop() {
  chooseDir();
  if (!moveForward()) {
    resetSnake();
  } else {
    if (isSnakeOnApple()) {
      if (!generateApple()) {
        resetSnake();
      }
    } else {
      removeTail();
    }
  }

  delay(20);
}
