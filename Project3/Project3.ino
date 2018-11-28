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


//analogPin
int analogPin = 14; // MSGEQ7 OUT
int strobePin = 15; // MSGEQ7 STROBE
int resetPin = 16; // MSGEQ7 RESET
int spectrumValue[7];

int filterValue = 80;

//analogpin 
int ledPinR = 17;
int ledPinG = 18;
int ledPinB = 19;




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
Serial.begin(9600);
  // Read from MSGEQ7 OUT
  pinMode(analogPin, INPUT);
  // Write to MSGEQ7 STROBE and RESET
  pinMode(strobePin, OUTPUT);
  pinMode(resetPin, OUTPUT);
 
  // Set analogPin's reference voltage
  analogReference(AR_DEFAULT); // 5V
 
  // Set startup values for pins
  digitalWrite(resetPin, LOW);
  digitalWrite(strobePin, HIGH);

  
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
    // Set reset pin low to enable strobe
  digitalWrite(resetPin, HIGH);
  digitalWrite(resetPin, LOW);
 
  // Get all 7 spectrum values from the MSGEQ7
  for (int i = 0; i < 7; i++)
  {
    digitalWrite(strobePin, LOW);
    delayMicroseconds(3000); // Allow output to settle
 
    spectrumValue[i] = analogRead(analogPin);
 
    // Constrain any value above 1023 or below filterValue
    spectrumValue[i] = constrain(spectrumValue[i], filterValue, 1023);
 
 
    // Remap the value to a number between 0 and 255
    spectrumValue[i] = map(spectrumValue[i], filterValue, 1023, 0, 255);
 
    // Remove serial stuff after debugging
    Serial.print(spectrumValue[i]);
    Serial.print(" ");
    digitalWrite(strobePin, HIGH);
   }
 
   Serial.println();
 
   // Write the PWM values to the LEDs
   // I find that with three LEDs, these three spectrum values work the best
   analogWrite(ledPinR, spectrumValue[1]);
   analogWrite(ledPinG, spectrumValue[4]);
   analogWrite(ledPinB, spectrumValue[6]);
   
}


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
