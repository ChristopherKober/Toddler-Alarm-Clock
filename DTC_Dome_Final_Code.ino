
//////////////////////////////////////////////////////////////////////////

//User Changing Definitions
#define NIGHTLIGHTTIME 30 //The amount of time the night light activates in 
                          //minutes can be from 0-59
#define SOUNDBARRIER 400//The amount of noise you have to make for the light 
                         //to turn on minimum is 0 and max is 512
                          
                          
// Default Times
int bedTime[] = {9,30,1}; // hours;minutes; 0-AM,1-PM
int wakeTime[] = {7,0,0}; // hours;minutes; 0-AM,1-PM
int hardTime[] = {11,58,0}; // hours;minutes; 0-AM,1-PM

////////////////////////////////////////////////////////////////////////

// Include Time Library
#include <Time.h>

// Include I2C LED Libraries
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


//Include LED libraries
#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

//////////////////////////////////////////////////////////////////////////


int oldTime;

bool night = false;

#define MODE 9
#define PLUS 7
#define MINUS 11
#define NIGHTLIGHT 10


//Light Definitions
#define PIN            13
#define NUMPIXELS      4

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);




#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);


#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


void setup()   {    
  pinMode(7,INPUT);
  pinMode(MODE,INPUT);
  pinMode(PLUS,INPUT);
  pinMode(MINUS,INPUT);
  pinMode(NIGHTLIGHT,INPUT);

  setTime(11,58,00,12,5,2015);


  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)
  // init done
  
  //Clearing the display buffer
  display.display();
  delay(2);
  display.clearDisplay();
  
  //initializing the NeoPixel Library
  pixels.begin();
  
  pixels.clear();
  pixels.show();

}


void loop() {
  display.clearDisplay();
  drawCurTime();
  display.display();
  if (digitalRead(NIGHTLIGHT)==HIGH){
    nightLight();
  }
  if (digitalRead(MODE) == HIGH){
    set_Time();
  }
  display.clearDisplay();
  drawCurTime();
  display.display();
  nightTimeDetection();
  delay(500);
    
}

void displayGreenLight(){
  for (int i = 0; i<4;i++){
    pixels.setPixelColor(i,pixels.Color(200,0,0));
  }
  pixels.show();
  hardTime[0] = hourFormat12();
  hardTime[1] = minute() + 20; // 20 minute green light display
  if (hardTime[1] > 59){
    hardTime[1] = hardTime[1] - 60;
    hardTime[0] = hardTime[0] + 1;
  }
  if (hardTime[0] == 13){
    hardTime[0] = 1;
  }
  while ((hardTime[0] != hourFormat12() || hardTime[1] != minute() ) && digitalRead(MODE) != HIGH){
    delay(500);
  }
  pixels.clear();  
  pixels.show();
}

void displayRedLight(){
  for (int i = 0; i<4;i++){
    pixels.setPixelColor(i,pixels.Color(0,200,0));
  }
  pixels.show();
  hardTime[0] = hourFormat12();
  hardTime[1] = minute() + 5; // 5 minute red light display
  if (hardTime[1] > 59){
    hardTime[1] = hardTime[1] - 60;
    hardTime[0] = hardTime[0] + 1;
  }
  if (hardTime[0] == 13){
    hardTime[0] = 1;
  }
  while ((hardTime[0] != hourFormat12() || hardTime[1] != minute() ) && digitalRead(MODE) != HIGH && nightTime()){
    delay(500);
  }
  pixels.clear();
  pixels.show();
}

void nightTimeDetection(){
  if (nightTime()){
    night = true;
    if (detectNoise()){
      displayRedLight();
    }
  }
  else{
    if (night){
      displayGreenLight();
    }
    night = false;
  }

}

bool nightTime(void){
  int bed = 100*bedTime[0] + bedTime[1] + 1200*bedTime[2];
  int wake = 100 * wakeTime[0] + wakeTime[1] + 1200*wakeTime[2];
  int actual = 100 * hour() + minute();
  if (bed/100 == 12 || bed/100 == 24){
    bed = bed - 1200;
  }
  if (wake/100 == 12 || wake/100 == 24){
    wake = wake - 1200;
  }
  if (((bed < actual || actual < wake) && bed > wake) || ((bed < actual && actual < wake) && bed < wake)){
    return true;
  }
  return false;
}

bool detectNoise(void){
  int highValue = 512+SOUNDBARRIER;
  int lowValue = 512-SOUNDBARRIER;
  int temp;
  int counter = 0;
  for (int i = 0;i<100;i++){
    temp = analogRead(A3);
    if (temp > highValue || temp < lowValue) {counter++;}
    delay(100);
  }
  if (counter > 20){
    return true;
  }
  return false;
}


