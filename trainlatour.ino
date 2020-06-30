// include library, include base class, make path known
#include <GxEPD.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <SPI.h>
#include "board_def.h"
#include "Button2.h"
#include "Esp.h"
#include <SPIFFS.h>

#include <WiFi.h>
#include <HTTPClient.h>
#define ARDUINOJSON_DECODE_UNICODE 1
#include <ArduinoJson.h>
#include <time.h> 

const char* ssid = "";
const char* password = "";
int heureencours;
 
char* trainencache1  = "";
char* trainencache2  = "";
char* trainencache3  = "";
char* trainencache4  = "";

 bool needrefresh= true;
  bool blindled= false;

struct Train
{
    const char* gare_dest;
    time_t dept_time;
    time_t base_dept_time;
    const char* physical_mode;
    const char* direction_type;
    const char* numtrain;
};



#include <GxGDE0213B72B/GxGDE0213B72B.h>      // 2.13" b/w

#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBoldOblique9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoOblique9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBoldOblique9pt7b.h>
#include <Fonts/FreeSansOblique9pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeSerifBoldItalic9pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>

//#define DEFALUT_FONT  FreeMono9pt7b
// #define DEFALUT_FONT  FreeMonoBoldOblique9pt7b
// #define DEFALUT_FONT FreeMonoBold9pt7b
// #define DEFALUT_FONT FreeMonoOblique9pt7b
// #define DEFALUT_FONT FreeSans9pt7b
 #define DEFALUT_FONT FreeSansBold9pt7b
// #define DEFALUT_FONT FreeSansBoldOblique9pt7b
// #define DEFALUT_FONT FreeSansOblique9pt7b
// #define DEFALUT_FONT FreeSerif9pt7b
// #define DEFALUT_FONT FreeSerifBold9pt7b
// #define DEFALUT_FONT FreeSerifBoldItalic9pt7b
// #define DEFALUT_FONT FreeSerifItalic9pt7b

const GFXfont *fonts[] = {
    &FreeMono9pt7b,
    &FreeMonoBoldOblique9pt7b,
    &FreeMonoBold9pt7b,
    &FreeMonoOblique9pt7b,
    &FreeSans9pt7b,
    &FreeSansBold9pt7b,
    &FreeSansBoldOblique9pt7b,
    &FreeSansOblique9pt7b,
    &FreeSerif9pt7b,
    &FreeSerifBold9pt7b,
    &FreeSerifBoldItalic9pt7b,
    &FreeSerifItalic9pt7b};

typedef enum
{
    RIGHT_ALIGNMENT = 0,
    LEFT_ALIGNMENT,
    CENTER_ALIGNMENT,
} Text_alignment;

GxIO_Class io(SPI, ELINK_SS, ELINK_DC, ELINK_RESET);
GxEPD_Class display(io, ELINK_RESET, ELINK_BUSY);

Train tabtrains[10]; 


void displayText(const String &str, int16_t y, uint8_t alignment)
{
    int16_t x = 0;
    int16_t x1, y1;
    uint16_t w, h;
    display.setCursor(x, y);
    display.getTextBounds(str, x, y, &x1, &y1, &w, &h);

    switch (alignment)
    {
    case RIGHT_ALIGNMENT:
        display.setCursor(display.width() - w - x1, y);
        break;
    case LEFT_ALIGNMENT:
        display.setCursor(0, y);
        break;
    case CENTER_ALIGNMENT:
        display.setCursor(display.width() / 2 - ((w + x1) / 2), y);
        break;
    default:
        break;
    }
    display.println(str);
}

void displayInit(void)
{
    static bool isInit = false;
    if (isInit)
    {
        return;
    }
    isInit = true;
    display.init();
    display.setRotation(0);
    display.eraseDisplay();
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&DEFALUT_FONT);
    display.setTextSize(2);
}

void setup() {
  // put your setup code here, to run once:
    displayInit();
    display.fillScreen(GxEPD_WHITE);
    Serial.begin(115200);
     WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
      }
      pinMode(32, OUTPUT);
      pinMode(22, OUTPUT);
      digitalWrite(32, HIGH); 
      digitalWrite(22, LOW); 
      delay(500); 
      digitalWrite(32, LOW); 
      digitalWrite(22, HIGH); 
      delay(500); 
      digitalWrite(22, LOW); 
      Serial.println("Setup termine");
}

