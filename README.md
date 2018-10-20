# Iridium SBD library ISBD 

Lightweight and robust Arduino Iridium Short Burst Data (SBD) library for the
Rockblock Iridium SBD 9600 modem (http://rock7mobile.com). 

This library is specifically suitable and tested for reliable and lightweight
transmission of large binary data sets (typically > 100 messages) using the
Arduino Due.  

Written by Jari Kruetzfeldt and Jakob Kuttenkeuler. See
https://github.com/jaruselski

Partly based on the IridiumSBD library by Mikal Hart
(https://github.com/mikalhart/IridiumSBD). Many thanks.  


## Introduction 

### Get a class instance
Get a instance of the ISBD class. The streams can either be software or hardware
serial. If hardware serial is used, initiate the stream prior to usage, e.g with
```Serial3.begin(ISBD_SERIAL_BAUDRATE)```.  
```cpp 
ISBD(Stream &iridium_stream, Stream &console_stream, int modem_power_pin, int modem_sleep_pin)
```
- Parameter 
    - Iridium stream. This is the serial connection to the Iridium modem. 
    - Console stream. This is the serial connection for the console. 
    - Iridium modem power pin 
    - Iridium modem sleep pin
- Return    
    - Class instance
- Settings  
    - None. 

#### Example
```cpp 
#include "ISBD.h"
int power_pin = 11;
int sleep_pin = 12; 
ISBD isbd(Serial3, Serial, power_pin, sleep_pin);   // Get class instance
Serial.begin(115200);                               // Initiate console stream  
Serial3.begin(ISBD_SERIAL_BAUDRATE);                // Initiate modem stream. Important!
int status = isbd.sendTextMsg("Hello world!");      // Send txt msg
```


### Library name and version 
Use the following function to get the name and version of the library: 
```cpp 
String getLibraryNameAndVersion()
```

### Console 
The library can be compiled with or without console prints. Disable the console
by commenting the following line in ```ISBD.h```. This saves storage!
```cpp 
#define ISBD_CONSOLE   
```

## Settings 
### Modem specific 
- Baudrate            
    - 19200
- Maximum text Tx message size         
    - 120 byte   
- Maximum text Rx message size
    - 135 byte  
- Maximum binary message Tx size         
    - 340 byte   
- Maximum binary message Rx size         
    - 270 byte   

### Default settings 
- Timeout for Iridium SBD message transmission  
    - 300 seconds 
- Timeout for Iridium SBD network check
    - 120 seconds
- Time to power modem up in low power mode 
    - 30 seconds 

### Status codes 
```
ISBD_SUCCESS                    0
ISBD_ERR_NOT_SPECIFIED          1
ISBD_ERR_SERIAL_FAILURE         2
ISBD_ERR_NO_MODEM_DETECTED      3
ISBD_ERR_MSG_SIZE               4
ISBD_ERR_SENDRECEIVE_TIMEOUT    5
ISBD_ERR_UPLOAD_TO_MODEM        6
ISBD_ERR_LOAD_FROM_MODEM        7
ISBD_ERR_GET_STATUS             8 
ISBD_ERR_CLEAR_MODEM_BUFFER     9 
```



## Main functionality 
The functionality of the library includes sending and receiving SBD text
messages, sending SBD binary messages and checking the availability of the
Iridium network. See the examples for details. 

### Checking network availability
Check if the Iridium satellite network is available.  
Use ```setNetworkCheckTimeoutSec(int network_check_timeout_sec)``` to set the
timeout when looking for the network. The timeout can be viewed with
```getNetworkCheckTimeoutSec()```. 
```cpp 
bool getNetworkStatus()
```
- Parameter 
    - None 
- Return    
    - Network availability (true=available / false=not available)
- Settings  
    - Network check timeout [sec] (default = 120sec)



### Sending SBD text messages  
Send an Iridium SBD text message.   
Use ```setTransmissionTimeoutSec(int transmission_timeout_sec)``` to set the
transmission timeout. The timeout can be viewed with
```getTransmissionTimeoutSec()```.
```cpp 
int sendTextMsg(String& msg_out)
```
- Parameter 
    - Message
- Return 
    - Status code
- Settings
    - Transmission timeout [sec] (default = 300sec)



### Sending and receiving SBD text messages  
Send and receive an Iridium SBD text message.   
Use ```setTransmissionTimeoutSec(int transmission_timeout_sec)``` to set the
transmission timeout. The timeout can be viewed with
```getTransmissionTimeoutSec()```.
```cpp 
int sendReceiveTxtMsg(String& msg_out, String& msg_in, int& num_msg_in)
```
- Parameter 
    - Message out
    - Message in 
    - Number of received messages 
- Return 
    - Status code
- Settings
    - Transmission timeout [sec] (default = 300sec)



### Sending SBD binary messages  
Send an Iridium SBD binary message.  
Use ```setTransmissionTimeoutSec(int transmission_timeout_sec)``` to set the
transmission timeout. The timeout can be viewed with
```getTransmissionTimeoutSec()```.  
```cpp 
int sendBinaryMsg(const uint8_t *tx_data, size_t tx_buffer_size);
```
- Parameter 
    - Message 
    - Message size
- Return 
    - Status code
- Settings
    - Transmission timeout [sec] (default = 300sec)





## Sub functionality 

### Enable modem
Enable and initiate Iridium modem.   
Function enables and initiates the Iridium modem, i.e. prepare it for
transmission. In case the function can not enable the modem on first try, it
waits 30 sec (default, this can be altered with ```setTransmissionTimeoutSec(int
transmission_timeout_sec)```) and tries again. In a low power application use
```enableModemPower()``` function (see below) first to do useful stuff while
waiting for the modem to be powered up. The current modem status can be viewed
with ```getModemIsEnabled()```.  
Function does not need to be called explicitly since it is automatically handled
by main functions (see above). 
```cpp 
int enableModem()
```
- Parameter 
    - None
- Return 
    - Status code
- Settings
    - None


### Disable modem
Disable Iridium modem.  
Needs to be called explicitly since it is NOT automatically handled by main
functions (in contrast to enable function). Iridium should not be disabled in
between transmission of several messages. Use the disable function to shut down
modem in order to save battery in case no transmission is planned in the near
future.   
The current modem status can be viewed with ```getModemIsEnabled()```. 
```cpp 
int disableModem()
```
- Parameter 
    - None
- Return 
    - Status code
- Settings
    - None



### Enable and disable modem power
Enable and disable power to Iridum modem.  
Function enables power to the Iridium modem without initiating it. In a low
power applications (e.g. using 5V power from the Arduino power pin) it can be
useful to power up the modem and do other things in the mean time while the
modem capacitor needed for transmission is charged (takes about 30 sec). Does
not need to be called explicitly since it is automatically handled by
enable/disable functions (see above). 
```cpp 
void enableModemPower()  
void disableModemPower()
```
- Parameter 
    - None
- Return 
    - None
- Settings
    - None


### Get modem IMEI 
Get the serial number (IMEI) of the modem.  
```cpp 
String getModemIMEI()
```
- Parameter 
    - None
- Return 
    - IMEI
- Settings
    - None

### Get modem manufacturer ID 
Get the manufacturer ID of the modem.
```cpp
String getModemManufacturerId()
```

- Parameter
    - None
- Return 
    - Manufacturer ID
- Settings
    - None

### Get modem model ID 
Get the model ID of the modem.  
```cpp 
String getModemModelId()
```
- Parameter 
    - None
- Return 
    - Model ID
- Settings
    - None


### Enable/disable console output
Enable/disable debug information and maintenance output on the console.
```cpp 
bool setIsConsolePrint(bool is_console_print)
```
- Parameter 
    - true: enable / false: disable
- Return 
    - true: Setting set / false: Console not attached during compile  
- Settings
    - None


### Example 
```cpp 
/**
 * Send/receive example
 */
#include "ISBD/ISBD.h"

#define IRIDIUM_POWER_PIN 12
#define IRIDIUM_SLEEP_PIN 21

ISBD isbd(Serial3, Serial, IRIDIUM_POWER_PIN, IRIDIUM_SLEEP_PIN);

void setup()
    Serial.begin(115200);                   
    Serial3.begin(ISBD_SERIAL_BAUDRATE);  
    String msg_out = "Hello world!";
    String msg_in  = "";
    int num_msg_in = 0;
    int status = isbd.sendReceiveTxtMsg(msg_out, msg_in, num_msg_in);
}
```



## Todos 
- Implement receive binary messages

## Known issues 

## Version history 

### v0.1 
- Initial release 
