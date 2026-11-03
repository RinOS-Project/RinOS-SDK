/* SPDX-License-Identifier: MIT */
/* Linux bpf(2) command and attribute ABI supported by RinOS. */
#ifndef RIN_BPF_ABI_H
#define RIN_BPF_ABI_H

#include <stdint.h>
#include <stddef.h>

#if defined(_MSC_VER)
typedef __declspec(align(8)) uint64_t rin_bpf_aligned_u64;
#else
typedef uint64_t rin_bpf_aligned_u64 __attribute__((aligned(8)));
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum rin_bpf_command {
    RIN_BPF_MAP_CREATE = 0,
    RIN_BPF_MAP_LOOKUP_ELEM = 1,
    RIN_BPF_MAP_UPDATE_ELEM = 2,
    RIN_BPF_MAP_DELETE_ELEM = 3,
    RIN_BPF_MAP_GET_NEXT_KEY = 4,
    RIN_BPF_PROG_LOAD = 5,
    RIN_BPF_OBJ_PIN = 6,
    RIN_BPF_OBJ_GET = 7,
    RIN_BPF_PROG_ATTACH = 8,
    RIN_BPF_PROG_DETACH = 9,
    RIN_BPF_PROG_TEST_RUN = 10,
    RIN_BPF_OBJ_GET_INFO_BY_FD = 15,
    RIN_BPF_BTF_GET_NEXT_ID = 23,
    RIN_BPF_MAP_LOOKUP_BATCH = 24,
    RIN_BPF_MAP_LOOKUP_AND_DELETE_BATCH = 25,
    RIN_BPF_MAP_UPDATE_BATCH = 26,
    RIN_BPF_MAP_DELETE_BATCH = 27,
    RIN_BPF_LINK_CREATE = 28,
    RIN_BPF_LINK_UPDATE = 29,
    RIN_BPF_ENABLE_STATS = 32,
    RIN_BPF_PROG_QUERY = 16,
    RIN_BPF_PROG_GET_NEXT_ID = 11,
    RIN_BPF_MAP_GET_NEXT_ID = 12,
    RIN_BPF_PROG_GET_FD_BY_ID = 13,
    RIN_BPF_MAP_GET_FD_BY_ID = 14,
    RIN_BPF_BTF_GET_FD_BY_ID = 19,
    RIN_BPF_TASK_FD_QUERY = 20,
    RIN_BPF_MAP_FREEZE = 22,
    RIN_BPF_PROG_BIND_MAP = 31,
    RIN_BPF_FEATURE_QUERY = 0x52494e01u
};

enum rin_bpf_map_type {
    RIN_BPF_MAP_TYPE_UNSPEC = 0,
    RIN_BPF_MAP_TYPE_HASH = 1,
    RIN_BPF_MAP_TYPE_ARRAY = 2
};

enum rin_bpf_program_type {
    RIN_BPF_PROG_TYPE_UNSPEC = 0,
    RIN_BPF_PROG_TYPE_SOCKET_FILTER = 1,
    RIN_BPF_PROG_TYPE_TRACEPOINT = 5
};

enum rin_bpf_attach_type {
    RIN_BPF_ATTACH_TYPE_TRACE_SYSCALL_ENTER = 0x52494e01u
};

enum rin_bpf_map_update_flag {
    RIN_BPF_ANY = 0,
    RIN_BPF_NOEXIST = 1,
    RIN_BPF_EXIST = 2
};

enum rin_bpf_instruction_class {
    RIN_BPF_LD = 0x00,
    RIN_BPF_LDX = 0x01,
    RIN_BPF_ST = 0x02,
    RIN_BPF_STX = 0x03,
    RIN_BPF_ALU = 0x04,
    RIN_BPF_JMP = 0x05,
    RIN_BPF_JMP32 = 0x06,
    RIN_BPF_ALU64 = 0x07
};

enum rin_bpf_instruction_mode {
    RIN_BPF_IMM = 0x00,
    RIN_BPF_MEM = 0x60,
    RIN_BPF_ATOMIC = 0xc0
};

enum rin_bpf_instruction_size {
    RIN_BPF_W = 0x00,
    RIN_BPF_H = 0x08,
    RIN_BPF_B = 0x10,
    RIN_BPF_DW = 0x18
};

