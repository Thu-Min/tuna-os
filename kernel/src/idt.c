#include "idt.h"

#include <stddef.h>
#include <stdint.h>

#include "pic.h"
#include "serial.h"
#include "syscall.h"

#define IDT_ENTRIES 256
#define EXCEPTION_COUNT 32
#define ISR_STUB_COUNT 48
#define IRQ_BASE 32
#define KERNEL_CODE_SELECTOR 0x08
#define IDT_GATE_INTERRUPT  0x8E
#define IDT_GATE_TRAP_USER  0xEF  /* present, DPL=3, trap gate */
#define SYSCALL_VECTOR       128

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static struct idt_entry idt[IDT_ENTRIES];
static irq_handler_t irq_handlers[16];

extern void (*isr_stub_table[ISR_STUB_COUNT])(void);
extern void isr_stub_128(void);

static const char *const exception_names[EXCEPTION_COUNT] = {
    "Divide Error",
    "Debug",
    "NMI",
    "Breakpoint",
    "Overflow",
    "BOUND Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved",
};

static void idt_set_gate(uint8_t vector, void (*handler)(void), uint8_t type_attr) {
    uint64_t addr = (uint64_t)handler;
    idt[vector].offset_low = (uint16_t)(addr & 0xFFFF);
    idt[vector].selector = KERNEL_CODE_SELECTOR;
    idt[vector].ist = 0;
    idt[vector].type_attr = type_attr;
    idt[vector].offset_mid = (uint16_t)((addr >> 16) & 0xFFFF);
    idt[vector].offset_high = (uint32_t)((addr >> 32) & 0xFFFFFFFF);
    idt[vector].zero = 0;
}

static void lidt(const struct idt_ptr *idtr) {
    __asm__ volatile ("lidt %0" : : "m"(*idtr));
}

void idt_init(void) {
    struct idt_ptr idtr;

    for (size_t i = 0; i < IDT_ENTRIES; i++) {
        idt[i] = (struct idt_entry){0};
    }

    for (size_t i = 0; i < ISR_STUB_COUNT; i++) {
        idt_set_gate((uint8_t)i, isr_stub_table[i], IDT_GATE_INTERRUPT);
    }

    /* Syscall vector 0x80 — DPL=3 trap gate so ring 3 can invoke it */
    idt_set_gate(SYSCALL_VECTOR, isr_stub_128, IDT_GATE_TRAP_USER);

    idtr.limit = (uint16_t)(sizeof(idt) - 1);
    idtr.base = (uint64_t)idt;
    lidt(&idtr);
}

void irq_register_handler(uint8_t irq, irq_handler_t handler) {
    if (irq < 16) {
        irq_handlers[irq] = handler;
    }
}

void isr_dispatch(struct interrupt_frame *frame) {
    // Syscall dispatch (vector 128 = 0x80).
    if (frame->vector == SYSCALL_VECTOR) {
        syscall_dispatch(frame);
        return;
    }

    // IRQ dispatch (vectors 32-47).
    if (frame->vector >= IRQ_BASE && frame->vector < IRQ_BASE + 16) {
        uint8_t irq = (uint8_t)(frame->vector - IRQ_BASE);
        if (irq_handlers[irq]) {
            irq_handlers[irq](frame);
        } else {
            serial_write("[irq] irq=");
            serial_write_dec_u64(irq);
            serial_write("\n");
        }
        pic_eoi(irq);
        return;
    }

    // CPU exception dispatch (vectors 0-31).
    serial_write("\n[exception] vector=");
    serial_write_dec_u64(frame->vector);

    if (frame->vector < EXCEPTION_COUNT) {
        serial_write(" (");
        serial_write(exception_names[frame->vector]);
        serial_write(")");
    }

    serial_write(" error=");
    serial_write_hex_u64(frame->error_code);
    serial_write(" rip=");
    serial_write_hex_u64(frame->rip);
    serial_write("\n");

    if (frame->vector == 3) {
        serial_write("[exception] breakpoint handled, resuming.\n");
        return;
    }

    if (frame->vector == 13 && (frame->cs & 3) == 3) {
        serial_write("[exception] GPF from ring 3 at rip=");
        serial_write_hex_u64(frame->rip);
        serial_write(" — privilege enforcement confirmed\n");
        serial_write("[exception] halting.\n");
        for (;;)
            __asm__ volatile ("cli; hlt");
    }

    serial_write("[exception] fatal exception, halting.\n");
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}
