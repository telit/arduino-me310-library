/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
  @file
    ATCommandDataParsing.h

  @brief
   AT command data parsing

  @details
    The class implements the data parsing function for AT command which need specific response.\n
    It is possible obtain data payload, receivedBytes
  
  @version 
    1.0.0

  @note
    Dependencies: 
    Arduino.h
    string.h

  @author
    Cristina Desogus

  @date
    23/02/2021
*/

#ifndef __ATCOMMANDDATAPARSING__H
#define __ATCOMMANDDATAPARSING__H

/* Include files ================================================================================*/
#include "Arduino.h"
#include <Parser.h>
/* Using namespace ================================================================================*/
using namespace std;
using namespace telitAT;

/* Class definition ================================================================================*/

/*! \class ATCommandDataParsing
    \brief Implements the data parsing to AT command
    \details 
    The class implements the data parsing function.\n    
*/
class ATCommandDataParsing
{
    public:

    ATCommandDataParsing(char* aCommand, char* str, int flag);
    uint8_t * extractedPayload();
    int receivedBytes();
    bool commandResponseResult();
    char* commandResponseString();
    int startPositionPayloadOffset();
    bool parserIs();
    ~ATCommandDataParsing();

    private:
    
    char* findCommand(char* aCommand);

    private:

    int _exBytes;         //!< Expected bytes
    int _recvBytes;       //!< Received bytes
    char *_str;           //!< Pointer to string
    char _command[64];    //!< Array to command string
    Parser *_parser;      //!< Pointer to class parser
};

#endif //__ATCOMMANDDATAPARSING__H