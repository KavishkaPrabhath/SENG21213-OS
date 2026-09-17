/* =============================================================================
 * SENG21213-OS :: Main Kernel  (Stage 0 – Foundations)
 * File   : kernel/kernel.c
 *
 * PURPOSE
 *   This is the heart of your operating system. Right now it:
 *     1. Initialises VGA text-mode display
 *     2. Initialises the keyboard driver
 *     3. Prints a splash screen
 *     4. Runs a minimal interactive shell ("ksh")
 *
 * ASSIGNMENT MILESTONES  (what YOU will add in later lectures)
 *   Lecture  9  – Process Management  →  process.h / process.c / scheduler.c
 *   Lecture 10  – Threads             →  thread.h  / thread.c
 *   Lecture 11  – Memory Management   →  pmm.h     / pmm.c / vmm.c
 *   Lecture 12  – File System         →  fs.h      / fs.c
 *
 * CODING CONVENTION
 *   - Prefix kernel-internal functions with k_ (e.g. k_strcmp)
 *   - All driver APIs live in their own .h/.c pair
 *   - NEVER call malloc – use the PMM you build in Lecture 11
 * ============================================================================*/

#include "vga.h"
#include "keyboard.h"
#include "process.h"
#include "thread.h"
#include "mutex.h"
#include "semaphore.h"
#include "pmm.h"
#include "vmm.h"
#include "fs.h"
#include "timer.h"
#include "interrupts.h"
#include "../include/types.h"

static void print_uint(uint32_t value)
{
    char buffer[11];
    int i = 0;

    if (value == 0) {
        vga_puts("0");
        return;
    }

    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0) {
        char digit[2];
        digit[0] = buffer[--i];
        digit[1] = '\0';
        vga_puts(digit);
    }
}

/* ---------------------------------------------------------------------------
 * Forward declarations of shell commands
 * --------------------------------------------------------------------------*/
static void cmd_help(void);
static void print_uint(uint32_t value);
static void cmd_clear(void);
static void cmd_about(void);
static void cmd_echo(const char *args);
static void cmd_mem(void);
static void cmd_ps(void);
static void cmd_threads(void);
static void cmd_ls(void);
static void cmd_cat(const char *filename);

/* ---------------------------------------------------------------------------
 * Utility: minimal string helpers (no libc in a freestanding kernel!)
 * --------------------------------------------------------------------------*/
static int k_strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) { a++; b++; }
    return (uint8_t)*a - (uint8_t)*b;
}

static int k_strncmp(const char *a, const char *b, size_t n) {
    while (n-- && *a && (*a == *b)) { a++; b++; }
    return n == (size_t)-1 ? 0 : (uint8_t)*a - (uint8_t)*b;
}

