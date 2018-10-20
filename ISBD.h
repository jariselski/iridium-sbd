/*
 * ISBD.h
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

#ifndef ISBD_H
#define ISBD_H


#include "Arduino.h"
#include "Stream.h"

#define ISBD_NAME       "ISBD"
#define ISBD_VERSION    "v0.1"

/* Error codes */
#define ISBD_SUCCESS                            0
#define ISBD_ERR_NOT_SPECIFIED                  1
#define ISBD_ERR_SERIAL_FAILURE                 2
#define ISBD_ERR_NO_MODEM_DETECTED              3
#define ISBD_ERR_MSG_SIZE                       4
#define ISBD_ERR_SENDRECEIVE_TIMEOUT            5
#define ISBD_ERR_UPLOAD_TO_MODEM                6
#define ISBD_ERR_LOAD_FROM_MODEM                7
#define ISBD_ERR_GET_STATUS                     8 
#define ISBD_ERR_CLEAR_MODEM_BUFFER             9 


/* CONSOLE PRINT */ 
#define ISBD_CONSOLE                                    // Comment to disable console prints completly. This saves storage!     

/* Default settings */
#define ISBD_DEFAULT_LOW_POWER_UP_TIME_SEC      30      //[sec] Time to power up modem in low power mode
#define ISBD_DEFAULT_TRANSMISSION_TIMEOUT_SEC   300     //[sec] Timeout for Iridium SBD message transmission
#define ISBD_DEFAULT_NETWORK_CHECK_TIMEOUT_SEC  120     //[sec] Timeout for Iridium SBD network check

/* Irdium SBD modem settings */
#define ISBD_SERIAL_BAUDRATE                    19200
#define ISBD_TXT_MAX_TX_MSG_SIZE                120     //[byte] Maximum txt Tx message size (see Iridium documentation)
#define ISBD_TXT_MAX_RX_MSG_SIZE                135     //[byte] Maximum txt Rx message size (see Iridium documentation)
#define ISBD_BIN_MAX_TX_MSG_SIZE                340     //[byte] Maximum bin Tx message size (see Iridium documentation)
#define ISBD_BIN_MAX_RX_MSG_SIZE                270     //[byte] Maximum bin Rx message size (see Iridium documentation)


class ISBD 
{
public:
        ISBD(Stream &iridium_stream, Stream &console_stream, int modem_power_pin, int modem_sleep_pin);
        ~ISBD();
        
        bool   getNetworkStatus();
        int    sendTextMsg(String& msg_out);
        int    sendReceiveTxtMsg(String& msg_out, String& msg_in, int& num_msg_in);
        int    sendBinaryMsg(const uint8_t *tx_data, size_t tx_buffer_size);
        int    sendBinaryReceiveMsg(const uint8_t *txData, size_t txDataSize, const uint8_t *rxBuffer, size_t &rxBufferSize);

        String getLibraryNameAndVersion();
        int    enableModem();
        int    disableModem();
        void   enableModemPower();
        void   disableModemPower();
        bool   getModemIsEnabled();
        int    getTransmissionTimeoutSec();
        void   setTransmissionTimeoutSec(int transmission_timeout_sec);
        int    getNetworkCheckTimeoutSec();
        void   setNetworkCheckTimeoutSec(int network_check_timeout_sec);
        int    getLowPowerUpTimeSec();
        void   setLowPowerUpTimeSec(int low_power_up_time_sec);        
        String getModemIMEI();
        String getModemManufacturerId();
        String getModemModelId();
        int    getModemPowerPin();
        int    getModemSleepPin();
        bool   getIsConsolePrint();
        bool   setIsConsolePrint(bool is_console_print);


private: 
        Stream *iridiumStream_;
        Stream *consoleStream_;
        
        int    modemSleepPin_           = -1;
        int    modemPowerPin_           = -1;
        int    lowPowerUpTimeSec_       = ISBD_DEFAULT_LOW_POWER_UP_TIME_SEC;
        int    transmissionTimeoutSec_  = ISBD_DEFAULT_TRANSMISSION_TIMEOUT_SEC;  
        int    networkCheckTimeoutSec_  = ISBD_DEFAULT_NETWORK_CHECK_TIMEOUT_SEC;  
        bool   modemIsEnabled_          = false;
        bool   isConsolePrint_          = false;
        byte   gMOBuffer_               = 0;                                            //Mobile originated buffer
        byte   gMTBuffer_               = 0;                                            //Mobile terminated buffer
        int    gMTLength_               = 0;                                            //Length of incoming message [byte]
        int    gMTqueued_               = 99;                                           //Number of incoming messages waiting     
        
        void enableSleep();
        void disableSleep();
        bool modemPowerUp();
        void modemPowerDown();
        void sendToModem(String msg);
        char readFromModem();
        int  getAvailableAtModem();
        bool checkNetworkService();
        bool uploadTxtMsgToModem(String& msg_out);
        bool uploadBinaryMsgToModem(const uint8_t *tx_data, size_t tx_data_size); 
        bool waitForModemResponse(long timeout_sec, String ending);
        bool waitForModemResponse(long timeout_sec, String ending, String& response);
        bool getSbdStatus();
        bool getSbdAttention();
        bool connectToSatellites(long timeout_sec);
        bool getIncomingTxtMsgFromModem(String& msg);
        bool strS1EndsWithS2(const String &S1, const String &S2); 
        int  clearMOBuffer();
        int  clearMTBuffer();
        void flushSerialRxBuffer();
        void clearSbdMOBuffer();
        void clearSbdMTBuffer();
        bool stripModemReturnString(String& msg);

        #ifdef ISBD_CONSOLE        
                void console(String msg);
                void console(const String msg, const String data, const String ending);
                void consoleHeaderless(const String msg);
        #endif
};

#endif