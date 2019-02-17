/*
Override of tmk_core/protocol/arm_atsam/led_matrix.c to allow "key-by-key"
color config which has been implimented in Massdrop's fork for some time, but
has yet to be merged into QMK master. That fork's version of QMK is out of date
and was causing compile errors when using newer versions of the compiler
tool-chain on MacOs
*/

#include "arm_atsam_protocol.h"
#include "tmk_core/common/led.h"
#include <math.h>
#include <string.h>

extern led_disp_t disp;
extern issi3733_led_t *lede;
extern issi3733_led_t led_map[];

uint8_t breathe_dir;
float breathe_mult;
uint8_t breathe_step;
uint8_t highest_active_layer = 0;

uint8_t led_animation_breathing;
uint8_t led_animation_breathe_cur;
uint8_t led_animation_direction;
uint8_t led_animation_id;
float led_animation_speed;

issi3733_led_t *led_cur;
uint8_t led_enabled;
uint8_t led_lighting_mode;
uint64_t led_next_run;
uint8_t led_per_run;

float pxmod;
uint32_t temp_layer_state = 0;

void led_run_pattern(led_setup_t *f, float* ro, float* go, float* bo, float pxs) {
    float px;

    while (f->end != 1)
    {
        px = pxs; //Reset px for new frame

        //Add in any moving effects
        if ((!led_animation_direction && f->ef & EF_SCR_R) || (led_animation_direction && (f->ef & EF_SCR_L)))
        {
            px -= pxmod;

            if (px > 100) px -= 100;
            else if (px < 0) px += 100;
        }
        else if ((!led_animation_direction && f->ef & EF_SCR_L) || (led_animation_direction && (f->ef & EF_SCR_R)))
        {
            px += pxmod;

            if (px > 100) px -= 100;
            else if (px < 0) px += 100;
        }

        //Check if LED's px is in current frame
        if (px < f->hs) { f++; continue; }
        if (px > f->he) { f++; continue; }
        //note: < 0 or > 100 continue

        //Calculate the px within the start-stop percentage for color blending
        px = (px - f->hs) / (f->he - f->hs);

        //Add in any color effects
        if (f->ef & EF_OVER)
        {
            *ro = (px * (f->re - f->rs)) + f->rs;// + 0.5;
            *go = (px * (f->ge - f->gs)) + f->gs;// + 0.5;
            *bo = (px * (f->be - f->bs)) + f->bs;// + 0.5;
        }
        else if (f->ef & EF_SUBTRACT)
        {
            *ro -= (px * (f->re - f->rs)) + f->rs;// + 0.5;
            *go -= (px * (f->ge - f->gs)) + f->gs;// + 0.5;
            *bo -= (px * (f->be - f->bs)) + f->bs;// + 0.5;
        }
        else
        {
            *ro += (px * (f->re - f->rs)) + f->rs;// + 0.5;
            *go += (px * (f->ge - f->gs)) + f->gs;// + 0.5;
            *bo += (px * (f->be - f->bs)) + f->bs;// + 0.5;
        }

        f++;
    }
}

// Ideally we could leave this in, but I can't remember enough C to make it happen
// without errors
// __attribute__((weak))
// led_instruction_t led_instructions[] = { { .end = 1 } };

