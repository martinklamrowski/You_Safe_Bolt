#include <Keypad.h>   //Keypad library
#include <Wire.h> //Wire Pin Libarary
#include <LiquidCrystal_I2C.h> //LCD library
#include <Adafruit_Fingerprint.h> //Fingerprint library
#include <SoftwareSerial.h> //Library to enable the Arduino to talk with the Fingerprint scanner
SoftwareSerial mySerial(14,15); //Specifying what pins are used for the connection of the Fingerprint scanner and the Arduino

//creating an instance of class Adafruit_Fingerprint which is from the Adafruit_Fingerprint library 
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

//Stepper Motor pins 
#define IN1  2
#define IN2  3
#define IN3  4
#define IN4  5

//address,enable,read/write,reset,data 4,data 5,data 6,data 7,backlight,backlight pol
LiquidCrystal_I2C lcd(0x20, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); //Hardware set up = SDA -> A4 / SCL -> A5

//Global Variables
int revolutionSteps = 4096;   //Amount of steps required to complete a full 360 degree rotation
int motorSteps = 0;           //going through 7 different cases to send voltages to certain pins on the motor
String pinCode = "";          //the pin that is entered on the keypad
String packetReceieved = "";  //the string packet that is receieved from the RPi
int accessOrChange = true;    //fingerprint scanner variable - true means user wants to scan to gain access - false means user wants to add/remove fingerprint user

//Set up Keypad
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  { '1','2','3',' '},
  { '4','5','6',' '},
  { '7','8','9',' '},
  { 'A','0','B',' '}
};

byte rowPins[ROWS] = {6, 7, 8, 9}; //Hardware set up = y1 y2 y3 y4 //connect to the row pinouts of the keypad
byte colPins[COLS] = {10, 11, 12, 13}; //Hardware set up = x1 x2 x3 x4 //connect to the column pinouts of the keypad

//initialize an instance of class Keypad which is from the Keypad library 
Keypad myKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

void setup()
{
  Serial.begin(9600); //sets data rate for serial
  finger.begin(57600); //sets data rate for fingerprint scanner

  //Set Motor driver pins as output pins (sent voltages to pin)
  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT); 
  pinMode(IN3, OUTPUT); 
  pinMode(IN4, OUTPUT); 
  
  lcd.begin(16,2);      //Initialize the lcd
  welcome();            //Display welcome message on lcd
}

void loop() {
  //If the fingerprint match is found and the user wants to gain access
  if (findFingerMatch() == true && accessOrChange == true) {
    AccessGranted(""); //Grant user access
    welcome();         //Display welcome message
  }
  //If the fingerprint match is found and the user wants to add/remove fingerprint user
  if (findFingerMatch() == true && accessOrChange == false) {
    lcd.clear();                      //clear LCD screen
    lcd.print("1 to add");            //enter 1 on keypad if user wants to add a new fingerpint user
    lcd.setCursor(0,1);               
    lcd.print("2 to remove");         //enter 2 on keypad if user wants to remove a new fingerpint user
    char key = myKeypad.waitForKey(); //wait until the user presses a button
    int addOrRemove = key - '0';      //conver the char value to integer
    if (addOrRemove == 1) {
      addFingerUser();                //function to add a fingerprint user
    }
    else if (addOrRemove == 2) {
      removeFingerUser();             //function to remove a fingerprint user
    }
  }
  char keyPressed = myKeypad.getKey();             //Get the key value
  perform (keyPressed);                            //Calling function to see what button is pressed
  if (Serial.available() > 0) {                    //Checking to see if anything is being sent to the Arduino from the RPi
    packetReceieved = Serial.readString();         //Save incoming message into string
    if (packetReceieved.substring(0,2) == "AG") {  //Check if message incoming is an Access Granted (AG) message (AG + data)
      AccessGranted(packetReceieved.substring(2)); //Call function to deal with access being granted (input paramter = data) //data = username
    }
    else if (packetReceieved == "AD") {       //Check if message incoming is an Access Denied (AD) message
      AccessDenied();                         //Call function to deal with access being granted
    }
    else if (packetReceieved == "UD") {       //Unlock Door
      lcd.clear();                            //Clear LCD screen
      lcd.print("Door Opening");              //Print String on LCD
      rotateCounterClockwise();               //Unlock Door (retract bolt)
      lcd.clear();                                
      lcd.print("Door Opened");               
      delay(1000);                            //Wait for 1 second
    }
    else if (packetReceieved == "LD") {      //Lock Door
      lcd.clear();                           
      lcd.print("Door Closing");
      rotateClockwise();                    //Lock Door (re-enforce bolt)
      lcd.clear();
      lcd.print("Door Closed");
      delay(1000);
    }
    else {
      Serial.println("Error&Wrong Packet Sent");  //Send error message back to RPi if wrong packet sent
    }
    welcome();                  //Display Welcome message again after incoming messages are dealt with
    packetReceieved = "";       //Clear previous message receieved
  }
}

