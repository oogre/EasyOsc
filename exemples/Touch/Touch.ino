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


}

void loop(){
    com.update();

    EasyOsc::MessageOSC("/touch/4").add(touchRead(4)).send(&com);
    EasyOsc::MessageOSC("/touch/12").add(touchRead(12)).send(&com);
    delay(30);
}