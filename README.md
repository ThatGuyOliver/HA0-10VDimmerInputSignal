# Smart 0–10V Dimmer Replacement with Arduino & Shelly 2PM PRO

This project replaces a traditional Niko modular analogue dimmer (e.g. Niko Modular analogue dimmer, 0–10V input, 330-00701) by combining a PLC-based 0–10V control system with modern smart lighting using 
an Arduino-based controller and Shelly 2PM Pro devices.

The solution was chosen not only for its flexibility, but also because it is significantly more cost-effective than replacing or expanding the original Niko hardware. Additionally, many of these Niko modules are aging 
and becoming harder to maintain or source. This approach upgrades the existing installation into a smart, network-connected, and fully local system without requiring a complete redesign.

## Overview

The system reads analogue control signals (0–10V equivalent, scaled in software) from a Niko 05-000-02 module and translates them into network-based commands for Shelly dimmers. This enables a classic electrical installation 
to be upgraded into a smart lighting system while preserving the existing control logic.

Instead of directly dimming loads through analogue hardware, the Arduino acts as a bridge between the Niko control signals and IoT devices.
## How It Works

1. A Niko 05-000-02 module generates analogue control values (0–10V equivalent of a 0–100% brightness range).

2. The Arduino Opta reads these values via PLC variables.

3. Based on changes and hysteresis filtering, the Arduino sends HTTP commands over Ethernet.

4. Shelly 2PM Pro devices receive these commands and adjust lighting accordingly.

### Signal Conversion Example

The PLC converts raw signals into percentage values:

    dimmer11Control = LIMIT((signal1 × 100 / 60000), 0, 100)

    dimmer12Control = LIMIT((signal2 × 100 / 60000), 0, 100)

This maps the analogue input range to a usable brightness percentage.
## Key Features

- Direct replacement for Niko analogue dimmers using existing wiring.

- Supports up to 8 channels with a base Arduino; expandable via additional modules.

- Two dimmer channels per Shelly 2PM Pro device.

- DIN rail mountable components for clean electrical cabinet integration.

- Hysteresis filtering to reduce unnecessary network traffic.

- Watchdog protection for improved reliability.

- Compatible with the Shelly RPC API.

- Fully local operation (no cloud dependency).

## Architecture

- Controller: Arduino Opta (Ethernet-based)

- Software: Arduino PLC IDE

- Input: Niko 05-000-02 module (0–10V equivalent signals)

- Output: HTTP RPC commands to Shelly 2PM Pro devices

- Network: Local LAN with static IP configuration

## Software Logic

The Arduino continuously:

- Reads dimmer target values from PLC variables.

- Compares them with previous values using a hysteresis threshold.

- Sends updates only when:

  -The ON/OFF state changes, or

  -The brightness changes beyond the hysteresis threshold.

This minimizes network traffic and improves overall system stability.
### Example Commands

To set brightness:

    /rpc/Light.Set?id=0&on=true&brightness=75

To switch off:

    /rpc/Light.Set?id=0&on=false
## Reliability
- An 8-second watchdog timer ensures automatic recovery in case of failure.

- Ethernet connection is continuously maintained.

- Short HTTP timeouts prevent blocking behavior.

## Use Cases

- Retrofitting existing installations with smart lighting.

- Bridging PLC-controlled systems with IoT devices.

- Preserving industrial control logic while adding smart functionality.

- Integration with systems like Home Assistant.

- Adding diagnostics, logging, or remote monitoring.

## Future Improvements

Shelly has recently released the Shelly Pro Sensor Add-on, which could potentially eliminate the need for the Arduino controller in this setup. This add-on is capable of directly reading analogue signals, 
meaning the 0-10V inputs from the Niko system could be processed without an intermediate device.

If this proves reliable, the architecture could be simplified significantly by allowing the Shelly Pro devices to handle both input and output functionality. 
This would reduce system complexity, hardware cost, and points of failure.

However, this approach has not yet been tested in this project, and further validation is required to confirm compatibility, accuracy, and responsiveness in a real-world installation.

## Hurdles and Lessons Learned

One of the biggest challenges in this project was the very first upload of the program to the Arduino Opta using the Arduino Opta PLC IDE. Getting the device into the correct state for programming was not straightforward, 
and this created a lot of frustration during the initial setup. The issue was eventually solved by using the Arduino Opta Factory Reset Tool. After restoring the device to a clean state, 
the program could be uploaded properly and the Opta started behaving as expected.
