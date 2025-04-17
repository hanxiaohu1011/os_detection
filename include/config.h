/**
 * @file config.h
 * @brief 配置文件头文件
 */

#ifndef CONFIG_H
#define CONFIG_H

/* 系统配置 */
#define DEFAULT_WINDOW_SIZE 60      // 默认滑动窗口大小（数据点数量）
#define DEFAULT_SIGMA_FACTOR 3.0    // 默认N-Sigma因子
#define DEFAULT_SAMPLING_INTERVAL 5 // 默认采样间隔（秒）
#define MAX_ANOMALIES 1000          // 最大异常记录数
#define LOG_FILE_PATH "anomalies.log" // 异常日志文件路径

/* 默认设备名 */
#define DEFAULT_DISK_DEVICE "sda"   // 默认磁盘设备
#define DEFAULT_NET_INTERFACE "eth0" // 默认网络接口

/* 指标阈值配置 */
#define CPU_USAGE_THRESHOLD 90.0    // CPU使用率阈值（%）
#define CPU_IOWAIT_THRESHOLD 20.0   // CPU IO等待阈值（%）
#define CPU_IRQ_THRESHOLD 10.0      // CPU中断阈值（%）
#define MEM_USAGE_THRESHOLD 90.0    // 内存使用率阈值（%）
#define DISK_READ_AWAIT_THRESHOLD 100.0 // 磁盘读响应时间阈值（ms）
#define DISK_WRITE_AWAIT_THRESHOLD 100.0 // 磁盘写响应时间阈值（ms）
#define DISK_UTIL_THRESHOLD 90.0    // 磁盘使用率阈值（%）
#define NET_DROPPED_THRESHOLD 100.0 // 网络丢包阈值（每秒丢包数）

#endif /* CONFIG_H */
