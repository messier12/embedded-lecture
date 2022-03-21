#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 128

// For led chips like WS2812, which have a data line, ground, and power, you just
// need to define DATA_PIN. For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// Clock pin only needed for SPI based chipsets when not using hardware SPI
#define DATA_PIN 8
#define CLOCK_PIN 13
#define HEIGHT 16
#define WIDTH 8
#define LEFT -1
#define RIGHT 1

#define BUTTON_P1_LEFT 2
#define BUTTON_P1_RIGHT 3
#define BUTTON_P2_LEFT 4
#define BUTTON_P2_RIGHT 5

int platform_size = 2;
int player_1_pos = 0; //0
int player_2_pos = 0; //HEIGHT
struct Peluru{
    float h;
    float w;
    float v_h;
    float v_w;
} peluru;

// Define the array of leds
CRGB leds[NUM_LEDS];

CRGB player_1_color = CRGB::White;
CRGB player_2_color = CRGB::White;
CRGB peluru_color = CRGB::Red;

CRGB* ledaddress(int h,int w)
{
    if(h%2==0)
    return leds + ((h+1)*WIDTH - w - 1);
    else
    return leds + (h*WIDTH + w);
}

void draw_player_1(int pos)
{
    *ledaddress(0,pos) = player_1_color;
    *ledaddress(0,pos+1) = player_1_color;
}
void draw_player_2(int pos)
{
    *ledaddress(HEIGHT-1,pos) = player_2_color;
    *ledaddress(HEIGHT-1,pos+1) = player_2_color;
}

void move_player_1(int move){
    player_1_pos += move;
    if(player_1_pos<0)
        player_1_pos = 0;
    if(player_1_pos+1>=WIDTH)
        player_1_pos = WIDTH;
}

void move_player_2(int move){
    player_2_pos += move;
    if(player_2_pos<0)
        player_2_pos = 0;
    if(player_2_pos+1>=WIDTH)
        player_2_pos = WIDTH;
}

bool peluru_collide_player()
{
    if(peluru.h == HEIGHT-1)
        if((int)peluru.w == player_2_pos || (int)peluru.w == player_2_pos+1)
            return true;
    if(peluru.h == 0)
        if((int)peluru.w == player_1_pos || (int)peluru.w == player_1_pos+1)
            return true;
    return false;
}
bool peluru_out()
{
  if(peluru.h>=HEIGHT)return true;
  if(peluru.h<0)return true;
  return false;
}
void update_peluru()
{
    peluru.h += peluru.v_h;
    peluru.w += peluru.v_w;
    if(peluru.w<=0)
    {
        peluru.v_w = -peluru.v_w;
        peluru.w = 0;
    }
    if(peluru.w>=WIDTH)
    {
        peluru.v_w = -peluru.v_w;
        peluru.w = WIDTH;
    }
    if (peluru_collide_player())
    {
        peluru.v_h = -peluru.v_h;
        peluru.v_w = random(-10,10)/10.0 * 2;
    }
}
void draw_peluru() {
    int hh = peluru.h;
    int ww = peluru.w;
    *ledaddress(hh,ww) = peluru_color;
}
int test_w;
int test_h;
int test;
void resetall()
{
      player_1_color = CRGB::White;
    player_2_color = CRGB::White;
    peluru_color = CRGB::Red;
    player_1_pos = 0;
    player_2_pos = 0;
    peluru.v_h = 1.0;
    peluru.v_w = -0.5;
    peluru.h = HEIGHT/2;
    peluru.w = WIDTH/2;
    player_1_pos = 0;
    player_2_pos = 0;

}
void setup() {
// Uncomment/edit one of the following lines for your leds arrangement.
// ## Clockless types ##

    pinMode(BUTTON_P1_LEFT ,  INPUT);
    pinMode(BUTTON_P1_RIGHT,INPUT );
    pinMode(BUTTON_P2_LEFT,INPUT);
    pinMode(BUTTON_P2_RIGHT,INPUT); 

    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS); // GRB ordering is assumed
    FastLED.setBrightness(1);
    resetall();

}
bool up_down = 0;
bool down_up = 1;
void clearLED()
{
    for (unsigned int i = 0; i < 128 ; i++)
    {
        leds[i] = CRGB::Black;
    }
}

void _loop()
{
  if(test_w>=WIDTH){test_w=0;test_h++;}
  if(test_h>=HEIGHT)test_h=0;
  
  *ledaddress(test_h,test_w) = CRGB::White;
//  leds[test] = CRGB::White;
  if(digitalRead(BUTTON_P1_LEFT)){
        up_down = 1;
    }
   else
   {
      if(up_down)
        test++;
      up_down=0;
      
   }
  if(test>127)test=0;
  FastLED.show();
  delay(200);
  test_w++;
  clearLED();
  
}
void loop() {
    // Turn the LED on, then pause
    //for (unsigned int i = 1; i < 128 ; i++)
    //{
    //    leds[i] = crgb::white;
    //// delay(500);
    //}
    clearLED();
    if(digitalRead(BUTTON_P1_LEFT)){
        move_player_1(LEFT);
    }
    if(digitalRead(BUTTON_P2_LEFT)){
        move_player_2(LEFT);
    }
    if(digitalRead(BUTTON_P1_RIGHT)){
        move_player_1(RIGHT);
    }
    if(digitalRead(BUTTON_P2_RIGHT)){
        move_player_2(RIGHT);
    }
    update_peluru();
    if(peluru_out()){
      resetall();
      clearLED();
      FastLED.show();
      delay(1000);
    }
      
    draw_peluru();
    draw_player_1(player_1_pos);
    draw_player_2(player_2_pos);

    FastLED.show();
    delay(200);
// // Now turn the LED off, then pause
// leds[127] = CRGB::Black;
// FastLED.show();
// delay(500);
}
