## Makefile

[https://blog.csdn.net/afei__/article/details/82696682]

### 语法规则

* 定义变量

* 目标1：依赖1

  [tab]命令1

  [tab]命令2
  [tab]...

  目标2：依赖2
  ...

#### 目标

要生成的文件，如 xxx.o

#### 依赖

目标文件由哪些文件生成，如 xxx.；
如果依赖条件中存在不存在的依赖条件，则会寻找其它规则是否可以产生依赖条件；

#### 命令

同shell命令，**每条命令前必须有且仅有一个tab**

命令前加上 **@**，则不显示该命令，**只显示执行的结果**

#### ALL

Makefile 文件默认只生成第一个目标文件即完成编译，但是我们可以通过 “ALL” 指定需要生成的目标文件。

#### 示例

```makefile
ALL: hello.out
 
hello.out: hello.c
    gcc hello.c -o hello.out
```

```shell
$ make
gcc hello.c -o hello.out
$ ./hello.out
Hello World !
```

### 函数

#### wildcard

`SRCS=$(wildcard ./src/*.cpp)`

匹配目录下所有的 .cpp 文件，并将其赋值给 SRC 变量。

#### pastsubst

`OBJS=$(patsubst %.cpp,%.o ,$(SRCS))`

pat 是 pattern 的缩写，subst 是 substring 的缩写

意思是取出 SRC 中所有的值，然后将 “.cpp” 替换为 “.o”，最后赋值给 OBJ 变量

### 自动化变量

```makefile
SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))
 
ALL: hello.out
 
hello.out: $(OBJ)
        gcc $< -o $@

$(OBJ): $(SRC)
        gcc -c $< -o $@
```

`make hello.out` 

$@ 规则中的目标 OBJ ，如例子的hello.out

$^  规则中的所有的依赖文件，以空格分开，不包含重复的依赖文件，如例子的hello.cpp

$<  规则中的第一个依赖

### 其他

#### clean

```makefile
clean:
  @rm -rf $(BIN) $(OBJS)
```

#### 伪目标 .PHONY

make在默认情况下将目标当作是一个文件来处理，所以这里就会出现一个情况，如果makefile中的目标是实际已经存在的文件，那么，我们在使用makefile的时候，就会报错。

引入伪目标，哪怕当前目录下已经存在了一个名为clean的文件，make仍然会去执行clean目标下的命令



#### [MAKECMDGOALS](https://www.cnblogs.com/chenhuan001/p/6970686.html)

```makefile
ifeq ("$(MAKECMDGOALS)","are you ok cmdgoals")
who="you"
endif

are:
	@echo "are"
you:
	@echo "you"
ok:
	@echo "ok"
cmdgoals:
	@echo "MAKECMDGOALS: $(MAKECMDGOALS)"
	@echo "who are $(who)"
```

```shell
[...]# make are you ok cmdgoals
are
you
ok
MAKECMDGOALS: are you ok cmdgoals
who are you
```

