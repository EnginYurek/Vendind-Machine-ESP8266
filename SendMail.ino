/* @file MultiKey.ino
|| @version 1.0
|| @author Mark Stanley
|| @contact mstanley@technologist.com
||
|| @description
|| | The latest version, 3.0, of the keypad library supports up to 10
|| | active keys all being pressed at the same time. This sketch is an
|| | example of how you can get multiple key presses from a keypad or
|| | keyboard.
|| #
*/

#include <Keypad.h>
#include <Adafruit_ESP8266.h>
#include <SoftwareSerial.h>

#define ESP_RX   9//3
#define ESP_TX   12//4
#define ESP_RST  13
SoftwareSerial softser(ESP_RX, ESP_TX);

Adafruit_ESP8266 wifi(&softser, &Serial, ESP_RST);

#define ESP_SSID "ITU-NET Misafir" // Your network name here
#define ESP_PASS "" // Your network password here


// smtp2go account name and passwprd base64 encoded at https://www.base64encode.org/
#define EMAIL_FROM_BASE64 "d2lyZWxlc3Njb20="  //wirelesscom
#define EMAIL_PASSWORD_BASE64 "ZW5naW50dWdheQ==" //engintugay

#define HOST     "mail.smtp2go.com"     //smtp2go server

#define PORT     2525                // smtp2go port


char EMAIL_FROM[] = "your email";
char EMAIL_PASSWORD[] =  "your password";
char EMAIL_TO[] = "destination email";
char SUBJECT[]  = "Remaining Stock";
char EMAIL_CONTENT[100] ;


const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
{'#','0','*'},
{'9','8','7'},
{'6','5','4'},
{'3','2','1'}
};
byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the kpd
byte colPins[COLS] = {8, 7, 6}; //connect to the column pinouts of the kpd

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

unsigned long loopCount;
unsigned long startTime;
String msg;
int state = 1;
int product_number;
int product_counter[5] = {15, 15, 15, 15, 15};

void setup() {
  char buffer[50];
   wifi.setBootMarker(F("Version:0.9.2.4]\r\n\r\nready"));
softser.begin(9600); // Soft serial connection to ESP8266
   
  Serial.begin(57600); while(!Serial); // UART serial debug
  Serial.println(F("Adafruit ESP8266 Email"));

Serial.print(F("Hard reset..."));
  if(!wifi.hardReset()) {
    Serial.println(F("no response from module."));
    for(;;);
  }
  Serial.println(F("OK."));

  Serial.print(F("Soft reset..."));
  if(!wifi.softReset()) {
    Serial.println(F("no response from module."));
    for(;;);
  }
  Serial.println(F("OK."));

  Serial.print(F("Checking firmware version..."));
  wifi.println(F("AT+GMR"));
  if(wifi.readLine(buffer, sizeof(buffer))) {
    Serial.println(buffer);
    wifi.find(); // Discard the 'OK' that follows
  } else {
    Serial.println(F("error"));
  }

  Serial.print(F("Connecting to WiFi..."));
  if(wifi.connectToAP(F(ESP_SSID), F(ESP_PASS))) {

    // IP addr check isn't part of library yet, but
    // we can manually request and place in a string.
    Serial.print(F("OK\nChecking IP addr..."));
    wifi.println(F("AT+CIFSR"));
    if(wifi.readLine(buffer, sizeof(buffer))) {
        Serial.println(buffer);
        wifi.find(); // Discard the 'OK' that follows

        Serial.print(F("Connecting to host..."));

        Serial.print("Connected..");
        wifi.println("AT+CIPMUX=0"); // configure for single connection, 
                                     //we should only be connected to one SMTP server
        wifi.find();
        wifi.closeTCP(); // close any open TCP connections
        wifi.find();
        wifi.closeTCP();
        Serial.println("Choose your product: \nWater = 10 \nbiscuit = 11 \ngum = 12 \ncoffer = 13 \ntea = 14\n");
        
    } else { // IP addr check failed
      Serial.println(F("error"));
    }
  } else { // WiFi connection failed
    Serial.println(F("FAIL"));
  }
    loopCount = 0;
    startTime = millis();
    msg = "";

   
}


