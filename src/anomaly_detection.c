#include "../include/anomaly_detection.h"
#include "../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

int init_detector(AnomalyDetector *detector, int window_size, double sigma_factor) {
    if (!detector) {
        return -1;
    }

    // 初始化检测器参数
    detector->window_size = window_size;
    detector->sigma_factor = sigma_factor;
    detector->anomaly_count = 0;
    detector->anomaly_capacity = MAX_ANOMALIES;
    
    // 分配异常数组内存
    detector->anomalies = (Anomaly *)malloc(sizeof(Anomaly) * detector->anomaly_capacity);
    if (!detector->anomalies) {
        return -1;
    }

    // 初始化指标
    for (int i = 0; i < METRIC_COUNT; i++) {
        Metric *metric = &detector->metrics[i];
        metric->type = (MetricType)i;
        metric->history_capacity = window_size;
        metric->history_size = 0;
        metric->history = (double *)malloc(sizeof(double) * window_size);
        
        if (!metric->history) {
            free_detector(detector);
            return -1;
        }
        
        // 设置指标名称和描述
        switch (i) {
            case METRIC_CPU_USAGE:
                strcpy(metric->name, "cpu_usage");
                strcpy(metric->description, "CPU使用率(%)");
                metric->threshold = CPU_USAGE_THRESHOLD;
                break;
            case METRIC_CPU_IOWAIT:
                strcpy(metric->name, "cpu_iowait");
                strcpy(metric->description, "CPU IO等待时间(%)");
                metric->threshold = CPU_IOWAIT_THRESHOLD;
                break;
            case METRIC_CPU_IRQ:
                strcpy(metric->name, "cpu_irq");
                strcpy(metric->description, "CPU中断时间(%)");
                metric->threshold = CPU_IRQ_THRESHOLD;
                break;
            case METRIC_MEM_USAGE:
                strcpy(metric->name, "mem_usage");
                strcpy(metric->description, "内存使用率(%)");
                metric->threshold = MEM_USAGE_THRESHOLD;
                break;
            case METRIC_MEM_ACTIVE:
                strcpy(metric->name, "mem_active");
                strcpy(metric->description, "活跃内存大小(KB)");
                metric->threshold = 0; // 使用动态阈值
                break;
            case METRIC_DISK_READ_AWAIT:
                strcpy(metric->name, "disk_read_await");
                strcpy(metric->description, "磁盘读响应时间(ms)");
                metric->threshold = DISK_READ_AWAIT_THRESHOLD;
                break;
            case METRIC_DISK_WRITE_AWAIT:
                strcpy(metric->name, "disk_write_await");
                strcpy(metric->description, "磁盘写响应时间(ms)");
                metric->threshold = DISK_WRITE_AWAIT_THRESHOLD;
                break;
            case METRIC_DISK_UTIL:
                strcpy(metric->name, "disk_util");
                strcpy(metric->description, "磁盘使用率(%)");
                metric->threshold = DISK_UTIL_THRESHOLD;
                break;
            case METRIC_NET_DROPPED:
                strcpy(metric->name, "net_dropped");
                strcpy(metric->description, "网络丢包数");
                metric->threshold = NET_DROPPED_THRESHOLD;
                break;
            default:
                strcpy(metric->name, "unknown");
                strcpy(metric->description, "未知指标");
                metric->threshold = 0;
                break;
        }
    }

    return 0;
}

void free_detector(AnomalyDetector *detector) {
    if (!detector) {
        return;
    }

    // 释放指标历史数据
    for (int i = 0; i < METRIC_COUNT; i++) {
        if (detector->metrics[i].history) {
            free(detector->metrics[i].history);
            detector->metrics[i].history = NULL;
        }
    }

    // 释放异常数组
    if (detector->anomalies) {
        free(detector->anomalies);
        detector->anomalies = NULL;
    }
}

int add_metric_datapoint(Metric *metric, double value) {
    if (!metric || !metric->history) {
        return -1;
    }

    // 如果历史数据已满，移动数据
    if (metric->history_size == metric->history_capacity) {
        for (int i = 0; i < metric->history_size - 1; i++) {
            metric->history[i] = metric->history[i + 1];
        }
        metric->history[metric->history_size - 1] = value;
    } else {
        // 否则直接添加
        metric->history[metric->history_size++] = value;
    }

    // 更新当前值
    metric->value = value;

    // 更新统计信息
    update_metric_stats(metric);

    return 0;
}

void update_metric_stats(Metric *metric) {
    if (!metric || metric->history_size == 0) {
        return;
    }

    // 计算均值
    double sum = 0;
    for (int i = 0; i < metric->history_size; i++) {
        sum += metric->history[i];
    }
    metric->mean = sum / metric->history_size;

    // 计算标准差
    double variance = 0;
    for (int i = 0; i < metric->history_size; i++) {
        double diff = metric->history[i] - metric->mean;
        variance += diff * diff;
    }
    metric->stddev = sqrt(variance / metric->history_size);
}

