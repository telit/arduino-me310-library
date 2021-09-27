/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    PathParsing.cpp

  @brief
    PathParsing interface for AT command

  @details
    This library contains a interface that implements a parsing functions for path.\n

  @version
    1.0.0
  
  @note
    Dependencies:
    PathParsing.h

  @author
    Cristina Desogus

  @date
    02/23/2021
*/

#include "PathParsing.h"
#include <string.h>
#include "Arduino.h"

using namespace std;
namespace telitAT
{

    const char *ResponseFind::OK_STRING = "OK";                   ///< String for OK modem answer
    const char *ResponseFind::ERROR_STRING = "ERROR";             ///< String for ERROR modem answer
    const char *ResponseFind::CME_ERROR_STRING = "+CME ERROR: ";  ///< String for +CME ERROR modem answer
    const char *ResponseFind::NO_CARRIER_STRING = "NO CARRIER";   ///< String for NO CARRIER modem answer

    //! \brief Class Constructor
    /*! 
    * \param str pointer to a char string
    */
    PathParsing::PathParsing(char* str)
    {
        memset(_path, 0, 128);
        memset(_filename,0,64);
        string tmp_str;
        
        tmp_str = str;
        std::size_t found = tmp_str.find_last_of("/\\");
        if(found != string::npos)
        {
            int len = tmp_str.copy(_path, found, 0); 
            _path[len] = '\0';
            len = tmp_str.copy(_filename, tmp_str.length() - len, found + 1);
            _filename[len] = '\0';
        }
        else
        {
            int len = tmp_str.copy(_filename, tmp_str.length(), 0);
            _filename[len] = '\0';
        }
    }

    //! \brief Gets the path without file name
    /*! \details
    This method returns the path identified within the string passed to the constructor.
    /*! \return path string without file name.
    */
    char* PathParsing::getPath()
    {
        return _path;
    }

    //! \brief Gets the file name 
    /*! \details
    This method returns the file name identified within the string passed to the constructor.
    /*! \return file name string
    */
    char* PathParsing::getFilename()
    {
        return _filename;
    }

    //! \brief Gets the file size 
    /*! \details
    Returns the size of the file identified in the list. The file name is the one identified in the constructor. 
    The value of the size is an integer, if the file is not found, -1 is returned.
    * \param list list to parse.
    * \return file size
    */
    int PathParsing::getFileSize(char* list)
    {
        string str;
        str = list;
        char sizeFile[16];

        int posFilename = str.rfind(_filename);
        if(posFilename != string::npos)
        {
            int posComma =  str.find_first_of(",", posFilename);
            int posNewRow = str.find_first_of("\n", posComma);
            int len = str.copy(sizeFile, posNewRow - posComma,posComma+1); 
            sizeFile[len] = '\0';
            return atoi(sizeFile);
        }
        else
        {
            return -1;
        }
    }

    /*-------------------------------------
        Response Find
    --------------------------------------*/
    bool ResponseFind::findResponse(char* str)
    {
        string tmp_str;
        tmp_str = str;
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
        posResponse = tmp_str.find(CME_ERROR_STRING);
        if(posResponse != string::npos)
        {
            strcpy(_commandResponse, CME_ERROR_STRING);
            return true;
        }
        posResponse = tmp_str.find(NO_CARRIER_STRING);
        if(posResponse != string::npos)
        {
            strcpy(_commandResponse, NO_CARRIER_STRING);
            return true;
        }
        return false;        
    }

    char* ResponseFind::getResponse(char* str)
    {
        if(findResponse(str))
        {
            return _commandResponse;
        }           
        else
        {
            return nullptr;
        }
    }
}// end namespace telitAT
