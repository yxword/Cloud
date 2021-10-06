## Shell 编程规范

### 文件格式

1. UTF-8无BOM
2. [回车换行为UNIX风格](https://blog.csdn.net/Bleachswh/article/details/51659505)
   * Unix，每行结尾只有“<换行>”，即“\n”
   * Windows，每行结尾是“ <回车><换行>”，即“\r\n”
   * Mac，每行结尾是“<回车>”
   * getchar区别
   * ……
3. 缩进来区分代码块，缩进为4个空格，**不得使用制表符**
4. bash执行脚本，文件第一行为`#!/bin/bash`

### 命名规约

#### 脚本名

1.  脚本文件均以帕斯卡方式命名，并有`.sh`后缀

   `DnUpgrade.sh`

2.  `/etc/init.d/`目录下的脚本名以`系统名-模块名`方式命名，其中系统名为全大写方式命名，模块名以帕斯卡方式命名

   `EFS-Datanode`

### 脚本结构

1. 为便于确认执行入口，脚本的执行入口函数必须叫`main`

    ```shell
    #!/bin/bash

    function main()
    {
        do something
    }

    main "$@"
    ```

2. 除全局变量的定义和main函数的调用外，不得有其他代码位于函数体外

### 函数名

1. 函数名以驼峰方式命名（与C++规范保持一致）
   函数以`function`修饰，并在同一脚本内保持风格一致

   ```shell
   function findFile()
   ```
   

### 变量名

1. 全局变量以大写加下划线方式命名
   `EFS_DATANODE_HOME_PATH`
2. （全局变量采用模块相关前缀，防止脚本被包含时，全局变量被污染）
   `EFS_DATANODE_HOME_PATH` `EFS_MDS_HOME_PATH`
3. 局部变量以小写加下划线方式命名
   局部变量以`local`修饰，并在同一脚本内保持风格一致
   `local loop_count=10`

### 注释规约

1. 在脚本头部有对脚本功能及使用方法的注释说明

   ```shell
   # @brief 控制服务启动，停止，重启
   # @param $1 服务名称，可以是[mds|cs|dn]中的一个
   # @param $2 服务命令，可以是[start|stop|restart]中的一个 
   # @return 成功输出success，失败输出failed
   ```

2. （对功能复杂的函数增加注释，并详细介绍函数功能和入参）

3. 函数注释风格如下

   ```shell
   # @brief 打印日志
   # @param $1 日志级别
   # @param $2 模块名
   # @param $3 日志信息 
   # @return 成功返回0，失败返回1
   function log()
   {
   }
   ```

4. 行注释在所需注释代码的上一行

   ```shell
   # 输出信息到屏幕
   echo "hello world"
   ```

### 变量定义与使用

1. （全局变量定义在脚本功能注释之后，函数定义之前 ）

2. （定义常量变量时以`readonly`修饰）
   `readonly EFS_DATANODE_HOME_PATH=/cloud/dahua/EFS/Datanode`

3. 不得直接使用`$1 $2`等进行操作，需赋值给有明确含义的命名变量

   ```shell
   function log()
   {
   	local log_level=$1
   	local log_module=$2 
   	local log_message=$3
   	echo "${log_level}" "${log_module}" "${log_message}"
   }
   ```

4. （除shell特殊变量如`$1 $2 $@ $#`外，其他变量取值时都加大括号，既增加可读性，又减少出错）

   ```shell
   log="test"
   echo "${log}_level"
   ```

5. `local`修饰的局部变量值是执行命令的结果时，将声明和赋值分开两行。如果放在同一行，`$?`表示赋值是否成功，并不是命令执行退出码。

   ```shell
   function myFunc2()
   {
   	local result
   	result=$(ssh 192.168.0.1 ls)
   	# 此处 $? 表示ssh执行是否成功。
   }
   function myFunc1()
   {
   	local result=$(ssh 192.168.0.1 ls)
   	# 此处 $? 代表赋值给result是否成功，并非表示ssh执行是否成功
   }
   ```

6. （字符串变量取值时加双引号）

   ```shell
   filename="./test.txt"
   if [[ "${filename}" == "test.txt" ]];then 
   	...
   fi
   ```

### 流程控制

1. `if`中`then`放在`if`同一行

   ```shell
   if condition 1; then
   	...
   elif condition 2; then
   	...
   else
   	...
   fi
   ```

2.  `do`需另起一行

   ```shell
   for color in "red" "blue" "green"
   do
   	...
   done
   
   for (( i = 0; i < 5; i ++ ))
   do
   	...
   done
   
   while true
   do
   	...
   done
   ```

3.  `case`分支缩进四个空格，结束符`;;`单独一行

   ```shell
   case ${color} in
   	red)
   		echo "Red is selected !"
   		;;
   	*)
   		echo "Unknow is selected!"
   		;;
   esac
   ```

4. 条件判断`[]`和`[[]]`中比较运算符`=`，`!=`只能用于字符串比较；整数类型比较使用`-eq`，`-ne`等方式。

   ```shell
   if [[ "${var}" = "1" ]]; then
   	...
   fi
   
   if [[ "${var}" != "1" ]]; then
   	...
   fi
   ```

5. （条件判断使用`[[]]`。`[[]]`结构比`[]`结构更加通用，能够防止脚本中的许多逻辑错误，bash把`[[]]`的表达式看作一个单独的元素，并返回一个退出状态码。）

   ```shell
   if [[ ${var} -ne 1 && ${var} -ne 2 ]]; then
   fi
   ```

### 其他

1. （包含其他脚本，使用`source`而非`.`。`source`和`.`本质上是一样的，但是`source`更可读，减少因书写疏忽导致的错误）

   ```shell
   # 正例
   `source ./DnCommon.sh`
   
   # 反例
   # 书写疏忽如果少了'.'，使得dn_function是在子shell中执行，someFunction无法被调用
   ./DnCommon.sh
   # 调用在dn_function中的函数
   someFunction
   ```

2. （参数为空或不存在时，初始化为默认值）

   ```shell
   function myFunc()
   {
   	# 参数$1不存在时，foo默认值为1
   	# 参数$2不存在时，bar默认值为2
   	local foo=${1:-1}
   	local bar=${2:-2}
   	echo "foo : ${foo} , bar : ${bar}"
   }
   myFunc
   myFunc 3 4
   ```

3. （文件路径尽量采用相对于脚本自身路径。）

4. （执行子命令时使用`$(command)`，反引号是老用法不推荐使用。）

   ```shell
   # 正例
   # var是'\'
   var=$(echo \\)
   # 反例
   # var是'\'，需要3个'\'才能得到正确的字符，与其他场景下的使用习惯不符
   var=`echo \\\\`
   ```

   

## C++ 编程规范

### <u>文件组织规则</u>

#### 文件命名

1. 目录和文件名使用大写字母开头的单词组合
   目录名单词之间可以用空格分开

2. 文件命名时要严格区分大小写，makefile和源代码中要严格按照原文件名引用

3. 与操作系统关系密切的工程的命名可以参考操作系统的规则

   ```
   Timer.cpp	//源文件
   Timer.h	//头文件
   Font.bin	//资源文件
   Config1	//配置文件
   ```

#### 目录组织

| 目录或文件            | 说明                                                         |
| --------------------- | ------------------------------------------------------------ |
| Bin                   | 测试程序目录                                                 |
| Doc                   | 设计文档目录                                                 |
| Include               | 依赖的组件头文件目录与本组件的外部接口头文件目录，映射到其他地址 |
| Lib                   | 不同平台生成的库和依赖的库文件目录                           |
| Makefile.Configs      | Makefile的不同平台的配置文件目录                             |
| Src                   | 源文件，内部头文件                                           |
| Test                  | 单元测试代码                                                 |
| Makefile  allRules.mk | Makefile文件，一次性编译Makefile.Configs目录下所有配置对应的库，测试程序 |
| *.sln  *.vcproject    | VS解决方案文件和工程文件                                     |

#### 头文件描述

1. 源文件/头文件的头部都应进**行注释，版权申明，文件描述(说明是什么模块或什么类对应的文件)，修改记录(修改时间、svn作者、修改内容)**，可使用==辅助工具==自动生成。
2. 对于新加入svn的文件，应该其文件属性的svn属性列表中加入**(svn:keywords, Id)属性**。

#### 头文件预处理

1. 为了防止头文件被重复引用，使用ifndef/define/endif结构产生预处理块

2. 预处理宏的格式 `__<命名空间1>__...__<命名空间N>__<模块名>__文件基本名_H__`

   ```C++
   //命名空间为Renas,  Infra组件中文件名为Guard.h的预处理宏定义
   #ifndef __RENAS_INFRA_GUARD_H__
   #define __RENAS_INFRA_GUARD_H__
   #endif //__RENAS_INFRA_GUARD_H__
   ```

#### 头文件包含

1. 通过==编译器的选项==指定系统头文件包含目录

2. 通过编译器的项目属性或者Makefile的 -I 选项指定外部头文件包含目录，一般就是统一的Include目录，或者开源项目的头文件目录

3. **不允许**通过编译选项指定**内部头文件目录**

4. 包含**系统头文件**时按照标准路径使用`#include <>`包含；
   ==包含**外部头文件**目录下的头文件应该**省略所在的目录**；==
   包含**内部头文件**应该写头文件的相对路径，并使用`#include ""`包含

5. 按照标准顺序包含头文件，提高可读性，避免隐含依赖。
   比如对头文件foo2.h， 对应源文件为foo2.cpp, 则foo2.cpp中foo2.h应该放在最前，完整的头文件包含顺序如下：
   foo2.h、C system files、C++ system files、Other libraries' .h files、Your project's .h files

   ```C++
   #include "foo2.h "
   
   #include <sys/types.h>
   #include <unistd.h>
   
   #include <hash_map>
   #include <vector>
   
   #include "base/basictypes.h"
   #include "base/commandlineflags.h"
   #include "foo/public/bar.h"
   ```

6. ==头文件包含其他头文件时，要放在 namepsace外部==

7. 头文件包含其他头文件时，要以**最少原则**，避免不必要的头文件依赖，不用到的头文件不要包含，能够前向声明就不要直接包含

8. CPP 文件中引用头文件同样是最少原则，避免不必要的头文件依赖

### <u>代码组织规则</u>

#### 命名空间

1. 使用命名空间**防止定义冲突**
2. ==头文件中不要使用using namespace ***，必须写上全名==
3. ==源文件中可以使用using namespace ***，有冲突时再包含全名==

#### 分界符 ‘{’ 和 ‘}’

1. if、else、for、while等关键字必须包括分界符‘{’和‘}’；
2. 程序的分界符‘{’和‘}’应独占一行并且位于同一列，同时与引用它们的语句左对齐；
3. { }之内的代码块使用4个空格TAB缩进并对齐，换行后的代码块使用TAB缩进并对齐。

#### 空行

1. 类、结构、联合、函数、枚举等定义结束后，应加空行

2. 类定义内部相关的成员变量或操作之间不加空行，其他地方应加空行

3. 函数体内，逻揖上密切相关的语句之间不加空行，其它地方应加空行

4. 源文件和头文件末尾保留一个空行

   ```C++
   class A
   {
   public:
   	// 对某个属性的get和set操作
   	int getValue() const;
   	void setValue(int value);
   
   	// 其他操作
   	int handle();
   
   private:
   	int m_value1;
   	int m_value2;
   
   	bool m_state1;
   	bool m_state2
   };
   
   class B;
   
   ```

   

#### 换行

#### 空格

#### 就近原则

#### 运算符优先级

#### 精简原则

#### 成员顺序



### <u>命名规则</u>

#### 规则系统

#### 自描述

#### 变量

#### 函数

#### 类型

#### 宏

#### 常量



### <u>基本设计规则</u>

#### 规模

#### 函数参数

#### 函数返回值

#### 函数实现

#### 类型转换

#### 宏

#### 常量

#### 内联

#### 可重入

#### 可扩展

#### 关于goto

#### 关于sizeof



### <u>面向对象设计规则</u>

#### Struct vs Class

#### 模块（包）的划分

#### 识别类和函数

#### 构造函数

#### 析构函数

#### 拷贝构造函数与赋值运算符

#### 虚函数

#### 封装性

#### 接口设计



### <u>内存管理规则</u>

#### 模块化

#### new / delete

#### 正确性

#### 拷贝

#### 防堆栈溢出

#### 非POD类型

### <u>可移植规则</u>

#### 字符串操作

#### 大小端

#### 64位

#### 多语言



### <u>注释规则</u>

#### 有效性

#### 基本格式

#### 类型定义

#### 代码行

#### 特殊标签



### <u>Doxygen注释（建议推广）</u>

#### 基本格式

#### 定义

#### 申明

#### 模块



### <u>维护规则</u>

#### 打印输出

#### 消除警告

#### 代码修改

#### 标本兼治



### Reference