void loop() {

    if (kpd.getKeys())
    {
        for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.
        { 
            if ( kpd.key[i].stateChanged )   // Only find keys that have changed state.
            {
             if (kpd.key[i].kstate == PRESSED)
             {
              if (kpd.key[i].kchar != '#'){
                 Serial.println(state);
                switch (state) {  //  key state : IDLE, PRESSED, HOLD, or RELEASED
                    case 1:
                    product_number = kpd.key[i].kcode*10;
                    state = 2;
                    Serial.println("state 1");
                break;
                    case 2:
                    product_number = product_number + kpd.key[i].kcode;
                    Serial.println( "state 2");
                    state = 3;
                    product_counter[product_number % 10] = product_counter[product_number % 10] - 1;

                    for (int j = 0; j<4; j++)
                    {
                      if(product_counter[j] < 12)
                      { 
                        String send_product_data= String(product_number);
                     
                        send_product_data.toCharArray(EMAIL_CONTENT,  send_product_data.length()); //Copy the string (+1 is to hold the terminating null char)
                        


                        send_mail();
                      }
                    }
                    product_number = 0;
                    state = 1;
                    break;                    
                }
              }else
              {
                product_number = 0;
                state = 1;
                
              }//sıfırla
            }
            }
        }
    }
}  // End loop


boolean send_mail( )
{

        Serial.println("Connecting...");
        wifi.connectTCP(F(HOST), PORT);

        // send "EHLO ip_address" command. Server will reply with "250" and welcome message
        wifi.cipSend("EHLO 161.9.115.194",F("250"));                                  
      
    
        // send "AUTH LOGIN" command to the server will reply with "334 username" base 64 encoded
        wifi.cipSend("AUTH LOGIN",F("334 VXNlcm5hbWU6"));
       
   
        // send username/email base 64 encoded, the server will reply with "334 password" base 64 encoded
        wifi.cipSend(EMAIL_FROM_BASE64,F("334 UGFzc3dvcmQ6")); 
      
   
        // send password base 64 encoded, upon successful login the server will reply with 235.
         wifi.cipSend(EMAIL_PASSWORD_BASE64,F("235"));
       
    
        // send "MAIL FROM:<emali_from@domain.com>" command
        char mailFrom[50] = "MAIL FROM:<"; // If 50 is not long enough change it, do the same for the array in the other cases
        strcat(mailFrom,EMAIL_FROM);
        strcat(mailFrom,">");

        wifi.cipSend(mailFrom,F("250"));
       
  
        // send "RCPT TO:<email_to@domain.com>" command
        char rcptTo[50] = "RCPT TO:<";
        strcat(rcptTo,EMAIL_TO);
        strcat(rcptTo,">");
         wifi.cipSend(rcptTo,F("250"));  
        
    
    
        // Send "DATA"  command, the server will reply with something like "334 end message with \r\n.\r\n."
        wifi.cipSend("DATA",F("354"));
        
    
        // apply "FROM: from_name <from_email@domain.com>" header
        char from[100] = "FROM: ";
        strcat(from,EMAIL_FROM);
        strcat(from," ");
        strcat(from,"<");
        strcat(from,EMAIL_FROM);
        strcat(from,">");
         wifi.cipSend(from);  
        
    
    
        // apply TO header 
        char to[100] = "TO: ";
        strcat(to,EMAIL_TO);
        strcat(to,"<");
        strcat(to,EMAIL_TO);
        strcat(to,">");
         wifi.cipSend(to);  
        
   
        // apply SUBJECT header
        char subject[50] = "SUBJECT: ";
        strcat(subject,SUBJECT);
         wifi.cipSend(subject);
      
  
    
         wifi.cipSend("\r\n");   // marks end of header (SUBJECT, FROM, TO, etc);
        
    
         wifi.cipSend(EMAIL_CONTENT);
        
    
        wifi.cipSend("\r\n.");  // marks end of data command
        
    
        wifi.cipSend("QUIT"); 
    
        wifi.closeTCP();
        Serial.println("Done");
}
