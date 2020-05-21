//#include <Arduino.h>
//#include <SPI.h>
#include <U8g2lib.h>

#define UP_BTN 2
#define DOWN_BTN 4
#define LEFT_BTN 5
#define RIGHT_BTN 3
#define E_BTN 6
#define F_BTN 7

#define HEAD_STOP 0
#define HEAD_UP 1
#define HEAD_DOWN 2
#define HEAD_LEFT 3
#define HEAD_RIGHT 4
int heading = HEAD_STOP; // 0-stopped,1-up,2-down,3-left,4-right

int snakeHead[2] = {5,6}; // starting spot

int apple[2]= {-1,-1}; // apple starts out of view

//15x15 grid of game positions, 2 coords for each
int snakeBody[225][2]= { //initial starting positions
  {1,6},
  {2,6},
  {3,6},
  {4,6},
  {5,6}
};
int snakeLength = 5; //starts at 5
char scoreText[8];
char lastScore;
bool gameOver = false;
int gs = 4; //gs is GRID SIZE, 4 works well on my duinotech 128x64 OLED
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
const long interval = 300; // interval to take game turns, 300 is pretty nice
bool inputLock = false;

/* U8G2 Constructor */
//U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9); // uses more memory
U8G2_SH1106_128X64_NONAME_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9); // uses less memory

void setup() {
  //Set all button pins as inputs with internal pullup resistors enabled.
  pinMode(UP_BTN, INPUT_PULLUP);
  pinMode(DOWN_BTN, INPUT_PULLUP);
  pinMode(LEFT_BTN, INPUT_PULLUP);
  pinMode(RIGHT_BTN, INPUT_PULLUP);
  pinMode(E_BTN, INPUT_PULLUP);
  pinMode(F_BTN, INPUT_PULLUP);
  u8g2.begin(); //start u8g2 display
  u8g2.setFont(u8g2_font_crox1t_tf);
  //randomSeed(analogRead(0));
  Serial.begin(9600); //for troubleshooting
  }

void inputs() {
  // buttons change heading, up button for heading=up and so on
  if ( inputLock == false && heading!=HEAD_DOWN && digitalRead(UP_BTN) == LOW) {
    heading = HEAD_UP;
    inputLock = true; // stop accepting input presses, prevents snake being able to head back on itself by changing heading rapidly
    gameOver = false;
  }
  if ( inputLock == false && heading!=HEAD_UP && digitalRead(DOWN_BTN) == LOW) {
    heading = HEAD_DOWN;
    inputLock = true;
    gameOver = false;
  }
  if ( inputLock == false && heading!=HEAD_RIGHT && digitalRead(LEFT_BTN) == LOW) {
    heading = HEAD_LEFT;
    inputLock = true;
    gameOver = false;
  }
  if ( inputLock == false && heading!=HEAD_LEFT && digitalRead(RIGHT_BTN) == LOW) {
    heading = HEAD_RIGHT;
    inputLock = true;
    gameOver = false;
  }
  // reset game to starting state
  if ( digitalRead(F_BTN) == LOW) {
    restartGame();
  }

}
bool onSnakeBody(int x,int y) {
  for (int i=0; i< snakeLength; i++) {
    if (snakeBody[i][0]==x && snakeBody[i][1] == y) {
      return true;
    }
  }
  return false;
}

