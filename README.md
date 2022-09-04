# Pong
## Brief Description:
A stand alone game of Pong developed on an Arduino Uno R3 wired up to an OLED display with physical buttons and a buzzer for sound effects.

## More Details:
There are 6 essential components to this system:
 - The Arduino Uno R3
 - The Arduino Sensor Kit Shield
 - The breadboard circuit with pushbuttons
 - The small OLED display
 - The buzzer
 - The software itself

 The Uno is the brains of the operation, the code is stored on the controller and as long as the Arduino is supplied power, the system will run. 

 The sensor kit shield sits directly on top of the Uno. It mirrors all of the I/O of the Uno so you plug your input, power, and ground pins into the shield instead of the Arduino itself. You still have all the same functionality of the Arduino, shields give you more functionality on top of that. This shield in particular gives you 16 more I/O ports for the various devices in the Arduino Sensor Kit. 

 The breadboard circuit is how the physical buttons are wired to the system. I have power running from the Arduino to the breadboard and from the power rail to each of the 4 buttons. The buttons are all then wired to their own digital input pin. Each each button is then connected to a 10kΩ resistor. These resistors bring the voltage down to 0 and are necessary to keep the button signal stable. If there was no resistor (i.e. the button was connected only to the input pin) the input signal that the Arduino receives would oscillate between HIGH and LOW and easily be affected by any digital noise. The resistor is then wired to ground which goes back to the GND pin on the Arduino. There is a diagram in the repository that shows a more clear version of the button circuit and how it's all wired up, the positioning isn't exact but the order of circuit elements and resistor values are all the same.

 The small 128x64 OLED display came with the Arduino Sensor Kit and uses the u8x8 library. The buzzer came with the Arduino Sensor Kit and can output a specified frequency for either a specified duration or until you tell it to stop. I used the buzzer to produce sound effects by manipulating different frequencies and durations. 

 Now for the nitty gritty.

 ## The Software:
All of the code is contained within Pong.ino. Every Arduino sketch contains a setup() that runs once and a loop() that runs infinitely. setup() is fairly simple, all I'm doing there is establishing the button pins as input and the buzzer as output, starting the serial monitor used for textual game state updates, and initializing the OLED display. loop() is where all of the game logic is. In loop(), I'm constantly checking for a new game state, new round state, the state of each button, and calculating the next ball position. 

I set up structs for each of the game objects and the game itself. The rackets struct keeps track of points, their location, if they're active or not (i'll get into what that means in a bit) and if they won or not. The ball struct keeps track of its location and direction. The game struct is used for game state, I have two booleans in there that let the system know to start a new game or a new round. 

For racket movement, I'm checking to see if any of the button states are HIGH and if that corresponding racket is still within the bounds of the display. If the racket 1 up button is pressed, I decrement the racket1 y coordinate and draw a new racket tile in the correct location, and then draw an empty tile to remove the excess racket tile at the old position. I found this to be the smoothest method. The ball is constantly moving, and I found that drawing the next tile and then removing the old one made the ball look like it was dragging, so I clear the old tile out before drawing the new one. 

For ball collision with the rackets, I'm checking for equivalent x coordinates. For ball collision with boundaries, I'm checking for equivalent x (right and left) and y (upper and lower) coordinates. When the ball collides with a racket, I calculate the slope based on where the ball hit the racket. The slope = 3(racketY - ballY)/racketHeight, in this case the racket height is 3 tiles so this simplifies out to (racketY - ballY). If the ball hits the bottom or top of the racket, it will move with -1 or 1 slope respectively, if it hits the middle tile of the racket the ball will move in a straight line with a 0 slope. 

At the start of each round, the ball moves in the direction it was previously moving. At the very beginning, no racket is active. Once the ball hits a racket, that racket is the active racket, and this switches off as the ball is hit between the two rackets. This is how I keep track of who gets the points. If both rackets miss in the very beginning, i.e. there is no active racket and the ball goes past the x boundary, nobody gets any points. 

## Problems I ran into
One of the biggest problems I ran into was not being able to easily pass and return arrays with a function. This was important for getNextBallPosition() which takes the current position of the ball: [x, y] and the direction vector of the ball: [direction, slope] and spits out the next [x, y] for the ball. So I was going to have to use pointers to the arrays, but pointers, as I understood them, did not work the same way in Arduino. The Arduino language seems to be some weird combination of C and C++, and sometimes the C or C++ code I'd write wouldn't work the way I expected it to. Lots of trial and error and a separate test sketch helped me figure out how to get it all working. There might be a better/more efficient way of doing things that I wasn't able to find, but it works the way I want it to. 

This problem leads me to my next problem, which is two particular Serial.println() statements in getNextBallPosition(). While testing the pointers and making sure values were passed and returned correctly, I used two Serial.println() statements to print out an address and a dereferenced pointer. Then, when I got everything working, I removed those two statements and everything broke. I figured out that for whatever reason, whenever I commented out or removed those two statements, the function returned an incorrect output which causes issues in loop(). I spent hours trying to figure out what was going on, messing with different Serial.println() statements, different return values, etc. Everytime I commented or removed those two statements, the function would return a 0, despite previously returning the correct value when those statements were there. These two statements should have no functional purpose, they don't do anything programmatically other than print to the serial monitor. I have no idea why this is happening. The code outside of these two statements didn't change, theoretically the function should have produced the exact same output with or without those Serial.println() statements there. Given that this is just a personal project, I didn't feel that it was important enough to continue wasting time on, so I have decided to just leave them there, and made a note reminding me or anyone else who may work on this in the future to not remove those print statements. 

## What I would have liked to do differently
I must move on to other projects, but if I had more time with this one I would have figured out how to code the OLED display to work with all 128x64 pixels instead of just 8x8 tiles. The tutorial on the Arduino Sensor Kit web page used the u8x8 library so that's what I continued to use, but being able to manipulate each pixel would have allowed me to make the ball movement more fluid and allowed for more hit ratios with the racket which would allow for more various slopes rather than just 1 and -1. 

## What I learned
This was my first real project with arduino that involved a circuit and several components and a significant amount of software and time. The others that came before it were small, quick, and simple learning experiments. This project taught me a lot about how to program Arduino development boards. I learned some of the differences between C and C++, and how those two languages are used. The button circuit taught me about using resistors for stabilizing voltage signals. The OLED display and Buzzer taught me about incorporating different devices and hardware components in my circuits and using different libraries of functions to work with them. 