/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_LINUX_PERF_EVENT_H
#define RIN_SDK_LINUX_PERF_EVENT_H

#include <stdint.h>

#define PERF_TYPE_HARDWARE 0
#define PERF_TYPE_SOFTWARE 1
#define PERF_TYPE_TRACEPOINT 2
#define PERF_TYPE_HW_CACHE 3
#define PERF_TYPE_RAW 4
#define PERF_TYPE_BREAKPOINT 5

#define PERF_COUNT_HW_CPU_CYCLES 0
#define PERF_COUNT_HW_INSTRUCTIONS 1
#define PERF_COUNT_HW_CACHE_REFERENCES 2
#define PERF_COUNT_HW_CACHE_MISSES 3
#define PERF_COUNT_HW_BRANCH_INSTRUCTIONS 4
#define PERF_COUNT_HW_BRANCH_MISSES 5
#define PERF_COUNT_HW_BUS_CYCLES 6
#define PERF_COUNT_HW_STALLED_CYCLES_FRONTEND 7
#define PERF_COUNT_HW_STALLED_CYCLES_BACKEND 8
#define PERF_COUNT_HW_REF_CPU_CYCLES 9

#define PERF_COUNT_SW_CPU_CLOCK 0
#define PERF_COUNT_SW_TASK_CLOCK 1
#define PERF_COUNT_SW_PAGE_FAULTS 2
#define PERF_COUNT_SW_CONTEXT_SWITCHES 3
#define PERF_COUNT_SW_CPU_MIGRATIONS 4
#define PERF_COUNT_SW_PAGE_FAULTS_MIN 5
#define PERF_COUNT_SW_PAGE_FAULTS_MAJ 6
#define PERF_COUNT_SW_ALIGNMENT_FAULTS 7
#define PERF_COUNT_SW_EMULATION_FAULTS 8
#define PERF_COUNT_SW_DUMMY 9
#define PERF_COUNT_SW_BPF_OUTPUT 10
#define PERF_COUNT_SW_CGROUP_SWITCHES 11

#define PERF_SAMPLE_IP (1ULL << 0)
#define PERF_SAMPLE_TID (1ULL << 1)
#define PERF_SAMPLE_TIME (1ULL << 2)
#define PERF_SAMPLE_ADDR (1ULL << 3)
#define PERF_SAMPLE_READ (1ULL << 4)
#define PERF_SAMPLE_CALLCHAIN (1ULL << 5)
#define PERF_SAMPLE_ID (1ULL << 6)
#define PERF_SAMPLE_CPU (1ULL << 7)
#define PERF_SAMPLE_PERIOD (1ULL << 8)
#define PERF_SAMPLE_STREAM_ID (1ULL << 9)
#define PERF_SAMPLE_RAW (1ULL << 10)
#define PERF_SAMPLE_BRANCH_STACK (1ULL << 11)
#define PERF_SAMPLE_REGS_USER (1ULL << 12)
#define PERF_SAMPLE_STACK_USER (1ULL << 13)
#define PERF_SAMPLE_WEIGHT (1ULL << 14)
#define PERF_SAMPLE_DATA_SRC (1ULL << 15)
#define PERF_SAMPLE_IDENTIFIER (1ULL << 16)

#define PERF_FORMAT_TOTAL_TIME_ENABLED (1ULL << 0)
#define PERF_FORMAT_TOTAL_TIME_RUNNING (1ULL << 1)
#define PERF_FORMAT_ID (1ULL << 2)
#define PERF_FORMAT_GROUP (1ULL << 3)
#define PERF_FORMAT_LOST (1ULL << 4)

#define PERF_FLAG_FD_NO_GROUP (1UL << 0)
#define PERF_FLAG_FD_OUTPUT (1UL << 1)
#define PERF_FLAG_PID_CGROUP (1UL << 2)
#define PERF_FLAG_FD_CLOEXEC (1UL << 3)

