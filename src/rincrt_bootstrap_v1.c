/* SPDX-License-Identifier: MIT */
/*
 * Minimal freestanding C runtime used by the native rcc bootstrap images.
 * Keep this file independent from the header-only application libc: rincrt
 * is the stable shared-library owner of these symbols.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <limits.h>

#include <rin/syscall_abi.h>

extern void* rin_user_allocator_malloc(size_t size);
extern void rin_user_allocator_free(void* pointer);
extern void* rin_user_allocator_realloc(void* pointer, size_t size);
extern size_t rin_user_allocator_usable_size(const void* pointer);
extern void rin_user_allocator_after_fork_child(void);

#if UINTPTR_MAX > 0xffffffffu
#define RINCRT_ALIGNED_META_MARKER ((size_t)0xC0DEC0DEBADC0FFEULL)
#else
#define RINCRT_ALIGNED_META_MARKER ((size_t)0xC0DEC0DEUL)
#endif

typedef struct RinCrtAlignedAllocationMeta {
    size_t guard;
    void* raw;
} RinCrtAlignedAllocationMeta;

static RinCrtAlignedAllocationMeta* rincrt_aligned_meta_from_ptr(void* pointer)
{
    return pointer
        ? (RinCrtAlignedAllocationMeta*)((unsigned char*)pointer -
                                         sizeof(RinCrtAlignedAllocationMeta))
        : (RinCrtAlignedAllocationMeta*)0;
}

static size_t rincrt_aligned_guard(void* pointer)
{
    return RINCRT_ALIGNED_META_MARKER ^ (size_t)(uintptr_t)pointer;
}

static int rincrt_load_aligned_raw(void* pointer, void** raw_out)
{
    RinCrtAlignedAllocationMeta* meta;
    if (!pointer || !raw_out) return 0;
    meta = rincrt_aligned_meta_from_ptr(pointer);
    if (!meta || meta->guard != rincrt_aligned_guard(pointer) ||
        !meta->raw || (uintptr_t)meta->raw >= (uintptr_t)pointer)
        return 0;
    *raw_out = meta->raw;
    return 1;
}

#if defined(RINCRT_HOST_TEST)
#define __errno_location rincrt_test_errno_location
#define __rin_stderr rincrt_test_stderr
#define _exit rincrt_test__exit
#define atexit rincrt_test_atexit
#define atoi rincrt_test_atoi
#define close rincrt_test_close
#define calloc rincrt_test_calloc
#define execvp rincrt_test_execvp
#define exit rincrt_test_exit
#define fclose rincrt_test_fclose
#define feof rincrt_test_feof
#define ferror rincrt_test_ferror
#define fgets rincrt_test_fgets
#define fopen rincrt_test_fopen
#define fork rincrt_test_fork
#define fprintf rincrt_test_fprintf
#define fputc rincrt_test_fputc
#define fputs rincrt_test_fputs
#define fread rincrt_test_fread
#define free rincrt_test_free
#define fseek rincrt_test_fseek
#define ftell rincrt_test_ftell
#define fwrite rincrt_test_fwrite
#define isalnum rincrt_test_isalnum
#define isalpha rincrt_test_isalpha
#define isdigit rincrt_test_isdigit
#define isspace rincrt_test_isspace
#define isxdigit rincrt_test_isxdigit
#define malloc rincrt_test_malloc
#define malloc_usable_size rincrt_test_malloc_usable_size
#define memchr rincrt_test_memchr
#define memcpy rincrt_test_memcpy
#define memset rincrt_test_memset
#define mkstemp rincrt_test_mkstemp
#define perror rincrt_test_perror
#define printf rincrt_test_printf
#define qsort rincrt_test_qsort
#define realloc rincrt_test_realloc
#define aligned_alloc rincrt_test_aligned_alloc
#define posix_memalign rincrt_test_posix_memalign
#define remove rincrt_test_remove
#define rename rincrt_test_rename
#define snprintf rincrt_test_snprintf
#define strchr rincrt_test_strchr
#define strcmp rincrt_test_strcmp
#define strcpy rincrt_test_strcpy
#define strlen rincrt_test_strlen
#define strncat rincrt_test_strncat
#define strncmp rincrt_test_strncmp
#define strncpy rincrt_test_strncpy
#define strrchr rincrt_test_strrchr
#define strtod rincrt_test_strtod
#define strtoull rincrt_test_strtoull
#define tolower rincrt_test_tolower
#define vfprintf rincrt_test_vfprintf
#define vsnprintf rincrt_test_vsnprintf
#define waitpid rincrt_test_waitpid
#endif

enum {
    RINCRT_EINTR = 4,
    RINCRT_EIO = 5,
    RINCRT_EBADF = 9,
    RINCRT_ENOMEM = 12,
    RINCRT_EEXIST = 17,
    RINCRT_EINVAL = 22,
    RINCRT_EMFILE = 24,
    RINCRT_ERANGE = 34
};

enum {
    RINCRT_O_RDONLY = 0x0001,
    RINCRT_O_WRONLY = 0x0002,
    RINCRT_O_RDWR = 0x0003,
    RINCRT_O_CREAT = 0x0004,
    RINCRT_O_EXCL = 0x0008,
    RINCRT_O_TRUNC = 0x0010,
    RINCRT_O_APPEND = 0x0020
};

enum {
    RINCRT_EOF = -1,
    RINCRT_SEEK_CUR = 1,
    RINCRT_FILE_CAPACITY = 16,
    RINCRT_ATEXIT_CAPACITY = 32,
    RINCRT_GETRANDOM = 350
};

typedef struct RCCBootstrapFile {
    int descriptor;
    int eof;
    int error;
    unsigned int flags;
} FILE;

typedef void (*RinCrtExitHandler)(void);

/* The stable symbol ABI goes through __errno_location(); the signed-library
 * graph loader assigns this module's local-exec slot to every thread. */
