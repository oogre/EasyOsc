#include <EasyOsc.h>
EasyOsc com;

void setup(){
    Serial.begin(115200);
    
    while(!Serial) delay(1); // wait for Serial


    EasyOsc::ConnectionConf conf = EasyOsc::ConnectionConf()
                                    .setSSID("EasyOSC")
                                    .setPWD("")
                                    .setInPort(8888)
                                    .setOutPort(9999)
                                    .setOutIP(IPAddress(192, 168, 4, 255))
                                    .setConType(EasyOsc::CONNECTION_TYPE::ACCESS_POINT);
    com = EasyOsc(conf);
    com.begin();

    Serial.println(com.toString());

    com.onMessage("*", {
        [](OSCMessage & msg) {
            char buffer[64];
            int result = msg.getAddress(buffer);
            Serial.println(buffer);
        }
    });


    com.onMessage("/led/2", "i", {
        [](OSCMessage & msg) {
            pinMode(2, OUTPUT);
            digitalWrite(2, msg.getInt(0));
        }
    });
}

void loop(){
   com.update();
}