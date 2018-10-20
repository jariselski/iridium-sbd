/*
 * ISBD.cc
 * 
 * Arduino library for a Rockblock Iridium SBD modem.     
 * 
 * Copyright (c) 2018 Jari Kruetzfeldt, Jakob Kuttenkeuler
 * 
 * Partly based on the IridumSBD Library by Mikal Hart available at http://arduiniana.org. 
 * 
 * MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "ISBD.h"


ISBD::ISBD(Stream &iridium_stream, Stream &console_stream, int modem_power_pin, int modem_sleep_pin)
{
        iridiumStream_ = &iridium_stream;
        consoleStream_ = &console_stream;
        modemPowerPin_ = modem_power_pin;
        modemSleepPin_ = modem_sleep_pin;
}


ISBD::~ISBD()
{
        disableModem();
}


bool ISBD::getNetworkStatus()
{
        #ifdef ISBD_CONSOLE        
                console(F("CHECKING NETWORK SERVICE\n"));
        #endif
        if (enableModem() != ISBD_SUCCESS) return ISBD_ERR_NO_MODEM_DETECTED;
        return checkNetworkService();
}


int ISBD::sendTextMsg(String& msg_out)
{
        #ifdef ISBD_CONSOLE        
                console(F("SENDING TXT MSG\n"));
        #endif
        gMTqueued_ = 99;
        if (msg_out.length() <= 0)                              return ISBD_ERR_MSG_SIZE;
        if (enableModem() != ISBD_SUCCESS)                      return ISBD_ERR_NO_MODEM_DETECTED;
        flushSerialRxBuffer();            
        bool success = getSbdStatus(); 
        if (gMOBuffer_ > 0) {
                clearMOBuffer(); 
                // delay(300);                                     // TODO: needed?
                success=getSbdStatus();
        } 
        if (!success)                                           return ISBD_ERR_GET_STATUS; 
        if (!uploadTxtMsgToModem(msg_out))                      return ISBD_ERR_UPLOAD_TO_MODEM;
        if (!connectToSatellites(transmissionTimeoutSec_))      return ISBD_ERR_SENDRECEIVE_TIMEOUT;
        if (gMOBuffer_ == 2)                                    return ISBD_ERR_NOT_SPECIFIED;          // gMOBuffer_ should be ether 0 or 1
        return ISBD_SUCCESS;
}


int ISBD::sendReceiveTxtMsg(String& msg_out, String& msg_in, int& num_msg_in)
{
        #ifdef ISBD_CONSOLE
                console(F("SENDING/RECEIVING TXT MSG\n"));
        #endif        
        
        gMTqueued_ = 0;                                  
        gMOBuffer_ = 0;                         
        if (msg_out.length() <= 0)              return ISBD_ERR_MSG_SIZE;
        if (enableModem() != ISBD_SUCCESS)      return ISBD_ERR_NO_MODEM_DETECTED;
        flushSerialRxBuffer();          
        bool success = getSbdStatus(); 
        if (gMOBuffer_ > 0) {
                clearMOBuffer();
                // delay(300);                     // TODO: needed?
                success = getSbdStatus();
        } 
        if (!success)                           return ISBD_ERR_GET_STATUS; 
        if (!uploadTxtMsgToModem(msg_out))      return ISBD_ERR_UPLOAD_TO_MODEM;
        do {                                    //We are only using the latest incoming message
                success = connectToSatellites(transmissionTimeoutSec_);
        } while (gMTqueued_);
        if (!success)                           return ISBD_ERR_SENDRECEIVE_TIMEOUT;
        msg_in     = "";
        num_msg_in = 0;
        if (gMTBuffer_ > 0) {                    //Incoming message available at modem 
                if (getIncomingTxtMsgFromModem(msg_in)) num_msg_in = 1;
        } 
        return ISBD_SUCCESS;
}


int ISBD::sendBinaryMsg(const uint8_t *tx_data, size_t tx_buffer_size)
{
        #ifdef ISBD_CONSOLE
                console(F("SENDING BINARY MSG\n"));
        #endif 
        if (tx_buffer_size <= 0)                                return ISBD_ERR_MSG_SIZE;        
        if (enableModem() != ISBD_SUCCESS)                      return ISBD_ERR_NO_MODEM_DETECTED;
        flushSerialRxBuffer();
        gMOBuffer_ = 0;                        
        bool success = getSbdStatus(); 
        if (gMOBuffer_>0) {
                clearMOBuffer();
                // delay(300);             // TODO: needed?
                success=getSbdStatus();
        }
        if (!success)                                           return ISBD_ERR_GET_STATUS; 
        if (!uploadBinaryMsgToModem(tx_data,tx_buffer_size))    return ISBD_ERR_UPLOAD_TO_MODEM;
        if (!connectToSatellites(transmissionTimeoutSec_))      return ISBD_ERR_SENDRECEIVE_TIMEOUT;
        if (gMOBuffer_==2)                                      return ISBD_ERR_NOT_SPECIFIED;          // gMOBuffer_ should be ether 0 or 1
        return ISBD_SUCCESS;
}


int sendReceiveBinaryMsg(const uint8_t *tx_data, size_t tx_data_size, uint8_t *rx_Buffer, size_t &rx_buffer_size)
{
        // TODO: Implement
        return ISBD_ERR_NOT_SPECIFIED;
}


bool ISBD::getModemIsEnabled()
{
        return modemIsEnabled_;
}


String ISBD::getModemIMEI()
{
        sendToModem(F("AT+CGSN\r"));
        String imei_str = "";          
        waitForModemResponse(10, F("OK\r\n"), imei_str);  
        stripModemReturnString(imei_str);
        return imei_str;
} 

String ISBD::getModemManufacturerId()
{
        sendToModem(F("AT+CGMI\r"));
        String manufacturer_str = "";     
        waitForModemResponse(10, F("OK\r\n"), manufacturer_str);  
        stripModemReturnString(manufacturer_str);
        return manufacturer_str;
} 

String ISBD::getModemModelId()
{
        sendToModem(F("AT+CGMM\r"));
        String model_str = "";            
        waitForModemResponse(10, F("OK\r\n"), model_str);   
        stripModemReturnString(model_str);       
        return model_str;
} 


bool ISBD::checkNetworkService()
{
        unsigned long start_time_ms = millis();
        bool found_network_service = false;
        while (!found_network_service) {
                sendToModem(F("AT+CIER=1,0,1\r"));
                delay(200);
                found_network_service = waitForModemResponse(10, F("+CIEV:1,1\r\n"));
                if ( !found_network_service && (millis() >= (start_time_ms + (networkCheckTimeoutSec_*1000))) ) {           
                        #ifdef ISBD_CONSOLE
                                console(F("Timeout\n"));
                        #endif 
                        return false;
                }
        }
        return true;
}


int ISBD::getModemPowerPin()
{
        return modemPowerPin_;
}

int ISBD::getModemSleepPin()
{
        return modemSleepPin_;
}

String ISBD::getLibraryNameAndVersion()
{       
        String name = ISBD_NAME;
        #ifdef ISBD_CONSOLE
                name += " (Console)";   // Add console indication
        #endif 
        String version = ISBD_VERSION;
        return (name + " " + version);
}

bool ISBD::getIsConsolePrint()
{
        return isConsolePrint_;
}

bool ISBD::setIsConsolePrint(const bool is_console_print)
{
        #ifdef ISBD_CONSOLE
                if (!is_console_print) console(F("Console OFF\n"));
                isConsolePrint_ = is_console_print;
                if (is_console_print) console(F("Console ON\n"));
                return true;
        #endif 
        
        return false;   // Console not compiled 
}



int ISBD::getLowPowerUpTimeSec()
{
        return lowPowerUpTimeSec_;
}

void ISBD::setLowPowerUpTimeSec(int low_power_up_time_sec)
{
        lowPowerUpTimeSec_ = low_power_up_time_sec;
}





//----------------------------------------------------// 

bool ISBD::uploadTxtMsgToModem(String& msg_out)
{  
        #ifdef ISBD_CONSOLE
                console(F("Uploading txt msg\n"));
        #endif 
        if (msg_out.length() > ISBD_TXT_MAX_TX_MSG_SIZE) {
                msg_out = msg_out.substring(0, ISBD_TXT_MAX_TX_MSG_SIZE);
        }
        sendToModem(F("AT+SBDWT="));  
        sendToModem(msg_out.c_str());  
        sendToModem(F("\r"));
        if (!waitForModemResponse(10, F("OK\r\n"))) {
                return false;
        }
        return true;
}


bool ISBD::uploadBinaryMsgToModem(const uint8_t *tx_data, size_t tx_data_size)
{
        #ifdef ISBD_CONSOLE
                console(F("Uploading bin msg\n"));
        #endif 
        if (tx_data_size > ISBD_BIN_MAX_TX_MSG_SIZE) {
                tx_data_size = ISBD_BIN_MAX_TX_MSG_SIZE;
        } 
        delay(200);
        sendToModem(F("AT+SBDWB="));                            //Sending binary mode
        sendToModem(String(tx_data_size ,DEC));                 //Msg buffersize  // TODO: This was console.print before (instead of write), does it work? @test
        sendToModem("\r"); 
        delay(200);
        uint16_t checksum = 0;
        for (unsigned int i=0; i<tx_data_size; ++i) {
                iridiumStream_->write(tx_data[i]);              //Transfering byte by byte
                checksum += (uint16_t)tx_data[i];
        }
        iridiumStream_->write(checksum >> 8);                   //Highbyte 
        iridiumStream_->write(checksum & 0xFF);                 //Lowbyte
        delay(200);
        bool success = false; 
        success = waitForModemResponse(60, F("READY\r\n"));  
        success = waitForModemResponse(60, F("0\r\n\r\nOK\r\n")); 
        success = getSbdStatus();
        if (!success) {
                return false;
        }
        return true;
}


bool ISBD::waitForModemResponse(long timeout_sec, String ending)
{
        String response = "";
        return waitForModemResponse(timeout_sec, ending, response);
}
bool ISBD::waitForModemResponse(long timeout_sec, String ending, String& response)
{
        #ifdef ISBD_CONSOLE
                console(F("From Modem: "));                                         
        #endif 
        response = "";
        const unsigned long start_time_ms = millis();   //RTC does not work here, possibly signal distortion when communicating with Iridium modem leads to wrong readings 
        while (!strS1EndsWithS2(response, ending)) {
                if (iridiumStream_->available()) {
                        char c   = iridiumStream_->read(); 
                        response += String(c);
                        #ifdef ISBD_CONSOLE
                                consoleHeaderless(String(c));                                         
                        #endif 
                } 
                if (millis() >= (start_time_ms+(timeout_sec*1000))) {
                        return false;
                }
        }
        return true;
}


bool ISBD::getSbdStatus()
{
        String response = "";        
        sendToModem(F("AT+SBDS\r"));   // +SBDS: 1, 55, 0, -1
        if(!waitForModemResponse(20, F("OK\r\n"), response)) return false;
        int index_1 = response.indexOf(':');     
        int index_2 = response.indexOf(',');
        gMOBuffer_  = (byte)response.substring(index_1+1,index_2).toInt(); 
        index_1     = index_2;    
        index_2     = response.indexOf(',',index_1+1);
        // int MOMSN  = response.substring(index_1+1,index_2).toInt();
        index_1     = index_2;    
        index_2     = response.indexOf(',',index_1+1);
        gMTBuffer_  = (byte)response.substring(index_1+1,index_2).toInt();
        index_1     = index_2;   
        index_2     = response.indexOf(',',index_1+1);
        // int MTMSN  = response.substring(index_1+1,index_2).toInt();
        return true;
}


bool ISBD::getSbdAttention()
{
        sendToModem(F("AT\r"));  
        if(!waitForModemResponse(10, F("OK\r\n"))) {
                return false;
        }
        return true;
}


bool ISBD::connectToSatellites(const long timeout_sec) 
{
        #ifdef ISBD_CONSOLE
                console(F("Connecting to satellites ... (Press 'c' to cancel)\n"));  
        #endif 

        unsigned long start_time_ms = millis();
        String response = "";
        do {    // repeat until no error or timeout
                sendToModem("AT+SBDI\r");
                if(!waitForModemResponse(60, F("OK\r\n"), response)) return false; 

                int index_1 = response.indexOf(':'); 
                int index_2 = response.indexOf(',');
                gMOBuffer_  = (byte)response.substring(index_1+1,index_2).toInt();
                index_1     = index_2;  
                index_2     = response.indexOf(',',index_1+1);
                // int MOMSN  = response.substring(index_1+1,index_2).toInt();
                index_1     = index_2;     index_2 = response.indexOf(',',index_1+1);
                gMTBuffer_  = (byte)response.substring(index_1+1,index_2).toInt();
                index_1     = index_2;     index_2 = response.indexOf(',',index_1+1);
                // int MTMSN  = response.substring(index_1+1,index_2).toInt();
                index_1     = index_2;     
                index_2     = response.indexOf(',',index_1+1);
                gMTLength_  = response.substring(index_1+1,index_2).toInt();
                index_1     = index_2;     
                index_2     = response.indexOf(',',index_1+1);
                gMTqueued_  = response.substring(index_1+1,index_2).toInt();
                
                if( millis() > (start_time_ms+(timeout_sec*1000)) ) {
                        #ifdef ISBD_CONSOLE
                                console(F("Timeout\n"));
                        #endif 
                        return false;
                }
                
                if(consoleStream_->read() == 'c') {    
                        #ifdef ISBD_CONSOLE
                                console(F("Canceled\n"));
                        #endif  
                        gMTqueued_ = 0;                  // Reset on cancel
                        gMTBuffer_ = 0;                  // Reset on cancel
                        return false;
                }
        } while (gMOBuffer_==2);
        #ifdef ISBD_CONSOLE
                console(F("Success\n"));
        #endif 
        if (gMOBuffer_ == 1) {
                clearMOBuffer();
                delay(100);
        }
        return true;
}


bool ISBD::getIncomingTxtMsgFromModem(String& msg)
{  
        #ifdef ISBD_CONSOLE
                console(F("Downloading incoming msg\n"));
        #endif 
        
        sendToModem(F("AT+SBDRB\r"));            //Get message from modem 

        unsigned long start_time_ms = millis();     //Millis should be removed in a later version, but is works here
        while (iridiumStream_->available() < 2) {
                if (millis() > start_time_ms+15000UL) {
                        break;
                }
        }

        //Read incoming data as: rxSize[2], body[rxsSize], checksum[2]
        uint16_t rx_size = 256 * iridiumStream_->read() + iridiumStream_->read(); //Incoming message size
        if(rx_size > ISBD_TXT_MAX_RX_MSG_SIZE){rx_size = ISBD_TXT_MAX_RX_MSG_SIZE;}      //Limit incoming size.

        uint16_t checksum  = 0;
        uint16_t bytes_read = 0;
        uint8_t  rx_buffer [(size_t)rx_size];
        char     input [(size_t)rx_size]; 

        start_time_ms = millis();
        while (bytes_read < rx_size) {
                if ( iridiumStream_->available() ) {
                        uint8_t t_ui8In       = iridiumStream_->read();
                        rx_buffer[bytes_read] = t_ui8In;
                        checksum             += (uint16_t)t_ui8In;
                        bytes_read++; 
                }
                if ( millis()>(start_time_ms+30000UL) ) {
                        break;
                }
        }
  
        start_time_ms = millis();
        while (iridiumStream_->available() < 2) {
                if ( millis()>start_time_ms+15000UL ) {
                        break;
                }
        }

        uint16_t checksumModem = 256 * iridiumStream_->read() + iridiumStream_->read();

        delay(10);
        msg = "";
        if(checksumModem == checksum) {
                for (int i=0; i<rx_size; ++i) {
                        input[i] = (char)rx_buffer[i];
                        msg     += (String)input[i];  
                }
                // delay(5);                              // TODO: needed?
        }
        else {
                return false;
        }

        clearMTBuffer();                // Clear incoming buffer!
        return true;
}


bool ISBD::strS1EndsWithS2(const String &S1, const String &S2)
{       
        int y = S1.substring( S1.length()-0-S2.length(), S1.length()-0 ).compareTo(S2);
        if (y != 0) return false;
        return true;
}


int ISBD::clearMOBuffer()
{
        //delay(1000);    // TODO: why?? @test 
        sendToModem(F("AT+SBDD0\r")); // Clear the message out buffer
        if (!waitForModemResponse(60, F("OK\r\n"))) return ISBD_ERR_CLEAR_MODEM_BUFFER;
        return ISBD_SUCCESS;
}


int ISBD::clearMTBuffer()
{
        //delay(1000); // TODO: why?? @test
        sendToModem(F("AT+SBDD1\r")); // Clear the message out buffer
        if(!waitForModemResponse(60,"OK\r\n")) return ISBD_ERR_CLEAR_MODEM_BUFFER;
        return ISBD_SUCCESS;        
}


void ISBD::flushSerialRxBuffer()
{ 
        while (iridiumStream_->available()) iridiumStream_->read();
}


void ISBD::enableSleep()
{
        if (!modemSleepPin_) return;
        digitalWrite(modemSleepPin_ , LOW);      //Modem sleepPin -- LOW -> asleep

}


void ISBD::disableSleep()
{
        if (!modemSleepPin_) return;        
        digitalWrite(modemSleepPin_ , HIGH);      //Modem sleepPin -- HIGH -> awake
}


void ISBD::enableModemPower()
{
        if (!modemPowerPin_) return;
        digitalWrite(modemPowerPin_ , HIGH);     //activate Iridium modem
}


void ISBD::disableModemPower()
{
        if (!modemPowerPin_) return;
        digitalWrite(modemPowerPin_ , LOW);      //deactivate Iridium modem
}


bool ISBD::modemPowerUp()
{
        #ifdef ISBD_CONSOLE
                console(F("Power up\n"));
        #endif 
        enableModemPower();
        delay(1000);                                                    // wait for power up
        disableSleep();
        delay(1000);                                                    // wait for wake up // TODO: Is this enough?
        if(!getSbdAttention()) return false;   
        sendToModem(F("Z0\r"));                                         // SoftReset
        sendToModem(F("ATE0\r"));                                       // Turn off Echo
        sendToModem(F("AT&K0\r"));                                      // Disable RTS/CTS flow control for 3-wire mode
        if ( !waitForModemResponse(10, F("OK\r\n")) ) return false;  
        return true;
}


void ISBD::modemPowerDown()
{
        #ifdef ISBD_CONSOLE
                console(F("Power down\n"));        
        #endif 
        sendToModem(F("AT*F\r"));               // Flushs pending writes to EEPROM and waits for completion before shut-down
        waitForModemResponse(20, F("OK\r\n"));  // Wait for response before shut-down
        enableSleep();                          // We are shutting the modem down anyway
        delay(100);                             // Wait for serial to be turned off
        disableModemPower();
}     


int ISBD::enableModem()
{
        if(modemIsEnabled_) return ISBD_SUCCESS;
        #ifdef ISBD_CONSOLE
                console(F("Enable modem\n"));
        #endif 
        if ( !modemPowerUp() ) {   
                #ifdef ISBD_CONSOLE
                        console(F("Modem unavailable. Trying again in "), String(lowPowerUpTimeSec_), F("sec. (Press 'c' to cancel)\n"));
                #endif 
                // In a low power application, we need to wait 
                const unsigned long millis_start = millis();
                while (millis() < (millis_start + lowPowerUpTimeSec_)) {
                        if(consoleStream_->read() == 'c') {    
                                #ifdef ISBD_CONSOLE
                                        console(F("Canceled\n"));
                                #endif  
                                enableSleep();
                                delay(100);                     // Wait for serial to be turned off
                                disableModemPower();
                                modemIsEnabled_ = false;     
                                return ISBD_ERR_NOT_SPECIFIED;
                        }
                        delay(500);
                }
                if ( !modemPowerUp() ) {                 
                        enableSleep();
                        delay(100);                     // Wait for serial to be turned off
                        disableModemPower();
                        modemIsEnabled_ = false;     
                        return ISBD_ERR_NO_MODEM_DETECTED;
                }
        }
        modemIsEnabled_ = true;
        return ISBD_SUCCESS;
}


int ISBD::disableModem()
{
        if(!modemIsEnabled_) return ISBD_SUCCESS;
        #ifdef ISBD_CONSOLE
                console(F("Disable modem\n"));
        #endif 
        modemPowerDown();
        modemIsEnabled_ = false;
        return ISBD_SUCCESS;
} 


void ISBD::sendToModem(const String msg)
{
        #ifdef ISBD_CONSOLE
                console(F("To modem: "), msg, F("\n"));
        #endif 
        iridiumStream_->print(msg);
}


/**
 * Strip modem return string 
 * 
 * The modem return on IMEI, Modem ID, etc requests needs to be stripped.
 * Since this returns have a similar pattern (see example) we can easily strip 
 * the message. 
 * Example return: \r\nIridium\r\nOK\r\n    
 */
bool ISBD::stripModemReturnString(String& msg)
{
        if (msg.length() < 3) return false;     // Check if correct lenght
        msg = msg.substring(2);                 // Strip leading \r\n 
        const int index = msg.indexOf('\r');    // Find trailing \r\n
        if (index == -1) return false;  
        msg = msg.substring(0, index);          // Strip trailing part
        return true;
}


#ifdef ISBD_CONSOLE
        void ISBD::console(const String msg, const String data, const String ending)
        {
                console(msg+data+ending);
        }
        void ISBD::console(const String msg)
        {       
                if (isConsolePrint_) {
                        String header = "[" + String(ISBD_NAME) + " " + String(millis()/1000.0, 2) + "] "; 
                        consoleStream_->print(header + msg);
                }
        }


        void ISBD::consoleHeaderless(const String msg)
        {       
                if (isConsolePrint_) {
                        consoleStream_->print(msg);
                }
        }
#endif 