void nightLight(void){
  hardTime[0] = hourFormat12();
  hardTime[1] = minute() + NIGHTLIGHTTIME;
  if (hardTime[1] > 59){
    hardTime[1] = hardTime[1] - 60;
    hardTime[0] = hardTime[0] + 1;
  }
  if (hardTime[0] == 13){
    hardTime[0] = 1;
  }
  for (int i =0;i<4;i++){
    pixels.setPixelColor(i,pixels.Color(0,150,0));
  }
  pixels.show();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.clearDisplay();
  char text[] = "Night Light: Press Mode Button to Quit";
  for (int i=0;i<38;i++){
    display.write(text[i]);
  }
  display.display(); 
  while (hardTime[0] != hourFormat12() || hardTime[1] != minute()) { 
    if (digitalRead(MODE) == HIGH){ 
       display.clearDisplay();
       display.write('.');
       display.display();
       delay(2000);
       break; 
     }
    delay(1000); 
  }
  pixels.clear();
  pixels.show();
}


void set_Time(void){
    int mode_num = 1; // 1 - Hard Time; 2 - Bed Time; 3 - Wake time
    hardTime[0] = hourFormat12();
    hardTime[1] = minute();
    hardTime[2] = 1;
    if (isAM()){ hardTime[2] = 0;}
    delay(1000);
    while (true){
      if(digitalRead(MODE) == HIGH){
        mode_num = mode_num + 1;
        if (mode_num == 3){
          mode_num = 0;
        }
        display.clearDisplay();
        drawCurMode(mode_num);
        display.display();
        delay(1000);
        continue;
      }
      if (digitalRead(PLUS) == HIGH || digitalRead(MINUS) == HIGH){
        break;
      }
    }
    if (mode_num == 1){
      while (true) {
        oldTime = hardTime[0];
        if (digitalRead(PLUS) == HIGH){
          hardTime[0] = hardTime[0] + 1;
          display.clearDisplay();
          drawCurMode(mode_num);
          display.display();
          delay(1000);
        }
        if (digitalRead(MINUS) == HIGH){
          hardTime[0] = hardTime[0] - 1;
          display.clearDisplay();
          drawCurMode(mode_num);
          display.display();
          delay(1000);  
        }
        if (digitalRead(MODE) == HIGH){
          delay(1000);
          break;
        }
      }
      while (true){
        oldTime = hardTime[0];
        if (digitalRead(PLUS) == HIGH){
          hardTime[1] = hardTime[1] + 1;
          display.clearDisplay();
          drawCurMode(mode_num);
          display.display();
          delay(1000);
        }
        if (digitalRead(MINUS) == HIGH){
          hardTime[1] = hardTime[1] - 1;
          display.clearDisplay();
          drawCurMode(mode_num);
          display.display();
          delay(1000);  
        }
        if (digitalRead(MODE) == HIGH){
          delay(1000);
          hardTime[0] = hardTime[0] + 12*hardTime[2];
          setTime(hardTime[0],hardTime[1],00,12,5,2015);
          break;
        }
      }
    }
    else if (mode_num == 2){
      while (true){
        oldTime = bedTime[0];
        if (digitalRead(PLUS) == HIGH){
          bedTime[0] = bedTime[0] + 1;
          display.clearDisplay();
          drawCurMode(mode_num);
          display.display();
          delay(1000);
        }
        if (digitalRead(MINUS) == HIGH){
          bedTime[0] = bedTime[0] - 1;
          display.clearDisplay();
          drawCurMode(mode_num);
          display.display();
          delay(1000);  
        }
        if (digitalRead(MODE) == HIGH){
          delay(1000);
          break;
        }        
      }
      while (true){
        oldTime = bedTime[0];
        if (digitalRead(PLUS) == HIGH){
          bedTime[1] = bedTime[1] + 1;
          display.clearDisplay();
          drawCurMode(mode_num);
          display.display();
          delay(1000);
        }
        if (digitalRead(MINUS)== HIGH){
          bedTime[1] = bedTime[1] - 1;
          display.clearDisplay();
          drawCurMode(mode_num);
          display.display();
          delay(1000);
        }
        if (digitalRead(MODE) == HIGH){
          delay(1000);
          break;
        }
      }
    }
    else if (mode_num == 0){
      while (true){
        oldTime = wakeTime[0];
        if (digitalRead(MODE) == HIGH){
          delay(1000);
          break;
        }
        if (digitalRead(PLUS)==HIGH){
          wakeTime[0] = wakeTime[0] + 1;
          display.clearDisplay();
          drawCurMode(mode_num);
          display.display();
          delay(1000);
        }
        if (digitalRead(MINUS)==HIGH){
          wakeTime[0] = wakeTime[0] - 1;
          display.clearDisplay();
          drawCurMode(mode_num);
          display.display();
          delay(1000);
        }
        
      }
      while (true){
        oldTime = wakeTime[0];
        if(digitalRead(PLUS)==HIGH){
          wakeTime[1] = wakeTime[1] + 1;
          display.clearDisplay();
          drawCurMode(mode_num);
          display.display();
          delay(1000);
        }
        if (digitalRead(MINUS)==HIGH){
          wakeTime[1] = wakeTime[1] - 1;
          display.clearDisplay();
          drawCurMode(mode_num);
          display.display();
          delay(1000);
        }
        if (digitalRead(MODE)==HIGH){
          delay(1000);
          break;
        }
      }
    
    }
}




