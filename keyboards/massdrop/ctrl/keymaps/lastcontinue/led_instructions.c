#include "lc_led_matrix.h"

/******************************************************************************
 * Key-by-key instructions
 * If you want to edit patterns, see "led_programs.c"
 *****************************************************************************/

//Even If you don't want key-by-key codes, please keep this
//as it will enable you to use this "instructions" scheme as if they were
//just the default pattern
led_instruction_t patterns[] = {
    { .flags = LED_FLAG_MATCH_ID | LED_FLAG_USE_ROTATE_PATTERN, .id0 = 4294967295, .id1 = 4294967295, .id2 = 4294967295, .id3 = 8388607},
    { .end = 1 }
};

led_instruction_t primary[] = {
    //light gray keys
    { .flags = LED_FLAG_MATCH_ID | LED_FLAG_USE_RGB, .id0 = 536813086, .id1 = 1073250300, .id2 = 33791, .r = 255, .g = 255, .b = 255 },
    //dark gray keys
    { .flags = LED_FLAG_MATCH_ID | LED_FLAG_USE_RGB, .id0 = 3758154208, .id1 = 3221716995, .id2 = 8354816, .r = 0, .g = 0, .b = 255 },
    //backlight keys
    { .flags = LED_FLAG_MATCH_ID | LED_FLAG_USE_RGB, .id2 = 4286578688, .id3 = 8388607, .r = 0, .g = 0, .b = 255 },
    // escape
    { .flags = LED_FLAG_MATCH_ID | LED_FLAG_MATCH_LAYER | LED_FLAG_USE_RGB, .id0 = 1, .layer = 0, .r = 0, .g = 0, .b = 255 },
    { .flags = LED_FLAG_MATCH_ID | LED_FLAG_MATCH_LAYER | LED_FLAG_USE_RGB, .id0 = 1, .layer = 1, .r = 197, .g = 9, .b = 213 },
    { .end = 1 }
};

//Example led_instructions from MD's old fork
led_instruction_t demo[] = {
    { .flags = LED_FLAG_MATCH_ID | LED_FLAG_USE_RGB, .id0 = 10, .id1 = 9, .r = 255, .g = 0, .b = 0 },
    { .flags = LED_FLAG_MATCH_ID | LED_FLAG_USE_PATTERN, .id0 = 4, .id1 = 0, .pattern_id = 8 },
    { .flags = LED_FLAG_MATCH_ID | LED_FLAG_USE_RGB, .id0 = 8, .id1 = 0, .r = 0, .g = 255, .b = 0 },
    { .flags = LED_FLAG_MATCH_ID | LED_FLAG_USE_PATTERN, .id0 = 16, .id1 = 0, .pattern_id = 9 },
    { .flags = LED_FLAG_MATCH_ID | LED_FLAG_USE_RGB, .id0 = 32, .id1 = 0, .r = 0, .g = 0, .b = 255 },
    { .flags = LED_FLAG_MATCH_ID | LED_FLAG_USE_RGB, .id0 = 40, .id1 = 0, .r = 0, .g = 0, .b = 255 },
    { .flags = LED_FLAG_MATCH_ID | LED_FLAG_USE_ROTATE_PATTERN, .id0 = 64, .id1 = 0},
    { .flags = LED_FLAG_MATCH_ID | LED_FLAG_MATCH_LAYER | LED_FLAG_USE_ROTATE_PATTERN, .id0 = 262144, .id1 = 0, .layer = 0 },
    { .flags = LED_FLAG_MATCH_ID | LED_FLAG_MATCH_LAYER | LED_FLAG_USE_ROTATE_PATTERN, .id0 = 16777216, .id1 = 0, .layer = 1 },
    { .end = 1 }
};


/******************************************************************************
 * Final light layout mapping
 *****************************************************************************/

led_instruction_t *led_instructions[] = {
  primary,
  patterns,
  demo
};

const uint8_t led_instructions_count = sizeof(led_instructions) / sizeof(led_instructions[0]);
uint8_t current_map_idx = 0;

/******************************************************************************
 * Useful numbers
 *****************************************************************************/
/*
All lights -
.id0 = 4294967295, .id1 = 4294967295, .id2 = 4294967295, .id3 = 8388607,
All keys -
.id0 = 4294967295, .id1 = 4294967295, .id2 = 4194303
Back light -
.id2 = 4286578688, .id3 = 8388607,
WASD -
.id1 = 3670024,
Arrows -
.id2 = 7342080,
*/


/**
 * Change the current active index of the array
 * and loop back to the beginning if the end has been reached
 */
void cycle_colors() {
  if(current_map_idx + 1 == led_instructions_count){
    current_map_idx = 0;
  } else {
    current_map_idx ++;
  }
}
