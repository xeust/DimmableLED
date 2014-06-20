/*
spark-torch.ino is a program that runs as firmware for the CloudLamp. It uses an array with brightness values to have a dimming curve approximated
as (e^x - 1) / 1.578. It also has a spark function "turnTo" that allows for remote dimmability. The light engine is two Adafruit Neopixel LED boards.
*/

// This #include statement was automatically added by the Spark IDE.
#include "neopixel/neopixel.h"

#include "math.h"


#define NEOPIXEL_PIN A0 //data pin for the neopixel LEDs
#define NEOPIXEL_COUNT 32 //# of LEDs
#define NEOPIXEL_TYPE WS2812B //type of LEDs

double lightState = 0; //variable for keeping track of the brightness value for the lamp.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEOPIXEL_TYPE); //setup neopixel strip
uint32_t color = strip.Color(255, 205, 152); //set color of LEDs to imitate tungsten light via RGB 
int top = D0; // Define encoder pin A (The top pin)
int bottom = D1;// Define encoder pin B (The bottom pin)
int count = 0; // pre-initiate the count of the encoder to zero for debugging
int topVal = LOW; // pre-initiate the state of pin A to low
int topLast = LOW; // make its last value the same
int bottomVal = LOW; // make the bottom value low 
int inputPin = D2; //pin for reading value of pushbutton fucntionality
int point = 0; //index for array based dim value
int val = 0; //pushbutton state value
double brightness[] = {0.0, 0.11493046921721886, 0.250704760561023, 0.411103115457414, 0.6005913609774866, 0.8244452478422774, 1.0888973384030416, 1.401310534266823, 1.7703830749408593, 2.206390715088621, 2.721472823120924, 3.3299703667077485, 4.048825195437263, 4.898051737489861, 5.901294243353279, 7.086485091414136, 8.486622484036182, 10.14068918681737, 12.094737890656232, 14.403173415386968, 17.130267454225375, 20.351948032692242, 24.157913504308894, 28.654129941144937, 33.965781451816035, 40.24075556998111, 47.653760753874806, 56.41119063670973, 66.75687045889588, 78.97884567481861, 93.41740174324569, 110.4745383891715, 130.62516212044608, 154.43030862235864, 182.55276316941087, 215.77551395934938, 255.0};

/*
 * setup() - this function runs once when you turn your Arduino one
 * We set the motors pin to be an output (turning the pin high (+5v) or low (ground) (-))
 * rather than an input (checking whether a pin is high or low)
 */
void setup(){
    
    strip.begin();
    pinMode(top, INPUT);  // setup the pins as inputs
    pinMode(bottom, INPUT);
    pinMode(inputPin, INPUT);
    
    
    for(int i = 0; i < strip.numPixels(); i++) {//set pixels to given RGB color
        strip.setPixelColor(i, color);
    }
    
    strip.setBrightness(lightState);//set LEDs to lightState(init. 0)
    strip.show();
    Spark.function("turnTo", turnTo); //Setup Spark turnTo function for remote dimmability
    Serial.begin(9600); //Serial for debugging
}


/*
 * loop() - this function will start after setup finishes and then repeat
 * we call a function called motorOnThenOff()
 */

void loop(){
    topVal = digitalRead(top); // read pin A as top
    bottomVal = digitalRead(bottom);  // read pin B as bottom
    val = digitalRead(inputPin); //read value of pushbutton

    if (topVal != topLast) {// top pin has changed value; rotary encoder has spun
        if (bottomVal) {//If bottomVal is set to HIGH
            if (topVal == HIGH){
                turnDown(); //Encoder must be turning left so dim Lamp
                count--; //decrement the encoder count
            } else {
                turnUp(); //Encoder must be turning right so brighten lamp
                count++; //increment the encoder count
            }
        } else { //If bottomVal is set to LOW
            if (topVal == HIGH){
                turnUp(); //Encoder must be turning right so brighten lamp
                count++; //increment the encoder count
            } else {
                turnDown(); //Encoder must be turning left so dim Lamp
                count--; //decrement the encoder count
            }
        }
        Serial.print("count:"); //Print encoder count for debugging
        Serial.println(count);
        topLast = topVal;  // store current top state for next pass
    }
    
    if (val == LOW) { //if clause activated if pushbutton pressed and thus inputPin reads LOW
        if (lightState >= 1){ //If light on
            point = 5; //set index of array to 5; point where no visible light borders visible light
            lightState = 0; //set local state brightness to 0
        } else if (lightState < 1) { //if light off
            point = 37; //set index of array to max value
            lightState = 255; //set local state brightness to full
        }
        for(int i = 0; i < strip.numPixels(); i++) { //set each pixel to color
            strip.setPixelColor(i, color);
        }
        strip.setBrightness(lightState);//set brightness ot lightState
        strip.show();
        Serial.println(lightState);//print light state for debugging
        delay(250); //primitive button debouncing
    }

}


/*
turnUp() is a function for turning the lamp up, using a point value to index the brightness state from the brightness array.

*/
int turnUp(){
    if (lightState < 247) {//if light less than full
        point++; //increment index
        lightState = brightness[point]; //set local state brightness to new value
    } else if (lightState >= 247){ // if light full
        point = 37; //set index to max value
        lightState = 255; //set local state to max value
    }
    for(int i = 0; i < strip.numPixels(); i++) {//set LEDs to color
        strip.setPixelColor(i, color);
    }
    
    strip.setBrightness(lightState); //set brightness of LEDs to lightState
    strip.show();
    Serial.println(lightState); //print for debugging
    return 1;
}

/*
turnDown() is a function for turning the lamp down, using a point value to index the brightness state from the brightness array.

*/
int turnDown(){
    if (lightState > 0) {//if lamp on
        point--;//decrement index
        lightState = brightness[point];//set local state brightness to new value
    } else if (lightState <= 0) { // if lamp off
        lightState = 0;//set local brightness state to 0
        point = 5; //set index to 5, boundary between visible light and none for dimming function.
    }
    
    for(int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, color);
    }
    
    strip.setBrightness(lightState);
    strip.show();
    Serial.println(lightState);
    return 1;
}

/*
turnTo() is a handler function for the Spark.function "turnTo" turning the lamp to a given state.
The sole parameter "command" is supposed to be a string representation of an integer between 0 and 255 that
represents the state to write to the lamp.

*/
int turnTo(String command){
    int state = atoi(command.c_str());//parse the string into an integer
    
    for(int i = 0; i < strip.numPixels(); i++) {//set LEDs to color
        strip.setPixelColor(i, color);
    }
    
    if (state < 0 || state > 255) {//if state out of bounds, return
        return 1;
    }
    
    lightState = state;//else set local state equal to state
    point = indexRet(lightState);//set the index of the array to the closest brightness to current brightness #see indexRet().
    Serial.println("State: " + String(state)); //print for debugging
    Serial.println("lightState: " + String(lightState));
    strip.setBrightness(lightState);//set brightness to lightState
    strip.show();
    return 1;
}

/*
indexRet() takes in a given lightState and returns the index of the array that is closest to a given brightness. 
This ensures a smooth transition between physical and cloud dimming; no sporadic jumps between brightness.
*/
int indexRet(double lightState){
    double index = log(1.578*lightState + 1); //conversion function for returning raw value between 0 and 6 (1/6 steps)
    int point = (int) 6*index; //adjustment for returning index between 0 and 36
    return point;
}
