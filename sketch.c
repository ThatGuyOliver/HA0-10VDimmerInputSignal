#include <Arduino.h>
#include <Ethernet.h>
#include <ArduinoHttpClient.h>
#include "mbed.h"
#include "opta_info.h"

mbed::Watchdog &watchdog = mbed::Watchdog::get_instance();

EthernetClient ethernetClientDimmer1; //This is an object for 1 Shelly Dimmer 2PM 


HttpClient httpClientDimmer1(ethernetClientDimmer1, "IP Addres Dimmer Shelly", 80);

const uint16_t HYST = 3;

uint16_t lastBright11=0, lastBright12=0, lastBright21=0, lastBright22=0;
bool dimmer11On=false, dimmer12On=false, dimmer21On=false, dimmer22On=false;

IPAddress ip( ); //IP Addres Arduino Opta
IPAddress dns(1,1,1,1); //CLOUDFLARE
IPAddress gateway( ); //Gateway/Router Addres 
IPAddress subnet(255,255,255,0);


// ----------------------------
void sendRequest(HttpClient &client, const char *url)
{
    client.get(url);

    int status = client.responseStatusCode();

    unsigned long timeout = millis();

    while (client.connected() && millis() - timeout < 500)
    {
        while (client.available())
        {
            client.read();
            timeout = millis();
        }
    }

    client.stop();
}


// ----------------------------
void handleDimmer(
    HttpClient &client,
    uint16_t target,
    uint16_t &lastBright,
    bool &dimmerOn,
    int id)
{
    bool newOn = (target > 1);
    int diff = (int)target - (int)lastBright;

    if (newOn != dimmerOn || (newOn && (diff > HYST || diff < -HYST)))
    {
        dimmerOn = newOn;
        lastBright = target;

        char url[80];

        if (newOn)
            snprintf(url, sizeof(url),
                     "/rpc/Light.Set?id=%d&on=true&brightness=%u",
                     id, target);
        else
            snprintf(url, sizeof(url),
                     "/rpc/Light.Set?id=%d&on=false",
                     id);

        sendRequest(client, url);
    }
}


void setup()
{
    Ethernet.begin(ip, dns, gateway, subnet);

    watchdog.start(8000); // 8 s watchdog
}


void loop()
{
    watchdog.kick();

    Ethernet.maintain();

    handleDimmer(httpClientDimmer1, PLCOut.dimmer11Aansturing, lastBright11, dimmer11On, 0); //1st Channel Shelly 
    handleDimmer(httpClientDimmer1, PLCOut.dimmer12Aansturing, lastBright12, dimmer12On, 1); //2nd Channel Shelly

    delay(100);
}


extern "C" void RPC_LightOn()
{
    sendRequest(httpClientDimmer1, "/rpc/Light.Set?id=0&on=true");
}
