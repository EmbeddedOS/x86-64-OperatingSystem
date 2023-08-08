#include "keyboard.h"
#include "printk.h"
#include "process.h"
/* Private define ------------------------------------------------------------*/

#define E0_SIGN                 (1 << 0)
#define SHIFT                   (1 << 1)
#define CAPS_LOCK               (1 << 2)

#define SENT_SCAN_CODE_PORT     0x60
/* Private variable ----------------------------------------------------------*/
static unsigned char s_shift_code[256] = {
    [0x2A] = SHIFT,
    [0x36] = SHIFT,
    [0xAA] = SHIFT,
    [0xB6] = SHIFT
};

static unsigned char s_lock_code[256] = {
    [0x3A] = CAPS_LOCK
};

static unsigned int s_flag = 0;
/**
 * @brief   These are the characters we have in the keyboard. The data sent from
 *          keyboard to the handler is not the data here. The data is called
 *          scan code, which means we need to convert the scan code to the ASCII
 *          code for the keyboard character and print it in the screen. So we
 *          define the keymap arrays to do that.
 *          NOTE: We don't handle pause key also which start with 0xE1, so we
 *          don't define it in key maps.
 */
static char s_key_map[256] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', '\b', 0, 'q', 'w', 'e', 'r', 't', 'y', 'u',
    'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's', 'd', 'f',
    'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z',
    'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

/**
 * @brief   The shift map is used when we hold shift key or caps lock key and
 *          press other keys.
 * 
 */
static char s_shift_key_map[256] = {
    0, 1, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U',
    'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C',
    'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};


/* Private function prototypes -----------------------------------------------*/
/**
 * @brief   The scan code is sent to the handler when we press a key and
 *          and release a key, so we have two scenarios key down and key up or
 *          make code and break code. The break code is the code of
 *          corresponding make code add with 0x80.
 *          We hae three sets of scan code, in our system, keyboard driver works
 *          with scan code set 1:
 *          (http://users.utcluj.ro/~baruch/sie/labor/PS2/Scan_Codes_Set_1.htm)
 * 
 *          We have one byte code for a key, multiple-byte code used for
 *          function keys which be sent one byte at a time. Currently, we don't
 *          support function keys in our system, so we will return invalid in
 *          this case.
 * 
 * @return char 
 */
static char ReadCharacter(void);

/* Public function -----------------------------------------------------------*/
void KeyboardHandler(void)
{
    char ch[2] = {0};
    ch[0] = ReadCharacter();
    if (ch[0] > 0) {
        printk("%s", ch);
    }
}

/* Private function ----------------------------------------------------------*/
static char ReadCharacter(void)
{
    unsigned char scan_code;
    char c;

    /* We read scan code from port 0x60. */
    scan_code = InByte(SENT_SCAN_CODE_PORT);

    /* Multiple-byte code used for function keys which be sent one byte at a
     * time. Most of the multiple-byte key code comes with E0 first. So here we
     * check scan code start with 0xE0, we will add E0_SIGN to the flag and
     * return 0 meaning key not valid. */
    if (scan_code == 0xE0) {
        s_flag |= E0_SIGN;
        return 0;
    }

    /* If the key not equal 0xE0 but 0xE0 flag is set, mean the last scan is
     * 0xE0, so we clear flag and return invalid. */
    if (s_flag & E0_SIGN) {
        s_flag &= ~E0_SIGN;
        return 0;
    }

    /* Handling up key. */
    if (scan_code & 0x80) {
        s_flag &= ~(s_shift_code[scan_code]);
        return 0;
    }

    s_flag |= s_shift_code[scan_code];
    s_flag ^= s_lock_code[scan_code];

    /* Handle shift + another keys. */
    if (s_flag & SHIFT) {
        c = s_shift_key_map[scan_code];
    } else {
        c = s_key_map[scan_code];
    }

    /* Handle caps lock + another keys. */
    if (s_flag & CAPS_LOCK) {
        if (c >= 'a' && c <= 'z') {
            c -= 32;
        } else if (c >= 'A' && c <= 'Z')
        {
            c+= 32;
        }
    }

    return c;
}