static _Thread_local int g_rincrt_errno;
static FILE g_rincrt_stdout = {1, 0, 0, RINCRT_O_WRONLY};
static FILE g_rincrt_stderr = {2, 0, 0, RINCRT_O_WRONLY};
static FILE g_rincrt_files[RINCRT_FILE_CAPACITY];
static unsigned char g_rincrt_file_used[RINCRT_FILE_CAPACITY];
static RinCrtExitHandler g_rincrt_exit_handlers[RINCRT_ATEXIT_CAPACITY];
static unsigned int g_rincrt_exit_handler_count;

int* __errno_location(void)
{
    return &g_rincrt_errno;
}

FILE* __rin_stderr(void)
{
    return &g_rincrt_stderr;
}

/* Syscall carriers are target-width words, not C `long`.  The latter is
 * 32-bit on LLP64 hosted compilers even when pointers and the kernel ABI are
 * 64-bit, which silently truncates pointer arguments before inline asm sees
 * them. */
static intptr_t rincrt_raw_syscall0(uintptr_t number)
{
    intptr_t result;
#if defined(__x86_64__)
    __asm__ __volatile__("syscall" : "=a"(result) : "a"(number)
                         : "rcx", "r11", "memory");
#else
    __asm__ __volatile__("int $0x80" : "=a"(result) : "a"(number)
                         : "memory");
#endif
    return result;
}

static intptr_t rincrt_raw_syscall1(uintptr_t number, uintptr_t first)
{
    intptr_t result;
#if defined(__x86_64__)
    __asm__ __volatile__("syscall" : "=a"(result)
                         : "a"(number), "D"(first)
                         : "rcx", "r11", "memory");
#else
    __asm__ __volatile__("int $0x80" : "=a"(result)
                         : "a"(number), "b"(first) : "memory");
#endif
    return result;
}

static intptr_t rincrt_raw_syscall2(uintptr_t number, uintptr_t first,
                                    uintptr_t second)
{
    intptr_t result;
#if defined(__x86_64__)
    __asm__ __volatile__("syscall" : "=a"(result)
                         : "a"(number), "D"(first), "S"(second)
                         : "rcx", "r11", "memory");
#else
    __asm__ __volatile__("int $0x80" : "=a"(result)
                         : "a"(number), "b"(first), "c"(second)
                         : "memory");
#endif
    return result;
}

static intptr_t rincrt_raw_syscall3(uintptr_t number, uintptr_t first,
                                    uintptr_t second, uintptr_t third)
{
    intptr_t result;
#if defined(__x86_64__)
    __asm__ __volatile__("syscall" : "=a"(result)
                         : "a"(number), "D"(first), "S"(second), "d"(third)
                         : "rcx", "r11", "memory");
#else
    __asm__ __volatile__("int $0x80" : "=a"(result)
                         : "a"(number), "b"(first), "c"(second), "d"(third)
                         : "memory");
#endif
    return result;
}

#if !defined(RINCRT_HOST_TEST)
static intptr_t rincrt_raw_syscall6(uintptr_t number, uintptr_t first,
                                    uintptr_t second, uintptr_t third,
                                    uintptr_t fourth, uintptr_t fifth,
                                    uintptr_t sixth)
{
    intptr_t result;
#if defined(__x86_64__)
    register uintptr_t r10 __asm__("r10") = fourth;
    register uintptr_t r8 __asm__("r8") = fifth;
    register uintptr_t r9 __asm__("r9") = sixth;
    __asm__ __volatile__("syscall"
                         : "=a"(result)
                         : "a"(number), "D"(first), "S"(second), "d"(third),
                           "r"(r10), "r"(r8), "r"(r9)
                         : "rcx", "r11", "memory");
#else
    __asm__ __volatile__(
        "pushl %%ebp\n\t"
        "movl %7, %%ebp\n\t"
        "int $0x80\n\t"
        "popl %%ebp"
        : "=&a"(result)
        : "0"(number), "b"(first), "c"(second), "d"(third),
          "S"(fourth), "D"(fifth), "m"(sixth)
        : "memory");
#endif
    return result;
}
#endif

static int rincrt_status_result(intptr_t result);

void rin_user_allocator_corruption(void)
{
    g_rincrt_errno = RINCRT_EIO;
    (void)rincrt_raw_syscall1(RIN_SYS_EXIT, 134u);
    for (;;) {}
}

void* rin_user_allocator_backend_map(size_t size)
{
#if defined(RINCRT_HOST_TEST)
    extern void* rincrt_test_allocator_map(size_t size);
    return rincrt_test_allocator_map(size);
#else
    intptr_t result;
    if (size == 0u) {
        g_rincrt_errno = RINCRT_EINVAL;
        return (void*)0;
    }
    result = rincrt_raw_syscall6(
        RIN_SYS_MMAP, 0u, (uintptr_t)size, 3u, 0x22u,
        (uintptr_t)-1, 0u);
    if (result < 0) {
        if (result >= -4095) g_rincrt_errno = (int)-result;
        else g_rincrt_errno = RINCRT_EIO;
        return (void*)0;
    }
    return (void*)(uintptr_t)result;
#endif
}

int rin_user_allocator_backend_unmap(void* pointer, size_t size)
{
#if defined(RINCRT_HOST_TEST)
    extern int rincrt_test_allocator_unmap(void* pointer, size_t size);
    return rincrt_test_allocator_unmap(pointer, size);
#else
    intptr_t result;
    if (!pointer || size == 0u) {
        g_rincrt_errno = RINCRT_EINVAL;
        return -1;
    }
    result = rincrt_raw_syscall2(RIN_SYS_MUNMAP, (uintptr_t)pointer,
                                 (uintptr_t)size);
    return rincrt_status_result(result);
#endif
}

