/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    ME310.cpp
    string.h
    stdio.h
    ATCommandDataParsing.h
    PathParsing.h

  @brief
    Driver Library for ME310 Telit Modem

  @details
    The library contains a single class that implements a C++ interface to all ME310 AT Commands.
    It makes it easy to build Arduino applications that use the full power of ME310 module

  @version
    2.11.0

  @note

  @author

  @date
    28/10/2020
 */

#include "ME310.h"
#include <string.h>
#include <stdio.h>
#include <ATCommandDataParsing.h>
#include <PathParsing.h>
#include <vector>

using namespace telitAT;
using namespace me310;
using namespace std;

const char ME310::CTRZ[1] = {0x1A};                    ///< String for Termination Ctrl-z
const char *ME310::OK_STRING = "OK";                   ///< String for OK modem answer
const char *ME310::ERROR_STRING = "ERROR";             ///< String for ERROR modem answer
const char *ME310::CONNECT_STRING = "CONNECT";         ///< String for CONNECT modem answer
const char *ME310::CME_ERROR_STRING = "+CME ERROR: ";  ///< String for +CME ERROR modem answer
const char *ME310::SEQUENCE_STRING = ">>> ";           ///< Sequence string
const char *ME310::WAIT_DATA_STRING = "> ";            ///< Wait Data string
const char *ME310::TERMINATION_STRING = "";            ///< Termination character
const char *ME310::NO_CARRIER_STRING = "NO CARRIER";   ///< String for NO CARRIER modem answer

//! \brief Class Constructor
/*!
 * \param aSerial Uart object for serial communication
 */
ME310::ME310(Uart &aSerial): mSerial(aSerial)
{}

//! \brief Class Destructor
/*!
 */
ME310::~ME310()
{
   mSerial.end();
}

/*! \brief Begin method
   \param baudRate baud rate of Uart for serial communication
*/
void ME310::begin(unsigned long baudRate, bool debug)
{
   mBaudrate = baudRate;
   _debug = debug;
   mSerial.begin(baudRate);
}

/*! \brief End method
/*! \details
This method calls the end of the Serial.
*/
void ME310::end()
{
   mSerial.end();
}

