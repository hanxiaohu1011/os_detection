#include "../include/metrics_collector.h"
#include "../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 存储上一次CPU统计信息，用于计算使用率
static unsigned long long prev_total_user = 0;
static unsigned long long prev_total_user_low = 0;
static unsigned long long prev_total_sys = 0;
static unsigned long long prev_total_idle = 0;
static unsigned long long prev_total_iowait = 0;
static unsigned long long prev_total_irq = 0;
static unsigned long long prev_total_softirq = 0;
static unsigned long long prev_total_steal = 0;

// 存储上一次网络统计信息，用于计算丢包率
static unsigned long long prev_rx_dropped = 0;
static unsigned long long prev_tx_dropped = 0;
static time_t prev_net_time = 0;

int init_metrics_collector() {
    // 初始化CPU统计信息
    FILE *file = fopen("/proc/stat", "r");
    if (!file) {
        return -1;
    }

    char buffer[256];
    if (fgets(buffer, sizeof(buffer), file)) {
        // 解析CPU行
        sscanf(buffer, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
               &prev_total_user, &prev_total_user_low, &prev_total_sys, 
               &prev_total_idle, &prev_total_iowait, &prev_total_irq,
               &prev_total_softirq, &prev_total_steal);
    }
    fclose(file);

    // 初始化网络统计信息
    file = fopen("/proc/net/dev", "r");
    if (!file) {
        return -1;
    }

    // 跳过前两行
    fgets(buffer, sizeof(buffer), file);
    fgets(buffer, sizeof(buffer), file);

    char interface[32];
    unsigned long long rx_bytes, rx_packets, rx_errs, rx_drop;
    unsigned long long tx_bytes, tx_packets, tx_errs, tx_drop;

    while (fgets(buffer, sizeof(buffer), file)) {
        sscanf(buffer, "%[^:]: %llu %llu %llu %llu %*u %*u %*u %*u %llu %llu %llu %llu",
               interface, &rx_bytes, &rx_packets, &rx_errs, &rx_drop,
               &tx_bytes, &tx_packets, &tx_errs, &tx_drop);
        
        if (strcmp(interface, DEFAULT_NET_INTERFACE) == 0) {
            prev_rx_dropped = rx_drop;
            prev_tx_dropped = tx_drop;
            prev_net_time = time(NULL);
            break;
        }
    }
    fclose(file);

    return 0;
}

void cleanup_metrics_collector() {
    // 目前没有需要清理的资源
}

int read_cpu_usage(double *usage) {
    if (!usage) {
        return -1;
    }

    FILE *file = fopen("/proc/stat", "r");
    if (!file) {
        return -1;
    }

    unsigned long long total_user, total_user_low, total_sys, total_idle;
    unsigned long long total_iowait, total_irq, total_softirq, total_steal;

    char buffer[256];
    if (fgets(buffer, sizeof(buffer), file)) {
        // 解析CPU行
        sscanf(buffer, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
               &total_user, &total_user_low, &total_sys, &total_idle,
               &total_iowait, &total_irq, &total_softirq, &total_steal);
    } else {
        fclose(file);
        return -1;
    }
    fclose(file);

    // 计算差值
    unsigned long long diff_user = total_user - prev_total_user;
    unsigned long long diff_user_low = total_user_low - prev_total_user_low;
    unsigned long long diff_sys = total_sys - prev_total_sys;
    unsigned long long diff_idle = total_idle - prev_total_idle;
    unsigned long long diff_iowait = total_iowait - prev_total_iowait;
    unsigned long long diff_irq = total_irq - prev_total_irq;
    unsigned long long diff_softirq = total_softirq - prev_total_softirq;
    unsigned long long diff_steal = total_steal - prev_total_steal;

    // 计算总差值
    unsigned long long diff_total = diff_user + diff_user_low + diff_sys + diff_idle +
                                   diff_iowait + diff_irq + diff_softirq + diff_steal;

    // 计算CPU使用率
    if (diff_total > 0) {
        *usage = 100.0 * (diff_total - diff_idle - diff_iowait) / diff_total;
    } else {
        *usage = 0.0;
    }

    // 更新上一次的值
    prev_total_user = total_user;
    prev_total_user_low = total_user_low;
    prev_total_sys = total_sys;
    prev_total_idle = total_idle;
    prev_total_iowait = total_iowait;
    prev_total_irq = total_irq;
    prev_total_softirq = total_softirq;
    prev_total_steal = total_steal;

    return 0;
}

