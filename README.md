# SENG21213-OS — Operating Systems Project

> **Course:** SENG 21213 – Computer Architecture & Operating Systems  
> **Year:** 2nd Year, Software Engineering  
> **Project:** Build Your Own x86 Operating System  
> **Status:** Stage 0 – Stage 4 Completed

---

## Project Overview

SENG21213-OS is a small educational x86 operating system developed through five project milestones.

The project starts with a bootable kernel and progressively adds process management, scheduling, threads, synchronization, memory management, and a simple RAM-based file system.

The operating system is written mainly in C and x86 Assembly and can be built and tested using NASM, GCC, GNU Make, and QEMU.

---

## Completed Milestones

| Stage | Milestone | Status |
|------|-----------|--------|
| Stage 0 | Boot, VGA and Kernel Shell | Completed |
| Stage 1 | Process Management and Round-Robin Scheduler | Completed |
| Stage 2 | Threads and Mutex Synchronization | Completed |
| Stage 3 | Physical and Virtual Memory Management | Completed |
| Stage 4 | RAM-Based File System | Completed |

---

## Stage 0 — Boot and Kernel Shell

Stage 0 provides the basic foundation of the operating system.

Implemented features include:

- MBR bootloader
- 16-bit real mode to 32-bit protected mode transition
- Global Descriptor Table (GDT)
- Kernel entry point
- VGA text-mode output
- PS/2 keyboard input
- Interactive kernel shell

---

## Stage 1 — Process Management

Stage 1 introduces basic process management and scheduling.

Implemented features include:

- Process Control Blocks (PCB)
- Process creation
- Process states
- Process listing
- Round-Robin scheduling
- Scheduler integration

---

## Stage 2 — Threads and Synchronization

Stage 2 introduces kernel threads and synchronization.

Implemented features include:

- Kernel thread management
- Thread creation
- Thread states
- Thread listing
- Mutex initialization
- Mutex locking
- Mutex unlocking
- Synchronization support

---

## Stage 3 — Memory Management

Stage 3 introduces physical and virtual memory management.

Implemented features include:

- Physical Memory Manager (PMM)
- Virtual Memory Manager (VMM)
- Page management
- Memory initialization
- Memory statistics
- `free` shell command

---

## Stage 4 — File System

Stage 4 adds a simple RAM-based file system.

Implemented features include:

- File system initialization
- File creation
- In-memory file storage
- File writing
- File lookup by name
- File deletion support
- File table management
- File count support
- `ls` shell command
- `cat <filename>` shell command
- Sample files initialized during kernel startup

Example files:

- `welcome.txt`
- `readme.txt`

---

## Project Structure

```text
SENG21213-OS/
├── boot/
│   └── boot.asm
├── include/
│   └── types.h
├── kernel/
│   ├── kernel_entry.asm
│   ├── kernel.c
│   ├── vga.c
│   ├── vga.h
│   ├── keyboard.c
│   ├── keyboard.h
│   ├── process.c
│   ├── process.h
│   ├── scheduler.c
│   ├── scheduler.h
│   ├── thread.c
│   ├── thread.h
│   ├── mutex.c
│   ├── mutex.h
│   ├── pmm.c
│   ├── pmm.h
│   ├── vmm.c
│   ├── vmm.h
│   ├── fs.c
│   └── fs.h
├── .gitignore
├── Makefile
├── linker.ld
└── README.md
