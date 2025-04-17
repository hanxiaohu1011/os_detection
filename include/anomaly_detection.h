/**
 * @file anomaly_detection.h
 * @brief 操作系统指标异常检测系统头文件
 */

#ifndef ANOMALY_DETECTION_H
#define ANOMALY_DETECTION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

/* 定义指标类型 */
typedef enum {
    METRIC_CPU_USAGE,           // CPU使用率
    METRIC_CPU_IOWAIT,          // CPU IO等待
    METRIC_CPU_IRQ,             // CPU中断
    METRIC_MEM_USAGE,           // 内存使用率
    METRIC_MEM_ACTIVE,          // 活跃内存
    METRIC_DISK_READ_AWAIT,     // 磁盘读响应时间
    METRIC_DISK_WRITE_AWAIT,    // 磁盘写响应时间
    METRIC_DISK_UTIL,           // 磁盘使用率
    METRIC_NET_DROPPED,         // 网络丢包
    METRIC_COUNT                // 指标总数
} MetricType;

/* 定义指标数据结构 */
typedef struct {
    MetricType type;            // 指标类型
    char name[64];              // 指标名称
    char description[256];      // 指标描述
    double value;               // 当前值
    double threshold;           // 阈值
    double mean;                // 均值
    double stddev;              // 标准差
    double *history;            // 历史数据
    int history_size;           // 历史数据大小
    int history_capacity;       // 历史数据容量
} Metric;

/* 定义异常结构 */
typedef struct {
    MetricType type;            // 异常指标类型
    char message[256];          // 异常信息
    double value;               // 异常值
    double threshold;           // 触发阈值
    time_t timestamp;           // 时间戳
    int severity;               // 严重程度 (1-5)
} Anomaly;

/* 定义异常检测器结构 */
typedef struct {
    Metric metrics[METRIC_COUNT];   // 监控的指标
    Anomaly *anomalies;             // 检测到的异常
    int anomaly_count;              // 异常数量
    int anomaly_capacity;           // 异常容量
    int window_size;                // 滑动窗口大小
    double sigma_factor;            // N-Sigma因子
} AnomalyDetector;

/* 函数声明 */

/**
 * @brief 初始化异常检测器
 * @param detector 异常检测器指针
 * @param window_size 滑动窗口大小
 * @param sigma_factor N-Sigma因子
 * @return 成功返回0，失败返回非0
 */
int init_detector(AnomalyDetector *detector, int window_size, double sigma_factor);

/**
 * @brief 释放异常检测器资源
 * @param detector 异常检测器指针
 */
void free_detector(AnomalyDetector *detector);

/**
 * @brief 收集系统指标数据
 * @param detector 异常检测器指针
 * @return 成功返回0，失败返回非0
 */
int collect_metrics(AnomalyDetector *detector);

/**
 * @brief 使用N-Sigma算法检测异常
 * @param detector 异常检测器指针
 * @return 检测到的异常数量
 */
int detect_anomalies_nsigma(AnomalyDetector *detector);

/**
 * @brief 使用阈值检测异常
 * @param detector 异常检测器指针
 * @return 检测到的异常数量
 */
int detect_anomalies_threshold(AnomalyDetector *detector);

/**
 * @brief 打印检测到的异常
 * @param detector 异常检测器指针
 */
void print_anomalies(AnomalyDetector *detector);

/**
 * @brief 将异常保存到日志文件
 * @param detector 异常检测器指针
 * @param filename 日志文件名
 * @return 成功返回0，失败返回非0
 */
int log_anomalies(AnomalyDetector *detector, const char *filename);

/**
 * @brief 更新指标的统计信息（均值、标准差）
 * @param metric 指标指针
 */
void update_metric_stats(Metric *metric);

/**
 * @brief 添加指标数据点
 * @param metric 指标指针
 * @param value 数据值
 * @return 成功返回0，失败返回非0
 */
int add_metric_datapoint(Metric *metric, double value);

/**
 * @brief 添加异常记录
 * @param detector 异常检测器指针
 * @param type 异常指标类型
 * @param value 异常值
 * @param threshold 触发阈值
 * @param message 异常信息
 * @param severity 严重程度
 * @return 成功返回0，失败返回非0
 */
int add_anomaly(AnomalyDetector *detector, MetricType type, double value, 
                double threshold, const char *message, int severity);

#endif /* ANOMALY_DETECTION_H */
