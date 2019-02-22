/*
Override of tmk_core/protocol/arm_atsam/led_matrix.h to allow "key-by-key"
color config which has been implimented in Massdrop's fork for some time, but
has yet to be merged into QMK master. That fork's version of QMK is out of date
and was causing compile errors when using newer versions of the compiler
tool-chain on MacOs
*/

#ifndef _LC_LED_MATRIX_H_
#define _LC_LED_MATRIX_H_

//LED Extra Instructions
#define LED_FLAG_NULL                0x00
#define LED_FLAG_MATCH_ID            0x01
#define LED_FLAG_MATCH_LAYER         0x02
#define LED_FLAG_USE_RGB             0x10
#define LED_FLAG_USE_PATTERN         0x20
#define LED_FLAG_USE_ROTATE_PATTERN  0x40

typedef struct led_instruction_s {
    uint16_t flags; // Bitfield for LED instructions
    uint32_t id0; // Bitwise id, IDs 0-31
    uint32_t id1; // Bitwise id, IDs 32-63
    uint32_t id2; // Bitwise id, IDs 64-95
    uint32_t id3; // Bitwise id, IDs 96-127
    uint8_t layer;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t pattern_id;
    uint8_t end;
} led_instruction_t;

extern const uint8_t led_instructions_count;
extern led_instruction_t *led_instructions[];

extern uint32_t layer_state;
extern uint8_t current_map_idx;

void cycle_colors(void);

#endif //_LC_LED_MATRIX_H_
