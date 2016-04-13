#ifndef CONFIG_H
#define CONFIG_H
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <NeoPixelBus.h>
#include <ArduinoJson.h>
#include <FS.h>

#define USE_SERIAL      Serial
#define BAUDRATE        115200

#define ADMIN_PW        "admin"
#define LOGOUT_TIME     300     //Seconds

#define LED_PIN         5
#define COLOR_ORDER     NEO_GRB
#define BRIGHTNESS      64
#define WIDTH           16
#define HEIGHT          16
#define CLOCK_COLOR     {BRIGHTNESS,BRIGHTNESS,BRIGHTNESS}
#define BLACK           {0,0,0}

#define AP_IP           {192,168,4,1}
#define AP_NAME         "LightFrame"
#define AP_PW           "adminpass"
#define WEBSERV_PORT    80
#define WLAN_SSID       "WLAN SSID"
#define WLAN_PW         "WLAN PWD"
#define UDP_PORT        2390
#define SUB_IP          {255,255,255,0}
#define NTP_PACKET_SIZE 48
#define TIME_ZONE       2
#define NTP_SERV        "ptbtime1.ptb.de"
#define WEATHER_URL     "http://api.openweathermap.org..."


enum CFG_ENUM{
  CFG_R_OK,
  CFG_W_OK,
  CFG_ERR_O_W,
  CFG_ERR_O_R,
  CFG_ERR_PARSE  
} ;

struct weatherS {
  int id;
  String icon;
  double temp;
  int pressure;
  int hum;
  float speed;
  int deg;
  int clouds;
  double rain;
  long rise;
  long set; 
} ;

const String cfg_t_dec[6] = {"CFG_R_OK","CFG_W_OK","CFG_ERR_O_W","CFG_ERR_O_R","CFG_ERR_SIZE","CFG_ERR_PARSE"};

typedef enum CFG_ENUM cfg_t;

class Config {
  public:
    Config() :
      clock_color(CLOCK_COLOR),
      cfg_dec(cfg_t_dec),
      led_pin(LED_PIN),
      color_order(COLOR_ORDER) ,
      brightness(BRIGHTNESS),
      width(WIDTH),
      height(HEIGHT),
      ntp_packet_size(NTP_PACKET_SIZE),
      ap_ip(IPAddress(AP_IP)),
      sub_ip(IPAddress(SUB_IP)),
      timezone(TIME_ZONE),
      udp_port(UDP_PORT),
      webserv_port(WEBSERV_PORT),
      baudrate(BAUDRATE),
      logout_time(LOGOUT_TIME), 
      ap_name((String)AP_NAME),
      ap_pw((String)AP_PW),
      wlan_ssid((String)WLAN_SSID),
      wlan_pw ((String)WLAN_PW),
      ntp_serv((String)NTP_SERV),
      admin_pw((String)ADMIN_PW){ }

    RgbColor  clock_color;
    
    String    cfg_dec[6];
    
    uint8_t   led_pin,
              color_order,
              brightness,
              width,
              height,
              ntp_packet_size;

    IPAddress ap_ip,
              sub_ip;
            
    int       timezone,
              udp_port,
              webserv_port,
              baudrate,
              logout_time;
            
    String    ap_name,
              ap_pw,
              wlan_ssid,
              wlan_pw,
              ntp_serv,
              admin_pw;
    
    cfg_t loadCfg(void);
    cfg_t saveCfg(void);
    void printCfg(void);
    void printCfg_f(void);
};

extern Config cfg;

#endif


