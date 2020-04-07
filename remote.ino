
/*
 pres a key then system waits 4 sec for Reciving IR Data to save in that key then that key sends that specific code next time
ir reciver pin 11
ir sender LED pin 3 and with a resistor (470 ohm maybe) to ground
keypad cols: 2,8,4
keypad Rows: 5,6,7,9
*/
#include <IRremote.h>
///-----------------------------------------------------
#include <Keypad.h>
const byte ROWS = 4; //four rows
const byte COLS = 3; //four columns

///char hexaKeys[ROWS][COLS] = { 
///  {'12','10','11'},
///  {'9','8','7'},
///  {'6','5','4'},
///  {'3','2','1'}

byte hexaKeys[ROWS][COLS] = {
  {12,10,11},
  {9,8,7},
  {6,5,4},
  {3,2,1}
}; //define the cymbols on the buttons of the keypads
byte rowPins[ROWS] = {5, 6, 7, 9}; ///connect to the row pinouts of the keypad
byte colPins[COLS] = {2, 8, 4}; ///connect to the column pinouts of the keypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); //initialize an instance of class NewKeypad
///-----------------------------------------------------
IRrecv irrecv(11); //-- RECV_PIN
IRsend irsend;
decode_results results;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver - Reciver pin 11
  pinMode(LED_BUILTIN, OUTPUT); // STATUS_PIN
}

byte customKey;
int toggle = 0; // The RC5/6 toggle state
//---------------------------------------------------------------------------------------------------------------
// Storage for the recorded code
int codeType[] = {0,0,0,0,0,0,0,0,0,0,0,0,0}; // The type of code
// unsigned int rawCodes[3][RAWBUF]; // The durations if raw
int codeLen[13]; // The length of the code
unsigned long codeValue[13]; // The code value if not raw  ///  
int tmpcodtype=0;
//----------------------------Stores the code for later playback-------------------------------------------------
// Most of this code is just logging
void storeCode(decode_results *results) {
  tmpcodtype = results->decode_type;
if (tmpcodtype == UNKNOWN) {
  ////-- UNKNOWN Code - Do nothing
}else{
  codeType[customKey] = tmpcodtype ;
  // Serial.println(codeType[customKey]);
  if (codeType[customKey] == UNKNOWN) {
    Serial.println("Received unknown code, saving as raw");
    /*
    codeLen[customKey] = results->rawlen - 1;
    for (int i = 1; i <= codeLen[customKey]; i++) {
      if (i % 2) {
        // Mark
        rawCodes[customKey][i - 1] = results->rawbuf[i]*USECPERTICK - MARK_EXCESS;
        Serial.print(" m");
      } 
      else {
        rawCodes[customKey][i - 1] = results->rawbuf[i]*USECPERTICK + MARK_EXCESS;
        Serial.print(" s");
      }
      Serial.print(rawCodes[customKey][i - 1], DEC);
    }
    */
    Serial.println("");
  }
  else {
    if (codeType[customKey] == NEC) {
      Serial.print("Received NEC: ");
      if (results->value == REPEAT) {
        // Don't record a NEC repeat value as that's useless.
        Serial.println("repeat; ignoring.");
        return;
      }
    } 
    else if (codeType[customKey] == SONY) {
      Serial.print("Received SONY: ");
    } 
    else if (codeType[customKey] == PANASONIC) {
      Serial.print("Received PANASONIC: ");
    }
    else if (codeType[customKey] == JVC) {
      Serial.print("Received JVC: ");
    }
    else if (codeType[customKey] == RC5) {
      Serial.print("Received RC5: ");
    } 
    else if (codeType[customKey] == RC6) {
      Serial.print("Received RC6: ");
    } 
    else {
      Serial.print("Unexpected codeType ");
      Serial.print(codeType[customKey], DEC);
      Serial.println("");
    }
    Serial.println(results->value, HEX);
    codeValue[customKey] = results->value;
    codeLen[customKey] = results->bits;
  }
}
}
///-----------------------------FINISH STORAGE------------------------------------
///-------------------------------------------------------------------------------

