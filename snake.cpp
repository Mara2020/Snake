/** ~~~~~~~~~~~~~~ SNAKE ~~~~~~~~~~~~~~
 *  
 *  Bianca Angotti   
 *  Celeste Chiasson 
 *  Randi Derbyshire
 *  Section A1
 * 
 *  Here we bring back the fantastic classic game of SNAKE!
 *  To start the game, there is a menu screen for the user to 
 *  choose a difficulty level. Once the user selects a level, 
 *  the game begins. Movement of the snake is controlled by the joystick.
 *  When the snake passes overtop of a food piece, the snake will grow 
 *  in length by 1. If the snake is driven to overlap itself, or if 
 *  the snake touches a border, the game is over. The GAME OVER screen 
 *  is displayed, showing the total length of the snake. 
 *  The game can begin again if the user presses the joystick. 
 * 
 *  Notes for improvement:
 *  - reduce usage of global variables
 *  - create more movement functions to reduce code size
 *  **/


/** SETUP **/
#include <Arduino.h>
#include <SD.h> // the SD card library
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <queue.h>
#include <stdlib.h>
#include <assert.h>

// Display pins:
// standard U of A library settings, assuming Atmel Mega SPI pins
#define SD_CS    5 // Chip select line for SD card
#define TFT_CS   6 // Chip select line for TFT display
#define TFT_DC   7 // Data/command line for TFT
#define TFT_RST  8 // Reset Line for TFT (or connect to +5V

// Color definitions 
#define BLACK     0x0000 
#define DARKRED   0xD000
#define RED       0xF800 
#define CYAN      0x07FF 
#define YELLOW    0xFFE0 
#define WHITE     0xFFFF
#define DARKGRN   0x0D4203

// Just like "tft" for screen, we have "card" for SD
// Make it global so that every function can use it
Sd2Card card;

// Define the tft
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Joystick pins
#define JOYSTICK_VERT   0  // Analog input A0 - vertical
#define JOYSTICK_HORIZ  1  // Analog input A1 - horizontal
#define JOYSTICK_BUTTON 9  // Digital input pin 9 for the button
#define speakerPin 		11 // Digital output pin 11 for speaker

void soundsetup() { pinMode(speakerPin, OUTPUT); }
    /* play a tone with period given in microseconds for a duration 
    given in milliseconds */
    
void playTone(int period, int duration) {
    // elapsed time in microseconds, the long is needed to get a big enough range.
    long elapsedTime = 0;
    // note we are making a slight error here if period is not even
    int  halfPeriod = period / 2;
    //~ duration = digitalRead(JOYSTICK_BUTTON);
    while (elapsedTime < duration * 1000L) {
        // generate a square wave of the given period.  Half the 
        // period is on, the other half is off.
            
        digitalWrite(speakerPin, HIGH);
        delayMicroseconds(halfPeriod);
     
        digitalWrite(speakerPin, LOW);
        delayMicroseconds(halfPeriod);
     
        // count the time we just consumed for one cycle of the tone
        elapsedTime = elapsedTime + period;
    }
}
    
void soundLoop() {
    int select = digitalRead(JOYSTICK_BUTTON);  
    if (select == HIGH) {
        playTone(660,100);
        delay(10);playTone(700,100);
        delay(10);playTone(760,100);
        delay(10);playTone(880,100);
    }
}

/** Functions and Structures **/
// Printing
// To print snake main display
void snake() {
  // printing to the screen
  tft.fillScreen(BLACK);
  
  tft.fillRoundRect(14,20,100,40,5,CYAN);
  // where the characters will be displayed
  tft.setCursor(20, 30); 
  tft.setTextColor(BLACK);
  tft.setTextSize(3);
  tft.setTextWrap(true);
  tft.print("SNAKE");
}

// To print the difficulty options
void easy(int box_colour) {
  tft.fillRoundRect(37,70,54,20,5,box_colour);
  tft.setCursor(41, 73); 
  tft.print("EASY");
}