int read_cpu_iowait(double *iowait) {
    if (!iowait) {
        return -1;
    }

    FILE *file = fopen("/proc/stat", "r");
    if (!file) {
        return -1;
    }

    unsigned long long total_user, total_user_low, total_sys, total_idle;
    unsigned long long total_iowait, total_irq, total_softirq, total_steal;

    char buffer[256];
    if (fgets(buffer, sizeof(buffer), file)) {
        // 解析CPU行
        sscanf(buffer, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
               &total_user, &total_user_low, &total_sys, &total_idle,
               &total_iowait, &total_irq, &total_softirq, &total_steal);
    } else {
        fclose(file);
        return -1;
    }
    fclose(file);

    // 计算差值
    unsigned long long diff_iowait = total_iowait - prev_total_iowait;
    
    // 计算总差值
    unsigned long long diff_total = (total_user - prev_total_user) + 
                                   (total_user_low - prev_total_user_low) + 
                                   (total_sys - prev_total_sys) + 
                                   (total_idle - prev_total_idle) +
                                   diff_iowait + 
                                   (total_irq - prev_total_irq) + 
                                   (total_softirq - prev_total_softirq) + 
                                   (total_steal - prev_total_steal);

    // 计算IO等待率
    if (diff_total > 0) {
        *iowait = 100.0 * diff_iowait / diff_total;
    } else {
        *iowait = 0.0;
    }

    return 0;
}

int read_cpu_irq(double *irq) {
    if (!irq) {
        return -1;
    }

    FILE *file = fopen("/proc/stat", "r");
    if (!file) {
        return -1;
    }

    unsigned long long total_user, total_user_low, total_sys, total_idle;
    unsigned long long total_iowait, total_irq, total_softirq, total_steal;

    char buffer[256];
    if (fgets(buffer, sizeof(buffer), file)) {
        // 解析CPU行
        sscanf(buffer, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
               &total_user, &total_user_low, &total_sys, &total_idle,
               &total_iowait, &total_irq, &total_softirq, &total_steal);
    } else {
        fclose(file);
        return -1;
    }
    fclose(file);

    // 计算差值
    unsigned long long diff_irq = (total_irq - prev_total_irq) + 
                                 (total_softirq - prev_total_softirq);
    
    // 计算总差值
    unsigned long long diff_total = (total_user - prev_total_user) + 
                                   (total_user_low - prev_total_user_low) + 
                                   (total_sys - prev_total_sys) + 
                                   (total_idle - prev_total_idle) +
                                   (total_iowait - prev_total_iowait) + 
                                   diff_irq + 
                                   (total_steal - prev_total_steal);

    // 计算中断率
    if (diff_total > 0) {
        *irq = 100.0 * diff_irq / diff_total;
    } else {
        *irq = 0.0;
    }

    return 0;
}

int read_mem_usage(double *usage) {
    if (!usage) {
        return -1;
    }

    FILE *file = fopen("/proc/meminfo", "r");
    if (!file) {
        return -1;
    }

    unsigned long total_mem = 0;
    unsigned long free_mem = 0;
    unsigned long buffers = 0;
    unsigned long cached = 0;

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file)) {
        if (strncmp(buffer, "MemTotal:", 9) == 0) {
            sscanf(buffer, "MemTotal: %lu", &total_mem);
        } else if (strncmp(buffer, "MemFree:", 8) == 0) {
            sscanf(buffer, "MemFree: %lu", &free_mem);
        } else if (strncmp(buffer, "Buffers:", 8) == 0) {
            sscanf(buffer, "Buffers: %lu", &buffers);
        } else if (strncmp(buffer, "Cached:", 7) == 0) {
            sscanf(buffer, "Cached: %lu", &cached);
        }
    }
    fclose(file);

    if (total_mem > 0) {
        // 计算已使用内存（不包括缓存和缓冲区）
        unsigned long used_mem = total_mem - free_mem - buffers - cached;
        *usage = 100.0 * used_mem / total_mem;
    } else {
        *usage = 0.0;
    }

    return 0;
}

