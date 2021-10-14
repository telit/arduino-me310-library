/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
  @file
    PathParsing.h

  @brief
   Parsing string

  @details


  @version
    1.0.0

  @note
    Dependencies:
    string.h

  @author
    Cristina Desogus

  @date
    01/03/2021
*/

#ifndef __PATHPARSING__H
#define __PATHPARSING__H

/* Include files ================================================================================*/
#include <string>
/* Using namespace ================================================================================*/
using namespace std;
namespace telitAT
{

/* Class definition ================================================================================*/

/*! \class PathParsing
    \brief Implements the path parsing.
    \details
    The class implements the path parsing function.\n
    The constructor receives a string from which to extract the path, the file name and the file size. These values ​​are returned by their respective get functions as a string.
*/
  class PathParsing
  {
      public:

      PathParsing(char* str);
      char* getPath();
      char* getFilename();
      int getFileSize(char* list);
      ~PathParsing(){}

      private:
      char _path[128];    //!< Path string array
      char _filename[64]; //!< File name array
  };



  class ResponseFind
  {
    public:
      bool findResponse(char* str);
      char* getResponse(char* str);

    private:
      char _commandResponse[64];

    protected:

      static const char *OK_STRING;
      static const char *ERROR_STRING;
      static const char *CME_ERROR_STRING;
      static const char *NO_CARRIER_STRING;

  };
} //end namespace

#endif //__PATHPARSING__H