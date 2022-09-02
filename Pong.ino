#include <U8g2lib.h>
#include <U8x8lib.h>
#include <U8x8lib.h>
#include <Arduino_SensorKit_BMP280.h>
#include <Arduino_SensorKit_LIS3DHTR.h>
#include "Arduino_SensorKit.h"

#define BUZZER 5

// establish pin numbers
int racket1DownPin = 10;
int racket1UpPin = 11;
int racket2DownPin = 8;
int racket2UpPin = 9;

// u8x8 tiles for rackets, ball and empty (just a blank tile used when moving the rackets/ball)
uint8_t racket1Tile[16] = {0x0, 0x0, 0x0, 0x0, 0xff, 0xff, 0xff, 0xff, 255, 255, 255, 255, 255, 255, 255, 255};
uint8_t racket2Tile[16] = {0xff, 0xff, 0xff, 0xff, 0x0, 0x0, 0x0, 0x0, 255, 255, 255, 255, 255, 255, 255, 255};
uint8_t emptyTile[16] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t ballTile[16] = {0x0, 0x0, 0x0f, 0x0f, 0x0f, 0x0f, 0x0, 0x0, 100, 100, 100, 100, 100, 100};

// racket struct, keeps track of racket coordinates, points, which racket is 'active' (which racket hit the ball last) and which racket is the winner
struct {
  int x;
  int y;
  int points = 0;
  bool active = false;
  bool winner = false;
} racket1, racket2;

// ball struct, keeps track of ball coordinates and the direction vector of the ball
struct {
  int coordinates[2] = {8, 3};
  // direction vector: [1/0, x]
  // [right/left, slope]
  // ex: [1, 0] ball is moving right in a straight line
  // [0, 2] ball is moving left in the upwards direction with a slope of 2x
  int direction[2] = {1, 0};
} ball;

// game struct, keeps track of when a new game starts and when a new round starts
struct {
  bool start;
  bool newRound;
} game;



void setup() {
  // establish buttons as input and buzzer as output
  pinMode(racket1DownPin, INPUT);
  pinMode(racket1UpPin, INPUT);
  pinMode(racket2DownPin, INPUT);
  pinMode(racket2UpPin, INPUT);
  pinMode(BUZZER, OUTPUT);

  Serial.begin(9600); // serial monitor is used for textual game state updates

  // initialize display
  Oled.begin();
  Oled.setFlipMode(true);
  Oled.setFont(u8x8_font_courB18_2x3_f);

  game.start = true; // flag game start

  // set rackets x coordinates which never change
  racket1.x = 0; 
  racket2.x = 15;
}

