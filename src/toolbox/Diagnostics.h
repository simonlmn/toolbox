#ifndef TOOLBOX_DIAGNOSTICS_H_
#define TOOLBOX_DIAGNOSTICS_H_

inline unsigned long freeMemory() {
#if defined(ARDUINO_ARCH_AVR)
    extern int __heap_start, *__brkval;
    return (unsigned long)SP - (__brkval == 0 ? (unsigned long) &__heap_start : (unsigned long) __brkval);
#else
    return 0ul;
#endif
}

inline unsigned long heapSize() {
#if defined(ARDUINO_ARCH_AVR)
    extern int __heap_start, *__brkval;
    return (__brkval == 0 ? 0ul : ((unsigned long)__brkval) - (unsigned long)&__heap_start);
#else
    return 0ul;
#endif
}

inline unsigned long stackSize() {
#if defined(ARDUINO_ARCH_AVR)
    extern int __heap_start, *__brkval;
    return (unsigned long)(RAMEND) - (unsigned long)(SP);
#else
    return 0ul;
#endif
}

#endif // TOOLBOX_DIAGNOSTICS_H_