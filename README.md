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

For racket movement, I'm checking to see if any of the button states are HIGH and if that corresponding racket is still within the bounds of the display. If the racket 1 up button is pressed, the racket is moved up, so on and so forth. 

For ball collision with the rackets, I'm checking for equivalent x coordinates. For ball collision with boundaries, I'm checking for equivalent x (right and left) and y (upper and lower) coordinates.