//Display welcome message on LCD
void welcome(){
  lcd.clear();
  lcd.print("Welcome to YSB"); 
  lcd.setCursor(0,1); //Set cursor to print on second line of LCD
  lcd.print("B to buzz owner"); 
}

//function to deal with access being granted
void AccessGranted(String packetReceieved) {  
    lcd.clear();                              
    lcd.print("Access Granted");              
    lcd.setCursor(0,1);                       
    lcd.print("Door Unlocking");              
    rotateCounterClockwise();
    lcd.clear();
    lcd.print(packetReceieved);
    lcd.setCursor(0,1);
    lcd.print("Door Locking");
    rotateClockwise();
}

//Method to deal with if owner does not want guest to enter 
void AccessDenied() {
  lcd.clear();
  lcd.print("Access Denied");
  lcd.setCursor(0,1);
  lcd.print("Go Away");
  delay(2000);
}

//deal with the keypad inputs (either buzzer pressed, pin entered, or want to add/remove fingerprint user)
void perform (char keyPressed) {
  //If the buzzer was pressed
  if (keyPressed == 'B') {
    lcd.clear();
    lcd.print("Buzzer Pressed");
    Serial.println("DATA&BUZZ"); //Send the Data packet which instructs the RPi that buzzer was pressed
    delay(2000);
    welcome();
    pinCode = ""; //clear previous pin entered
  }
  //If a number on the keypad was pressed
  else if (keyPressed == '0' || keyPressed == '1' || keyPressed == '2' || keyPressed == '3' || keyPressed == '4' || keyPressed == '5' || keyPressed == '6' || keyPressed == '7' || keyPressed == '8' || keyPressed == '9') {
    pinCode += keyPressed; //append key pressed to the pin code
    lcd.clear();
    lcd.print(pinCode); //print each key pressed
    if (pinCode.length() == 4) { //Once 4 digits are entered 
      Serial.println("DATA&" + pinCode); //send the pin to the RPi
      pinCode = "";
    }
  }
  //If you want to add/remove a fingerprint user
  else if (keyPressed == 'A') {
    lcd.clear();
    lcd.print("Scan finger");
    accessOrChange = false; //false means user wants to add/remove fingerprint user
    pinCode = "";
  }
}

//Method to remove a fingerprint from the fingerprint scanner database
void removeFingerUser() {
  lcd.clear();
  lcd.print("ID to delete");
  char idKey = myKeypad.waitForKey();
  if (idKey == '0' || idKey == '1' || idKey == '2' || idKey == '3' || idKey == '4' || idKey == '5' || idKey == '6' || idKey == '7' || idKey == '8' || idKey == '9') {
    lcd.setCursor(0,1);
    lcd.print(idKey);
    int id = idKey - '0';
    uint8_t p = -1; //8 bit packet that talks to the fingerprint scanner database
    delay(2000);
    p = finger.deleteModel(id); //delete a fingerprint with that partcular ID in the fingerprint scanner database
    lcd.clear();
    lcd.print("User Deleted");
  }
  delay(2000);
  welcome();
}

//Method to add a new fingerprint 
void addFingerUser () {
  lcd.clear();
  lcd.print("Add new user");
  lcd.setCursor(0,1);
  lcd.print("Enter ID"); //Ask user to enter ID to save as the ID for that particular fingerprint on the fingerprint scanner database
  char idKey = myKeypad.waitForKey(); //Wait until a key is pressed on the Keypad
  //If the character is a number
  if (idKey == '0' || idKey == '1' || idKey == '2' || idKey == '3' || idKey == '4' || idKey == '5' || idKey == '6' || idKey == '7' || idKey == '8' || idKey == '9') {
    lcd.clear();
    lcd.print("New User ID");
    lcd.setCursor(0,1);
    lcd.print(idKey); //print the ID written
    int id = idKey - '0'; //convert character to an int
    delay(1000);
    while (accessOrChange == false) { //wait until adding a new user to the fingerprint scanner database is complete
      enrollFingerPrint(id); //call method to add new user
    }
  }
  delay(2000);
  welcome();
}