static size_t k_strlen(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

/* Skip leading spaces */
static const char *k_ltrim(const char *s) {
    while (*s == ' ') s++;
    return s;
}

/* ---------------------------------------------------------------------------
 * Splash Screen
 * --------------------------------------------------------------------------*/
static void print_splash(void) {
    vga_clear(VGA_BLACK);

    /* Top banner box */
    vga_draw_box(0, 0, 7, 80, VGA_LIGHT_MAGENTA);

    vga_set_cursor(1, 2);
    vga_puts_color("  SENG21213-OS  |  Computer Architecture & Operating Systems",
                   VGA_YELLOW, VGA_BLACK);

    vga_set_cursor(2, 2);
    vga_puts_color("  Stage 0: Kernel Foundations", VGA_LIGHT_CYAN, VGA_BLACK);

    vga_set_cursor(3, 2);
    vga_puts_color("  Faculty of Engineering – Department of Software Engineering",
                   VGA_LIGHT_GREY, VGA_BLACK);

    vga_set_cursor(4, 2);
    vga_puts_color("  Built by students, for students.  Type 'help' to begin.",
                   VGA_LIGHT_GREEN, VGA_BLACK);

    vga_set_cursor(5, 2);
    vga_puts_color("  CPU: i686 (32-bit Protected Mode)  |  Display: VGA 80x25",
                   VGA_DARK_GREY, VGA_BLACK);

    vga_set_cursor(8, 0);
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("  Welcome! This kernel was compiled from source and booted entirely\n");
    vga_puts("  from bare metal. There is no Linux or Windows underneath – only\n");
    vga_puts("  the code you and your team write.\n");
    vga_puts("\n");
    vga_puts("  Assignment milestones to implement:\n");
    vga_puts_color("    [L09] ", VGA_YELLOW, VGA_BLACK);
    vga_puts("Process Management  – PCB, ready queue, round-robin scheduler\n");
    vga_puts_color("    [L10] ", VGA_YELLOW, VGA_BLACK);
    vga_puts("Threads & Sync      – kernel threads, mutex, semaphore\n");
    vga_puts_color("    [L11] ", VGA_YELLOW, VGA_BLACK);
    vga_puts("Memory Management   – physical page allocator, virtual memory\n");
    vga_puts_color("    [L12] ", VGA_YELLOW, VGA_BLACK);
    vga_puts("File System         – RAM disk, FAT-like directory structure\n");
    vga_puts("\n");
}

/* ---------------------------------------------------------------------------
 * Shell command implementations
 * --------------------------------------------------------------------------*/
static void cmd_help(void) {
    vga_puts_color("\n  SENG21213-OS Shell Commands\n", VGA_YELLOW, VGA_BLACK);
    vga_puts("  ─────────────────────────────────────────────\n");
    vga_puts("  help    – Show this help message\n");
    vga_puts("  clear   – Clear the screen\n");
    vga_puts("  about   – About this OS and course\n");
    vga_puts("  echo    – Echo text to screen\n");
    vga_puts("  mem     – Memory map (stub)\n");
    vga_puts_color("\n  Milestones (to implement):\n", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("  ps      – [L09] List processes\n");
    vga_puts("  kill    – [L09] Terminate a process\n");
    vga_puts("  threads – [L10] List kernel threads\n");
    vga_puts("  free    – [L11] Show free memory\n");
    vga_puts("  ls      – [L12] List files\n");
    vga_puts("  cat     – [L12] Print file contents\n\n");
}

static void cmd_clear(void) {
    vga_clear(VGA_BLACK);
}

static void cmd_about(void) {
    vga_puts_color("\n  About SENG21213-OS\n", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("  ─────────────────────────────────────────────\n");
    vga_puts("  Architecture : x86 (i686), 32-bit Protected Mode\n");
    vga_puts("  Bootloader   : Custom MBR (NASM)\n");
    vga_puts("  Kernel       : Freestanding C (GCC, no libc)\n");
    vga_puts("  VM Target    : QEMU (qemu-system-i386)\n");
    vga_puts("  Course       : SENG 21213 – Sem 2\n");
    vga_puts("  Reference    : Stallings, OS: Internals & Design Principles\n\n");
}

static void cmd_echo(const char *args) {
    vga_puts("  ");
    vga_puts(args);
    vga_puts("\n");
}

static void cmd_mem(void) {
    /* Stage 0 stub – students implement the real PMM in Lecture 11 */
    vga_puts_color("\n  Memory Map (stub – implement PMM in Lecture 11)\n",
                   VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("  ─────────────────────────────────────────────\n");
    vga_puts("  0x00000000 – 0x000FFFFF  :  First 1 MB (reserved/BIOS)\n");
    vga_puts("  0x00100000 – 0x00EFFFFF  :  Extended memory (usable ~14 MB)\n");
    vga_puts("  0x00F00000 – 0x00FFFFFF  :  BIOS / ROM area\n");
    vga_puts("  0xB8000    – 0xBFFFF     :  VGA frame buffer\n");
    vga_puts_color("\n  TODO: Use BIOS int 0x15, EAX=0xE820 to get real memory map\n\n",
                   VGA_YELLOW, VGA_BLACK);
}

static const char *process_state_name(proc_state_t state)
{
    switch (state) {
        case READY:      return "READY";
        case RUNNING:    return "RUNNING";
        case BLOCKED:    return "BLOCKED";
        case TERMINATED: return "TERMINATED";
        default:         return "UNKNOWN";
    }
}

static void cmd_ps(void)
{
    uint32_t i;
    const pcb_t *process;

    vga_puts_color("\n  Process List\n", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("  PID     STATE\n");
    vga_puts("  ----------------\n");

    for (i = 0; i < MAX_PROCESSES; i++) {
        process = process_get(i);

        if (process != 0 && process->pid != 0) {
            vga_puts("  ");

            /* Stage 1 supports PIDs 1-16. */
            if (process->pid >= 10) {
                vga_putchar('0' + (process->pid / 10));
            }

            vga_putchar('0' + (process->pid % 10));
            vga_puts("       ");
            vga_puts(process_state_name(process->state));
            vga_puts("\n");
        }
    }

    vga_puts("\n");
}

static void cmd_threads(void)
{
    uint32_t i;

    vga_puts_color("\n  Thread List\n", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("  TID     OWNER     STATE\n");
    vga_puts("  -----------------------\n");

    for (i = 0; i < MAX_THREADS; i++) {
        const thread_t *thread = thread_get(i);

        if (thread != 0) {
            vga_puts("  ");

            if (thread->tid >= 10)
                vga_putchar('0' + (thread->tid / 10));

            vga_putchar('0' + (thread->tid % 10));
            vga_puts("       ");

            if (thread->owner_pid >= 10)
                vga_putchar('0' + (thread->owner_pid / 10));

            vga_putchar('0' + (thread->owner_pid % 10));
            vga_puts("         ");

            vga_puts(thread_state_name(thread->state));
            vga_puts("\n");
        }
    }

    vga_puts("\n");
}

static void cmd_ls(void)
{
    uint32_t i;
    uint32_t count = 0;

    vga_puts_color("\n  Files\n", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("  ------------------------------\n");

    for (i = 0; i < FS_MAX_FILES; i++) {
        fs_file_t *file = fs_get_file(i);

        if (file != 0 && file->used) {
            vga_puts("  ");
            vga_puts(file->name);
            vga_puts("  (");
            print_uint(file->size);
            vga_puts(" bytes)\n");
            count++;
        }
    }

    if (count == 0) {
        vga_puts("  No files found.\n");
    }

    vga_puts("\n");
}

static void cmd_cat(const char *filename)
{
    fs_file_t *file;

    filename = k_ltrim(filename);

    if (*filename == '\0') {
        vga_puts("Usage: cat <filename>\n");
        return;
    }

    file = fs_find(filename);

    if (file == 0) {
        vga_puts("File not found: ");
        vga_puts(filename);
        vga_puts("\n");
        return;
    }

    vga_puts(file->data);
    vga_puts("\n");
}

/* ---------------------------------------------------------------------------
 * Shell process
 * --------------------------------------------------------------------------*/
static char  shell_buf[256];
static char  prompt[] = "\n  ksh> ";

static void shell_run(void) {
    vga_puts_color("\n  Kernel Shell ready. Type 'help' for commands.\n",
                   VGA_LIGHT_GREEN, VGA_BLACK);

    while (true) {
        vga_puts_color(prompt, VGA_LIGHT_GREEN, VGA_BLACK);
        kb_readline(shell_buf, sizeof(shell_buf));

        /* Trim leading whitespace */
        const char *cmd = k_ltrim(shell_buf);
        if (k_strlen(cmd) == 0) continue;

        /* Dispatch */
        if (k_strcmp(cmd, "help")  == 0) { cmd_help();  continue; }
        if (k_strcmp(cmd, "clear") == 0) { cmd_clear(); continue; }
        if (k_strcmp(cmd, "about") == 0) { cmd_about(); continue; }
        if (k_strcmp(cmd, "mem")   == 0) { cmd_mem();   continue; }

        if (k_strncmp(cmd, "echo ", 5) == 0) {
            cmd_echo(k_ltrim(cmd + 5));
            continue;
        }

        /* Stage 1: Process Management */
        if (k_strcmp(cmd, "ps") == 0) {
            cmd_ps();
            continue;
        }

        if (k_strcmp(cmd, "yield") == 0) {
    /* process_yield(); -- disabled: PIT scheduler performs the real context switch */
           vga_puts("Process yielded.\n");
           continue;
        }

	/* Stage 2: Threads */
		if (k_strcmp(cmd, "threads") == 0) {
    		cmd_threads();
   	 	continue;
	}

	/* Stage 3: Memory Management */
if (k_strcmp(cmd, "free") == 0) {
    vga_puts("Total pages : ");
    print_uint(pmm_total_pages());
    vga_puts("\n");

    vga_puts("Used pages  : ");
    print_uint(pmm_used_pages());
    vga_puts("\n");

    vga_puts("Free pages  : ");
    print_uint(pmm_free_pages());
    vga_puts("\n");

    continue;
}	

	/* Stage 4: File System */
	if (k_strcmp(cmd, "ls") == 0) {
    		cmd_ls();
    		continue;
	}

	if (k_strncmp(cmd, "cat ", 4) == 0) {
 	    cmd_cat(k_ltrim(cmd + 4));
	    continue;
	}

        /* Milestone stubs */
        if (k_strcmp(cmd, "kill")    == 0) {
            vga_puts_color("  [TODO] This command is not yet implemented.\n",
                           VGA_YELLOW, VGA_BLACK);
            vga_puts("  Implement it as part of your lecture assignment.\n");
            continue;
        }

        vga_puts_color("  Unknown command: ", VGA_LIGHT_RED, VGA_BLACK);
        vga_puts(cmd);
        vga_puts("\n  Type 'help' for a list of commands.\n");
    }
}

static void test_process(void)
{
    while (true) {
        /* Stage 1 test process */
    }
}

/*
 * Stage 2 synchronization demonstration.
 * Both kernel threads update the same global value.
 * The mutex protects the critical section.
 */
static volatile uint32_t myglobal = 0;
static mutex_t test_mutex;
static semaphore_t test_semaphore;

static void test_thread(void)
{
    while (true) {
        mutex_lock(&test_mutex);

        /* Critical section: shared global data. */
        myglobal++;

        mutex_unlock(&test_mutex);

        /* Allow the timer scheduler to run another context. */
        thread_yield();
    }
}

/* ---------------------------------------------------------------------------
 * Kernel entry point – called from kernel_entry.asm
 * --------------------------------------------------------------------------*/
void kernel_main(void) {
    vga_init();
    kb_init();
    process_init();
    thread_init();
    interrupts_init();
    timer_init(100);
    pmm_init();
    vmm_init();
    fs_init();
    fs_create("welcome.txt");
    fs_write("welcome.txt", "Welcome to SENG21213-OS File System!");

    fs_create("readme.txt");
    fs_write("readme.txt", "Stage 4 RAM-based file system is working.");
    process_create(test_process);
    process_create(test_process);

    /* Stage 2 synchronization objects. */
    mutex_init(&test_mutex);
    semaphore_init(&test_semaphore, 1);

    /* Two kernel threads share myglobal through the mutex. */
    thread_create(1, test_thread);
    thread_create(1, test_thread);
    /* process_yield(); -- disabled: PIT scheduler performs the real context switch */
    interrupts_enable();
    print_splash();
    shell_run();

    /* Should never reach here */
    __asm__ __volatile__("hlt");
}