int add_anomaly(AnomalyDetector *detector, MetricType type, double value, 
                double threshold, const char *message, int severity) {
    if (!detector || !detector->anomalies || !message) {
        return -1;
    }

    // 检查容量
    if (detector->anomaly_count >= detector->anomaly_capacity) {
        // 扩展容量
        int new_capacity = detector->anomaly_capacity * 2;
        Anomaly *new_anomalies = (Anomaly *)realloc(detector->anomalies, 
                                                   sizeof(Anomaly) * new_capacity);
        if (!new_anomalies) {
            return -1;
        }
        detector->anomalies = new_anomalies;
        detector->anomaly_capacity = new_capacity;
    }

    // 添加异常
    Anomaly *anomaly = &detector->anomalies[detector->anomaly_count++];
    anomaly->type = type;
    anomaly->value = value;
    anomaly->threshold = threshold;
    strncpy(anomaly->message, message, sizeof(anomaly->message) - 1);
    anomaly->message[sizeof(anomaly->message) - 1] = '\0';
    anomaly->timestamp = time(NULL);
    anomaly->severity = severity;

    return 0;
}

int detect_anomalies_nsigma(AnomalyDetector *detector) {
    if (!detector) {
        return -1;
    }

    int anomalies_detected = 0;

    // 遍历所有指标
    for (int i = 0; i < METRIC_COUNT; i++) {
        Metric *metric = &detector->metrics[i];
        
        // 需要足够的历史数据
        if (metric->history_size < 3) {
            continue;
        }

        // 计算上下限
        double upper_bound = metric->mean + detector->sigma_factor * metric->stddev;
        double lower_bound = metric->mean - detector->sigma_factor * metric->stddev;

        // 检测异常
        if (metric->value > upper_bound) {
            char message[256];
            snprintf(message, sizeof(message), 
                    "%s 异常偏高: %.2f > %.2f (均值: %.2f, 标准差: %.2f)",
                    metric->description, metric->value, upper_bound, 
                    metric->mean, metric->stddev);
            
            // 计算严重程度 (1-5)
            int severity = (int)(((metric->value - upper_bound) / upper_bound) * 5) + 1;
            if (severity > 5) severity = 5;
            
            add_anomaly(detector, metric->type, metric->value, upper_bound, 
                       message, severity);
            anomalies_detected++;
        } 
        else if (metric->value < lower_bound && lower_bound > 0) {
            char message[256];
            snprintf(message, sizeof(message), 
                    "%s 异常偏低: %.2f < %.2f (均值: %.2f, 标准差: %.2f)",
                    metric->description, metric->value, lower_bound, 
                    metric->mean, metric->stddev);
            
            // 计算严重程度 (1-5)
            int severity = (int)(((lower_bound - metric->value) / lower_bound) * 5) + 1;
            if (severity > 5) severity = 5;
            
            add_anomaly(detector, metric->type, metric->value, lower_bound, 
                       message, severity);
            anomalies_detected++;
        }
    }

    return anomalies_detected;
}

int detect_anomalies_threshold(AnomalyDetector *detector) {
    if (!detector) {
        return -1;
    }

    int anomalies_detected = 0;

    // 遍历所有指标
    for (int i = 0; i < METRIC_COUNT; i++) {
        Metric *metric = &detector->metrics[i];
        
        // 跳过没有设置阈值的指标
        if (metric->threshold <= 0) {
            continue;
        }

        // 检测异常
        if (metric->value > metric->threshold) {
            char message[256];
            snprintf(message, sizeof(message), 
                    "%s 超过阈值: %.2f > %.2f",
                    metric->description, metric->value, metric->threshold);
            
            // 计算严重程度 (1-5)
            int severity = (int)(((metric->value - metric->threshold) / metric->threshold) * 5) + 1;
            if (severity > 5) severity = 5;
            
            add_anomaly(detector, metric->type, metric->value, metric->threshold, 
                       message, severity);
            anomalies_detected++;
        }
    }

    return anomalies_detected;
}

void print_anomalies(AnomalyDetector *detector) {
    if (!detector || !detector->anomalies) {
        return;
    }

    if (detector->anomaly_count == 0) {
        printf("没有检测到异常\n");
        return;
    }

    printf("检测到 %d 个异常:\n", detector->anomaly_count);
    printf("---------------------------------------------------\n");
    
    for (int i = 0; i < detector->anomaly_count; i++) {
        Anomaly *anomaly = &detector->anomalies[i];
        char time_str[64];
        struct tm *tm_info = localtime(&anomaly->timestamp);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
        
        printf("异常 #%d:\n", i + 1);
        printf("  时间: %s\n", time_str);
        printf("  指标: %s\n", detector->metrics[anomaly->type].name);
        printf("  消息: %s\n", anomaly->message);
        printf("  严重程度: %d/5\n", anomaly->severity);
        printf("---------------------------------------------------\n");
    }
}

int log_anomalies(AnomalyDetector *detector, const char *filename) {
    if (!detector || !detector->anomalies || !filename) {
        return -1;
    }

    FILE *file = fopen(filename, "a");
    if (!file) {
        return -1;
    }

    for (int i = 0; i < detector->anomaly_count; i++) {
        Anomaly *anomaly = &detector->anomalies[i];
        char time_str[64];
        struct tm *tm_info = localtime(&anomaly->timestamp);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
        
        fprintf(file, "[%s] 严重程度=%d, 指标=%s, 值=%.2f, 阈值=%.2f, 消息=%s\n",
                time_str, anomaly->severity, detector->metrics[anomaly->type].name,
                anomaly->value, anomaly->threshold, anomaly->message);
    }

    fclose(file);
    return 0;
}
