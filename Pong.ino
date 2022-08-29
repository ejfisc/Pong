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
  int y = 3;
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
  // current ball direction and position
  int *dirVector = &ball.direction[0];
  int *nextPos = &ball.coordinates[0];

  // current state of buttons
  int player1DownState = digitalRead(player1DownPin);
  int player1UpState = digitalRead(player1UpPin);
  int player2DownState = digitalRead(player2DownPin);
  int player2UpState = digitalRead(player2UpPin);

  // flag game start
  if(game.start) {
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
    Oled.drawTile(*nextPos, *(nextPos+1), 1, ballTile);
    game.start = false;
  }

  // Player Move Logic:
  // player 1 move down
  if(player1DownState == HIGH and player1.y < 5) {
    player1.y++;
    drawPlayer(1);
    clearPlayer(1, 0);
  }

  // player 1 move up
  if(player1UpState == HIGH and player1.y > 0) {
    player1.y--;
    drawPlayer(1);   
    clearPlayer(1, 1);
  }
  
  // player 2 move down
  if(player2DownState == HIGH and player2.y < 5) {
    player2.y++;
    drawPlayer(2);
    clearPlayer(2, 0);
  }

  // player 2 move up
  if(player2UpState == HIGH and player2.y > 0) {
    player2.y--;
    drawPlayer(2);
    clearPlayer(2, 1);
  }

  // Ball Move Logic:
  int lastPos[2] = {*nextPos, *(nextPos+1)};
  int *nextCoord = getNextBallPos(*nextPos, *dirVector);
  int ballPos[] = {*nextCoord, *(nextCoord+1)};
  Serial.println(lastPos[0]);
  Serial.println(lastPos[1]);
  Serial.println(*nextCoord);
  int address = nextCoord;
  Serial.println(address);
  //Oled.drawTile(lastPos[0], lastPos[1], 1, emptyTile);
  //Oled.drawTile(nextPos[0], nextPos[1], 1, ball);
    
  if(ballPos[0] == 0 or ballPos[0] == 15) {
    game.start = true;
    tone(BUZZER, 50, 1000);
    delay(1000);
    Oled.clearDisplay();
  }

  // collision with player 1, change direction
  if(ballPos[0] == player1.x+1 and (ballPos[1] >= player1.y-1) and (ballPos[1] <= player1.y+1)) {
    ball.direction[0] = 1;   
  }
  
  // collision with player 2, change direciton
  if(ballPos[0] == player2.x-1 and (ballPos[1] >= player2.y-1) and (ballPos[1] <= player2.y+1)) {
    ball.direction[0] = 0;
  }

  // game movement rate
  delay(5000);
}

// takes current ball pos and direction vector and returns array with next ball pos [x, y]
int getNextBallPos(int *currentPos, int *direction) {
  int next[2] = {4, 7};
  int *nextPos;
  nextPos = &next[0];
  int address = nextPos;
  Serial.println(address);
  Serial.println(*nextPos);
  return nextPos;
}

// replace the old player tile with empty tile
void clearPlayer(int playerNum, int previousPosition) {
  // player 1
  if(playerNum == 1) {
    // player moved up
    if(previousPosition > player1.y) {
      Oled.drawTile(player1.x, player1.y-2, 1, emptyTile);
    }
    // player moved down
    else {
      Oled.drawTile(player1.x, player1.y+2, 1, emptyTile);
    }
  }
  // player 2
  else {
    // player moved up
    if(previousPosition > player2.y) {
      Oled.drawTile(player2.x, player2.y-2, 1, emptyTile);
    }
    // player moved down
    else {
      Oled.drawTile(player2.x, player2.y+2, 1, emptyTile);
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




