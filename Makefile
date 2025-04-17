CC = gcc
CFLAGS = -Wall -Wextra -g -O2
LDFLAGS = -lm

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

# 源文件和目标文件
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
TARGET = $(BIN_DIR)/anomaly_detection

# 创建目录
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

# 默认目标
all: $(TARGET)

# 链接
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# 编译
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# 清理
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# 运行
run: $(TARGET)
	$(TARGET)

# 调试
debug: $(TARGET)
	gdb $(TARGET)

# 安装
install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

# 卸载
uninstall:
	rm -f /usr/local/bin/anomaly_detection

.PHONY: all clean run debug install uninstall
