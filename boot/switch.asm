[BITS 32]

[EXTERN scheduler_switch]
[GLOBAL irq0_stub]

irq0_stub:
    ; CPU has already pushed EFLAGS, CS and EIP.
    ; Save general-purpose registers and segment registers.
    pusha

    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax

    ; Pass current saved ESP to the scheduler.
    mov eax, esp
    push eax
    call scheduler_switch
    add esp, 4

    ; scheduler_switch returns the stack to restore in EAX.
    mov esp, eax

    ; Send End Of Interrupt to master PIC.
    mov al, 0x20
    out 0x20, al

    ; Restore the selected process context.
    pop gs
    pop fs
    pop es
    pop ds

    popa
    iretd