int rin_user_allocator_backend_protect(void* pointer, size_t size, int prot)
{
#if defined(RINCRT_HOST_TEST)
    extern int rincrt_test_allocator_protect(void* pointer, size_t size,
                                             int prot);
    return rincrt_test_allocator_protect(pointer, size, prot);
#else
    intptr_t result;
    if (!pointer || size == 0u) {
        g_rincrt_errno = RINCRT_EINVAL;
        return -1;
    }
    result = rincrt_raw_syscall3(RIN_SYS_MPROTECT, (uintptr_t)pointer,
                                 (uintptr_t)size, (uintptr_t)prot);
    return rincrt_status_result(result);
#endif
}

static intptr_t rincrt_posix_result(intptr_t result)
{
    if (result < 0) {
        if (result >= -4095) g_rincrt_errno = (int)-result;
        else g_rincrt_errno = RINCRT_EIO;
        return -1;
    }
    return result;
}

/* Public bootstrap APIs expose `int`/`long`, but the syscall result remains
 * an intptr_t until the ABI boundary is validated.  Never let a positive
 * target word wrap into a successful negative descriptor or PID. */
static int rincrt_result_to_int(intptr_t result)
{
    result = rincrt_posix_result(result);
    if (result < 0) return -1;
    if (result > (intptr_t)INT_MAX) {
        g_rincrt_errno = RINCRT_ERANGE;
        return -1;
    }
    return (int)result;
}

static int rincrt_status_result(intptr_t result)
{
    result = rincrt_posix_result(result);
    if (result < 0) return -1;
    if (result != 0) {
        g_rincrt_errno = RINCRT_EIO;
        return -1;
    }
    return 0;
}

void* memset(void* destination, int value, size_t length)
{
    unsigned char* output = (unsigned char*)destination;
    while (length-- != 0u) *output++ = (unsigned char)value;
    return destination;
}

void* memcpy(void* destination, const void* source, size_t length)
{
    unsigned char* output = (unsigned char*)destination;
    const unsigned char* input = (const unsigned char*)source;
    while (length-- != 0u) *output++ = *input++;
    return destination;
}

void* memchr(const void* memory, int character, size_t length)
{
    const unsigned char* input = (const unsigned char*)memory;
    unsigned char expected = (unsigned char)character;
    while (length-- != 0u) {
        if (*input == expected) return (void*)(uintptr_t)input;
        ++input;
    }
    return (void*)0;
}

size_t strlen(const char* text)
{
    size_t length = 0u;
    while (text[length] != '\0') ++length;
    return length;
}

int strcmp(const char* left, const char* right)
{
    while (*left != '\0' && *left == *right) {
        ++left;
        ++right;
    }
    return (int)(unsigned char)*left - (int)(unsigned char)*right;
}

int strncmp(const char* left, const char* right, size_t length)
{
    while (length != 0u && *left != '\0' && *left == *right) {
        ++left;
        ++right;
        --length;
    }
    return length == 0u ? 0 :
        (int)(unsigned char)*left - (int)(unsigned char)*right;
}

char* strcpy(char* destination, const char* source)
{
    char* result = destination;
    while ((*destination++ = *source++) != '\0') {}
    return result;
}

char* strncpy(char* destination, const char* source, size_t length)
{
    char* result = destination;
    while (length != 0u && *source != '\0') {
        *destination++ = *source++;
        --length;
    }
    while (length-- != 0u) *destination++ = '\0';
    return result;
}

char* strncat(char* destination, const char* source, size_t length)
{
    char* result = destination;
    while (*destination != '\0') ++destination;
    while (length != 0u && *source != '\0') {
        *destination++ = *source++;
        --length;
    }
    *destination = '\0';
    return result;
}

char* strchr(const char* text, int character)
{
    char expected = (char)character;
    for (;;) {
        if (*text == expected) return (char*)(uintptr_t)text;
        if (*text == '\0') return (char*)0;
        ++text;
    }
}

char* strrchr(const char* text, int character)
{
    const char* found = (const char*)0;
    char expected = (char)character;
    do {
        if (*text == expected) found = text;
    } while (*text++ != '\0');
    return (char*)(uintptr_t)found;
}

int isdigit(int character)
{
    return character >= '0' && character <= '9';
}

int isalpha(int character)
{
    return (character >= 'a' && character <= 'z') ||
           (character >= 'A' && character <= 'Z');
}

int isalnum(int character)
{
    return isalpha(character) || isdigit(character);
}

int isspace(int character)
{
    return character == ' ' || character == '\t' || character == '\n' ||
           character == '\r' || character == '\f' || character == '\v';
}

int isxdigit(int character)
{
    return isdigit(character) ||
           (character >= 'a' && character <= 'f') ||
           (character >= 'A' && character <= 'F');
}

int tolower(int character)
{
    return character >= 'A' && character <= 'Z'
        ? character + ('a' - 'A') : character;
}

int atoi(const char* text)
{
    int negative = 0;
    int value = 0;
    while (isspace((unsigned char)*text)) ++text;
    if (*text == '-' || *text == '+') negative = *text++ == '-';
    while (isdigit((unsigned char)*text)) {
        value = value * 10 + (*text++ - '0');
    }
    return negative ? -value : value;
}

static int rincrt_digit_value(int character)
{
    if (character >= '0' && character <= '9') return character - '0';
    if (character >= 'a' && character <= 'z') return character - 'a' + 10;
    if (character >= 'A' && character <= 'Z') return character - 'A' + 10;
    return -1;
}

