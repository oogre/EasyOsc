#include <EasyOsc.h>
EasyOsc com;


int btnPin = 5;
bool btnValue = true;

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

    pinMode(btnPin, INPUT_PULLUP);
}

void loop(){
    com.update();
    if(btnValue != !digitalRead(btnPin)){
        btnValue = !btnValue;
        Serial.println(EasyOsc::MessageOSC("/button/"+String(btnPin)).add(btnValue).send(&com).toString());
    }

    
    delay(30);
}