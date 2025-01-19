#pragma once
/*! @defgroup hat_state Joystick hat states
 *  @brief Joystick hat states.
 *
 *  See [joystick hat input](@ref joystick_hat) for how these are used.
 *
 *  @ingroup input
 *  @{ */
#define WMS_HAT_CENTERED           0
#define WMS_HAT_UP                 1
#define WMS_HAT_RIGHT              2
#define WMS_HAT_DOWN               4
#define WMS_HAT_LEFT               8
#define WMS_HAT_RIGHT_UP           (WMS_HAT_RIGHT | WMS_HAT_UP)
#define WMS_HAT_RIGHT_DOWN         (WMS_HAT_RIGHT | WMS_HAT_DOWN)
#define WMS_HAT_LEFT_UP            (WMS_HAT_LEFT  | WMS_HAT_UP)
#define WMS_HAT_LEFT_DOWN          (WMS_HAT_LEFT  | WMS_HAT_DOWN)

 /*! @ingroup input
  */
#define WMS_KEY_UNKNOWN            -1

  /*! @} */

  /*! @defgroup keys Keyboard key tokens
   *  @brief Keyboard key tokens.
   *
   *  See [key input](@ref input_key) for how these are used.
   *
   *  These key codes are inspired by the _USB HID Usage Tables v1.12_ (p. 53-60),
   *  but re-arranged to map to 7-bit ASCII for printable keys (function keys are
   *  put in the 256+ range).
   *
   *  The naming of the key codes follow these rules:
   *   - The US keyboard layout is used
   *   - Names of printable alphanumeric characters are used (e.g. "A", "R",
   *     "3", etc.)
   *   - For non-alphanumeric characters, Unicode:ish names are used (e.g.
   *     "COMMA", "LEFT_SQUARE_BRACKET", etc.). Note that some names do not
   *     correspond to the Unicode standard (usually for brevity)
   *   - Keys that lack a clear US mapping are named "WORLD_x"
   *   - For non-printable keys, custom names are used (e.g. "F4",
   *     "BACKSPACE", etc.)
   *
   *  @ingroup input
   *  @{
   */

   /* Printable keys */
