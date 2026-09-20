#ifndef INPUT_OUTPUT_H
#define INPUT_OUTPUT_H

typedef enum {
    NO_INPUT,
    LEFT_DOWN,
    LEFT_UP,
    RIGHT_DOWN,
    RIGHT_UP
} KEY_TYPE;

typedef struct GPIOState {
    volatile int triggered;
    void (*callback)();
    void* volatile argument;
} GPIOState;

KEY_TYPE getInput();
void input_output_init();

#endif