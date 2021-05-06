/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    Parser.cpp
    string.h

  @brief
    Parser interface for AT command

  @details
    This library contains a interface that implements a parsing functions for AT command.\n
    Parser is an abstract class and handles common methods, like gets and parse.\n
    The concrete classes, derived by Parser, implement the parsing methods for the specific AT command.\n
  @version 
    1.0.0
  
  @note
    Dependencies:
    Parser.h

  @author
    Cristina Desogus

  @date
    02/23/2021
*/

/* Include files ==================================================================================*/
#include <Parser.h>

/* Start namespace telitAT================================================================================*/
namespace telitAT
{
    const char *Parser::OK_STRING = "OK";                   ///< String for OK modem answer
    const char *Parser::ERROR_STRING = "ERROR";             ///< String for ERROR modem answer
    const char *Parser::CME_ERROR_STRING = "+CME ERROR: ";  ///< String for +CME ERROR modem answer
    const char *Parser::NO_CARRIER_STRING = "NO CARRIER";   ///< String for NO CARRIER modem answer


    //! \brief Implements the parse method.
    /*! \details
        This methods calls the methods to parser the different part of received string.
    * \param str string to parse.
    * \return 0 if the string is different from null, else -1.
    */
    int Parser::parse(char* str)
    {
        _rawData = str;
        if(_rawData != NULL)
        {
            _recvBytes = receivedBytes();
            _startPayloadOffset = findPayloadStart();
            _response = searchCommandResponseString();
            _payload = extractedData();
            return 0;
        }
        else
        {
            return -1;
        }      
    }

    //! \brief Class Destructor
    /*!
    */
    Parser::~Parser()
    {
        delete _payload;
        delete _rawData;
    }

    /*=================================*/
    /*        GET METHODS              */
    /*=================================*/

    //! \brief Gets the received bytes
    /*! \details
        This method gets the number of payload len. Before calling this method it is mandatory to call the parse() method.
    /*! \return number of received bytes
    */
    int Parser::getReceivedBytes()
    {
        return _recvBytes;
    }
    
    //! \brief Gets the payload string
    /*! \details
        This method gets payload pointer. Before calling this method it is mandatory to call the parse() method.
    /*! \return payload pointer
    */
    uint8_t * Parser::getPayload()
    {
        return _payload;
    }

    //! \brief Gets the position where the payload starts
    /*! \details
        This method gets the start position of payload. Before calling this method it is mandatory to call the parse() method.
    /*! \return number of start position of payload
    */
    int Parser::getPayloadStart()
    {
        return _startPayloadOffset;
    }

    //! \brief Gets the command response string
    /*! \details 
        This method gets command response string pointer. Before calling this method it is mandatory to call the parse() method.
    /*! \return pointer of command response string
    */
    char* Parser::getCommandResponse()
    {
        return _commandResponse;
    }