unsigned long long strtoull(const char* text, char** end, int base)
{
    const char* original = text;
    unsigned long long value = 0u;
    const unsigned long long maximum = ~(unsigned long long)0;
    int negative = 0;
    int any = 0;
    int overflow = 0;

    while (isspace((unsigned char)*text)) ++text;
    if (*text == '-' || *text == '+') negative = *text++ == '-';
    if ((base == 0 || base == 16) && text[0] == '0' &&
        (text[1] == 'x' || text[1] == 'X') &&
        rincrt_digit_value((unsigned char)text[2]) >= 0 &&
        rincrt_digit_value((unsigned char)text[2]) < 16) {
        base = 16;
        text += 2;
    } else if (base == 0) {
        base = text[0] == '0' ? 8 : 10;
    }
    if (base < 2 || base > 36) {
        g_rincrt_errno = RINCRT_EINVAL;
        if (end) *end = (char*)(uintptr_t)original;
        return 0u;
    }
    for (;;) {
        int digit = rincrt_digit_value((unsigned char)*text);
        if (digit < 0 || digit >= base) break;
        any = 1;
        if (value > (maximum - (unsigned int)digit) /
                    (unsigned int)base) {
            overflow = 1;
            value = maximum;
        } else if (!overflow) {
            value = value * (unsigned int)base + (unsigned int)digit;
        }
        ++text;
    }
    if (end) *end = (char*)(uintptr_t)(any ? text : original);
    if (overflow) g_rincrt_errno = RINCRT_ERANGE;
    return negative && !overflow ? 0u - value : value;
}

double strtod(const char* text, char** end)
{
    const char* original = text;
    double value;
    double scale;
    int negative = 0;
    int any = 0;
    int exponent = 0;
    int exponent_negative = 0;

    value = 0.0;
    scale = 0.1;
    while (isspace((unsigned char)*text)) ++text;
    if (*text == '-' || *text == '+') negative = *text++ == '-';
    while (isdigit((unsigned char)*text)) {
        any = 1;
        value = value * 10.0 + (double)(*text++ - '0');
    }
    if (*text == '.') {
        ++text;
        while (isdigit((unsigned char)*text)) {
            any = 1;
            value += (double)(*text++ - '0') * scale;
            scale *= 0.1;
        }
    }
    if (any && (*text == 'e' || *text == 'E')) {
        const char* exponent_start = text;
        ++text;
        if (*text == '-' || *text == '+') exponent_negative = *text++ == '-';
        if (!isdigit((unsigned char)*text)) {
            text = exponent_start;
        } else {
            while (isdigit((unsigned char)*text)) {
                if (exponent < 4096) exponent = exponent * 10 + (*text - '0');
                ++text;
            }
            while (exponent-- > 0) {
                value = exponent_negative ? value / 10.0 : value * 10.0;
            }
        }
    }
    if (end) *end = (char*)(uintptr_t)(any ? text : original);
    return negative ? -value : value;
}

static int rincrt_unwrap_aligned_pointer(void* pointer, void** raw_out)
{
    return rincrt_load_aligned_raw(pointer, raw_out);
}

void* malloc(size_t size)
{
    if (size == 0u) return (void*)0;
    return rin_user_allocator_malloc(size);
}

void free(void* pointer)
{
    void* raw = pointer;
    if (!pointer) return;
    (void)rincrt_unwrap_aligned_pointer(pointer, &raw);
    rin_user_allocator_free(raw);
}

void* calloc(size_t count, size_t size)
{
    size_t total;
    unsigned char* bytes;
    if (count == 0u || size == 0u) return (void*)0;
    if (count > (size_t)-1 / size) {
        g_rincrt_errno = RINCRT_ERANGE;
        return (void*)0;
    }
    total = count * size;
    bytes = (unsigned char*)malloc(total);
    if (!bytes) return (void*)0;
    for (size_t index = 0u; index < total; ++index) bytes[index] = 0u;
    return bytes;
}

void* realloc(void* pointer, size_t size)
{
    void* raw = pointer;
    size_t raw_capacity;
    size_t offset;
    size_t copy_size;
    unsigned char* replacement;
    const unsigned char* source;
    if (!pointer) return malloc(size);
    if (!rincrt_unwrap_aligned_pointer(pointer, &raw))
        return rin_user_allocator_realloc(pointer, size);
    if (size == 0u) {
        rin_user_allocator_free(raw);
        return (void*)0;
    }
    raw_capacity = rin_user_allocator_usable_size(raw);
    offset = (size_t)((unsigned char*)pointer - (unsigned char*)raw);
    copy_size = offset < raw_capacity ? raw_capacity - offset : 0u;
    replacement = (unsigned char*)rin_user_allocator_malloc(size);
    if (!replacement) return (void*)0;
    if (copy_size > size) copy_size = size;
    source = (const unsigned char*)pointer;
    for (size_t index = 0u; index < copy_size; ++index)
        replacement[index] = source[index];
    rin_user_allocator_free(raw);
    return replacement;
}

void* aligned_alloc(size_t alignment, size_t size)
{
    size_t total;
    size_t address;
    size_t aligned;
    size_t metadata_size = sizeof(RinCrtAlignedAllocationMeta);
    void* raw;
    if (alignment < sizeof(void*) ||
        (alignment & (alignment - 1u)) != 0u ||
        (size & (alignment - 1u)) != 0u)
        return (void*)0;
    if (size > (size_t)-1 - alignment ||
        size + alignment > (size_t)-1 - metadata_size)
        return (void*)0;
    total = size + alignment + metadata_size;
    raw = malloc(total);
    if (!raw) return (void*)0;
    address = (size_t)raw + metadata_size;
    if (address > (size_t)-1 - (alignment - 1u)) {
        free(raw);
        return (void*)0;
    }
    aligned = (address + alignment - 1u) & ~(alignment - 1u);
    {
        RinCrtAlignedAllocationMeta* meta =
            rincrt_aligned_meta_from_ptr((void*)aligned);
        meta->guard = rincrt_aligned_guard((void*)aligned);
        meta->raw = raw;
    }
    return (void*)aligned;
}

