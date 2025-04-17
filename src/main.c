#include "../include/anomaly_detection.h"
#include "../include/metrics_collector.h"
#include "../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

// 全局变量，用于信号处理
static volatile bool running = true;

// 信号处理函数
void signal_handler(int sig) {
    printf("接收到信号 %d，准备退出...\n", sig);
    running = false;
}

void print_help() {
    printf("操作系统指标异常检测系统\n");
    printf("用法: anomaly_detection [选项]\n");
    printf("选项:\n");
    printf("  -h            显示帮助信息\n");
    printf("  -i <秒>       设置采样间隔（默认: %d秒）\n", DEFAULT_SAMPLING_INTERVAL);
    printf("  -w <数量>     设置滑动窗口大小（默认: %d个数据点）\n", DEFAULT_WINDOW_SIZE);
    printf("  -s <因子>     设置N-Sigma因子（默认: %.1f）\n", DEFAULT_SIGMA_FACTOR);
    printf("  -l <文件>     设置日志文件路径（默认: %s）\n", LOG_FILE_PATH);
    printf("  -d <设备>     设置磁盘设备名（默认: %s）\n", DEFAULT_DISK_DEVICE);
    printf("  -n <接口>     设置网络接口名（默认: %s）\n", DEFAULT_NET_INTERFACE);
}

int main(int argc, char *argv[]) {
    // 默认参数
    int sampling_interval = DEFAULT_SAMPLING_INTERVAL;
    int window_size = DEFAULT_WINDOW_SIZE;
    double sigma_factor = DEFAULT_SIGMA_FACTOR;
    char log_file[256] = LOG_FILE_PATH;
    
    // 解析命令行参数
    int opt;
    while ((opt = getopt(argc, argv, "hi:w:s:l:d:n:")) != -1) {
        switch (opt) {
            case 'h':
                print_help();
                return 0;
            case 'i':
                sampling_interval = atoi(optarg);
                if (sampling_interval <= 0) {
                    fprintf(stderr, "错误: 采样间隔必须大于0\n");
                    return 1;
                }
                break;
            case 'w':
                window_size = atoi(optarg);
                if (window_size <= 0) {
                    fprintf(stderr, "错误: 窗口大小必须大于0\n");
                    return 1;
                }
                break;
            case 's':
                sigma_factor = atof(optarg);
                if (sigma_factor <= 0) {
                    fprintf(stderr, "错误: Sigma因子必须大于0\n");
                    return 1;
                }
                break;
            case 'l':
                strncpy(log_file, optarg, sizeof(log_file) - 1);
                log_file[sizeof(log_file) - 1] = '\0';
                break;
            case 'd':
                // 这里可以设置磁盘设备，但需要修改config.h
                fprintf(stderr, "警告: 磁盘设备设置需要修改config.h\n");
                break;
            case 'n':
                // 这里可以设置网络接口，但需要修改config.h
                fprintf(stderr, "警告: 网络接口设置需要修改config.h\n");
                break;
            default:
                fprintf(stderr, "使用 -h 选项获取帮助\n");
                return 1;
        }
    }
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("操作系统指标异常检测系统启动\n");
    printf("采样间隔: %d秒\n", sampling_interval);
    printf("滑动窗口大小: %d个数据点\n", window_size);
    printf("N-Sigma因子: %.1f\n", sigma_factor);
    printf("日志文件: %s\n", log_file);
    printf("磁盘设备: %s\n", DEFAULT_DISK_DEVICE);
    printf("网络接口: %s\n", DEFAULT_NET_INTERFACE);
    printf("按Ctrl+C退出\n\n");
    
    // 初始化指标收集器
    if (init_metrics_collector() != 0) {
        fprintf(stderr, "错误: 无法初始化指标收集器\n");
        return 1;
    }
    
    // 初始化异常检测器
    AnomalyDetector detector;
    if (init_detector(&detector, window_size, sigma_factor) != 0) {
        fprintf(stderr, "错误: 无法初始化异常检测器\n");
        cleanup_metrics_collector();
        return 1;
    }
    
    // 主循环
    int cycle = 0;
    while (running) {
        printf("\n--- 周期 %d ---\n", ++cycle);
        
        // 收集指标
        if (collect_metrics(&detector) != 0) {
            fprintf(stderr, "警告: 收集指标时出错\n");
        }
        
        // 打印当前指标值
        printf("当前指标值:\n");
        for (int i = 0; i < METRIC_COUNT; i++) {
            Metric *metric = &detector.metrics[i];
            printf("  %s: %.2f", metric->name, metric->value);
            
            // 如果有足够的历史数据，显示统计信息
            if (metric->history_size >= 3) {
                printf(" (均值: %.2f, 标准差: %.2f)", metric->mean, metric->stddev);
            }
            printf("\n");
        }
        
        // 需要足够的历史数据才能进行异常检测
        if (cycle >= 3) {
            // 重置异常计数
            detector.anomaly_count = 0;
            
            // 使用N-Sigma算法检测异常
            int nsigma_anomalies = detect_anomalies_nsigma(&detector);
            
            // 使用阈值检测异常
            int threshold_anomalies = detect_anomalies_threshold(&detector);
            
            // 打印异常
            print_anomalies(&detector);
            
            // 记录异常到日志
            if (detector.anomaly_count > 0) {
                if (log_anomalies(&detector, log_file) != 0) {
                    fprintf(stderr, "警告: 无法写入日志文件 %s\n", log_file);
                }
            }
        } else {
            printf("收集更多数据点以进行异常检测...\n");
        }
        
        // 等待下一个采样周期
        printf("等待 %d 秒...\n", sampling_interval);
        for (int i = 0; i < sampling_interval && running; i++) {
            sleep(1);
        }
    }
    
    printf("清理资源并退出...\n");
    
    // 清理资源
    free_detector(&detector);
    cleanup_metrics_collector();
    
    return 0;
}
