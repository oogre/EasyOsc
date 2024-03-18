
#ifndef easyOsc_h
#define easyOsc_h

#include <Arduino.h>

#include "Dictionary/Dictionary.h"
#include <OSCMessage.h>
#include <OSCBoards.h>
#include <functional>
#include <utility>

#ifdef BOARD_HAS_USB_SERIAL
#include <SLIPEncodedUSBSerial.h>
SLIPEncodedUSBSerial SLIPSerial( thisBoardsSerialUSB );
#else
#include <SLIPEncodedSerial.h>
SLIPEncodedSerial SLIPSerial(Serial); // Change to Serial1 or Serial2 etc. for boards with multiple serial ports that don’t have Serial
#endif

#ifdef ESP32
#include <WiFi.h>
#include <WiFiUdp.h>
#endif

class EasyOsc {
  public :
    struct MessageHandler {
      static bool handlerValidator(OSCMessage & messageIN, String validator) {
        if(validator == "*") return true;
        if (messageIN.size() != validator.length()) return false;
        String knownType = "ifsb";
        uint8_t position = 0;
        bool isValid = true;
        auto c = validator.begin();
        while (c != validator.end()) {
          switch (*c) {
            case 'i' : isValid &= messageIN.isInt(position); break;
            case 'f' : isValid &= messageIN.isFloat(position); break;
            case 's' : isValid &= messageIN.isString(position); break;
            case 'b' : isValid &= messageIN.isBlob(position); break;
            case '*' : isValid &= true; break;
            default : return false;
          }
          c++;
          position++;
        }
        return isValid;
      }
      using MessageHandlerFn = std::function<void(OSCMessage &msg)>;
      MessageHandler() {};
      MessageHandler(MessageHandlerFn dec) : run {std::move(dec)} {};
      MessageHandlerFn run;
      String validator = "*";
      void setValidator(String value){ validator = value; }
      void operator()(OSCMessage &msg) { 
        if(MessageHandler::handlerValidator(msg, validator)){
            this->run(msg);
        }else{
          char buffer[64];
          int result = msg.getAddress(buffer);
          String signature = "";
          for(uint8_t i = 0 ; i < msg.size() ; i ++){
            signature += String(msg.getType(i));
          }
          Serial.println("Wrong singnature for " + String(buffer) + " ! \"" + validator + "\" needed. \"" + signature +"\" recieved.");
        }
      };
    };

    String CONNECTION_TYPE_DICT [3] = { "ACCESS_POINT", "REGULAR_WIFI", "USB_SERIAL" };
    enum CONNECTION_TYPE { ACCESS_POINT, REGULAR_WIFI, USB_SERIAL };

    struct MessageOSC {
      OSCMessage message;
    public :
      MessageOSC(){}
      MessageOSC(String address){ setAddress(address); }
      ~MessageOSC(){
        message.empty();
      }
      MessageOSC & setAddress(String value){
        message.empty();
        char* buffer = new char[value.length() + 1];
        value.toCharArray(buffer, value.length() + 1);
        message.setAddress(buffer);
        delete buffer;
        return *this;
      }
      MessageOSC & add(int32_t value){
        message.add(value);
        return *this;
      }
      MessageOSC & add(float value){
        message.add(value);
        return *this;
      }
      MessageOSC & add(String value){
        char* buffer = new char[value.length() + 1];
        value.toCharArray(buffer, value.length() + 1);
        message.add(buffer);
        delete buffer;
        return *this;
      }
      String toString(){
        String value = "";
        char * buffer = new char[128];
        message.getAddress(buffer);
        value += String(buffer);
        delete buffer;
        value += " ";
        for(uint8_t i = 0 ; i < message.size() ; i++){
          switch(message.getType(i)){
            case 'i': value += String(message.getInt(i)); break;
            case 'f': value += String(message.getFloat(i)); break;
            case 's': 
            {
              char * buffer = new char[128];
              message.getString(i, buffer);
              value += String(buffer);
              delete buffer;
            }
              break;
            default : Serial.print(message.getType(i)); break;
          }
          value += " ";
        }
        value += "\n";
        return value;
      }
      MessageOSC & loopBack(EasyOsc * com){
        switch(com->conf.type){
          case USB_SERIAL :
            SLIPSerial.beginPacket();
            message.send(SLIPSerial);
            SLIPSerial.endPacket();
            break;
          default :
            #ifdef ESP32
            com->Udp.beginPacket(IPAddress(127, 0, 0, 1), com->conf.inPort);
            message.send(com->Udp);
            com->Udp.endPacket();
            #endif
            break;
        }
        return *this;
      }
      MessageOSC & send(EasyOsc * com){
        switch(com->conf.type){
          case USB_SERIAL :
            SLIPSerial.beginPacket();
            message.send(SLIPSerial);
            SLIPSerial.endPacket();
            break;
          default :
            #ifdef ESP32
            com->Udp.beginPacket(com->conf.outIP, com->conf.outPort);
            message.send(com->Udp);
            com->Udp.endPacket();
            #endif
            break;
        }
        return *this;
      }
    };

