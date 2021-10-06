#ifndef __INCLUDED_STRING_BUFFER_H__
#define __INCLUDED_STRING_BUFFER_H__

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>
#include <stdlib.h>
#include "Infra/Types.h"

#define BUFFER_SIZE 4096
#define FILE_HEAD_SIZE 4096

using namespace std;

class CStringBufferCreate
{
public:
    CStringBufferCreate();
    // ~CStringBufferCreate();
    bool append( const char* format, ... );
    int size(){ return m_index; }
    char* getBuffer();
private:
    char m_string_buffer[BUFFER_SIZE];
    int m_index;
};

enum REQUEST{
    UNKNOWN = 0,
    UPLOAD = 1,
    DOWNLOAD = 2
};

class CStringBufferParse
{  
public:
    CStringBufferParse();
    ~CStringBufferParse();

    

    bool parseBuffer( char* buffer );
    string filename(){ return m_filename; }
    uint64_t fileSize(){ return m_size; }
    int size(){ return m_index; }
    REQUEST request(){ return m_request; }
    //...
    
private:
    bool parseLine( char* buffer );
    void parseRequestLine( char* buffer );
    char* m_string_buffer;
    int m_index;
    string m_filename;
    uint64_t m_size;
    bool m_parse_request;
    REQUEST m_request;
};

#endif


