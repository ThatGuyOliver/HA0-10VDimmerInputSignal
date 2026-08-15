#include <Arduino.h>
#include <Ethernet.h>
#include <ArduinoHttpClient.h>
#include "mbed.h"
#include "opta_info.h"

mbed::Watchdog &watchdog = mbed::Watchdog::get_instance();

//Example for 2 Shelly Dimmers (4 Channels)

EthernetClient ethernetClientDimmer1;
EthernetClient ethernetClientDimmer2;

static const IPAddress SHELLY1_IP(192, 168, XXX, XXX); //IP Shelly, Device IP should be static in device settings
static const IPAddress SHELLY2_IP(192, 168, XXX, XXX);

HttpClient httpClientDimmer1(ethernetClientDimmer1, SHELLY1_IP, 80);
HttpClient httpClientDimmer2(ethernetClientDimmer2, SHELLY2_IP, 80);

const uint16_t HYST = 3;

static constexpr size_t URL_BUF_SIZE               = 80;
static constexpr unsigned long HTTP_TIMEOUT_MS      = 2000;
static constexpr unsigned long INTER_DIMMER_DELAY_MS = 10;
static constexpr unsigned long LOOP_DELAY_MS        = 100;
static constexpr unsigned long RECONNECT_DELAY_MS   = 2000;
static constexpr uint8_t MAX_ERR_COUNT              = 255;

uint16_t lastBright11 = 0, lastBright12 = 0, lastBright21 = 0, lastBright22 = 0;
bool dimmer11On = false, dimmer12On = false, dimmer21On = false, dimmer22On = false;
uint8_t errCount11 = 0, errCount12 = 0, errCount21 = 0, errCount22 = 0;

bool ethernetWasUp = false;
bool forceDimmerSync = true;

IPAddress ip(192, 168, XXX, XXX); //Static IP Adress Arduino Opta PLC 
IPAddress gateway(192, 168, XXX, XXX); //IP Adress Router
IPAddress subnet(255, 255, 255, 0);

bool sendRequest(HttpClient &client, const char *url)
{
    int result = client.get(url);

    if (result < 0)
    {
        client.stop();
        return false;
    }

    int status = client.responseStatusCode();

    client.stop();

    return (status >= 200 && status < 300);
}

bool handleDimmer(
    HttpClient &client,
    uint16_t target,
    uint16_t &lastBright,
    bool &dimmerOn,
    int id,
    uint8_t &errCount,
    bool forceUpdate)
{
    bool newOn = (target > 1);
    int diff = (int)target - (int)lastBright;

    if (forceUpdate ||
        newOn != dimmerOn ||
        (newOn && (diff > HYST || diff < -HYST)))
    {
        char url[URL_BUF_SIZE];

        if (newOn)
        {
            snprintf(url, sizeof(url),
                     "/rpc/Light.Set?id=%d&on=true&brightness=%u",
                     id, target);
        }
        else
        {
            snprintf(url, sizeof(url),
                     "/rpc/Light.Set?id=%d&on=false",
                     id);
        }

        bool ok = sendRequest(client, url);

        if (ok)
        {
            dimmerOn = newOn;
            lastBright = target;
            errCount = 0;
        }
        else
        {
            if (errCount < MAX_ERR_COUNT)
                errCount++;
        }

        return ok;
    }

    return true;
}

void setup()
{
    Ethernet.begin(ip, gateway, subnet);

    //Ethernet.setRetransmissionTimeout(50);
    //Ethernet.setRetransmissionCount(3);

    ethernetClientDimmer1.setTimeout(HTTP_TIMEOUT_MS);
    ethernetClientDimmer2.setTimeout(HTTP_TIMEOUT_MS);

    ethernetClientDimmer1.setSocketTimeout(HTTP_TIMEOUT_MS);
    ethernetClientDimmer2.setSocketTimeout(HTTP_TIMEOUT_MS);

    httpClientDimmer1.setHttpResponseTimeout(HTTP_TIMEOUT_MS);
    httpClientDimmer2.setHttpResponseTimeout(HTTP_TIMEOUT_MS);

    watchdog.start(30000);
}

void loop()
{
    bool ethernetUp = (Ethernet.linkStatus() != LinkOFF);

    if (!ethernetUp)
    {
        ethernetWasUp = false;

        Ethernet.begin(ip, gateway, subnet);
        delay(RECONNECT_DELAY_MS);
    }
    else
    {
        if (!ethernetWasUp)
        {
            forceDimmerSync = true;
            ethernetWasUp = true;
        }

        bool syncOK = true;

        syncOK &= handleDimmer(
            httpClientDimmer1,
            PLCOut.dimmer11Control,
            lastBright11,
            dimmer11On,
            0,
            errCount11,
            forceDimmerSync);

        delay(INTER_DIMMER_DELAY_MS);

        syncOK &= handleDimmer(
            httpClientDimmer1,
            PLCOut.dimmer12Control,
            lastBright12,
            dimmer12On,
            1,
            errCount12,
            forceDimmerSync);

        delay(INTER_DIMMER_DELAY_MS);

        syncOK &= handleDimmer(
            httpClientDimmer2,
            PLCOut.dimmer21Control,
            lastBright21,
            dimmer21On,
            0,
            errCount21,
            forceDimmerSync);

        delay(INTER_DIMMER_DELAY_MS);

        syncOK &= handleDimmer(
            httpClientDimmer2,
            PLCOut.dimmer22Control,
            lastBright22,
            dimmer22On,
            1,
            errCount22,
            forceDimmerSync);

        if (forceDimmerSync && syncOK)
        {
            forceDimmerSync = false;
        }
    }

    delay(LOOP_DELAY_MS);
    watchdog.kick();
}