    struct ConnectionConf {
      String ssid;
      String pwd;
      uint16_t inPort;
      uint16_t outPort;
      IPAddress outIP;
      CONNECTION_TYPE type;
    public :
      ConnectionConf () { setSSID("easy-OSC").setPWD("").setInPort(8888).setOutPort(9999).setOutIP(IPAddress (192, 168, 4, 255)).setConType(ACCESS_POINT); }
      ConnectionConf (const ConnectionConf &copy  ) {
        setSSID(copy.ssid);
        setPWD(copy.pwd);
        setInPort(copy.inPort);
        setOutPort(copy.outPort);
        setOutIP(copy.outIP);
        setConType(copy.type);
      }
      // ConnectionConf (String ssid = "easy-OSC", String pwd = "", uint16_t inPort = 8888, uint16_t outPort = 9999, IPAddress outIP = IPAddress (192, 168, 4, 255), CONNECTION_TYPE type = ACCESS_POINT) : ssid(ssid), pwd(pwd), inPort(inPort), outPort(outPort), outIP(outIP), type(type){}
      ConnectionConf & setSSID(String value) { 
        ssid = value;
        return *this; 
      }
      ConnectionConf & setPWD(String value) { 
        pwd = value;
        return *this; 
      }
      ConnectionConf & setInPort(uint16_t value) { 
        inPort = value;
        return *this; 
      }
      ConnectionConf & setOutPort(uint16_t value) { 
        outPort = value;
        return *this; 
      }
      ConnectionConf & setOutIP(IPAddress value) { 
        outIP = value;
        return *this; 
      }
      ConnectionConf & setConType(CONNECTION_TYPE value) { 
        type = value;
        return *this; 
      }
      String toString(){
        return 
          "conType : " + String((uint8_t)type) + "\n" +
          "SSID    : " + ssid + "\n" +
          "pwd     : " + pwd + "\n" +
          "inPort  : " + String(inPort) + "\n" +
          "outPort : " + String(outPort) + "\n" +
          "outIP   : " + String(outIP[0]) + "." + String(outIP[1]) + "." + String(outIP[2]) + "." + String(outIP[3]) + "\n";
      }
    };

    #ifdef ESP32
      EasyOsc (ConnectionConf conf = ConnectionConf()) : conf(conf) {}
    #else
      EasyOsc () {}
    #endif

    #ifdef ESP32
      IPAddress begin() {
        switch (conf.type) {
          case USB_SERIAL : break;

          case REGULAR_WIFI:
            if(conf.pwd.equals(""))WiFi.begin(conf.ssid);
            else WiFi.begin(conf.ssid, conf.pwd);
            Udp.begin(conf.inPort);
           return WiFi.localIP();

          case ACCESS_POINT:
            if(conf.pwd.equals("")){
              if(!WiFi.softAP(conf.ssid)){
                Serial.println("Soft AP creation failed.");
                while(1);
              }
            } else {
              if(!WiFi.softAP(conf.ssid, conf.pwd)){
                Serial.println("Soft AP creation failed.");
                while(1);
              }
            }
            Udp.begin(conf.inPort);
            return WiFi.softAPIP();
        }
        return IPAddress();
      }
    #else
      void begin() {
        conf.con_type = USB_SERIAL;
      }
    #endif

    IPAddress getIp(){
      switch (conf.type) {
        case USB_SERIAL : break;
        case REGULAR_WIFI : return WiFi.localIP();
        case ACCESS_POINT : return WiFi.softAPIP();
      }
      return IPAddress();
    }

    String onMessage(String address, String validator, MessageHandler handler) {
      handler.setValidator(validator);
      return onMessage(address, handler);
    }

    String onMessage(String address, MessageHandler handler) {
      listeners.set(address, handler);
      return address +  " \"" + handler.validator + "\"\n";
    }

    void update() {
      OSCMessage messageIN;
      int size;

      switch(conf.type){
        case USB_SERIAL : 
          if (SLIPSerial.available())
            while (!SLIPSerial.endofPacket())
              while (SLIPSerial.available())
                messageIN.fill(SLIPSerial.read());
        break;
        default : 
          #ifdef ESP32
            if ( (size = Udp.parsePacket()) > 0)
              while (size--) messageIN.fill(Udp.read());
          #endif
        break;
      }
      
      if (messageIN.hasError()) return;

      if(listeners.contains("*")){
        listeners.get("*")(messageIN);
      }
      
      char buffer[64];
      int result = messageIN.getAddress(buffer);

      if (listeners.contains(buffer)){
        listeners.get(buffer)(messageIN);
      }
    }

    String toString(){
      IPAddress ip = getIp();
      String t = 
        "conType : " + CONNECTION_TYPE_DICT[(uint8_t)conf.type] + "(" + String((uint8_t)conf.type) + ")" +"\n" +
        "SSID    : " + conf.ssid + "\n" +
        "pwd     : " + conf.pwd + "\n" +
        "inOSC   : " + String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]) + ":" + String(conf.inPort) + "\n" +
        "outOSC  : " + String(conf.outIP[0]) + "." + String(conf.outIP[1]) + "." + String(conf.outIP[2]) + "." + String(conf.outIP[3]) + ":" + String(conf.outPort) + "\n" ;
      return t;
    }

    ConnectionConf conf;
    WiFiUDP Udp;
  private :
    Dictionary<String, MessageHandler> listeners;
};


#endif //easyOsc_h