void drawCurTime(void){
  char Time[2];
  sprintf(Time, "%i",hourFormat12());
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0,24);
  if (hourFormat12() < 10){
    display.write(Time[0]);
    display.write(':');
  }
  else{
    display.write(Time[0]);
    display.write(Time[1]);
    display.write(':');
  }
  sprintf(Time,"%i",minute());
  if (minute() < 10){
    display.write('0');
    display.write(Time[0]);
  }
  else{
    display.write(Time[0]);
    display.write(Time[1]);
  }
  display.setTextSize(1);
  display.setCursor(100,0);
  if (isAM()){
    display.write('A');
    display.write('M');
  }
  else {
    display.write('P');
    display.write('M');  
  }
}

void drawCurMode(int curMode){
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  if (curMode == 0){
    char m[] = "Wake Up Time";
    for (int i = 0; i<12;i++){
      display.write(m[i]);
    }
    
    display.setTextSize(4);
    display.setTextColor(WHITE);
    display.setCursor(0,24);
    char time[2];
    if (wakeTime[1] == 60){
      wakeTime[1] = 0;
    }
    if (wakeTime[1]==-1){
      wakeTime[1] = 59;
    }
    if (wakeTime[0] == 13){
      wakeTime[0] = 1;
    }
    if (wakeTime[0] == 0){
      wakeTime[0] = 12;
    }
    if (oldTime == 11 && wakeTime[0] == 12 || oldTime == 12 && wakeTime[0]==11){
      wakeTime[2] = 1 - wakeTime[2];
    }
    sprintf(time,"%i",wakeTime[0]);
    display.write(time[0]);
    display.write(time[1]);
    display.write(':');
    sprintf(time,"%i",wakeTime[1]);
    if (wakeTime[1]<10){display.write('0');}
    display.write(time[0]);
    display.write(time[1]);
    if (wakeTime[2] == 0){
      display.setTextSize(1);
      display.setCursor(100,0);
      display.write('A');
      display.write('M');
    }
    else if (wakeTime[2] == 1) {
      display.setTextSize(1);
      display.setCursor(100,0);
      display.write('P');
      display.write('M');      
    }
    
  }
  if (curMode == 1){
      char m[] = "Actual Time";
      for (int i = 0; i<11;i++){
        display.write(m[i]);
      }
      display.setTextSize(4);
      display.setTextColor(WHITE);
      display.setCursor(0,24);
      if (hardTime[1] == 60){
        hardTime[1] = 0;
      }
      if (hardTime[1] == -1){
        hardTime[1] = 59;
      }
      if (hardTime[0]==13){
        hardTime[0] = 1;
      }
      if (hardTime[0] == 0){
        hardTime[0] = 12;
      }
      if (oldTime == 11 && hardTime[0] == 12 || oldTime == 12 && hardTime[0]==11){
        hardTime[2] = 1 - hardTime[2];
      }
      char time[2];
      sprintf(time,"%i",hardTime[0]);
      display.write(time[0]);
      display.write(time[1]);
      display.write(':');
      sprintf(time,"%i",hardTime[1]);
      if (hardTime[1]<10){display.write('0');}
      display.write(time[0]);
      display.write(time[1]);
      if (hardTime[2] == 0){
        display.setTextSize(1);
        display.setCursor(100,0);
        display.write('A');
        display.write('M');
      }
      else if (hardTime[2] == 1)  {
        display.setTextSize(1);
        display.setCursor(100,0);
        display.write('P');
        display.write('M');      
      }
  }
  if (curMode == 2){
      char m[] = "Bed Time";
      for (int i = 0; i<8;i++){
        display.write(m[i]);
      }
    if (bedTime[1] == 60){
      bedTime[1] = 0;
    }
    if (bedTime[1]==-1){
      bedTime[1] = 59;
    }
    if (bedTime[0] == 13){
      bedTime[0] = 1;
    }
    if (bedTime[0] == 0){
      bedTime[0] = 12;
    }
    if (oldTime == 11 && bedTime[0] == 12 || oldTime == 12 && bedTime[0]==11){
      bedTime[2] = 1 - bedTime[2];
    }
      
      display.setTextSize(4);
      display.setTextColor(WHITE);
      display.setCursor(0,24);
      char time[2];
      sprintf(time,"%i",bedTime[0]);
      //if (bedTime[0] < 10){display.write('0');}
      display.write(time[0]);
      display.write(time[1]);
      display.write(':');
      sprintf(time,"%i",bedTime[1]);
      if (bedTime[1] < 10){display.write('0');}
      display.write(time[0]);
      display.write(time[1]);
      if (bedTime[2] == 0){
        display.setTextSize(1);
        display.setCursor(100,0);
        display.write('A');
        display.write('M');
      }
      else if(bedTime[2] ==1 )  {
        display.setTextSize(1);
        display.setCursor(100,0);
        display.write('P');
        display.write('M');      
      }
  }
}