int read_mem_active(double *active) {
    if (!active) {
        return -1;
    }

    FILE *file = fopen("/proc/meminfo", "r");
    if (!file) {
        return -1;
    }

    unsigned long active_mem = 0;

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file)) {
        if (strncmp(buffer, "Active:", 7) == 0) {
            sscanf(buffer, "Active: %lu", &active_mem);
            break;
        }
    }
    fclose(file);

    *active = (double)active_mem;
    return 0;
}

int read_disk_stats(const char *device, unsigned long *reads_completed, 
                   unsigned long *reads_merged, unsigned long long *sectors_read,
                   unsigned long *read_time_ms, unsigned long *writes_completed,
                   unsigned long *writes_merged, unsigned long long *sectors_written,
                   unsigned long *write_time_ms, unsigned long *io_in_progress,
                   unsigned long *io_time_ms, unsigned long *weighted_io_time_ms) {
    
    FILE *file = fopen("/proc/diskstats", "r");
    if (!file) {
        return -1;
    }

    char buffer[256];
    char dev_name[32];
    int found = 0;

    while (fgets(buffer, sizeof(buffer), file)) {
        int major, minor;
        int matched = sscanf(buffer, "%d %d %s %lu %lu %llu %lu %lu %lu %llu %lu %lu %lu %lu",
                            &major, &minor, dev_name,
                            reads_completed, reads_merged, sectors_read, read_time_ms,
                            writes_completed, writes_merged, sectors_written, write_time_ms,
                            io_in_progress, io_time_ms, weighted_io_time_ms);
        
        if (matched == 14 && strcmp(dev_name, device) == 0) {
            found = 1;
            break;
        }
    }
    fclose(file);

    return found ? 0 : -1;
}

int read_disk_read_await(double *read_await, const char *device) {
    if (!read_await || !device) {
        return -1;
    }

    unsigned long reads_completed, reads_merged, read_time_ms;
    unsigned long writes_completed, writes_merged, write_time_ms;
    unsigned long io_in_progress, io_time_ms, weighted_io_time_ms;
    unsigned long long sectors_read, sectors_written;

    int ret = read_disk_stats(device, &reads_completed, &reads_merged, &sectors_read,
                             &read_time_ms, &writes_completed, &writes_merged,
                             &sectors_written, &write_time_ms, &io_in_progress,
                             &io_time_ms, &weighted_io_time_ms);
    
    if (ret != 0) {
        return -1;
    }

    if (reads_completed > 0) {
        *read_await = (double)read_time_ms / reads_completed;
    } else {
        *read_await = 0.0;
    }

    return 0;
}

int read_disk_write_await(double *write_await, const char *device) {
    if (!write_await || !device) {
        return -1;
    }

    unsigned long reads_completed, reads_merged, read_time_ms;
    unsigned long writes_completed, writes_merged, write_time_ms;
    unsigned long io_in_progress, io_time_ms, weighted_io_time_ms;
    unsigned long long sectors_read, sectors_written;

    int ret = read_disk_stats(device, &reads_completed, &reads_merged, &sectors_read,
                             &read_time_ms, &writes_completed, &writes_merged,
                             &sectors_written, &write_time_ms, &io_in_progress,
                             &io_time_ms, &weighted_io_time_ms);
    
    if (ret != 0) {
        return -1;
    }

    if (writes_completed > 0) {
        *write_await = (double)write_time_ms / writes_completed;
    } else {
        *write_await = 0.0;
    }

    return 0;
}

int read_disk_util(double *util, const char *device) {
    if (!util || !device) {
        return -1;
    }

    unsigned long reads_completed, reads_merged, read_time_ms;
    unsigned long writes_completed, writes_merged, write_time_ms;
    unsigned long io_in_progress, io_time_ms, weighted_io_time_ms;
    unsigned long long sectors_read, sectors_written;

    int ret = read_disk_stats(device, &reads_completed, &reads_merged, &sectors_read,
                             &read_time_ms, &writes_completed, &writes_merged,
                             &sectors_written, &write_time_ms, &io_in_progress,
                             &io_time_ms, &weighted_io_time_ms);
    
    if (ret != 0) {
        return -1;
    }

    // 假设采样间隔为1秒，计算磁盘利用率
    // io_time_ms是自系统启动以来磁盘处于活动状态的毫秒数
    // 这里简单地将其除以1000，得到磁盘利用率的近似值
    *util = (double)io_time_ms / 1000.0;
    if (*util > 100.0) {
        *util = 100.0;
    }

    return 0;
}

