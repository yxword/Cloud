* -c xxx.cpp：将源文件编译、汇编为目标代码，xxx.o 文件
* -o file xxx.o：链接目标代码，生成可执行程序
* -Wall：输出所有编译警告
* -g：编译器编译时产生调试信息
* -O1~-O3：优化选项的级别，0没有优化，3优化级别最高
* -I<u>dir</u>：添加编译时的头文件路径dir
* -L<u>dir</u>：添加编译时的库路径dir
* -l<u>library</u>：链接时用到的库library
  * -lc：libc.so
  * -lrt：librt.so实时库
  * -ldl：libdl.so库
  * -lpthread：libpthread.so



https://www.cnblogs.com/bugutian/p/4626247.html