    //! \brief Gets the command response is
    /*! \details 
        This method gets a boolean value of command response, if the command response is present, return true, else return false.\n
        Before calling this method it is mandatory to call the parse() method.
    /*! \return true if the command response string is in the string, otherwise false
    */
    bool Parser::commandResponseIs()
    {
        if(searchCommandResponseString() == 0)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    /*=====================================
            SRECVParser class methods
    =======================================*/

    //! \brief Implements  the search for payload string
    /*! \details
        This method parses the string to search the payload string, it is specific for #SRECV AT command.
    /*! \return start position to payload string if the format is right otherwise return npos (-1).
    */
    int SRECVParser::findPayloadStart()
    {
        string tmp_str;
        tmp_str = _rawData;
        std::size_t strPos = tmp_str.find("#SRECV:");
        if(strPos != string::npos)
        {
            std::size_t posNewRow = tmp_str.find_first_of("\n", strPos+1);
            if(posNewRow != string::npos)
            {
                return posNewRow++;
            }
            else
            {
                return posNewRow;
            }
        }
    }

    //! \brief Implements  the search for expected bytes
    /*! \details
        This method parses the string to search the expected bytes, it is specific for #SRECV AT command.\n
        It returns the number of expected bytes which are requied in #SRECV command. It is called by parse() method.
    /*! \return number of expected bytes if the format is right otherwise return npos (-1)
    */
    int SRECVParser::expectedBytes()
    {
        string tmp_str;
        tmp_str = _rawData;
        char expBytes[32];
    
        size_t posComma = tmp_str.find_first_of(",");
        if(posComma != string::npos)
        {
            size_t posSecondComma = tmp_str.find_first_of(",", posComma+1);
            if(posSecondComma != string::npos)
            {
                int len = tmp_str.copy(expBytes, ((posSecondComma-1) - posComma), posComma+1); 
                expBytes[len] = '\0';
                return atoi(expBytes);
            }
            else
            {
                return posSecondComma;  
            }
        }
        else
        {
            return posComma;
        }
    }

    //! \brief Implements  the search for payload data
    /*! \details
        This method parses the string to search the payload data, it is specific for #SRECV AT command.\n
        It returns a pointer to payload data. 
    /*! \return pointer to payload data if the format is right otherwise return null pointer.
    */
    uint8_t * SRECVParser::extractedData()
    {
        memset(_payloadData, 0, MAX_PAYLOAD);
        string tmp_str;
        tmp_str = _rawData;
        char pl[MAX_PAYLOAD];
        int len = tmp_str.copy(pl, _recvBytes, _startPayloadOffset+1); 
        if(len != string::npos)
        {
            pl[len] = '\0';
            memcpy(_payloadData, pl, len);
            return _payloadData;
        }
        else
        {
            return nullptr;
        }
    }

    //! \brief Implements  the search for received bytes
    /*! \details
        This method parses the string to search the received bytes, it is specific for #SRECV AT command.
    /*! \return number of expected bytes if the format is right otherwise return npos (-1).
    */
    int SRECVParser::receivedBytes()
    {
        string tmp_str;
        tmp_str = _rawData;
        char recvBytes[32];
        size_t posNewRow = tmp_str.find_first_of("\n");
        if(posNewRow != string::npos)
        {
            size_t posDots = tmp_str.find_first_of(":", posNewRow+1);
            if(posDots != string::npos)
            {
                size_t posComma = tmp_str.find_first_of(",", posDots+1);
                if(posComma != string::npos)
                {
                    size_t posSecondNewRow = tmp_str.find_first_of("\n", posComma+1);
                    if(posSecondNewRow != string::npos)
                    {
                        int len = tmp_str.copy(recvBytes, ((posSecondNewRow-1) - posComma), posComma+1); 
                        recvBytes[len] = '\0';
                        return atoi(recvBytes); 
                    }
                    else
                    {
                        return posSecondNewRow;
                    }
                }  
                else
                {
                    return posComma;
                }
            }  
            else
            {
                return posDots;
            }
        }
        else
        {
            return posNewRow;
        }
    }

    //! \brief Implements  the search for command response string
    /*! \details
        This method parses the string to search the command response, it is specific for #SRECV AT command.
    /*! \return true if the format is right otherwise return false.
    */
    bool SRECVParser::searchCommandResponseString()
    { 
        memset(_commandResponse, 0, MAX_CMD_RESPONSE);
        string tmp_str;
        tmp_str = _rawData;
        int len = tmp_str.copy(_commandResponse, tmp_str.length() - (_startPayloadOffset + _recvBytes), _startPayloadOffset + _recvBytes);
       
        if(len != string::npos)
        {
            _commandResponse[len] = '\0';
            string tmp_cmd;
            tmp_cmd = _commandResponse;
            std::size_t posNewRow = tmp_cmd.find_first_of("\n");
            if(posNewRow != string::npos)
            {
                std::size_t posSecondNewRow = tmp_cmd.find_first_of("\n", posNewRow+1);
                if(posSecondNewRow != string::npos)
                {
                    len = tmp_cmd.copy(_commandResponse, ((posSecondNewRow-1) - posNewRow), posNewRow+1);
                    _commandResponse[len] = '\0';
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }


    /*====================================
            FTPRECVParser class methods
    =====================================*/

    //! \brief Implements  the search for payload string
    /*! \details
        This method parses the string to search the payload string, it is specific for #FTPRECV AT command.
    /*! \return start position to payload string if the format is right otherwise return npos (-1).
    */
    int FTPRECVParser::findPayloadStart()
    {
        string tmp_str;
        tmp_str = _rawData;
        std::size_t posNewRow = tmp_str.find_first_of("\n");
        if(posNewRow != string::npos)
        {
            std::size_t posSecondNewRow = tmp_str.find_first_of("\n", posNewRow+1);
            if(posSecondNewRow != string::npos)
            {
                return posSecondNewRow++;
            }
            else
            {
                return posSecondNewRow;
            }
        }
        else
        {
            return posNewRow;
        }
    }

    //! \brief Implements  the search for expected bytes
    /*! \details
        This method parses the string to search the expected bytes, it is specific for #FTPRECV AT command.
    /*! \return number of expected bytes if the format is right otherwise return npos (-1)
    */
    int FTPRECVParser::expectedBytes()
    {
        string tmp_str;
        tmp_str = _rawData;
        char expBytes[32];
        std::size_t posEq = tmp_str.find_first_of("=");
        if(posEq != string::npos)
        {
            std::size_t posNewRow = tmp_str.find_first_of("\n", posEq+1);
            if(posNewRow != string::npos)
            {
                int len = tmp_str.copy(expBytes, (posNewRow - posEq+1), posEq+1); 
                expBytes[len] = '\0';
                return atoi(expBytes);
            }
            else
            {
                return posNewRow;
            }
        }
        else
        {
            return posEq;
        }
    }

    //! \brief Implements  the search for payload data
    /*! \details
        This method parses the string to search the payload data, it is specific for #FTPRECV AT command.
    /*! \return pointer to payload data string if the format is right otherwise return null pointer.
    */
    uint8_t * FTPRECVParser::extractedData()
    {
        memset(_payloadData, 0, MAX_PAYLOAD);
        string tmp_str;
        tmp_str = _rawData;
        char pl[MAX_PAYLOAD];
        int len = tmp_str.copy(pl, _recvBytes-1, _startPayloadOffset+1);
        if(len != string::npos)
        {
            pl[len] = '\0';
            memcpy(_payloadData, pl, len);
            return _payloadData;
        }
        else
        {
            return nullptr;
        }
    }

    //! \brief Implements  the search for received bytes
    /*! \details
        This method parses the string to search the received bytes, it is specific for #FTPRECV AT command.
    /*! \return number of expected bytes if the format is right otherwise return npos (-1).
    */
    int FTPRECVParser::receivedBytes()
    {
        string tmp_str;
        tmp_str = _rawData;
        char recvBytes[32];

        std::size_t posNewRow = tmp_str.find_first_of("\n");
        if(posNewRow != string::npos)
        {
            std::size_t posDots = tmp_str.find_first_of(":", posNewRow+1);
            if(posDots != string::npos)
            {
                std::size_t posSecondNewRow = tmp_str.find_first_of("\n", posDots+1);
                if(posSecondNewRow != string::npos)
                {
                    int len = tmp_str.copy(recvBytes, ((posSecondNewRow-1)-posDots), posDots+1);
                    recvBytes[len] = '\0';
                    return atoi(recvBytes);
                }
                else
                {
                    return posSecondNewRow;
                }
            }
            else
            { 
                return posDots;
            }
        }
        else
        {
            return posNewRow;
        }
    }

    //! \brief Implements  the search for command response string
    /*! \details
        This method parses the string to search the command response, it is specific for #FTPRECV AT command.
    /*! \return true if the format is right otherwise return false.
    */
    bool FTPRECVParser::searchCommandResponseString()
    {
        memset(_commandResponse, 0, MAX_CMD_RESPONSE);
        string tmp_str;
        tmp_str = _rawData;
        int len = tmp_str.copy(_commandResponse, tmp_str.length() - (_startPayloadOffset + _recvBytes), _startPayloadOffset + _recvBytes); 
        if(len != string::npos)
        {
            _commandResponse[len] = '\0';
            string tmp_cmd;
            tmp_cmd = _commandResponse;
            std::size_t posResponse = tmp_str.find(OK_STRING);
            if(posResponse != string::npos)
            {
                strcpy(_commandResponse, OK_STRING);
                return true;
            }
            posResponse = tmp_str.find(ERROR_STRING);
            if(posResponse != string::npos)
            {
                strcpy(_commandResponse, ERROR_STRING);
                return true;
            }
            posResponse = tmp_str.find(NO_CARRIER_STRING);
            if(posResponse != string::npos)
            {
                strcpy(_commandResponse, NO_CARRIER_STRING);
                return true;
            }
            posResponse = tmp_str.find(CME_ERROR_STRING);
            if(posResponse != string::npos)
            {
                strcpy(_commandResponse, CME_ERROR_STRING);
                return true;
            }
            return false; 
            }
        else
        {
            return false;
        }
    }

    /*-----------------------------------
            MQREADParser class methods
    -----------------------------------*/

    //! \brief Implements  the search for payload string
    /*! \details
        This method parses the string to search the payload string, it is specific for #MQREAD AT command.
    /*! \return start position to payload string if the format is right otherwise return npos (-1).
    */
    int MQREADParser::findPayloadStart()
    {
        string tmp_str;
        tmp_str = _rawData;
        size_t posNewRow = tmp_str.find_first_of("\n");
        if(posNewRow != string::npos)
        {
            size_t posSecondNewRow = tmp_str.find_first_of("\n", posNewRow+1);
            if(posSecondNewRow != string::npos)
            {
                size_t posMaj = tmp_str.find_first_of("<", posSecondNewRow + 1);
                if(posMaj != string::npos)
                {
                    size_t posMaj2 = tmp_str.find_first_of("<", posMaj + 1);
                    if(posMaj2 != string::npos)
                    {
                        size_t posMaj3 = tmp_str.find_first_of("<", posMaj2+1);
                        if(posMaj3 != string::npos)
                        {
                            posMaj3++;
                        }
                        return posMaj3;
                    }
                    else
                    {
                        return posMaj2;
                    }
                }
                else
                {
                    return posMaj;
                }
            }
            else
            {
                return posSecondNewRow;
            }
        }
        else
        {
            return posNewRow;
        }
    }

    //! \brief Implements  the search for expected bytes
    /*! \details
        This method parses the string to search the expected bytes, it is specific for #MQREAD AT command.
    /*! \return 0
    */
    int MQREADParser::expectedBytes()
    {
        return 0;
    }

    //! \brief Implements  the search for payload data
    /*! \details
        This method parses the string to search the payload data, it is specific for #MQREAD AT command.
    /*! \return pointer to payload data string if the format is right otherwise return null pointer.
    */
    uint8_t * MQREADParser::extractedData()
    {
        memset(_payloadData, 0, MAX_PAYLOAD);
        string tmp_str;
        tmp_str = _rawData;
        char pl[MAX_PAYLOAD];
        int len = tmp_str.copy(pl, _recvBytes, _startPayloadOffset);
        if(len != string::npos)
        {
            pl[len] = '\0';
            memcpy(_payloadData, pl, len);
            return _payloadData;
        }
        else
        {
            return nullptr;
        }
    }

    //! \brief Implements  the search for received bytes
    /*! \details
        This method parses the string to search the received bytes, it is specific for #MQREAD AT command.
    /*! \return number of expected bytes if the format is right otherwise return npos (-1).
    */
    int MQREADParser::receivedBytes()
    {
        string tmp_str;
        tmp_str = _rawData;
        char recvBytes[64];
        size_t posNewRow = tmp_str.find_first_of("\n");
        if(posNewRow != string::npos)
        {
            size_t posDots = tmp_str.find_first_of(":", posNewRow+1);
            if(posDots != string::npos)
            {
                size_t posComma = tmp_str.find_first_of(",", posDots+1);
                if(posComma != string::npos)
                {
                    size_t posSecondComma = tmp_str.find_first_of(",", posComma+1);
                    if(posSecondComma != string::npos)
                    {
                        std::size_t posSecondNewRow = tmp_str.find_first_of("\n", posSecondComma+1);
                        if(posSecondNewRow != string::npos)
                        {
                            int len = tmp_str.copy(recvBytes, ((posSecondNewRow-1) - posSecondComma), posSecondComma+1); 
                            recvBytes[len] = '\0';
                            return atoi(recvBytes); 
                        }
                        else
                        {
                            return posSecondNewRow;
                        }
                    }
                    else
                    {
                        return posSecondComma;
                    }
                }
                else
                {
                    return posComma;
                }
            }
            else
            {
                return posDots;
            }
        }
        else
        {
            return posNewRow;
        }
    }

    //! \brief Implements  the search for command response string
    /*! \details
        This method parses the string to search the command response, it is specific for #MQREAD AT command.
    /*! \return true if the format is right otherwise return false.
    */
    bool MQREADParser::searchCommandResponseString()
    {
        memset(_commandResponse, 0, MAX_CMD_RESPONSE);
        string tmp_str;
        tmp_str = _rawData;
        int len = tmp_str.copy(_commandResponse, tmp_str.length() - (_startPayloadOffset + _recvBytes), _startPayloadOffset + _recvBytes);
        if(len != string::npos)
        {
            _commandResponse[len] = '\0';
            string tmp_cmd;
            tmp_cmd = _commandResponse;
            std::size_t posNewRow = tmp_cmd.find_first_of("\n");
            if(posNewRow != string::npos)
            {
                std::size_t posSecondNewRow = tmp_cmd.find_first_of("\n", posNewRow+1);
                if(posSecondNewRow != string::npos)
                {
                    len = tmp_cmd.copy(_commandResponse, ((posSecondNewRow-1) - posNewRow), posNewRow+1);
                    _commandResponse[len] = '\0';
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        return false;
    }

    /*-----------------------------------
            M2MReadParser class methods
    -----------------------------------*/

    //! \brief Implements  the search for payload string
    /*! \details
        This method parses the string to search the payload string, it is specific for #M2MREAD AT command.
    /*! \return start position to payload string if the format is right otherwise return npos (-1).
    */
    int M2MReadParser::findPayloadStart()
    {
        string tmp_str;
        tmp_str = _rawData;
        size_t posNewRow = tmp_str.find_first_of("\n");
        if(posNewRow != string::npos)
        {
            size_t posMaj = tmp_str.find_first_of("<", posNewRow + 1);
            if(posMaj != string::npos)
            {
                size_t posMaj2 = tmp_str.find_first_of("<", posMaj + 1);
                if(posMaj2 != string::npos)
                {
                    size_t posMaj3 = tmp_str.find_first_of("<", posMaj2+1);
                    if(posMaj3 != string::npos)
                    {
                        posMaj3++;
                    }
                    return posMaj3;
                }
                else
                {
                    return posMaj2;
                }
            }
            else
            {
                return posMaj;
            }
        }
        else
        {
            return posNewRow;
        }
    }

    //! \brief Implements  the search for expected bytes
    /*! \details
        This method parses the string to search the expected bytes, it is specific for #M2MREAD AT command.
    /*! \return 0
    */
    int M2MReadParser::expectedBytes()
    {
        return 0;
    }

    //! \brief Implements  the search for payload data
    /*! \details
        This method parses the string to search the payload data, it is specific for #M2MREAD AT command.
    /*! \return pointer to payload data string if the format is right otherwise return null pointer.
    */
    uint8_t * M2MReadParser::extractedData()
    {
        memset(_payloadData, 0, MAX_PAYLOAD);
        string tmp_str;
        tmp_str = _rawData;
        char pl[MAX_PAYLOAD];
        int len = tmp_str.copy(pl, _recvBytes, _startPayloadOffset);
        if(len != string::npos)
        {
            pl[len] = '\0';
            memcpy(_payloadData, pl, len);
            return _payloadData;
        }
        else
        {
            return nullptr;
        }
    }

    //! \brief Implements  the search for received bytes
    /*! \details
        This method parses the string to search the received bytes, it is specific for #M2MREAD AT command.
    /*! \return number of expected bytes if the format is right otherwise return npos (-1).
    */
    int M2MReadParser::receivedBytes()
    {
        return _fileSize;
    }

    //! \brief Implements  the search for command response string
    /*! \details
        This method parses the string to search the command response, it is specific for #M2MREAD AT command.
    /*! \return true if the format is right otherwise return false.
    */
    bool M2MReadParser::searchCommandResponseString()
    {
        memset(_commandResponse, 0, MAX_CMD_RESPONSE);
        string tmp_str;
        tmp_str = _rawData;
        int len = tmp_str.copy(_commandResponse, tmp_str.length() - (_startPayloadOffset + _recvBytes), _startPayloadOffset + _recvBytes);
        if(len != string::npos)
        {
            _commandResponse[len] = '\0';
            string tmp_cmd;
            tmp_cmd = _commandResponse;
            std::size_t posNewRow = tmp_cmd.find_first_of("\n");
            if(posNewRow != string::npos)
            {
                std::size_t posSecondNewRow = tmp_cmd.find_first_of("\n", posNewRow+1);
                if(posSecondNewRow != string::npos)
                {
                    len = tmp_cmd.copy(_commandResponse, ((posSecondNewRow-1) - posNewRow), posNewRow+1);
                    _commandResponse[len] = '\0';
                    return true;
                }
                else
                {
                    return false;
                } 
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    /*-----------------------------------
            PingParser class methods
    -----------------------------------*/

    //! \brief Implements  the search for payload string
    /*! \details
        This method parses the string to search the payload string, it is specific for #PING AT command.
    /*! \return start position to payload string if the format is right otherwise return npos (-1).
    */
    int PingParser::findPayloadStart()
    {
        string tmp_str;
        tmp_str = _rawData;
        std::size_t posNewRow = tmp_str.find_first_of("\n");
        if(posNewRow != string::npos)
        {
            return posNewRow++;
        }
        else
        {
            return posNewRow;
        }
    }

    //! \brief Implements  the search for expected bytes
    /*! \details
        This method parses the string to search the expected bytes, it is specific for #PING AT command.
        The command #PING does not contain within it the value of the bytes that it is expected to receive, always returns 0.
    /*! \return 0
    */
    int PingParser::expectedBytes()
    {
        return 0;
    }

    //! \brief Implements  the search for payload data
    /*! \details
        This method parses the string to search the payload data, it is specific for #PING AT command.
    /*! \return pointer to payload data string if the format is right otherwise return null pointer.
    */
    uint8_t * PingParser::extractedData()
    {
        memset(_payloadData, 0, MAX_PAYLOAD);
        string tmp_str;
        tmp_str = _rawData;
        char pl[MAX_PAYLOAD];
        int len = tmp_str.copy(pl, _recvBytes, _startPayloadOffset); 
        if(len != string::npos)
        {
            pl[len] = '\0';
            string tmp_pl = pl;
            std::size_t posNewRow = tmp_pl.find_first_of("\n");
            std::size_t posLastRow = tmp_pl.find_last_not_of("\n");
            if(posNewRow != string::npos && posLastRow)
            {
                string pld;
                std::size_t pos = tmp_pl.find(":");
                string tmp = tmp_pl.substr(pos+2);
                std::size_t pos1 = tmp.find("#");
                string tmp1 = tmp.substr(0, pos1);
                pld.insert(pld.length(),tmp1);
                
                while(pos != string::npos)
                {
                    pos = tmp.find(":");
                    tmp = tmp.substr(pos+2);               
                    std::size_t pos2 = tmp.find("#");
                    tmp1 = tmp.substr(0, pos2);
                    pld.insert(pld.length(),tmp1);
                    pos = tmp.find(":");
                }
                
                len = pld.copy(pl, pld.length());
                pl[len] = '\0';
                memcpy(_payloadData, pl, len);
                return _payloadData;
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }

    //! \brief Implements  the search for received bytes
    /*! \details
        This method parses the string to search the received bytes, it is specific for #PING AT command.
    /*! \return number of expected bytes if the format is right otherwise return npos (-1).
    */
    int PingParser::receivedBytes()
    {
        string tmp_str;
        tmp_str = _rawData;
        char tmp_data[MAX_PAYLOAD];
        std::size_t posNewRow = tmp_str.find_first_of("\n");
        if(posNewRow != string::npos)
        {
            std::size_t pos1NewRow = tmp_str.find_first_of("\n", posNewRow+1);
            std::size_t pos2NewRow = tmp_str.find_first_of("\n", pos1NewRow+1);
            std::size_t pos3NewRow = tmp_str.find_first_of("\n", pos2NewRow+1);
            std::size_t pos4NewRow = tmp_str.find_first_of("\n", pos3NewRow+1);
            int len = tmp_str.copy(tmp_data, pos4NewRow - posNewRow, posNewRow+1);
            return len;
        }
        else
        {
            return posNewRow;
        }
    }

    //! \brief Implements  the search for command response string
    /*! \details
        This method parses the string to search the command response, it is specific for #PING AT command.
    /*! \return true if the format is right otherwise return false.
    */
    bool PingParser::searchCommandResponseString()
    {
        memset(_commandResponse, 0, MAX_CMD_RESPONSE);
        string tmp_str;
        tmp_str = _rawData;
        int len = tmp_str.copy(_commandResponse, tmp_str.length() - (_startPayloadOffset + _recvBytes), _startPayloadOffset + _recvBytes);      
        if(len != string::npos)
        {
            _commandResponse[len] = '\0';
            string tmp_cmd;
            tmp_cmd = _commandResponse;
            std::size_t posResponse = tmp_cmd.find(OK_STRING);
            if(posResponse != string::npos)
            {
            strcpy(_commandResponse, OK_STRING);
            return true;
            }
            posResponse = tmp_cmd.find(ERROR_STRING);
            if(posResponse != string::npos)
            {
                strcpy(_commandResponse, ERROR_STRING);
                return true;
            }
            posResponse = tmp_cmd.find(CME_ERROR_STRING);
            if(posResponse != string::npos)
            {
                strcpy(_commandResponse, CME_ERROR_STRING);
                return true;
            }
        }
        else
        {
            return false;
        }
    }

    /*-----------------------------------
            GenericParser class methods
    -----------------------------------*/

    //! \brief Implements  the search for payload string
    /*! \details
        This method parses the string to search the payload string, it is specific for a generic AT command.
    /*! \return start position to payload string if the format is right otherwise return npos (-1).
    */
    int GenericParser::findPayloadStart()
    {
        string tmp_str;
        tmp_str = _rawData;
        std::size_t posNewRow = tmp_str.find_first_of("\n");
        if(posNewRow != string::npos)
        {
            posNewRow++;
            return posNewRow;
        }
        else
        {
            return posNewRow;
        }
    }

    //! \brief Implements  the search for expected bytes
    /*! \details
        This method parses the string to search the expected bytes, it is specific for a generic AT command.
    /*! \return 0
    */
    int GenericParser::expectedBytes()
    {
        return 0;
    }

    //! \brief Implements  the search for payload data
    /*! \details
        This method parses the string to search the payload data, it is specific for a generic AT command.
    /*! \return pointer to payload data string if the format is right otherwise return null pointer.
    */
    uint8_t * GenericParser::extractedData()
    {
        memset(_payloadData, 0, MAX_PAYLOAD);
        string tmp_str;
        tmp_str = _rawData;
        char pl[MAX_PAYLOAD];
        int len = tmp_str.copy(pl, _recvBytes, _startPayloadOffset);
        if(len != string::npos)
        {
            pl[len] = '\0';
            memcpy(_payloadData, pl, len);
            return _payloadData;
        }
        else
        {
            return nullptr;
        }
    }

    //! \brief Implements  the search for received bytes
    /*! \details
        This method parses the string to search the received bytes, it is specific for a generic AT command.
    /*! \return number of expected bytes if the format is right otherwise return npos (-1).
    */
    int GenericParser::receivedBytes()
    {
        string tmp_str;
        tmp_str = _rawData;
        char tmp_data[MAX_PAYLOAD];
        std::size_t posNewRow = tmp_str.find_first_of("\n");
        if(posNewRow != string::npos)
        {
            int len = tmp_str.copy(tmp_data, tmp_str.length() - posNewRow, posNewRow);
            return len;
        }
        else
        {
            return posNewRow;
        }
    }

    //! \brief Implements  the search for command response string
    /*! \details
        This method parses the string to search the command response, it is specific for a generic AT command.
    /*! \return true if the format is right otherwise return false.
    */
    bool GenericParser::searchCommandResponseString()
    {
        memset(_commandResponse, 0, MAX_CMD_RESPONSE);
        string tmp_str;
        tmp_str = _rawData;
        std::size_t posResponse = tmp_str.find(OK_STRING);
        if(posResponse != string::npos)
        {
           strcpy(_commandResponse, OK_STRING);
           _posCommandResponse = posResponse;
           return true;
        }
        posResponse = tmp_str.find(ERROR_STRING);
        if(posResponse != string::npos)
        {
            strcpy(_commandResponse, ERROR_STRING);
           _posCommandResponse = posResponse;
            return true;
        }
        posResponse = tmp_str.find(NO_CARRIER_STRING);
        if(posResponse != string::npos)
        {
            strcpy(_commandResponse, NO_CARRIER_STRING);
           _posCommandResponse = posResponse;
            return true;
        }
        posResponse = tmp_str.find(CME_ERROR_STRING);
        if(posResponse != string::npos)
        {
            strcpy(_commandResponse, CME_ERROR_STRING);
            _posCommandResponse = posResponse;
            return true;
        }
        return false;
    }
} //end namespace telitAT