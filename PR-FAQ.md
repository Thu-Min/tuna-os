# Tuna OS PR-FAQ

## Headline

Tuna OS is a learn-by-building operating system course that takes you from a blank screen to a working x86_64 kernel, one phase at a time.

## Subheadline

For developers and CS students who want to truly understand what happens between their code and the hardware, Tuna OS provides a structured, phase-by-phase curriculum where you write a real kernel in C — with a working, testable system at every step.

## Problem

Learning operating system internals today is harder than it needs to be.

University courses and textbooks teach theory — page tables, scheduling algorithms, virtual memory — but leave students unable to write real kernel code. When learners turn to existing projects for hands-on experience, they hit a wall: Linux is millions of lines of production complexity, xv6 is a complete system that's hard to build up incrementally, and most OSDev wiki tutorials target 32-bit architectures with inconsistent quality and no guided progression.

There is no clean, modern path for someone who wants to build an x86_64 kernel from scratch and understand every line along the way.

## Solution

Tuna OS is structured as a series of phases, each adding exactly one subsystem to a working kernel. Phase 1 boots and prints to serial. Phase 2 adds a GDT and IDT with exception handling. Phase 3 brings hardware interrupts and a timer. Each phase produces a kernel you can build, run, and test — there is never a broken intermediate state.

Three design choices keep the project accessible:

- **C-first, minimal assembly.** Assembly appears only where hardware demands it (boot sequence, GDT reload, ISR stubs). Everything else is readable C11, so learners focus on OS concepts rather than syntax.
- **Serial-first output.** No VGA text mode or framebuffer complexity. COM1 serial output from day one means you can debug with plain text in your terminal immediately.
- **Phase-by-phase progression.** Each phase has a clear goal, builds on the previous one, and can be understood independently. You never have to hold the entire kernel in your head to make progress.

## Quote

"We believe everyone should understand what happens between their code and the hardware. Operating systems shouldn't be a black box — they're the most fundamental software we interact with every day. Tuna OS makes that knowledge accessible by letting you build it yourself, one piece at a time."

## Call to Action

Start with Phase 1 in the documentation, which walks you through `boot.S` line by line — what the Multiboot2 header does, how the CPU transitions to long mode, and why each instruction is there. By the end of Phase 1, you'll have a kernel that boots in QEMU and prints "hello from tuna os!" over serial. Then build up from there, phase by phase.

Prerequisites: a Mac or Linux machine, basic C programming knowledge, and curiosity about how computers actually work.

## Customer FAQ

**Q: Do I need operating system experience to start?**
A: No. You need working knowledge of C and a basic understanding of what a CPU does (registers, memory, instructions). The phases are designed to introduce OS concepts as you encounter them — you learn about the GDT when you build the GDT, not before.

**Q: Will this run on real hardware?**
A: Tuna OS targets QEMU, not bare-metal deployment. QEMU provides a consistent, debuggable environment where you can inspect registers, set breakpoints, and step through boot code. Running on real hardware introduces variables (firmware differences, driver requirements) that distract from learning OS fundamentals.

**Q: How far does the curriculum go?**
A: The planned progression goes from boot through interrupts, memory management, a basic scheduler, userspace processes, a simple filesystem, and a minimal shell. The goal is a kernel that can load and run a user program — enough to understand all the major subsystems of a real OS.

**Q: Why not just read xv6?**
A: xv6 is an excellent reference implementation, but it's a finished system. Reading someone else's complete kernel is a different experience from building one yourself, phase by phase. Tuna OS is also native x86_64 (xv6 targets RISC-V in its current version or 32-bit x86 in older versions) and uses a serial-first approach that simplifies the early learning curve.

## Internal FAQ

**Q: How long will the full curriculum take to complete?**
A: The project is structured in milestones of 3-5 phases each. The current milestone (boot through interrupts) covers phases 1-3. The full progression to a minimal userspace OS is estimated at 10-15 phases across 3-4 milestones. Development pace depends on complexity per phase — early phases are faster, memory management and scheduling phases require more research and iteration.

**Q: What's the ongoing maintenance burden?**
A: The toolchain (x86_64-elf-gcc, GRUB, QEMU) is stable and changes infrequently. The main maintenance surface is keeping QEMU compatibility as new versions release and ensuring the cross-compilation toolchain installs cleanly on current macOS and Linux. The codebase itself is small and has no external runtime dependencies.

**Q: Why x86_64 instead of RISC-V?**
A: x86_64 is what most developers' machines actually run. Understanding the architecture you use daily has immediate practical value — it demystifies performance characteristics, security boundaries, and debugging that developers encounter in their real work. RISC-V is cleaner architecturally, but x86_64 connects the learning to lived experience.

**Q: Is this a solo project or intended to become a community effort?**
A: Currently a solo educational project. The phase structure and documentation are designed so that others can follow along and learn, but development decisions prioritize clarity and learning value over community governance. Contributions that improve explanations or fix bugs are welcome; feature additions should align with the curriculum progression.
