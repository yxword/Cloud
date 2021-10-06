* tar -zxvf  压缩包名
  * -z  gz后缀的压缩包
  * -x  解压  -c  是压缩打包
  * -v  查看过程
  * -f  必须，后面接压缩包名
  
  
  
* cp src dest
  * -a 此选项通常在复制目录时使用，它保留链接、文件属性，并复制目录下的所有内容。其作用等于dpR参数组合。
  * -r 若给出的源文件是一个目录文件，此时将复制该目录下所有的子目录和文件。
  * -p：除复制文件的内容外，还把修改时间和访问权限也复制到新文件中。
  
* grep
  -v name 过滤包含name的信息



* pidof

* ps -aux

* iperf 和 iperf3 网络性能测试工具

* stat

* [ \>/dev/null 2>&1](https://www.cnblogs.com/everest33Tong/p/11553383.html)
  符号 \> 的左边表示文件描述符， **如果没有的话表示1，也就是标准输出**

  2是错误输出，错误输出由于重用了标准输出的描述符，所以错误输出也被定向到了/dev/null中，错误输出同样也被丢弃了。执行了这条命令之后， 该条shell命令将不会输出任何信息到控制台，也不会有任何信息输出到文件中 

* wc 
  计算文件的Byte数、字数、或是行数

* ls
  -l   -r  -t  -a

* tr

  -s 缩减连续重复的字符

* df   disk free

* du  disk usage

## [系统相关](https://www.cnblogs.com/klb561/p/9157569.html)

* 查看Linux系统版本

  * cat /etc/redhat-release
  * cat /etc/issue
  * lsb_release -a
* 查看内核版本

  * uname -a
  * cat /proc/version

* 查看物理接口

  cat /proc/scsi/scsi

* 查看硬盘和分区分布

  lsblk

* 查看硬盘信息
  smartctl -i /dev/sdx

  * -H 健康
  * -a  所有信息
  * -i   基本信息

## 文本相关

* [sed](https://www.linuxprobe.com/linux-sed-command.html)

  ```shell
  sed [options] [file]
   -e ：直接在命令行模式上进行sed动作编辑，default;
   -f ：将sed的动作写在一个文件内，-f [scriptfile],以脚本中的命令参数执行
   -i ：直接修改文件内容,不输出到终端;
   -n ：只打印模式匹配的行,和p命令一起使用
   定界符：/ 或者 |
  old files old files
  old files old files
  content 123 123 111
  content 456 456 222
  hello world!
  
  # 替换每一行第一个old字符串为new
  # s表示每一行，old是要替换掉的字符串，new是替换后的字符串
  # 加上 -i，表示直接修改文本内容，不加则不会修改
  [root @svn307298 ~/tmp]# sed 's/old/new/' test.sh
  new files old files
  new files old files
  1. content 123 123
  2. content 456 456
  hello world!
  # -n 和 p命令一起，只打印发生替换的行
  [root @svn307298 ~/tmp]# sed -n 's/old/new/p' test.sh
  new files old files
  new files old files
  # 替换每一行全部的old字符串为new，并只打印发生替换的行
  # g命令 表示全部替换
  [root @svn307298 ~/tmp]# sed -n 's/old/new/pg' test.sh
  new files new files
  new files new files
  # 替换含有content字符串的行中第一个123字符为321，并只打印发生替换的行
  [root @svn307298 ~/tmp]# sed -n '/content/s/123/321/p' test.sh
  1. content 321 123
  # ...
  [root @svn307298 ~/tmp]# sed -n '/content/s/content/new content/p' test.sh
  1. new content 123 123
  2. new content 456 456
  # 每行第一个空格替换为空，即删除空格
  [root @svn307298 ~/tmp]# sed -n 's/ //p' test.sh 
  oldfiles old files
  oldfiles old files
  1.content 123 123
  2.content 456 456
  helloworld!
  # 删除files结尾的行
  # d命令删除
  [root @svn307298 ~/tmp]# sed '/files$/d' test.sh 
  1. content 123 123
  2. content 456 456
  hello world!
  # 删除hello开始的行
  [root @svn307298 ~/tmp]# sed '/^hello/d' test.sh 
  old files old files
  old files old files
  1. content 123 123
  2. content 456 456
  # 追加（行下）：a\newline
  # 插入（行上）：i\newline
  ```

* awk

* tail -f filename 读取后10行，并不断刷新 （循环读取）
  tail -n 20 filename  读取后n行

* tee
  读取标准输入，输出到文件
  -i  [file] 截断 （默认）
  -a [file] 追加
  `echo "install all tar success" | tee -i b.log`

