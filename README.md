# 操作系统指标异常检测系统

操作系统指标异常检测系统，可以监控CPU、内存、磁盘和网络等关键指标，并使用多种算法检测异常。

## 功能特点

- 实时监控操作系统关键指标
- 支持多种异常检测算法（N-Sigma、阈值检测）
- 自动生成异常报告和告警
- 可配置的检测参数和阈值
- 低资源占用，适合在操作系统内部运行

## 监控指标

系统监控以下关键指标：

- **CPU相关指标**：
  - CPU使用率
  - CPU IO等待时间
  - CPU中断时间

- **内存相关指标**：
  - 内存使用率
  - 活跃内存大小

- **磁盘I/O指标**：
  - 磁盘读响应时间
  - 磁盘写响应时间
  - 磁盘使用率

- **网络相关指标**：
  - 网络丢包数

## 编译和安装

### 依赖项

- GCC编译器
- Make工具
- 数学库（libm）

### 编译

```bash
make
```

### 安装

```bash
sudo make install
```

## 使用方法

```bash
anomaly_detection [选项]
```

### 选项

- `-h`            显示帮助信息
- `-i <秒>`       设置采样间隔（默认: 5秒）
- `-w <数量>`     设置滑动窗口大小（默认: 60个数据点）
- `-s <因子>`     设置N-Sigma因子（默认: 3.0）
- `-l <文件>`     设置日志文件路径（默认: anomalies.log）
- `-d <设备>`     设置磁盘设备名（默认: sda）
- `-n <接口>`     设置网络接口名（默认: eth0）

### 示例

```bash
# 使用默认参数运行
anomaly_detection

# 设置2秒采样间隔和30个数据点的窗口大小
anomaly_detection -i 2 -w 30

# 设置自定义日志文件
anomaly_detection -l /var/log/system_anomalies.log
```

## 异常检测算法

系统使用以下算法进行异常检测：

1. **N-Sigma算法**：基于均值和标准差的异常检测，适用于数据量较少的场景。系统计算每个指标的均值和标准差，当当前值超出均值±N倍标准差范围时，判定为异常。

2. **阈值检测**：基于预设阈值的异常检测。当指标值超过预设阈值时，判定为异常。

## 配置

系统的主要配置参数在`include/config.h`文件中定义，包括：

- 默认滑动窗口大小
- 默认N-Sigma因子
- 默认采样间隔
- 指标阈值

## 日志格式

异常日志的格式如下：

```
[时间戳] 严重程度=X, 指标=指标名, 值=当前值, 阈值=触发阈值, 消息=异常描述
```


## 许可证

本项目采用MIT许可证。