enum rin_bpf_instruction_alu_op {
    RIN_BPF_ADD = 0x00,
    RIN_BPF_SUB = 0x10,
    RIN_BPF_MUL = 0x20,
    RIN_BPF_DIV = 0x30,
    RIN_BPF_OR = 0x40,
    RIN_BPF_AND = 0x50,
    RIN_BPF_LSH = 0x60,
    RIN_BPF_RSH = 0x70,
    RIN_BPF_NEG = 0x80,
    RIN_BPF_MOD = 0x90,
    RIN_BPF_XOR = 0xa0,
    RIN_BPF_MOV = 0xb0,
    RIN_BPF_ARSH = 0xc0,
    RIN_BPF_END = 0xd0
};

enum rin_bpf_instruction_jump_op {
    RIN_BPF_JA = 0x00,
    RIN_BPF_JEQ = 0x10,
    RIN_BPF_JGT = 0x20,
    RIN_BPF_JGE = 0x30,
    RIN_BPF_JSET = 0x40,
    RIN_BPF_JNE = 0x50,
    RIN_BPF_JSGT = 0x60,
    RIN_BPF_JSGE = 0x70,
    RIN_BPF_CALL = 0x80,
    RIN_BPF_EXIT = 0x90,
    RIN_BPF_JLT = 0xa0,
    RIN_BPF_JLE = 0xb0,
    RIN_BPF_JSLT = 0xc0,
    RIN_BPF_JSLE = 0xd0
};

#define RIN_BPF_PSEUDO_MAP_FD 1u
#define RIN_BPF_MAX_INSTRUCTIONS 4096u
#define RIN_BPF_MAX_NAME 16u
#define RIN_BPF_MAX_KEY_SIZE 64u
#define RIN_BPF_MAX_VALUE_SIZE 256u
#define RIN_BPF_MAX_ENTRIES 4096u
#define RIN_BPF_MAX_LOG_SIZE 65536u
#define RIN_BPF_LICENSE_MAX 64u
#define RIN_BPF_FEATURE_MAPS (1ull << 0)
#define RIN_BPF_FEATURE_TRACEPOINT_PROGRAMS (1ull << 1)
#define RIN_BPF_FEATURE_BOUNDED_VERIFIER (1ull << 2)
#define RIN_BPF_FEATURE_TRACEPOINT_ATTACH (1ull << 3)
#define RIN_BPF_FEATURE_MAP_HELPERS (1ull << 4)
#define RIN_BPF_FEATURE_PID_UID_HELPERS (1ull << 5)
#define RIN_BPF_FEATURE_PROGRAM_TEST_RUN (1ull << 6)
#define RIN_BPF_FEATURE_TIME_CPU_HELPERS (1ull << 7)
#define RIN_BPF_FEATURE_TASK_HELPERS (1ull << 8)

struct rin_bpf_insn {
    uint8_t code;
    union {
        uint8_t regs;
        struct {
            uint8_t dst_reg : 4;
            uint8_t src_reg : 4;
        };
    };
    union {
        int16_t offset;
        int16_t off;
    };
    union {
        int32_t immediate;
        int32_t imm;
    };
};

/* Context passed to TRACEPOINT programs attached to the syscall-enter hook. */
struct rin_bpf_sys_enter_context {
    uint32_t arch;
    int32_t syscall_number;
    rin_bpf_aligned_u64 arguments[6];
    rin_bpf_aligned_u64 pid_tgid;
    rin_bpf_aligned_u64 uid_gid;
};

struct rin_bpf_map_create_attr {
    uint32_t map_type;
    uint32_t key_size;
    uint32_t value_size;
    uint32_t max_entries;
    uint32_t map_flags;
    uint32_t inner_map_fd;
    uint32_t numa_node;
    uint8_t map_name[RIN_BPF_MAX_NAME];
    uint32_t map_ifindex;
    uint32_t btf_fd;
    uint32_t btf_key_type_id;
    uint32_t btf_value_type_id;
    uint32_t btf_vmlinux_value_type_id;
    uint64_t map_extra;
};

struct rin_bpf_map_element_attr {
    uint32_t map_fd;
    uint32_t pad;
    rin_bpf_aligned_u64 key;
    union {
        rin_bpf_aligned_u64 value;
        rin_bpf_aligned_u64 next_key;
    };
    rin_bpf_aligned_u64 flags;
};

