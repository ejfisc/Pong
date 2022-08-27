#include <U8g2lib.h>
#include <U8x8lib.h>
#include <U8x8lib.h>
#include <Arduino_SensorKit_BMP280.h>
#include <Arduino_SensorKit_LIS3DHTR.h>
#include "Arduino_SensorKit.h"

#define BUZZER 5

struct {
  int x;
  int y;
  int nextY;
  int points;
} player1, player2;

struct {
  int x;
  int y;
  // direction vector: [1/0, 1/0, x]
  // [right/left, positive/negative, magnitude of slope]
  // ex: [1, 1, 0] ball is moving right in a straight line
  // [0, 1, 2] ball is moving left in the upwards direction with a slope of 2x
  static int direction[3];
} ball;

struct {
  bool start;
} game;

int player1Down = 10;
int player1Up = 11;
int player2Down = 8;
int player2Up = 9;
uint8_t playerTile[16] = {0xff, 0xff, 0xff, 0xff, 0x0, 0x0, 0x0, 0x0, 255, 255, 255, 255, 255, 255, 255, 255}; 
uint8_t emptyTile[16] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t ballTile[16] = {0x0f, 0x0f, 0x0f, 0x0f, 0x0, 0x0, 0x0, 0x0, 100, 100, 100, 100, 100, 100};


void setup() {
  // establish buttons as input
  pinMode(player1Down, INPUT);
  pinMode(player1Up, INPUT);
  pinMode(player2Down, INPUT);
  pinMode(player2Up, INPUT);
  pinMode(BUZZER, OUTPUT);

  // initialize display
  Oled.begin();
  Oled.setFlipMode(true);
  Oled.setFont(u8x8_font_courB18_2x3_f);
  game.start = true;
  player1.x = 1;
  player2.x = 14;
}

void loop() {
  if(game.start) {
    // initialize player and ball positions
    player1.y = 3;
    player2.y = 3;
    ball.x = 8;
    ball.y = 3;
    ball.direction[0] = 1;
    ball.direction[1] = 1;
    ball.direction[2] = 0;
    
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
    Oled.drawTile(ball.x, ball.y, 1, ballTile);
    game.start = false;
  }
  // PLAYER MOVE LOGIC
  // get current state of buttons
  int player1DownState = digitalRead(player1Down);
  int player1UpState = digitalRead(player1Up);
  int player2DownState = digitalRead(player2Down);
  int player2UpState = digitalRead(player2Up);

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
  
  // collision with player 1, change direction
  if(ball.x == player1.x+1 and (ball.y >= player1.y-1) and (ball.y <= player1.y+1)) {
    ball.direction[0] = 1;   
  }
  
  // collision with player 2, change direciton
  if(ball.x == player2.x-1 and (ball.y >= player2.y-1) and (ball.y <= player2.y+1)) {
    ball.direction[0] = 0;
  }

  if(ball.x == 0 or ball.x == 15) {
    game.start = true;
    tone(BUZZER, 50, 1000);
    delay(1000);
    Oled.clearDisplay();
  }

  int ballPos[2];
  ballPos[0] = ball.x;
  ballPos[1] = ball.y;
  int nextPos[2] = getNextBallPos(ballPos, ball.direction);
  Oled.drawTile(ballPos[0], ballPos[1], 1, emptyTile);
  Oled.drawTile(nextPos[0], nextPos[1], 1, ball);
    

  delay(30);
}

// takes current ball pos and direction vector and returns array with next ball pos [x, y]
int getNextBallPos(int currentPos[], int direction[]) {
  // ball is moving right
  if(direction[0] == 1) {
    ball.x = currentPos[0] + 1;
  }
  else if(direction[0] == 0) {
    ball.x = currentPos[0] - 1;
  }
  ball.y = currentPos[1];
  return [ball.x, ball.y]

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