#include <U8g2lib.h>
#include <U8x8lib.h>
#include <U8x8lib.h>
#include <Arduino_SensorKit_BMP280.h>
#include <Arduino_SensorKit_LIS3DHTR.h>
#include "Arduino_SensorKit.h"

#define BUZZER 5

int player1DownPin = 10;
int player1UpPin = 11;
int player2DownPin = 8;
int player2UpPin = 9;
uint8_t playerTile[16] = {0xff, 0xff, 0xff, 0xff, 0x0, 0x0, 0x0, 0x0, 255, 255, 255, 255, 255, 255, 255, 255}; 
uint8_t emptyTile[16] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t ballTile[16] = {0x0f, 0x0f, 0x0f, 0x0f, 0x0, 0x0, 0x0, 0x0, 100, 100, 100, 100, 100, 100};

struct {
  int x;
  int y;
  int points;
} player1, player2;

struct {
  int coordinates[2] = {8, 3};
  // direction vector: [1/0, 1/0, x]
  // [right/left, positive/negative, magnitude of slope]
  // ex: [1, 1, 0] ball is moving right in a straight line
  // [0, 1, 2] ball is moving left in the upwards direction with a slope of 2x
  int direction[3] = {1, 1, 0};
} ball;

struct {
  bool start;
} game;



void setup() {
  // establish buttons as input
  pinMode(player1DownPin, INPUT);
  pinMode(player1UpPin, INPUT);
  pinMode(player2DownPin, INPUT);
  pinMode(player2UpPin, INPUT);
  pinMode(BUZZER, OUTPUT);

  Serial.begin(9600);

  // initialize display
  Oled.begin();
  Oled.setFlipMode(true);
  Oled.setFont(u8x8_font_courB18_2x3_f);
  game.start = true;
  player1.x = 1;
  player2.x = 14;
}

void loop() {
  // Flag Game Start:
  if(game.start) {
    // reset coordinates
    ball.coordinates[0] = 8;
    ball.coordinates[1] = 3;
    player1.y = 3;
    player2.y = 3;

    // draw players
    drawPlayer(1);
    drawPlayer(2);

    // countdown timer
    int countdown = 3;
    for(int i = 3; i > 0; i--) {
      Oled.setCursor(7, 2);
      Oled.print(i);
      delay(1000);
      Oled.refreshDisplay();
    }
    
    // clear screen and draw the ball
    Oled.setCursor(7, 2);
    Oled.print(" ");
    Oled.refreshDisplay();
    Oled.drawTile(ball.coordinates[0], ball.coordinates[1], 1, ballTile);
    game.start = false;
  }

  // Player Move Logic:

  // current state of buttons
  int player1DownState = digitalRead(player1DownPin);
  int player1UpState = digitalRead(player1UpPin);
  int player2DownState = digitalRead(player2DownPin);
  int player2UpState = digitalRead(player2UpPin);

  // player 1 move down
  if(player1DownState == HIGH and player1.y+1 < 7) {
    int previous = player1.y;
    player1.y++;
    movePlayer(1, previous);
  }

  // player 1 move up
  if(player1UpState == HIGH and player1.y-1 > 0) {
    int previous = player1.y;
    player1.y--;
    movePlayer(1, previous);
  }
  
  // player 2 move down
  if(player2DownState == HIGH and player2.y+1 < 7) {
    int previous = player2.y;
    player2.y++;
    movePlayer(2, previous);
  }

  // player 2 move up
  if(player2UpState == HIGH and player2.y-1 > 0) {
    int previous = player2.y;
    player2.y--;
    movePlayer(2, previous);
  }

  // Ball Move Logic:

  int lastPos[2] = {ball.coordinates[0], ball.coordinates[1]};
  int dirVector[3] = {ball.direction[0], ball.direction[1], ball.direction[2]};
  int *nextBallCoord = getNextBallPos(lastPos, dirVector);
  ball.coordinates[0] = *nextBallCoord;
  ball.coordinates[1] = nextBallCoord[1];
  Oled.drawTile(lastPos[0], lastPos[1], 1, emptyTile);
  Oled.drawTile(ball.coordinates[0], ball.coordinates[1], 1, ballTile);

  Serial.println(ball.coordinates[0]);
  if(ball.coordinates[0] == 0 or ball.coordinates[0] == 15) {
    Serial.println("game over");
    game.start = true;
    tone(BUZZER, 50, 1000);
    delay(1000);
    Oled.clearDisplay();
  }

  // collision with player 1, change direction
  if(ball.coordinates[0] == player1.x+1 and (ball.coordinates[1] >= player1.y-1) and (ball.coordinates[1] <= player1.y+1)) {
    ball.direction[0] = 1;
    Serial.println("Ball changed direction");
  }
  
  // collision with player 2, change direciton
  if(ball.coordinates[0] == player2.x-1 and (ball.coordinates[1] >= player2.y-1) and (ball.coordinates[1] <= player2.y+1)) {
    ball.direction[0] = 0;
    Serial.println("Ball changed direction");
  }

  // game movement rate
  delay(30);
}

// takes current ball pos and direction vector and returns array with next ball pos [x, y]
int getNextBallPos(int *currentPos, int *dirVector) {
  int newX;
  int newY = currentPos[1];
  Serial.println(*dirVector);
  if(*dirVector == 1) {
    newX = *currentPos + 1;
    Serial.println("ball moved right");
  }
  else if(*dirVector == 0) {
    newX = *currentPos - 1;
    Serial.println("ball moved left");
  }
  int next[2] = {newX, newY};
  int *nextPos;
  nextPos = &next[0];
  int address = nextPos;
  // DO NOT REMOVE THESE PRINT STATEMENTS!!
  Serial.println(address);    // I have absolutely no idea why, but this code literally doesn't work without them. If they're not there, this function returns incorrect output.
  Serial.println(*nextPos);   // I'm not sure what they're doing in the backend, I've spent hours trying to figure it out, I couldn't, so I've resolved to just keeping these here.
  return nextPos;
}

// replace the old player tile with empty tile
void movePlayer(int playerNum, int previousPosition) {
  // player 1
  if(playerNum == 1) {
    // player moved up
    if(previousPosition > player1.y) {
      Oled.drawTile(player1.x, player1.y-1, 1, playerTile);
      Oled.drawTile(player1.x, player1.y+2, 1, emptyTile);
    }
    // player moved down
    else {
      Oled.drawTile(player1.x, player1.y+1, 1, playerTile);
      Oled.drawTile(player1.x, player1.y-2, 1, emptyTile);
    }
  }
  // player 2
  else {
    // player moved up
    if(previousPosition > player2.y) {
      Oled.drawTile(player2.x, player2.y-1, 1, playerTile);
      Oled.drawTile(player2.x, player2.y+2, 1, emptyTile);
    }
    // player moved down
    else {
      Oled.drawTile(player2.x, player2.y+1, 1, playerTile);
      Oled.drawTile(player2.x, player2.y-2, 1, emptyTile);
    }
  }
}

void drawPlayer(int playerNum) {
  if(playerNum == 1) {
    Oled.drawTile(player1.x, player1.y-1, 1, playerTile);
    Oled.drawTile(player1.x, player1.y, 1, playerTile);
    Oled.drawTile(player1.x, player1.y+1, 1, playerTile);
  }
  else if(playerNum == 2) {
    Oled.drawTile(player2.x, player2.y-1, 1, playerTile);
    Oled.drawTile(player2.x, player2.y, 1, playerTile);
    Oled.drawTile(player2.x, player2.y+1, 1, playerTile);
  }
}