#define WMS_KEY_SPACE              32
#define WMS_KEY_APOSTROPHE         39  /* ' */
#define WMS_KEY_COMMA              44  /* , */
#define WMS_KEY_MINUS              45  /* - */
#define WMS_KEY_PERIOD             46  /* . */
#define WMS_KEY_SLASH              47  /* / */
#define WMS_KEY_0                  48
#define WMS_KEY_1                  49
#define WMS_KEY_2                  50
#define WMS_KEY_3                  51
#define WMS_KEY_4                  52
#define WMS_KEY_5                  53
#define WMS_KEY_6                  54
#define WMS_KEY_7                  55
#define WMS_KEY_8                  56
#define WMS_KEY_9                  57
#define WMS_KEY_SEMICOLON          59  /* ; */
#define WMS_KEY_EQUAL              61  /* = */
#define WMS_KEY_A                  65
#define WMS_KEY_B                  66
#define WMS_KEY_C                  67
#define WMS_KEY_D                  68
#define WMS_KEY_E                  69
#define WMS_KEY_F                  70
#define WMS_KEY_G                  71
#define WMS_KEY_H                  72
#define WMS_KEY_I                  73
#define WMS_KEY_J                  74
#define WMS_KEY_K                  75
#define WMS_KEY_L                  76
#define WMS_KEY_M                  77
#define WMS_KEY_N                  78
#define WMS_KEY_O                  79
#define WMS_KEY_P                  80
#define WMS_KEY_Q                  81
#define WMS_KEY_R                  82
#define WMS_KEY_S                  83
#define WMS_KEY_T                  84
#define WMS_KEY_U                  85
#define WMS_KEY_V                  86
#define WMS_KEY_W                  87
#define WMS_KEY_X                  88
#define WMS_KEY_Y                  89
#define WMS_KEY_Z                  90
#define WMS_KEY_LEFT_BRACKET       91  /* [ */
#define WMS_KEY_BACKSLASH          92  /* \ */
#define WMS_KEY_RIGHT_BRACKET      93  /* ] */
#define WMS_KEY_GRAVE_ACCENT       96  /* ` */
#define WMS_KEY_WORLD_1            161 /* non-US #1 */
#define WMS_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define WMS_KEY_ESCAPE             256
#define WMS_KEY_ENTER              257
#define WMS_KEY_TAB                258
#define WMS_KEY_BACKSPACE          259
#define WMS_KEY_INSERT             260
#define WMS_KEY_DELETE             261
#define WMS_KEY_RIGHT              262
#define WMS_KEY_LEFT               263
#define WMS_KEY_DOWN               264
#define WMS_KEY_UP                 265
#define WMS_KEY_PAGE_UP            266
#define WMS_KEY_PAGE_DOWN          267
#define WMS_KEY_HOME               268
#define WMS_KEY_END                269
#define WMS_KEY_CAPS_LOCK          280
#define WMS_KEY_SCROLL_LOCK        281
#define WMS_KEY_NUM_LOCK           282
#define WMS_KEY_PRINT_SCREEN       283
#define WMS_KEY_PAUSE              284
#define WMS_KEY_F1                 290
#define WMS_KEY_F2                 291
#define WMS_KEY_F3                 292
#define WMS_KEY_F4                 293
#define WMS_KEY_F5                 294
#define WMS_KEY_F6                 295
#define WMS_KEY_F7                 296
#define WMS_KEY_F8                 297
#define WMS_KEY_F9                 298
#define WMS_KEY_F10                299
#define WMS_KEY_F11                300
#define WMS_KEY_F12                301
#define WMS_KEY_F13                302
#define WMS_KEY_F14                303
#define WMS_KEY_F15                304
#define WMS_KEY_F16                305
#define WMS_KEY_F17                306
#define WMS_KEY_F18                307
#define WMS_KEY_F19                308
#define WMS_KEY_F20                309
#define WMS_KEY_F21                310
#define WMS_KEY_F22                311
#define WMS_KEY_F23                312
#define WMS_KEY_F24                313
#define WMS_KEY_F25                314
#define WMS_KEY_KP_0               320
#define WMS_KEY_KP_1               321
#define WMS_KEY_KP_2               322
#define WMS_KEY_KP_3               323
#define WMS_KEY_KP_4               324
#define WMS_KEY_KP_5               325
#define WMS_KEY_KP_6               326
#define WMS_KEY_KP_7               327
#define WMS_KEY_KP_8               328
#define WMS_KEY_KP_9               329
#define WMS_KEY_KP_DECIMAL         330
#define WMS_KEY_KP_DIVIDE          331
#define WMS_KEY_KP_MULTIPLY        332
#define WMS_KEY_KP_SUBTRACT        333
#define WMS_KEY_KP_ADD             334
#define WMS_KEY_KP_ENTER           335
#define WMS_KEY_KP_EQUAL           336
#define WMS_KEY_LEFT_SHIFT         340
#define WMS_KEY_LEFT_CONTROL       341
#define WMS_KEY_LEFT_ALT           342
#define WMS_KEY_LEFT_SUPER         343
#define WMS_KEY_RIGHT_SHIFT        344
#define WMS_KEY_RIGHT_CONTROL      345
#define WMS_KEY_RIGHT_ALT          346
#define WMS_KEY_RIGHT_SUPER        347
#define WMS_KEY_MENU               348

#define WMS_KEY_LAST               WMS_KEY_MENU

/*! @} */

/*! @defgroup mods Modifier key flags
 *  @brief Modifier key flags.
 *
 *  See [key input](@ref input_key) for how these are used.
 *
 *  @ingroup input
 *  @{ */

 /*! @brief If this bit is set one or more Shift keys were held down.
  *
  *  If this bit is set one or more Shift keys were held down.
  */
#define WMS_MOD_SHIFT           0x0001
  /*! @brief If this bit is set one or more Control keys were held down.
   *
   *  If this bit is set one or more Control keys were held down.
   */
#define WMS_MOD_CONTROL         0x0002
   /*! @brief If this bit is set one or more Alt keys were held down.
	*
	*  If this bit is set one or more Alt keys were held down.
	*/
#define WMS_MOD_ALT             0x0004
	/*! @brief If this bit is set one or more Super keys were held down.
	 *
	 *  If this bit is set one or more Super keys were held down.
	 */
#define WMS_MOD_SUPER           0x0008
	 /*! @brief If this bit is set the Caps Lock key is enabled.
	  *
	  *  If this bit is set the Caps Lock key is enabled and the @ref
	  *  WMS_LOCK_KEY_MODS input mode is set.
	  */
#define WMS_MOD_CAPS_LOCK       0x0010
	  /*! @brief If this bit is set the Num Lock key is enabled.
	   *
	   *  If this bit is set the Num Lock key is enabled and the @ref
	   *  WMS_LOCK_KEY_MODS input mode is set.
	   */
#define WMS_MOD_NUM_LOCK        0x0020