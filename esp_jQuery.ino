#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <TimeLib.h> 
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <NeoPixelBus.h>
#include "./config.h"
#include "./pictures.h"
#include "./PImage.h"

Config cfg;

MDNSResponder mdns;
ESP8266WebServer server;
WiFiUDP Udp;
NeoPixelBus* strip;
bool showClock = true;
bool admin = false;
unsigned long adminTimer = 0;
struct weatherS weather;


void setup(void){
  uint8_t myblink=0;
  char cr[25];
  
//FILESYSTEM
  SPIFFS.begin();
  
//CONFIG
  /*cfg_t loadConf = CFG_R_OK;// cfg.loadCfg();
  cfg_t saveConf = cfg.saveCfg(); */
  
  cfg_t loadConf = cfg.loadCfg();
  cfg_t saveConf = CFG_R_OK;
  if(loadConf != CFG_R_OK)
    saveConf = cfg.saveCfg();
    
//SERIAL
  USE_SERIAL.begin(cfg.baudrate);
  USE_SERIAL.println("");
  USE_SERIAL.println(cfg.cfg_dec[loadConf]);
  USE_SERIAL.println(cfg.cfg_dec[saveConf]);
  
//LED STRIP
  strip = new NeoPixelBus((cfg.width*cfg.height),cfg.led_pin,cfg.color_order);
  strip->Begin();
  
//WIFI MODE
  //WiFi.mode(WIFI_STA);
  WiFi.softAP(cfg.ap_name.c_str(),cfg.ap_pw.c_str());
  WiFi.softAPConfig(cfg.ap_ip, cfg.ap_ip, cfg.sub_ip);
  
//WIFI STA
  WiFi.begin(cfg.wlan_ssid.c_str(), cfg.wlan_pw.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    USE_SERIAL.print(".");
    sprintf(cr,"/sys_img/wlan_%i.bmp",myblink++);
    drawImage(cr);
    if(myblink>4)
      myblink = 0;
    delay(300);
  }
  USE_SERIAL.println("");
  USE_SERIAL.print("IP address: ");
  USE_SERIAL.println(WiFi.localIP());

//MDNS
  if (!mdns.begin(cfg.ap_name.c_str(), WiFi.localIP()))
    USE_SERIAL.println("MDNS failed");
  else
    USE_SERIAL.println("MDNS ok");
  
//WEBSERVER
  server = ESP8266WebServer(cfg.webserv_port);  
  server.on("/", handleRoot);
  server.on("/ajax", handleAjax);
  server.onNotFound(handleNotFound);
  server.begin();
  USE_SERIAL.println("HTTP server started");

//LIST FILES
Dir dir = SPIFFS.openDir("/");
  while (dir.next())
    USE_SERIAL.println(dir.fileName());

//NTP
  Udp.begin(cfg.udp_port);
  setSyncInterval(15*60);
  setSyncProvider(getNtpTime);
  printTi();

//WEATHER  //does not work (low RAM?)
 //getWeather();
 //printWe();
}


void loop(void){  
  if(millis() > (adminTimer + cfg.logout_time * 1000))
    admin = false;
  mdns.update();
  server.handleClient();
  if(showClock)
    showDigiClock();
   
}


void handleAjax(void){
  if(server.method()==HTTP_POST){
    showClock = false;
    if(server.hasArg("subCol") ){
      RgbColor col = {(uint8_t)server.arg("r").toInt(),(uint8_t)server.arg("g").toInt(),(uint8_t)server.arg("b").toInt()};
      fill(col);
      strip->Show();
      
    }
    else if(server.hasArg("subTime")){
      showClock = true; 
      if(server.hasArg("r") && server.hasArg("g") && server.hasArg("b")){
        RgbColor col = {(uint8_t)server.arg("r").toInt(),(uint8_t)server.arg("g").toInt(),(uint8_t)server.arg("b").toInt()};
        cfg.clock_color=col;
        if( (cfg.clock_color.R != col.R || cfg.clock_color.G != col.G || cfg.clock_color.B !=col.B) && server.hasArg("saveCol") ){
          cfg_t res = cfg.saveCfg();
          USE_SERIAL.println(cfg.cfg_dec[res]);
        }
      }
      showDigiClock();
    }
    server.send(200, "text/plain", "OK");
  }
  else
    server.send(404, "text/plain", "NOPE");
}


void handleRoot(void){
  getFile("/index.html");
}


void handleNotFound(void){
  //if((server.uri().indexOf("jquery") >=0))
    //server.sendHeader("Cache-Control", "no-transform,public,max-age=1209600");
  getFile(server.uri()); 
}


void getFile(String file){
  File f = SPIFFS.open(file,"r");
  if(f){
    server.streamFile(f,getContentType(file));
    f.close();
  }
  else{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i=0; i<server.args(); i++)
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    server.send(404, "text/plain", message);
  }
}


