/*  NetworkUp
 *
 *      This sketch uses the Ethernet Shield 2 library and some custom hardware
 *  to ping a web site to determine if the web site is reachable and is up. This
 *  repeats for some user-defined period (in practice, about every 15 minutes).
 */

#include <Ethernet2.h>

#define INTERVAL        900000  /* Every 15 minutes */

byte mac[] = { 0x2C, 0xF7, 0xF1, 0x08, 0x1B, 0x28 };
char server[] = "hollinscertificates.com";
char validate[] = "X-Validate: Y";

EthernetClient client;     // The Ethernet Client
unsigned long nextTime;    // Time (in ms) to check the web
bool connectFlag;          // True if we've connected
bool successFlag;          // True if we found our validate string
byte valPos;               // Position as we scan our string.

void setup() 
{    
    /*
     *  Set red, green LEDs on.
     */
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(5, OUTPUT);
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(5, HIGH);

    /*
     *  Try to initialize our Ethernet library
     */

    if (Ethernet.begin(mac) == 0) {
        /*
         *  Something went wrong. Fast flash red LED.
         */

        digitalWrite(2,LOW);
        digitalWrite(3,LOW);
        for (;;) {
            digitalWrite(5,HIGH);
            delay(200);
            digitalWrite(5,LOW);
            delay(200);
        }
    }

    /*
     *  Quick animation. This also delays us 1 second to allow the Ethernet
     *  chip to come up.
     */
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    digitalWrite(5, LOW);
    delay(200);

    digitalWrite(2,HIGH);
    delay(200);
    digitalWrite(3,HIGH);
    delay(200);
    digitalWrite(5,HIGH);
    delay(200);
    digitalWrite(2,LOW);
    delay(200);
    digitalWrite(3,LOW);
    delay(200);
    digitalWrite(5,LOW);

    nextTime = 0;
    connectFlag = false;
}

void loop() 
{
    /*
     *  At the top of this loop try to get the home page for the
     *  specified web site.
     */

    unsigned long t = millis();
    if (nextTime <= t) {
        client.stop();
        
        digitalWrite(2, LOW);
        digitalWrite(3, LOW);
        digitalWrite(5, LOW);
        if (!client.connect(server,80)) {
            /*
             *  We have an error. Immediately light up red.
             */
            digitalWrite(5, HIGH);      /* Error */
        } else {
            client.println("GET / HTTP/1.1");
            client.println("Host: hollinscertificates.com");
            client.println("User-Agent: arduino-ethernet");
            client.println("Connection: close");
            client.println();

            connectFlag = true;
            successFlag = false;
            valPos = 0;
        }
        nextTime = millis() + INTERVAL;
    }

    if (client.available()) {
        char c = client.read();
        
        if (validate[valPos] == c) {
            ++valPos;
            if (validate[valPos] == 0) {
                successFlag = true;
                digitalWrite(2,HIGH);       /* Success */
            }
        } else {
            valPos = 0;
            /* Catch special case where the mismatch is actually part of the substring */
            /* For example, if we want ABC, and we see ABABC, on the second A, we want */
            /* to actually advance by one. */
            if (validate[valPos] == c) ++valPos;
        }
    } else if (!client.connected() && connectFlag) {
        client.stop();
        connectFlag = false;
        if (successFlag == false) {
            digitalWrite(3,HIGH);           /* Did not see our validation string */
        }
    }
}
