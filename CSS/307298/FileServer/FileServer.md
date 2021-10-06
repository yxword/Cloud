* 收发两端需要定义好缓冲区大小，否则会造成发送和接收不一致
  * 举例：如发送方缓冲区大于接收方缓冲区时，当发送方调用两次Send发送buffer，接收方也调用两次Recv接受buffer，则会导致接收方的第二次Recv调用接收的是发送方第一次的Send调用。所以会造成接收方两次的handle_input









***

mysql



* 创建表

```mysql
create table xxx( id int unsigned auto_increment primary key, filename varchar(20) not null unique, size int unsigned default 0 );

mysql> desc xxx;
+----------+------------------+------+-----+---------+----------------+
| Field    | Type             | Null | Key | Default | Extra          |
+----------+------------------+------+-----+---------+----------------+
| id       | int(10) unsigned | NO   | PRI | NULL    | auto_increment |
| filename | varchar(20)      | NO   | UNI | NULL    |                |
| size     | int(10) unsigned | YES  |     | 0       |                |
+----------+------------------+------+-----+---------+----------------+

```



```mysql
# 插入
mysql> insert into test2 (filename,size) values('1.txt',0);
# 查找
mysql> select * from test2 where filename = '1.txt';
```



* c++ mysql

```
mysql_init
mysql_real_connect
mysql_query
mysql_store_result
mysql_num_fields
mysql_num_rows
mysql_fetch_row
mysql_free_result
```



```c++
#include <mysql/mysql.h>
#include <iostream>
#include <mysql/mysql.h>

using namespace std;
int main(){
    MYSQL* mysql = mysql_init( NULL );
    MYSQL_RES* res;
    MYSQL_ROW row;
    mysql = mysql_real_connect( mysql, "127.0.0.1", "root", "123456", "files_server", 3306, NULL, 0 ); // default port 3306

    if( mysql == NULL ){
        cout << "mysql == NULL" << endl;
        mysql_close( mysql );
        return 0;
    }

    // if( mysql_query( mysql, "insert test (filename, size) values('file2', 100);" ) ){
    //     cout << "query error" << mysql_error( mysql ) << endl;
    // }
    if( mysql_query( mysql, "select * from test;" ) ){
        cout << "query error" << mysql_error( mysql ) << endl;
    }
    else {
        cout << "query success" << endl;
        res = mysql_store_result( mysql );
        if( res ){
            int num_fields = mysql_num_fields( res );
            int num_rows = mysql_num_rows( res );
            for( int i = 0; i < num_rows; i++ ){
                row = mysql_fetch_row( res );
                if( row < 0 ) break;
                for( int j = 0; j < num_fields; j++ ){
                    cout << row[j] << "\t\t" ;
                }
                cout << endl;
            }
        }
        else cout << "..." << endl;
        mysql_free_result( res );   
    }
    mysql_close( mysql );

    return 0;
}
```

