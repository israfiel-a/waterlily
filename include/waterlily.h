#ifndef WATERLILY_H
#define WATERLILY_H

#include <stdint.h>
#define __need_size_t
#include <stddef.h>

#define WATERLILY_CONCURRENT_FRAMES 2
#define WATERLILY_KEY_TIMER_MS 50

typedef struct waterlily_context waterlily_context_t;

typedef enum waterlily_key_state
{
    WATERLILY_KEY_STATE_DOWN,
    WATERLILY_KEY_STATE_UP,
    WATERLILY_KEY_STATE_REPEAT
} waterlily_key_state_t;

typedef enum waterlily_keycode : uint32_t
{
    WATERLILY_KEY_NONE = 0x00000,
    WATERLILY_KEY_BACKSPACE = 0xff08, /* U+0008 BACKSPACE */
    WATERLILY_KEY_TAB = 0xff09,       /* U+0009 CHARACTER TABULATION */
    WATERLILY_KEY_LINEFEED = 0xff0a,  /* U+000A LINE FEED */
    WATERLILY_KEY_CLEAR = 0xff0b,     /* U+000B LINE TABULATION */
    WATERLILY_KEY_RETURN = 0xff0d,    /* U+000D CARRIAGE RETURN */
    WATERLILY_KEY_PAUSE = 0xff13,     /* Pause, hold */
    WATERLILY_KEY_SCROLL_LOCK = 0xff14,
    WATERLILY_KEY_SYS_REQ = 0xff15,
    WATERLILY_KEY_ESCAPE = 0xff1b,    /* U+001B ESCAPE */
    WATERLILY_KEY_DELETE = 0xffff,    /* U+007F DELETE */
    WATERLILY_KEY_MULTI_KEY = 0xff20, /* Multi-key character compose */
    WATERLILY_KEY_CODEINPUT = 0xff37,
    WATERLILY_KEY_SINGLE_CANDIDATE = 0xff3c,
    WATERLILY_KEY_MULTIPLE_CANDIDATE = 0xff3d,
    WATERLILY_KEY_PREVIOUS_CANDIDATE = 0xff3e,
    WATERLILY_KEY_KANJI = 0xff21,       /* Kanji, Kanji convert */
    WATERLILY_KEY_MUHENKAN = 0xff22,    /* Cancel Conversion */
    WATERLILY_KEY_HENKAN_MODE = 0xff23, /* Start/Stop Conversion */
    WATERLILY_KEY_HENKAN = 0xff23,   /* non-deprecated alias for Henkan_Mode */
    WATERLILY_KEY_ROMANJI = 0xff24,  /* to Romaji */
    WATERLILY_KEY_HIRAGANA = 0xff25, /* to Hiragana */
    WATERLILY_KEY_KATAKANA = 0xff26, /* to Katakana */
    WATERLILY_KEY_HIRAGANA_KATAKANA = 0XFF27, /* HIRAGANA/KATAKANA TOGGLE */
    WATERLILY_KEY_ZENKAKU = 0XFF28,           /* TO ZENKAKU */
    WATERLILY_KEY_HANKAKU = 0XFF29,           /* TO HANKAKU */
    WATERLILY_KEY_ZENKAKU_HANKAKU = 0XFF2A,   /* ZENKAKU/HANKAKU TOGGLE */
    WATERLILY_KEY_TOUROKU = 0XFF2B,           /* ADD TO DICTIONARY */
    WATERLILY_KEY_MASSYO = 0XFF2C,            /* DELETE FROM DICTIONARY */
    WATERLILY_KEY_KANA_LOCK = 0XFF2D,         /* KANA LOCK */
    WATERLILY_KEY_KANA_SHIFT = 0XFF2E,        /* KANA SHIFT */
    WATERLILY_KEY_EISU_SHIFT = 0XFF2F,        /* ALPHANUMERIC SHIFT */
    WATERLILY_KEY_EISU_TOGGLE = 0XFF30,       /* ALPHANUMERIC TOGGLE */
    WATERLILY_KEY_KANJI_BANGOU = 0XFF37,      /* CODEINPUT */
    WATERLILY_KEY_ZEN_KOHO = 0XFF3D,          /* MULTIPLE/ALL CANDIDATE(S) */
    WATERLILY_KEY_MAE_KOHO = 0XFF3E,          /* PREVIOUS CANDIDATE */
    WATERLILY_KEY_HOME = 0XFF50,
    WATERLILY_KEY_LEFT = 0XFF51,  /* MOVE LEFT, LEFT ARROW */
    WATERLILY_KEY_UP = 0XFF52,    /* MOVE UP, UP ARROW */
    WATERLILY_KEY_RIGHT = 0XFF53, /* MOVE RIGHT, RIGHT ARROW */
    WATERLILY_KEY_DOWN = 0XFF54,  /* MOVE DOWN, DOWN ARROW */
    WATERLILY_KEY_PRIOR = 0XFF55, /* PRIOR, PREVIOUS */
    WATERLILY_KEY_NEXT = 0XFF56,  /* NEXT */
    WATERLILY_KEY_END = 0XFF57,   /* EOL */
    WATERLILY_KEY_BEGIN = 0XFF58, /* BOL */
    WATERLILY_KEY_PRINT = 0XFF61,
    WATERLILY_KEY_EXECUTE = 0XFF62, /* EXECUTE, RUN, DO */
    WATERLILY_KEY_INSERT = 0XFF63,  /* INSERT, INSERT HERE */
    WATERLILY_KEY_UNDO = 0XFF65,
    WATERLILY_KEY_REDO = 0XFF66, /* REDO, AGAIN */
    WATERLILY_KEY_MENU = 0XFF67,
    WATERLILY_KEY_FIND = 0XFF68,   /* FIND, SEARCH */
    WATERLILY_KEY_CANCEL = 0XFF69, /* CANCEL, STOP, ABORT, EXIT */
    WATERLILY_KEY_HELP = 0XFF6A,   /* HELP */
    WATERLILY_KEY_BREAK = 0XFF6B,
    WATERLILY_KEY_MODE_SWITCH = 0XFF7E,   /* CHARACTER SET SWITCH */
    WATERLILY_KEY_SCRIPT_SWITCH = 0XFF7E, /* ALIAS FOR MODE_SWITCH */
    WATERLILY_KEY_NUM_LOCK = 0XFF7F,
    WATERLILY_KEY_KP_SPACE = 0XFF80, /*<U+0020 SPACE>*/
    WATERLILY_KEY_KP_TAB = 0XFF89,   /*<U+0009 CHARACTER TABULATION>*/
    WATERLILY_KEY_KP_ENTER = 0XFF8D, /*<U+000D CARRIAGE RETURN>*/
    WATERLILY_KEY_KP_F1 = 0XFF91,    /* PF1, KP_A, ... */
    WATERLILY_KEY_KP_F2 = 0XFF92,
    WATERLILY_KEY_KP_F3 = 0XFF93,
    WATERLILY_KEY_KP_F4 = 0XFF94,
    WATERLILY_KEY_KP_HOME = 0XFF95,
    WATERLILY_KEY_KP_LEFT = 0XFF96,
    WATERLILY_KEY_KP_UP = 0XFF97,
    WATERLILY_KEY_KP_RIGHT = 0XFF98,
    WATERLILY_KEY_KP_DOWN = 0XFF99,
    WATERLILY_KEY_KP_PRIOR = 0XFF9A,
    WATERLILY_KEY_KP_NEXT = 0XFF9B,
    WATERLILY_KEY_KP_END = 0XFF9C,
    WATERLILY_KEY_KP_BEGIN = 0XFF9D,
    WATERLILY_KEY_KP_INSERT = 0XFF9E,
    WATERLILY_KEY_KP_DELETE = 0XFF9F,
    WATERLILY_KEY_KP_EQUAL = 0XFFBD,     /*<U+003D EQUALS SIGN>*/
    WATERLILY_KEY_KP_Multiply = 0xffaa,  /*<U+002A ASTERISK>*/
    WATERLILY_KEY_KP_Add = 0xffab,       /*<U+002B PLUS SIGN>*/
    WATERLILY_KEY_KP_Separator = 0xffac, /*<U+002C COMMA>*/
    WATERLILY_KEY_KP_Subtract = 0xffad,  /*<U+002D HYPHEN-MINUS>*/
    WATERLILY_KEY_KP_Decimal = 0xffae,   /*<U+002E FULL STOP>*/
    WATERLILY_KEY_KP_Divide = 0xffaf,    /*<U+002F SOLIDUS>*/
    WATERLILY_KEY_KP_0 = 0xffb0,         /*<U+0030 DIGIT ZERO>*/
    WATERLILY_KEY_KP_1 = 0xffb1,         /*<U+0031 DIGIT ONE>*/
    WATERLILY_KEY_KP_2 = 0xffb2,         /*<U+0032 DIGIT TWO>*/
    WATERLILY_KEY_KP_3 = 0xffb3,         /*<U+0033 DIGIT THREE>*/
    WATERLILY_KEY_KP_4 = 0xffb4,         /*<U+0034 DIGIT FOUR>*/
    WATERLILY_KEY_KP_5 = 0xffb5,         /*<U+0035 DIGIT FIVE>*/
    WATERLILY_KEY_KP_6 = 0xffb6,         /*<U+0036 DIGIT SIX>*/
    WATERLILY_KEY_KP_7 = 0xffb7,         /*<U+0037 DIGIT SEVEN>*/
    WATERLILY_KEY_KP_8 = 0xffb8,         /*<U+0038 DIGIT EIGHT>*/
    WATERLILY_KEY_KP_9 = 0xffb9,         /*<U+0039 DIGIT NINE>*/
    WATERLILY_KEY_F1 = 0xffbe,
    WATERLILY_KEY_F2 = 0xffbf,
    WATERLILY_KEY_F3 = 0xffc0,
    WATERLILY_KEY_F4 = 0xffc1,
    WATERLILY_KEY_F5 = 0xffc2,
    WATERLILY_KEY_F6 = 0xffc3,
    WATERLILY_KEY_F7 = 0xffc4,
    WATERLILY_KEY_F8 = 0xffc5,
    WATERLILY_KEY_F9 = 0xffc6,
    WATERLILY_KEY_F10 = 0xffc7,
    WATERLILY_KEY_F11 = 0xffc8,
    WATERLILY_KEY_F12 = 0xffc9,
    WATERLILY_KEY_F13 = 0xffca,
    WATERLILY_KEY_F14 = 0xffcb,
    WATERLILY_KEY_F15 = 0xffcc,
    WATERLILY_KEY_F16 = 0xffcd,
    WATERLILY_KEY_F17 = 0xffce,
    WATERLILY_KEY_F18 = 0xffcf,
    WATERLILY_KEY_F19 = 0xffd0,
    WATERLILY_KEY_F20 = 0xffd1,
    WATERLILY_KEY_F21 = 0xffd2,
    WATERLILY_KEY_F22 = 0xffd3,
    WATERLILY_KEY_F23 = 0xffd4,
    WATERLILY_KEY_F24 = 0xffd5,
    WATERLILY_KEY_F25 = 0xffd6,
    WATERLILY_KEY_F26 = 0xffd7,
    WATERLILY_KEY_F27 = 0xffd8,
    WATERLILY_KEY_F28 = 0xffd9,
    WATERLILY_KEY_F29 = 0xffda,
    WATERLILY_KEY_F30 = 0xffdb,
    WATERLILY_KEY_F31 = 0xffdc,
    WATERLILY_KEY_F32 = 0xffdd,
    WATERLILY_KEY_F33 = 0xffde,
    WATERLILY_KEY_F34 = 0xffdf,
    WATERLILY_KEY_F35 = 0xffe0,
} waterlily_keycode_t;

