#include "IntTypes.h"
#include "Defs.h"
#include "File.h"
#include "Bucket.h"
#include "EFileSystem.h"

#include <unistd.h>
#include <assert.h>
#include <iostream>
#include <string>
#include <fstream>

#define SLEEP_MS( X ) usleep( (X)*1000 );
using namespace std;

string inputString( bool echo = true )
{
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

    // cout << endl << str;
    // cout << endl ;
    return str;
}

void printHelp()
{
    cout << " ============ Help ============ " << endl;
    cout << " [0] create_bucket [bucket_name] " << endl;  // 创建 bucket
    cout << " [1] delete_bucket [bucket_name] " << endl;  // 删除 bucket
    cout << " [2] set_life_cycle [bucket_name] [delay] " << endl;  // 设置 bucket 生命周期
    cout << " [3] upload [bucket_name] [filename] " << endl;  // 上传文件
    cout << " [4] download [bucket_name] [filename] " << endl;  // 下载文件
    cout << " [5] delete_file [bucket_name] [filename] " << endl;  // 删除文件
    cout << " [6] traverse [bucket_name] " << endl;         // 遍历文件
    cout << " [7] search_file [bucket_name] [filename] " << endl;  // 检索文件
    cout << " [8] lock_file [bucket_name] [filename] " << endl;  // 锁定文件
    cout << " [9] unlock_file [bucket_name] [filename] " << endl;  // 解锁文件
    cout << " [10] help " << endl;
    cout << " [11] quit " << endl;
    cout << " ============================== " << endl << endl;
}

// todo according to error code :
// username pwd wrong
// ip or port
void login( Dahua::EFS::CEFileSystem* efs )
{
    while(1){
        string address, username, password;       
        Dahua::EFS::Config cfg;
        // 输入登录所需配置信息
        cout << "Login" << endl;
        
        cout << "Server IP： ";        
        // cfg.address = inputString().c_str();  // ???
        address = inputString();
        cfg.address = address.c_str();
        
        cout << "Server Port: ";        
        cfg.port = atoi( inputString().c_str() );
	    
        cout << "Username: ";
        // cfg.userName = inputString().c_str();
        username = inputString();
        cfg.userName = username.c_str();

        cout << "Password: ";
        // cfg.password = inputString( false ).c_str();
        password = inputString( false );
        cfg.password = password.c_str();


        cout << "initializing......" << endl;
        if( efs->initialize( cfg ) ){
            cout << endl << "Successfully efs initialized" << endl;
            break;
        }
        else{
            int32_t error_code = Dahua::EFS::getLastError();
            switch ( error_code )
            {
            case -60000 :
                cout << "error " << error_code << " : user not exists !" << endl;
                break;
            case -60001 :
                cout << "error " << error_code << " : wrong password !" << endl;
                break;
            // case 
            default:
                cout << "error " << error_code << " : wrong ip or port !" << endl;
                break;
            }
            // cout << "efs initialize failed error:" << Dahua::EFS::getLastError() << endl;
        }
    }
}

void login_test( Dahua::EFS::CEFileSystem* efs )
{
    Dahua::EFS::Config cfg;
	cfg.address = "192.168.30.45"; // EFS服务地址
	cfg.port = 38100; // EFS服务端口，固定不变
	cfg.userName = "yuxuwei";// 存储用户名
	cfg.password = "yuxuwei@1234"; // 存储用户密码
    if( efs->initialize( cfg ) ){
            cout << endl << "Successfully efs initialized" << endl;
    }
    else{
        cout << "efs initialize failed error:" << Dahua::EFS::getLastError() << endl;
    }
}

void createBucket( Dahua::EFS::CEFileSystem* efs, const string bucket_name )
{
    Dahua::EFS::CBucket bucket = efs->createBucket( bucket_name.c_str() );
    if( bucket.isValid() ){
        cout << "Successfully create bucket : " << bucket_name << endl;
    }
    else if( -80005 == Dahua::EFS::getLastError() ){
        cout << "create bucket failed, error : -80005 bucket already exists" << endl;
    }
    else{
        cout << "create bucket failed, error : " << Dahua::EFS::getLastError() << endl;
    }
}

