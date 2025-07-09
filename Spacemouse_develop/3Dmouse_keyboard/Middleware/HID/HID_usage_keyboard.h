// hid_usage_keyboard_custom.h

#ifndef __HID_USAGE_KEYBOARD_CUSTOM_H
#define __HID_USAGE_KEYBOARD_CUSTOM_H

/**
 * @brief USB HID Keyboard Scan Codes
 * @note  Customized for the CH450 key map project.
 */

#define KEY_NONE                0x00 // No key pressed

// Alphanumeric Keys
#define KEY_A                   0x04
#define KEY_B                   0x05
#define KEY_C                   0x06
#define KEY_D                   0x07
#define KEY_E                   0x08
#define KEY_F                   0x09
#define KEY_G                   0x0A
#define KEY_H                   0x0B
#define KEY_I                   0x0C
#define KEY_J                   0x0D
#define KEY_K                   0x0E
#define KEY_L                   0x0F
#define KEY_M                   0x10
#define KEY_N                   0x11
#define KEY_O                   0x12
#define KEY_P                   0x13
#define KEY_Q                   0x14
#define KEY_R                   0x15
#define KEY_S                   0x16
#define KEY_T                   0x17
#define KEY_U                   0x18
#define KEY_V                   0x19
#define KEY_W                   0x1A
#define KEY_X                   0x1B
#define KEY_Y                   0x1C
#define KEY_Z                   0x1D

// Numeric Keys from the key map
#define KEY_1                   0x1E
#define KEY_2                   0x1F
#define KEY_3                   0x20
#define KEY_4                   0x21
#define KEY_5                   0x22
#define KEY_6                   0x23
#define KEY_7                   0x24
#define KEY_8                   0x25
#define KEY_9                   0x26
#define KEY_0                   0x27

// Control Keys from the key map
#define KEY_ENTER               0x28
#define KEY_ESCAPE              0x29
#define KEY_BACKSPACE           0x2A
#define KEY_DELETE              0x4C

// Punctuation from the key map
#define KEY_DOT                 0x37 // . and >
#define KEY_KP_DOT              0x63 // Keypad . and Del

// Modifier Keys (Used in the first byte of the report)
// Kept because Ctrl, Alt, Shift are in the key map.
#define MOD_LEFT_CTRL           (1 << 0)
#define MOD_LEFT_SHIFT          (1 << 1)
#define MOD_LEFT_ALT            (1 << 2)
#define MOD_LEFT_GUI            (1 << 3) // Windows/Command Key
#define MOD_RIGHT_CTRL          (1 << 4)
#define MOD_RIGHT_SHIFT         (1 << 5)
#define MOD_RIGHT_ALT           (1 << 6)
#define MOD_RIGHT_GUI           (1 << 7)

#endif // __HID_USAGE_KEYBOARD_CUSTOM_H