void loop() {
  // Flag Game Start (displays title and plays a song)
  if(game.start) {
    Serial.println("new game start");

    // reset ball
    ball.coordinates[0] = 8;
    ball.coordinates[1] = 3;

    // reset rackets
    racket1.y = 3;
    racket2.y = 3;
    racket1.points = 0;
    racket2.points = 0;
    racket1.active = false;
    racket1.winner = false;
    racket2.active = false;
    racket2.winner = false;

    // title
    Oled.setCursor(4,2);
    Oled.print("PONG");

    // countdown buzzer
    for(int i = 1; i <= 3; i++) {
      tone(BUZZER, 50*i, 333);
      delay(333);
      tone(BUZZER, 100*i, 333);
      delay(333);
      tone(BUZZER, 150*i, 333);
      delay(333);
    }
    
    // clear screen and start new round
    Oled.clearDisplay();
    game.start = false;
    game.newRound = true;
  }

  // Flag New Round (shows how many points each racket has, draws rackets and displays a countdown):
  if(game.newRound) {
    Serial.println("new round start");

    // display points
    Oled.setCursor(3, 1);
    Oled.print("R1: ");
    Oled.print(racket1.points);
    Oled.setCursor(3, 4);
    Oled.print("R2: ");
    Oled.print(racket2.points);
    delay(1000);
    Oled.clearDisplay();    

    // reset ball
    ball.coordinates[0] = 8;
    ball.coordinates[1] = 3;

    // reset rackets
    racket1.y = 3;
    racket2.y = 3;
    racket1.active = false;
    racket2.active = false;

    // draw rackets
    drawRacket(1);
    drawRacket(2);

    // countdown timer
    for(int i = 3; i > 0; i--) {
      Oled.setCursor(7, 2);
      Oled.print(i);
      delay(500);
      Oled.refreshDisplay();
    }
    
    // clear screen and draw the ball
    Oled.setCursor(7, 2);
    Oled.print(" ");
    Oled.refreshDisplay();
    Oled.drawTile(ball.coordinates[0], ball.coordinates[1], 1, ballTile);
    game.newRound = false;
  }

  // Racket Move Logic:

  // current state of buttons
  int racket1DownState = digitalRead(racket1DownPin);
  int racket1UpState = digitalRead(racket1UpPin);
  int racket2DownState = digitalRead(racket2DownPin);
  int racket2UpState = digitalRead(racket2UpPin);

  // racket 1 move down
  if(racket1DownState == HIGH and racket1.y+1 < 7) {
    int previous = racket1.y;
    racket1.y++;
    moveRacket(1, previous);
    Serial.println("racket 1 moved down");
  }

  // racket 1 move up
  if(racket1UpState == HIGH and racket1.y-1 > 0) {
    int previous = racket1.y;
    racket1.y--;
    moveRacket(1, previous);
    Serial.println("racket 1 moved up");
  }
  
  // racket 2 move down
  if(racket2DownState == HIGH and racket2.y+1 < 7) {
    int previous = racket2.y;
    racket2.y++;
    moveRacket(2, previous);
    Serial.println("racket 2 moved down");
  }

  // racket 2 move up
  if(racket2UpState == HIGH and racket2.y-1 > 0) {
    int previous = racket2.y;
    racket2.y--;
    moveRacket(2, previous);
    Serial.println("racket 2 moved up");
  }

  // Ball Move Logic:

  int lastPos[2] = {ball.coordinates[0], ball.coordinates[1]}; // save current ball coordinates as lastPos
  int dirVector[2] = {ball.direction[0], ball.direction[1]}; // save direction into dirVector (for parameter passing purposes, getNextBallPos() didn't want to work when I passed ball.direction)
  int *nextBallCoord = getNextBallPos(lastPos, dirVector);
  ball.coordinates[0] = *nextBallCoord; // update ball coordinates with next position
  ball.coordinates[1] = nextBallCoord[1]; 
  Oled.drawTile(lastPos[0], lastPos[1], 1, emptyTile); // move ball
  Oled.drawTile(ball.coordinates[0], ball.coordinates[1], 1, ballTile);
  
  // ball hits vertical border, increment points if necessary and flag new round or game over
  if(ball.coordinates[0] == 0 or ball.coordinates[0] == 15) {
    Serial.println("racket miss");
    
    // play sound effect
    tone(BUZZER, 80, 500);
    delay(500);
    tone(BUZZER, 50, 500);
    delay(500);
    Oled.clearDisplay();

    game.newRound = true; // flag new round

    // award points
    if(racket1.active) {
      racket1.points++;
      if(racket1.points == 6) {
        Serial.println("racket 1 wins");
        racket1.winner = true; // set winner
        game.newRound = false; // negate new round because game is over
        gameOver();
        return; // return so loop doesn't continue running after gameOver(), if player chooses to play again loop() will be called
      }
    }
    else if(racket2.active) {
      racket2.points++;
      if(racket2.points == 6) {
        Serial.println("racket 2 wins");        
        racket2.winner = true;
        game.newRound = false;
        gameOver();
        return;
      }
    }
  }

  // collision with racket 1, change slope and direction
  if(ball.coordinates[0] == racket1.x+1 and (ball.coordinates[1] >= racket1.y-1) and (ball.coordinates[1] <= racket1.y+1)) {
    tone(BUZZER, 30, 100);
    ball.direction[0] = 1; // change direction
    ball.direction[1] = racket1.y - ball.coordinates[1]; // slope = 3(rackety - bally)/racketheight but since racketheight = 3, slope = rackety - bally
    racket1.active = true; // switch active racket
    racket2.active = false;
    Serial.println("ball hit racket 1");
  }
  
  // collision with racket 2, change slope and direction
  if(ball.coordinates[0] == racket2.x-1 and (ball.coordinates[1] >= racket2.y-1) and (ball.coordinates[1] <= racket2.y+1)) {
    tone(BUZZER, 30, 100);
    ball.direction[0] = 0;    
    ball.direction[1] = racket2.y - ball.coordinates[1];
    racket2.active = true;
    racket1.active = false;
    Serial.println("ball hit racket 2");
  }

  // collision with boundary, reverse slope
  if(ball.coordinates[1] == 0 or ball.coordinates[1] == 7) {
    tone(BUZZER, 30, 100);
    ball.direction[1] = -ball.direction[1];
    Serial.println("ball hit upper/lower bound");
  }

  // game movement rate
  delay(60);
}