void loop() {
  // put your main code here, to run repeatedly:
 // button_loop();
 //display.eraseDisplay();
 if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to the WiFi network");
    HTTPClient http;  //Object of class HTTPClient
    http.begin("https://APIKEY@api.sncf.com/v1/coverage/sncf/stop_areas/stop_area%3AOCE%3ASA%3A87721548/departures?count=4&direction_type=forward");//trains gare
    //http.begin("https://APIKEY@api.sncf.com/v1/coverage/sncf/stop_areas/stop_area%3AOCE%3ASA%3A87698316/departures?count=6");//car hippodrome
   
    int httpCode = http.GET();
    //Check the returning code
    Serial.print("HTTP COde : ");Serial.println(httpCode);                                                               
    if (httpCode > 0) {
   
      // Get the request response payload
      Stream& response = http.getStream();
    
      const size_t capacity = 100000;
      DynamicJsonDocument doc(capacity);
      
      DeserializationError error = deserializeJson(doc, response);
      if (error) {
        Serial.print(F("deserializeJson() failed: "));    
        Serial.println(error.c_str()); 
        display.fillScreen(GxEPD_WHITE);
        displayText(String("DESERIALISATION erreur"), 10, LEFT_ALIGNMENT); 
        needrefresh = true; 
        blindled=true;
        return;  
      }

      //combiens de trains ?
      JsonObject pagination = doc["pagination"];
      int pagination_total_result = pagination["total_result"];
      Serial.print("Nb trains : ");Serial.println(pagination_total_result);  
      if (pagination_total_result == 0) {
          displayText(String("Pas de trains"), 30, LEFT_ALIGNMENT);
          needrefresh = true; 
      }
      else{
        
        
        JsonArray departures = doc["departures"];
        JsonObject context = doc["context"];
         const char* current_datetime = context["current_datetime"]; 
         struct tm time= {0};
         int yr, mnth, d, m, s;
         //struct tm currentdatetime= {0};
         sscanf( current_datetime, "%4d%2d%2dT%2d%2d%2d", &yr, &mnth, &d, &heureencours, &m, &s); //spfrtime bugge sur arduino   
         //currentdatetime.tm_hour  = h; 
           Serial.print("Heure en cours");Serial.println(heureencours);
         
  
        
        for (int i = 0; i < pagination_total_result; i++) {       
            JsonObject departure = departures[i];
            JsonObject departures_display_informations = departure["display_informations"];
            const char* departures_display_informations_direction = departures_display_informations["direction"]; 
            const char* departures_display_informations_name = departures_display_informations["name"]; 
            const char* physicalmode = departures_display_informations["physical_mode"]; 
            const char* numerotrain = departures_display_informations["headsign"]; 
            
            JsonObject departures_stop_date_time = departure["stop_date_time"];
            const char* departure_date_time = departures_stop_date_time["departure_date_time"]; 
            const char* base_departure_date_time = departures_stop_date_time["base_departure_date_time"];

            JsonObject departures_route = departure["route"];
            const char* departures_route_direction_type = departures_route["direction_type"]; 

            Train train;
            train.gare_dest =  departures_display_informations_direction;
            train.physical_mode =  physicalmode;
            train.numtrain = numerotrain;

            if ((i == 0 &&  (strcmp(numerotrain,trainencache1) != 0)) || (strcmp(trainencache1,"") == 0) ){
              Serial.print("Refresh e-ink ! From json :");Serial.print(numerotrain);Serial.print(" en cache :");Serial.println(trainencache1);//la plupart du temps cet apppel suffit
              trainencache1 = strdup(numerotrain);
              needrefresh = true;
            }
            
            if ((i == 1 &&  (strcmp(numerotrain,trainencache2) != 0)) || (strcmp(trainencache2,"") == 0) ){trainencache2 = strdup(numerotrain);needrefresh = true;}
            if ((i == 2 &&  (strcmp(numerotrain,trainencache3) != 0)) || (strcmp(trainencache3,"") == 0) ){trainencache3 = strdup(numerotrain);needrefresh = true;}
            if ((i == 3 &&  (strcmp(numerotrain,trainencache4) != 0)) || (strcmp(trainencache4,"") == 0) ){trainencache4 = strdup(numerotrain);needrefresh = true;}

            train.direction_type = departures_route_direction_type;

            struct tm time= {0};
            struct tm basetime= {0};
  
            int yr, mnth, d, h, m, s;
            sscanf( departure_date_time, "%4d%2d%2dT%2d%2d%2d", &yr, &mnth, &d, &h, &m, &s); //spfrtime bugge sur arduino   
            time.tm_year  = yr - 1970; time.tm_mon  = mnth; time.tm_mday  = d; time.tm_hour  = h;  time.tm_min   = m;  time.tm_sec  = s;

            sscanf( base_departure_date_time, "%4d%2d%2dT%2d%2d%2d", &yr, &mnth, &d, &h, &m, &s); //spfrtime bugge sur arduino   
            basetime.tm_year  = yr - 1970; basetime.tm_mon  = mnth; basetime.tm_mday  = d; basetime.tm_hour  = h;  basetime.tm_min   = m;  basetime.tm_sec  = s;
            
            
            //Serial.println(&time);
            time_t loctime = mktime(&time);  // timestamp in current timezone
           train.dept_time = loctime;
           
            time_t baseloctime = mktime(&basetime);  // timestamp in current timezone
           train.base_dept_time = baseloctime;
           
           tabtrains[i]=train;
        }

        //affichage
        int offsetaffichage = 30;
        display.fillScreen(GxEPD_WHITE);
        for (int i = 0; i < pagination_total_result; i++) {
          char buffer[80];
          char basebuffer[80];
          strftime (buffer, 80, "%H:%M", localtime(&(tabtrains[i].dept_time)));
          strftime (basebuffer, 80, "%H:%M", localtime(&(tabtrains[i].base_dept_time)));
          
          Serial.print(buffer);Serial.print(" : ");Serial.print(tabtrains[i].gare_dest);Serial.print(tabtrains[i].direction_type);Serial.print(" (");Serial.print(tabtrains[i].physical_mode);Serial.println(")");
  
           //if  (strcmp(tabtrains[i].direction_type,"forward") == 0){//resultats fournis par l'API alĂ©atoire sur direction_type
           //if  (strncmp(tabtrains[i].gare_dest,"Lyon",4) == 0){
                display.setTextSize(2);
                displayText(basebuffer, offsetaffichage, CENTER_ALIGNMENT);
                offsetaffichage+=20;
                display.setTextSize(0);
  
                if(tabtrains[i].dept_time  != tabtrains[i].base_dept_time){
                    displayText(String("RETARD"), offsetaffichage, CENTER_ALIGNMENT);
                    blindled = true;
                }
                else{
                    displayText(tabtrains[i].numtrain, offsetaffichage, CENTER_ALIGNMENT);
                }
                offsetaffichage+=8;
                display.drawFastHLine( 0, offsetaffichage, 128, GxEPD_BLACK);
                offsetaffichage+=35;
             // }
        }
        
      

      }
       
    }

    http.end();   //Close connection
  }

  
  // Delay between APi call
  if(needrefresh){Serial.println("Refresh");display.update();needrefresh = false;}
  if (blindled){//retard
      for (int i = 0; i < 30; i++) {  
          digitalWrite(32, HIGH); 
          digitalWrite(22, LOW); 
          delay(1000); 
          digitalWrite(32, LOW); 
          digitalWrite(22, HIGH); 
          delay(1000); 
      }
      blindled=false;
      digitalWrite(22, LOW); 
      delay(120000);
  }
  else{
    switch( heureencours )
    {
        case 6:
            Serial.println("6 heures");
            delay(120000);
            break;
        case 7:
            Serial.println("7 HEURES");
            delay(120000);
            break;
        case 8:
            Serial.println("8 HEURES");
            delay(120000);
            break;
        case 22:
            Serial.println("22 HEURES");
            delay(28800000);
            break;
        default :
            Serial.print("Autres HEURES "); Serial.println(heureencours);
            delay(600000);
            break;
            
    }
    
  }
}