int posix_memalign(void** pointer_out, size_t alignment, size_t size)
{
    void* result;
    if (!pointer_out || alignment < sizeof(void*) ||
        (alignment & (alignment - 1u)) != 0u)
        return RINCRT_EINVAL;
    if (size > (size_t)-1 - (alignment - 1u)) return RINCRT_ENOMEM;
    result = aligned_alloc(alignment,
                            (size + alignment - 1u) & ~(alignment - 1u));
    if (!result) return RINCRT_ENOMEM;
    *pointer_out = result;
    return 0;
}

size_t malloc_usable_size(void* pointer)
{
    void* raw = pointer;
    size_t capacity;
    size_t offset;
    if (!pointer || !rincrt_unwrap_aligned_pointer(pointer, &raw))
        return rin_user_allocator_usable_size(pointer);
    capacity = rin_user_allocator_usable_size(raw);
    offset = (size_t)((unsigned char*)pointer - (unsigned char*)raw);
    return offset < capacity ? capacity - offset : 0u;
}

static void rincrt_swap_bytes(unsigned char* left, unsigned char* right,
                              size_t size)
{
    while (size-- != 0u) {
        unsigned char temporary = *left;
        *left++ = *right;
        *right++ = temporary;
    }
}

static void rincrt_sift_down(unsigned char* bytes, size_t start, size_t end,
                             size_t size,
                             int (*compare)(const void*, const void*))
{
    size_t root = start;
    if (start >= end) return;
    while (root <= (end - 1u) / 2u) {
        size_t child = root * 2u + 1u;
        size_t selected = root;
        if (compare(bytes + selected * size, bytes + child * size) < 0)
            selected = child;
        if (child + 1u <= end &&
            compare(bytes + selected * size, bytes + (child + 1u) * size) < 0)
            selected = child + 1u;
        if (selected == root) return;
        rincrt_swap_bytes(bytes + root * size, bytes + selected * size, size);
        root = selected;
    }
}

void qsort(void* base, size_t count, size_t size,
           int (*compare)(const void*, const void*))
{
    unsigned char* bytes = (unsigned char*)base;
    size_t start;
    size_t end;
    if (!base || !compare || size == 0u || count < 2u) return;
    start = (count - 2u) / 2u + 1u;
    while (start != 0u) {
        --start;
        rincrt_sift_down(bytes, start, count - 1u, size, compare);
    }
    end = count - 1u;
    while (end != 0u) {
        rincrt_swap_bytes(bytes, bytes + end * size, size);
        --end;
        rincrt_sift_down(bytes, 0u, end, size, compare);
    }
}

int atexit(void (*function)(void))
{
    if (!function || g_rincrt_exit_handler_count >= RINCRT_ATEXIT_CAPACITY)
        return -1;
    g_rincrt_exit_handlers[g_rincrt_exit_handler_count++] = function;
    return 0;
}

void _exit(int status)
{
    (void)rincrt_raw_syscall1(RIN_SYS_EXIT, (uintptr_t)(intptr_t)status);
    for (;;) {}
}

void exit(int status)
{
    while (g_rincrt_exit_handler_count != 0u) {
        RinCrtExitHandler function =
            g_rincrt_exit_handlers[--g_rincrt_exit_handler_count];
        function();
    }
    _exit(status);
}

int close(int descriptor)
{
    return rincrt_status_result(rincrt_raw_syscall1(
        RIN_SYS_CLOSE, (uintptr_t)(intptr_t)descriptor));
}

int fork(void)
{
    intptr_t result = rincrt_raw_syscall0(RIN_SYS_FORK);
    if (result == 0) rin_user_allocator_after_fork_child();
    return rincrt_result_to_int(result);
}

int execvp(const char* file, char* const arguments[])
{
    if (!file || !*file || !arguments) {
        g_rincrt_errno = RINCRT_EINVAL;
        return -1;
    }
    return rincrt_result_to_int(rincrt_raw_syscall3(
        RIN_SYS_EXEC, (uintptr_t)file, (uintptr_t)arguments, 0u));
}

int waitpid(int pid, int* status, int options)
{
    return rincrt_result_to_int(rincrt_raw_syscall3(
        RIN_SYS_WAIT, (uintptr_t)(intptr_t)pid, (uintptr_t)status,
        (uintptr_t)(intptr_t)options));
}

static FILE* rincrt_allocate_file(int descriptor, unsigned int flags)
{
    unsigned int index;
    for (index = 0u; index < RINCRT_FILE_CAPACITY; ++index) {
        if (!g_rincrt_file_used[index]) {
            g_rincrt_file_used[index] = 1u;
            g_rincrt_files[index].descriptor = descriptor;
            g_rincrt_files[index].eof = 0;
            g_rincrt_files[index].error = 0;
            g_rincrt_files[index].flags = flags;
            return &g_rincrt_files[index];
        }
    }
    return (FILE*)0;
}

static void rincrt_release_file(FILE* stream)
{
    unsigned int index;
    for (index = 0u; index < RINCRT_FILE_CAPACITY; ++index) {
        if (&g_rincrt_files[index] == stream) {
            g_rincrt_file_used[index] = 0u;
            return;
        }
    }
}