void medium(int box_colour) {
  tft.fillRoundRect(25,100,79,20,5,box_colour);
  tft.setCursor(29, 103); 
  tft.print("MEDIUM");
}

void hard(int box_colour) {
  tft.fillRoundRect(37,130,54,20,5,box_colour);
  tft.setCursor(41, 133); 
  tft.print("HARD");
}

// Updating function to change highlighting of difficulty options
void update(int selection) {
  // Replace the old selection, so that it is not highlighted.
  // Need to set the cursor to the line old selection.
  // Since every line has 8 pixels, we skip 8 pixels for every line
  if (selection == 1) {
    tft.setTextColor(WHITE);
    easy(RED);
    tft.setTextColor(RED);
    medium(WHITE);
    hard(WHITE);
  }
  
  else if (selection == 2) {
    tft.setTextColor(WHITE);
    medium(RED);
    tft.setTextColor(RED);
    easy(WHITE);
    hard(WHITE);
  }
  
  else if (selection == 3) {
    tft.setTextColor(WHITE);
    hard(RED);
    tft.setTextColor(RED);
    easy(WHITE);
    medium(WHITE);
  }
}


// Grid check functionality (1 if snake body is there, 0 if it is not)
bool grid[24][30];

// Queue functionality

/*
  Constructor: create an instance of the
  queue and return a pointer to that instance.
*/
queue q_create(int max_size) {
  queue q = (queue)malloc(sizeof(q_array_t));

  //assert(boolean value)
  //will terminate the program if the boolean is false
  assert(q != NULL);

  q->x = (int *)malloc(max_size * sizeof(int));
  q->y = (int *)malloc(max_size * sizeof(int));
  assert(q->x != NULL);
  assert(q->y != NULL);

  q->length = 0;
  q->start = 0;
  q->buf_len = max_size;

  return q;
}

/*
  Destructor: free up the space used by
  the queue.
*/
void q_destroy(queue q) {
  free(q->x);
  free(q->y);
  free(q);
}

/*
  return the length of the queue
*/
int q_length(queue q) {
  return q->length;
}

/*
  add val to the end of the queue
*/
void q_add(queue q, int valx, int valy) {
  //~ Serial.print("qlength??");Serial.println(q->length);
  //~ Serial.print("qbuflength??");Serial.println(q->buf_len);
  assert(q_length(q) < q->buf_len);

  q->x[(q->start + q->length) % q->buf_len] = valx;
  q->y[(q->start + q->length) % q->buf_len] = valy;
  q->length = q->length + 1;
}

/*
  return the item at the front of the queue
*/
int q_frontx(queue q) {
  assert(q_length(q) > 0);

  return q->x[q->start];
}
int q_fronty(queue q) {
  assert(q_length(q) > 0);

  return q->y[q->start];
}

/*
  remove the item at the front of the queue
*/
void q_remove(queue q) {
  assert(q->length > 0);

  q->start = (q->start+1) % q->buf_len;
  q->length -= 1;
}

// Creating a queue with coordinates
// snakemax = ((120/5)*(150/5)) = 720
queue q;
int i,j, random_x, random_y, direction, pausedirection = 0;

// to read the user input of a change in direction
int init_vert, init_horiz, vertical, horizontal, speed;
int read_direction() {
  vertical = analogRead(JOYSTICK_VERT);      // will be 0-1023
  horizontal = analogRead(JOYSTICK_HORIZ);   // will be 0-1023
  
  int direction = 0;
  if (vertical > init_vert + 100) {
    direction = 1;
  }
  else if (vertical < init_vert - 100) {
    direction = 2;
  }
  else if (horizontal > init_horiz + 100) {
    direction = 3;
  }
  else if (horizontal < init_horiz - 100) {
    direction = 4;
  }
  
  return direction;
}

