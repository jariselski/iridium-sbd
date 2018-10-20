/**
 * Test Iridium
 */


#include "ISBD/ISBD.h"

#define IRIDIUM_POWER_PIN       12
#define IRIDIUM_SLEEP_PIN       21

ISBD isbd(Serial3, Serial, IRIDIUM_POWER_PIN, IRIDIUM_SLEEP_PIN);

void setup()
{

        Serial.begin(115200);                   
        Serial3.begin(ISBD_SERIAL_BAUDRATE);                            // This is hardware serial and we need to initialize the port

        delay(5000);                                                    // Wait for me to start my serial monitor!              

        isbd.setIsConsolePrint(true);                                   // Enable console print

        String name = isbd.getLibraryNameAndVersion();
        print("Library name & version = " + name + "\n");

        int status = -1;
        status = isbd.getModemPowerPin();
        print("Power pin = " + String(status) + "\n");

        status = isbd.getModemIsEnabled();
        print("Modem status (before enable) = " + String(status) + "\n");

        // Enable modem
        status = isbd.enableModem();
        print("Enable return code = " + String(status) + "\n");

        status = isbd.getModemIsEnabled();
        print("Modem status (after enable) = " + String(status) + "\n");

        // Get modem IMEI 
        String imei = isbd.getModemIMEI();
        print("Modem IMEI = " + imei + "\n");
        
        // Get modem manufacturer ID
        String manufacturer = isbd.getModemManufacturerId();
        print("Manufacturer ID = " + manufacturer + "\n");

        // Get modem model ID
        String model = isbd.getModemModelId();
        print("Model ID = " + model + "\n");

        // Get network availability
        bool network_available = false;
        network_available = isbd.getNetworkStatus();
        print("Network availability = " + String(network_available) + "\n");


        // Send and receive text message
        print(F("Send SBD text message.\n"));        
        String msg_out = "Hello world!";
        String msg_in  = "";
        int num_msg_in = 0;
        status = isbd.sendReceiveTxtMsg(msg_out, msg_in, num_msg_in);
        if (status == ISBD_SUCCESS) {
                print("Msg out = " + msg_out + "\n");
                if (num_msg_in) print("Msg in  = " + msg_in + "\n");
                else            print("No incoming msg.\n");
        } else {
                print("Error during sendReceiveTxtMsg(). Error code: " + String(status) + "\n");
        }


        // Sending lots of data with binary messages
        print(F("Send SBD binary messages.\n"));        
        const int transmission_num_msgs = 5;
        size_t  bin_buffer_size_byte = ISBD_BIN_MAX_TX_MSG_SIZE;
        uint8_t bin_msg[bin_buffer_size_byte];
        bin_msg[2] = 0x13;                                           // Make msg unique                         
        bin_msg[3] = 0xFF;                                           // Make msg unique                         
        unsigned int msg_counter = 0;
        while (msg_counter < transmission_num_msgs) {
                bin_msg[0] = msg_counter; 
                isbd.sendBinaryMsg(bin_msg, bin_buffer_size_byte);
                msg_counter++;
        }

}


void loop()
{
        // Get network availability
        bool network_available = false;
        network_available      = isbd.getNetworkStatus();
        print("Network availability = " + String(network_available) + "\n");
        isbd.disableModem();
        delay(30000);
        
        // Switch console print
        isbd.getIsConsolePrint() ? isbd.setIsConsolePrint(false) : isbd.setIsConsolePrint(true);
}



void print(String msg)
{
        Serial.print(msg);
}