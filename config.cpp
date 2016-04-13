#include "./config.h"

cfg_t Config::saveCfg(void) {
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  JsonArray& cc = json.createNestedArray("cc");
  cc.add(clock_color.R);
  cc.add(clock_color.G);
  cc.add(clock_color.B);
  
  json["lp"]    = led_pin;
  json["co"]    = color_order;
  json["b"]     = brightness;
  json["w"]     = width;
  json["h"]     = height;
  json["ntpps"] = ntp_packet_size;

  json["ai"]    = (uint32_t)ap_ip;
  json["si"]    = (uint32_t)sub_ip;
            
  json["tz"]    = timezone;
  json["up"]    = udp_port;
  json["wsp"]   = webserv_port;
  json["bd"]    = baudrate;
  json["lt"]    = logout_time;
            
  json["an"]    = ap_name.c_str();
  json["ap"]    = ap_pw.c_str();
  json["wn"]    = wlan_ssid.c_str();
  json["wp"]    = wlan_pw.c_str();
  json["admin"] = admin_pw.c_str();
  
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile)   
    return CFG_ERR_O_W;


  json.printTo(configFile);
  configFile.close();
  return CFG_W_OK;
}


cfg_t Config::loadCfg(void) {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile)   
    return CFG_ERR_O_R;
 
  size_t size = configFile.size();
  std::unique_ptr<char[]> buf(new char[size]);
  
  configFile.readBytes(buf.get(), size);
  configFile.close();
  
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success())  
    return CFG_ERR_PARSE;
 
  clock_color       = {json["cc"][0],json["cc"][1],json["cc"][2]};
  //uint8_t
  led_pin           = json["lp"];
  color_order       = json["co"];
  brightness        = json["b"];
  width             = json["w"];
  height            = json["h"];
  ntp_packet_size   = json["ntpps"];
  //IPAddress
  ap_ip = IPAddress(json["ai"]);
  sub_ip = IPAddress(json["si"]);
  //int     
  timezone          = json["tz"];
  udp_port          = json["up"];
  webserv_port      = json["wsp"];
  baudrate          = json ["bd"];
  logout_time       = json["lt"];
  //String
  ap_name           = json["an"].asString();
  ap_pw             = json["ap"].asString();
  wlan_ssid         = json["wn"].asString();
  wlan_pw           = json["wp"].asString();
  ntp_serv          = json["ntps"].asString();
  admin_pw          = json["admin"].asString();
  
  return CFG_R_OK;
}

void Config::printCfg_f(void){
  File f = SPIFFS.open("/config.json","r");
  if(f){
    USE_SERIAL.println("");
    USE_SERIAL.println("---START CFG FILE---\n\r");
    char c = 0;
    while((c <f.size() )){
      char chr = f.read();
      USE_SERIAL.print(chr);
      if(chr==',')
        USE_SERIAL.println("");
      c++;
    }
    USE_SERIAL.println("");
    USE_SERIAL.println("---START CFG FILE---\n\r");
  }
}


void Config::printCfg(void){
  USE_SERIAL.println("");
  USE_SERIAL.println("---START CFG MEMORY---\n\r");
  USE_SERIAL.print("LED_PIN: ");
  USE_SERIAL.println(led_pin);
  USE_SERIAL.print("COLOR_ORDER: ");
  USE_SERIAL.println(color_order);
  USE_SERIAL.print("BRIGHTNESS: ");
  USE_SERIAL.println(brightness);
  USE_SERIAL.print("WIDTH: ");
  USE_SERIAL.println(width);
  USE_SERIAL.print("HEIGHT: ");
  USE_SERIAL.println(height);
  USE_SERIAL.print("NTP_PACKAGE_SIZE: ");
  USE_SERIAL.println(ntp_packet_size);
  USE_SERIAL.print("TIMEZONE: ");
  USE_SERIAL.println(timezone);
  USE_SERIAL.print("UDP_PORT: ");
  USE_SERIAL.println(udp_port);
  USE_SERIAL.print("WEBSERV_PORT: ");
  USE_SERIAL.println(webserv_port);
  USE_SERIAL.print("AP_NAME: ");
  USE_SERIAL.println(ap_name);
  USE_SERIAL.print("AP_PW: ");
  USE_SERIAL.println(ap_pw);
  USE_SERIAL.print("WLAN_SSID: ");
  USE_SERIAL.println(wlan_ssid);
  USE_SERIAL.print("WLAN_PW: ");
  USE_SERIAL.println(wlan_pw );
  USE_SERIAL.print("NTP_SERV: ");
  USE_SERIAL.println(ntp_serv);
  USE_SERIAL.print("ADMIN_PW: ");
  USE_SERIAL.println(admin_pw);
  USE_SERIAL.print("BAUDRATE: ");
  USE_SERIAL.println(baudrate);
  USE_SERIAL.print("AP_IP: ");
  int i=0;
  for(i=0;i<3;i++){
    USE_SERIAL.print(ap_ip>>((24-i*8)&0x0f));
    USE_SERIAL.print(".");
  }
  USE_SERIAL.println(ap_ip&0x0f);
  USE_SERIAL.print("SUB_IP: ");
  for(i=0;i<3;i++){
    USE_SERIAL.print(sub_ip>>((24-i*8)&0x0f));
    USE_SERIAL.print(".");
  }
  USE_SERIAL.println(sub_ip&0x0f);
  USE_SERIAL.println("");
  USE_SERIAL.println("---END CFG MEMORY---\n\r");
}