int read_net_dropped(double *dropped, const char *interface) {
    if (!dropped || !interface) {
        return -1;
    }

    FILE *file = fopen("/proc/net/dev", "r");
    if (!file) {
        return -1;
    }

    char buffer[256];
    // 跳过前两行
    fgets(buffer, sizeof(buffer), file);
    fgets(buffer, sizeof(buffer), file);

    char if_name[32];
    unsigned long long rx_bytes, rx_packets, rx_errs, rx_drop;
    unsigned long long tx_bytes, tx_packets, tx_errs, tx_drop;
    int found = 0;

    while (fgets(buffer, sizeof(buffer), file)) {
        sscanf(buffer, "%[^:]: %llu %llu %llu %llu %*u %*u %*u %*u %llu %llu %llu %llu",
               if_name, &rx_bytes, &rx_packets, &rx_errs, &rx_drop,
               &tx_bytes, &tx_packets, &tx_errs, &tx_drop);
        
        if (strcmp(if_name, interface) == 0) {
            found = 1;
            break;
        }
    }
    fclose(file);

    if (!found) {
        return -1;
    }

    // 计算丢包率（每秒丢包数）
    time_t current_time = time(NULL);
    double time_diff = difftime(current_time, prev_net_time);
    
    if (time_diff > 0) {
        unsigned long long total_drop = rx_drop + tx_drop;
        unsigned long long diff_drop = total_drop - (prev_rx_dropped + prev_tx_dropped);
        *dropped = (double)diff_drop / time_diff;
    } else {
        *dropped = 0.0;
    }

    // 更新上一次的值
    prev_rx_dropped = rx_drop;
    prev_tx_dropped = tx_drop;
    prev_net_time = current_time;

    return 0;
}

int collect_metrics(AnomalyDetector *detector) {
    if (!detector) {
        return -1;
    }

    double value;

    // 收集CPU使用率
    if (read_cpu_usage(&value) == 0) {
        add_metric_datapoint(&detector->metrics[METRIC_CPU_USAGE], value);
    }

    // 收集CPU IO等待
    if (read_cpu_iowait(&value) == 0) {
        add_metric_datapoint(&detector->metrics[METRIC_CPU_IOWAIT], value);
    }

    // 收集CPU中断
    if (read_cpu_irq(&value) == 0) {
        add_metric_datapoint(&detector->metrics[METRIC_CPU_IRQ], value);
    }

    // 收集内存使用率
    if (read_mem_usage(&value) == 0) {
        add_metric_datapoint(&detector->metrics[METRIC_MEM_USAGE], value);
    }

    // 收集活跃内存
    if (read_mem_active(&value) == 0) {
        add_metric_datapoint(&detector->metrics[METRIC_MEM_ACTIVE], value);
    }

    // 收集磁盘读响应时间
    if (read_disk_read_await(&value, DEFAULT_DISK_DEVICE) == 0) {
        add_metric_datapoint(&detector->metrics[METRIC_DISK_READ_AWAIT], value);
    }

    // 收集磁盘写响应时间
    if (read_disk_write_await(&value, DEFAULT_DISK_DEVICE) == 0) {
        add_metric_datapoint(&detector->metrics[METRIC_DISK_WRITE_AWAIT], value);
    }

    // 收集磁盘使用率
    if (read_disk_util(&value, DEFAULT_DISK_DEVICE) == 0) {
        add_metric_datapoint(&detector->metrics[METRIC_DISK_UTIL], value);
    }

    // 收集网络丢包
    if (read_net_dropped(&value, DEFAULT_NET_INTERFACE) == 0) {
        add_metric_datapoint(&detector->metrics[METRIC_NET_DROPPED], value);
    }

    return 0;
}