FILE* fopen(const char* path, const char* mode)
{
    unsigned int flags;
    int update = 0;
    size_t index;
    int descriptor;
    FILE* stream;
    if (!path || !mode || !mode[0]) {
        g_rincrt_errno = RINCRT_EINVAL;
        return (FILE*)0;
    }
    for (index = 1u; mode[index] != '\0'; ++index) {
        if (mode[index] == '+') {
            if (update) {
                g_rincrt_errno = RINCRT_EINVAL;
                return (FILE*)0;
            }
            update = 1;
        } else if (mode[index] != 'b') {
            g_rincrt_errno = RINCRT_EINVAL;
            return (FILE*)0;
        }
    }
    if (mode[0] == 'r') flags = update ? RINCRT_O_RDWR : RINCRT_O_RDONLY;
    else if (mode[0] == 'w')
        flags = (update ? RINCRT_O_RDWR : RINCRT_O_WRONLY) |
                RINCRT_O_CREAT | RINCRT_O_TRUNC;
    else if (mode[0] == 'a')
        flags = (update ? RINCRT_O_RDWR : RINCRT_O_WRONLY) |
                RINCRT_O_CREAT | RINCRT_O_APPEND;
    else {
        g_rincrt_errno = RINCRT_EINVAL;
        return (FILE*)0;
    }
    descriptor = rincrt_result_to_int(rincrt_raw_syscall3(
        RIN_SYS_OPEN, (uintptr_t)path, (uintptr_t)flags, 0666u));
    if (descriptor < 0) return (FILE*)0;
    stream = rincrt_allocate_file(descriptor, flags);
    if (!stream) {
        (void)rincrt_raw_syscall1(RIN_SYS_CLOSE,
                                  (uintptr_t)(intptr_t)descriptor);
        g_rincrt_errno = RINCRT_EMFILE;
    }
    return stream;
}

int fclose(FILE* stream)
{
    int result;
    if (!stream) {
        g_rincrt_errno = RINCRT_EINVAL;
        return RINCRT_EOF;
    }
    result = rincrt_status_result(rincrt_raw_syscall1(
        RIN_SYS_CLOSE, (uintptr_t)(intptr_t)stream->descriptor));
    if (result < 0) stream->error = 1;
    rincrt_release_file(stream);
    return result < 0 ? RINCRT_EOF : 0;
}

size_t fread(void* buffer, size_t size, size_t count, FILE* stream)
{
    size_t total;
    size_t received = 0u;
    if (!stream || (!buffer && size != 0u && count != 0u)) {
        g_rincrt_errno = RINCRT_EINVAL;
        if (stream) stream->error = 1;
        return 0u;
    }
    if (size == 0u || count == 0u) return 0u;
    if (count > ~(size_t)0 / size) {
        g_rincrt_errno = RINCRT_ERANGE;
        stream->error = 1;
        return 0u;
    }
    total = size * count;
    while (received < total) {
        intptr_t result = rincrt_posix_result(rincrt_raw_syscall3(
            RIN_SYS_READ, (uintptr_t)(intptr_t)stream->descriptor,
            (uintptr_t)((unsigned char*)buffer + received),
            (uintptr_t)(total - received)));
        if (result < 0) {
            stream->error = 1;
            break;
        }
        if (result == 0) {
            stream->eof = 1;
            break;
        }
        if ((size_t)result > total - received) {
            g_rincrt_errno = RINCRT_EIO;
            stream->error = 1;
            break;
        }
        received += (size_t)result;
    }
    return received / size;
}

size_t fwrite(const void* buffer, size_t size, size_t count, FILE* stream)
{
    size_t total;
    size_t written = 0u;
    if (!stream || (!buffer && size != 0u && count != 0u)) {
        g_rincrt_errno = RINCRT_EINVAL;
        if (stream) stream->error = 1;
        return 0u;
    }
    if (size == 0u || count == 0u) return 0u;
    if (count > ~(size_t)0 / size) {
        g_rincrt_errno = RINCRT_ERANGE;
        stream->error = 1;
        return 0u;
    }
    total = size * count;
    while (written < total) {
        intptr_t result = rincrt_posix_result(rincrt_raw_syscall3(
            RIN_SYS_WRITE, (uintptr_t)(intptr_t)stream->descriptor,
            (uintptr_t)((const unsigned char*)buffer + written),
            (uintptr_t)(total - written)));
        if (result <= 0 || (size_t)result > total - written) {
            if (result == 0) g_rincrt_errno = RINCRT_EIO;
            stream->error = 1;
            break;
        }
        written += (size_t)result;
    }
    return written / size;
}

char* fgets(char* buffer, int size, FILE* stream)
{
    int index = 0;
    if (!buffer || size <= 0 || !stream) {
        g_rincrt_errno = RINCRT_EINVAL;
        if (stream) stream->error = 1;
        return (char*)0;
    }
    while (index + 1 < size) {
        char character;
        intptr_t result = rincrt_posix_result(rincrt_raw_syscall3(
            RIN_SYS_READ, (uintptr_t)(intptr_t)stream->descriptor,
            (uintptr_t)&character, 1u));
        if (result < 0) {
            stream->error = 1;
            break;
        }
        if (result == 0) {
            stream->eof = 1;
            break;
        }
        buffer[index++] = character;
        if (character == '\n') break;
    }
    if (index == 0) return (char*)0;
    buffer[index] = '\0';
    return buffer;
}

int fseek(FILE* stream, long offset, int origin)
{
    int result;
    if (!stream) {
        g_rincrt_errno = RINCRT_EINVAL;
        return -1;
    }
    result = rincrt_status_result(rincrt_raw_syscall3(
        RIN_SYS_SEEK, (uintptr_t)(intptr_t)stream->descriptor,
        (uintptr_t)(intptr_t)offset, (uintptr_t)(intptr_t)origin));
    if (result < 0) {
        stream->error = 1;
        return -1;
    }
    stream->eof = 0;
    return 0;
}

long ftell(FILE* stream)
{
    intptr_t result;
    if (!stream) {
        g_rincrt_errno = RINCRT_EINVAL;
        return -1;
    }
    result = rincrt_posix_result(rincrt_raw_syscall3(
        RIN_SYS_SEEK, (uintptr_t)(intptr_t)stream->descriptor, 0u,
        RINCRT_SEEK_CUR));
    if (result < 0) return -1;
    if (result > (intptr_t)LONG_MAX) {
        g_rincrt_errno = RINCRT_ERANGE;
        return -1;
    }
    return (long)result;
}

int feof(FILE* stream)
{
    return stream ? stream->eof : 0;
}

