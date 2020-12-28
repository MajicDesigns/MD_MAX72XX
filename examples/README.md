# MD_MAX72xx Library Examples

If you like and use this library please consider making a small donation 
using [PayPal](https://paypal.me/MajicDesigns/4USD)

[Library Documentation](https://majicdesigns.github.io/MD_MAX72XX/)

<hr>

**MD_MAX72xx_DaftPunk**  
Uses the library to display a Daft Punk LED Helmet animation.  
The display can be set to change animation through a switch or 
just cycle through all the animations.
<hr>

**MD_MAX72xx_Dynamic_HW**  
Use the library to display text on the display while cycling through 
the library supported hardware types. It prints out the module type - 
the one that is legible on the display is the type of hardware being 
used.

This is an easy way to determine what type of hardware is being used 
as an alternative to the HW_Mapper sketch. Recommended that at least 
three modules are used.
<hr>

**MD_MAX72xx_Eyes**  
Uses the graphics functions to animate a pair of eyes on 
two matrix modules. Eyes are coordinated to work together and are
created to fill all available modules.
<hr>

**MD_MAX72xx_HW_Mapper**  
Test software to map display hardware rows and columns. Uses a 
generic SPI interface and only one MAX72xx/8x8 LED module is required.
This code does not use any libraries as the code is used to directly 
map the display orientation.

The user needs to observe the display 
and relate it to the MAX7219 hardware being exercised through 
instructions and output on the serial monitor. The outcome is a 
recommendation used to set the hardware type for the library.
<hr>

**MD_MAX72XX_Message_ESP8266**  
Use the MD_MAX72XX library to scroll text on the display received 
through the ESP8266 WiFi interface. Text is sent from a web page 
served by the application and displayed as a scrolling message on 
the display. The IP address for the ESP8266 is displayed on the 
scrolling display after startup initialization and WiFi is connected.
<hr>

**MD_MAX72xx_Message_SD**  
Demonstrates the use of the callback function to control what is 
scrolled on the display. Text to be displayed is stored on a SD card file. 
Each line from the text file is scrolled continuously on the display 
and run off before the next one is shown. At end of file the display 
loops back to the first line. The speed for the display is controlled 
by a pot on an analog input.
<hr>

**MD_MAX72xx_Message_Serial**  
Demonstrates the use of the callback function to control what is 
scrolled on the display text. Text typed on the serial monitor 
and will display as a scrolling message on the display. The speed 
for the display is controlled by a pot on an analog input.
<hr>

**MD_MAX72xx_Pacman**
Use the MD_MAX72XX library to display a Pacman animation. Because we can!
<hr>

**MD_MAX72xx_PrintText**
Demonstrates the use of the library to print text. Text typed on 
the serial monitor and this will display as a message on the display.
<hr>

**MD_MAX72xx_PrintText_ML**  
Demonstrates the use of the library to print text on multiple lines by 
using separate matrix displays (no zones). The DAT and CLK lines are 
shared with one LD/CS per string of matrix devices. The user can enter 
text on the serial monitor and this will display as a message on the 
display.
<hr>

**MD_MAX72xx_PushWheel**
Use library to create an mechanical pushwheel type display. 
When numbers change they are scrolled up or down as if on a cylinder.
<hr>

**MD_MAX72xx_RobotEyes**  
Uses a sequence of bitmaps defined as a font to display animations 
of eyes trying to convey emotion. Eyes are coordinated to work together.
<hr>

**MD_MAX72xx_ScrollChart**  
Implements a scroll chart across the display using random numbers. 
Display style can be changed from line to bar chart, triggered by a switch.
<hr>

**MD_MAX72xx_Shift**  
Tests the library shift and transform functions.
<hr>

**MD_MAX72xx_SimplePong**  
Use the library to play Pong using just one 8x8 LED matrix.
The bat is controlled by 2 switches for left and right movement.
Optionally use a pot on analog input to set the speed.
<hr>

**MD_MAX72xx_SimpleSlots**  
Implements a slot machine type display with scrolling symbols 
and simple sound effects. Control is through a digital switch 
and sound uses the Arduino tone() facility.
<hr>

**MD_MAX72xx_Test**  
The main testing sketch for the library. This also demonstrates 
almost all the functions of the library.
<hr>

**MD_MAX72xx_Zones**  
Implements the 'zones' concept from MD_Parola without the Parola library. 
The display is divided into separate sub-displays and managed independently.
<hr>