void logic() {
  
  // if apple is out of view, spawn it on the game grid at a random location where the snake doesn't occupy
  if (apple[0]== -1 && apple[1] == -1) {
    do {
      apple[0]=random(15);
      apple[1]=random(15);
      //keep picking apple spawn pos so long as...
    } while ( onSnakeBody(apple[0],apple[1]) );
  }

  // this controls game speed to make the game "tick" along at a set interval
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    inputLock = false; //release input lock, accepting direction presses again

    // detect snake eating apple
    if (apple[0] == snakeHead[0] && apple[1] == snakeHead[1]) {
      //Serial.println("ate apple. yum!");
      snakeLength++;
      apple[0]= -1;
      apple[1]= -1;
    }
    
    // if snake moving update last body postion with head position
    if (heading!=HEAD_STOP) {
  
      //the front of body becomes becomes head position
      snakeBody[snakeLength-1][0] = snakeHead[0];
      snakeBody[snakeLength-1][1] = snakeHead[1];
  
      // the other segments of the body update, the tail[0] becomes segment[1], [1] becomes [2] and so on...
      for (int i=0; i< snakeLength-1; i++) {
        snakeBody[i][0] = snakeBody[i+1][0];
        snakeBody[i][1] = snakeBody[i+1][1];
      }
    }
    // move snake head if heading == HEAD_UP/DOWN/LEFT/RIGHT (nothing here for HEAD_STOP)
    if (heading==HEAD_UP) {
      snakeHead[1]--;
    }
    if (heading==HEAD_DOWN) {
      snakeHead[1]++;
    }
    if (heading==HEAD_LEFT) {
      snakeHead[0]--;
    }
    if (heading==HEAD_RIGHT) {
      snakeHead[0]++;
    }

    //detect if snake head touched snake body
    if ( onSnakeBody(snakeHead[0],snakeHead[1]) ) {
      heading=HEAD_STOP;
      restartGame();
      gameOver = true;
    }
    
    //detect if snake head left game area
    if ( snakeHead[0] < 0 || snakeHead[0] > 15 || snakeHead[1] < 0 || snakeHead[1] > 15 ) {
      heading=HEAD_STOP;
      restartGame();
      gameOver = true;
    }
  }
}

void loop() {
  
  currentMillis = millis();
  
  inputs();
  logic();
  
  u8g2.firstPage(); // required for forst part of display stuff
  do {
    draw();
  } while ( u8g2.nextPage() );
}


/*
//considered a function for a detailed snake head which changes based on heading, not being used
void johnHead(int x,int y) {
  x = x * gs;
  y = y * gs;
  u8g2.drawPixel(x, y);
}
*/

void johnBox(int x,int y) {
  x = x * gs;
  y = y * gs;
  u8g2.drawBox(x, y, gs, gs);
}
void johnDisc(int x,int y) {
  x = x * gs+(gs/2);
  y = y * gs+(gs/2);
  int rad = (gs/2);
  u8g2.drawDisc(x,y,rad,U8G2_DRAW_ALL);
}

void draw() {
  //draw apple
  johnDisc(apple[0],apple[1]);
  
  //draw snake head
  johnBox(snakeHead[0],snakeHead[1]);
  //johnHead(snakeHead[0],snakeHead[1]);
  
  //draw snake body  
  for (int i=0; i< snakeLength-1; i++) {
    johnBox(snakeBody[i][0],snakeBody[i][1]);
  }
  
  //draw SNAKE logo with blank space
  u8g2.drawStr(70,10,  "S");
  u8g2.drawStr(75,12,  "N");
  u8g2.drawStr(83,10,  "A");
  u8g2.drawStr(90,12,  "K");
  u8g2.drawStr(98,10,  "E");
  u8g2.drawStr(107,10,  "!");
  u8g2.drawStr(113,12,  "!");
  u8g2.drawStr(119,10,  "!");

  u8g2.drawFrame(0,0,64,64); // divider line, pretty

  /* meh
  u8g2.drawStr(70,35, "Score: "); // score stuff
  itoa(snakeLength-5, scoreText, 10);
  u8g2.drawStr(98,35,  scoreText);
  */  

  if (gameOver) {
    u8g2.drawStr(70,42, "You Lose!"); // show 'you lose'
  }
}

void restartGame()  {
  snakeHead[0] = 5; 
  snakeHead[1] = 6; 
  heading = HEAD_STOP;
  apple[0]= -1;
  apple[1]= -1;
  snakeBody[0][0]=1;
  snakeBody[0][1]=6;
  snakeBody[1][0]=2;
  snakeBody[1][1]=6;
  snakeBody[2][0]=3;
  snakeBody[2][1]=6;
  snakeBody[3][0]=4;
  snakeBody[3][1]=6;
  snakeBody[4][0]=-1;
  snakeBody[4][1]=-1;
  snakeLength = 5;
  gameOver = false;
}
