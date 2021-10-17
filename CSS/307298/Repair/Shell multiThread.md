https://www.cnblogs.com/xudong-bupt/p/6079849.html
```shell
#!/bin/bash

function my_cmd(){
    t=$RANDOM
    t=$[t%15]
    sleep $t
    echo "sleep $t s"
}

tmp_fifofile="/tmp/$$.fifo"
mkfifo $tmp_fifofile      # 新建一个fifo类型的文件
exec 6<>$tmp_fifofile     # 将fd6指向fifo类型

thread_num=5  # 最大可同时执行线程数量
job_num=100   # 任务总数

#根据线程总数量设置令牌个数
for ((i=0;i<${thread_num};i++));do
    echo
done >&6

for ((i=0;i<${job_num};i++));do # 任务数量
    # 一个read -u6命令执行一次，就从fd6中减去一个回车符，然后向下执行，
    # fd6中没有回车符的时候，就停在这了，从而实现了线程数量控制
    read -u6

    #可以把具体的需要执行的命令封装成一个函数
    {
        my_cmd
    } &

    echo >&6 # 当进程结束以后，再向fd6中加上一个回车符，即补上了read -u6减去的那个
done

wait
exec 6>&- # 关闭fd6
rm $tmp_fifofile    #删也可以
echo "over"
```



https://zhuanlan.zhihu.com/p/68574239

```shell
#!/bin/bash
# bam to bed

start_time=`date +%s`  #定义脚本运行的开始时间

tmp_fifofile="/tmp/$$.fifo"
mkfifo $tmp_fifofile   # 新建一个FIFO类型的文件
exec 6<>$tmp_fifofile  # 将FD6指向FIFO类型
rm $tmp_fifofile  #删也可以，

thread_num=5  # 定义最大线程数

#根据线程总数量设置令牌个数
#事实上就是在fd6中放置了$thread_num个回车符
for ((i=0;i<${thread_num};i++));do
    echo
done >&6

for i in data/*.bam # 找到data文件夹下所有bam格式的文件
do
    # 一个read -u6命令执行一次，就从FD6中减去一个回车符，然后向下执行
    # 当FD6中没有回车符时，就停止，从而实现线程数量控制
    read -u6
    {
        echo "great" # 可以用实际命令代替
        echo >&6 # 当进程结束以后，再向FD6中加上一个回车符，即补上了read -u6减去的那个
    } &
done

wait # 要有wait，等待所有线程结束

stop_time=`date +%s` # 定义脚本运行的结束时间
echo "TIME:`expr $stop_time - $start_time`" # 输出脚本运行时间

exec 6>&- # 关闭FD6
echo "over" # 表示脚本运行结束
```

