/* SPDX-License-Identifier: MIT */
/* Minimal process-entry contract used by RinOS-SDK target consumers. */

typedef __UINTPTR_TYPE__ RinSdkWord;
typedef void (*RinSdkArrayFunction)(void);

extern char **environ;
extern int main(int argc, char **argv, char **envp);
extern int atexit(void (*function)(void));
extern void rin_exit(int status) __attribute__((noreturn));
extern RinSdkArrayFunction __init_array_start[];
extern RinSdkArrayFunction __init_array_end[];
extern RinSdkArrayFunction __fini_array_start[];
extern RinSdkArrayFunction __fini_array_end[];

#define RIN_SDK_STARTUP_MAX_ARGUMENTS 64u
#define RIN_SDK_STARTUP_MAX_ENVIRONMENT 128u
#define RIN_SDK_STARTUP_MAX_ARRAY_ENTRIES 4096u
#define RIN_SDK_STARTUP_MAX_AUXV_ENTRIES 64u

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
    RinSdkWord count;
    if (rin_sdk_array_bounds(__fini_array_start, __fini_array_end,
                             &count) != 0) {
        return;
    }
    while (count != 0u) {
        RinSdkArrayFunction function = __fini_array_start[--count];
        if (function != (RinSdkArrayFunction)0) function();
    }
}

__attribute__((noreturn)) void _start(void)
{
    RinSdkWord *initial_stack = (RinSdkWord *)0;
    RinSdkWord argument_count;
    char **arguments;
    char **environment;
    char **environment_end;
    RinSdkWord *auxiliary;
    RinSdkWord initializer_count;

#if defined(__x86_64__)
    __asm__ volatile("lea 8(%%rbp), %0"
                     : "=r"(initial_stack) : : "memory");
#elif defined(__i386__)
    __asm__ volatile("lea 4(%%ebp), %0"
                     : "=r"(initial_stack) : : "memory");
#else
#error unsupported RinOS-SDK startup architecture
#endif

    if (initial_stack == (RinSdkWord *)0) rin_exit(127);
    argument_count = initial_stack[0];
    if (argument_count > RIN_SDK_STARTUP_MAX_ARGUMENTS) rin_exit(127);
    arguments = (char **)&initial_stack[1];
    if (arguments[argument_count] != (char *)0) rin_exit(127);

    environment = &arguments[argument_count + 1u];
    environment_end = environment;
    while ((RinSdkWord)(environment_end - environment) <=
           RIN_SDK_STARTUP_MAX_ENVIRONMENT &&
           *environment_end != (char *)0) {
        ++environment_end;
    }
    if ((RinSdkWord)(environment_end - environment) >
        RIN_SDK_STARTUP_MAX_ENVIRONMENT) {
        rin_exit(127);
    }
    auxiliary = (RinSdkWord *)(environment_end + 1);
    {
        RinSdkWord entry;
        for (entry = 0u; entry < RIN_SDK_STARTUP_MAX_AUXV_ENTRIES; ++entry) {
            if (auxiliary[entry * 2u] == 0u) break;
        }
        if (entry == RIN_SDK_STARTUP_MAX_AUXV_ENTRIES ||
            auxiliary[entry * 2u + 1u] != 0u) {
            rin_exit(127);
        }
    }

    environ = environment;
    if (rin_sdk_array_bounds(__init_array_start, __init_array_end,
                             &initializer_count) != 0) {
        rin_exit(127);
    }
    for (RinSdkWord index = 0u; index < initializer_count; ++index) {
        RinSdkArrayFunction function = __init_array_start[index];
        if (function != (RinSdkArrayFunction)0) function();
    }
    if (atexit(rin_sdk_run_finalizers) != 0) rin_exit(127);
    rin_exit(main((int)argument_count, arguments, environment));
}