void deleteBucket( Dahua::EFS::CEFileSystem* efs, const string bucket_name )
{
    Dahua::EFS::CBucket bucket = efs->getBucket( bucket_name.c_str() );
    if( bucket.isValid() ){
        if( efs->removeBucket( bucket_name.c_str() ) ){
            cout << "Successfully delete bucket " << bucket_name << endl;
        }
        else{
            cout << "delete bucket failed, error : " << Dahua::EFS::getLastError() << endl;
        }        
    }
    else{
        cout << "error : bucket is invalid " << endl;
    }
}

void setLifeCycle( Dahua::EFS::CEFileSystem* efs, const string bucket_name, const int64_t delay )
{
    Dahua::EFS::CBucket bucket = efs->getBucket( bucket_name.c_str() );
    if( bucket.isValid() ){
        bool enable_action = true;
        bool remove_when_bucket_empty = false;
        Dahua::EFS::RecycleAction act = Dahua::EFS::doDelete;
        if( bucket.setLifeCycle( delay, enable_action, remove_when_bucket_empty, act, NULL ) ){
            cout << "Successfully set bucket lifecycle delay : " << delay << endl; 
        }
        else{
            cout << "set bucket lifecycle failed, error : " << Dahua::EFS::getLastError() << endl;
        }
    }
}

// 上传文件，先读取本地的文件，再写入到EFS
// todo bucke not exsit
void upload( Dahua::EFS::CEFileSystem* efs, string bucket_name, string filename )
{
    string absolute_name = bucket_name + "/" + filename;
    int16_t N = 1, M = 1;
    bool big_file = true;
    ifstream read_file;

    read_file.open( filename.c_str() );
    if( read_file == NULL ){
        cout << "error : this file not exists, upload failed " << endl;
        return;
    }
    
    Dahua::EFS::CFile file = efs->create( absolute_name.c_str(), N, M, big_file );
    if( file.isValid() ){
        cout << "Successfully create file : " ;
        cout << N << " + " << M << endl;
        
        // 获取文件大小
        read_file.seekg( 0, ios::end );
        uint32_t want_write_len = read_file.tellg(); 
        read_file.seekg( 0, ios::beg );
        // uint32_t want_write_len = 300 << 20; // 300M
        uint32_t write_once_len = 32768*4*5; // 32K * N 的倍数
        char* buf = new char[write_once_len];
        uint32_t write_once_offset = write_once_len;
        uint32_t write_len = 0;
        while( write_len < want_write_len ){
            if( write_once_len == write_once_offset ){
                write_once_offset = 0;
                uint32_t left_write_len = want_write_len - write_len;
                write_once_len = write_once_len > left_write_len ? left_write_len : write_once_len;

                // memset( buf, 0, write_once_len );
                read_file.read( buf, write_once_len );                
            }
            int ret = file.write( buf + write_once_offset, write_once_len - write_once_offset );
            if( ret > 0 ){
                write_once_offset += ret;
                write_len += ret;
            }
            else if( ret == 0 ){
                SLEEP_MS( 1 );
            }
            else if ( ret == -1 ){
                cout << "write failed, error : " << Dahua::EFS::getLastError() << endl;
                break;
            }
            else {
                assert ( 0 );
            }
        }

        read_file.close();
        file.close();
        delete[] buf;
        buf = NULL;
        cout << "write over, write length : " << write_len << endl;        
    }
    else{
        cout << "create file failed, error : " << Dahua::EFS::getLastError() << endl;
    }
}

