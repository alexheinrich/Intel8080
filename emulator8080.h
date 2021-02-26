typedef struct condition_flags {
    uint8_t s; // sign
    uint8_t z; // zero
    uint8_t ac;// auxiliary carry over
    uint8_t p; // is_even_parity
    uint8_t c; // carry
} condition_flags;

typedef struct state8080 {
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;
    size_t pc;
    uint16_t sp;
    uint8_t *memory;
    struct condition_flags cf;
    uint8_t interrupts_enabled;
} state8080;

