/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    ATCommandDataParsing.cpp

  @brief
   AT command data parsing

  @details
    The class implements the data parsing function for AT command which need specific response.\n
    It is possible obtain data payload  

  @version
    1.1.0
  
  @note
    Dependencies:
    Arduino.h
    Parser.h

  @author

  @date
    02/23/2021
*/

#include <string.h>
#include <ATCommandDataParsing.h>

using namespace telitAT;

//!\brief Class Constructor
/*! \details 
    Calls findCommand to pars the command string and creates a specific parser class.
/*!
 * \param str string to parse
*/
ATCommandDataParsing::ATCommandDataParsing(const char* aCommand, const char* str, int flag, uint32_t option)
{
    _str = (char* )str;
    char* cmd;
    cmd = findCommand(aCommand);
    if(cmd == NULL && flag == -1)
    {
        _parser = nullptr;
    }
    else 
    {
        if(flag != 0)
        {
            _parser = new M2MReadParser(flag);
        }
        else
        {
            if(strcmp(cmd, "AT#SRECV") == 0)
            {
                _parser = new SRECVParser(option);
            }
            else if(strcmp(cmd, "AT#FTPRECV") == 0)
            {
                _parser = new FTPRECVParser();
            }
            else if(strcmp(cmd, "AT#MQREAD") == 0)
            {
                _parser = new MQREADParser();
            }
            else if(strcmp(cmd, "AT#PING") == 0)
            {
                _parser = new PingParser();
            }
            else if(strcmp(cmd, "AT+CMGL") == 0)
            {
                _parser = new SMSListParser();
            }
            else
            {
                _parser = new GenericParser();
            }
        }
        _parser->parse(_str);
    }
}

//! \brief Implements the extraction of the payload from the string
/*! \details
    Calls getPayload method to the specific parser class.
* \return payload string if parser class is different from null pointer otherwise return null.
*/
uint8_t * ATCommandDataParsing::extractedPayload()
{    
    if(_parser == nullptr)
    {
        return NULL;
    }
    return _parser->getPayload();
}

//! \brief Implements the control to parser class exists
/*! \details
    
* \return true if parser class is different from null pointer otherwise return false.
*/
bool ATCommandDataParsing::parserIs()
{
    if(_parser == nullptr)
    {
        return false;
    }
    else
    {
        return true;
    }
}
//! \brief Implements the  extraction of the received bytes from the string
/*! \details
    Calls getReceivedBytes method to the specific parser class.
* \return number of received bytes if parser class is different from null pointer otherwise return -1.
*/
int ATCommandDataParsing::receivedBytes()
{
    if(_parser == nullptr)
    {
        return -1;
    }
    return _parser->getReceivedBytes();
}
//! \brief Implements the  extraction of the command response from the string
/*! \details
    Calls commandResponseIs method to the specific parser class.
* \return true if the command response is in the string, false if the parser class is a null pointer or the command response is not in the string.
*/
bool ATCommandDataParsing::commandResponseResult()
{
    if(_parser == nullptr)
    {
        return false;
    }
    else
    {
        return _parser->commandResponseIs();
    }
}
//! \brief Implements the  extraction of the command response from the string
/*! \details
    Calls getCommandResponse method to the specific parser class.
* \return command response string if parser class is different from null pointer otherwise return null
*/
char* ATCommandDataParsing::commandResponseString()
{
    if(_parser == nullptr)
    {
        return NULL;
    }
    else
    {
        return _parser->getCommandResponse();
    }
}
//!\brief Implements the  extraction of the command response from the string
/*! \details
    Calls getPayloadStart method to the specific parser class.
* \return start position of payload if parser class is different from null pointer otherwise return -1.
*/
int ATCommandDataParsing::startPositionPayloadOffset()
{
    if(_parser == nullptr)
    {
        return -1;
    }
    else
    {
        return _parser->getPayloadStart();
    }
}
//! \brief Implements the  extraction of the command from the string
/*! \details
    Searches the string of command 
* \return command string if the format of string is right otherwise return null.
*/
char* ATCommandDataParsing::findCommand(const char* aCommand)
{
    string tmp_str;
    tmp_str = aCommand;
    memset(_command, 0, 64);
    int len; 
    std::size_t posColon = tmp_str.find_first_of("=");
    if(posColon != string::npos)
    {
        len = tmp_str.copy(_command, posColon, 0);
        _command[len] = '\0';
    }
    else
    {
        tmp_str.copy(_command, tmp_str.length(), 0);
        _command[len] = '\0';
    }
    return _command;
}

//!\brief Class Destructor
/*!
*/
ATCommandDataParsing::~ATCommandDataParsing()
{
    delete (_parser);
}