time_t getNtpTime(void){
  while (Udp.parsePacket() > 0) ;
  IPAddress timeServerIP; 
  byte packetBuffer[48]; 
  WiFi.hostByName("ptbtime1.ptb.de", timeServerIP); 
  memset(packetBuffer, 0, 48);
  packetBuffer[0] = 0b11100011;   
  packetBuffer[1] = 0;     
  packetBuffer[2] = 6;    
  packetBuffer[3] = 0xEC;  
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;           
  Udp.beginPacket(timeServerIP, 123); 
  Udp.write(packetBuffer, 48);
  Udp.endPacket();
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    if (Udp.parsePacket() >= 48) {
      Udp.read(packetBuffer, 48);  
      unsigned long secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + cfg.timezone * SECS_PER_HOUR ;
    }
  }
  return 0;
}


String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".bmp")) return "image/bmp";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}


bool drawImage(String file){
  PImage img(file);
  if(img.isValid()){
    img.draw(strip);
    strip->Show();
    img.close();
    return true;
  } 
  return false;
}


void fill(RgbColor color) {
  for (int i=0; i<(256); i++) 
    strip->SetPixelColor(i, color);
}


void putStr(int x, int y, String str, RgbColor color){
  for(int i = 0; i < (int)str.length(); i++)
    for(int j=0;j<5;j++)
      for(int k = 7; k > 0; k--)
        if(  pgm_read_byte_near(&(font_5x7[str[i]-32][j])) & (1<<k) )
          strip->SetPixelColor(XY(x+7-k,y+(i*5)+j+i), color);
}


void showDigiClock(void){
  static int prevDisplay = 0; 
  if (timeStatus() != timeNotSet ) {
    if ((second() != prevDisplay)) { 
      prevDisplay = second();
      char buff[10];
      fill(BLACK);
      if(hour()<10)
        sprintf(buff,"0%i",hour());
      else
        sprintf(buff,"%i",hour());
      putStr(0,2,buff,cfg.clock_color);
      if(minute()<10)
        sprintf(buff,"0%i",minute());
      else
        sprintf(buff,"%i",minute());
      putStr(9,2,buff,cfg.clock_color);
      for(int i = 5;i>=0;i--){
        if(second() & (1 << i)){
          RgbColor col;
          switch(i){
            case 0: col = {240,0,0};break;
            case 1: col = {255,210,0};break;
            case 2: col = {0,240,0};break;
            case 3: col = {0,225,240};break;
            case 4: col = {0,0,240};break;
            case 5: col = {240,0,240};break;
            default: col = {128,128,128};break;
          }
           strip->SetPixelColor(XY(10+5-i,15), col);
        } 
      }  
      strip->Show();
    } 
  }
  else{
    fill(BLACK);
    strip->Show();
  }
}


void printTi(void){
  if (timeStatus() != timeNotSet ) {
    char buff[20];
    sprintf(buff,"%u%u/%u%u/%u%u, %u%u:%u%u:%u%u",
        (day()/10),(day()%10),(month()/10),(month()%10),(year()%100/10),(year()%10),(hour()/10),(hour()%10),(minute()/10),(minute()%10),(second()/10),(second()%10));
    USE_SERIAL.println(buff);
  }
}
void printWe(void){
  if (timeStatus() != timeNotSet ) {
    char buff[400];
    sprintf(buff,"WID: %u\n\rWIcon: %s\n\rTemp: %f °C\n\rPress: %u mBar\n\rHum: %u %%\n\rSpeed: %f m/s\n\rDeg: %u °\n\rCloud: %u %%\n\rRain: %u l/3h\n\rRise: %u:%u h\n\rSet: %u:%u h\n\r",
        weather.id,weather.icon.c_str(),weather.temp,weather.pressure,weather.hum,weather.speed,weather.deg,weather.clouds,weather.rain,hour(weather.rise + cfg.timezone * SECS_PER_HOUR),minute(weather.rise),hour(weather.set + cfg.timezone * SECS_PER_HOUR),minute(weather.set ));
    USE_SERIAL.println(buff);
  }
}


void getWeather(void){
  
  HTTPClient http;
  USE_SERIAL.println("a");
  http.begin(WEATHER_URL);
  USE_SERIAL.println("b");
  int httpCode = http.GET();
  USE_SERIAL.println("c");
  if(httpCode == 200) {
    USE_SERIAL.println("d");
    DynamicJsonBuffer jsonBuffer;
    USE_SERIAL.println("e");
    JsonObject& json = jsonBuffer.parseObject(http.getString());
    USE_SERIAL.println("f");
    if (!json.success()) 
      USE_SERIAL.println("json load fail");
    else{
      USE_SERIAL.println("g");
    /*  weather.id = json["weather"][0]["id"];
      weather.icon = json["weather"][0]["icon"].asString();
      weather.temp = ((double)json["main"]["temp_min"]+(double)json["main"]["temp_max"])/2;
      weather.pressure =json["main"]["pressure"];
      weather.hum = json["main"]["humidity"];
      weather.speed = json["wind"]["speed"];
      weather.deg = json["wind"]["deg"];
      weather.clouds = json["clouds"]["all"];
      weather.rain = json["rain"]["3h"];
      weather.rise = json["sys"]["sunrise"];
      weather.set = json["sys"]["sunset"]; */
      //printWe();
    } 
  }
  else{
    Serial.println("HTTP REQ FAIL");
    Serial.println(httpCode);
  }
  ESP.wdtEnable(WDTO_500MS);

  http.end();
  
}