// 下载文件，先读取EFS上的文件，再写入到本地
void download( Dahua::EFS::CEFileSystem* efs, string bucket_name, string filename )
{
    string absolute_name = bucket_name + "/" + filename;
    Dahua::EFS::CFile file = efs->open( absolute_name.c_str(), Dahua::EFS::fileModeRead );    
    if( file.isValid() ){
        ofstream write_file;
        write_file.open( filename.c_str() ); // 默认ios::trunc
        
        // 获取服务器上的该文件大小
        Dahua::EFS::FileStat s;
        if( file.stat( s ) ){
            cout << "Successfully get file stat " << endl;
            cout << "totalSize : " << s.totalSize << endl;
        }
        else{
            cout << "get file stat failed, error : " << Dahua::EFS::getLastError() << endl;
        }
        uint32_t want_read_len = s.totalSize;
        // uint32_t want_read_len = 10  << 20; // 100M
        uint32_t read_once_len = 32768*4*5; // 32K * N 的倍数
        char* buf = new char[read_once_len];
        uint32_t read_len = 0;
        while( read_len < want_read_len ){
            int ret = file.read( buf, read_once_len );
            if( ret > 0 ){
                read_len += ret;
                uint32_t left_read_len = want_read_len - read_len;
                read_once_len = read_once_len > left_read_len ? left_read_len : read_once_len;

                write_file.write( buf, ret ); //
            }
            else if( ret == 0 ){
                SLEEP_MS( 1 );
            }
            else if( ret == -1 ){
                cout << "read failed error: " << Dahua::EFS::getLastError() << endl;
                break;
            }
            else if( ret == -2 ){
                cout << "read end" << endl;
                break;
            }
        }

        write_file.close();
        file.close();
        delete[] buf;
        buf = NULL;
        cout << "read over, read length : " << read_len << endl;
    }
    else{
        cout << "open file failed, error : " << Dahua::EFS::getLastError() << endl;
    }
    return;
}

void deleteFile( Dahua::EFS::CEFileSystem* efs, string bucket_name, string filename )
{
    string absolute_name = bucket_name + "/" + filename;
    if( efs->remove( absolute_name.c_str() ) ){
        cout << "Successfully delete file : " << absolute_name << endl;
    }
    else{
        cout << "delet file failed, error : " << Dahua::EFS::getLastError() << endl;
    }
    return;
}

void traverse( Dahua::EFS::CEFileSystem* efs, string bucket_name )
{
    Dahua::EFS::CBucket bucket = efs->getBucket( bucket_name.c_str() );
    if( bucket.isValid() ){
        cout << "Successfully get bucket : " << bucket_name << endl;
        while ( 1 )
        {
            Dahua::EFS::CNameList namelist;
            if( bucket.list( NULL, NULL, 1024, namelist ) ){
                for( int I = 0; I < namelist.size(); ++I ){
                    cout << namelist.getName( I ) << endl;
                }
                if( 1024 > namelist.size() ) break;
                // else{}
            }
            else{
                cout << "list file failed, error : " << Dahua::EFS::getLastError() << endl; 
            }
        }
    }
    else{
        cout << "get bucket failed, error : " << Dahua::EFS::getLastError() << endl;
    }
}

Dahua::EFS::CFile searchFile( Dahua::EFS::CEFileSystem* efs, string bucket_name, string filename )
{
    string absolute_name = bucket_name + "/" + filename;
    Dahua::EFS::CFile file = efs->open( absolute_name.c_str(), Dahua::EFS::fileModeRead );
    if( file.isValid() ){
        Dahua::EFS::FileStat s;
        if( file.stat( s ) ){
            cout << "Successfully get file stat" << endl;
            cout << filename << " : " << endl;
            cout << "mode : " << s.mode << endl;
            cout << "writing ? " << ( 0 == s.isAppend ? "false" : "true" ) << endl;
            cout << "N + M : " << s.dataNum << " + " << s.parityNum << endl;
            cout << "ctime : " << s.cTime << endl;
            cout << "mtime : " << s.mTime << endl;
            cout << "size : " << s.totalSize << endl;
        }
        else{
            cout << "get file stat failed, error : " << Dahua::EFS::getLastError() << endl;
        }
    }
    else{
        cout << "search file failed, error : " << Dahua::EFS::getLastError() << endl;
    }
    return file;
}

