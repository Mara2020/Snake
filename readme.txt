This code was made to create the game 'Snake' on an arduino board. The object of the game is to use the joystick to move the snake around the screen, eating as much food on the screen as possible. Every time the snake eats some food, it will grow by a fixed increment every time. It will be a game over if the snake hits one of the red borders, or runs over its own tail.

Accessories:
- Arduino Mega Board (AGM)
- USB cable
- LCD screen
- Joystick


Wiring Instructions:
GND <-> BB Negative Bus
5V Power <-> BB Positive Bus
Pin 11 < Resistor > Speaker <-> BB Negative Bus

Wiring for the LCD Screen: 
(in the order from left to right (LCD screen facing up), with pins of the LCD at the bottom, closest to you)
1. GND to BB GND bus
2. VCC to BB positive bus
3. RESET to Pin 8
4. D/C (Data/Command) to Pin 7
5. CARD_CS (Card Chip Select) to Pin 5
6. TFT_CS (TFT/screen Chip Select) to Pin 6
7. MOSI (Master Out Slave In) to Pin 51
8. SCK (Clock) to Pin 52
9. MISO (Master In Slave Out) to Pin 50
10. LITE (Backlite) to BB positive bus

Wiring for the Joystick:
1. VCC to BB positive bus
2. VERT to Pin A0
3. HOR to Pin A1
4. SEL to Pin 9
5. GND to BB GND bus

Running Code:
- Open Virtual Machine and login to cmput274
- Open terminal, load directory containing snake.cpp and its Makefile
- Make sure the Arduino is properly wired and configured to USB using command "arduino-port-select"
- To edit code, use command "geany snake.cpp &"
- Compile code using command "make"
- If code correctly compiles, use command "make upload" to upload the code
- Use the joystick to play the game snake. Click on the joystick to click 'start' on the game and to select your levels. During the game, click the joystick to pause the game. Click it again to unpause.

Assumptions:
- Upon uploading, the initial screen should be the start screen of the game. 
- When the snake hits one of the red borders, the game is over. 