void led_matrix_run()
{
    float ro;
    float go;
    float bo;
    uint8_t led_this_run = 0;
    led_setup_t *f = (led_setup_t*)led_setups[led_animation_id];

    if (led_cur == 0) //Denotes start of new processing cycle in the case of chunked processing
    {
        led_cur = led_map;

        disp.frame += 1;

        breathe_mult = 1;

        if (led_animation_breathing)
        {
            //+60us 119 LED
            led_animation_breathe_cur += breathe_step * breathe_dir;

            if (led_animation_breathe_cur >= BREATHE_MAX_STEP)
                breathe_dir = -1;
            else if (led_animation_breathe_cur <= BREATHE_MIN_STEP)
                breathe_dir = 1;

            //Brightness curve created for 256 steps, 0 - ~98%
            breathe_mult = 0.000015 * led_animation_breathe_cur * led_animation_breathe_cur;
            if (breathe_mult > 1) breathe_mult = 1;
            else if (breathe_mult < 0) breathe_mult = 0;
        }

        //Only needs to be calculated once per frame
        pxmod = (float)(disp.frame % (uint32_t)(1000.0f / led_animation_speed)) / 10.0f * led_animation_speed;
        pxmod *= 100.0f;
        pxmod = (uint32_t)pxmod % 10000;
        pxmod /= 100.0f;

        highest_active_layer = 0;
        temp_layer_state = layer_state;

        while (temp_layer_state >> 1 != 0) {
            highest_active_layer++;
            temp_layer_state = temp_layer_state >> 1;
        }
    }
    uint8_t led_per_run = 15;

    while (led_cur < lede && led_this_run < led_per_run)
    {
        ro = 0;
        go = 0;
        bo = 0;

        if (led_lighting_mode == LED_MODE_KEYS_ONLY && led_cur->scan == 255)
        {
            //Do not act on this LED
        }
        else if (led_lighting_mode == LED_MODE_NON_KEYS_ONLY && led_cur->scan != 255)
        {
            //Do not act on this LED
        }
        else if (led_lighting_mode == LED_MODE_INDICATORS_ONLY)
        {
            //Do not act on this LED (Only show indicators)
        }
        else
        {
            led_instruction_t *led_cur_instruction;
            led_cur_instruction = led_instructions;

            //Act on LED
            if (led_cur_instruction->end) {
                // If no instructions, use normal pattern
                led_run_pattern(f, &ro, &go, &bo, led_cur->px);
            } else {
                uint8_t skip;
                uint8_t modid = (led_cur->id - 1) / 32;                         //PS: Calculate which id# contains the led bit
                uint32_t modidbit = 1 << ((led_cur->id - 1) % 32);              //PS: Calculate the bit within the id#
                uint32_t *bitfield;                                             //PS: Will point to the id# within the current instruction

                while (!led_cur_instruction->end) {
                    skip = 0;

                    //PS: Check layer active first
                    if (led_cur_instruction->flags & LED_FLAG_MATCH_LAYER) {
                        if (led_cur_instruction->layer != highest_active_layer) {
                            skip = 1;
                        }
                    }

                    if (!skip)
                    {
                        if (led_cur_instruction->flags & LED_FLAG_MATCH_ID) {
                            bitfield = &led_cur_instruction->id0 + modid;       //PS: Add modid as offset to id0 address. *bitfield is now idX of the led id
                            if (~(*bitfield) & modidbit) {                      //PS: Check if led bit is not set in idX
                                skip = 1;
                            }
                        }
                    }

                    if (!skip) {
                        if (led_cur_instruction->flags & LED_FLAG_USE_RGB) {
                            ro = led_cur_instruction->r;
                            go = led_cur_instruction->g;
                            bo = led_cur_instruction->b;
                        } else if (led_cur_instruction->flags & LED_FLAG_USE_PATTERN) {
                            led_run_pattern(led_setups[led_cur_instruction->pattern_id], &ro, &go, &bo, led_cur->px);
                        } else if (led_cur_instruction->flags & LED_FLAG_USE_ROTATE_PATTERN) {
                            led_run_pattern(f, &ro, &go, &bo, led_cur->px);
                        }
                    }

                    led_cur_instruction++;
                }
            }
        }

        //Clamp values 0-255
        if (ro > 255) ro = 255; else if (ro < 0) ro = 0;
        if (go > 255) go = 255; else if (go < 0) go = 0;
        if (bo > 255) bo = 255; else if (bo < 0) bo = 0;

        if (led_animation_breathing)
        {
            ro *= breathe_mult;
            go *= breathe_mult;
            bo *= breathe_mult;
        }

        *led_cur->rgb.r = (uint8_t)ro;
        *led_cur->rgb.g = (uint8_t)go;
        *led_cur->rgb.b = (uint8_t)bo;

#ifdef USB_LED_INDICATOR_ENABLE
        if (keyboard_leds())
        {
            uint8_t kbled = keyboard_leds();
            if (
                #if USB_LED_NUM_LOCK_SCANCODE != 255
                (led_cur->scan == USB_LED_NUM_LOCK_SCANCODE && kbled & (1<<USB_LED_NUM_LOCK)) ||
                #endif //NUM LOCK
                #if USB_LED_CAPS_LOCK_SCANCODE != 255
                (led_cur->scan == USB_LED_CAPS_LOCK_SCANCODE && kbled & (1<<USB_LED_CAPS_LOCK)) ||
                #endif //CAPS LOCK
                #if USB_LED_SCROLL_LOCK_SCANCODE != 255
                (led_cur->scan == USB_LED_SCROLL_LOCK_SCANCODE && kbled & (1<<USB_LED_SCROLL_LOCK)) ||
                #endif //SCROLL LOCK
                #if USB_LED_COMPOSE_SCANCODE != 255
                (led_cur->scan == USB_LED_COMPOSE_SCANCODE && kbled & (1<<USB_LED_COMPOSE)) ||
                #endif //COMPOSE
                #if USB_LED_KANA_SCANCODE != 255
                (led_cur->scan == USB_LED_KANA_SCANCODE && kbled & (1<<USB_LED_KANA)) ||
                #endif //KANA
                (0))
            {
                if (*led_cur->rgb.r > 127) *led_cur->rgb.r = 0;
                else *led_cur->rgb.r = 255;
                if (*led_cur->rgb.g > 127) *led_cur->rgb.g = 0;
                else *led_cur->rgb.g = 255;
                if (*led_cur->rgb.b > 127) *led_cur->rgb.b = 0;
                else *led_cur->rgb.b = 255;
            }
        }
#endif //USB_LED_INDICATOR_ENABLE

        led_cur++;
        led_this_run++;
    }
}