struct rin_bpf_program_load_attr {
    uint32_t program_type;
    uint32_t instruction_count;
    rin_bpf_aligned_u64 instructions;
    rin_bpf_aligned_u64 license;
    uint32_t log_level;
    uint32_t log_size;
    rin_bpf_aligned_u64 log_buffer;
    uint32_t kernel_version;
    uint32_t program_flags;
    uint8_t program_name[RIN_BPF_MAX_NAME];
    uint32_t ifindex;
    uint32_t expected_attach_type;
    uint32_t prog_btf_fd;
    uint32_t func_info_rec_size;
    rin_bpf_aligned_u64 func_info;
    uint32_t func_info_count;
    uint32_t line_info_rec_size;
    rin_bpf_aligned_u64 line_info;
    uint32_t line_info_count;
    uint32_t attach_btf_id;
    uint32_t attach_prog_fd;
    uint32_t core_relocation_count;
    rin_bpf_aligned_u64 core_relocations;
    uint32_t core_relocation_rec_size;
    uint32_t log_true_size;
    uint32_t token_fd;
};

struct rin_bpf_object_info_attr {
    uint32_t bpf_fd;
    uint32_t info_length;
    rin_bpf_aligned_u64 info;
};

struct rin_bpf_program_attach_attr {
    uint32_t target_fd;
    uint32_t program_fd;
    uint32_t attach_type;
    uint32_t attach_flags;
    uint32_t replace_program_fd;
    uint32_t reserved;
};

struct rin_bpf_program_test_run_attr {
    uint32_t prog_fd;
    uint32_t retval;
    uint32_t data_size_in;
    uint32_t data_size_out;
    rin_bpf_aligned_u64 data_in;
    rin_bpf_aligned_u64 data_out;
    uint32_t repeat;
    uint32_t duration;
    uint32_t ctx_size_in;
    uint32_t ctx_size_out;
    rin_bpf_aligned_u64 ctx_in;
    rin_bpf_aligned_u64 ctx_out;
    uint32_t flags;
    uint32_t cpu;
    uint32_t batch_size;
};

struct rin_bpf_feature_query_attr {
    uint32_t version;
    uint32_t struct_size;
    rin_bpf_aligned_u64 feature_flags;
    uint32_t map_type_mask;
    uint32_t program_type_mask;
    uint32_t helper_mask;
    uint32_t max_instruction_count;
    uint32_t max_map_entries;
    uint32_t max_map_bytes;
    uint32_t max_map_count;
    uint32_t max_program_map_count;
    uint32_t reserved[4];
};

struct rin_bpf_program_info {
    uint32_t type;
    uint32_t id;
    uint8_t tag[8];
    uint32_t instruction_count;
    uint32_t load_time_low;
    uint32_t creator_uid;
    uint32_t map_count;
    uint32_t name_length;
    uint8_t name[RIN_BPF_MAX_NAME];
    uint32_t attach_type;
    uint32_t reserved[8];
};

struct rin_bpf_map_info {
    uint32_t type;
    uint32_t id;
    uint32_t key_size;
    uint32_t value_size;
    uint32_t max_entries;
    uint32_t map_flags;
    uint8_t name[RIN_BPF_MAX_NAME];
    uint32_t ifindex;
    uint32_t reserved[8];
};

/* Linux bpf(2) uses a command-selected union.  The fixed 256-byte storage is
 * also the versioned attribute envelope for architecture-independent calls. */