// takes current ball pos and direction vector and returns array with next ball pos [x, y]
int getNextBallPos(int *currentPos, int *dirVector) {
  // calculate next x position
  int newX;
  if(*dirVector == 1) { newX = *currentPos + 1; }
  else if(*dirVector == 0) { newX = *currentPos - 1; }
  // calculate next y position
  int newY;
  if(dirVector[1] == 0) { newY = currentPos[1]; } // if slope is 0, y stays the same
  if(dirVector[1] > 0) { newY = currentPos[1]-1; } // if slope is 1, y decreases (display is bottom-up so to go 'up' y must decrease)
  if(dirVector[1] < 0) { newY = currentPos[1]+1; } // if slope is -1 y increases
  int next[2] = {newX, newY}; // store new x and y in array
  int *nextPos; 
  nextPos = &next[0]; // create pointer to address of next array, we'll return the address so that loop() can reference that array
  int address = nextPos;
  // DO NOT REMOVE THESE PRINT STATEMENTS!!
  Serial.println(address);    // I have absolutely no idea why, but this code literally doesn't work without them. If they're not there, this function returns incorrect output and breaks everything.
  Serial.println(*nextPos);   // I'm not sure what they're doing in the backend, I've spent hours trying to figure it out, I couldn't, so I've resolved to just keeping these here.
  return nextPos;
}

// draw next racket tile (up or down) and replace the one left behind with empty tile
void moveRacket(int racketNum, int previousPosition) {
  // racket 1
  if(racketNum == 1) {
    // racket moved up
    if(previousPosition > racket1.y) {
      Oled.drawTile(racket1.x, racket1.y-1, 1, racket1Tile);
      Oled.drawTile(racket1.x, racket1.y+2, 1, emptyTile);
    }
    // racket moved down
    else {
      Oled.drawTile(racket1.x, racket1.y+1, 1, racket1Tile);
      Oled.drawTile(racket1.x, racket1.y-2, 1, emptyTile);
    }
  }
  // racket 2
  else {
    // racket moved up
    if(previousPosition > racket2.y) {
      Oled.drawTile(racket2.x, racket2.y-1, 1, racket2Tile);
      Oled.drawTile(racket2.x, racket2.y+2, 1, emptyTile);
    }
    // racket moved down
    else {
      Oled.drawTile(racket2.x, racket2.y+1, 1, racket2Tile);
      Oled.drawTile(racket2.x, racket2.y-2, 1, emptyTile);
    }
  }
}

// draws the specified racket
void drawRacket(int racketNum) {
  if(racketNum == 1) {
    Oled.drawTile(racket1.x, racket1.y-1, 1, racket1Tile);
    Oled.drawTile(racket1.x, racket1.y, 1, racket1Tile);
    Oled.drawTile(racket1.x, racket1.y+1, 1, racket1Tile);
  }
  else if(racketNum == 2) {
    Oled.drawTile(racket2.x, racket2.y-1, 1, racket2Tile);
    Oled.drawTile(racket2.x, racket2.y, 1, racket2Tile);
    Oled.drawTile(racket2.x, racket2.y+1, 1, racket2Tile);
  }
}

// displays winner to screen, asks user to press button to play again
void gameOver() {
  // display winner
  Oled.setCursor(0, 2);
  if(racket1.winner) { Oled.print("R1 Wins!"); }
  else { Oled.print("R2 Wins!"); }
  delay(2000);
  Oled.clearDisplay();

  // ask user to play again or quit
  Oled.setFont(u8x8_font_7x14_1x2_f); // set smaller font
  Oled.setCursor(0,1);
  Oled.print("left - play");
  Oled.setCursor(0,5);
  Oled.print("right - quit");

  // infinite loop that waits for user to press one of the buttons
  while(true) {
    int racket1DownState = digitalRead(racket1DownPin);
    int racket1UpState = digitalRead(racket1UpPin);
    int racket2DownState = digitalRead(racket2DownPin);
    int racket2UpState = digitalRead(racket2UpPin);
    // player pressed left buttons, play again
    if(racket1DownState == HIGH or racket1UpState == HIGH) {
      Oled.clearDisplay();
      game.start = true; // flag new game
      Oled.setFont(u8x8_font_courB18_2x3_f); // reset font
      loop(); 
    }
    // player pressed right buttons, quit
    else if(racket2DownState == HIGH or racket2UpState == HIGH) {
      Oled.clearDisplay();
      Oled.setCursor(4, 2);
      Oled.print("Goodbye");
      delay(1000);
      Oled.noDisplay();
    }
  }
}