/*! \brief Power ON module
/*! \details
This method makes the Power ON of the module
 * \param onoff_gpio GPIO to control ON/OFF
*/
void ME310::powerOn(unsigned int onoff_gpio)
{
  bool is_ready = false;
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  char mybuffer[100];
  while(!is_ready)
  {
    if(attention() == ME310::RETURN_TOUT)
    {
      delay(1000);
    }
    else
    {
      is_ready = true;
      break;
    }
    mSerial.end();
    digitalWrite(onoff_gpio, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(6000);
    digitalWrite(onoff_gpio, LOW);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    begin(mBaudrate);
    for(int i = 0; i < 5; i++)
    {
      if(attention() == ME310::RETURN_TOUT)
      {
        delay(1000);
      }
      else
      {
        is_ready = true;
        break;
      }
    }
    /*Read any remaining bytes from the uart buffer to get rid of any pending response*/
    mSerial.readBytes(mybuffer, 100);
  }
}

// Command Line Prefixes -------------------------------------------------------
//! \brief Implements the AT command and waits for OK answer
/*!
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::attention(tout_t aTimeout)
{
   return send_wait(F("AT"),OK_STRING,aTimeout);
}

//! \brief Implements the A/ command and waits for OK answer
/*! \details
The command immediately executes the previously issued command or commands.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::repeat_last_auto(tout_t aTimeout)
{
   return send_wait(F("A/"),OK_STRING,aTimeout);
}

//! \brief Implements the AT\#/ command and waits for OK answer
/*! \details
The command immediately executes the previously issued command or commands.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::repeat_last(tout_t aTimeout)
{
   return send_wait(F("AT#/"),OK_STRING,aTimeout);
}

//! \brief Generic Modem Control -------------------------------------------------------
//! \details Implements the AT\#SELINT command and waits for OK answer
/*!
This command sets the AT command interface style.
 * \param value    interface value
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::select_interface_style(int value, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SELINT=%d"), value);
   return send_wait((char*)mBuffer,OK_STRING,aTimeout);
}

//! \brief Implements the AT&F command and waits for OK answer
/*! \details
Set configuration parameters to default values.
 * \param value    configuration value
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::set_factory_config (int value, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT&F%d"), value);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the ATZ command and waits for OK answer
/*! \details
Soft Reset
 * \param value    reset type
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::soft_reset (int value, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("ATZ%d"), value);
   return send_wait((char*)mBuffer,OK_STRING,aTimeout);
}

//! \brief Implements the AT&Y command and waits for OK answer
/*! \details
Basic profile on startup.
 * \param value    profile index
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::default_reset_basic_profile_designation (int value, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT&Y=%d"), value);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT&P command and waits for OK answer
/*! \details
Define which full profile is loaded at startup.
 * \param value    profile index
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::default_reset_full_profile_designation (int value, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT&P%d"), value);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT&W command and waits for OK answer
/*! \details
Execution command stores on profile <n> the complete configuration of the device.
 * \param value    profile identifier
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::store_current_configuration (int value, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT&W%d"), value);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT&N command and waits for OK answer
/*! \details
The command displays telephone numbers stored in the internal phonebook.
 * \param value    phonebook record number
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::display_internal_phonebook_number (int value, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT&N%d"), value);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#Z command and waits for OK answer
/*! \details
Set command loads both base section and extended section of the specified user profile stored with
AT&W and selected with AT&P.
 * \param value    Parameter to select the user profile
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::extended_reset (int value, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#Z%d"), value);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT&V command and waits for OK answer
/*! \details
The command displays some of the basic modem configuration settings and parameters
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::display_config_profile(tout_t aTimeout)
{
   return send_wait(F("AT&V"), OK_STRING, aTimeout);
}

//! \brief Implements the AT+CGI command and waits for OK answer
/*! \details
Set command allows to select the installation country code according to ITU-T35 Annex A.
 * \param value    installation country code
 * \param aTimeout timeout in ms
 * \return return code
 */
/*ME310::return_t ME310::country_installation (int value,ME310::tout_t aTimeout)
{
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CGI=%d"), value);
   return send_wait((char*)mBuffer,OK_STRING,aTimeout);
}*/

//! \brief Implements the AT+GCAP command and waits for OK answer
/*! \details
This command returns the equipment supported command set list.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::capabilities_list(tout_t aTimeout)
{
   return send_wait(F("AT+GCAP"), OK_STRING, aTimeout);
}

//! \brief Implements the AT+GMI command and waits for OK answer
/*! \details
This command returns the manufacturer identification.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::manufacturer_identification(tout_t aTimeout)
{
   return send_wait(F("AT+GMI"), OK_STRING, aTimeout);
}

//! \brief Implements the AT+GMM command and waits for OK answer
/*! \details
The command returns the model identification.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::model_identification(tout_t aTimeout)
{
   return send_wait(F("AT+GMM"), OK_STRING, aTimeout);
}

//! \brief Implements the AT+GMR command and waits for OK answer
/*! \details
The command returns the software revision identification.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::revision_identification(tout_t aTimeout)
{
   return send_wait(F("AT+GMR"), OK_STRING, aTimeout);
}

//! \brief Implements the AT+GSN command and waits for OK answer
/*! \details
The command reports the device board serial number.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::serial_number(tout_t aTimeout)
{
   return send_wait(F("AT+GSN"), OK_STRING, aTimeout);
}

//! \brief Implements the AT+CGMI command and waits for OK answer
/*! \details
The command returns device manufacturer identification code .
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::request_manufacturer_identification(tout_t aTimeout)
{
   return send_wait(F("AT+CGMI"),OK_STRING,aTimeout);
}

//! \brief Implements the AT+CGMM command and waits for OK answer
/*! \details
This command returns the device model identification.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::request_model_identification(tout_t aTimeout)
{
   return send_wait(F("AT+CGMM"),OK_STRING,aTimeout);
}

//! \brief Implements the AT+CGMR command and waits for OK answer
/*! \details
The command returns device software revision number.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::request_revision_identification(tout_t aTimeout)
{
   return send_wait(F("AT+CGMR"),OK_STRING,aTimeout);
}

//! \brief Implements the AT+CGSN command and waits for OK answer
/*! \details
This command allows to retrieve the product serial number in form of IMEI of the mobile.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::request_psn_identification(tout_t aTimeout)
{
   return send_wait(F("AT+CGSN"),OK_STRING,aTimeout);
}

//! \brief Implements the AT\#CGMI command and waits for OK answer
/*! \details
The command returns device manufacturer identification code.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::request_manufacturer_identification_echo(tout_t aTimeout)
{
   return send_wait(F("AT#CGMI"),OK_STRING,aTimeout);
}

//! \brief Implements the AT\#CGMR command and waits for OK answer
/*! \details
The command returns device software revision number.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::request_revision_identification_echo(tout_t aTimeout)
{
   return send_wait(F("AT#CGMR"),OK_STRING,aTimeout);
}

//! \brief Implements the AT\#CGSN command and waits for OK answer
/*! \details
The execution command returns the product serial number, in form of IMEI of the mobile, with #CGSN:
command echo.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::request_psn_identification_echo(tout_t aTimeout)
{
   return send_wait(F("AT#CGSN"),OK_STRING,aTimeout);
}

//! \brief Implements the AT\#CGMF command and waits for OK answer
/*! \details
Execution command returns the device product code without #CGMF: command echo.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::request_product_code(tout_t aTimeout)
{
   return send_wait(F("AT#CGMF"),OK_STRING,aTimeout);
}

//! \brief Implements the AT\#SWPKGV command and waits for OK answer
/*! \details
This command allows to retrieve the software package version.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::request_software_package_version(tout_t aTimeout)
{
   return send_wait(F("AT#SWPKGV"),OK_STRING,aTimeout);
}

//! \brief Implements the AT+CPAS command and waits for OK answer
/*! \details
Execution command reports the device status in the form shown in Additional info section.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::phone_activity_status(tout_t aTimeout)
{
   return send_wait(F("AT+CPAS"),OK_STRING,aTimeout);
}

//! \brief Implements the AT+CFUN command and waits for OK answer
/*! \details
This command selects the level of functionality in the ME.
 * \param fun    Power saving function mode
 * \param rst    reset flag
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::set_phone_functionality(int fun ,int rst, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CFUN=%d,%d"), fun, rst);
   return send_wait((char*)mBuffer,OK_STRING,aTimeout);
}

//! \brief Implements the AT+CMER command and waits for OK answer
/*! \details
This command configures sending of unsolicited result codes from TA to TE.
 * \param mode    controls the processing of unsolicited result codes
 * \param keyp    keypad event reporting
 * \param disp    display event reporting
 * \param ind     indicator event reporting
 * \param bfr     TA buffer clearing
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::mobile_equipment_event_reporting(int mode, int keyp, int disp, int ind, int bfr, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CMER=%d,%d,%d,%d,%d"), mode, keyp, disp, ind, bfr);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CSVM command and waits for OK answer
/*! \details
Command to set voice mail server number.
 * \param mode    enable/disable voice mail number
 * \param number    string type phone number of format specified by <type>
 * \param type    string type phone number of format specified by <type>
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::set_voice_mail_number(int mode, const char *number, int type, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CSVM=%d,\"%s\",%d"), mode, number, type);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#MBN command and waits for OK answer
/*! \details
This command returns the mailbox numbers stored on SIM.
 * \param index    record number
 * \param number    string type mailbox number in the format <type>
 * \param type    type of mailbox number octet in integer format
 * \param text    the alphanumeric text associated to the number
 * \param mboxtype    the message waiting group type of the mailbox, if available
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::mailbox_numbers(int index, const char *number, int type, const char *text, const char *mboxtype, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#MBN=%d,\"%s\",%d,\"%s\",\"%s\""), index, number, type, text, mboxtype);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#MWI command and waits for OK answer
/*! \details
This command enables/disables the presentation of the Message Waiting Indicator (MWI) URC.
 * \param enable    enables/disables the presentation of the #MWI: URC
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::message_waiting_indication(int enable, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#MWI=%d"), enable);
   return send_wait((char*)mBuffer,OK_STRING,aTimeout);
}

//! \brief Implements the AT+CLAC command and waits for OK answer
/*! \details
This command shows the available AT commands list.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::available_at_commands(tout_t aTimeout)
{
   return send_wait(F("AT+CLAC"),OK_STRING,aTimeout);
}

//! \brief Implements the AT\#LANG command and waits for OK answer
/*! \details
Set command selects the currently used language for displaying different messages.
 * \param lan    Selected language
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::select_language(const char *lan, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#LANG=\"%s\""), lan);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CMEE command and waits for OK answer
/*! \details
The command enables the use of result code.
 * \param n    enables/disables +CME ERROR: <err> result code and selects the format
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::report_mobile_equipment_error(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CMEE=%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#CEER command and waits for OK answer
/*! \details
The command is related to extended numeric error report.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::extended_numeric_error_report(tout_t aTimeout)
{
   return send_wait(F("AT#CEER"),OK_STRING,aTimeout);
}

//! \brief Implements the AT\#PSMRI command and waits for OK answer
/*! \details
The command enables or disables the Ring Indicator pin response to an URC message while modem is in
power saving mode.
 * \param n    disables, enables/sets duration of the generated pulse
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::power_saving_mode_ring_indicator(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#PSMRI=%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CSCS command and waits for OK answer
/*! \details
The command purpose is to set different character sets that are used by the device.
 * \param chset    character set to be used by the device.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::select_te_character_set(const char* chset, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CSCS=%s"), chset);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CMUX command and waits for OK answer
/*! \details
This command is used to enable/disable the multiplexing protocol control channel.
 * \param mode    basic option is currently the only supported mode
 * \param subset    the way in which the multiplexer control channel is set up
 * \param port_speed    transmission rate
 * \param n1    maximum frame size
 * \param t1    acknowledgement timer in units of ten milliseconds
 * \param n2    maximum number of re-transmissions
 * \param t2    response timer for the multiplexer control channel
 * \param t3    wake up response timer
 * \param k    window size
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::multiplexing_mode(int mode, int subset, int port_speed, int n1, int t1, int n2, int t2, int t3, int k, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CMUX=%d,%d,%d,%d,%d,%d,%d,%d,%d"), mode, subset, port_speed, n1, t1, n2, t2, t3, k);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#USBCFG command and waits for OK answer
/*! \details
This command sets USB configuration on the modem device.
 * \param mode    USB configuration mode
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::usb_configuration(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#USBCFG=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#PORTCFG command and waits for OK answer
/*! \details
This command allows to connect Service Access Points (software anchorage points) to the external physical
ports.
 * \param variant    set port configuration
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::connect_physycal_port_sap(int variant, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#PORTCFG=%d"), variant);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#ATDELAY command and waits for OK answer
/*! \details
Set command sets a delay in second for the execution of successive AT command.
 * \param delay    delay interval in 100 ms intervals
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::at_command_delay(int delay, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#ATDELAY=%d"), delay);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT&Z command and waits for OK answer
/*! \details
The command stores a telephone number in the internal phonebook.
 * \param n    phonebook record
 * \param nr    telephone number (maximum length 24 digits)
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::store_telephone_number(int n, const char *nr, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT&Z%d=\"%s\""), n, nr);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT&V2 command and waits for OK answer
/*! \details
The command displays last connection statistics.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::display_last_connection_statistics(tout_t aTimeout)
{
   return send_wait(F("AT&V2"), OK_STRING, aTimeout);
}

//! \brief Implements the AT+IMEISV command and waits for OK answer
/*! \details
Execution command returns the International Mobile Station Equipment Identity and Software Version Number
(IMEISV) of the module without +IMEISV: command echo.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::request_imei_software_version(tout_t aTimeout)
{
   return send_wait(F("AT+IMEISV"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#CGMM command and waits for OK answer
/*! \details
This command returns the device model identification.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::request_model_identification_echo(tout_t aTimeout)
{
   return send_wait(F("AT#CGMM"), OK_STRING, aTimeout);
}

//! \brief Implements the AT&V0 command and waits for OK answer
/*! \details
The command displays current modem configuration and profile.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::display_current_configuration_profile(tout_t aTimeout)
{
   return send_wait(F("AT&V0"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#FWSWITCH command and waits for OK answer
/*! \details
Set command allows enabling a specific firmware image on products embedding 2 or more different firmware
images.
 * \param image_number    To know the <imageNumber> values range refer to the test command.
 * \param storage_conf    selects storage type
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::set_active_firmware_image(int image_number, int storage_conf, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FWSWITCH=%d,%d"), image_number, storage_conf);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#IMSPDPSET command and waits for OK answer
/*! \details
This command sets IMS Pdp APN Name. This name should be one of the APN names set in +CGDCONT
command and appropriated context will be opened for IMS.
 * \param pdpApnName    from 1 to 32 symbols ANSI fixed string.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ims_pdp_apn_number_set(const char* pdpApnName, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#IMSPDPSET=%s"), pdpApnName);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#TID command and waits for OK answer
/*! \details
The command returns Telit ID and version number.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::request_telit_id(tout_t aTimeout)
{
   return send_wait(F("AT#TID"), OK_STRING, aTimeout);
}

// S Parameters ----------------------------------------------------------------

//! \brief Implements the ATS0 command and waits for OK answer
/*! \details
The command controls the automatic answering feature of the DCE.
 * \param n    number of rings
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::number_rings_auto_answer(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("ATS0=%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the ATS1 command and waits for OK answer
/*! \details
S1 is incremented each time the device detects the ring signal of an incoming call. S1 is cleared as soon as no
ring occur.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ring_counter(tout_t aTimeout)
{
   return send_wait(F("ATS1"), OK_STRING, aTimeout);
}

//! \brief Implements the ATS2 command and waits for OK answer
/*! \details
The command manages the ASCII character used as escape character.
 * \param chr    escape character decimal ASCII
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::escape_character(int chr, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("ATS2=%d"), chr);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the ATS3 command and waits for OK answer
/*! \details
The command manages the character configured as command line terminator.
 * \param chr    command line termination character (decimal ASCII)
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::command_line_terminator_character(int chr, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("ATS3=%d"), chr);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the ATS4 command and waits for OK answer
/*! \details
The command manages the character generated by the device as part of the header, trailer, and terminator for
result codes and information text.
 * \param chr    response formatting character (decimal ASCII)
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::response_formatting_character(int chr, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("ATS4=%d"), chr);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the ATS5 command and waits for OK answer
/*! \details
The command manages the value of the character recognized by the DCE as a request to delete from the
command line the immediately preceding character.
 * \param chr    command line editing character (decimal ASCII)
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::command_line_editing_character(int chr, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("ATS5=%d"), chr);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the ATS7 command and waits for OK answer
/*! \details
This set command specifies the amount of time that the DCE shall allow between either answering a call
(automatically or by the ATA command) or completion of signaling of call addressing information to network
(dialing), and establishment of a connection with the remote DCE. If no connection is established during this
time, the DCE disconnects from the line and returns a result code indicating the cause of the disconnection.
 * \param tout    defines time interval expressed in seconds
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::connection_completed_timeout(int tout, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("ATS7=%d"), tout);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the ATS12 command and waits for OK answer
/*! \details
The command manages the prompt delay between two different escape characters.
 * \param time    delay expressed in fiftieth of a second
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::escaper_prompt_delay(int time, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("ATS12=%d"), time);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the ATS25 command and waits for OK answer
/*! \details
The command manages the amount of time that the device will ignore the DTR.
 * \param time    expressed in hundredths of a second
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::delay_dtr_off(int time, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("ATS25=%d"), time);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the ATS&V1 command and waits for OK answer
/*! \details
The command displays the S registers values.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::s_registers_display(tout_t aTimeout)
{
   return send_wait(F("ATS&V1"), OK_STRING, aTimeout);
}

//! \brief Implements the ATS10 command and waits for OK answer
/*! \details
The command is available only for backward compatibility
 * \param n    dummy
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::carrier_off_firm_time(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("ATS10=%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the ATS&V3 command and waits for OK answer
/*! \details
The command displays the extended S registers values.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::extended_s_registers_display(tout_t aTimeout)
{
   return send_wait(F("ATS&V3"), OK_STRING, aTimeout);
}

// DTE Modem Interface Control -------------------------------------------------

//! \brief Implements the ATE command and waits for OK answer
/*! \details
This command allows to enable or disable the command echo.
 * \param n    Configuration value
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::command_echo(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("ATE%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the ATQ command and waits for OK answer
/*! \details
This command allows to enable or disable the result code.
 * \param n    enables/disables result codes
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::quiet_result_codes(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("ATQ%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the ATV command and waits for OK answer
/*! \details
Set command determines the contents of the header and trailer transmitted with result codes and information
responses. It also determines if result codes are transmitted in a numeric form or an alphanumeric form.
 * \param n    format of information responses and result codes.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::response_format(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("ATV%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the ATI command and waits for OK answer
/*! \details
This command returns identification information.
 * \param n    information request
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::identification_information(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("ATI%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT&C command and waits for OK answer
/*! \details
This set command controls the DCD output behavior of the AT commands serial port.
 * \param n    DCD output behavior
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::data_carrier_detect_control(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT&C%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT&D command and waits for OK answer
/*! \details
This set command configures the behavior of the module according to the DTR control line transitions (RS232).
 * \param n    defines the module behavior according to the DTR control line transitions
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::data_terminal_ready_control(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT&D%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT&K command and waits for OK answer
/*! \details
Flow Control settings.
 * \param n    flow control behavior
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::flow_control(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT&K%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT&S command and waits for OK answer
/*! \details
Set DSR pin behavior.
 * \param n    Configuration parameter
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::data_set_ready_control(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT&S%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+IPR command and waits for OK answer
/*! \details
The command sets the speed of the USIF0 serial port, see document [1].
 * \param rate    speed of the serial USIF0 port expressed in bit per second.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::uart_dce_interface_speed(int rate, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+IPR=%d"), rate);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+IFC command and waits for OK answer
/*! \details
This set command selects the flow control of the serial port in both directions.
 * \param byDTE    specifies the method used by the DTE to control the flow of data received from the device (DCE)
 * \param byDCE    specifies the method used by the device (DCE) to control the flow of data received from the DTE
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::dte_modem_local_control_flow(int byDTE, int byDCE, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+IFC=%d,%d"), byDTE, byDCE);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+ICF command and waits for OK answer
/*! \details
This set command selects the flow control of the serial port in both directions.
 * \param format    sets the number of Data bits and Stop bits
 * \param parity    setting this sub parameter has no meaning
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::dte_modem_character_framing(int format, int parity, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+ICF=%d,%d"), format, parity);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SKIPESC command and waits for OK answer
/*! \details
This command enables/disables skipping the escape sequence.
 * \param mode    enable/disable skipping the escape sequence (+++)
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::skip_escape_sequence(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SKIPESC=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#E2ESC command and waits for OK answer
/*! \details
This set command sets a guard time in seconds for the escape sequence in PS to be considered a valid one,
and return to on-line command mode.
 * \param gt    sets a guard time in seconds
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::escape_sequence_guard_time(int gt, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#E2ESC=%d"), gt);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the ATX command and waits for OK answer
/*! \details
Set command selects the subset of result code messages the modem uses to reply to the DTE upon AT
commands execution.
 * \param n    configuration value
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::extended_result_codes(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("ATX=%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// Call Control ----------------------------------------------------------------
//! \brief Implements the ATD command and waits for OK answer
/*! \details
This command establishes a GPRS communication between the TE and the external PDN.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::dialup_connection(tout_t aTimeout)
{
   return send_wait(F("ATD"), NO_CARRIER_STRING, aTimeout);
}

//! \brief Implements the ATH command and waits for OK answer
/*! \details
This execution command hangs up/disconnects the current voice/data call or dial-up.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::hang_up(tout_t aTimeout)
{
   return send_wait(F("ATH"), OK_STRING, aTimeout);
}

//! \brief Implements the ATO command and waits for OK answer
/*! \details
This execution command is used, during a suspended data conversation, to return in on-line mode from
command mode. If there is no suspended conversation, it returns NO CARRIER
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::return_online(tout_t aTimeout)
{
   return send_wait(F("ATO"), OK_STRING, aTimeout);
}

//! \brief Implements the AT#DIALMODE command and waits for OK answer
/*! \details
This execution command sets command manages dialing modality.
 * \param mode sets dialing modality
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::set_dialing_mode(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#DIALMODE=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// Modulation and Compression Control ------------------------------------------
//! \brief Implements the AT%E command and waits for OK answer
/*! \details
This command is used for line quality monitoring and auto retrain or fall back/fall forward.
 * \param n    this parameter is not really used
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::line_quality_auto_retain(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT%%E%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// Network ---------------------------------------------------------------------

//! \brief Implements the AT+CNUM command and waits for OK answer
/*! \details
Execution command returns the MSISDN (if the phone number of the device has been stored in the SIM card)
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::subscriber_number(tout_t aTimeout)
{
   return send_wait(F("AT+CNUM"), 0, OK_STRING, aTimeout);
}

//! \brief Implements the AT+COPN command and waits for OK answer
/*! \details
This command read operator names.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::read_operator_names(tout_t aTimeout)
{
   return send_wait(F("AT+COPN"), 0, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CREG command and waits for OK answer
/*! \details
The command enables/disables the network registration unsolicited result code (URC) and selects its
presentation format.
 * \param mode    enables/disables the network registration unsolicited result code and selects one of the two formats: shot or long format
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::network_registration_status(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CREG=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+COPS command and waits for OK answer
/*! \details
The command selects a network operator and registers the module.
 * \param mode    defines the operator selection: automatic or manual
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::operator_selection(int mode,  tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+COPS=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+COPS command and waits for OK answer
/*! \details
The command selects a network operator and registers the module.
 * \param mode    defines the operator selection: automatic or manual
 * \param format    specifies the operator name format
 * \param oper    network operator in format defined by <format> parameter
 * \param act    selects access technology
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::operator_selection(int mode, int format, const char *oper, int act,  tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+COPS=%d,%d,\"%s\",%d"), mode, format, oper,act );
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+COPS command and waits for OK answer
/*! \details
The command selects a network operator and registers the module.
 * \param mode    defines the operator selection: automatic or manual
 * \param format    specifies the operator name format
 * \param oper    network operator in format defined by <format> parameter
 * \param act    selects access technology
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::operator_selection(int mode, int format, int oper, int act, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+COPS=%d,%d,%d,%d"), mode, format, oper,act );
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CLCK command and waits for OK answer
/*! \details
This command is used to lock or unlock a ME on a network facility.
 * \param fac    facility to lock, unlock or interrogate
 * \param mode    defines the operation to be done on the facility
 * \param password    shall be the same as password specified for the facility from the DTE user interface or with command Change Password +CPWD
 * \param _class    is a sum of integers, each representing an information class of which the command refers to
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::facility_lock_unlock(const char * fac, int mode, const char *password, int classType, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CLCK=\"%s\",%d,\"%s\",%d"), fac, mode, password, classType);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CLCK command and waits for OK answer
/*! \details
This command is used to lock or unlock a ME on a network facility.
 * \param fac    facility to lock, unlock or interrogate
 * \param mode    defines the operation to be done on the facility
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::facility_lock_unlock(const char * fac, int mode,tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CLCK=\"%s\",%d"), fac, mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CLCK command and waits for OK answer
/*! \details
This command is used to lock or unlock a ME on a network facility.
 * \param fac    facility to lock, unlock or interrogate
 * \param mode    defines the operation to be done on the facility
 * \param password    shall be the same as password specified for the facility from the DTE user interface or with command Change Password +CPWD
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::facility_lock_unlock(const char * fac, int mode, const char *password,tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CLCK=\"%s\",%d,\"%s\""), fac, mode, password);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CPWD command and waits for OK answer
/*! \details
This command changes the password for the facility lock function defined by command Facility Lock +CLCK
command.
 * \param fac    facility lock function
 * \param old_password    it shall be the same as password specified for the facility from the ME user interface or with command +CPWD
 * \param new_password    new password
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::change_facility_password(const char * fac, const char *old_password, const char *new_password, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CPWD=\"%s\",\"%s\",\"%s\""), fac, old_password, new_password );
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CLCC command and waits for OK answer
/*! \details
This command returns the list of current calls and their characteristics
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::list_current_calls(tout_t aTimeout)
{
   return send_wait(F("AT+CLCC"), OK_STRING, aTimeout);
}

//! \brief Implements the AT+CPOL command and waits for OK answer
/*! \details
The command is used to edit or update the UICC preferred list of networks. The list is read in the UICC file
selected by the command +CPLS
 * \param index    the order number of operator in the UICC preferred operator list
 * \param format    format for <oper> parameter
 * \param oper    Operator Identifier
 * \param gsm_act    GSM access technology
 * \param gsm_compact_cact    GSM compact access technology
 * \param utran_act    UTRAN access technology
 * \param e_utran_actn    E-UTRAN access technology
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::preferred_operator_list(int index, int format, const char *oper, int gsm_act, int gsm_compact_cact, int utran_act, int e_utran_actn, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CPOL=%d,%d,\"%s\",%d,%d,%d,%d"), index, format, oper, gsm_act, gsm_compact_cact, utran_act, e_utran_actn );
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CPOL command and waits for OK answer
/*! \details
The command is used to edit or update the UICC preferred list of networks. The list is read in the UICC file
selected by the command +CPLS
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::preferred_operator_list(tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CPOL="));
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CPLS command and waits for OK answer
/*! \details
The command is used to select a list of preferred PLMNs in the SIM/USIM card.
 * \param list    PLMNs list selector
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::selection_preferred_plmn_list(int list, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CPLS=%d"), list );
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CSQ command and waits for OK answer
/*! \details
Execution command returns received signal strength indication <rssi> and channel bit error rate <ber> from
the MT.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::signal_quality(tout_t aTimeout)
{
   return send_wait(F("AT+CSQ"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SERVINFO command and waits for OK answer
/*! \details
This command reports information about the serving cell.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::serving_cell_information(tout_t aTimeout)
{
   return send_wait(F("AT#SERVINFO"), OK_STRING, aTimeout);
}

//! \brief Implements the AT#BCCHLOCK command and waits for OK answer
/*! \details
This command enables/disable the single BCCH ARFCN locking.
 * \param lockedBcch enable/disable 2G BCCH locking
 * \param lockedUarfcn enable/disable 3G BCCH locking
 * \param lockedPsc enable/disable 3G BCCH locking Primary Scrambling Code selection
 * \param lockedEarfcn a number representing the earfcn to be locked
 * \param lockedPci E-UTRAN physical cell ID in hexadecimal format.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::lock_to_single_BCCH_ARFCN(int lockedBcch, int lockedUarfcn, int lockedPsc, int lockedEarfcn, const char *lockedPci, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#BCCHLOCK=%d,%d,%d,%d,%s"), lockedBcch, lockedUarfcn, lockedPsc, lockedEarfcn, lockedPci);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#NWEN command and waits for OK answer
/*! \details
This command enables the unsolicited result code of emergency number update
 * \param en    enables/disables unsolicited indication of emergency number update
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::network_emergency_number_update(int en, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#NWEN=%d"), en );
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#PLMNUPDATE command and waits for OK answer
/*! \details
This set command adds a new entry, or updates an already present one, in the module PLMN list stored in file
system.
 * \param action    remove/update PLMN list items
 * \param mcc    Mobile Country Code
 * \param mnc    Mobile Network Code
 * \param plmnname    name of the PLMN; string value, max length 30 characters
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::update_plmn_list(int action, int mcc, int mnc, const char* plmnname, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#PLMNUPDATE%d,%d,%d,\"%s\""), action, mcc, mnc, plmnname );
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#PLMNMODE command and waits for OK answer
/*! \details
Set command is used to select the list of operator names to be used in +COPN command, and in internal
researches for operator name matching given MCC and MNC.
 * \param mode    list of operator names to be used for internal search
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::plmn_list_selection(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#PLMNMODE=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#BND command and waits for OK answer
/*! \details
This command selects RF bands
 * \param band    GSM band selection
 * \param umts_band    this parameter is used for backward compatibility
 * \param lte_band    indicates the LTE supported bands expressed as the sum of Band number
 * \param tdscdma_band    this parameter is used for backward compatibility
 * \param lte_band_over_64    indicates the LTE high supported bands expressed as the sum of Band number
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::select_band(int band, int umts_band, int lte_band, int tdscdma_band, int lte_band_over_64, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#BND=%d,%d,%d,%d,%d"), band, umts_band, lte_band, tdscdma_band, lte_band_over_64);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#AUTOBND command and waits for OK answer
/*! \details
This command has no effect and is included only for backward compatibility.
 * \param value    only for backward compatibility
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::automatic_band_selection(int value, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#AUTOBND=%d"), value);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SNUM command and waits for OK answer
/*! \details
This set command writes the MSISDN information related to the subscriber (own number) in the EFmsisdn SIM
file.
 * \param index    the number of the record in the EFmsisdn file in SIM
 * \param number    string containing the phone number
 * \param alpha    alphanumeric string associated to <number>
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::set_subscriber_number(int index, const char *number, const char *alpha, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SNUM=%d,\"%s\",\"%s\""), index, number, alpha);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#CEERNET command and waits for OK answer
/*! \details
The command is related to extended numeric error report.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::extended_numeric_error_report_net(tout_t aTimeout)
{
   return send_wait(F("AT#CEERNET"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#CEERNETEXT command and waits for OK answer
/*! \details
This command is both a set and an execution command.
 * \param func    enable/disable the URC or delete the last network info
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::extended_error_report_network_reject_cause(int func, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#CEERNETEXT=%d"), func);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#CIPHIND command and waits for OK answer
/*! \details
This command enables/disables unsolicited result code for cipher indication.
 * \param mode    enable/disable #CIPHIND: unsolicited result code
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ciphering_indication(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#CIPHIND=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#PSNT command and waits for OK answer
/*! \details
The command enables/disables unsolicited result code for packet service network type (PSNT)
 * \param mode    enables/disables PSNT unsolicited result code
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::packet_service_network_type(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#PSNT=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#ENCALG command and waits for OK answer
/*! \details
This command enables or disables the GSM and/or GPRS encryption algorithms supported by the module.
 * \param encgsm    The <encGSM> (one byte long) is a bit mask where each bit, when set, indicates the corresponding GSM encryption algorithm
 * \param encgprs    The <encGPRS> (one byte long) is a bit mask where each bit, when set, indicates the corresponding GPRS encryption algorithm
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::set_encryption_algorithm(int encgsm, int encgprs, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#ENCALG=%d,%d"), encgsm,encgprs);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CEMODE command and waits for OK answer
/*! \details
This set command configures the mode of operation for EPS.
 * \param mode    mode of operation
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::set_mode_operation_eps(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CEMODE=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CESQ command and waits for OK answer
/*! \details
Execution command returns received signal quality parameters according to the network on which the module
is registered.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::extended_signal_quality(tout_t aTimeout)
{
   return send_wait(F("AT+CESQ"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#ENS command and waits for OK answer
/*! \details
Set command is used to activate the Enhanced Network Selection (ENS) functionality.
 * \param mode    enable/disable ENS functionality
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::enhanced_network_selection(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#ENS=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+WS46 command and waits for OK answer
/*! \details
This command selects the cellular network (Wireless Data Service, WDS).
 * \param value    WDS-Side Stack to be used by the TA.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::select_wireless_network(int value, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+WS46=%d"), value);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CEDRXS command and waits for OK answer
/*! \details
This command controls the setting of the UEs eDRX parameters.
 * \param mode    disable or enable the use of eDRX in the UE
 * \param acttype    type of access technology
 * \param req_edrx    half a byte in a 4-bit format
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::edrx_settings(int mode, int acttype, const char *req_edrx, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CEDRXS=%d,%d,\"%s\""), mode, acttype, req_edrx);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CEDRXS command and waits for OK answer
/*! \details
This command controls the setting of the UEs eDRX parameters.
 * \param mode    disable or enable the use of eDRX in the UE
 * \return return code
 */
ME310::return_t ME310::edrx_settings(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CEDRXS=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#WS46 command and waits for OK answer
/*! \details
This command selects the IoT technology.
 * \param n    select the technology to be used.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::select_iot_technology(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#WS46=%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CEDRXRDP command and waits for OK answer
/*! \details
This command returns a message related to Extended Discontinuous Reception (eDRX).
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::edrx_read_dynamic_parameters(tout_t aTimeout)
{
   return send_wait(F("AT+CEDRXRDP"), OK_STRING, aTimeout);
}

//! \brief Implements the AT+CEREG command and waits for OK answer
/*! \details
This command monitors the Evolved Packet System (EPS) network registration status in LTE.
 * \param mode    enables/disables the network registration unsolicited result code
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::eps_network_registration_status(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CEREG=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#RFSTS command and waits for OK answer
/*! \details
Command reads current network status.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::read_current_network_status(tout_t aTimeout)
{
   return send_wait(F("AT#RFSTS"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SPN command and waits for OK answer
/*! \details
This command reads SIM fields SPN.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::read_sim_field_spn(tout_t aTimeout)
{
   return send_wait(F("AT#SPN"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#CEDRXS command and waits for OK answer
/*! \details
This command controls the setting of the UEs eDRX parameters.
 * \param mode    disable or enable the use of eDRX in the UE
 * \param acttype    type of access technology
 * \param req_edrx    half a byte in a 4 bit format
 * \param reqpagetimewindow    half a byte in a 4-bit format
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::extended_edrx_settings(int mode, int acttype, const char *req_edrx, const char *reqpagetimewindow, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#CEDRXS=%d,%d,\"%s\",\"%s\""), mode, acttype, req_edrx, reqpagetimewindow);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#MONI command and waits for OK answer
/*! \details
This command is both a set and an execution command.
 * \param number    the parameter meaning depends on the network
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::cell_monitor(int number, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#MONI=%d"), number);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT#LTESFN command and waits for OK answer
/*! \details
Execution command returns system frame number and subframe number of LTE network.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::LTE_frame_information( tout_t aTimeout)
{
   return send_wait(F("AT#LTESFN=?"), OK_STRING, aTimeout);
}

//! \brief Implements the AT#SNRSET command and waits for OK answer
/*! \details
This function sets command manages EFS file SNR_level_scan_scope.
 * \param level sets the SNR level to be used in scanning networks
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::SNR_set_level(int level, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SNRSET=%d"), level);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#IOTBND command and waits for OK answer
/*! \details
Set command configures the CAT-M1 and NB-IoT bands.
 * \param m1_band_1_64    indicates the lower (1-64) CAT-M1 supported bands
 * \param m1_band_65_128    indicates the higher (65-128) CAT-M1 supported bands
 * \param nb1_band_1_64    indicates the lower (1-64) NB-IoT supported bands
 * \param nb1_band_65_128    indicates the higher (65-128) NB-IoT supported bands
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::catm1_nbiot_band_setting(int m1_band_1_64, int m1_band_65_128, int nb1_band_1_64, int nb1_band_65_128, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#IOTBND=%d,%d,%d,%d"), m1_band_1_64, m1_band_65_128, nb1_band_1_64, nb1_band_65_128);
   return send_wait((char*)mBuffer,OK_STRING,aTimeout);
}

//! \brief Implements the AT+CLIP command and waits for OK answer
/*! \details
This function refers to the supplementary service CLIP (Calling Line Identification Presentation) that enables
a called subscriber to get the calling line identity (CLI) of the calling party when receiving a mobile terminated
call. Only for firmware voice WWV.
 * \param enable   enables (1) or disables (0) the presentation of the CLI
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::calling_line_identification_presentation (int enable, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CLIP=%d"), enable);
   return send_wait((char*)mBuffer,OK_STRING,aTimeout);
}

//! \brief Implements the AT\#IOTBND command and waits for OK answer
/*! \details
Set command configures the CAT-M1 and NB-IoT bands.
 * \return return code
 */
ME310::return_t ME310::catm1_nbiot_band_setting(tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#IOTBND="));
   return send_wait((char*)mBuffer,OK_STRING,aTimeout);
}

//! \brief Implements the AT#IRATTIMER command and waits for OK answer
/*! \details
This function sets command configures the IRAT timer and search alignment timer in dedicated
EFS file configures the CAT-M1 and NB-IoT bands.
 * \param m1_band_1_64    indicates the lower (1-64) CAT-M1 supported bands
 * \param m1_band_65_128    indicates the higher (65-128) CAT-M1 supported bands
 * \param nb1_band_1_64    indicates the lower (1-64) NB-IoT supported bands
 * \param nb1_band_65_128    indicates the higher (65-128) NB-IoT supported bands
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::inter_RAT_timer_setting(int irat_timer, int search_alignment, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#IRATTIMER=%d,%d"), irat_timer,search_alignment);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT#NB2ENA command and waits for OK answer
/*! \details
This function enables/disables NB2 mode.
 * \param ena    Enable/Disable NB2 mode
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::enable_NB2_mode(int ena, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#NB2ENA=%d"), ena);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// SIM -------------------------------------------------------------------------
//! \brief Implements the AT+CPIN command and waits for OK answer
/*! \details
The command sends to the device a password which is necessary before it can be operated.
 * \param pin    PIN required
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::enter_pin(const char *pin, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CPIN=%s"), pin);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CPIN command and waits for OK answer
/*! \details
The command sends to the device a password which is necessary before it can be operated.
 * \param pin    old PIN
 * \param newpin    new PIN
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::enter_pin(const char *pin,const char *newpin, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CPIN=%s,%s"), pin, newpin);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#PCT command and waits for OK answer
/*! \details
This command reports the PIN/PUK or PIN2/PUK2 input remaining attempts, if +CPIN password is required.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::display_pin_counter(tout_t aTimeout)
{
   return send_wait(F("AT#PCT"), OK_STRING, aTimeout);
}

//! \brief Implements the AT+CCID command and waits for OK answer
/*! \details
Execution command reads on SIM the Integrated Circuit Card Identification (ICCID). It is the card identification
number that provides a unique identification number for the SIM.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::read_iccid(tout_t aTimeout)
{
   return send_wait(F("AT+CCID"), OK_STRING, aTimeout);
}

//! \brief Implements the AT+CIMI command and waits for OK answer
/*! \details
This command returns the International Mobile Subscriber Identity (IMSI number).
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::imsi(tout_t aTimeout)
{
   return send_wait(F("AT+CIMI"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#CIMI command and waits for OK answer
/*! \details
This command returns the International Mobile Subscriber Identity (IMSI number).
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::imsi_echo(tout_t aTimeout)
{
   return send_wait(F("AT#CIMI"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SIMDET command and waits for OK answer
/*! \details
The command manages the SIM Detection mode.
 * \param mode    the <mode> parameter can be used as shown in Values section
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::sim_detection_mode(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SIMDET=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SIMPR command and waits for OK answer
/*! \details
The command enables/disables the SIM Presence Status unsolicited indication.
 * \param mode    Specifies if notification must be enabled or disabled
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::sim_presence_status(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SIMPR=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SIMPR command and waits for OK answer
/*! \details
Execution command reads on SIM the Integrated Circuit Card Identification (ICCID).
It is the card identification number that provides a unique identification number for the SIM.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::read_iccid_1(tout_t aTimeout)
{
   return send_wait(F("AT#CCID"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#QSS command and waits for OK answer
/*! \details
Query SIM Status.
 * \param mode    Type of notification
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::query_sim_status(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#QSS=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CRSM command and waits for OK answer
/*! \details
The command transmits to the UICC some specific commands and their required parameters.
 * \param command    command passed on to the UICC
 * \param field    dentifier of an elementary data file on UICC
 * \param p1    parameter passed on to the UICC
 * \param p2    parameter passed on to the UICC
 * \param p3    parameter passed on to the UICC;
 * \param data    nformation to be read from UICC or written to the UICC (hexadecimal character format)
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::restricted_sim_access(int command, int field, int p1, int p2, int p3, const char *data, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CRSM=%d,%d,%d,%d,%d,\"%s\""), command, field, p1, p2, p3, data);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CSIM command and waits for OK answer
/*! \details
This command sends a generic command to the UICC.
 * \param length    characters number of the <command> sent to UICC
 * \param command    command passed on by the MT to the UICC
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::generic_sim_access(int length, const char *command, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CSIM=%d,%s"), length, command);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CCHO command and waits for OK answer
/*! \details
Open Logical Channel.
 * \param dfname    all selectable applications in the UICC are referenced by a DF name coded on 1 to 16 bytes
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::open_logical_channel(const char *dfname, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CCHO=\"%s\""), dfname);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CCHC command and waits for OK answer
/*! \details
Close a communication session.
 * \param sessionid    a session Id to be used to target a specific application on the smart card
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::close_logical_channel(int sessionid, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CCHC=%d"), sessionid);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CGLA command and waits for OK answer
/*! \details
This command is used to control the currently selected UICC on the TE.
 * \param sessionid    the identifier of the session to be used in order to send the APDU commands to the UICC
 * \param length    characters number of the <command> sent to UICC
 * \param command    command passed on by the MT to the UICC
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::generic_uicc_logical_channell_access(int sessionid, int length, const char *command, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CGLA=%d,%d,\"%s\""), sessionid,length, command);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+ICCID command and waits for OK answer
/*! \details
Execution command reads on SIM the Integrated Circuit Card Identification (ICCID).
It is the card identification number that provides a unique identification number for the SIM.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::read_iccid_2(tout_t aTimeout)
{
   return send_wait(F("AT+ICCID"), OK_STRING, aTimeout);
}

//! \brief Implements the "AT#FWAUTOSIM command and waits for OK answer
/*! \details
This command allows to set automatically the active firmware image in accordance with the inserted SIM.
 * \param mode disable/enable automatic setting of active firmware image switch by SIM
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::automatic_switch_firmware(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FWAUTOSIM=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SIMINCFG command and waits for OK answer
/*! \details
This command configures a GPIO pin as SIMIN pin.
 * \param gpiopin    number of the GPIO pin used as SIMIN pin
 * \param simindetmode    number of the GPIO pin used as SIMIN pin
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::simin_pin_configuration(int gpiopin, int simindetmode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SIMINCFG=%d,%d"), gpiopin, simindetmode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// SIM Toolkit -----------------------------------------------------------------
//! \brief Implements the AT\#STIA command and waits for OK answer
/*! \details
The SIM/USIM Application Toolkit (SAT/USAT) provides an interface to the applications existing in the
SIM/USIM device. The module must support the mechanisms required by the SIM/USIM applications.
 * \param mode    enables/disables SAT/USAT
 * \param timeout    timeout
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::simtoolkit_interface_action(int mode, int timeout, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#STIA=%d,%d"), mode, timeout);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#STGI command and waits for OK answer
/*! \details
The #STGI command interfaces to the SIM/USIM Application Toolkit to get information on the ongoing
proactive command.
 * \param cmdType    proactive command code
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::simtoolkit_get_information(int cmdType, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#STGI=%d"), cmdType);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#STSR command and waits for OK answer
/*! \details
This command allows the user to provide a response to confirm the execution of the ongoing proactive
command.
 * \param cmdType    proactive command code
 * \param userAction    identify the user action
 * \param data    data entered by user
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::simtoolkit_send_response(int cmdType, int userAction, const char *data, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#STSR=%d,%d,\"%s\""), cmdType, userAction, data);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// SMS & CB --------------------------------------------------------------------

//! \brief Implements the AT+CSMS command and waits for OK answer
/*! \details
Set command selects messaging service <service>
 * \param service    Select Message Service
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::select_messaging_service(int service, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CSMS=%d"), service);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CPMS command and waits for OK answer
/*! \details
The command selects the memory storage used by SMs (Short Messages).
 * \param memr    memory from which messages are read and deleted
 * \param memw    memory to which writing and sending operations are made
 * \param mems    memory to which received SMs are preferred to be stored
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::preferred_message_storage(const char *memr, const char *memw, const char *mems, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CPMS=\"%s\",\"%s\",\"%s\""), memr, memw, mems);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CMGF command and waits for OK answer
/*! \details
Selects the format of SMS messages to be used in following SMS commands.
 * \param mode    format to use for SMS operations
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::message_format(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CMGF=%d"), mode );
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CSCA command and waits for OK answer
/*! \details
This command allows to set the Service Center Address for SMS transmissions.
 * \param number    String type phone number of forwarding address in format specified by <type> parameter
 * \param type    The type of number
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::service_center_address(const char *number, int type, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CSCA=\"%s\",%d"), number, type);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CSMP command and waits for OK answer
/*! \details
This command is used to select values for additional parameters for storing and sending SMS when the text
mode is used (AT+CMGF=1).
 * \param fo    depending on the command or result code
 * \param vp    depending on SMS-SUBMIT <fo> setting
 * \param pid    3GPP TS 03.40/23.040 TP-Protocol-Identifier in integer format
 * \param dcs    depending on the command or result code
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::set_text_mode_parameters(int fo, const char *vp, int pid, int dcs, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CSMP=%d,\"%s\",%d,%d"), fo, vp, pid, dcs);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CSMP command and waits for OK answer
/*! \details
This command is used to select values for additional parameters for storing and sending SMS when the text
mode is used (AT+CMGF=1).
 * \param fo    depending on the command or result code
 * \param vp    depending on SMS-SUBMIT <fo> setting
 * \param pid    3GPP TS 03.40/23.040 TP-Protocol-Identifier in integer format
 * \param dcs    depending on the command or result code
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::set_text_mode_parameters(int fo, int vp, int pid, int dcs, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CSMP=%d,%d,%d,%d"), fo, vp, pid, dcs);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CSDH command and waits for OK answer
/*! \details
This command controls whether detailed header information is shown in text mode.
 * \param show    control the display of the result codes
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::show_text_mode_parameters(int show, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CSDH=%d"), show);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CSAS command and waits for OK answer
/*! \details
Execution command saves settings which have been made by the +CSCA, and +CSMP commands in local
non-volatile memory.
 * \param profile    Index of the profile where the settings are saved
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::save_settings(int profile, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CSAS=%d"), profile);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CRES command and waits for OK answer
/*! \details
Execution command restores message service settings saved by +CSAS command from either NVM or SIM.
 * \param profile    Defines which message service profiles to restore
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::restore_settings(int profile, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CRES=%d"), profile);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CMMS command and waits for OK answer
/*! \details
Set command controls the continuity of SMS relay protocol link. When feature is enabled (and supported by
network) multiple messages can be sent much faster as link is kept open.
 * \param n    enables/disables the relay protocol link continuity
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::more_message_send(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CMMS=%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CNMI command and waits for OK answer
/*! \details
This command sets the parameters for receiving SMS messages.
 * \param mode    unsolicited result codes buffering option
 * \param mt    result code indication reporting for SMS-DELIVER
 * \param bm    broadcast reporting option
 * \param ds    SMS-STATUS-REPORTs reporting option
 * \param bfr    buffered result codes handling method
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::new_message_indications_TE(int mode, int mt, int bm, int ds, int bfr, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CNMI=%d,%d,%d,%d,%d"), mode, mt, bm, ds, bfr);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CNMA command and waits for OK answer
/*! \details
This command is used to confirm the correct reception of a new message.
 * \param n    type of acknowledgement in PDU mode
 * \param length    length of the PDU message
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::new_message_ack(int n, int length, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CNMA=%d,%d"), n, length);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CNMA command and waits for OK answer
/*! \details
This command is used to confirm the correct reception of a new message.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::new_message_ack(tout_t aTimeout)
{
   return send_wait(F("AT+CNMA"), OK_STRING, aTimeout);
}

//! \brief Implements the AT+CMGL command and waits for OK answer
/*! \details
This command is used to list the messages.
 * \param stat    <stat> parameter type and the command output depend on the last settings of the +CMGF command
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::list_messages(int stat, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CMGL=%d"), stat);
   return send_wait((char*)mBuffer, 0, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CMGL command and waits for OK answer
/*! \details
This command is used to list the messages.
 * \param stat    <stat> parameter type and the command output depend on the last settings of the +CMGF command
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::list_messages(const char *stat, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CMGL=\"%s\""), stat);
   return send_wait((char*)mBuffer, 0, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CMGL command and waits for OK answer
/*! \details
This command is used to list the messages.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::list_messages(tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CMGL"));
   return send_wait((char*)mBuffer,0, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CMGR command and waits for OK answer
/*! \details
This command is used to read a message.
 * \param index    message index
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::read_message(int index, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CMGR=%d"), index);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CMGS command and waits for OK answer
/*! \details
The command is related to sending short messages.
 * \param length    length in bytes of the PDU to be sent
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::send_short_message(int length, char *data, tout_t aTimeout)
{
   ME310::return_t ret;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CMGS=%d"), length);
   ret =  send_wait((char*)mBuffer, WAIT_DATA_STRING, aTimeout);
   if ((ret == RETURN_VALID))
   {
      memset(mBuffer, 0, ME310_BUFFSIZE);
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, "%s",data);
      ret =  send_wait((char*)mBuffer, OK_STRING, CTRZ, aTimeout);
   }
   return ret;
}

//! \brief Implements the AT+CMGS command and waits for OK answer
/*! \details
The command is related to sending short messages.
 * \param da    destination address
 * \param toda    type of destination address
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::send_short_message(const char *da, const char *toda, char *data, tout_t aTimeout)
{
   ME310::return_t ret;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CMGS=\"%s\",%s"), da, toda);
   ret =  send_wait((char*)mBuffer, WAIT_DATA_STRING, aTimeout);
   if ((ret == RETURN_VALID))
   {
      memset(mBuffer, 0, ME310_BUFFSIZE);
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, "%s", data);
      ret =  send_wait((char*)mBuffer, OK_STRING, CTRZ, aTimeout);
   }
   return ret;
}

//! \brief Implements the AT+CMSS command and waits for OK answer
/*! \details
This command sends to the network a message which is already stored in the <memw> storage.
 * \param index    location value in the message storage <memw> of the message to send
 * \param da    destination address
 * \param toda    type of destination address
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::send_message_from_storage(const char *index, const char *da, const char *toda, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CMSS=\"%s\",\"%s\",\"%s\""), index, da, toda);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CMGW command and waits for OK answer
/*! \details
The command is related to writing short messages.
 * \param length    length in bytes of the PDU to be written
 * \param stat    message status
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::write_short_message_memory(int length, int stat, char *data,  tout_t aTimeout)
{
   ME310::return_t ret;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CMGW=%d,%d"), length, stat);
   ret =  send_wait((char*)mBuffer, WAIT_DATA_STRING, aTimeout);
   if ((ret == RETURN_VALID))
   {
      memset(mBuffer, 0, ME310_BUFFSIZE);
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, data);
      ret =  send_wait((char*)mBuffer, OK_STRING, CTRZ, aTimeout);
   }
   return ret;
}

//! \brief Implements the AT+CMGW command and waits for OK answer
/*! \details
The command is related to writing short messages.
 * \param da    destination address
 * \param toda    type of destination address
 * \param stat    message status
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::write_short_message_memory(const char *da, int toda, const char *stat, char *data, tout_t aTimeout)
{
   ME310::return_t ret;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CMGW=\"%s\",%d,\"%s\""), da, toda, stat);
   ret =  send_wait((char*)mBuffer, WAIT_DATA_STRING, aTimeout);
   if ((ret == RETURN_VALID))
   {
      memset(mBuffer, 0, ME310_BUFFSIZE);
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, data);
      ret =  send_wait((char*)mBuffer, OK_STRING, CTRZ, aTimeout);
   }
   return ret;
}

//! \brief Implements the AT\#CMGD command and waits for OK answer
/*! \details
This command allows to delete from memory messages.
 * \param index    Message index in the selected storage
 * \param delflag    Type of multiple message deletion
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::delete_message(int index, int delflag, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CMGD=%d,%d"), index, delflag);
   return send_wait((char*)mBuffer,OK_STRING,aTimeout);
}

//! \brief Implements the AT+CGSMS command and waits for OK answer
/*! \details
Set command is used to specify the service or service preference that the MT will use to send MO SMS
messages.
 * \param service    indicates the service or service preference to be used
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::select_service_mo_sms(int service, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CGSMS=%d"), service);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SMSMODE command and waits for OK answer
/*! \details
SMS Commands Operation Mode.
 * \param mode    Enables/disables the check for presence of SCA in FDN phonebook
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::sms_commands_operation_mode(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SMSMODE=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#CMGLCONCINDEX command and waits for OK answer
/*! \details
The command reports list of all concatenated SMS
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::report_concatenated_sms_indexes(tout_t aTimeout)
{
   return send_wait(F("AT#CMGLCONCINDEX"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#E2SMSRI command and waits for OK answer
/*! \details
This set command enables/disables the Ring Indicator pin response to an incoming SMS message. If enabled,
a negative going pulse is generated when receiving an incoming SMS message.
 * \param n    RI enabling
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::sms_ring_indicator(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#E2SMSRI=%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SMOV command and waits for OK answer
/*! \details
The command is used to enable the SMS overflow signaling functionality.
 * \param mode    signaling functionality mode
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::sms_overflow(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SMOV=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SMSMOVE command and waits for OK answer
/*! \details
This command moves selected Short Message from current memory to destination memory.
 * \param index    message index in the memory selected by +CPMS command
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::sms_move(int index, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SMSMOVE=%d"), index);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// Phonebook -------------------------------------------------------------------

//! \brief Implements the AT+CPBS command and waits for OK answer
/*! \details
This set command selects phonebook memory storage, which will be used by other phonebook commands.
 * \param storage    specify the phonebook memory storage
 * \param password    string type value representing the PIN2-code required when selecting PIN2-code locked <storage> above "FD"
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::phonebook_select_memory_storage(const char *storage, const char *password, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CPBS=\"%s\",\"%s\""), storage, password);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CPBS command and waits for OK answer
/*! \details
This set command selects phonebook memory storage, which will be used by other phonebook commands.
 * \param storage    specify the phonebook memory storage
 * \param password    string type value representing the PIN2-code required when selecting PIN2-code locked <storage> above "FD"
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::phonebook_select_memory_storage(const char *storage, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CPBS=\"%s\""), storage);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}
//! \brief Implements the AT+CPBR command and waits for OK answer
/*! \details
The command reads phonebook entries.
 * \param index1    value in the range of location numbers of the currently selected phonebook memory storage
 * \param index2    value in the range of location numbers of the currently selected phonebook memory storage
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::phonebook_read_entries( int index1, int index2, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CPBR=%d,%d"), index1, index2);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CPBR command and waits for OK answer
/*! \details
The command reads phonebook entries.
 * \param index1    value in the range of location numbers of the currently selected phonebook memory storage
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::phonebook_read_entries( int index1, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CPBR=%d"), index1);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CPBF command and waits for OK answer
/*! \details
This command returns phonebook entries.
 * \param findtext    string to be searched among the phonebook entries
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::phonebook_find_entries(const char *findtext, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CPBF=\"%s\""), findtext);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CPBW command and waits for OK answer
/*! \details
This command writes phonebook entry in the current phonebook memory.
 * \param index    value in the range of location numbers of the currently selected phonebook memory storage
 * \param number    phone number in the format <type>
 * \param type    type of number
 * \param text    text associated to the number
 * \param group    string type field of maximum length <glength> indicating a group the entry may belong to
 * \param adnumber    additional number; string type phone number of format <adtype>
 * \param adtype    type of address octet
 * \param secondtext    string type field of maximum length <slength> indicating a second text field associated with the number
 * \param email    field of maximum length <elength> indicating an email address
 * \param hidden    indicates if the entry is hidden or not
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::phonebook_write_entry(int index, const char *number, int type, const char *text, const char *group, const char * adnumber, int adtype, const char *secondtext, const char * email, int hidden, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CPBW=%d,\"%s\",%d,\"%s\",\"%s\",\"%s\",%d,\"%s\",\"%s\",%d"), index, number, type, text, group, adnumber, adtype, secondtext, email, hidden);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CPBW command and waits for OK answer
/*! \details
This command writes phonebook entry in the current phonebook memory.
 * \param index    value in the range of location numbers of the currently selected phonebook memory storage
 * \param number    phone number in the format <type>
 * \param type    type of number
 * \param text    text associated to the number
 * \param group    string type field of maximum length <glength> indicating a group the entry may belong to
 * \param adnumber    additional number; string type phone number of format <adtype>
 * \param adtype    type of address octet
 * \param secondtext    string type field of maximum length <slength> indicating a second text field associated with the number
 * \param email    field of maximum length <elength> indicating an email address
 * \param hidden    indicates if the entry is hidden or not
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::phonebook_write_entry(int index, const char *number, int type, const char *text, const char *group, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CPBW=%d,\"%s\",%d,\"%s\",\"%s\""), index, number, type, text, group);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CPBGR command and waits for OK answer
/*! \details
This command returns Grouping information Alpha String (GAS) USIM file entries.
 * \param indexFirst    first location to be read
 * \param indexLast    last location to be read
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::phonebook_read_group_entries( int indexFirst, int indexLast, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#CPBGR=%d,%d"), indexFirst, indexLast);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CPBGR command and waits for OK answer
/*! \details
This command returns Grouping information Alpha String (GAS) USIM file entries.
 * \param index    location to be read
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::phonebook_read_group_entries( int index, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#CPBGR=%d"), index);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CPBGW command and waits for OK answer
/*! \details
Set command writes the name of a phonebook group <text> in the Grouping information Alpha String (GAS)
USIM file in a specified location number <index>.
 * \param index    number of the record in the GAS file to be written
 * \param text    text to be stored in the record
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::phonebook_write_group_entry(int index, const char *text, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#CPBGW=%d,\"%s\""), index, text);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#CPBD command and waits for OK answer
/*! \details
This command deletes all phonebook entries.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::phonebook_delete_all_entries(tout_t aTimeout)
{
   return send_wait(F("AT#CPBD"), OK_STRING, aTimeout);
}

// Time & Alarm ----------------------------------------------------------------

//! \brief Implements the AT+CCLK command and waits for OK answer
/*! \details
Set command sets the real-time clock of the module.
 * \param time    Current time as quoted string in the format: "yy/MM/dd,hh:mm:ss±zz,d"
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::clock_management(const char *time, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CCLK=\"%s\""), time);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CALA command and waits for OK answer
/*! \details
Set command stores in the internal Real Time Clock of the module an alarm time with respective
settings.
 * \param time    current alarm time as quoted string in the format: "yy/MM/dd,hh:mm:ss±zz"
 * \param index    index of the alarm
 * \param type    alarm behavior type
 * \param string    alarm code text string used in the URC +CALA
 * \param recurr    sets a recurrent alarm for one or more days in the week
 * \param silent    indicates if the alarm is silent or not
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::alarm_management(const char *time, int index, int type, const char *string, const char *recurr, int silent, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CALA=\"%s\",%d,%d,\"%s\",\"%s\",%d"), time, index, type, string, recurr, silent);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CAPD command and waits for OK answer
/*! \details
Set command postpones or dismisses a currently active alarm.
 * \param sec    time in seconds to postpone the alarm
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::alarm_postpone(int sec, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CAPD=%d"), sec);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CSDF command and waits for OK answer
/*! \details
This command sets the date format of the date information presented to the user.
 * \param mode    phone display data format
 * \param auxmode    TE-TA interface data format
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::setting_date_format(int mode, int auxmode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CSDF=%d,%d"), mode, auxmode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CTZR command and waits for OK answer
/*! \details
This command enables and disables the time zone change event reporting.
 * \param onoff    enable/disable the time zone change event reporting
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::time_zone_reporting(int onoff, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CTZR=%d"), onoff);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CTZU command and waits for OK answer
/*! \details
Set command enables/disables the automatic time zone update via NITZ.
 * \param onoff    enables/disables the automatic time zone update via NITZ
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::automatic_time_zone_update(int onoff, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CTZU=%d"), onoff);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#NITZ command and waits for OK answer
/*! \details
This command handles Network Identity and Time Zone.
 * \param val    identifies the functionalities to enable
 * \param mode    enables/disables the #NITZ URC
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::network_identity_time_zone(int val, int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#NITZ=%d,%d"), val, mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#CCLK command and waits for OK answer
/*! \details
The command is related to real time clock management.
 * \param time    Current time as quoted string in the format: "yy/MM/dd,hh:mm:ss±zz,d"
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::clock_management_ext(const char *time, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#CCLK=\"%s\""), time);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#CCLKMODE command and waits for OK answer
/*! \details
This command allows to enable the local time or the UTC time.
 * \param mode    Time and date mode
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::clock_mode(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#CCLKMODE=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#WAKE command and waits for OK answer
/*! \details
Stop any alarm activity
 * \param opmode    operating mode
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::wake_alarm_mode(int opmode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#WAKE=%d"), opmode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CSTF command and waits for OK answer
/*! \details
Set command sets the time format of the time information presented to the user, which is specified by use of
the <mode> parameter.
 * \param mode    affects the time format on the phone display
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::setting_time_format(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CSTF=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CALD command and waits for OK answer
/*! \details
This command deletes an alarm in the ME.
 * \param n    alarm index
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::alarm_delete(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CALD=%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// Audio ----------------------------------------------------------------------

//! \brief Implements the AT+CMUT command and waits for OK answer
/*! \details
This command enables/disables the muting of the uplink audio line during a voice call.
 * \param n   controls the muting of the uplink audio line
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::microphone_mute_control(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CMUT=%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT#OAP command and waits for OK answer
/*! \details
This command enables/disables the muting of the uplink audio line during a voice call.
 * \param mode    enables/disables the Open Audio Path
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::open_audio_loop(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#OAP=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// Digital Voice Interface -----------------------------------------------------
//! \brief Implements the AT#DVI  command and waits for OK answer
/*! \details
This command enables/disables the muting of the uplink audio line during a voice call.
 * \param mode    enables/disables the DVI
 * \param dviport select DVI port
 * \param clockmode reserved for backward compatibility. Default value 1.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::digital_voiceband_interface(int mode, int dviport, int clockmode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#DV=%d,%d,%d"), mode, dviport, clockmode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// HW and Radio Control --------------------------------------------------------
//! \brief Implements the AT\#CBC command and waits for OK answer
/*! \details
This command returns the current Battery and Charger state.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::battery_charger_status(tout_t aTimeout)
{
   return send_wait(F("AT#CBC"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#GPIO command and waits for OK answer
/*! \details
Set the value of the general-purpose input/output GPIO pins.
 * \param pin    GPIO pin number
 * \param mode    sets GPIO pin configuration
 * \param dir    sets the GPIO pin in input, output, or alternate functions
 * \param save    GPIO pin save configuration
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::gpio_control(int pin, int mode, int dir, int save, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#GPIO=%d,%d,%d,%d"), pin, mode, dir, save);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#ALARMPIN command and waits for OK answer
/*! \details
This command allows to configure the ALARM Pin.
 * \param pin    GPIO pin number
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::alarm_pin_configuration(int pin, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#ALARMPIN=%d"), pin);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SLED command and waits for OK answer
/*! \details
The command configures the behavior of the STAT_LED status.
 * \param mode    defines the STAT_LED pin behavior
 * \param onDuration    duration of period in which STAT_LED pin is tied high
 * \param offDuration    duration of period in which STAT_LED pin is tied low
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::stat_led_gpio_setting(int mode,int onDuration, int offDuration, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SLED=%d,%d,%d"), mode,onDuration, offDuration);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SLEDSAV command and waits for OK answer
/*! \details
This command allows to save the current STAT_LED GPIO setting.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::stat_led_gpio_setting_save(tout_t aTimeout)
{
   return send_wait(F("AT#SLEDSAV"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#ADC command and waits for OK answer
/*! \details
This command returns the current voltage value of the specified ADC inputs, expressed in mV.
 * \param adc    index of input pin
 * \param mode    required action
 * \param dir    direction
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::adc_read(int adc,int mode, int dir, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#ADC=%d,%d,%d"), adc, mode, dir);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#V24CFG command and waits for OK answer
/*! \details
This command sets the AT commands serial port interface output pins mode.
 * \param pin    AT commands serial port interface hardware pin
 * \param mode    AT commands serial port interface hardware pins mode
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::v24_output_pin_configuration(int pin,int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#V24CFG=%d,%d"), pin, mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#V24 command and waits for OK answer
/*! \details
This command sets the state of the output pins of the AT commands serial port interface.
 * \param pin    AT commands serial port interface hardware pin
 * \param state    State of AT commands serial port interface output hardware pins
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::v24_output_pin_control(int pin, int state, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#V24=%d,%d"), pin, state);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#I2CWR command and waits for OK answer
/*! \details
This command is used to send data to an I2C peripheral connected to module.
 * \param sdaPin    GPIO number for SDA
 * \param sclPin    GPIO number for SCL
 * \param deviceId    address of the I2C device (7 bits)
 * \param registerId    register to write data to
 * \param len    number of data to send
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::i2c_write(int sdaPin,int sclPin, int deviceId, int registerId, int len, char* data, tout_t aTimeout)
{
   ME310::return_t ret;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#I2CWR=%d,%d,%x,%x,%d"), sdaPin, sclPin, deviceId, registerId, len);
   ret =  send_wait((char*)mBuffer, WAIT_DATA_STRING, aTimeout);
   if ((ret == RETURN_VALID))
   {
      char dataF[ME310_BUFFSIZE-1] = {};
      strcat(dataF, data);
      strcat(dataF, "0x1A");
      memset(mBuffer, 0, ME310_BUFFSIZE);
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, data);
      ret =  send_wait((char*)mBuffer, OK_STRING, aTimeout);
   }
   return ret;
}

//! \brief Implements the AT\#I2CRD command and waits for OK answer
/*! \details
This command is used to read data from an I2C peripheral connected to module.
 * \param sdaPin    GPIO number for SDA
 * \param sclPin    GPIO number for SCL
 * \param deviceId    address of the I2C device (7 bits)
 * \param registerId    Register to read data from
 * \param len    Number of data to receive
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::i2c_read(int sdaPin,int sclPin, int deviceId, int registerId, int len, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#I2CRD=%d,%d,%x,%x,%d"), sdaPin, sclPin, deviceId, registerId, len);
   return send_wait((char*)mBuffer,0, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#I2CCF command and waits for OK answer
/*! \details
This command is used to write and read data to/from an I2C device using the I2C Combined Format. The
module acts as an I2C master.
 * \param sdaPin    GPIO number for SDA
 * \param sclPin    GPIO number for SCL
 * \param deviceId    address of the I2C device (7 bits)
 * \param lenwr    number of data to write
 * \param lenrd    number of data to read
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::i2c_write_read(int sdaPin,int sclPin, int deviceId, int lenwr, int lenrd, char* data, tout_t aTimeout)
{
   ME310::return_t ret;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#I2CCF=%d,%d,%x,%d,%d"), sdaPin, sclPin, deviceId, lenwr, lenrd);
   if(lenwr > 0)
   {
      ret =  send_wait((char*)mBuffer, WAIT_DATA_STRING, aTimeout);
      if ((ret == RETURN_VALID))
      {
         char dataF[ME310_BUFFSIZE-1] = {};
         strcat(dataF, data);
         strcat(dataF, "0x1A");
         memset(mBuffer, 0, ME310_BUFFSIZE);
         snprintf((char *)mBuffer, ME310_BUFFSIZE-1, data);
         ret =  send_wait((char*)mBuffer, OK_STRING, aTimeout);
      }
      return ret;
   }
   else
   {
      return send_wait((char*)mBuffer, 0, OK_STRING, aTimeout);
   }
}

//! \brief Implements the AT\#TESTMODE command and waits for OK answer
/*! \details
Set module in Test Mode for configuring and testing the POWER level (not signaling mode).
 * \param cmd    dentifies one of the commands
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::test_mode_configuration(const char *cmd, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#TESTMODE=\"%s\""), cmd);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}


// Power Down ------------------------------------------------------------------
//! \brief Implements the AT\#REBOOT command and waits for OK answer
/*! \details
Immediate module reboot.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::module_reboot(tout_t aTimeout)
{
   return send_wait(F("AT#REBOOT"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#ENHRST command and waits for OK answer
/*! \details
Enable or Disable the one shot or periodic unit reset
 * \param mode    Enable\Disable mode
 * \param delay    time interval in minutes after that the unit reboots
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::periodic_reset(int mode, int delay, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   if(mode == 0)
   {
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#ENHRST=%d"), mode);
      return send_wait((char*)mBuffer, OK_STRING, aTimeout);
   }
   else
   {
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#ENHRST=%d,%d"), mode,delay);
      return send_wait((char*)mBuffer, OK_STRING, aTimeout);
   }
}

//! \brief Implements the AT\#SHDN command and waits for OK answer
/*! \details
This command turns the module OFF.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::software_shutdown(tout_t aTimeout)
{
   return send_wait(F("AT#SHDN"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SYSHALT command and waits for OK answer
/*! \details
The execution command sets the module in SYSHALT state. To power down the module, the serial port
(ASC0) must have the control signals CTS, DTR, DCD and RING low.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::system_turnoff(tout_t aTimeout)
{
   return send_wait(F("AT#SYSHALT"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#FASTSHDN command and waits for OK answer
/*! \details
This command can be used as a set command to configure a GPIO pin performing a fast shutdown when on it
is forced a High to Low level transition. Or can be used as an execute command to force immediately a fast
shutdown.
 * \param enable    enable/disable the fast shutdown execution via a GPIO pin
 * \param gpio    selects the GPIO to execute the fast shutdown
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::fast_shutdown_configuration(int enable, int gpio, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FASTSHDN=%d,%d"), enable,gpio);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#FASTSHDN command and waits for OK answer
/*! \details
This command can be used as a set command to configure a GPIO pin performing a fast shutdown when on it
is forced a High to Low level transition. Or can be used as an execute command to force immediately a fast
shutdown.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::fast_shutdown_configuration(tout_t aTimeout)
{
   return send_wait(F("AT#FASTSHDN"), OK_STRING, aTimeout);
}

// Easy Scan -------------------------------------------------------------------

//! \brief Implements the AT\#CSURV command and waits for OK answer
/*! \details
The command performs a survey on the selected band channels.
 * \param s    starting channel
 * \param e    ending channel
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::network_survey(int s, int e, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#CSURV=%d,%d"),s ,e);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#CSURV command and waits for OK answer
/*! \details
The command performs a survey on the selected band channels.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::network_survey(tout_t aTimeout)
{
   return send_wait(F("AT#CSURV"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#CSURVC command and waits for OK answer
/*! \details
This command performs a survey on the selected band channels. The return message uses only the numeric
format.
 * \param s    starting channel
 * \param e    ending channel
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::nertwork_survey_numeric_format(int s, int e, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#CSURVC=%d,%d"), s,e);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#CSURVC command and waits for OK answer
/*! \details
This command performs a survey on the selected band channels. The return message uses only the numeric
format.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::nertwork_survey_numeric_format(tout_t aTimeout)
{
   return send_wait(F("AT#CSURVC"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#CSURVF command and waits for OK answer
/*! \details
The command configures the numbers format used in the messages related to the surveying of the network
bands channels.
 * \param format    format of the numbers in each network survey information text line
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::network_survey_format(int format, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#CSURVF=%d"), format);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#CSURVNLF command and waits for OK answer
/*! \details
This command enables/disables the automatic <CR><LF> removing from each network survey information text
line.
 * \param value    enables/disables the automatic <CR><LF> removing from each network survey information text line
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::network_survey_crlf(int value, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#CSURVNLF=%d"), value);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#CSURVEXT command and waits for OK answer
/*! \details
The command is present only for backward compatibility.
 * \param value    dummy parameter
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::network_survey_extended(int value, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#CSURVEXT=%d"), value);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// Jamming Detection and Report ------------------------------------------------

//! \brief Implements the AT\#JDRENH2 command and waits for OK answer
/*! \details
This command enables/disable jamming detection, and reports the relative result to the user.
 * \param mode    enable/disable jamming detection and select reporting mode
 * \param sat2G    is the starting absolute threshold of RSSI 2G Network
 * \param carrNum2G    is the minimum number of possible jammed carriers to consider that the module is under jamming condition
 * \param pRxLevT2G    set the threshold of RxLev in 2G Network
 * \param p_rssi_t4g    Set the threshold of RSSI
 * \param p_rsrq_t4g    Set the threshold of RSRQ
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::jamming_detect_report(int mode, int sat2G, int carrNum2G, int pRxLevT2G, int p_rssi_t4g, int p_rsrq_t4g, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#JDRENH2=%d,%d,0,%d,%d,0,0,0,%d,%d,0"), mode, sat2G, carrNum2G,pRxLevT2G, p_rssi_t4g, p_rsrq_t4g);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#JDR4GCFG command and waits for OK answer
/*! \details
The command configures the LTE Jamming Detection thresholds.
 * \param p_rsrp_t4g    set the threshold of RSRP
 * \param p_rsrq_t4g    set the threshold of RSRQ
 * \param initial_delay    Sets the delay in tens of ms from power ON
 * \param sampling_number    Number of samples required to validate the Jamming condition
 * \param p_rssi_s4g    RSSI threshold level in dBm, above which the Jamming condition is detected
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::jamming_detect_threshold(int p_rsrp_t4g, int p_rsrq_t4g, int initial_delay, int sampling_number,int p_rssi_s4g, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#JDR4GCFG=%d,%d,%d,%d,%d"), p_rsrp_t4g, p_rsrq_t4g, initial_delay, sampling_number,p_rssi_s4g);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// Packet Domain ---------------------------------------------------------------

//! \brief Implements the AT+CGDCONT command and waits for OK answer
/*! \details
Define PDP Context.
 * \param cid    (PDP Context Identifier) numeric parameter which specifies a particular PDP context definition
 * \param pdp_type    (Packet Data Protocol type) a string parameter which specifies the type of packet data protocol
 * \param apn    (Access Point Name) a string parameter which is a logical name that is used to select the GGSN or the external packet data network
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::define_pdp_context(int cid, const char *pdp_type, const char * apn, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CGDCONT=%d,\"%s\",\"%s\""), cid,pdp_type,apn);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CGPADDR command and waits for OK answer
/*! \details
This command returns a list of PDP addresses for the specified context identifiers.
 * \param cid    specifies a PDP context definition
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::show_pdp_address(int cid, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CGPADDR=%d"), cid);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CGPADDR command and waits for OK answer
/*! \details
This command returns a list of PDP addresses for the specified context identifiers.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::show_pdp_address(tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CGPADDR="));
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#AUTOATT command and waits for OK answer
/*! \details
Execution command has no effect and is included only for backward compatibility.
 * \param _auto    dummy parameter
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::auto_attach_property(int _auto, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#AUTOATT=%d"), _auto);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#MSCLASS command and waits for OK answer
/*! \details
Set command sets the GPRS multislot class.
 * \param _class    GPRS multislot class
 * \param autoattach    specify when the new multislot class will be enabled
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::multislot_class_control(int _class, int autoattach, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#MSCLASS=%d,%d"), _class, autoattach);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#GAUTH command and waits for OK answer
/*! \details
This command sets the authentication type used in PDP Context Activation during PPP-PS connections.
 * \param type    authentication type used in PDP Context Activation during PPP-PS connections
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ppp_data_connection_auth_type(int type, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#GAUTH=%d"), type);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#CGAUTH command and waits for OK answer
/*! \details
This command allows the TE to specify authentication parameters for a PDP context.
 * \param cid    specifies a particular PDP context definition
 * \param auth_type    selects the authentication protocol used for this PDP context
 * \param username    User name for access to the IP network
 * \param password    Password for access to the IP network
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::define_pdp_context_auth_params(int cid, int auth_type, const char *username, const char *password, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CGAUTH=%d,%d,\"%s\",\"%s\""), cid, auth_type, username, password);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CGCONTRDP command and waits for OK answer
/*! \details
The execution command returns the relevant information for a PDP Context established by the network.
 * \param value    parameter
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::pdp_context_read_dynamic_parameters( tout_t aTimeout)
{
   return send_wait(F("AT+CGCONTRDP"), OK_STRING, aTimeout);
}

//! \brief Implements the AT+CGCONTRDP command and waits for OK answer
/*! \details
The execution command returns the relevant information for a PDP Context established by the network.
 * \param cid    identifies a non secondary PDP context definition
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::pdp_context_read_dynamic_parameters(int cid, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CGCONTRDP=%d"), cid);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CGPIAF command and waits for OK answer
/*! \details
This command selects the printout format of the IPv6 address.
 * \param ipv6_addressFormat    selects the IPv6 address format
 * \param ipv6_subnetNotation    selects the subnet-notation for remote address and subnet mask
 * \param ipv6_leadingZeros    selects whether leading zeros are omitted or not
 * \param ipv6_compressZeros    selects whether 1-n instances of 16-bit- zero values are replaced by only "::"
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::printing_ip_address_format(int ipv6_addressFormat, int ipv6_subnetNotation, int ipv6_leadingZeros, int ipv6_compressZeros, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CGPIAF=%d,%d,%d,%d"), ipv6_addressFormat, ipv6_subnetNotation, ipv6_leadingZeros, ipv6_compressZeros);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CGACT command and waits for OK answer
/*! \details
This command activates or deactivates the specified PDP context(s).
 * \param state    activate/deactivate the PDP context
 * \param cid    specifies a PDP context definition
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::pdp_context_activate(int cid, int state,  tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CGACT=%d,%d"), state, cid);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CGACT command and waits for OK answer
/*! \details
This command activates or deactivates the specified PDP context(s).
 * \param state    activate/deactivate the PDP context
 * \param cid    specifies a PDP context definition
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::pdp_context_activate(tout_t aTimeout)
{
   ME310::return_t ret;
   int state = 1;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   for(int i = 1; i < 4; i++)
   {
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CGACT=%d,%d"), state, i);
      ret = send_wait((char*)mBuffer, OK_STRING, aTimeout);
   }
   return ret;
}

//! \brief Implements the AT+CGEREP command and waits for OK answer
/*! \details
This command enables or disables the presentation of unsolicited result codes.
 * \param mode    controls the processing of URCs specified with this command
 * \param bfr    controls the effect on buffered codes when <mode> 1 or 2 is entered
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::packet_domain_event_reporting(int mode, int bfr, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CGEREP=%d,%d"), mode,bfr);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#PPPCFG command and waits for OK answer
/*! \details
This command configures the PPP mode.
 * \param mode    sets PPP mode
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ppp_configuration(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#PPPCFG=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CGREG command and waits for OK answer
/*! \details
Set command controls the presentation of the +CGREG: unsolicited result code
 * \param mode    enables/disables the network registration unsolicited result code (URC), and selects one of the available formats
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::gprs_network_registration_status(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CGREG=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CGATT command and waits for OK answer
/*! \details
This execution command is used to register (attach) the terminal to or deregister (detach) the terminal from the
Packet Domain service.
 * \param state    state of PS attachment
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ps_attach_detach(int state, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CGATT=%d"), state);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CSODCP command and waits for OK answer
/*! \details
The set command is used by the TE to transmit data over control plane to network via MT. Context identifier
<cid> is used to link the data to particular context.
 * \param cid    specifies a particular PDP context or EPS bearer context definition
 * \param cpdataLength    indicates the number of octets of the <cpdata> information element
 * \param cpdata    a string of octets
 * \param rai    indicates the value of the release assistance indication
 * \param typeOfUserData    indicates whether the user data that is transmitted is regular or exceptional
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::sending_originating_data(int cid, int cpdataLength, const char *cpdata, int rai, int typeOfUserData, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CSODCP=%d,%d,\"%s\",%d,%d"), cid, cpdataLength, cpdata, rai, typeOfUserData);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+CRTDCP command and waits for OK answer
/*! \details
The set command is used to enable and disable reporting of data from the network to the MT that is
transmitted via the control plane in downlink direction.
 * \param reporting    controlling reporting of mobile terminated control plane data events
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::reporting_terminating_data(int reporting, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CRTDCP=%d"), reporting);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// IPEasy ----------------------------------------------------------------------

//! \brief Implements the AT\#SGACT command and waits for OK answer
/*! \details
This command enables/disables the PDP context activation.
 * \param cid    specifies a particular PDP context definition
 * \param stat    activates/disactivates the PDP context specified
 * \param userid    user identifier, used only if the context requires it
 * \param pwd    password, used only if the context requires it
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::context_activation(int cid, int stat, const char *userid, const char *pwd, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SGACT=%d,%d,\"%s\",\"%s\""), cid, stat, userid, pwd);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SGACT command and waits for OK answer
/*! \details
This command enables/disables the PDP context activation.
 * \param cid    specifies a particular PDP context definition
 * \param stat    activates/disactivates the PDP context specified
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::context_activation(int cid, int stat, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SGACT=%d,%d"), cid, stat);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SGACTAUTH command and waits for OK answer
/*! \details
This command sets the authentication type for IP Easy
 * \param type    authentication type for IP Easy
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::pdp_context_auth_type(int type, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SGACTAUTH=%d"), type);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SGACTCFG command and waits for OK answer
/*! \details
This command configures the automatic activation/reactivation of the specified PDP context
 * \param cid    PDP context identifier
 * \param retry    specifies the maximum number of context activation attempts in case of activation failure
 * \param delay    specifies the delay in seconds between an attempt and the next one
 * \param urcmode    URC presentation mode
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::pdp_automatic_context_activation(int cid, int retry, int delay, int urcmode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SGACTCFG=%d,%d,%d,%d"), cid, retry, delay, urcmode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SGACTCFGEXT command and waits for OK answer
/*! \details
This command manages the extended configuration of context activation.
 * \param cid    PDP context identifier
 * \param abortAttemptEnable    enables/disables abort during context activation attempt
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::extended_pdp_context_activation(int cid, int abortAttemptEnable, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SGACTCFGEXT=%d,%d"), cid, abortAttemptEnable);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SCFG command and waits for OK answer
/*! \details
The command sets the configuration for the socket.
 * \param connId    Socket connection identifier.
 * \param cid    PDP context identifier
 * \param pktSz    Packet size in bytes to be used by the TCP/UDP/IP stack for data sending
 * \param maxTo    Exchange timeout in seconds (or socket inactivity timeout)
 * \param connTo    Connection timeout in tenths of seconds
 * \param txTo    data sending timeout; data are sent even if they are less than max packet size, after this period
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_configuration(int connId, int cid, int pktSz, int maxTo, int connTo, int txTo, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SCFG=%d,%d,%d,%d,%d,%d"), connId, cid, pktSz, maxTo, connTo, txTo);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SCFGEXT command and waits for OK answer
/*! \details
This command sets the socket configuration extended parameters.
 * \param connId    socket connection identifier
 * \param srMode    SRING unsolicited mode, see Additional info section
 * \param recvDataMode    Data view mode for received data in command mode
 * \param keepalive    Set the TCP Keepalive value in minutes
 * \param listenAutoRsp    set the listen auto-response mode, that affects the commands #SL and #SLUDP
 * \param sendDataMode    Data mode for sending data in command mode (#SSEND)
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_configuration_extended(int connId, int srMode, int recvDataMode, int keepalive, int listenAutoRsp, int sendDataMode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SCFGEXT=%d,%d,%d,%d,%d,%d"), connId, srMode, recvDataMode, keepalive, listenAutoRsp, sendDataMode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SCFGEXT2 command and waits for OK answer
/*! \details
Socket Configuration Extended.
 * \param connId    socket connection identifier
 * \param bufferStart    select one of the two data sending timeout methods, the first one defined "old" the second one "new"
 * \param abortConnAttempt    enable the abort of an ongoing connection attempt started by #SD command and before the reception of the CONNECT message (in online mode) or OK message (in command mode).
 * \param unusedB    reserved for future use
 * \param unusedC    reserved for future use
 * \param noCarrierMode    select the NO CARRIER message format received when the socket is closed
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_configuration_extended_2(int connId, int bufferStart, int abortConnAttempt, int unusedB, int unusedC, int noCarrierMode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SCFGEXT2=%d,%d,%d,%d,%d,%d"), connId, bufferStart, abortConnAttempt, unusedB, unusedC, noCarrierMode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SKTRST command and waits for OK answer
/*! \details
Socket Parameters Reset
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_parameters_reset(tout_t aTimeout)
{
   return send_wait(F("AT#SKTRST"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SD command and waits for CONNECT answer
/*! \details
Execution command opens a remote connection via socket.
 * \param connId    Socket connection identifier.
 * \param txProt    Transmission protocol.
 * \param rPort    Remote host port to contact.
 * \param IPaddr    IP address of the remote host
 * \param closureType    Socket closure behavior only for TCP when remote host has closed
 * \param lPort    UDP connections local port
 * \param connMode    Connection mode
 * \param txTime    adjusting a time interval for series of UDP data packets will be uploaded
 * \param userIpType    ip type for socket to open
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_dial(int connId, int txProt, int rPort, const char *IPaddr, int closureType, int lPort, int connMode, int txTime, int userIpType, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SD=%d,%d,%d,\"%s\",%d,%d,%d,%d,%d"), connId, txProt, rPort, IPaddr, closureType, lPort, connMode, txTime, userIpType);
   if(connMode == 1)
   {
      return send_wait((char*)mBuffer, OK_STRING, aTimeout);
   }
   else
   {
      return send_wait((char*)mBuffer, CONNECT_STRING, aTimeout);
   }
}

//! \brief Implements the AT\#SD command and waits for CONNECT answer
/*! \details
Execution command opens a remote connection via socket.
 * \param connId    Socket connection identifier.
 * \param txProt    Transmission protocol.
 * \param rPort    Remote host port to contact.
 * \param IPaddr    IP address of the remote host
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_dial(int connId, int txProt, int rPort, const char *IPaddr, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SD=%d,%d,%d,\"%s\""), connId, txProt, rPort, IPaddr);
   return send_wait((char*)mBuffer, CONNECT_STRING, aTimeout);
}

//! \brief Implements the AT\#SO command and waits for OK answer
/*! \details
Execution command resumes the direct interface to a socket connection which has been suspended by the
escape sequence.
 * \param connId    socket connection identifier
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_restore(int connId, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SO=%d"), connId);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SH command and waits for OK answer
/*! \details
The set command closes a socket.
 * \param connId    socket connection identifier to be closed
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_shutdown(int connId, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SH=%d"), connId);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SL command and waits for OK answer
/*! \details
The command opens/closes socket listening.
 * \param connId    socket connection identifier
 * \param listenState    listening action
 * \param listenPort    local listening port
 * \param lingerT    linger time
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_listen(int connId, int listenState, int listenPort, int lingerT, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SL=%d,%d,%d,%d"), connId, listenState, listenPort, lingerT);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SLUDP command and waits for OK answer
/*! \details
This command opens/closes a socket listening for an incoming UDP connection on a specified port.
 * \param connId    socket connection identifier
 * \param listenState    indicates the action that will be performed
 * \param listenPort    local listening port
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_listen_UDP(int connId, int listenState, int listenPort, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SLUDP=%d,%d,%d"), connId, listenState, listenPort);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SA command and waits for OK answer
/*! \details
Execution command accepts an incoming socket connection.
 * \param connId    Socket connection identifier
 * \param connMode    Connection mode, as for command #SD
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_accept(int connId, int connMode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SA=%d,%d"), connId, connMode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SSEND command and waits for OK answer
/*! \details
This command is used to send data through a connected socket.
 * \param connId    Selection on which Socket connection identifier send data
 * \param rai    RAI (Release Assistance Indication) configuration
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_send_data_command_mode(int connId, char* data, int rai, tout_t aTimeout)
{
   int i = 0;
   ME310::return_t ret;
   string tmp_str;
   char* tmp_data;
   /*IS IRA*/
   read_socket_configuration_extended();
   /*Control if size is > 1500 if is not IRA  >3000 if is IRA*/
   char* tmp_buf = (char*) buffer_cstr(i);
   while(tmp_buf != NULL)
   {
      tmp_str = tmp_buf;
      string searchedString = "#SCFGEXT: ";
      searchedString.append(std::to_string(connId));
      size_t foundString = tmp_str.find(searchedString);
      if(foundString != string::npos)
      {
         break;
      }
      i++;
      tmp_buf = (char*)buffer_cstr(i);
   }
   CheckIRAOption((char*)tmp_str.c_str());
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SSEND=%d,%d"), connId, rai);
   ret =  send_wait((char*)mBuffer, WAIT_DATA_STRING, aTimeout);
   if ((ret == RETURN_VALID))
   {
      memset(mBuffer, 0, ME310_BUFFSIZE);
      if(!_isIRATx)
      {
         if(strlen(data)> ME310_SEND_BUFFSIZE)
         {
            ret = RETURN_ERROR;
            return ret;
         }
      }
      snprintf((char *)mBuffer, ME310_SEND_BUFFSIZE - 1, "%s", data);
      ret =  send_wait((char*)mBuffer, OK_STRING, CTRZ, aTimeout);
   }
   return ret;
}

//! \brief Implements the AT\#SSENDEXT command and waits for OK answer
/*! \details
This command allows to send data through a connected socket including all possible octets (from 0x00 to
0xFF).
 * \param connId    Selection on which Socket connection identifier send data
 * \param bytesToSend    number of bytes to be sent
 * \param rai    RAI (Release Assistance Indication) configuration
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_send_data_command_mode_extended(int connId, int bytesToSend, char* data, int rai, tout_t aTimeout)
{
   ME310::return_t ret;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SSENDEXT=%d,%d,%d"), connId, bytesToSend, rai);
   ret =  send_wait((char*)mBuffer, WAIT_DATA_STRING, aTimeout);
   if ((ret == RETURN_VALID))
   {
      memset(mBuffer, 0, ME310_BUFFSIZE);
      memcpy(mBuffer, data, bytesToSend);
      ret =  send_wait((char*)mBuffer, OK_STRING, aTimeout);
   }
   return ret;
}

//! \brief Implements the AT\#SRECV command and waits for OK answer
/*! \details
The command permits the user to read data arrived through a connected socket when the module is in
command mode.
 * \param connId    socket connection identifier
 * \param maxByte    max number of bytes to read
 * \param udpInfo    enables/disables the visualization of UDP datagram information
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_receive_data_command_mode(int connId, int maxByte, int udpInfo,  tout_t aTimeout)
{
   int i = 0;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   string tmp_str;
   if(udpInfo == 1)
   {
      SET_BIT_MASK(_option,_UDP_INFO_BIT);
   }
   else
   {
      UNSET_BIT_MASK(_option,_UDP_INFO_BIT);
   }
   read_socket_configuration_extended();
   /*Control if size is > 1500 if is not IRA  >3000 if is IRA*/
   char* tmp_buf = (char*) buffer_cstr(i);
   while(tmp_buf != NULL)
   {
      tmp_str = tmp_buf;
      string searchedString = "#SCFGEXT: ";
      searchedString.append(std::to_string(connId));
      size_t foundString = tmp_str.find(searchedString);
      if(foundString != string::npos)
      {
         break;
      }
      i++;
      tmp_buf = (char*) buffer_cstr(i);;
   }
   CheckIRAOption((char*)tmp_str.c_str());
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SRECV=%d,%d,%d"), connId, maxByte, udpInfo);
   return send_wait((char*)mBuffer, 0, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SSENDUDP command and waits for OK answer
/*! \details
This command allows to send data over UDP to a specific remote host.
 * \param connId    socket connection identifier
 * \param remoteIP    IP address of the remote host in dotted decimal notation
 * \param remotePort    remote host port
 * \param rai    RAI (Release Assistance Indication) configuration
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_send_udp_data_specific_remote_host(int connId, const char *remoteIP, int remotePort, int rai, char* data, tout_t aTimeout)
{
   ME310::return_t ret;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SSENDUDP=%d,\"%s\"%d,%d"), connId, remoteIP, remotePort, rai);
   ret =  send_wait((char*)mBuffer, WAIT_DATA_STRING, aTimeout);
   if ((ret == RETURN_VALID))
   {
      memset(mBuffer, 0, ME310_BUFFSIZE);
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, data);
      ret =  send_wait((char*)mBuffer, OK_STRING, TERMINATION_STRING, aTimeout);
   }
   return ret;
}

//! \brief Implements the AT\#SSENDUDPEXT command and waits for OK answer
/*! \details
This command permits, while the module is in command mode, to send data over UDP to a specific remote
host including all possible octets (from 0x00 to 0xFF)
 * \param connId    socket connection identifier
 * \param bytes_to_send    bytes to be sent
 * \param remoteIP    IP address of the remote host in dotted decimal notation, string type: "xxx.xxx.xxx.xxx"
 * \param remotePort    remote host port
 * \param rai    RAI (Release Assistance Indication) configuration
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_send_udp_data_specific_remote_host_extended(int connId, int bytes_to_send, const char *remoteIP, int remotePort, int rai, char* data, tout_t aTimeout)
{
   ME310::return_t ret;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SSENDUDPEXT=%d,%d,\"%s\"%d,%d"), connId, bytes_to_send, remoteIP, remotePort, rai);
   ret =  send_wait((char*)mBuffer, WAIT_DATA_STRING, aTimeout);
   if ((ret == RETURN_VALID))
   {
      memset(mBuffer, 0, ME310_BUFFSIZE);
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, data);
      ret =  send_wait((char*)mBuffer, OK_STRING, TERMINATION_STRING, aTimeout);
   }
   return ret;
}

//! \brief Implements the AT\#SLASTCLOSURE command and waits for OK answer
/*! \details
The command detects the cause of a socket disconnection.
 * \param connId    socket connection identifier
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_detect_cause_disconnection(int connId, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SLASTCLOSURE=%d"), connId);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SS command and waits for OK answer
/*! \details
Execution command reports the current sockets status.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_status(tout_t aTimeout)
{
   return send_wait(F("AT#SS"),OK_STRING,aTimeout);
}

//! \brief Implements the AT\#SS command and waits for OK answer
/*! \details
Execution command reports the current sockets status.
 * \param connId    socket connection identifier
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_status(int connId, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SS=%d"), connId);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
   //return send_wait((char*)mBuffer, 0, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SI command and waits for OK answer
/*! \details
This command is used to get socket information.
 * \param connId    socket connection identifier
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_info(int connId, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SI=%d"), connId);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SI command and waits for OK answer
/*! \details
This command is used to get socket information.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_info(tout_t aTimeout)
{
   return send_wait(F("AT#SI"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#ST command and waits for OK answer
/*! \details
Socket Type
 * \param connId    socket connection identifier
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_type(int connId, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#ST=%d"), connId);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#ST command and waits for OK answer
/*! \details
Socket Type
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_type(tout_t aTimeout)
{
   return send_wait(F("AT#ST"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#PADCMD command and waits for OK answer
/*! \details
This command sets features of the pending data flush to socket, opened with #SD command.
 * \param mode    enable/disable forwarding
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::pad_command_features(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#PADCMD=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#PADFWD command and waits for OK answer
/*! \details
PAD forward character
 * \param _char    specifies the ascii code of the char used to flush data
 * \param mode    flush mode
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::pad_forward_character(int _char, int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#PADFWD=%d,%d"), _char, mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#BASE64 command and waits for OK answer
/*! \details
This command is used to enable or disable base64 encoding and decoding data of a socket.
 * \param connId    socket connection identifier
 * \param enc    selects the encoding standard
 * \param dec    selects the decoding standard
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::base64_encode(int connId, int enc, int dec, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#BASE64=%d,%d,%d"), connId, enc, dec);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#FRWL command and waits for OK answer
/*! \details
Set command controls the internal firewall settings
 * \param action    command action
 * \param ip_addr    remote address to be added into the ACCEPT chain
 * \param net_mask    mask to be applied on the <ip_addr>
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::firewall_setup(int action, const char *ip_addr,  const char *net_mask, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FRWL=%d,\"%s\",\"%s\""), action, ip_addr, net_mask);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#E2SLRI command and waits for OK answer
/*! \details
Set command enables/disables the Ring Indicator pin response to a Socket Listen connect and, if
enabled, the duration of the negative going pulse generated on receipt of connect.
 * \param n    RI enabling
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_listen_ring_indicator(int n, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#E2SLRI=%d"), n);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#ICMP command and waits for OK answer
/*! \details
Set command enables/disables the ICMP Ping support.
 * \param mode    ICMP mode selection
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ping_support(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#ICMP=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#PING command and waits for OK answer
/*! \details
This command is used to send Ping Echo Request.
 * \param ipaddr    address of the remote host, string type
 * \param retryNum    the number of Ping Echo Request to send
 * \param len    the length of Ping Echo Request message
 * \param timeout    the timeout, in 100 ms units, waiting a single Echo Reply
 * \param ttl    time to live
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ping(const char *ipaddr, int retryNum, int len, int timeout, int ttl, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#PING=\"%s\",%d,%d,%d,%d"), ipaddr, retryNum, len, timeout, ttl);
   return send_wait((char*)mBuffer,0, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#PING command and waits for OK answer
/*! \details
This command is used to send Ping Echo Request.
 * \param ipaddr    address of the remote host, string type
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ping(const char *ipaddr, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#PING=\"%s\""), ipaddr);
   return send_wait((char*)mBuffer,0, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#QDNS command and waits for OK answer
/*! \details
Execution command executes a DNS query to solve the host name into an IP address. If the DNS
query is successful, then the IP address will be reported in the result code
 * \param host_name    Host name string
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::dns_query(const char *host_name, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#QDNS=\"%s\""), host_name);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#NWDNS command and waits for OK answer
/*! \details
The command allows to get the primary and secondary DNS addresses for selected GSM or PDP context
identifiers
 * \param cid    Generic context identifier
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::dns_from_network(int cid, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#NWDNS=%d"), cid);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#NWDNS command and waits for OK answer
/*! \details
The command allows to get the primary and secondary DNS addresses for selected GSM or PDP context
identifiers
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::dns_from_network(tout_t aTimeout)
{
   return send_wait(F("AT#NWDNS="), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#NTP command and waits for OK answer
/*! \details
The command handles the date and time update using NTP protocol.
 * \param ntpaddress    address of the NTP server.
 * \param ntpport    NTP server port to contact
 * \param updModClock    update mode
 * \param timeout    waiting timeout for server response in seconds
 * \param timezone    Time Zone: indicates the difference, expressed in quarter of an hour, between the local time and GMT.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ntp(const char *ntpaddress, int ntpport, int updModClock, int timeout, int timezone, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#NTP=\"%s\",%d,%d,%d,%d"), ntpaddress, ntpport, updModClock, timeout, timezone);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#NTPCFG command and waits for OK answer
/*! \details
This set command allows to configure additional parameters to be used for NTP operations.
 * \param cid    context id used to perform NTP operations
 * \param authType    authentication type to be used with NTP server
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::configure_ntp_parameters(int cid, int authType, int keyID, char* keysFilePath, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   if(authType == 0)
   {
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#NTPCFG=%d,%d"), cid, authType);
   }
   else
   {
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#NTPCFG=%d,%d,%d,%s"), cid, authType, keyID, keysFilePath);
   }
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SCFGEXT3 command and waits for OK answer
/*! \details
This command sets the socket configuration extended parameters for features not included in #SCFGEXT
command nor in #SCFGEXT2 command.
 * \param connId    socket connection identifier
 * \param immRsp    enables #SD command mode immediate response
 * \param closureType    backward compatibility
 * \param fastSRing    backward compatibility
 * \param ssendTimeout    timeout for #SSEND
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::socket_configuration_extended3(int connId, int immRsp, int closureType, int fastSRing, int ssendTimeout, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SCFGEXT3=%d,%d,%d,%d,%d"), connId, immRsp, closureType, fastSRing, ssendTimeout);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// FTPEasy ---------------------------------------------------------------------

//! \brief Implements the AT\#FTPAPP command and waits for OK answer
/*! \details
This command is used to append data to an already existing file via FTP during an FTP session.
 * \param fileName    the file name
 * \param connMode    the connection mode
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_append(const char *fileName, char* data, int connMode, tout_t aTimeout)
{
   ME310::return_t ret;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FTPAPP=\"%s\",%d"), fileName,connMode);
   if(connMode == 1)
   {
      ret = send_wait((char*)mBuffer, OK_STRING, aTimeout);
      if ((ret == RETURN_VALID))
      {
         memset(mBuffer, 0, ME310_BUFFSIZE);
         snprintf((char *)mBuffer, ME310_BUFFSIZE-1, data);
         ret =  send_wait((char*)mBuffer, OK_STRING, aTimeout);
      }
   }
   else
   {
      ret = send_wait((char*)mBuffer, CONNECT_STRING, aTimeout);
      if ((ret == RETURN_VALID))
      {
         memset(mBuffer, 0, ME310_BUFFSIZE);
         snprintf((char *)mBuffer, ME310_BUFFSIZE-1, data);
         ret =  send_wait((char*)mBuffer, OK_STRING, aTimeout);
      }
   }
   return ret;
}

//! \brief Implements the AT\#FTPAPPEXT command and waits for OK answer
/*! \details
The command sends data on a FTP data port while the module is in command mode.
 * \param bytesToSend    number of bytes to be sent
 * \param eof    data port closure
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_append_extended(int bytesToSend, char* data, int eof, tout_t aTimeout)
{
   ME310::return_t ret;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FTPAPPEXT=%d,%d"), bytesToSend,eof);
   ret =  send_wait((char*)mBuffer, WAIT_DATA_STRING, aTimeout);
   if ((ret == RETURN_VALID))
   {
      memset(mBuffer, 0, ME310_BUFFSIZE);
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, data);
      ret =  send_wait((char*)mBuffer, OK_STRING, aTimeout);
   }
   return ret;
}

//! \brief Implements the AT\#FTPCLOSE command and waits for OK answer
/*! \details
The command purpose is to close the previously open FTP connection.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_close(tout_t aTimeout)
{
   return send_wait(F("AT#FTPCLOSE"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#FTPCWD command and waits for OK answer
/*! \details
Command to change the working directory on FTP server.
 * \param dirname    Name of the new working directory
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_change_working_directory(const char *dirname, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FTPCWD=\"%s\""), dirname);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#FTPCWD command and waits for OK answer
/*! \details
Command to change the working directory on FTP server.
 * \param dirname    Name of the new working directory
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_change_working_directory(tout_t aTimeout)
{
   return send_wait(F("AT#FTPCWD="), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#FTPDELE command and waits for OK answer
/*! \details
This command, issued during a FTP connection, allows to delete a file from the remote working directory.
 * \param filename    Name of the file that must be deleted
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_delete(const char *filename, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FTPDELE=\"%s\""), filename);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#FTPSIZE command and waits for OK answer
/*! \details
This command returns the size of a file located on a FTP server.
 * \param filename    the name of the file that you want to know the size
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_get_file_size(const char *filename, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FTPFSIZE=\"%s\""), filename);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#FTPGET command and waits for OK answer
/*! \details
This command executes the FTP Get function during an FTP connection.
 * \param filename    file name to get from server
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_get(const char *filename, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FTPGET=\"%s\""), filename);
   return send_wait((char*)mBuffer, CONNECT_STRING, aTimeout);
}

//! \brief Implements the AT\#FTPGETPKT command and waits for OK answer
/*! \details
FTP gets in command mode.
 * \param filename    file name. Maximum length: 200 characters
 * \param viewMode    choose the view mode
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_get_command_mode(const char *filename, int viewMode,tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FTPGETPKT=\"%s\",%d"), filename, viewMode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#FTPLIST command and waits for OK answer
/*! \details
This command is used during a FTP connection.
 * \param name    is the name of the directory or file
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_list(const char *name, tout_t aTimeout)
{

   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FTPLIST=\"%s\""), name);
   return send_wait((char*)mBuffer, 0, NO_CARRIER_STRING, aTimeout);
}

//! \brief Implements the AT\#FTPLIST command and waits for OK answer
/*! \details
This command is used during a FTP connection.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_list(tout_t aTimeout)
{
   return ftp_list(".", aTimeout);
}

//! \brief Implements the AT\#FTPMSG command and waits for OK answer
/*! \details
This command returns the last response received from the FTP server.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_read_message(tout_t aTimeout)
{
   return send_wait(F("AT#FTPMSG"),0, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#FTPOPEN command and waits for OK answer
/*! \details
This execution command opens an FTP connection toward the FTP server.
 * \param server_port    address and port of FTP server
 * \param username    authentication user identification for FTP
 * \param password    authentication password for FTP
 * \param viewMode    active or passive mode
 * \param cid    PDP context identifier
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_open(const char *server_port, const char *username, const char *password, int viewMode, int cid, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FTPOPEN=\"%s\",\"%s\",\"%s\",%d,%d"), server_port, username, password, viewMode, cid);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#FTPPUT command and waits for OK answer
/*! \details
This command sends a file to the FTP server.
 * \param filename    name of the file
 * \param connMode    select online or command mode
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_put(const char *filename, int connMode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FTPPUT=\"%s\",%d"), filename, connMode);
   if(connMode == 1)
   {
      return send_wait((char*)mBuffer, OK_STRING, aTimeout);
   }
   else
   {
      return send_wait((char*)mBuffer, CONNECT_STRING, aTimeout);
   }
}

//! \brief Implements the AT\#FTPPWD command and waits for OK answer
/*! \details
This command, issued during an FTP connection, shows the current working directory on FTP server.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_print_working_directory(tout_t aTimeout)
{
   return send_wait(F("AT#FTPPWD"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#FTPRECV command and waits for OK answer
/*! \details
The command permits the user to read a given amount of data already transferred via FTP from a remote file.
 * \param block_size    maximum number of bytes to read
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_receive_data_command_mode(int block_size, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FTPRECV=%d"), block_size);
   return send_wait((char*)mBuffer, 0, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#FTPREST command and waits for OK answer
/*! \details
Set command sets the restart position for successive #FTPGET (or #FTPGETPKT) command. It permits to
restart a previously interrupted FTP download from the selected position in byte.
 * \param restartPosition    position in byte of restarting for successive #FTPGET (or #FTPGETPKT)
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_restart_posizion_get(int restartPosition, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FTPREST=%d"), restartPosition);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#FTPTO command and waits for OK answer
/*! \details
Set the FTP time out.
 * \param tout    time out in 100 milliseconds units
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_time_out(int tout, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FTPTO=%d"), tout);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#FTPTYPE command and waits for OK answer
/*! \details
This command sets the FTP file transfer type.
 * \param type    file transfer type
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_type(int type, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FTPTYPE=%d"), type);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#FTPCFG command and waits for OK answer
/*! \details
This command sets the time-out used when opening either the FTP control channel or the FTP traffic channel.
 * \param tout    time out in 100 milliseconds units
 * \param ipPIgnoring    enable or disable IP private ignoring
 * \param ftpSen    disable FTPS security
 * \param ftpext    PORT/PASV and EPRT/EPSV commands
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ftp_configuration(int tout , int ipPIgnoring, int ftpSen, int ftpext, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FTPCFG=%d,%d,%d,%d"), tout, ipPIgnoring, ftpSen, ftpext);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

/*
ME310::return_t ME310::cache_dns(int mode, ME310::tout_t aTimeout)
{
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#CACHEDNS=%d"), mode);
   return send_wait((char*)mBuffer,OK_STRING,aTimeout);
}

ME310::return_t ME310::manual_dns(int cid, const char *primary, const char *secondary, ME310::tout_t aTimeout)
{
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#DNS=%d,\"%s\",\"%s\""), cid, primary, secondary);
   return send_wait((char*)mBuffer,OK_STRING,aTimeout);
}
*/
// SMTP ------------------------------------------------------------------------

//! \brief Implements the AT\#ESMTP command and waits for OK answer
/*! \details
This command allows to set the SMTP server address for e-mail sending.
 * \param smtp    SMTP server address
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::smtp_mail_server(const char *smtp, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#ESMTP=\"%s\""), smtp);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#EMAILMSG command and waits for OK answer
/*! \details
The command returns the last response from SMTP server
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::smtp_read_message(tout_t aTimeout)
{
   return send_wait(F("AT#EMAILMSG"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SMTPCFG command and waits for OK answer
/*! \details
Configure SMTP parameters
 * \param ssl_enabled    Numeric parameter indicating if the SSL encryption is enabled
 * \param port    SMTP port to contact
 * \param mode    SMTP start session command
 * \param un1    for future purposes
 * \param un2    for future purposes
 * \param cid    PDP context identifier
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::smtp_configure(int ssl_enabled, int port, int mode, int un1, int un2, int cid, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SMTPCFG=%d,%d,%d,%d,%d,%d"), ssl_enabled, port, mode, un1, un2, cid);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#ERST command and waits for OK answer
/*! \details
This execution command resets the e-mail parameters to the "factory default" configuration.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::smtp_reset_parameters(tout_t aTimeout)
{
   return send_wait(F("AT#ERST"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#EUSER command and waits for OK answer
/*! \details
This command sets the user identification string to be used during the SMTP authentication step.
 * \param eUser    string containing the e-mail authentication User ID
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::smtp_user(const char *eUser, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#EUSER=\"%s\""), eUser);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#ESAV command and waits for OK answer
/*! \details
This execution command stores the e-mail parameters in the NVM.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::smtp_save_parameters(tout_t aTimeout)
{
   return send_wait(F("AT#ESAV"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#EPASSW command and waits for OK answer
/*! \details
This command sets the password string to be used during the authentication step of the SMTP.
 * \param ePwd    e-mail authentication password that can have any string value up to max length reported by test command
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::smtp_password(const char *ePwd, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#EPASSW=\"%s\""), ePwd);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#EMAILD command and waits for OK answer
/*! \details
This command sends an e-mail message.
 * \param da    destination address (maximum length 100 characters)
 * \param subj    subject of the message (maximum length 100 characters)
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::smtp_mail_send(const char *da, const char *subj, char* data, tout_t aTimeout)
{
   ME310::return_t ret;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#EMAILD=\"%s\",\"%s\""), da, subj);
   ret =  send_wait((char*)mBuffer, WAIT_DATA_STRING, aTimeout);
   if ((ret == RETURN_VALID))
   {
      memset(mBuffer, 0, ME310_BUFFSIZE);
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, data);
      ret =  send_wait((char*)mBuffer, OK_STRING, aTimeout);
   }
   return ret;
}

//! \brief Implements the AT\#EADDR command and waits for OK answer
/*! \details
This command sets the sender address string to be used for sending the e-mail.
 * \param eAddr    sender address
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::smtp_sender_address(const char *eAddr, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#EADDR=\"%s\""), eAddr);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// HTTP ------------------------------------------------------------------------

//! \brief Implements the AT\#HTTPCFG command and waits for OK answer
/*! \details
This command sets the parameters needed to the HTTP connection.
 * \param prof_id    select the profile identifier
 * \param server_address    IP address of the HTTP server
 * \param server_port    select TCP remote port
 * \param auth_type    select HTTP authentication type
 * \param username    configure authentication user identification string for HTTP
 * \param password    configure authentication password string for HTTP
 * \param ssl_enabled    enable/disable SSL encryption
 * \param timeout    timeout, data from HTTPS server
 * \param cid    PDP context identifier
 * \param pkt_size    size for data sending (#HTTPSND) or receiving (#HTTPRCV)
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::configure_http_parameters(int prof_id, const char *server_address, int server_port, int auth_type, const char *username, const char *password, int ssl_enabled, int timeout, int cid, int pkt_size, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#HTTPCFG=%d,\"%s\",%d,%d,\"%s\",\"%s\",%d,%d,%d,%d"), prof_id, server_address, server_port, auth_type, username, password, ssl_enabled, timeout, cid, pkt_size);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#HTTPCFG command and waits for OK answer
/*! \details
This command sets the parameters needed to the HTTP connection.
 * \param prof_id    select the profile identifier
 * \param server_address    IP address of the HTTP server
 * \param server_port    select TCP remote port
 * \param auth_type    select HTTP authentication type
 * \param ssl_enabled    enable/disable SSL encryption
 * \param timeout    timeout, data from HTTPS server
 * \param cid    PDP context identifier
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::configure_http_parameters(int prof_id, const char *server_address, int server_port, int auth_type, int ssl_enabled, int timeout, int cid, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#HTTPCFG=%d,\"%s\",%d,%d,,,%d,%d,%d"), prof_id, server_address, server_port, auth_type, ssl_enabled, timeout, cid);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#HTTPQRY command and waits for OK answer
/*! \details
This command performs a GET, HEAD or DELETE request to HTTP server.
 * \param prof_id    profile identifier
 * \param command    identifies command requested to HTTP server
 * \param resource    is the HTTP resource (URI), object of the request
 * \param extra_header_line    is the optional HTTP header line
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::send_http_query(int prof_id, int command, const char *resource, const char *extra_header_line, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#HTTPQRY=%d,%d,\"%s\",\"%s\""), prof_id, command, resource, extra_header_line);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#HTTPQRY command and waits for OK answer
/*! \details
This command performs a GET, HEAD or DELETE request to HTTP server.
 * \param prof_id    profile identifier
 * \param command    identifies command requested to HTTP server
 * \param resource    is the HTTP resource (URI), object of the request
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::send_http_query(int prof_id, int command, const char *resource, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#HTTPQRY=%d,%d,\"%s\""), prof_id, command, resource);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#HTTPSND command and waits for OK answer
/*! \details
This command performs a POST or PUT request to HTTP server and starts sending data to the server.
 * \param prof_id    profile identifier
 * \param command    command requested to HTTP server
 * \param resource    HTTP resource (uri), object of the request
 * \param data_len    data length to send in bytes
 * \param post_param    HTTP Content-type identifier, used only for POST command, optionally followed by colon character (:) and a string that extends with sub-types the identifier
 * \param extra_header_line    optional HTTP header line
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::send_http_send(int prof_id, int command, const char *resource, int data_len, char *data, const char *post_param, const char *extra_header_line, tout_t aTimeout)
{
   ME310::return_t ret;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#HTTPSND=%d,%d,\"%s\",%d,\"%s\",\"%s\""), prof_id, command, resource, data_len, post_param, extra_header_line);
   ret =  send_wait((char*)mBuffer,SEQUENCE_STRING,aTimeout);
   if ((ret == RETURN_VALID))
   {
      memset(mBuffer, 0, ME310_BUFFSIZE);
      memcpy(mBuffer, data, data_len);
      ret =  send_wait((char*)mBuffer,OK_STRING,TERMINATION_STRING,aTimeout);
   }
   return ret;
}

//! \brief Implements the AT\#HTTPRCV command and returns
/*! \details
This command permits the user to read data from HTTP server in response to a previous HTTP module
request. Returns immediately.
 * \param prof_id    profile identifier
 * \param max_byte max number of bytes to read at a time
 */
void ME310::receive_http_data_start(int prof_id, int max_byte)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#HTTPRCV=%d,%d"), prof_id, max_byte);
   send((char*)mBuffer);
}

//! \brief Implements the AT\#HTTPRCV command and waits for OK answer
/*! \details
This command permits the user to read data from HTTP server in response to a previous HTTP module
request.
 * \param prof_id    profile identifier
 * \param max_byte max number of bytes to read at a time
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::receive_http_data(int prof_id, int max_byte, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#HTTPRCV=%d,%d"), prof_id, max_byte);
   return send_wait((char*)mBuffer,0, OK_STRING, aTimeout);
}

// SSL -------------------------------------------------------------------------

//! \brief Implements the AT\#SSLCFG command and waits for OK answer
/*! \details
This command configures SSL connection parameters.
 * \param ssid    Secure Socket Identifier
 * \param cid    PDP context identifier, see +CGDCONT command
 * \param pktSx    packet size to be used by the SSL/TCP/IP stack for data sending
 * \param maxTo    exchange timeout or socket inactivity timeout
 * \param defTo    timeout that will be used by default whenever the corresponding parameter of each command is not set.
 * \param txTo    data sending timeout
 * \param skipHostMismatch    enables/disables Host Mismatch alert
 * \param sslRingMode    SSLRING unsolicited mode
 * \param equalizeTx    enable/disable equalized data transmission
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ssl_configure_general_param(int ssid , int cid, int pktSx, int maxTo, int defTo, int txTo, int SSLSRingMode, int noCarrierMode, int skipHostMismatch , int equalizeTx, int unused1, int unused2, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SSLCFG=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"), ssid, cid, pktSx, maxTo, defTo, txTo, SSLSRingMode, noCarrierMode, skipHostMismatch, equalizeTx);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SSLSECCFG command and waits for OK answer
/*! \details
This command allows configuring SSL connection parameters.
 * \param ssid    Secure Socket Identifier
 * \param cipherSuite    identify the cipher suite
 * \param authMode    authentication mode
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ssl_configure_security_param(int ssid, int cipherSuite, int authMode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SSLSECCFG=%d,%d,%d"), ssid, cipherSuite, authMode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SSLEN command and waits for OK answer
/*! \details
This command activates/deactivates a socket secured by SSL.
 * \param ssid    Secure Socket Identifier
 * \param enable    activate/deactivate secure socket
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ssl_enable(int ssid, int enable, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SSLEN=%d,%d"), ssid, enable);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SSLD command and waits for OK answer
/*! \details
This command opens a remote connection via socket secured through SSL.
 * \param ssid    Secure Socket Identifier
 * \param rPort    Remote TCP port to contact
 * \param IPAddress    address of SSL server
 * \param closureType    Closure type
 * \param connMode    connection mode (0 online mode, 1 command mode)
 * \param timeout    It represents the maximum allowed TCP inter-packet delay
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ssl_socket_open(int ssid, int rPort, const char *IPAddress, int closureType, int connMode, int timeout, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SSLD=%d,%d,\"%s\",%d,%d,%d"), ssid, rPort, IPAddress, closureType, connMode, timeout);
   if(connMode == 1)
   {
      return send_wait((char*)mBuffer, OK_STRING, aTimeout);
   }
   else
   {
      return send_wait((char*)mBuffer, CONNECT_STRING, aTimeout);
   }
}

//! \brief Implements the AT\#SSLO command and waits for OK answer
/*! \details
This command restores a SSL connection (online mode) suspended by an escape sequence (+++).
 * \param ssid    Secure Socket Identifier
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ssl_socket_restore(int ssid, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SSLO=%d"), ssid);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SSLH command and waits for OK answer
/*! \details
This command allows closing the SSL connection.
 * \param ssid    Secure Socket Identifier
 * \param closureType    type of socket closure
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ssl_socket_close(int ssid, int closureType, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SSLH=%d,%d"), ssid, closureType);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SSLSEND command and waits for OK answer
/*! \details
This command allows sending data through a secure socket.
 * \param ssid    Secure Socket Identifier
 * \param timeout    socket send timeout
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ssl_socket_send_data(int ssid, char *data, int timeout, tout_t aTimeout)
{
   ME310::return_t ret;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SSLSEND=%d,%d"), ssid, timeout);
   ret =  send_wait((char*)mBuffer, WAIT_DATA_STRING, aTimeout);
   if ((ret == RETURN_VALID))
   {
      memset(mBuffer, 0, ME310_BUFFSIZE);
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, data);
      ret =  send_wait((char*)mBuffer, OK_STRING, CTRZ, aTimeout);
   }
   return ret;
}

//! \brief Implements the AT\#SSLRECV command and waits for OK answer
/*! \details
This command reads data from a SSL socket.
 * \param ssid    Secure Socket Identifier.
 * \param maxNumByte    maximum number of bytes to read
 * \param timeout    time-out in 100 ms units
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ssl_socket_receive_data(int ssid, int maxNumByte, int timeout, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SSLRECV=%d,%d,%d"), ssid, maxNumByte, timeout);
   return send_wait((char*)mBuffer, 0, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SSLS command and waits for OK answer
/*! \details
This command reports the status of secure sockets.
 * \param ssid    Secure Socket Identifier.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ssl_socket_status(int ssid, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SSLS=%d"), ssid);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SSLI command and waits for OK answer
/*! \details
This command is used to get information about secure socket data traffic.
 * \param ssid    Secure Socket Identifier
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ssl_socket_info(int ssid, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SSLI=%d"), ssid);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SSLI command and waits for OK answer
/*! \details
This command is used to get information about secure socket data traffic.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ssl_socket_info(tout_t aTimeout)
{
   return send_wait(F("AT#SSLI"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#SSLSENDEXT command and waits for OK answer
/*! \details
This command sends data through a secure socket.
 * \param ssid    Secure Socket Identifier
 * \param bytestosend    number of bytes to be sent
 * \param timeout    time-out in 100 ms units
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ssl_socket_send_data_command_mode(int ssid, int bytestosend, char* data, int timeout, tout_t aTimeout)
{
   ME310::return_t ret;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SSLSENDEXT=%d,%d,%d"), ssid, bytestosend, timeout);
   ret =  send_wait((char*)mBuffer, WAIT_DATA_STRING, aTimeout);
   if ((ret == RETURN_VALID))
   {
      memset(mBuffer, 0, ME310_BUFFSIZE);
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, data);
      ret =  send_wait((char*)mBuffer, OK_STRING, CTRZ, aTimeout);
   }
   return ret;
}

//! \brief Implements the AT\#SSLSECDATA command and waits for data or for OK answer
/*! \details
The command stores, reads, and deletes security data (Certificate, CA certificate, private key) in / from NVM.
 * \param ssid    Secure Socket Identifier
 * \param action    required action
 * \param dataType    security data type
 * \param size    size of security data to be stored
 * \param md5WhenReading    handling of RSA Private key reading when <action> is 2
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ssl_security_data(int ssid, int action, int dataType, int size, int md5WhenReading, char* data, tout_t aTimeout)
{
   ME310::return_t ret = RETURN_TOUT;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SSLSECDATA=%d,%d,%d,%d,%d"), ssid, action, dataType, size, md5WhenReading);
   if(action == 1 || action == 3)
   {
      ret = send_wait((char*)mBuffer, WAIT_DATA_STRING, aTimeout);
      if ((ret == RETURN_VALID))
      {
         memset(mBuffer, 0, ME310_BUFFSIZE);
         snprintf((char *)mBuffer, ME310_BUFFSIZE-1, "%s",data);
         ret = send_wait((char*)mBuffer, 0, OK_STRING, CTRZ, aTimeout);
         return ret;
      }
   }
   else if(action == 2)
   {
      return send_wait((char*)mBuffer, 0, OK_STRING, aTimeout);
   }
   else
   {
      return send_wait((char*)mBuffer, OK_STRING, aTimeout);
   }
   return ret;
}

//! \brief Implements the AT\#SSLSECCFG2 command and waits for OK answer
/*! \details
This command allows configuring additional SSL security parameters.
 * \param ssid    Secure Socket Identifier
 * \param version Select SSL/TLS protocol version
 * \param SNI     enable/disable Service Name Indication
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ssl_additional_parameters(int ssid, int version, int SNI, int preloadedCA, int customCA, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#SSLSECCFG2=%d,%d,%d,%d,%d"), ssid,version, SNI, preloadedCA, customCA);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// CIoT Optimization -----------------------------------------------------------
//! \brief Implements the AT+CCIOTOPT command and waits for OK answer
/*! \details
This command controls CIoT EPS (Cellular Internet of Things, Evolved Packet System) optimizations.
 * \param n    enables/disables reporting of unsolicited
 * \param supportedUEopt     indicates the UE's support for CIoT EPS optimizations
 * \param preferredUEopt  indicates the UE's preference for CIoT EPS optimizations
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ciot_optimization_configuration(int n, int supportedUEopt, int preferredUEopt, tout_t aTimeout )
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CCIOTOPT=%d,%d,%d"), n, supportedUEopt, preferredUEopt);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT#CCIOTOPT command and waits for OK answer
/*! \details
This command controls CIoT EPS (Cellular Internet of Things, Evolved Packet System) optimizations.
 * \param bitmask    used to store the mask in NVM for subsequent use by protocol stack
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ciot_optimization_configuration2(char *bitmask, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#CCIOTOPT=%s"), bitmask);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// IoT Portal-------------------------------------------------------------------
//! \brief Implements the AT+ODIS command and waits for OK answer
/*! \details
This command allows the end-user to handle the Host Odis parameters. In case of AT&T LwM2M agent up
and running, the command is executed internally to the LwM2M client, updating this client about the values
change
 * \param hostUniqueDevId    contains a string, between double quotes, with the host unique device identifier
 * \param hostManufacturer   contains a string, between double quotes, with the host manufacturer identifier
 * \param hostModel          contains a string, between double quotes, with the host model identifier
 * \param hostSwVersion      contains a string, between double quotes, with the host software version identifier
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::odis_command_saving_retrieving_parameters(char* hostUniqueDevId, char *hostManufacturer, char* hostModel, char *hostSwVersion, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+ODIS=%s,%s,%s,%s"), hostUniqueDevId, hostManufacturer, hostModel, hostSwVersion);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}


//! \brief Implements the AT#FOTAURC command and waits for OK answer
/*! \details
This command allows the end-user to enable/disable the FOTA extended URCs, resulting in a verbosity FOTA
operations. Those settings are generally neither related nor manageable with other LwM2M agent commands.
 * \param enable    enable/disable extended URCs
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::FOTA_set_extended_URC(int enable, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#FOTAURC=%d"), enable);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT#OTAUPW command and waits for OK answer
/*! \details
Execution command starts injection of a delta file into the device
 * \param size    size in bytes of data to be injected
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::OTA_delta_write(int size,  tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#OTAUPW=%d"), size);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT#HOSTODIS command and waits for OK answer
/*! \details
The command manages the Host Odis parameters related to AT&T OMA-DM Client
 * \param param    selects the specific item on which work
 * \param action   selects the action to be performed on the item selected by param
 * \param value    contains a string, between double quotes, with data to be set. Maximum string length is 64 characters. It is valid only if <Action> = 0 ("SET" action)
 * \param instance instance number
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::odis_parameters_management(int param, int action, char* value, int instance, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   if(action == 0)
   {
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#HOSTODIS=%d,%d,%s,%d"), param, action, value, instance);
   }
   else
   {
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#HOSTODIS=%d,%d,%d"), param, action,instance);
   }
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MENA command and waits for unsolicited message
/*! \details
This function enables the Telit LwM2M Client feature.
 * \param enable contains the value that enables the LWM2M client
 * \param ctxID  contains the PDP context identifier
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::enableLWM2M(int enable, int ctxID, tout_t aTimeout)
{
   return LWM2M_enable(enable, ctxID, aTimeout);
}

/*! \brief Implements the AT#LWM2MENA command and waits for unsolicited message
/*! \details
This function enables the Telit LwM2M Client feature.
 * \param enable contains the value that enables the LWM2M client
 * \param ctxID  contains the PDP context identifier
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::LWM2M_enable(int enable, int ctxID,tout_t aTimeout)
{
   ME310::return_t ret;
	memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MENA=%d,%d"), enable, ctxID);
	ret = send_wait((char*)mBuffer,OK_STRING, aTimeout);
   if ((ret == RETURN_VALID))
   {
      memset(mBuffer, 0, ME310_BUFFSIZE);
      ret = wait_for_unsolicited(aTimeout);
   }
   return ret;
}

/*! \brief Implements the AT#LWM2MENA command and waits for unsolicited message
/*! \details
This function disable the Telit LwM2M Client feature.
 * \param disable contains the value that disables the LWM2M client
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::disableLWM2M(int disable, tout_t aTimeout)
{
   return LWM2M_disable(disable, aTimeout);
}

/*! \brief Implements the AT#LWM2MENA command and waits for unsolicited message
/*! \details
This function disable the Telit LwM2M Client feature.
 * \param disable contains the value that disables the LWM2M client
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::LWM2M_disable(int disable, tout_t aTimeout)
{
   ME310::return_t ret;
	memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MENA=%d"), disable);
	ret = send_wait((char*)mBuffer,OK_STRING, aTimeout);
   if ((ret == RETURN_VALID))
   {
      memset(mBuffer, 0, ME310_BUFFSIZE);
      ret = wait_for_unsolicited(aTimeout);
   }
   return ret;
}

/*! \brief Implements the AT#LWM2MW command and wait OK answer
/*! \details
This function selects the parameters for the write operation on the lwm2m agent, it requires the
correspondent lwm2m agent enabled and working.
 * \param agent specifies the agent (0=telit agent)
 * \param objID  identifies the object ID
 * \param instanceID identifies the object instance
 * \param resourceID identifies the resource ID
 * \param resourceInstance identifies the resource instance
 * \param value sets the resource value
 * \param aTimeout timeout in ms
 * \return return code
*/
 ME310::return_t ME310::writeResource(int agent, int objID, int instanceID, int resourceID, int resourceInstance, int value, tout_t aTimeout)
 {
	return LWM2M_write_resource(agent, objID, instanceID, resourceID, resourceInstance, value, aTimeout);
}

/*! \brief Implements the AT#LWM2MW command and wait OK answer
/*! \details
This function selects the parameters for the write operation on the lwm2m agent, it requires the
correspondent lwm2m agent enabled and working.
 * \param agent specifies the agent (0=telit agent)
 * \param objID  identifies the object ID
 * \param instanceID identifies the object instance
 * \param resourceID identifies the resource ID
 * \param resourceInstance identifies the resource instance
 * \param value sets the resource value
 * \param aTimeout timeout in ms
 * \return return code
*/
 ME310::return_t ME310::LWM2M_write_resource(int agent, int objID, int instanceID, int resourceID, int resourceInstance, int value, tout_t aTimeout)
 {
	memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MW=%d,%d,%d,%d,%d,%d"), agent,objID,instanceID,resourceID, resourceInstance,value);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MW command and wait OK answer
/*! \details
This function selects the parameters for the write operation on the lwm2m agent, it requires the
correspondent lwm2m agent enabled and working.
 * \param agent specifies the agent (0=telit agent)
 * \param objID  identifies the object ID
 * \param instanceID identifies the object instance
 * \param resourceID identifies the resource ID
 * \param resourceInstance identifies the resource instance
 * \param value sets the resource value
 * \param aTimeout timeout in ms
 * \return return code
*/
 ME310::return_t ME310::writeResource(int agent, int objID, int instanceID, int resourceID, int resourceInstance, char* value, tout_t aTimeout)
 {
	return LWM2M_write_resource(agent, objID, instanceID, resourceID, resourceInstance, value);
}

/*! \brief Implements the AT#LWM2MW command and wait OK answer
/*! \details
This function selects the parameters for the write operation on the lwm2m agent, it requires the
correspondent lwm2m agent enabled and working.
 * \param agent specifies the agent (0=telit agent)
 * \param objID  identifies the object ID
 * \param instanceID identifies the object instance
 * \param resourceID identifies the resource ID
 * \param resourceInstance identifies the resource instance
 * \param value sets the resource value
 * \param aTimeout timeout in ms
 * \return return code
*/
 ME310::return_t ME310::LWM2M_write_resource(int agent, int objID, int instanceID, int resourceID, int resourceInstance, char* value, tout_t aTimeout)
 {
	memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MW=%d,%d,%d,%d,%d,%s"), agent,objID,instanceID,resourceID, resourceInstance,value);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MW command and wait OK answer
/*! \details
This function selects the parameters for the write operation on the lwm2m agent, it requires the
correspondent lwm2m agent enabled and working.
 * \param agent specifies the agent (0=telit agent)
 * \param objID  identifies the object ID
 * \param instanceID identifies the object instance
 * \param resourceID identifies the resource ID
 * \param resourceInstance identifies the resource instance
 * \param value sets the resource value
 * \param aTimeout timeout in ms
 * \return return code
*/
 ME310::return_t ME310::writeResource(int agent, int objID, int instanceID, int resourceID, int resourceInstance, float value, tout_t aTimeout)
 {
	return LWM2M_write_resource(agent, objID, instanceID, resourceID, resourceInstance, value, aTimeout);
 }

/*! \brief Implements the AT#LWM2MW command and wait OK answer
/*! \details
This function selects the parameters for the write operation on the lwm2m agent, it requires the
correspondent lwm2m agent enabled and working.
 * \param agent specifies the agent (0=telit agent)
 * \param objID  identifies the object ID
 * \param instanceID identifies the object instance
 * \param resourceID identifies the resource ID
 * \param resourceInstance identifies the resource instance
 * \param value sets the resource value
 * \param aTimeout timeout in ms
 * \return return code
*/
 ME310::return_t ME310::LWM2M_write_resource(int agent, int objID, int instanceID, int resourceID, int resourceInstance, float value, tout_t aTimeout)
 {
	memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MW=%d,%d,%d,%d,%d,%f"), agent,objID,instanceID,resourceID, resourceInstance,value);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MW command and wait OK answer
/*! \details
This function selects the parameters for the write operation on the lwm2m agent, it requires the
correspondent lwm2m agent enabled and working.
 * \param agent specifies the agent (0=telit agent)
 * \param objID  identifies the object ID
 * \param instanceID identifies the object instance
 * \param resourceID identifies the resource ID
 * \param resourceInstance identifies the resource instance
 * \param value sets the resource value (float)
 * \param aTimeout timeout in ms
 * \return return code
*/
ME310::return_t ME310::writeResourcefloat(int agent, int objID, int instanceID, int resourceID, int resourceInstance, float value, tout_t aTimeout)
{
	return LWM2M_write_resource_float(agent, objID, instanceID, resourceID, resourceInstance, value, aTimeout);
}

/*! \brief Implements the AT#LWM2MW command and wait OK answer
/*! \details
This function selects the parameters for the write operation on the lwm2m agent, it requires the
correspondent lwm2m agent enabled and working.
 * \param agent specifies the agent (0=telit agent)
 * \param objID  identifies the object ID
 * \param instanceID identifies the object instance
 * \param resourceID identifies the resource ID
 * \param resourceInstance identifies the resource instance
 * \param value sets the resource value (float)
 * \param aTimeout timeout in ms
 * \return return code
*/
ME310::return_t ME310::LWM2M_write_resource_float(int agent, int objID, int instanceID, int resourceID, int resourceInstance, float value, tout_t aTimeout)
{
	memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MW=%d,%d,%d,%d,%d,%f"), agent,objID,instanceID,resourceID, resourceInstance,value);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MSET command and wait OK answer
/*! \details
This function sets a user defined value to the specified resource, if whitelisted.
 * \param type specifies the type of data to insert
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param value identifies the value to be set
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::setResourcefloat(int type, int objID, int instanceID, int resourceID, int resourceInstance, float value, tout_t aTimeout)
{
   (void) type;
   return setResourceFloat(objID, instanceID, resourceID, resourceInstance, value, aTimeout);
}

/*! \brief Implements the AT#LWM2MSET command and wait OK answer
/*! \details
This function sets a user defined value to the specified resource, if whitelisted.
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param value identifies the value to be set (float)
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::setResourceFloat(int objID, int instanceID, int resourceID, int resourceInstance, float value, tout_t aTimeout)
{
   return LWM2M_set_resource_float(objID, instanceID, resourceID, resourceInstance, value, aTimeout);
}

/*! \brief Implements the AT#LWM2MSET command and wait OK answer
/*! \details
This function sets a user defined value to the specified resource, if whitelisted.
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param value identifies the value to be set (float)
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_set_resource_float(int objID, int instanceID, int resourceID, int resourceInstance, float value, tout_t aTimeout)
{
   char buff[20];
   memset(mBuffer,0,ME310_BUFFSIZE);
   floatToString(value, 6, buff, sizeof(buff));
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MSET=%d,%d,%d,%d,%d,%s"), LWM2M_SET_FLOAT, objID, instanceID, resourceID, resourceInstance, buff);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MSET command and wait OK answer
/*! \details
This function sets a user defined value to the specified resource, if whitelisted.
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param value identifies the value to be set (int)
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::setResourceInt(int objID, int instanceID, int resourceID, int resourceInstance, int value, tout_t aTimeout)
{
	return LWM2M_set_resource_int(objID, instanceID, resourceID, resourceInstance, value, aTimeout);
}

/*! \brief Implements the AT#LWM2MSET command and wait OK answer
/*! \details
This function sets a user defined value to the specified resource, if whitelisted.
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param value identifies the value to be set (int)
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_set_resource_int(int objID, int instanceID, int resourceID, int resourceInstance, int value, tout_t aTimeout)
{
	memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MSET=%d,%d,%d,%d,%d,%d"), LWM2M_SET_INT, objID, instanceID,resourceID, resourceInstance, value);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MSET command and wait OK answer
/*! \details
This function sets a user defined value to the specified resource, if whitelisted.
 * \param type specifies the type of data to insert
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param value identifies the value to be set (bool)
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::setResourceBool(int type, int objID, int instanceID, int resourceID, int resourceInstance, int value, tout_t aTimeout)
{
   (void)type;
	return setResourceBool(objID, instanceID, resourceID, resourceInstance, value, aTimeout);
}

/*! \brief Implements the AT#LWM2MSET command and wait OK answer
/*! \details
This function sets a user defined value to the specified resource, if whitelisted.
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param value identifies the value to be set (bool)
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::setResourceBool(int objID, int instanceID, int resourceID, int resourceInstance, int value, tout_t aTimeout)
{
	return LWM2M_set_resource_bool(objID, instanceID, resourceID, resourceInstance, value, aTimeout);
}

/*! \brief Implements the AT#LWM2MSET command and wait OK answer
/*! \details
This function sets a user defined value to the specified resource, if whitelisted.
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param value identifies the value to be set (bool)
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_set_resource_bool(int objID, int instanceID, int resourceID, int resourceInstance, int value, tout_t aTimeout)
{
	memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MSET=%d,%d,%d,%d,%d,%d"), LWM2M_SET_INT, objID, instanceID, resourceID, resourceInstance, value);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MSET command and wait OK answer
/*! \details
This function sets a user defined value to the specified resource, if whitelisted.
 * \param type specifies the type of data to insert
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param value identifies the value to be set (char*)
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::setResourceString(int type, int objID, int instanceID, int resourceID, int resourceInstance, char* value, tout_t aTimeout)
{
   (void) type;
	return setResourceString(objID, instanceID, resourceID, resourceInstance, value);
}

/*! \brief Implements the AT#LWM2MSET command and wait OK answer
/*! \details
This function sets a user defined value to the specified resource, if whitelisted.
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param value identifies the value to be set (char*)
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::setResourceString(int objID, int instanceID, int resourceID, int resourceInstance, char* value, tout_t aTimeout)
{
	return LWM2M_set_resource_string(objID, instanceID, resourceID, resourceInstance, value, aTimeout);
}

/*! \brief Implements the AT#LWM2MSET command and wait OK answer
/*! \details
This function sets a user defined value to the specified resource, if whitelisted.
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param value identifies the value to be set (char*)
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_set_resource_string(int objID, int instanceID, int resourceID, int resourceInstance, char* value, tout_t aTimeout)
{
	memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MSET=%d,%d,%d,%d,%d,%s"), LWM2M_SET_STRING, objID, instanceID, resourceID, resourceInstance, value);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MSET command and wait OK answer
/*! \details
This function sets a user defined value to the specified resource, if whitelisted.
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param value object link values, string of max 11 characters, represented as a couple of integer numbers separated by colon, which represent an Object ID and an Object Instance ID.
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::setResourceObjectLink(int objID, int instanceID, int resourceID, int resourceInstance, char* value, tout_t aTimeout)
{
	return LWM2M_set_resource_object_link(objID, instanceID, resourceID, resourceInstance, value, aTimeout);
}

/*! \brief Implements the AT#LWM2MSET command and wait OK answer
/*! \details
This function sets a user defined value to the specified resource, if whitelisted.
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param value object link values, string of max 11 characters, represented as a couple of integer numbers separated by colon, which represent an Object ID and an Object Instance ID.
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_set_resource_object_link(int objID, int instanceID, int resourceID, int resourceInstance, char* value, tout_t aTimeout)
{
	memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MSET=%d,%d,%d,%d,%d,%s"), LWM2M_SET_OBJECT_LINK, objID, instanceID, resourceID, resourceInstance, value);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MSET command and wait OK answer
/*! \details
This function sets a user defined value to the specified resource, if whitelisted.
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param value time values, same notation as integer values, expressed as the number of seconds since Jan 1st, 1970 in the UTC time zone.
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::setResourceTime(int objID, int instanceID, int resourceID, int resourceInstance, int value, tout_t aTimeout)
{
	return LWM2M_set_resource_time(objID, instanceID, resourceID, resourceInstance, value, aTimeout);
}

/*! \brief Implements the AT#LWM2MSET command and wait OK answer
/*! \details
This function sets a user defined value to the specified resource, if whitelisted.
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param value time values, same notation as integer values, expressed as the number of seconds since Jan 1st, 1970 in the UTC time zone.
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_set_resource_time(int objID, int instanceID, int resourceID, int resourceInstance, int value, tout_t aTimeout)
{
	memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MSET=%d,%d,%d,%d,%d,%d"), LWM2M_SET_TIME, objID, instanceID,resourceID, resourceInstance, value);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MOBJSET command and wait OK answer
/*! \details
This function sets a object by a json string.
 * \param agent identifies the agent LWM2M
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param jsonString json string contained object parameters
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::setObject(int agent, int objID, int instanceID, char* jsonString, tout_t aTimeout)
{
   return LWM2M_set_object(agent, objID, instanceID, jsonString, aTimeout);
}

/*! \brief Implements the AT#LWM2MOBJSET command and wait OK answer
/*! \details
This function sets a object by a json string.
 * \param agent identifies the agent LWM2M
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param jsonString json string contained object parameters
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_set_object(int agent, int objID, int instanceID, char* jsonString, tout_t aTimeout)
{
   ME310::return_t ret;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#LWM2MOBJSET=%d,%d,%d"), agent, objID, instanceID);
   ret =  send_wait((char*)mBuffer, WAIT_DATA_STRING, aTimeout);
   if ((ret == RETURN_VALID))
   {
      memset(mBuffer, 0, ME310_BUFFSIZE);
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, jsonString);
      ret =  send_wait((char*)mBuffer, OK_STRING, CTRZ, aTimeout);
   }
   return ret;
}
/*! \brief Implements the AT#LWM2MR command and wait OK answer
/*! \details
This function selects the parameters for the read operation on the lwm2m agent, it requires the
correspondent lwm2m agent enabled and working.
 * \param agent identifies the agent LWM2M
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::readResource(int agent, int objID, int instanceID, int resourceID, int resourceInstance, tout_t aTimeout)
{
	return LWM2M_read_resource(agent, objID, instanceID, resourceID, resourceInstance, aTimeout);
}

/*! \brief Implements the AT#LWM2MR command and wait OK answer
/*! \details
This function selects the parameters for the read operation on the lwm2m agent, it requires the
correspondent lwm2m agent enabled and working.
 * \param agent identifies the agent LWM2M
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_read_resource(int agent, int objID, int instanceID, int resourceID, int resourceInstance, tout_t aTimeout)
{
	memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MR=%d,%d,%d,%d,%d"), agent,objID,instanceID,resourceID, resourceInstance);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}
/*! \brief Implements the AT#LWM2MR command and wait OK answer
/*! \details
This function selects the parameters for the read operation on the lwm2m agent, it requires the
correspondent lwm2m agent enabled and working.
 * \param agent identifies the agent LWM2M
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::readResourcefloat(int agent,int objID,int instanceID,int resourceID, int resourceInstance, tout_t aTimeout)
{
	return readResourceFloat(agent, objID, instanceID, resourceInstance, aTimeout);
}

/*! \brief Implements the AT#LWM2MR command and wait OK answer
/*! \details
This function selects the parameters for the read operation on the lwm2m agent, it requires the
correspondent lwm2m agent enabled and working.
 * \param agent identifies the agent LWM2M
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::readResourceFloat(int agent, int objID, int instanceID, int resourceID, int resourceInstance, tout_t aTimeout)
{
	return LWM2M_read_resource_float(agent, objID, instanceID, resourceID, resourceInstance, aTimeout);
}

/*! \brief Implements the AT#LWM2MR command and wait OK answer
/*! \details
This function selects the parameters for the read operation on the lwm2m agent, it requires the
correspondent lwm2m agent enabled and working.
 * \param agent identifies the agent LWM2M
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_read_resource_float(int agent, int objID, int instanceID, int resourceID, int resourceInstance, tout_t aTimeout)
{
	memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MR=%d,%d,%d,%d,%d"), agent,objID,instanceID,resourceID, resourceInstance);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MR command and wait OK answer
/*! \details
This function selects the parameters for the read operation on the lwm2m agent, it requires the
correspondent lwm2m agent enabled and working.
 * \param agent identifies the agent LWM2M
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param value identifies the value to be set (int)
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::readResourceInt(int agent, int objID, int instanceID, int resourceID, int resourceInstance, int &value, tout_t aTimeout)
{
   ME310::return_t ret;
   int i = 0;
	memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MR=%d,%d,%d,%d,%d"), agent,objID,instanceID,resourceID, resourceInstance);
	ret = send_wait((char*)mBuffer,OK_STRING, aTimeout);
   if(ret == RETURN_VALID)
   {
      while(buffer_cstr(i) != NULL)
      {
         String retValue = buffer_cstr(i);
         if(retValue.startsWith("#LWM2MR:"))
         {
            int pos = retValue.indexOf(":");
            value = atoi(retValue.substring(pos +1).c_str());
            ret = RETURN_VALID;
            break;
         }
         i++;
      }
   }
   return ret;
}

/*! \brief Implements the AT#LWM2MR command and wait OK answer
/*! \details
This function selects the parameters for the read operation on the lwm2m agent, it requires the
correspondent lwm2m agent enabled and working.
 * \param agent identifies the agent LWM2M
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param value identifies the value to be set (int)
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_read_resource_int(int agent, int objID, int instanceID, int resourceID, int resourceInstance, int &value, tout_t aTimeout)
{
   ME310::return_t ret;
   int i = 0;
	memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MR=%d,%d,%d,%d,%d"), agent,objID,instanceID,resourceID, resourceInstance);
	ret = send_wait((char*)mBuffer,OK_STRING, aTimeout);
   if(ret == RETURN_VALID)
   {
      while(buffer_cstr(i) != NULL)
      {
         String retValue = buffer_cstr(i);
         if(retValue.startsWith("#LWM2MR:"))
         {
            int pos = retValue.indexOf(":");
            value = atoi(retValue.substring(pos +1).c_str());
            ret = RETURN_VALID;
            break;
         }
         i++;
      }
   }
   return ret;
}

/*! \brief Implements the AT#LWM2MR command and wait OK answer
/*! \details
This function selects the parameters for the read operation on the lwm2m agent, it requires the
correspondent lwm2m agent enabled and working.
 * \param agent identifies the agent LWM2M
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::readResourceString(int agent, int objID, int instanceID, int resourceID, int resourceInstance, tout_t aTimeout)
{
   return LWM2M_read_resource_string(agent, objID, instanceID, resourceID, resourceInstance, aTimeout);
}

/*! \brief Implements the AT#LWM2MR command and wait OK answer
/*! \details
This function selects the parameters for the read operation on the lwm2m agent, it requires the
correspondent lwm2m agent enabled and working.
 * \param agent identifies the agent LWM2M
 * \param objID identifies the object LWM2M
 * \param instanceID identifies the instance of the object
 * \param resourceID identifies the resource of the object
 * \param resourceInstance identifies the instance of the resource
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_read_resource_string(int agent, int objID, int instanceID, int resourceID, int resourceInstance, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MR=%d,%d,%d,%d,%d"), agent, objID, instanceID, resourceID, resourceInstance);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MEXIST command and wait OK answer
/*! \details
This function allows the end-user to query the module in order to discover if a given agent exist.
 * \param agentInstance identifies the agent LWM2M
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::checkAgentExist(int agentInstance, tout_t aTimeout)
{
	return LWM2M_check_agent_exist(agentInstance, aTimeout);
}

/*! \brief Implements the AT#LWM2MEXIST command and wait OK answer
/*! \details
This function allows the end-user to query the module in order to discover if a given agent exist.
 * \param agentInstance identifies the agent LWM2M
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_check_agent_exist(int agentInstance, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MEXIST=%d"), agentInstance);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MEXIST command and wait OK answer
/*! \details
This function allows the end-user to query the module in order to discover if agiven agent and URI path combination exist.
 * \param agentInstance identifies the agent LWM2M
 * \param objectNumber object number to be selected
 * \param objectInstanceNumber object instance for the query
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::checkObjectExist(int agentInstance, int objectNumber, int objectInstanceNumber, tout_t aTimeout)
{
   return LWM2M_check_object_exist(agentInstance, objectNumber, objectInstanceNumber, aTimeout);
}

/*! \brief Implements the AT#LWM2MEXIST command and wait OK answer
/*! \details
This function allows the end-user to query the module in order to discover if agiven agent and URI path combination exist.
 * \param agentInstance identifies the agent LWM2M
 * \param objectNumber object number to be selected
 * \param objectInstanceNumber object instance for the query
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_check_object_exist(int agentInstance, int objectNumber, int objectInstanceNumber, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MEXIST=%d,%d,%d"), agentInstance, objectNumber, objectInstanceNumber);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MEXIST command and wait OK answer
/*! \details
This function allows the end-user to query the module in order to discover if agiven agent and URI path combination exist.
 * \param agentInstance identifies the agent LWM2M
 * \param objectNumber object number to be selected
 * \param objectInstanceNumber object instance for the query
 * \param resourceNumber resource number for the query
 * \param resourceInstanceNumber resource instance number for the query
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::checkURIExist(int agentInstance, int objectNumber, int objectInstanceNumber, int resourceNumber, int resourceInstanceNumber, tout_t aTimeout)
{
  return LWM2M_check_URI_exist(agentInstance, objectNumber, objectInstanceNumber, resourceNumber, resourceInstanceNumber, aTimeout);
}

/*! \brief Implements the AT#LWM2MEXIST command and wait OK answer
/*! \details
This function allows the end-user to query the module in order to discover if agiven agent and URI path combination exist.
 * \param agentInstance identifies the agent LWM2M
 * \param objectNumber object number to be selected
 * \param objectInstanceNumber object instance for the query
 * \param resourceNumber resource number for the query
 * \param resourceInstanceNumber resource instance number for the query
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_check_URI_exist(int agentInstance, int objectNumber, int objectInstanceNumber, int resourceNumber, int resourceInstanceNumber, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MEXIST=%d,%d,%d,%d,%d"), agentInstance, objectNumber, objectInstanceNumber, resourceNumber, resourceInstanceNumber);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MNEWINST command and wait OK answer
/*! \details
This function can be used to create dynamically the new object instance ID of the specified object.
 * \param agentInstance identifies the agent LWM2M
 * \param objectID object number to be selected
 * \param objectInstanceID object instance for the query
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::createNewObjectInstance(int agentInstance, int objectID, int objectInstanceID, tout_t aTimeout)
{
   return LWM2M_create_new_object_instance(agentInstance, objectID, objectInstanceID, aTimeout);
}

/*! \brief Implements the AT#LWM2MNEWINST command and wait OK answer
/*! \details
This function can be used to create dynamically the new object instance ID of the specified object.
 * \param agentInstance identifies the agent LWM2M
 * \param objectID object number to be selected
 * \param objectInstanceID object instance for the query
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_create_new_object_instance(int agentInstance, int objectID, int objectInstanceID, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MNEWINST=%d,%d,%d"), agentInstance, objectID, objectInstanceID);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MACK command and wait OK answer
/*! \details
This function requires an ACK to performs its operations on the dedicated data context.
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::sendLWM2MACK(tout_t aTimeout)
{
   return LWM2M_send_ACK(aTimeout);
}

/*! \brief Implements the AT#LWM2MACK command and wait OK answer
/*! \details
This function requires an ACK to performs its operations on the dedicated data context.
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_send_ACK(tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MACK=1"));
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}
/*! \brief Implements the AT#LWM2MCFG command and wait OK answer
/*! \details
This function allows the user to configure a parameter specified by parameter ID.
 * \param agentID identifier of the LwM2M agent related to the request
 * \param paramID identifier of the parameter to be configured
 * \param actionID the required action
 * \param value identifier of the configured value
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_set_configuration(int agentID, int paramID, int value, tout_t aTimeout)
{
   int actionID = 0;
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MCFG=%d,%d,%d,%d"), agentID, paramID, actionID, value);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MCFG command and wait OK answer
/*! \details
This function allows the user to configure a parameter specified by parameter ID.
 * \param agentID identifier of the LwM2M agent related to the request
 * \param paramID identifier of the parameter to be configured
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_get_configuration(int agentID, int paramID, tout_t aTimeout)
{
   int actionID = 1;
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MCFG=%d,%d,%d,%d"), agentID, paramID, actionID);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MCIPHERENA command and wait OK answer
/*! \details
This function sets the cipher suits for the agent specified.
 * \param agentID identifier of the LwM2M agent related to the request
 * \param cipher_mode Cipher mode (0 is default ciphers, 1 is advanced ciphers)
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_set_ciphers(int agentID, int cipher_mode, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MCIPHERENA=%d,%d"), agentID, cipher_mode);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MCUST command and wait OK answer
/*! \details
This function allows the end-user to set LwM2M customization parameters related to the module.\n
Those settings are generally neither related nor manageable with other LwM2M agent commands
 * \param paramID identifier of the parameter to be set.
 * \param data data to be set.
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_set_general_parameter(int paramID, char* data, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MCUST=%d,%s"), paramID, data);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2ME command and wait OK answer
/*! \details
This function allows the end-user to set LwM2M customization parameters related to the module.\n
Those settings are generally neither related nor manageable with other LwM2M agent commands
 * \param agentInstance selects the lwm2m instance.
 * \param objectID select object identifier.
 * \param objectInstanceID select object Instance identifier for the query
 * \param resourceID select resource identifier
 * \param resourceInstanceID selects the resource instance identifier
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_client_resource_exec(int agentInstance, int objectID, int objectInstanceID, int resourceID, int resourceInstanceID, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2ME=%d,%d,%d,%d,%d"), agentInstance, objectID, objectInstanceID, resourceID, resourceInstanceID);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MGET command and wait OK answer
/*! \details
This function gets a user defined value to the specified resource, if URI is whitelisted; error otherwise.
 * \param type data type to be inserted.
 * \param objectID select object identifier.
 * \param objectInstanceID select object Instance identifier for the query
 * \param resourceID select resource identifier
 * \param resourceInstanceID selects the resource instance identifier
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_get_resource(int type, int objectID, int objectInstanceID, int resourceID, int resourceInstanceID, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MGET=%d,%d,%d,%d,%d"), type, objectID, objectInstanceID, resourceID, resourceInstanceID);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MLIST command and wait OK answer
/*! \details
This function allows the end-user to query the module to retrieve the list of the objects and object instances supported for a given agent.
 * \param agentInstance selects the lwm2m instance.
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_get_report_object(int agentInstance, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MLIST=%d"), agentInstance);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MMON command and wait OK answer
/*! \details
This function can be used to activate/deactivate the resource changes monitoring.
 * \param action activate (value is 1), deactivate (value is 0) the resource monitoring.
 * \param objectID object identifier.
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_activate_resource(int action, int objectID, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MMON=%d,%d"), action, objectID);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MNFYACKENA command and wait OK answer
/*! \details
This function can be used to activate/deactivate the resource changes monitoring.
 * \param agentInstanceID selects the lwm2m instance
 * \param action disable URC reporting (value is 0), enable URC reporting (value is 1), read URC reporting status (value is 2).
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_control_URC_reporting(int agentInstanceID, int action, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MNFYACKENA=%d,%d"), agentInstanceID, action);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MNFYACKURI command and wait OK answer
/*! \details
This function can be used to remove, add and list the URIs of the resources\n
for issuing an URC at reception of an Ack sent from a server that receives a LWM2M notify for that resource.
 * \param agentInstanceID selects the lwm2m instance.
 * \param action remove URI reporting (value is 0), add URI reporting (value is 1), list URI reporting (value is 2).
 * \param objectID selects the object identifier of the URI.
 * \param objectInstanceID selects the object instance identifier of the URI,
 * \param resourceID selects the resource identifier of the URI.
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_manage_URI_reporting(int agentInstanceID, int action, int objectID, int objectInstanceID, int resourceID, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MNFYACKURI=%d,%d,%d,%d,%d"), agentInstanceID, action, objectID, objectInstanceID, resourceID);
	return send_wait((char*)mBuffer,OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MOBJGET command and wait OK answer
/*! \details
This function allows the user to read a LWM2M object.
 * \param agentInstanceID selects the lwm2m instance.
 * \param objectID selects the object identifier of the URI.
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_get_object(int agentInstanceID, int objectID, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MOBJGET=%d,%d"), agentInstanceID, objectID);
	return send_wait((char*)mBuffer, 0, OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MOBJGET command and wait OK answer
/*! \details
This function allows the user to read a LWM2M object instance.
 * \param agentInstanceID selects the lwm2m instance.
 * \param objectID selects the object identifier of the URI.
 * \param objectInstanceID selects object instance identifier.
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_get_object_instance(int agentInstanceID, int objectID, int objectInstanceID, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MOBJGET=%d,%d,%d"), agentInstanceID, objectID, objectInstanceID);
	return send_wait((char*)mBuffer, 0, OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MOBJGET command and wait OK answer
/*! \details
This function allows the user to read a LWM2M object resource.
 * \param agentInstanceID selects the lwm2m instance.
 * \param objectID selects the object identifier of the URI.
 * \param objectInstanceID selects object instance identifier.
 * \param resourceID selects resource identifier
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_get_object_resource(int agentInstanceID, int objectID, int objectInstanceID, int resourceID, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MOBJGET=%d,%d,%d,%d"), agentInstanceID, objectID, objectInstanceID,resourceID);
	return send_wait((char*)mBuffer, 0, OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MSTAT command and wait OK answer
/*! \details
This function sends a query about the status to the Telit LwM2M client.
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_client_current_status(tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MSTAT"));
	return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MREG command and wait OK answer
/*! \details
This function allows the user to request a full registration, a deregistration or a registration update to a LwM2M server.
 * \param agentInstanceID identifier of the LwM2M agent related to the request.
 * \param actionID identifier of the required action.
 * \param shortServerID identifier of the server related to the request.
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_registration(int agentInstanceID, int actionID, int shortServerID, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MREG=%d,%d,%d"), agentInstanceID, actionID, shortServerID);
	return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MREG command and wait OK answer
/*! \details
This function allows the user to request the registration info to a LwM2M server.
 * \param agentInstanceID identifier of the LwM2M agent related to the request.
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_registration_info(int agentInstanceID, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MREG=%d,%d"), agentInstanceID, REGISTRATION_INFO);
	return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MFOTAACK command and wait OK answer
/*! \details
This function sends an ACK to the LwM2M Client to authorize the FOTA operation required to the specified client.
 * \param agentInstanceID identifier of the LwM2M agent related to the request.
 * \param action acknowledge to the FOTA required operation
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_FOTA_operation_confirmation(int agentInstanceID, int action, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MFOTAACK=%d,%d"), agentInstanceID, action);
	return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MFOTACFG command and wait OK answer
/*! \details
This function sends an ACK to the LwM2M Client to authorize the FOTA operation required to the specified client.
 * \param agentInstanceID identifier of the LwM2M agent related to the request.
 * \param mode select the FOTA mode
 * \param timeoutAction select the action to be performed after the FOTA timeout for ACK expiration.
 * \param aTimeout specifies the timeout
 * \return return code
*/
ME310::return_t ME310::LWM2M_FOTA_configuration(int agentInstanceID, int mode, int timeoutAction, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MFOTACFG=%d,%d,%d"), agentInstanceID, mode, timeoutAction);
	return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

/*! \brief Implements the AT#LWM2MFOTASTATE command and wait OK answer
/*! \details
This function allows the end-user to query the module in order to retrieve, for a given agent, the FOTA client state,
both in terms of the LwM2M specification status and the internal FOTA client management status.
 * \param agentInstanceID identifier of the LwM2M agent related to the request.
 * \param aTimeout specifies the timeout.
 * \return return code
*/
ME310::return_t ME310::LWM2M_FOTA_state(int agentInstanceID, tout_t aTimeout)
{
   memset(mBuffer,0,ME310_BUFFSIZE);
	snprintf((char*)mBuffer, ME310_BUFFSIZE-1,F("AT#LWM2MFOTASTATE=%d"), agentInstanceID);
	return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}
// M2M -------------------------------------------------------------------------

//! \brief Implements the AT\#M2MCHDIR command and waits for OK answer
/*! \details
This command manages the M2M File System.
 * \param path    directory name
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::m2m_chdir(const char *path, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#M2MCHDIR=\"%s\""), path);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#M2MMKDIR command and waits for OK answer
/*! \details
This command manages the M2M File System.
 * \param directory_name    directory name
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::m2m_mkdir(const char *directory_name, ME310::tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#M2MMKDIR=\"%s\""), directory_name);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#M2MBACKUP command and waits for OK answer
/*! \details
This command manages the M2M File System and backup partition.
 * \param enable    set/reset the BACKUP permission
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::m2m_set_backup(int enable, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#M2MBACKUP=%d"), enable);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#M2MRMDIR command and waits for OK answer
/*! \details
This command removes a directory in the M2M file system.
 * \param directory_name    directory name
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::m2m_rmdir(const char *directory_name, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#M2MRMDIR=\"%s\""), directory_name);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+M2M command and waits for OK answer
/*! \details
This command enable/disable the M2M Application execution start mode.
 * \param mode    M2M application execution start mode
 * \param delay    M2M application execution start time-out expressed in seconds
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::m2m_application_execution(int mode, int delay, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   if(mode == 4)
   {
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+M2M=%d,%d"), mode, delay);
   }
   else
   {
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+M2M=%d"), mode);
   }
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT+M2MRUN command and waits for OK answer
/*! \details
Set command sets/resets the RUN file permission and the delay start of the executable binary files with ".bin"
extension, stored in the directory /mod. It supports the multi-app feature.
 * \param mode    set/reset the RUN file permission
 * \param file_bin    executable file name for which you set the RUN file permission
 * \param delay    s the <file_bin> delay parameter if the <file_bin> is present
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::m2m_set_run_file_permission(int mode, const char * file_bin, int delay, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+M2MRUN=%d,\"%s\",%d"), mode, delay);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#M2MDEL command and waits for OK answer
/*! \details
This command deletes specified file stored in the File System.
 * \param file_name    file name
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::m2m_delete(const char *file_name, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#M2MDEL=\"%s\""), file_name);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#M2MWRITE command and waits for OK answer
/*! \details
This command stores a file in the file system.
 * \param file_name    file name
 * \param size    file size
 * \param binToMod    if <file_name> is provided as filename with ".bin" extension, using <binToMod> set to 1, force the file to be automatically written on "/mod" folder whichever is the current directory
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::m2m_write_file(const char *file_name, int size, int binToMod, char* data, tout_t aTimeout)
{
   ME310::return_t ret;
   SET_BIT_MASK(_option, _M2MWRITE_BIT);
   memset(mBuffer, 0, ME310_BUFFSIZE);
   if(ME310_BUFFSIZE-1 < size)
   {
      ret = RETURN_ERROR;
   }
   else
   {
      if(binToMod != 0)
      {
         snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#M2MWRITE=\"%s\",%d,%d"), file_name, size, binToMod);
      }
      else
      {
         snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#M2MWRITE=\"%s\",%d"), file_name, size);
      }
      ret =  send_wait((char*)mBuffer, SEQUENCE_STRING, aTimeout);
      if ((ret == RETURN_VALID))
      {
         memset(mBuffer, 0, ME310_BUFFSIZE);
         memcpy(mBuffer, data, size);
         ret =  send_wait((char*)mBuffer, size, OK_STRING, NULL, aTimeout);
      }
   }
   UNSET_BIT_MASK(_option, _M2MWRITE_BIT);
   return ret;
}

//! \brief Implements the AT\#M2MLIST command and waits for OK answer
/*! \details
This command lists the contents of a folder in the File System.
 * \param path    directory name
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::m2m_list(const char *path, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#M2MLIST=\"%s\""), path);
   return send_wait((char*)mBuffer, 0, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#M2MLIST command and waits for OK answer
/*! \details
This command lists the contents of a folder in the File System.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::m2m_list(tout_t aTimeout)
{
   return m2m_list(".", aTimeout);
}

//! \brief Implements the AT\#M2MREAD command and waits for OK answer
/*! \details
This command reports the content of a file stored in the File System.
 * \param file_name    file name
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::m2m_read(const char *file_name, tout_t aTimeout)
{
   ME310::return_t ret;
   memset(mBuffer, 0, ME310_BUFFSIZE);
   PathParsing strPar((char*)file_name);
   m2m_list(strPar.getPath());
   char* tmp_data_raw = (char*)buffer_cstr_raw();
   int fileSize = strPar.getFileSize(tmp_data_raw);
   SET_BIT_MASK(_option, _M2MREAD_BIT);
   if(fileSize > ME310_BUFFSIZE-1)
   {
      ret = RETURN_ERROR;
   }
   else
   {
      snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#M2MREAD=\"%s\""), file_name);
      ret =  send_wait((char*)mBuffer, fileSize, OK_STRING, aTimeout);
   }
   UNSET_BIT_MASK(_option, _M2MREAD_BIT);
   return ret;
}

//! \brief Implements the AT\#M2MRAM command and waits for OK answer
/*! \details
The execution command returns information on RAM memory for AppZone applications.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::m2m_ram_info(tout_t aTimeout)
{
   return send_wait(F("AT#M2MRAM"), OK_STRING, aTimeout);
}

//! \brief Implements the AT\#M2MARG command and waits for OK answer
/*! \details
This command reports the content of a file stored in the File System.
 * \param file_bin   executable file name selected for arguments setting/resetting
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::m2m_set_arguments(const char* file_bin, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#M2MARG -=\"%s\""), file_bin);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// MQTT ------------------------------------------------------------------------

//! \brief Implements the AT\#MQEN command and waits for OK answer
/*! \details
This command initializes a MQTT client and allocates the necessary resources.
 * \param instanceNumber    selects the client instance to activate or deactivate
 * \param enable    selects if client must be activated or deactivated
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::mqtt_enable(int instanceNumber, int enable, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#MQEN=%d,%d"), instanceNumber, enable);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#MQCFG command and waits for OK answer
/*! \details
This sets the connection parameters for the selected MQTT client.
 * \param instanceNumber    selects the client instance
 * \param hostname    URL of the MQTT broker
 * \param port    TCP port of the MQTT broker
 * \param cid    PDP Context ID to be used for the connection
 * \param sslEn    enable/disable SSL
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::mqtt_configure(int instanceNumber, const char *hostname, int port, int cid, int sslEn, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#MQCFG=%d,\"%s\",%d,%d,%d"), instanceNumber, hostname, port, cid, sslEn);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#MQCFG2 command and waits for OK answer
/*! \details
This command sets the optional connection parameters for the selected MQTT client.
 * \param instanceNumber    selects the client instance
 * \param keepalive    timeout of periodic packet to keep connection open
 * \param cleanSession    Indicates whether a persistent connection is required
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::mqtt_configure_2(int instanceNumber, int keepalive, int cleanSession, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#MQCFG2=%d,%d,%d"), instanceNumber, keepalive, cleanSession);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#MQWCFG command and waits for OK answer
/*! \details
This command sets Last Will and Testament for the selected MQTT client.
 * \param instanceNumber    selects the client instance
 * \param willFlag    selects whether the client needs to specify a Last Will and Testament
 * \param willRetain    selects whether the Last Will message needs to be retained by the server
 * \param willQos    Quality of Service of the Last Will message
 * \param willTopic    Topic to publish the Last Will message to. Unused.
 * \param willMessage    Last Will message. Unused.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::mqtt_configure_lastwill_testament(int instanceNumber, int willFlag, int willRetain, int willQos, const char *willTopic, const char *willMessage, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#MQWCFG=%d,%d,%d,%d,\"%s\",\"%s\""), instanceNumber, willFlag, willRetain, willQos, willTopic, willMessage);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#MQTCFG command and waits for OK answer
/*! \details
This command writes the timeout options for the specified client.
 * \param instanceNumber    selects the client instance
 * \param pktTimeout    timeout of the packet delivery
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::mqtt_configure_timeout(int instanceNumber, int pktTimeout, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#MQTCFG=%d,%d"), instanceNumber, pktTimeout);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#MQCONN command and waits for OK answer
/*! \details
This command performs the connection and login to the MQTT broker.
 * \param instanceNumber    selects the client instance
 * \param clientId    identifies each MQTT client that connects to the MQTT broker
 * \param username    authentication and authorization
 * \param password    authentication and authorization
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::mqtt_connect(int instanceNumber, const char *clientId, const char *username, const char *password, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#MQCONN=%d,\"%s\",\"%s\",\"%s\""), instanceNumber, clientId, username, password);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#MQDISC command and waits for OK answer
/*! \details
This command performs the logout and disconnection from to the MQTT broker.
 * \param instanceNumber    selects the client instance
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::mqtt_disconnect(int instanceNumber, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#MQDISC=%d"), instanceNumber);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#MQSUB command and waits for OK answer
/*! \details
This command performs the subscription to a MQTT topic
 * \param instanceNumber    selects the client instance
 * \param topic    name of the topic
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::mqtt_topic_subscribe(int instanceNumber, const char *topic, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#MQSUB=%d,\"%s\""), instanceNumber, topic);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#MQUNS command and waits for OK answer
/*! \details
This command revokes the subscription to a MQTT topic.
 * \param instanceNumber    selects the client instance
 * \param topic    name of the topic
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::mqtt_topic_unsubscribe(int instanceNumber, const char *topic, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#MQUNS=%d,\"%s\""), instanceNumber, topic);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#MQPUBS command and waits for OK answer
/*! \details
This command publishes an ASCII string to the specified MQTT topic.
 * \param instanceNumber    selects the client instance
 * \param topic    name of the topic
 * \param retain    specifies if the broker must retain this message or not
 * \param qos    specifies the Quality of Service of this message
 * \param message    message to publish on the topic. Maximum 140 characters, printable ASCII.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::mqtt_publish(int instanceNumber, const char *topic, int retain, int qos, const char *message, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#MQPUBS=%d,\"%s\",%d,%d,\"%s\""), instanceNumber, topic, retain, qos, message);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#MQREAD command and waits for OK answer
/*! \details
This command reads the message payload from the queue slot provided.
 * \param instanceNumber    selects the client instance
 * \param mId    message slot Id to be read. The read operation will free the slot resource
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::mqtt_read(int instanceNumber, int mId, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#MQREAD=%d,%d"), instanceNumber, mId);
   return send_wait((char*)mBuffer, 0, OK_STRING, aTimeout);
}

// GNSS ------------------------------------------------------------------------

//! \brief Implements the AT$GPSCFG command and waits for OK answer
/*! \details
This command sets the following GNSS parameters: WWAN/GNSS priority, TBF (Time Between Fix) and
constellation. It needs a reboot to make effective the setting.
 * \param parameter    selects the configuration parameter to be set
 * \param value    value depends on the first parameter
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::gnss_configuration(int parameter, int value, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT$GPSCFG=%d,%d"), parameter, value);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT$GPSRST command and waits for OK answer
/*! \details
This command resets the GNSS parameters to "Factory Default" configuration.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::gnss_restore_default_GPS_parameters(tout_t aTimeout)
{
   return send_wait(F("AT$GPSRST"), OK_STRING, aTimeout);
}

//! \brief Implements the AT$GPSSAV command and waits for OK answer
/*! \details
This command stores the current GNSS parameters in the NVM of the cellular module.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::gnss_save_parameters_configuration( tout_t aTimeout)
{
   return send_wait(F("AT$GPSSAV"), OK_STRING, aTimeout);
}

//! \brief Implements the AT$GPSP command and waits for OK answer
/*! \details
This command powers on/off GNSS controller .
 * \param status    indicates the power status that has to be set
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::gnss_controller_power_management(int status, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT$GPSP=%d"), status);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT$GPSSW command and waits for OK answer
/*! \details
This command provides the GNSS module software version.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::gnss_software_version(tout_t aTimeout)
{
   return send_wait(F("AT$GPSSW"), OK_STRING, aTimeout);
}

//! \brief Implements the AT$GPSR command and waits for OK answer
/*! \details
This command resets the GNSS controller.
 * \param resetType set the type of GNSS controller reset
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::gnss_reset_GPS_controller(int resetType, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT$GPSR=%d"), resetType);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT$GPSNMUN command and waits for OK answer
/*! \details
Set command activates an unsolicited GNSS data stream built with NMEA sentences on the standard serial
port and defines which NMEA sentences will be available.
 * \param enable    Enables unsolicited GNSS data stream and selects one of the available GNSS data stream format display.
 * \param gga    enables/disables the presence of the Global Positioning System Fix Data NMEA sentence (GGA) in the GNSS data stream
 * \param gll    enable/disable the presence of the Geographic Position - Latitude/Longitude NMEA sentence (GLL) in the GNSS data stream
 * \param gsa    enable/disable the presence of the GNSS DOP and Active Satellites NMEA sentence (GSA) in the GNSS data stream
 * \param gsv    enable/disable the presence of the Satellites in View NMEA sentence (GSV) in the GNSS data stream
 * \param rmc    enable/disable the presence of the Recommended Minimum Specific GNSS Data NMEA sentence (RMC) in the GNSS data stream
 * \param vtg    enable/disable the presence of the GNSS Course Over Ground and Ground Speed NMEA sentence (VTG) in the GNSS data stream
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::gnss_nmea_data_configuration(int enable, int gga, int gll, int gsa, int gsv, int rmc, int vtg, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT$GPSNMUN=%d,%d,%d,%d,%d,%d,%d"), enable, gga, gll, gsa, gsv, rmc, vtg);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT$GPSNMUNEX command and waits for OK answer
/*! \details
Set command activates an unsolicited streaming of GNSS (or GLONASS) data (in NMEA extended format)
through the NMEA port and defines which NMEA extended sentences will be available.
 * \param gngns    Fix data of GNSS (or GLONASS) receivers.
 * \param gngsa    DOP and active satellites of GNSS (or GLONASS).
 * \param glgsv    GLONASS satellites in view
 * \param gpgrs    GPS Range Residuals
 * \param gagsv    Galileo satellites in view
 * \param gagsa    Galileo DOP and active satellites
 * \param gavtg    Galileo track made good and ground speed
 * \param gpgga    GPS fix data
 * \param pqgsa    Proprietary string for fix data regarding BeiDou and QZSS
 * \param pqgsv    Proprietary string for satellites in view regarding BeiDou and QZSS
 * \param gnvtg    Track made good and ground speed
 * \param gnrmc    Recommended Minimum Specific GNSS Data
 * \param gngga    GNSS fix data
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::gnss_nmea_extended_data_configuration(int gngns, int gngsa, int glgsv, int gpgrs, int gagsv, int gagsa, int gavtg, int gpgga, int pqgsa, int pqgsv, int gnvtg, int gnrmc, int gngga, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT$GPSNMUNEX=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"), gngns, gngsa, glgsv, gpgrs, gagsv, gagsa, gavtg, gpgga, pqgsa, pqgsv, gnvtg, gnrmc, gngga);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT#GTP command and waits for OK answer
/*! \details
Execute command returns a position based on cellular database from GTP (Global Terrestrial Positioning) WWAN service.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::get_position_GTP_WWAN_service(tout_t aTimeout)
{
   return send_wait(F("AT#GTP"), OK_STRING, aTimeout);
}

//! \brief Implements the AT#GTPENA command and waits for OK answer
/*! \details
This command returns information about the last GPS position.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::enable_GTP_WWAN_service(int enable, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#GTPENA=%d"), enable);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT$GPSACP command and waits for OK answer
/*! \details
This command returns information about the last GPS position.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::gps_get_acquired_position(tout_t aTimeout)
{
   return send_wait(F("AT$GPSACP"), OK_STRING, aTimeout);
}

//! \brief Implements the AT$AGNSS command and waits for OK answer
/*! \details
This command set the AGNSS providers enable or disable.
 * \param provider selects the AGNSS provider to be set
 * \param status set the AGNSS provider enable status
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::gnss_set_agnss_enable(int provider, int status, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT$AGNSS=%d,%d"), provider, status);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// Mobile Broadband ------------------------------------------------------------

//! \brief Implements the AT\#ECM command and waits for OK answer
/*! \details
This command sets up an Ethernet Control Model (ECM) session.
 * \param cid    context id that will be used by ECM
 * \param did    device id, currently limited to 0 (only one device)
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ecm_setup(int cid, int did, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#ECM=%d,%d"), cid, did);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#ECMD command and waits for OK answer
/*! \details
This command is used to shutdown an Ethernet Control Model (ECM) session.
 * \param did    Device id, currently limited to 0 (only one device)
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ecm_shutdown(int did, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#ECMD=%d"), did);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//IMS --------------------------------------------------------------------------

//! \brief Implements the AT+CIREG command and waits for OK answer
/*! \details
The command controls the presentation of an unsolicited result code when there is a change in the MT's IMS
registration information.
 * \param mode    enables or disables reporting of changes in the MT's IMS registration information.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::ims_registration_status(int mode, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CIREG=%d"), mode);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// PSM -------------------------------------------------------------------------

//! \brief Implements the AT+CPSMS command and waits for OK answer
/*! \details
This command enables/disables Power Saving Mode (PSM) mode.
 * \param mode    disables or enables the use of PSM in the UE
 * \param reqPeriodicRau    Requested extended periodic RAU value
 * \param reqGPRSreadyTimer    Requested GPRS READY timer value
 * \param reqPeriodicTau    Requested extended periodic TAU value
 * \param reqActiveTime    Requested Active Time value
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::psm_setting(int mode, const char *reqPeriodicRau, const char *reqGPRSreadyTimer, const char *reqPeriodicTau, const char *reqActiveTime, tout_t aTimeout)
{
   const int size_buf = 12;
   char reqPeriodicRau_tmp[size_buf] = "";
   char reqGPRSreadyTimer_tmp[size_buf] = "";
   char reqPeriodicTau_tmp[size_buf] = "";
   char reqActiveTime_tmp[size_buf] = "";
   if(strlen(reqPeriodicRau) != 0)
   {
      memset(reqPeriodicRau_tmp, 0, size_buf);
      snprintf(reqPeriodicRau_tmp, size_buf-1, F("\"%s\""), reqPeriodicRau);
   }
   if(strlen(reqGPRSreadyTimer) != 0)
   {
      memset(reqGPRSreadyTimer_tmp, 0, size_buf);
      snprintf(reqGPRSreadyTimer_tmp, size_buf-1, F("\"%s\""), reqGPRSreadyTimer);
   }
   if(strlen(reqPeriodicTau) != 0)
   {
      memset(reqPeriodicTau_tmp, 0, size_buf);
      snprintf(reqPeriodicTau_tmp, size_buf-1, F("\"%s\""), reqPeriodicTau);
   }
   if(strlen(reqActiveTime) != 0)
   {
      memset(reqActiveTime_tmp, 0, size_buf);
      snprintf(reqActiveTime_tmp, size_buf-1, F("\"%s\""), reqActiveTime);
   }
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT+CPSMS=%d,%s,%s,%s,%s"), mode, reqPeriodicRau_tmp, reqGPRSreadyTimer_tmp, reqPeriodicTau_tmp, reqActiveTime_tmp);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT\#CPSMS command and waits for OK answer
/*! \details
 This command enables/disables Power Saving Mode (PSM) mode.
 * \param mode    disables or enables the use of PSM in the UE
 * \param reqPeriodicRau    Requested extended periodic RAU value
 * \param reqGPRSreadyTimer    Requested GPRS READY timer value
 * \param reqPeriodicTau    Requested extended periodic TAU value
 * \param reqActiveTime    Requested Active Time value
 * \param psmVersion    bitmask to indicate PSM modes
 * \param psmThreshold    Minimum duration threshold (in sec) to enter PSM
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::psm_setting2(int mode, int reqPeriodicRau, int reqGPRSreadyTimer, int reqPeriodicTau, int reqActiveTime, int psmVersion, int psmThreshold, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#CPSMS=%d,%d,%d,%d,%d,%d,%d"), mode, reqPeriodicRau, reqGPRSreadyTimer, reqPeriodicTau, reqActiveTime, psmVersion, psmThreshold);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Implements the AT#PSMURC command and waits for OK answer
/*! \details
 Sets command enables/disables the URC that informs when modem entering in power saving mode.
 * \param en enable/disable URC message
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::psm_URC(int en, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#PSMURC=%d"), en);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

// Debugging -------------------------------------------------------------------

//! \brief Implements the AT#TRACE command and waits for OK answer
/*! \details
 The command selects which trace outputs you want to display through the debugging tool.
 * \param mode switches all trace outputs ON or OFF
 * \param configuration_string enables/disables a set of trace outputs.
 * \param aTimeout timeout in ms
 * \return return code
 */
ME310::return_t ME310::set_trace(int mode, char* configuration_string, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, F("AT#TRACE=%d,%s"), mode, configuration_string);
   return send_wait((char*)mBuffer, OK_STRING, aTimeout);
}

//! \brief Send the generic AT command and waits for a specific answer
/*! \details
 The command sends a generic AT command and waits for a specific answer.
 * \param aCommand command string to send
 * \param aAnswer  answer string to wait for
 * \param aTimeout answer timeout
 * \return return code
 */
ME310::return_t ME310::send_command(const char *aCommand, const char *aAnswer, tout_t aTimeout)
{
   memset(mBuffer, 0, ME310_BUFFSIZE);
   snprintf((char *)mBuffer, ME310_BUFFSIZE-1, aCommand);
   return send_wait((char*)mBuffer, aAnswer, aTimeout);
}


//! \brief Send the generic AT command, termination charater without waiting for an answer
/*! \details
 The command sends a generic AT command and waits for a specific answer.
 * \param aCommand command string to send
 * \param term  termination character (CR or LF or CRLF or other)
 * \param aTimeout answer timeout
 */
void ME310::send_data(const char *aCommand, const char* term, tout_t aTimeout)
{
  send(aCommand, term);
}
//--------------------------------------------------------------------------------------------------------------
//! \brief Returns the string by index received from the ME310 serial connection
/*!
 * \param aIndex    string index
 * \return address of the string buffer
 */
const char *ME310::buffer_cstr(int aIndex)
{
   if(mBuffLen)
   {
      const char *pBuffer = (char *)mBuffer;
      int index = 0;
      for(size_t i = 0; i < mBuffLen; i++)
      {
         if(mBuffer[i] == 0 )
         {
            if(aIndex == index++)
            {
               return pBuffer;
            }
            pBuffer = (char *)mBuffer+i+1;
         }
      }
   }
   return NULL;
}

//! \brief Returns the string received from the ME310 serial connection
/*!
 * \return address of the string buffer
 */
const char *ME310::buffer_cstr_raw()
{
   if(mBuffLen)
   {
      if(_payloadData != nullptr)
      {
         const char *pBuffer = (char*) _payloadData;
         return pBuffer;
      }
      return NULL;
   }
   return NULL;
}

//! \brief Sends a command to the ME310 serial and appends a terminator string
/*!
 * \param aCommand    command to be sent
 * \param aTerm       termination character (CR or LF or CRLF or other)
 */
void ME310::send(const char *aCommand, const char *aTerm)
{
   on_command(aCommand); //callback
   if(_debug)
   {
      Serial.print(aCommand);
      Serial.println(aTerm);
   }
   mSerial.write(aCommand);
   delay(200);
   mSerial.write(aTerm);
}

//! \brief Sends binary data to the ME310 serial
/*!
 * \param data    data buffer to be sent
 * \param len     amount of data to be written in bytes
 */
void ME310::send(const uint8_t* data, int len)
{
   on_command((char*)data); //callback
   if(_debug)
   {
      Serial.println((char*)data);
   }
   mSerial.write(data, len);
}

//! \brief Sends a read AT command and waits for answer or timeout
/*!
 * \param aCommand command string to send
 * \param aAnswer  answer string to wait for
 * \param aTimeout answer timeout
 * \return return code
 */
ME310::return_t ME310::read_send_wait(const char *aCommand, const char *aAnswer, ME310::tout_t aTimeout)
{
   send(aCommand, F("?\r"));
   return wait_for(aAnswer,aTimeout);
}

//! \brief Sends a test AT command and waits for answer or timeout
/*!
 * \param aCommand command string to send
 * \param aAnswer  answer string to wait for
 * \param aTimeout answer timeout
 * \return return code
 */
ME310::return_t ME310::test_send_wait(const char *aCommand, const char *aAnswer, ME310::tout_t aTimeout)
{
   send(aCommand, F("=?\r"));
   return wait_for(aAnswer,aTimeout);
}

//! \brief Sends an AT command and waits for answer or timeout
/*!
 * \param aCommand command string to send
 * \param aAnswer  answer string to wait for
 * \param aTimeout answer timeout
 * \return return code
 */
ME310::return_t ME310::send_wait(const char *aCommand, const char *aAnswer, ME310::tout_t aTimeout)
{
   send(aCommand,F("\r")); // send with terminator
   return wait_for(aAnswer,aTimeout);
}

//! \brief Sends an AT command and waits for answer or timeout
/*!
 * \param aCommand command string to send
 * \param aAnswer  answer string to wait for
 * \param term  termination character (CR or LF or CRLF or other)
 * \param aTimeout answer timeout
 * \return return code
 */
ME310::return_t ME310::send_wait(const char *aCommand, const char *aAnswer, const char* term, ME310::tout_t aTimeout)
{
   send(aCommand,term); // send with terminator
   return wait_for(aAnswer,aTimeout);
}
//! \brief Sends an AT command and waits for answer or timeout
/*!
 * \param flag flag to select wait for
 * \param aCommand command string to send
 * \param aAnswer  answer string to wait for
 * \param aTimeout answer timeout
 * \return return code
 */
ME310::return_t ME310::send_wait(const char *aCommand, int flag,  const char *aAnswer, ME310::tout_t aTimeout)
{
   send(aCommand,F("\r")); // send with terminator
   return wait_for(aCommand, flag, aAnswer, aTimeout);
}

//! \brief Sends an AT command and waits for answer or timeout
/*!
 * \param flag flag to select wait for
 * \param aCommand command string to send
 * \param aAnswer  answer string to wait for
 * \param term  termination character (CR or LF or CRLF or other)
 * \param aTimeout answer timeout
 * \return return code
 */
ME310::return_t ME310::send_wait(const char *aCommand, int flag,  const char *aAnswer, const char* term, ME310::tout_t aTimeout)
{
   if(term)
   {
      send(aCommand,term); // send with terminator
   }
   else
   {
      send((uint8_t*) aCommand, flag);
   }
   return wait_for(aCommand, flag, aAnswer, aTimeout);
}


//! \brief Waits for the answer to an AT command or timeouts
/*!
 * \param aAnswer  answer string to wait for
 * \param aTimeout answer timeout
 * \return return code
 */
ME310::return_t ME310::wait_for(const char *aAnswer, ME310::tout_t aTimeout)
{
   on_receive();
   mBuffLen = 0;
   mpBuffer = mBuffer;
   memset(mBuffer,0,ME310_BUFFSIZE);
   const uint8_t *pBuffer;
   unsigned long timeout = 0;
   do
   {
      if(mBuffLen != ME310_BUFFSIZE)
      {
         int bytesRead=mSerial.readBytesUntil('\n', mpBuffer, ME310_BUFFSIZE-mBuffLen-1);
         if(bytesRead>1) // if full string add to buffer
         {
            if(mpBuffer[bytesRead-1] != '\r')
               bytesRead++; // add space for 0 terminator
            mpBuffer[bytesRead-1] = 0;
            pBuffer = mpBuffer;
            mpBuffer += bytesRead;
            mBuffLen += bytesRead;
            return_t rc = on_message((const char *)pBuffer);
            if(rc != RETURN_CONTINUE)
               return rc;
            if(str_equal((const char *)pBuffer,aAnswer))
            {
               on_valid((const char *)pBuffer);
               return RETURN_VALID;
            }
            if(str_equal((const char *)pBuffer,ERROR_STRING))
            {
               on_error((const char *)pBuffer);
               return RETURN_ERROR;
            }
            if(str_start((const char *)pBuffer,CME_ERROR_STRING))
            {
               on_error((const char *)pBuffer);
               return RETURN_ERROR;
            }
         }
         else if(bytesRead == 1) // if empty string do no add to buffer
            {}
         else
            timeout+=mSerial.getTimeout();
      }
      else
      {
         on_pending_receive((const char *)pBuffer);
         mBuffLen = 0;
         mpBuffer = mBuffer;
         memset(mBuffer,0,ME310_BUFFSIZE);
      }

   }while(timeout < aTimeout);
   on_timeout();
return RETURN_TOUT;
}

//! \brief Waits for the answer to an AT command or timeouts
/*!
 * \param flag flag to wait for
 * \param aAnswer  answer string to wait for
 * \param aTimeout answer timeout
 * \return return code
 */
ME310::return_t ME310::wait_for(const char* aCommand, int flag, const char *aAnswer, ME310::tout_t aTimeout)
{
   ATCommandDataParsing* dataParsing;
   return_t rc;
   const uint8_t *pBuffer;
   int bytesRead;
   int len;
   int receivedDataLen;
   char tmp_str[ME310_BUFFSIZE];
   memset(tmp_str,0,ME310_BUFFSIZE);
   on_receive();
   mBuffLen = 0;
   mpBuffer = mBuffer;
   _payloadData = nullptr;
   memset(mBuffer,0,ME310_BUFFSIZE);
   unsigned long timeout = 0;
   do
   {
      if(mBuffLen < ME310_BUFFSIZE)
      {
         int bytesRead = mSerial.readBytes(mpBuffer, ME310_BUFFSIZE-mBuffLen-1);
         if(bytesRead > 1) /* if full string add to buffer */
         {
            mpBuffer[bytesRead+1] = '\0';
            bytesRead++;
            pBuffer = mpBuffer;
            strcat(tmp_str, (char*)mpBuffer);
            mpBuffer += bytesRead;
            mBuffLen += bytesRead;
            rc = on_message((const char *)pBuffer);
            ResponseFind rf;
            if(rf.findResponse((char*)pBuffer))
            {
               break;
            }
         }
         else if(bytesRead == 1) /* if empty string do no add to buffer */
         {}
         else
         {
            timeout+=mSerial.getTimeout();
         }
      }
      else
      {
         on_pending_receive((const char *)pBuffer);
         mBuffLen = 0;
         mpBuffer = mBuffer;
         memset(mBuffer,0,ME310_BUFFSIZE);
         memset(tmp_str,0,ME310_BUFFSIZE);
      }
   }
   while(timeout < aTimeout);
   dataParsing =  new  ATCommandDataParsing((char*)aCommand, tmp_str, flag, _option);
   if(dataParsing->parserIs())
   {
      receivedDataLen = dataParsing->receivedBytes();
      if(mBuffLen > receivedDataLen)
      {
         if(dataParsing->commandResponseResult())
         {
            _payloadData = (uint8_t *) dataParsing->extractedPayload();
            if(str_equal(dataParsing->commandResponseString(), OK_STRING))
            {
               rc = RETURN_VALID;
            }
            else if(str_equal(dataParsing->commandResponseString(), NO_CARRIER_STRING))
            {
               rc = RETURN_VALID;
            }
            else if (str_equal(dataParsing->commandResponseString(), ERROR_STRING))
            {
               rc = RETURN_ERROR;
            }
            else if (str_start(dataParsing->commandResponseString(), CME_ERROR_STRING))
            {
               rc = RETURN_ERROR;
            }
         }
         else
         {
            rc = RETURN_ERROR;
         }
      }
      else
      {
         rc = RETURN_ERROR;
      }
   }
   else
   {
      rc = RETURN_ERROR;
   }
   on_timeout();
   delete dataParsing;
   return rc;
}

//! \brief Waits for the answer to an AT command or timeout
/*!
 * \param aTimeout answer timeout
 * \return return code
 */
ME310::return_t ME310::receive_data(tout_t aTimeout)
{
   char cmd[64];
   return_t rc;
   const uint8_t *pBuffer;
   int bytesRead;
   int len;
   int receivedDataLen;
   char tmp_str[ME310_BUFFSIZE];
   memset(tmp_str,0,ME310_BUFFSIZE);
   on_receive();
   mBuffLen = 0;
   mpBuffer = mBuffer;
   _payloadData = nullptr;
   memset(mBuffer,0,ME310_BUFFSIZE);
   unsigned long timeout = 0;
   do
   {
      if(mBuffLen != ME310_BUFFSIZE)
      {
         int bytesRead = mSerial.readBytes(mpBuffer, ME310_BUFFSIZE-mBuffLen-1);
         if(bytesRead>1) // if full string add to buffer
         {
            mpBuffer[bytesRead+1] = '\0';
            bytesRead++;
            pBuffer = mpBuffer;
            strcat(tmp_str, (char*)mpBuffer);
            mpBuffer += bytesRead;
            mBuffLen += bytesRead;
            rc = on_message((const char *)pBuffer);
         }
         else if(bytesRead == 1) // if empty string do no add to buffer
         {}
         else
         {
            timeout+=mSerial.getTimeout();
         }
      }
      else
      {
         on_pending_receive((const char *)pBuffer);
         mBuffLen = 0;
         mpBuffer = mBuffer;
         memset(mBuffer,0,ME310_BUFFSIZE);
         memset(tmp_str,0,ME310_BUFFSIZE);
      }
   }while(timeout < aTimeout);
   _payloadData = (uint8_t *) tmp_str;
   on_timeout();
   return RETURN_VALID;
}

//! \brief Waits for the answer to an AT command or timeout
/*!
 * \param aTimeout answer timeout
 * \return return code
 */
ME310::return_t ME310::wait_for_unsolicited(tout_t aTimeout)
{
   return receive_data(aTimeout);
}

//! \brief Reads a line from serial until answer or timeout
/*!
 * \param aAnswer    string waiting for
 * \param aTimeout   timeout in ms
 * \return return code
 */

ME310::return_t ME310::read_line(const char *aAnswer, ME310::tout_t aTimeout)
{
   mBuffLen = 0;
   for(unsigned long timeout = 0; timeout < aTimeout; )
   {
      int bytesRead=mSerial.readBytesUntil('\n', mBuffer, ME310_BUFFSIZE-mBuffLen-1);
      if(bytesRead>0)
      {
         if(mBuffer[bytesRead-1] != '\r')
            bytesRead++; // add space for 0 terminator
         mBuffer[bytesRead-1] = 0;
         mBuffLen = bytesRead;

         return_t rc = on_message((const char *)mBuffer);
         if(rc != RETURN_CONTINUE)
         {
            return rc;
         }
         if(str_equal((const char *)mBuffer,aAnswer))
         {
            on_valid((const char *)mBuffer);
            return RETURN_VALID;
         }
         if(str_equal((const char *)mBuffer,ERROR_STRING))
         {
            on_error((const char *)mBuffer);
            return RETURN_ERROR;
         }
         if(str_start((const char *)mBuffer,CME_ERROR_STRING))
         {
            on_error((const char *)mBuffer);
            return RETURN_ERROR;
         }
         return RETURN_DATA;
      }
      else
      {
         timeout+=mSerial.getTimeout();
      }
   }
   on_timeout();
return RETURN_TOUT;
}

//! \brief Returns a string with return_t codes
/*!
 * \param  rc    return code
 * \return pointer to string
 */
const char *ME310::return_string(return_t rc)
{
   switch(rc)
   {
      case RETURN_VALID:
         return F("VALID");
         break;
      case RETURN_ERROR:
         return F("ERROR");
         break;
      case RETURN_DATA:
         return F("DATA");
         break;
      case RETURN_TOUT:
         return F("TIMEOUT");
         break;
      case RETURN_CONTINUE:
         return F("CONTINUE");
         break;
      case RETURN_ASYNC:
         return F("ASYNC");
         break;
      case RETURN_NO_CARRIER:
         return F("NO CARRIER");
         break;
      default:
         return F("CODE NOT DEFINED");
         break;
   }
}
//! \brief Compares 2 strings for inclusion from start
/*!
 * \param buffer   string buffer where to search the string
 * \param string   string to search for
 * \return return  NULL if string not included, pointer to start of string in buffer if string is included
 */
const char *ME310::str_start(const char *buffer, const char *string)
{
   if(!buffer)
   {
      return nullptr;
   }
   if(!string)
   {
      return nullptr;
   }
   const char *rc = buffer;
   for(; ;buffer++,string++)
   {
      if(*buffer != *string)
         return nullptr; // exit if different
      else if(*buffer == 0)
         return rc; // exit if equal but = 0
   }
}

//! \brief Compares 2 strings from start
/*!
 * \param buffer   string buffer where to search the string
 * \param string   string to search for
 * \return return  NULL if different, pointer to start of string in buffer if string is equal
 */
const char *ME310::str_equal(const char *buffer, const char *string)
{
   if(!buffer)
   {
      return NULL;
   }
   if(!string)
   {
       return NULL;
   }
   const char *rc = buffer;
   for(; (*buffer != 0) && (*string != 0);buffer++,string++)
   {
      if(*buffer != *string)
         return NULL;
   }
   // check terminator
   if(*buffer != *string)
   {
      return NULL;
   }
   else
   {
      return rc;
   }
}

/*! \brief Check IRA Option
/*! \details
This method checks if is IRA mode
 * \param str string of configuration command
*/
void ME310::CheckIRAOption(char* str)
{
   string tmp_str;
   tmp_str = str;
   char isIRARX[8];
   char isIRATX[8];
   std::size_t posComma = tmp_str.find_first_of(",");
   if(posComma != string::npos)
   {
      posComma = tmp_str.find_first_of(",", posComma + 1);
      if(posComma != string::npos)
      {
         std::size_t posIRARx = tmp_str.find_first_of(",", posComma + 1);
         if(posIRARx != string::npos)
         {
            int len = tmp_str.copy(isIRARX, posIRARx - (posComma + 1), posComma + 1);
            isIRARX[len] = '\0';
            int IRA_rx = atoi(isIRARX);

            posComma = posIRARx;
            posComma = tmp_str.find_first_of(",", posComma + 1);
            if(posComma != string::npos)
            {
               posComma = tmp_str.find_first_of(",", posComma + 1);
               if(posComma != string::npos)
               {
                  len = tmp_str.copy(isIRATX, tmp_str.length() - (posComma+1), posComma + 1);
                  isIRATX[len] = '\0';
                  int IRA_tx = atoi(isIRATX);
                  if(IRA_rx == 1)
                  {
                     SET_BIT_MASK(_option, _IS_IRA_RX_BIT);
                     _isIRARx = true;
                  }
                  else
                  {
                     UNSET_BIT_MASK(_option, _IS_IRA_RX_BIT);
                     _isIRARx = false;
                  }
                  if(IRA_tx == 1)
                  {
                     SET_BIT_MASK(_option, _IS_IRA_TX_BIT);
                     _isIRATx = true;
                  }
                  else
                  {
                     UNSET_BIT_MASK(_option, _IS_IRA_TX_BIT);
                     _isIRATx = false;
                  }
               }
               else
               {
                  UNSET_BIT_MASK(_option, _IS_IRA_RX_BIT);
                  UNSET_BIT_MASK(_option, _IS_IRA_TX_BIT);
                  _isIRATx = false;
                  _isIRARx = false;
               }
            }
            else
            {
               UNSET_BIT_MASK(_option, _IS_IRA_RX_BIT);
               UNSET_BIT_MASK(_option, _IS_IRA_TX_BIT);
               _isIRATx = false;
               _isIRARx = false;
            }
         }
         else
         {
            UNSET_BIT_MASK(_option, _IS_IRA_RX_BIT);
            UNSET_BIT_MASK(_option, _IS_IRA_TX_BIT);
            _isIRATx = false;
            _isIRARx = false;
         }
      }
      else
      {
         UNSET_BIT_MASK(_option, _IS_IRA_RX_BIT);
         UNSET_BIT_MASK(_option, _IS_IRA_TX_BIT);
         _isIRATx = false;
         _isIRARx = false;
      }
   }
   else
   {
      UNSET_BIT_MASK(_option, _IS_IRA_RX_BIT);
      UNSET_BIT_MASK(_option, _IS_IRA_TX_BIT);
      _isIRATx = false;
      _isIRARx = false;
   }
}

/*! \brief Convert buffer in IRA mode
/*! \details
This method converts buffer in IRA mode
 * \param recv_buf pointer of received buffer
 * \param out_buf pointer of output buffer
 * \param size size of received buffer
*/
void ME310::ConvertBufferToIRA(uint8_t* recv_buf, uint8_t* out_buf, int size)
{
    memset(out_buf, 0, (size*2)+1);
    for (unsigned int i = 0; i < size; i++)
    {
        byte nib1 = (recv_buf[i] >> 4) & 0x0F;
        byte nib2 = (recv_buf[i] >> 0) & 0x0F;
        out_buf[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        out_buf[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    out_buf[size*2] = '\0';
}

/*! \brief Convert float to string
/*! \details
This method converts float value to string
 * \param number float value
 * \param digits number of digits
 * \param buf buffer to fill with float value
 * \param size size of buffer
 * \return string value
*/
char * ME310::floatToString(double number, int digits, char *buf, int size)
{
   int _len = size;
   if (digits < 0)
      digits = 2;

   if (isnan(number))
   {
      strcpy(buf, "nan");
      return buf;
   }
   if (isinf(number))
   {
      strcpy(buf, "inf");
      return buf;
   }
   if (number > 4294967040.0) // constant determined empirically
   {
      strcpy(buf, "ovf");
      return buf;
   }
   if (number < -4294967040.0) // constant determined empirically
   {
      strcpy(buf, "ovf");
      return buf;
   }

   // Handle negative numbers
   if (number < 0.0)
   {
      strcpy(buf,"-");
      number = -number;
      _len--;
   }
   else
   {
      strcpy(buf,"");
   }

   // Round correctly so that print(1.999, 2) prints as "2.00"
   double rounding = 0.5;
   for (uint8_t i=0; i<digits; ++i)
   {
      rounding /= 10.0;
   }

   number += rounding;

   // Extract the integer part of the number and print it
   unsigned long int_part = (unsigned long)number;
   double remainder = number - (double)int_part;
   _len -= snprintf(buf + strlen(buf), _len, "%d", int_part);

   // Print the decimal point, but only if there are digits beyond
   if (digits > 0)
   {
      _len -= snprintf(buf + strlen(buf), _len, ".");
   }

   // Extract digits from the remainder one at a time
   while (digits-- > 0 && _len > 0)
   {
      remainder *= 10.0;
      unsigned int toPrint = (unsigned int)remainder;
      _len -= snprintf(buf + strlen(buf), _len, "%d", toPrint);
      remainder -= toPrint;
   }
   return buf;
}