//add a new fingerprint user and store in the fingerprint scanner database
uint8_t enrollFingerPrint(int id) {
  lcd.clear();
  lcd.print("Scan Finger");
  int p = -1; //8 bit packet that talks to the fingerprint scanner database
  //loop until the finger scanned correclty and no errors occurred 
  while (p != FINGERPRINT_OK) {
    p = finger.getImage(); //Takes picture of finger on scanner 
    switch (p) {
    //Image taken and finger scanned correclty 
    case FINGERPRINT_OK:
      break;
    //Indicates that no finger is placed on scanner
    case FINGERPRINT_NOFINGER:
      break;
     //Indicates imagine was taken incorreclty
    case FINGERPRINT_IMAGEFAIL:
      lcd.clear();
      lcd.print("Incorrect Scan");
      lcd.setCursor(0,1);
      lcd.print("Rescan");
      break;
    //Unknown error
    default:
      break;
    }
  }
  lcd.clear();
  lcd.print("Image Taken");
  p = finger.image2Tz(1); //convert image to bytes
  lcd.clear();
  lcd.print("Remove finger");
  delay(2000);
  p = -1;
  lcd.clear();
  lcd.print("Scan again");
  //loop until the finger scanned correclty and no errors occurred 
  while (p != FINGERPRINT_OK) {
    p = finger.getImage(); //Takes picture of finger on scanner 
    switch (p) {
    //Image taken and finger scanned correclty 
    case FINGERPRINT_OK:
      break;
    //Indicates that no finger is placed on scanner
    case FINGERPRINT_NOFINGER:
      break;
     //Indicates imagine was taken incorreclty
    case FINGERPRINT_IMAGEFAIL:
      lcd.clear();
      lcd.print("Incorrect Scan");
      lcd.setCursor(0,1);
      lcd.print("Rescan");
      break;
    //Unknown error
    default:
      break;
    }
  }
  p = finger.image2Tz(2); //Convert image to bytes
  p = finger.createModel(); //creates a packet of the image bytes
  p = finger.storeModel(id); //stores the fingerprint in the fingerprint database
  lcd.clear();
  lcd.print("Done");
  accessOrChange = true;
}

//Scans finger and returns true if match found on fingerprint scanner database
bool findFingerMatch() {
  uint8_t p = finger.getImage();        //Takes picture of finger on scanner 
  if (p != FINGERPRINT_OK)  return false;

  p = finger.image2Tz();                //Convert image to bytes
  if (p != FINGERPRINT_OK)  return false;

  p = finger.fingerFastSearch();        //Find a match in the database
  if (p != FINGERPRINT_OK)  return false;

  return true;                          //true if match found
}

//function to rotate stepper motor 360 degrees counterclockwise
void rotateCounterClockwise () { 
    for(int x=0;x<revolutionSteps;x++) {
      algorithm(true); //call function to rotate motor
      delayMicroseconds(1000); //rotation speed
    }
}

//function to rotate stepper motor 360 degrees clockwise
void rotateClockwise () { 
    for(int x=0;x<revolutionSteps;x++) {
      algorithm(false);
      delayMicroseconds(1000);
    }
}

//algorithm to rotate stepper motor
void algorithm(bool dir) 
{
  //Overview: send voltages to certain pins to rotate motor
  switch(motorSteps)
  {
   case 0:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, HIGH);
   break; 
   case 1:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, HIGH);
     digitalWrite(IN4, HIGH);
   break; 
   case 2:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, HIGH);
     digitalWrite(IN4, LOW);
   break; 
   case 3:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, HIGH);
     digitalWrite(IN3, HIGH);
     digitalWrite(IN4, LOW);
   break; 
   case 4:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, HIGH);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
   case 5:
     digitalWrite(IN1, HIGH); 
     digitalWrite(IN2, HIGH);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
     case 6:
     digitalWrite(IN1, HIGH); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
   case 7:
     digitalWrite(IN1, HIGH); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, HIGH);
   break; 
   default:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
  }
  SetDirection(dir); //Call method to reset motorSteps if all cases finished and to increment according to rotation direction
}

//function to rotate stepper motor clockwise or counter clockwise and increment motorSteps
void SetDirection(bool dir){ 
  if(motorSteps>7){
    motorSteps=0;
   }
  if(motorSteps<0){
    motorSteps=7; 
   }
  if(dir==true){ 
    motorSteps++;
   }
  if(dir==false){ 
    motorSteps--; 
   }
}