#define PERF_EVENT_IOC_ENABLE 0x2400UL
#define PERF_EVENT_IOC_DISABLE 0x2401UL
#define PERF_EVENT_IOC_REFRESH 0x2402UL
#define PERF_EVENT_IOC_RESET 0x2403UL
#define PERF_EVENT_IOC_PERIOD 0x40082404UL
#define PERF_EVENT_IOC_SET_OUTPUT 0x2405UL
#define PERF_EVENT_IOC_SET_FILTER 0x40082406UL
#define PERF_EVENT_IOC_ID 0x80082407UL
#define PERF_EVENT_IOC_SET_BPF 0x40042408UL
#define PERF_EVENT_IOC_PAUSE_OUTPUT 0x40042409UL
#define PERF_EVENT_IOC_QUERY_BPF 0xc008240aUL
#define PERF_EVENT_IOC_MODIFY_ATTRIBUTES 0x4080240bUL
#define PERF_IOC_FLAG_GROUP 1UL

#define PERF_RECORD_MMAP 1
#define PERF_RECORD_LOST 2
#define PERF_RECORD_COMM 3
#define PERF_RECORD_EXIT 4
#define PERF_RECORD_THROTTLE 5
#define PERF_RECORD_UNTHROTTLE 6
#define PERF_RECORD_FORK 7
#define PERF_RECORD_READ 8
#define PERF_RECORD_SAMPLE 9

struct perf_event_attr {
    uint32_t type;
    uint32_t size;
    uint64_t config;
    union {
        uint64_t sample_period;
        uint64_t sample_freq;
    };
    uint64_t sample_type;
    uint64_t read_format;
    union {
        uint64_t flags;
        struct {
            uint64_t disabled : 1;
            uint64_t inherit : 1;
            uint64_t pinned : 1;
            uint64_t exclusive : 1;
            uint64_t exclude_user : 1;
            uint64_t exclude_kernel : 1;
            uint64_t exclude_hv : 1;
            uint64_t exclude_idle : 1;
            uint64_t mmap : 1;
            uint64_t comm : 1;
            uint64_t freq : 1;
            uint64_t inherit_stat : 1;
            uint64_t enable_on_exec : 1;
            uint64_t task : 1;
            uint64_t watermark : 1;
            uint64_t precise_ip : 2;
            uint64_t mmap_data : 1;
            uint64_t sample_id_all : 1;
            uint64_t exclude_host : 1;
            uint64_t exclude_guest : 1;
            uint64_t exclude_callchain_kernel : 1;
            uint64_t exclude_callchain_user : 1;
            uint64_t mmap2 : 1;
            uint64_t comm_exec : 1;
            uint64_t use_clockid : 1;
            uint64_t context_switch : 1;
            uint64_t write_backward : 1;
            uint64_t namespaces : 1;
            uint64_t ksymbol : 1;
            uint64_t bpf_event : 1;
            uint64_t aux_output : 1;
            uint64_t cgroup : 1;
            uint64_t text_poke : 1;
            uint64_t build_id : 1;
            uint64_t inherit_thread : 1;
            uint64_t remove_on_exec : 1;
            uint64_t sigtrap : 1;
            uint64_t __reserved_1 : 26;
        };
    };
    uint32_t wakeup_events;
    uint32_t bp_type;
    uint64_t config1;
    uint64_t config2;
    uint64_t branch_sample_type;
    uint64_t sample_regs_user;
    uint32_t sample_stack_user;
    int32_t clockid;
    uint64_t sample_regs_intr;
    uint32_t aux_watermark;
    uint16_t sample_max_stack;
    uint16_t __reserved_2;
    uint32_t aux_sample_size;
    uint32_t __reserved_3;
    uint64_t sig_data;
};

#if defined(__cplusplus)
static_assert(sizeof(struct perf_event_attr) == 128,
              "Linux perf_event_attr ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(struct perf_event_attr) == 128,
               "Linux perf_event_attr ABI drift");
#endif

#endif /* RIN_SDK_LINUX_PERF_EVENT_H */