int ferror(FILE* stream)
{
    return stream ? stream->error : 0;
}

static int rincrt_write_all(FILE* stream, const char* text, size_t length)
{
    return fwrite(text, 1u, length, stream) == length ? 0 : -1;
}

int fputc(int character, FILE* stream)
{
    unsigned char value = (unsigned char)character;
    return rincrt_write_all(stream, (const char*)&value, 1u) == 0
        ? (int)value : RINCRT_EOF;
}

int fputs(const char* text, FILE* stream)
{
    if (!text || !stream) {
        g_rincrt_errno = RINCRT_EINVAL;
        return RINCRT_EOF;
    }
    return rincrt_write_all(stream, text, strlen(text)) == 0
        ? 0 : RINCRT_EOF;
}

typedef struct RinCrtFormatOutput {
    char* buffer;
    size_t capacity;
    size_t count;
} RinCrtFormatOutput;

static void rincrt_format_put(RinCrtFormatOutput* output, char character)
{
    if (output->capacity != 0u && output->count + 1u < output->capacity)
        output->buffer[output->count] = character;
    ++output->count;
}

static void rincrt_format_repeat(RinCrtFormatOutput* output, char character,
                                 int count)
{
    while (count-- > 0) rincrt_format_put(output, character);
}

static int rincrt_unsigned_text(char result[32], unsigned long long value,
                                unsigned int base, int upper)
{
    const char* digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char reverse[32];
    int count = 0;
    int index;
    do {
        reverse[count++] = digits[value % base];
        value /= base;
    } while (value != 0u && count < 32);
    for (index = 0; index < count; ++index)
        result[index] = reverse[count - index - 1];
    return count;
}

static unsigned long long rincrt_format_unsigned(va_list* arguments,
                                                  int length)
{
    if (length == 2) return va_arg(*arguments, unsigned long long);
    if (length == 1) return (unsigned long long)va_arg(*arguments, unsigned long);
    if (length == 3) return (unsigned long long)va_arg(*arguments, size_t);
    return (unsigned long long)va_arg(*arguments, unsigned int);
}

static long long rincrt_format_signed(va_list* arguments, int length)
{
    if (length == 2) return va_arg(*arguments, long long);
    if (length == 1) return (long long)va_arg(*arguments, long);
    if (length == 3) return (long long)va_arg(*arguments, size_t);
    return (long long)va_arg(*arguments, int);
}

int vsnprintf(char* buffer, size_t size, const char* format, va_list arguments)
{
    RinCrtFormatOutput output;
    va_list copy;
    output.buffer = buffer;
    output.capacity = buffer ? size : 0u;
    output.count = 0u;
    va_copy(copy, arguments);
    while (format && *format != '\0') {
        int left;
        int zero;
        int width;
        int precision;
        int length;
        char conversion;
        if (*format != '%') {
            rincrt_format_put(&output, *format++);
            continue;
        }
        ++format;
        left = 0;
        zero = 0;
        while (*format == '-' || *format == '0' || *format == '+' ||
               *format == ' ' || *format == '#') {
            if (*format == '-') left = 1;
            if (*format == '0') zero = 1;
            ++format;
        }
        width = 0;
        if (*format == '*') {
            width = va_arg(copy, int);
            ++format;
            if (width < 0) {
                left = 1;
                width = -width;
            }
        } else {
            while (isdigit((unsigned char)*format))
                width = width * 10 + (*format++ - '0');
        }
        precision = -1;
        if (*format == '.') {
            ++format;
            precision = 0;
            if (*format == '*') {
                precision = va_arg(copy, int);
                ++format;
            } else {
                while (isdigit((unsigned char)*format))
                    precision = precision * 10 + (*format++ - '0');
            }
        }
        length = 0;
        if (*format == 'l') {
            length = 1;
            ++format;
            if (*format == 'l') {
                length = 2;
                ++format;
            }
        } else if (*format == 'z') {
            length = 3;
            ++format;
        } else if (*format == 'h') {
            ++format;
            if (*format == 'h') ++format;
        }
        conversion = *format ? *format++ : '\0';
        if (conversion == 's') {
            const char* text = va_arg(copy, const char*);
            int text_length = 0;
            if (!text) text = "(null)";
            while (text[text_length] != '\0' &&
                   (precision < 0 || text_length < precision)) ++text_length;
            if (!left) rincrt_format_repeat(&output, ' ', width - text_length);
            for (int index = 0; index < text_length; ++index)
                rincrt_format_put(&output, text[index]);
            if (left) rincrt_format_repeat(&output, ' ', width - text_length);
        } else if (conversion == 'c') {
            char character = (char)va_arg(copy, int);
            if (!left) rincrt_format_repeat(&output, ' ', width - 1);
            rincrt_format_put(&output, character);
            if (left) rincrt_format_repeat(&output, ' ', width - 1);
        } else if (conversion == '%') {
            rincrt_format_put(&output, '%');
        } else if (conversion == 'd' || conversion == 'i' ||
                   conversion == 'u' || conversion == 'x' ||
                   conversion == 'X' || conversion == 'o' ||
                   conversion == 'p') {
            char digits[32];
            unsigned long long magnitude;
            unsigned int base = conversion == 'o' ? 8u :
                ((conversion == 'x' || conversion == 'X' || conversion == 'p')
                    ? 16u : 10u);
            int negative = 0;
            int digit_count;
            int prefix = conversion == 'p' ? 2 : 0;
            int padding;
            if (conversion == 'd' || conversion == 'i') {
                long long signed_value = rincrt_format_signed(&copy, length);
                negative = signed_value < 0;
                magnitude = negative
                    ? 0u - (unsigned long long)signed_value
                    : (unsigned long long)signed_value;
            } else if (conversion == 'p') {
                magnitude = (unsigned long long)(uintptr_t)va_arg(copy, void*);
            } else {
                magnitude = rincrt_format_unsigned(&copy, length);
            }
            digit_count = rincrt_unsigned_text(digits, magnitude, base,
                                               conversion == 'X');
            if (precision == 0 && magnitude == 0u) digit_count = 0;
            padding = width - digit_count - prefix - negative;
            if (!left && !zero) rincrt_format_repeat(&output, ' ', padding);
            if (negative) rincrt_format_put(&output, '-');
            if (prefix) {
                rincrt_format_put(&output, '0');
                rincrt_format_put(&output, 'x');
            }
            if (!left && zero) rincrt_format_repeat(&output, '0', padding);
            if (precision > digit_count)
                rincrt_format_repeat(&output, '0', precision - digit_count);
            for (int index = 0; index < digit_count; ++index)
                rincrt_format_put(&output, digits[index]);
            if (left) rincrt_format_repeat(&output, ' ', padding);
        } else if (conversion != '\0') {
            rincrt_format_put(&output, '%');
            rincrt_format_put(&output, conversion);
        }
    }
    va_end(copy);
    if (output.capacity != 0u) {
        size_t terminator = output.count < output.capacity
            ? output.count : output.capacity - 1u;
        output.buffer[terminator] = '\0';
    }
    return output.count > 0x7fffffffu ? -1 : (int)output.count;
}

