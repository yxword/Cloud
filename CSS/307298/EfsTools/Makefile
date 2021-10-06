CXX=g++
BIN=./bin/EfsTools-c++
SRCS=$(wildcard ./src/*.cpp)
OBJS=$(patsubst %.cpp,%.o ,$(SRCS))
CFLAGS=-I./include -g

.PHONY: help x86 x86_64 test_x86 test_x86_64

help:
	@echo "support following targets:"
	@echo "make x86 "
	@echo "make x86_64"
	@echo "make test_x86"
	@echo "make test_x86_64"


# 64位程序
# 定义变量 LDFLAGS=-L./libs/x86_64 -lEFSClient64 -lrt -ldl
demo: LDFLAGS=-L./libs/x86_64 -lEFSClient64 -lrt -ldl 
demo: $(OBJS)
	@echo "Generate $(BIN) ($@)"
	@$(CXX) $^ -o $(BIN) $(LDFLAGS)

# # grep -v是为了过滤一些不必要的输出
# test_x86_64:
# 	@echo "======test======"
# 	@export LD_LIBRARY_PATH=./libs/x86_64 && $(BIN) |grep -v "Src/Infra3/Thread" |grep -v "|T:"
# 	@echo "======test======"

clean:
	@rm -rf $(BIN) $(OBJS)

%.o:%.cpp
	@echo "Compile $@"
	@$(CXX) -c $^ -o $@ $(CFLAGS)