// functions to find the x and y coordinates of food
int food_x() { 
  int rand_x = 4 + 5*(rand() % 24);
  if (rand_x > 119) {rand_x = 119;}
  return rand_x;
}

int food_y() { 
  int rand_y = 5 + 5*(rand() % 30);
  if (rand_y > 150) {rand_y = 150;}
  return rand_y;
}

// To make the screen dissolve, find random numbers continuously.
int dissolve_x() {
  int rand_x = 5*(rand() % 27);
  if (rand_x > 128) {rand_x = 128;}
  return rand_x;
}

int dissolve_y() {
  int rand_y = 5*(rand() % 32);
  if (rand_y > 160) {rand_y = 160;}
  return rand_y;
}

int main() {
  // SETUP
  init();
  Serial.begin(9600);
  tft.initR(INITR_BLACKTAB);  // initialize screen
  
  // Setting the joystick button and LEDs
  pinMode(JOYSTICK_BUTTON, INPUT);
  digitalWrite(JOYSTICK_BUTTON, HIGH);
  
  // Initialize the SD card
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("failed!");
    while(true) {} // something is wrong
  } 
  else {Serial.println("OK!");}
  
  // More initialization
  Serial.print("Initializing Raw SD card...");
  if (!card.init(SPI_HALF_SPEED, SD_CS)) {
    Serial.println("failed!");
    while(true) {} // something is wrong
  } 
  else {Serial.println("OK!");}
  
  // Create states for different modes
  // C1 for Mode 1 - MENU screen
  // C2 for Mode 2 - Snake Game
  // C3 for Mode 3 - GAME OVER screen
  // C4 for Mode 4 - Choose level
  // C5 for Mode 5 - PAUSE screen
  typedef enum {C1 = 1, C2, C3, C4, C5, ERR} State;
  State state = C1;
  int select, snakelength;
  
  while (state!=ERR) {
    if  (state == C1) {
      /// ====== MODE 1 - MENU ====== ///
      Serial.println("Currently in Mode 1");
      snakelength = 1;
      init_vert = analogRead(JOYSTICK_VERT); 
      init_horiz = analogRead(JOYSTICK_HORIZ);
      
      // preparations for the game - to not overlap with the pause menu
      q = q_create(720);
      i = 64; // x component
      j = 80; // y component
      q_add(q,i,j); // load into the queue
      random_x = food_x(); // load x coordinate of food piece
      random_y = food_y(); // load y coordinate of food piece
      pausedirection = 0; // set paused direction to 0
      // reset grid to 0
      for (int a = 0; a < 24; a++) {
        for (int b = 0; b < 30; b++) {
        grid[a][b] = 0;
        }
      }
      
      // display main menu
      snake();
      tft.setTextSize(2);
      
      while(true) {
        // alternate highlighting of START
        unsigned long time = millis()%1000;
        int a = time%1000;
        if ((a<17)) {
        tft.setCursor(34, 83);
        tft.fillRoundRect(30,80,65,20,5,WHITE);
        tft.setTextColor(RED);
        tft.print("START");
        }
        else if ((a>500) && (a<520)) {
        tft.setCursor(34, 83);
        tft.fillRoundRect(30,80,65,20,5,RED);
        tft.setTextColor(WHITE);
        tft.print("START");
        }
        // Read the Joystick - HIGH if not pressed, LOW otherwise
        select = digitalRead(JOYSTICK_BUTTON);     
        if (select == LOW) {
          break;
        }
      }
      state = C4; 
    }
    
    else if (state == C2) {
      /// ====== MODE 2 - SNAKE GAME ====== ///
      Serial.println("Currently in Mode 2");
      delay(50);
      soundsetup(); //setting up sound pin
      // print the background
      tft.fillScreen(DARKRED);
      tft.fillRect(4,5,120,150,DARKGRN);
      
      // print the snake
      int x,y;
      x = q_frontx(q);
      y = q_fronty(q);
      tft.fillRect(x,y,5,5, WHITE);
      
      //Bringing the food in, outside while loop first.
      tft.fillRect(random_x, random_y, 5, 5, YELLOW);
      
      // do auto calibration
      int px, py;
      int lastmove;
      
      // read beginning direction chosen by user
      if (pausedirection == 0) {
        direction = read_direction();
      }
      else {
        direction = pausedirection;
      }
      lastmove = direction;
      
      while (true) {
        
        // to direct movement 
        // (without going in reverse direction of previous movement)
        
        // up
        if (direction == 1) {
          if (lastmove == 2) {
            direction = 2;
            j = j-5;
          }
          else {
            j = j+5;
        }
        q_add(q,i,j);
        }
        // down
        else if (direction == 2) {
          if (lastmove == 1) {
            direction = 1;
            j = j+5;
          }
          else {
            j = j-5;
          }
        q_add(q,i,j);
        }
        // right
        else if (direction == 3) {
          if (lastmove == 4) {
            direction = 4;
            i = i-5;
          }
          else {
            i = i+5;
          }
        q_add(q,i,j);
        }
        // left
        else if (direction == 4) {
          if (lastmove == 3) {
            direction = 3;
            i = i+5;
          }
          else {
            i = i-5;
          }
        q_add(q,i,j);
        }
        
        // if the direction is changed, store the new direction & last move
        int new_direc = read_direction();
        if ((new_direc != direction) && (new_direc != 0)) {
          lastmove = direction;
          direction = new_direc;
        }
        
        // if the snake hits a piece of food, the food vanishes and gets replaced 
        if ((i == random_x) && (j == random_y)) {
          // snake grows by 4 squares, except for the first time
          // this allows for it to end up as a max of 720 in the queue
          if (snakelength == 1) {
            q_add(q,i,j);
            q_add(q,i,j);
            q_add(q,i,j);
            snakelength += 3;
          }
          else {
            q_add(q,i,j);
            q_add(q,i,j);
            q_add(q,i,j);
            q_add(q,i,j);
            snakelength += 4;
          }
      if (snakelength < 720) {
        random_x = food_x();
        random_y = food_y();
      
        // if the snake is already there, find a new spot for the food
        while (grid[random_x/5][random_y/5-1] == 1) {
          random_x = food_x();
          random_y = food_y();
        }
        // print the new food
        tft.fillRect(random_x, random_y, 5, 5, YELLOW);
          }
        }
        
        // if the snake runs over itself
        if ((snakelength > 1) && (grid[i/5][j/5-1] == 1)) {
          delay(450); // pause when snake runs into itself
          int m = 0;
          soundLoop();
          while(m < 6000) {
            int rand_x = dissolve_x();
            int rand_y = dissolve_y();
            tft.fillRect(rand_x, rand_y, 5, 5, BLACK);
            m++;
          }
          state = C3;
          break;
        }
        
        px = q_frontx(q);
        py = q_fronty(q);
        // reprint the snake if there is movement
        if ((i != px) || (j != py)) {
          tft.fillRect(i,j,5,5, WHITE);
          grid[i/5][j/5-1] = 1;          // snake body is in grid
          tft.fillRect(px,py,5,5,DARKGRN);
          grid[px/5][py/5-1] = 0;        // snake body is no longer in grid
          q_remove(q);                   // take away from the queue
          delay(speed);                  // controls the speed of the snake
        }
       
        // if any of the borders are hit
        if ((i < 4)||(j < 5)||(i > 119)||(j > 150)) {
          delay(450); // pause when border is hit
          // dissolve the screen
          int m = 0;
          soundLoop();
          while(m < 6000) {
            int rand_x = dissolve_x();
            int rand_y = dissolve_y();
            tft.fillRect(rand_x, rand_y, 5, 5, BLACK);
            m++;
          }
          //~ delay(250);
          state = C3; 
          break;
        }
        
        // Read the Joystick - HIGH if not pressed, LOW otherwise
        select = digitalRead(JOYSTICK_BUTTON);     
        if (select == LOW) {
          state = C5;
          break;
        }
      }
    }
    
    else if (state == C3) {
      /// ====== MODE 3 - GAME OVER ====== ///
      Serial.println("Currently in Mode 3");
      q_destroy(q); // clear the queue
      tft.fillScreen(BLACK);
      tft.fillRoundRect(5,20,118,25,5,RED);
      tft.setCursor(10, 25); 
      tft.setTextColor(BLACK);
      tft.setTextSize(2);
      tft.setTextWrap(true);
      tft.print("GAME OVER");
      tft.print("\n"); 
      
      tft.setCursor(10, 55);
      tft.setTextColor(RED);
      tft.setTextSize(1.5);
      if (snakelength >= 720) {
        snakelength = 720;
        tft.print("YOU WON! CONGRATZ");
      }
      else {
        tft.print("      Oh no!         You hit something!");
      }
      
      tft.setCursor(10, 80);
      tft.setTextColor(WHITE);
      tft.setTextSize(1);
      tft.print("Length of Snake:");
      tft.print(snakelength);
      tft.setCursor(10, 100);
      tft.print("Press the joystick   to return to main    menu");
      
      // Read the Joystick - HIGH if not pressed, LOW otherwise
      while (true) {
        select = digitalRead(JOYSTICK_BUTTON);     
        if (select == LOW) {
          break;
        }
      }
      state = C1;
    }
    
    else if (state == C4) {
      /// ====== MODE 4 - CHOOSE LEVEL ====== ///
      Serial.println("Currently in Mode 4");
      // printing
      // snake display
      snake();
      // difficulty levels
      tft.setTextSize(2);  
      tft.setTextColor(WHITE);
      easy(RED);
      tft.setTextColor(RED);
      medium(WHITE);
      hard(WHITE);
      
      int selection = 1;
      int oldselection;
      while(true) {
        // read direction from the user for updating selection
        oldselection = selection;
        vertical = analogRead(JOYSTICK_VERT);      // will be 0-1023
        delay(100);
        
        // scroll down
        if (vertical > init_vert + 200) {
        selection++;
          if (selection > 3) {
            selection = 0;
          }
        } 
        // scroll up
        else if (vertical < init_vert - 200) {
          selection--;
          if (selection < 0) {
            selection = 3;
          }
        }
        
        if (selection != oldselection) {
          update(selection);
        }
        
        // Read the Joystick - HIGH if not pressed, LOW otherwise
        select = digitalRead(JOYSTICK_BUTTON);     
        if (select == LOW) {
          Serial.print("made selection: ");
          Serial.println(selection);
          if (selection == 1) {speed = 225;}
          else if (selection == 2) {speed = 150;}
          else if (selection == 3) {speed = 75;}
          break;
        }
      }
      state = C2;
    }
    
    else if (state == C5) {
      /// ====== MODE 5 - PAUSE MENU ====== ///
      Serial.println("Currently in Mode 5");
      pausedirection = direction;
      
      // printing snake and pause
      snake();
      tft.setTextSize(2);
      tft.setCursor(34, 73); 
      tft.fillRoundRect(30,70,65,20,5,WHITE);
      tft.setTextColor(RED);
      tft.print("Pause");
      
      while(true) {
        // Read the Joystick - HIGH if not pressed, LOW otherwise
        select = digitalRead(JOYSTICK_BUTTON);     
        if (select == LOW) {
          break;
        }
      }
      // reset grid to 0
      for (int a = 0; a < 24; a++) {
        for (int b = 0; b < 30; b++) {
        grid[a][b] = 0;
        }
      }
      state = C2; 
    }
    //if not any of this:
    else { 
      Serial.println("There has been an error");
      state = ERR; 
    }
  }
    
  Serial.end();
  return 0;
}