typedef struct waterlily_key
{
    uint64_t timestamp;
    waterlily_keycode_t symbol;
    waterlily_key_state_t state;
} waterlily_key_t;

typedef struct waterlily_key_combination
{
    waterlily_key_t first;
    waterlily_key_t second;
    void (*func)(waterlily_key_t *first, waterlily_key_t *second,
                 waterlily_context_t *context);
} waterlily_key_combination_t;

#define waterlily_entry(...)                                                   \
    __asm(".global _start\n"                                                   \
          "_start:\n"                                                          \
          "xorl %ebp, %ebp\n"                                                  \
          "movq 0(%rsp), %rdi\n"                                               \
          "lea 8(%rsp), %rsi\n"                                                \
          "call waterlily_create\n"                                            \
          "cmp $0, %rdi\n"                                                     \
          "jne .run\n"                                                         \
          "movq %rax, %rdi\n"                                                  \
          "movl $60, %eax\n"                                                   \
          "syscall\n"                                                          \
          ".run:\n"                                                            \
          "movq %rax, %rdi\n"                                                  \
          "call waterlily_application\n"                                       \
          "pushq %rdi\n"                                                       \
          "call waterlily_destroy\n"                                           \
          "popq %rdi\n"                                                        \
          "movl $60, %eax\n"                                                   \
          "syscall");                                                          \
    int waterlily_application(__VA_ARGS__)

bool waterlily_run(waterlily_context_t *context);

#endif // WATERLILY_H