union rin_bpf_attr {
    struct rin_bpf_map_create_attr map_create;
    struct rin_bpf_map_element_attr map_element;
    struct rin_bpf_program_load_attr program_load;
    struct rin_bpf_object_info_attr object_info;
    struct rin_bpf_program_attach_attr program_attach;
    struct rin_bpf_program_test_run_attr program_test_run;
    struct rin_bpf_feature_query_attr feature_query;
    struct {
        uint32_t map_type;
        uint32_t key_size;
        uint32_t value_size;
        uint32_t max_entries;
        uint32_t map_flags;
        uint32_t inner_map_fd;
        uint32_t numa_node;
        uint8_t map_name[RIN_BPF_MAX_NAME];
        uint32_t map_ifindex;
        uint32_t btf_fd;
        uint32_t btf_key_type_id;
        uint32_t btf_value_type_id;
        uint32_t btf_vmlinux_value_type_id;
    rin_bpf_aligned_u64 map_extra;
    };
    struct {
        uint32_t map_fd;
        uint32_t map_element_pad;
        rin_bpf_aligned_u64 key;
        union {
            rin_bpf_aligned_u64 value;
            rin_bpf_aligned_u64 next_key;
        };
        rin_bpf_aligned_u64 flags;
    };
    struct {
        uint32_t prog_type;
        uint32_t insn_cnt;
        rin_bpf_aligned_u64 insns;
        rin_bpf_aligned_u64 license;
        uint32_t log_level;
        uint32_t log_size;
        rin_bpf_aligned_u64 log_buf;
        uint32_t kern_version;
        uint32_t prog_flags;
        uint8_t prog_name[RIN_BPF_MAX_NAME];
        uint32_t prog_ifindex;
        uint32_t expected_attach_type;
        uint32_t prog_btf_fd;
        uint32_t func_info_rec_size;
        rin_bpf_aligned_u64 func_info;
        uint32_t func_info_cnt;
        uint32_t line_info_rec_size;
        rin_bpf_aligned_u64 line_info;
        uint32_t line_info_cnt;
        uint32_t attach_btf_id;
        uint32_t attach_prog_fd;
        uint32_t core_relo_cnt;
        rin_bpf_aligned_u64 fd_array;
        uint32_t core_relo_rec_size;
        uint32_t log_true_size;
        uint32_t prog_token_fd;
    };
    struct {
        uint32_t bpf_fd;
        uint32_t info_len;
        rin_bpf_aligned_u64 info;
    };
    struct {
        uint32_t target_fd;
        uint32_t prog_fd;
        uint32_t attach_type;
        uint32_t attach_flags;
        uint32_t replace_bpf_fd;
        uint32_t attach_reserved;
    };
    struct {
        uint32_t prog_fd;
        uint32_t retval;
        uint32_t data_size_in;
        uint32_t data_size_out;
        rin_bpf_aligned_u64 data_in;
        rin_bpf_aligned_u64 data_out;
        uint32_t repeat;
        uint32_t duration;
        uint32_t ctx_size_in;
        uint32_t ctx_size_out;
        rin_bpf_aligned_u64 ctx_in;
        rin_bpf_aligned_u64 ctx_out;
        uint32_t flags;
        uint32_t cpu;
        uint32_t batch_size;
    } test;
    uint8_t reserved[256];
};

#if defined(__cplusplus)
static_assert(sizeof(struct rin_bpf_insn) == 8u,
              "eBPF instruction layout must be 8 bytes");
static_assert(sizeof(struct rin_bpf_map_create_attr) == 72u,
              "eBPF map-create attribute layout mismatch");
static_assert(offsetof(struct rin_bpf_map_element_attr, key) == 8u,
              "eBPF map key pointer alignment mismatch");
static_assert(sizeof(struct rin_bpf_map_element_attr) == 32u,
              "eBPF map-element attribute layout mismatch");
static_assert(offsetof(struct rin_bpf_program_load_attr, instructions) == 8u,
              "eBPF instruction pointer alignment mismatch");
static_assert(sizeof(struct rin_bpf_program_load_attr) == 144u,
              "eBPF program-load attribute layout mismatch");
static_assert(sizeof(struct rin_bpf_program_test_run_attr) == 80u,
              "eBPF program-test-run attribute layout mismatch");
static_assert(sizeof(struct rin_bpf_sys_enter_context) == 72u,
              "eBPF syscall-enter context layout mismatch");
static_assert(sizeof(union rin_bpf_attr) == 256u,
              "eBPF attribute envelope must be 256 bytes");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(struct rin_bpf_insn) == 8u,
               "eBPF instruction layout must be 8 bytes");
_Static_assert(sizeof(struct rin_bpf_map_create_attr) == 72u,
               "eBPF map-create attribute layout mismatch");
_Static_assert(offsetof(struct rin_bpf_map_element_attr, key) == 8u,
               "eBPF map key pointer alignment mismatch");
_Static_assert(sizeof(struct rin_bpf_map_element_attr) == 32u,
               "eBPF map-element attribute layout mismatch");
_Static_assert(offsetof(struct rin_bpf_program_load_attr, instructions) == 8u,
               "eBPF instruction pointer alignment mismatch");
_Static_assert(sizeof(struct rin_bpf_program_load_attr) == 144u,
               "eBPF program-load attribute layout mismatch");
_Static_assert(sizeof(struct rin_bpf_program_test_run_attr) == 80u,
               "eBPF program-test-run attribute layout mismatch");
_Static_assert(sizeof(struct rin_bpf_sys_enter_context) == 72u,
               "eBPF syscall-enter context layout mismatch");
_Static_assert(sizeof(union rin_bpf_attr) == 256u,
               "eBPF attribute envelope must be 256 bytes");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_BPF_ABI_H */
