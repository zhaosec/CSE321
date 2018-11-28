#include <LiquidCrystal.h>




/*The circuit for DFPlayer Mini
 *DFPlayer RX pint to pin 1
 *DFPlayer TX pint to pin 0
 *DFPlayer Vcc pint to Vcc
 * DFPlayer Spk_1 pint to 3.5mm jack
 * DFPlayer Spk_2 pint to 3.5mm jack
 * buttonVolumeup to pin 6;
 * buttonVolumedown to pin 5;
 * buttonPause to pin 4;
 *buttonPrevious to pin 3;
 *buttonNext to pin 2;
 */


# define Start_Byte 0x7E
# define Version_Byte 0xFF
# define Command_Length 0x06
# define End_Byte 0xEF
# define Acknowledge 0x00 

String mys[] ={"Daniel Powter - free loop","OneRepublic - Apologize","Yiruma - Kiss the rain"};//shore the songs' name

int buttonVolumeup = 6;
int buttonVolumedown =5;
int buttonPause =4;
int buttonPrevious = 3;
int buttonNext = 2;
int Songstate = 0;
int Volumestate;  //recode the volume
boolean isPlaying = false;//recode the playing state
LiquidCrystal lcd(7,8,9,10,11,12);//RS,EN,DB4,DB5,DB6,DB7

//flag for lcd display
int stringStart, stringStop = 0;
int scrollCursor = 16 ;

//for led
#define msg7RESET 16
#define msg7Strobe 17
#define msg7DCout 0
const int LEDpins[7] = {13,15,13,18,18,19,19}; 





void setup() {
  // initialize digital pin for button.
   pinMode(buttonVolumeup,INPUT);
   pinMode(buttonVolumedown,INPUT);
   pinMode(buttonPause,INPUT);
   pinMode(buttonPrevious,INPUT);
   pinMode(buttonNext,INPUT);
   digitalWrite(buttonVolumeup,HIGH);
   digitalWrite(buttonVolumedown,HIGH);
   digitalWrite(buttonPause,HIGH);
   digitalWrite(buttonPrevious,HIGH);
   digitalWrite(buttonNext,HIGH);
   lcd.begin(16,2);
  Serial1.begin(9600);
//play the first song when plug in power
  playFirst();
  isPlaying = true; 

//for led
for (int x=0; x<7; x++) {
      pinMode(LEDpins[x], OUTPUT);
  }
  pinMode(msg7RESET, OUTPUT);
  pinMode(msg7Strobe, OUTPUT);
 
 

  
}
 
// the loop function runs over and over again forever
void loop() {
 //check the playing state, if true, display the volume state and the song's name
 if(isPlaying == false)
{
 lcd.setCursor(5,0);
 lcd.print("Pause");
  }
  else{
  lcd.setCursor(scrollCursor, 0);
  lcd.print(mys[Songstate].substring(stringStart,stringStop));
  lcd.setCursor(0, 1);
  lcd.print("Vol:");
  lcd.setCursor(4,1);
  lcd.print(Volumestate);
  delay(400);
  lcd.clear();
  //scroll the song's name
  if(stringStart == 0 && scrollCursor > 0){
    scrollCursor--;
    stringStop++;
  } else if (stringStart == stringStop){
    stringStart = stringStop = 0;
    scrollCursor = 16 ;
  } else if (stringStop == mys[Songstate].length() && scrollCursor == 0) {
    stringStart++;
  } else {
    stringStart++;
    stringStop++;
  }
  }

//when buttonPause is press, set the player to pause or resume  
if(digitalRead(buttonPause)==LOW)
     {  if(isPlaying)
                 {pause();                
                  
                 isPlaying =false;}
          else
          {
              play();
              
              isPlaying =true;
          }
      
      }


      
 if (digitalRead(buttonNext) == LOW)
  {
    if(isPlaying)
    { 
      Songstate++;
      playNext();
 //when approach to the end of song, set the songstate to 0
      if(Songstate>2)
      {Songstate = 0;
        }
     
      
    }
  }

   if (digitalRead(buttonPrevious) == LOW)
  {
    if(isPlaying)
    {
      playPrevious();
      Songstate--;
      if(Songstate==-1)
      {Songstate = 2;
        }
      
      
    }
  }
    
    
   //when buttonvolumeup is press, increase volume 
    if(digitalRead(buttonVolumeup)==LOW)
    {    if(Volumestate <30)
             {setVolume(Volumestate);
               Volumestate++;
             }
          else if(Volumestate >29)
          {   setVolume(30);
              Volumestate = 30;
            }
      
      }

    //when buttonvolumeup is press, decrease volume
   if(digitalRead(buttonVolumedown)==LOW)
    {    if(Volumestate >0)
             {setVolume(Volumestate);
               Volumestate--;
             }
          else if(Volumestate <0)
          {   setVolume(0);
              Volumestate = 0;
            }
      
      }
   


   //for led
   digitalWrite(msg7RESET, HIGH);          // reset the MSGEQ7's counter
    delay(5);
    digitalWrite(msg7RESET, LOW);
 
    for (int x = 0; x < 7; x++){
        digitalWrite(msg7Strobe, LOW);      // output each DC value for each freq band
        delayMicroseconds(35); // to allow the output to settle
        int spectrumRead = analogRead(msg7DCout);
 
        int PWMvalue = map(spectrumRead, 0, 1024, 0, 255);  // scale analogRead's value to Write's 255 max
        if (PWMvalue < 50)
            PWMvalue = PWMvalue / 2;        // bit of a noise filter, so the LEDs turn off at low levels
 
        analogWrite(LEDpins[x], PWMvalue);
        digitalWrite(msg7Strobe, HIGH);
    }
    
   
}
//for led



// send the cmd when the button was press
void playFirst()
{
  execute_CMD(0x3F, 0, 0);
  delay(400);
  setVolume(1);
  Volumestate = 1;
  delay(400);
  execute_CMD(0x11,0,1); 
  delay(400);
}

void pause()
{
  execute_CMD(0x0E,0,0);
  delay(400);
}

void play()
{
  execute_CMD(0x0D,0,1); 
  delay(400);
}

void playNext()
{
  execute_CMD(0x01,0,1);
  delay(400);
}

void playPrevious()
{
  execute_CMD(0x02,0,1);
  delay(400);
}

void setVolume(int volume)
{
  execute_CMD(0x06, 0, volume); // Set the volume (0x00~0x30)
  delay(400);
}



//function to send the cmd to seial1.
void execute_CMD(byte CMD, byte Par1, byte Par2)
{
// Calculate the checksum (2 bytes)
word checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
// Build the command line
byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge,
Par1, Par2, highByte(checksum), lowByte(checksum), End_Byte};
//Send the command line to DFplayer
for (byte k=0; k<10; k++)
{
Serial1.write( Command_line[k]);
}
}