void sendCode(byte repeat) {
  if (codeType[customKey] == NEC) {
    if (repeat) {
      irsend.sendNEC(REPEAT, codeLen[customKey]);
      Serial.println("Sent NEC repeat");
    } 
    else {
      irsend.sendNEC(codeValue[customKey], codeLen[customKey]);
      Serial.print("Sent NEC ");
      Serial.println(codeValue[customKey], HEX);
    }
  } 
  else if (codeType[customKey] == SONY) {
    irsend.sendSony(codeValue[customKey], codeLen[customKey]);
    Serial.print("Sent Sony ");
    Serial.println(codeValue[customKey], HEX);
  } 
  else if (codeType[customKey] == PANASONIC) {
    irsend.sendPanasonic(codeValue[customKey], codeLen[customKey]);
    Serial.print("Sent Panasonic");
    Serial.println(codeValue[customKey], HEX);
  }
  else if (codeType[customKey] == JVC) {
    irsend.sendJVC(codeValue[customKey], codeLen[customKey], false);
    Serial.print("Sent JVC");
    Serial.println(codeValue[customKey], HEX);
  }
  else if (codeType[customKey] == RC5 || codeType[customKey] == RC6) {
    if (!repeat) {
      // Flip the toggle bit for a new button press
      toggle = 1 - toggle;
    }
    // Put the toggle bit into the code to send
    codeValue[customKey] = codeValue[customKey] & ~(1 << (codeLen[customKey] - 1));
    codeValue[customKey] = codeValue[customKey] | (toggle << (codeLen[customKey] - 1));
    if (codeType[customKey] == RC5) {
      Serial.print("Sent RC5 ");
      Serial.println(codeValue[customKey], HEX);
      irsend.sendRC5(codeValue[customKey], codeLen[customKey]);
    } 
    else {
      irsend.sendRC6(codeValue[customKey], codeLen[customKey]);
      Serial.print("Sent RC6 ");
      Serial.println(codeValue[customKey], HEX);
    }
  } 
  else if (codeType[customKey] == UNKNOWN /* i.e. raw */) {
    
    // Assume 38 KHz
    // irsend.sendRaw(rawCodes[customKey], codeLen[customKey], 38);
    Serial.println("Sent raw");
  }
}
///------------------------------------------LOOP------------------------------------------------------------
byte lastButtonState;
byte canstore=0;
unsigned long thisMillis = millis();
  
void loop() {
  byte buttonState = 0;
  byte keycode = 0;  
  char CurKey = customKeypad.getKey();

  
    switch (customKeypad.getState()){
    case PRESSED:
        Serial.println("--------P Resed---------");
        break;
    case RELEASED:
        Serial.println("--------R Elease---------");
        break;
    case HOLD:
        Serial.println("--------H Old---------");
        break;
    }
  
  if(millis() < thisMillis)  {   
      /// ----- if reset milis -------
      thisMillis = millis();
  }
   
  if(canstore==1)  {
      if(millis()-thisMillis > 4000)      {   
          /// ---- if 4 secound passed Disable The Record Mode ----
          thisMillis = millis();
          canstore=0; /// Disable Saveing
          Serial.println("can store OFF");
          digitalWrite(LED_BUILTIN, LOW);   
      }
  }
  
  if(CurKey==""){
    keycode = 0;
  }else{
          keycode = CurKey;
  }
        
    if (keycode>0){   
        ///---------------------- key pressed ----
       Serial.println("Got Key code: ");
       Serial.println(keycode);
       customKey = keycode ;
       digitalWrite(LED_BUILTIN, HIGH); 
       buttonState=1;
       canstore=1;
       thisMillis = millis();
       Serial.println("Store is on: Reset Milis ");
    }else    {
        buttonState=2;   ///---- key Release----
    }

    if (lastButtonState == 1 && buttonState == 2)     {
       irrecv.enableIRIn(); // //// Butom Goes From Hi 2 Lo - Key Released -> Enable Reciver
       Serial.println("Enable reciver");
    }

    if (buttonState==1) 
    {  
        //// ---------- key Is pressed -----------
        Serial.println("SENDING CODE");
        sendCode(lastButtonState == buttonState); /// Parameter: True or 1-> Repeats , False or 0->Sends Code
        delay(50); // Wait a bit between retransmissions
    }else{
        if(irrecv.decode(&results)) /// key IS Open (UnPressed) Read The IR AND wait if something Recived then Save The Code
        {
          if(canstore==1){
            Serial.println("SAVE CODE: Custom Key is:");
            Serial.println(customKey);            
            storeCode(&results);
            if (tmpcodtype == UNKNOWN) {
              thisMillis =1000+thisMillis ; ///---- Wait 1 More Sec - Cod not detected
            }else{
              thisMillis = 0;  /// Disable Saveing
            }
          }
          irrecv.resume(); // resume receiveing data
        }
    }
    lastButtonState = buttonState;
}
