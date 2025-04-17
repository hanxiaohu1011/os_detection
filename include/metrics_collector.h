/**
 * @file metrics_collector.h
 * @brief 系统指标收集模块头文件
 */

#ifndef METRICS_COLLECTOR_H
#define METRICS_COLLECTOR_H

#include "anomaly_detection.h"

/**
 * @brief 从/proc文件系统读取CPU使用率
 * @param usage 存储CPU使用率的指针
 * @return 成功返回0，失败返回非0
 */
int read_cpu_usage(double *usage);

/**
 * @brief 从/proc文件系统读取CPU IO等待时间
 * @param iowait 存储IO等待时间的指针
 * @return 成功返回0，失败返回非0
 */
int read_cpu_iowait(double *iowait);

/**
 * @brief 从/proc文件系统读取CPU中断时间
 * @param irq 存储中断时间的指针
 * @return 成功返回0，失败返回非0
 */
int read_cpu_irq(double *irq);

/**
 * @brief 从/proc文件系统读取内存使用率
 * @param usage 存储内存使用率的指针
 * @return 成功返回0，失败返回非0
 */
int read_mem_usage(double *usage);

/**
 * @brief 从/proc文件系统读取活跃内存大小
 * @param active 存储活跃内存大小的指针（KB）
 * @return 成功返回0，失败返回非0
 */
int read_mem_active(double *active);

/**
 * @brief 从/proc/diskstats读取磁盘读响应时间
 * @param read_await 存储读响应时间的指针（ms）
 * @param device 磁盘设备名（如sda）
 * @return 成功返回0，失败返回非0
 */
int read_disk_read_await(double *read_await, const char *device);

/**
 * @brief 从/proc/diskstats读取磁盘写响应时间
 * @param write_await 存储写响应时间的指针（ms）
 * @param device 磁盘设备名（如sda）
 * @return 成功返回0，失败返回非0
 */
int read_disk_write_await(double *write_await, const char *device);

/**
 * @brief 从/proc/diskstats读取磁盘使用率
 * @param util 存储磁盘使用率的指针（%）
 * @param device 磁盘设备名（如sda）
 * @return 成功返回0，失败返回非0
 */
int read_disk_util(double *util, const char *device);

/**
 * @brief 从/proc/net/dev读取网络丢包数
 * @param dropped 存储丢包数的指针
 * @param interface 网络接口名（如eth0）
 * @return 成功返回0，失败返回非0
 */
int read_net_dropped(double *dropped, const char *interface);

/**
 * @brief 初始化指标收集器
 * @return 成功返回0，失败返回非0
 */
int init_metrics_collector();

/**
 * @brief 清理指标收集器资源
 */
void cleanup_metrics_collector();

#endif /* METRICS_COLLECTOR_H */
