#ifndef __INPUT__H__
#define __INPUT__H__

#include <string>
#include <iostream>
#include <stdlib.h>

using namespace std;

class CInput
{
    public: static string inputString( bool echo = true ){
        system( "stty -echo" );
        system( "stty -icanon" );
        string str;
        char ch;

        while( ch = cin.get() ){
            if( ch == '\n' ) {
                cout << endl;
                break;
            }
            else if( ch == '\b' ){
                if( str.size() < 1 ) continue;
                str.erase( str.end() - 1 );
                cout << "\b \b" ;
            }
            else if( ch == ' ' ){
                cout << ch;
                break;
            }
            else {
                str.push_back( ch );
                cout << ( echo == true ? ch : '*' );
            }
        }
        system( "stty echo" );
        system( "stty icanon" );

        return str;
    }
};

#endif