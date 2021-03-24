/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    Parser.h

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
    string
    Arduino.h

  @author
    Cristina Desogus

  @date
    02/23/2021
*/

#ifndef __PARSER__H
#define __PARSER__H

/* Include files ==================================================================================*/
#include "Arduino.h"
#include <string>

/* Using namespace ================================================================================*/
using namespace std;

#define MAX_PAYLOAD 2048        //!< max payload len
#define MAX_CMD_RESPONSE 64     //!< max command response len

/* Start telitAT namespace ========================================================================*/
namespace telitAT
{
  /* Classes definitions ============================================================================*/

  /*! \class Parser
      \brief Abstract class to parse received string     
      \details
      Being an abstract class it is not possible to instantiate it. It is therefore mandatory to instantiate one of the classes derived from it.\n
      It is a interface which offers a series of methods that allow to parse a string obtained as a response following an AT command.\n
      It also offers the gets methods to access the information contained in the string. Before calling this methods, it is mandatory to call the parse() method.\n
  */
    class Parser
    {
      public:
        virtual int parse(char* str); 
        virtual int getReceivedBytes();
        virtual uint8_t * getPayload();
        virtual int getPayloadStart();
        virtual char* getCommandResponse();
        virtual bool commandResponseIs();
        virtual ~Parser();

      protected:
        virtual int findPayloadStart() = 0;
        virtual int expectedBytes() = 0;
        virtual uint8_t * extractedData() = 0;
        virtual int receivedBytes() = 0;
        virtual bool searchCommandResponseString() = 0;

        uint8_t _payloadData[MAX_PAYLOAD];          //!< Payload buffer
        char _commandResponse[MAX_CMD_RESPONSE];    //!< Command Response buffer
        uint8_t *_payload;                          //!< Pointer to payload string
        char *_rawData;                             //!< Pointer to string data received
        int _recvBytes;                             //!< Received bytes
        int _startPayloadOffset;                    //!< Start position to payload offset
        bool _response;                             //!< Command Response flag
        bool _pendingRead;                          //!< Pending read flag
    };

    /*-----------------------------------
            SRECVParser class
    -----------------------------------*/
    /*! \class SRECVParser
        \brief Class to parse received string, specific to parsing #SRECV AT command
        \details
        This is a derivated class by Parser, specific to parsing #SRECV AT command.\n
        It offers a series of methods that allow to parse a string obtained as a response following an AT#SRECV command.
    */
    class SRECVParser : public Parser
    {
      public:
        ~SRECVParser(){}

      protected:
        int findPayloadStart();
        int expectedBytes();
        uint8_t * extractedData();
        int receivedBytes();
        bool searchCommandResponseString();
    };

    /*---------------------------------
            FTPRECVParser class
    ----------------------------------*/
    /*! \class FTPRECVParser
        \brief Class to parse received string, specific to parsing #FTPRECV AT command
        \details 
        This is a derivated class by Parser, specific to parsing #FTPRECV AT command.\n
        It offers a series of methods that allow to parse a string obtained as a response following an AT#FTPRECV command.
    */
    class FTPRECVParser : public Parser
    {
      public:
        ~FTPRECVParser(){}

      protected:
        int findPayloadStart();
        int expectedBytes();
        uint8_t * extractedData();
        int receivedBytes();
        bool searchCommandResponseString();
    };

    /*-----------------------------------
            MQREADParser class
    -----------------------------------*/
    /*! \class MQREADParser
        \brief Class to parse received string, specific to parsing #MQREAD AT command
        \details 
        This is a derivated class by Parser, specific to parsing #MQREAD AT command.\n
        It offers a series of methods that allow to parse a string obtained as a response following an AT#MQREAD command.\n
        The expectedBytes() method returns always 0 value.
    */
    class MQREADParser : public Parser
    {
      public:
        ~MQREADParser(){}

      protected:
        int findPayloadStart();
        int expectedBytes();
        uint8_t * extractedData();
        int receivedBytes();
        bool searchCommandResponseString();
    };

    /*-----------------------------------
            M2MReadParser class
    -----------------------------------*/
    /*! \class M2MReadParser
        \brief Class to parse received string, specific to parsing #M2MREAD AT command
        \details 
        This is a derivated class by Parser, specific to parsing #M2MREAD AT command.\n
        It offers a series of methods that allow to parse a string obtained as a response following an AT#M2MREAD command.\n
        The expectedBytes() method returns always 0 value. The receivedBytes() method returns the file size to read.\n
    */
    class M2MReadParser : public Parser
    {
      public:
        M2MReadParser(int fileSize):_fileSize(fileSize){}
        ~M2MReadParser(){}

      protected:
        int findPayloadStart();
        int expectedBytes();
        uint8_t * extractedData();
        int receivedBytes();
        bool searchCommandResponseString();

      private:
        int _fileSize;
    };

    /*---------------------------------
            PingParser class
    ----------------------------------*/
    /*! \class PingParser
        \brief Class to parse received string, specific to parsing #PING AT command
        \details 
        This is a derivated class by Parser, specific to parsing #PING AT command.\n
        It offers a series of methods that allow to parse a string obtained as a response following an AT#PING command.\n
    */
    class PingParser : public Parser
    {
      public:
        ~PingParser(){}

      protected:            
        int findPayloadStart();
        int expectedBytes();
        uint8_t * extractedData();
        int receivedBytes();
        bool searchCommandResponseString();
    };

    /*-----------------------------------
            GenericParser class
    -----------------------------------*/
    /*! \class GenericParser
        \brief Class to parse received string, specific to parsing generic AT command
        \details 
        This is a derivated class by Parser, specific to parsing generic AT command.\n
        It offers a series of methods that allow to parse a string obtained as a response following an generic command.\n
    */
    class GenericParser : public Parser
    {
      public:
        ~GenericParser(){}

      protected:            
        int findPayloadStart();
        int expectedBytes();
        uint8_t * extractedData();
        int receivedBytes();
        bool searchCommandResponseString();

      private:

      int _posCommandResponse;
    };
} //end namespace telitAT
#endif //__PARSER__H