void lockFile( Dahua::EFS::CEFileSystem* efs, string bucket_name, string filename )
{
    string absolute_name = bucket_name + "/" + filename;
    if( efs->lockFile( absolute_name.c_str() ) ){
        cout << "Successfully lock file : " << absolute_name << endl;
    }
    else{
        cout << "lock file failed, error : " << Dahua::EFS::getLastError() << endl;
    }
    return;
}

void unlockFile( Dahua::EFS::CEFileSystem* efs, string bucket_name, string filename )
{
    string absolute_name = bucket_name + "/" + filename;
    if( efs->unlockFile( absolute_name.c_str() ) ){
        cout << "Successfully unlock file : " << absolute_name << endl;
    }
    else{
        cout << "unlock file failed, error : " << Dahua::EFS::getLastError() << endl;
    }
    return;
}

void listBucket( Dahua::EFS::CEFileSystem* efs )
{
    Dahua::EFS::CToken token;
    do{
        Dahua::EFS::CNameList namelist;
        if(efs->listBucket( token, 1024, namelist ) ){
            cout << "list bucket, num : " << namelist.size() << endl;
            for( int32_t I = 0; I < namelist.size(); ++I ){
                cout << namelist.getName( I ) << endl;
            }
            if( namelist.size() < 1024 ) break;
        }
        else{
            cout << "list bucket failed, error : " << Dahua::EFS::getLastError() << endl;
            break;
        }
    }while( 1 );
}

int main( int argc, char *argv[] )
{
    // 关闭输入回显和缓存
    system( "stty -echo" );
    system( "stty -icanon" ); 

    Dahua::EFS::CEFileSystem* efs = new Dahua::EFS::CEFileSystem();

    // 连接EFS
    // login( efs );
    login_test( efs );
    printHelp();
    while( 1 ){
        string selected, bucket_name, filename;
        cout << "[root@EfsTools]# ";
        selected = inputString();
        if( selected == "quit" ) break;
        if( selected == "help" ) {
            printHelp();
            continue;
        }
        if ( selected == "create_bucket" ){
            // cin >> bucket_name;
            bucket_name = inputString();
            createBucket( efs, bucket_name );
        }
        else if ( selected == "delete_bucket" ){
            // cin >> bucket_name;
            bucket_name = inputString();
            deleteBucket( efs, bucket_name );
        }
        else if ( selected == "set_life_cycle" ){
            // cin >> bucket_name;
            bucket_name = inputString();
            int64_t delay;
            //cout << "enter delay: ";
            delay = atol( inputString().c_str() );
            setLifeCycle( efs, bucket_name, delay );
        }
        else if ( selected == "upload" ){
            // cin >> bucket_name >> filename;
            bucket_name = inputString();
            filename = inputString();
            upload( efs, bucket_name, filename );
        }
        else if ( selected == "download" ){
            // cin >> bucket_name >> filename;
            bucket_name = inputString();
            filename = inputString();
            download( efs, bucket_name, filename );
        }
        else if ( selected == "traverse" ){
            // cin >> bucket_name;
            bucket_name = inputString();
            traverse( efs, bucket_name );
        }
        else if ( selected == "search_file" ){
            // cin >> bucket_name >> filename;
            bucket_name = inputString();
            filename = inputString();
            searchFile( efs, bucket_name, filename );
        }
        else if ( selected == "delete_file" ){
            // cin >> bucket_name >> filename;
            bucket_name = inputString();
            filename = inputString();
            deleteFile( efs, bucket_name, filename );
        }
        else if ( selected == "lock_file" ){
            // cin >> bucket_name >> filename;
            bucket_name = inputString();
            filename = inputString();
            lockFile( efs, bucket_name, filename );
        }
        else if ( selected == "unlock_file" ){
            // cin >> bucket_name >> filename;
            bucket_name = inputString();
            filename = inputString();
            unlockFile( efs, bucket_name, filename );
        }
        else if ( selected == "list_bucket" ){
            listBucket( efs );
        }
        else{
            cout << endl << "error : command not found !" << endl;
        }
        cout << endl;
    }

    delete efs;
    efs = NULL;


    system( "stty echo" );
    system( "stty icanon" ); 
    return 0;
}