int snprintf(char* buffer, size_t size, const char* format, ...)
{
    va_list arguments;
    int result;
    va_start(arguments, format);
    result = vsnprintf(buffer, size, format, arguments);
    va_end(arguments);
    return result;
}

int vfprintf(FILE* stream, const char* format, va_list arguments)
{
    char buffer[1024];
    int result = vsnprintf(buffer, sizeof(buffer), format, arguments);
    size_t available;
    if (result < 0) return -1;
    available = (size_t)result < sizeof(buffer)
        ? (size_t)result : sizeof(buffer) - 1u;
    return rincrt_write_all(stream, buffer, available) == 0 ? result : -1;
}

int fprintf(FILE* stream, const char* format, ...)
{
    va_list arguments;
    int result;
    va_start(arguments, format);
    result = vfprintf(stream, format, arguments);
    va_end(arguments);
    return result;
}

int printf(const char* format, ...)
{
    va_list arguments;
    int result;
    va_start(arguments, format);
    result = vfprintf(&g_rincrt_stdout, format, arguments);
    va_end(arguments);
    return result;
}

void perror(const char* text)
{
    const char* message = "unknown error";
    if (g_rincrt_errno == RINCRT_EINTR) message = "interrupted";
    else if (g_rincrt_errno == RINCRT_EIO) message = "I/O error";
    else if (g_rincrt_errno == RINCRT_EBADF) message = "bad file descriptor";
    else if (g_rincrt_errno == RINCRT_ENOMEM) message = "out of memory";
    else if (g_rincrt_errno == RINCRT_EEXIST) message = "already exists";
    else if (g_rincrt_errno == RINCRT_EINVAL) message = "invalid argument";
    else if (g_rincrt_errno == RINCRT_EMFILE) message = "too many open files";
    else if (g_rincrt_errno == RINCRT_ERANGE) message = "result out of range";
    if (text && *text) {
        (void)fputs(text, &g_rincrt_stderr);
        (void)fputs(": ", &g_rincrt_stderr);
    }
    (void)fputs(message, &g_rincrt_stderr);
    (void)fputc('\n', &g_rincrt_stderr);
}

int remove(const char* path)
{
    return rincrt_status_result(rincrt_raw_syscall1(
        RIN_SYS_UNLINK, (uintptr_t)path));
}

int rename(const char* old_path, const char* new_path)
{
    return rincrt_status_result(rincrt_raw_syscall2(
        RIN_SYS_RENAME, (uintptr_t)old_path, (uintptr_t)new_path));
}

static int rincrt_random_bytes(unsigned char* output, size_t size)
{
    while (size != 0u) {
        intptr_t result = rincrt_posix_result(rincrt_raw_syscall3(
            RINCRT_GETRANDOM, (uintptr_t)output, (uintptr_t)size, 0u));
        if (result <= 0 || (size_t)result > size) {
            if (result == 0) g_rincrt_errno = RINCRT_EIO;
            return -1;
        }
        output += (size_t)result;
        size -= (size_t)result;
    }
    return 0;
}

int mkstemp(char* template_name)
{
    static const char alphabet[] =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    unsigned char random[6];
    size_t length;
    char* suffix;
    int attempt;
    if (!template_name) {
        g_rincrt_errno = RINCRT_EINVAL;
        return -1;
    }
    length = strlen(template_name);
    if (length < 6u) {
        g_rincrt_errno = RINCRT_EINVAL;
        return -1;
    }
    suffix = template_name + length - 6u;
    for (attempt = 0; attempt < 6; ++attempt) {
        if (suffix[attempt] != 'X') {
            g_rincrt_errno = RINCRT_EINVAL;
            return -1;
        }
    }
    for (attempt = 0; attempt < 100; ++attempt) {
        intptr_t descriptor;
        if (rincrt_random_bytes(random, sizeof(random)) != 0) return -1;
        for (int index = 0; index < 6; ++index)
            suffix[index] = alphabet[random[index] % 62u];
        descriptor = rincrt_raw_syscall3(
            RIN_SYS_OPEN, (uintptr_t)template_name,
            (uintptr_t)(RINCRT_O_RDWR | RINCRT_O_CREAT | RINCRT_O_EXCL), 0600u);
        if (descriptor >= 0) return rincrt_result_to_int(descriptor);
        if (descriptor != -RINCRT_EEXIST) {
            (void)rincrt_posix_result(descriptor);
            return -1;
        }
    }
    g_rincrt_errno = RINCRT_EEXIST;
    return -1;
}
