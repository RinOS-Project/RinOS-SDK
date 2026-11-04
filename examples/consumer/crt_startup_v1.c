/* SPDX-License-Identifier: MIT */
/* Minimal process-entry contract used by RinOS-SDK target consumers. */

typedef __UINTPTR_TYPE__ RinSdkWord;
typedef void (*RinSdkArrayFunction)(void);
typedef struct RinSdkAuxiliaryEntry {
    RinSdkWord type;
    RinSdkWord value;
} RinSdkAuxiliaryEntry;

extern char **environ;
extern int main(int argc, char **argv, char **envp);
extern int atexit(void (*function)(void));
extern void _exit(int status) __attribute__((noreturn));
extern void rin_exit(int status) __attribute__((noreturn));
extern int __rin_run_startup_dependency_initializers(void);
extern int __rin_run_process_dependency_finalizers(void);
extern RinSdkArrayFunction __init_array_start[];
extern RinSdkArrayFunction __init_array_end[];
extern RinSdkArrayFunction __fini_array_start[];
extern RinSdkArrayFunction __fini_array_end[];

#define RIN_SDK_STARTUP_MAX_ARGUMENTS 64u
#define RIN_SDK_STARTUP_MAX_ENVIRONMENT 128u
#define RIN_SDK_STARTUP_MAX_ARRAY_ENTRIES 4096u
#define RIN_SDK_STARTUP_MAX_AUXV_ENTRIES 64u

static RinSdkAuxiliaryEntry rin_sdk_auxiliary_entries[
    RIN_SDK_STARTUP_MAX_AUXV_ENTRIES];
static RinSdkWord rin_sdk_auxiliary_entry_count;
static RinSdkWord rin_sdk_fini_array_count;

__attribute__((visibility("hidden")))
int __rin_auxv_lookup(unsigned long type, unsigned long *value)
{
    RinSdkWord index;
    if (!value) return -1;
    if ((RinSdkWord)type == 0u) return 1;
    for (index = 0u; index < rin_sdk_auxiliary_entry_count; ++index) {
        if (rin_sdk_auxiliary_entries[index].type == (RinSdkWord)type) {
            *value = (unsigned long)rin_sdk_auxiliary_entries[index].value;
            return 0;
        }
    }
    return 1;
}

static int rin_sdk_array_bounds(
    RinSdkArrayFunction *begin,
    RinSdkArrayFunction *end,
    RinSdkWord *count_out)
{
    RinSdkWord first = (RinSdkWord)begin;
    RinSdkWord last = (RinSdkWord)end;
    RinSdkWord bytes;

    if (first == 0u && last == 0u) {
        *count_out = 0u;
        return 0;
    }
    if (first == 0u || last < first ||
        (last - first) % sizeof(RinSdkArrayFunction) != 0u) {
        return -1;
    }
    bytes = last - first;
    if (bytes / sizeof(RinSdkArrayFunction) >
        RIN_SDK_STARTUP_MAX_ARRAY_ENTRIES) {
        return -1;
    }
    *count_out = bytes / sizeof(RinSdkArrayFunction);
    return 0;
}

static void rin_sdk_run_finalizers(void)
{
    while (rin_sdk_fini_array_count != 0u) {
        RinSdkArrayFunction function =
            __fini_array_start[--rin_sdk_fini_array_count];
        if (function != (RinSdkArrayFunction)0) function();
    }
}

#if defined(__x86_64__)
static void rin_sdk_run_dependency_finalizers(void)
{
    if (__rin_run_process_dependency_finalizers() != 0) _exit(127);
}
#endif

static int rin_sdk_snapshot_auxiliary_vector(RinSdkWord *auxiliary)
{
    RinSdkWord index;
    if (!auxiliary) return -1;
    for (index = 0u; index < RIN_SDK_STARTUP_MAX_AUXV_ENTRIES; ++index) {
        RinSdkWord type = auxiliary[index * 2u];
        RinSdkWord value = auxiliary[index * 2u + 1u];
        if (type == 0u) {
            if (value != 0u) return -1;
            rin_sdk_auxiliary_entry_count = index;
            return 0;
        }
        rin_sdk_auxiliary_entries[index].type = type;
        rin_sdk_auxiliary_entries[index].value = value;
    }
    return -1;
}

__attribute__((noreturn))
void __rin_sdk_start_from_initial_stack(RinSdkWord *initial_stack)
{
    RinSdkWord argument_count;
    char **arguments;
    char **environment;
    RinSdkWord environment_count = 0u;
    char **environment_end;
    RinSdkWord *auxiliary;
    RinSdkWord initializer_count;

    if (!initial_stack) rin_exit(127);
    argument_count = initial_stack[0];
    if (argument_count > RIN_SDK_STARTUP_MAX_ARGUMENTS) rin_exit(127);
    arguments = (char **)&initial_stack[1];
    if (arguments[argument_count] != (char *)0) rin_exit(127);

    environment = &arguments[argument_count + 1u];
    environment_end = environment;
    while (environment_count < RIN_SDK_STARTUP_MAX_ENVIRONMENT &&
           *environment_end != (char *)0) {
        ++environment_end;
        ++environment_count;
    }
    if (*environment_end != (char *)0) rin_exit(127);
    auxiliary = (RinSdkWord *)(environment_end + 1);
    if (rin_sdk_snapshot_auxiliary_vector(auxiliary) != 0) rin_exit(127);

    environ = environment;
    if (rin_sdk_array_bounds(__init_array_start, __init_array_end,
                             &initializer_count) != 0 ||
        rin_sdk_array_bounds(__fini_array_start, __fini_array_end,
                             &rin_sdk_fini_array_count) != 0) {
        rin_exit(127);
    }
#if defined(__x86_64__)
    /* Register first so dependency finalizers run after the root image. */
    if (atexit(rin_sdk_run_dependency_finalizers) != 0 ||
        __rin_run_startup_dependency_initializers() != 0) {
        rin_exit(127);
    }
#endif
    /* Install the image finalizer before any constructor can call exit(). */
    if (atexit(rin_sdk_run_finalizers) != 0) rin_exit(127);
    for (RinSdkWord index = 0u; index < initializer_count; ++index) {
        RinSdkArrayFunction function = __init_array_start[index];
        if (function != (RinSdkArrayFunction)0) function();
    }
    rin_exit(main((int)argument_count, arguments, environment));
}

__attribute__((naked, noreturn)) void _start(void)
{
#if defined(__x86_64__)
    __asm__ __volatile__(
        "mov %rsp, %rdi\n"
        "andq $-16, %rsp\n"
        "call __rin_sdk_start_from_initial_stack\n"
        "ud2\n");
#elif defined(__i386__)
    __asm__ __volatile__(
        "mov %esp, %eax\n"
        "and $-16, %esp\n"
        "sub $12, %esp\n"
        "push %eax\n"
        "call __rin_sdk_start_from_initial_stack\n"
        "ud2\n");
#else
#error unsupported RinOS-SDK startup architecture
#endif
}
