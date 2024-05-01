/*
Copyright 2022 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H

#include "quantum.h"

// コード表
// 【KBC_RST: 0x5DA5】Keyball 設定のリセット
// 【KBC_SAVE: 0x5DA6】現在の Keyball 設定を EEPROM に保存します
// 【CPI_I100: 0x5DA7】CPI を 100 増加させます(最大:12000)
// 【CPI_D100: 0x5DA8】CPI を 100 減少させます(最小:100)
// 【CPI_I1K: 0x5DA9】CPI を 1000 増加させます(最大:12000)
// 【CPI_D1K: 0x5DAA】CPI を 1000 減少させます(最小:100)
// 【SCRL_TO: 0x5DAB】タップごとにスクロールモードの ON/OFF を切り替えます
// 【SCRL_MO: 0x5DAC】キーを押している間、スクロールモードになります
// 【SCRL_DVI: 0x5DAD】スクロール除数を１つ上げます(max D7 = 1/128)← 最もスクロール遅い
// 【SCRL_DVD: 0x5DAE】スクロール除数を１つ下げます(min D0 = 1/1)← 最もスクロール速い

////////////////////////////////////
///
/// 自動マウスレイヤーの実装 ここから
/// 参考にさせていただいたページ
/// https://zenn.dev/takashicompany/articles/69b87160cda4b9
/// https://twitter.com/d_kamiichi
///
////////////////////////////////////

enum custom_keycodes
{
    KC_MY_BTN1 = KEYBALL_SAFE_RANGE, // Remap上では 0x5DAF / USER 0
    KC_MY_BTN2,                      // Remap上では 0x5DB0 / USER 1
    KC_MY_BTN3,                      // Remap上では 0x5DB1 / USER 2
    KC_MY_SCR,                       // Remap上では 0x5DB2 / USER 3
};

enum click_state
{
    NONE = 0,
    WAITING,    // マウスレイヤーが有効になるのを待つ。 Wait for mouse layer to activate.
    CLICKABLE,  // マウスレイヤー有効になりクリック入力が取れる。 Mouse layer is enabled to take click input.
    CLICKING,   // クリック中。 Clicking.
    SCROLLING,  // スクロール中。 Scrolling.
};

enum click_state state; // 現在のクリック入力受付の状態 Current click input reception status
uint16_t click_timer;   // タイマー。状態に応じて時間で判定する。 Timer. Time to determine the state of the system.

uint16_t to_reset_time = 800; // この秒数(千分の一秒)、CLICKABLE状態ならクリックレイヤーが無効になる。 For this number of seconds (milliseconds), the click layer is disabled if in CLICKABLE state.

const int16_t to_clickable_movement = 2; // クリックレイヤーが有効になるしきい値
const uint16_t click_layer = 4;          // マウス入力が可能になった際に有効になるレイヤー。Layers enabled when mouse input is enabled

const uint16_t ignore_disable_mouse_layer_keys[] = { KC_LGUI, KC_LCTL, KC_LALT, KC_LSFT, KC_RGUI, KC_RCTL, KC_RALT, KC_RSFT };   // この配列で指定されたキーはマウスレイヤー中に押下してもマウスレイヤーを解除しない

// クリック用のレイヤーを有効にする。　Enable layers for clicks
void enable_click_layer(void)
{
    layer_on(click_layer);
    click_timer = timer_read();
    state = CLICKABLE;
}

// クリック用のレイヤーを無効にする。 Disable layers for clicks.
void disable_click_layer(void)
{
    state = NONE;
    layer_off(click_layer);
}

// 自前の絶対数を返す関数。 Functions that return absolute numbers.
int16_t my_abs(int16_t num)
{
    return (num < 0) ? -num : num;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record)
{

    switch (keycode)
    {
        case KC_MY_BTN1:
        case KC_MY_BTN2:
        case KC_MY_BTN3:
            {
                report_mouse_t currentReport = pointing_device_get_report();

                // どこのビットを対象にするか。 Which bits are to be targeted?
                uint8_t btn = 1 << (keycode - KC_MY_BTN1);

                if (record->event.pressed)
                {
                    // ビットORは演算子の左辺と右辺の同じ位置にあるビットを比較して、両方のビットのどちらかが「1」の場合に「1」にします。
                    // Bit OR compares bits in the same position on the left and right sides of the operator and sets them to "1" if either of both bits is "1".
                    currentReport.buttons |= btn;
                    state = CLICKING;
                }
                else
                {
                    // ビットANDは演算子の左辺と右辺の同じ位置にあるビットを比較して、両方のビットが共に「1」の場合だけ「1」にします。
                    // Bit AND compares the bits in the same position on the left and right sides of the operator and sets them to "1" only if both bits are "1" together.
                    currentReport.buttons &= ~btn;
                    enable_click_layer();
                }

                pointing_device_set_report(currentReport);
                pointing_device_send();
                return false;
            }

        case KC_MY_SCR:
            if (record->event.pressed)
            {
                keyball_set_scroll_mode(true);
                state = SCROLLING;
            }
            else
            {
                keyball_set_scroll_mode(false);
                state = NONE;
                enable_click_layer();
            }
            return false;

        default:
            if (record->event.pressed)
            {
                for (int i = 0; i < sizeof(ignore_disable_mouse_layer_keys) / sizeof(ignore_disable_mouse_layer_keys[0]); i++)
                {
                    if (keycode == ignore_disable_mouse_layer_keys[i])
                    {
                        return true;
                    }
                }

                disable_click_layer();
            }
    }

    return true;
}

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report)
{
    int16_t current_x = mouse_report.x;
    int16_t current_y = mouse_report.y;

    if (current_x != 0 || current_y != 0)
    {

        switch (state)
        {
            case CLICKABLE:
                click_timer = timer_read();
                break;

            case CLICKING:
            case SCROLLING:
                break;

            case WAITING:
                int16_t mouse_movement = my_abs(current_x) + my_abs(current_y);

                if (mouse_movement >= to_clickable_movement)
                {
                    enable_click_layer();
                }
                break;

            default:
                click_timer = timer_read();
                state = WAITING;
        }
    }
    else
    {
        switch (state)
        {
            case CLICKING:
            case SCROLLING:
                break;

            case CLICKABLE:
                if (timer_elapsed(click_timer) > to_reset_time)
                {
                    disable_click_layer();
                }
                break;

            case WAITING:
                if (timer_elapsed(click_timer) > 50)
                {
                    state = NONE;
                }
                break;

            default:
                state = NONE;
        }
    }

    mouse_report.x = current_x;
    mouse_report.y = current_y;

    return mouse_report;
}

////////////////////////////////////
///
/// 自動マウスレイヤーの実装 ここまで
///
////////////////////////////////////

layer_state_t layer_state_set_user(layer_state_t state)
{
    // レイヤーが1または3の場合、スクロールモードが有効になる
    // keyball_set_scroll_mode(get_highest_layer(state) == 1 || get_highest_layer(state) == 3);
    bool isScrollOn = get_highest_layer(state) == 2;
    keyball_set_scroll_mode(isScrollOn);
    if (isScrollOn)
    {
        state = SCROLLING;
    }

#ifdef RGBLIGHT_ENABLE
    // レイヤーとLEDを連動させる
    uint8_t layer = biton32(state);
//*
    switch (layer)
    {
    case 4:
        rgblight_enable_noeeprom();
        break;

    default:
        rgblight_disable_noeeprom();
    }
/*/
    switch (layer)
    {
    case 1:
        rgblight_sethsv(HSV_CYAN);
        break;
    case 2:
        rgblight_sethsv(HSV_YELLOW);
        break;
    case 3:
        rgblight_sethsv(HSV_GREEN);
        break;
    case 4:
        rgblight_sethsv(0, 0, 70);
        break;
    default:
        rgblight_sethsv(HSV_OFF);
    }
//*/
#endif

    return state;
}

#ifdef OLED_ENABLE
#include "lib/oledkit/oledkit.h"
void oledkit_render_logo_user(void) {
    // Require `OLED_FONT_H "keyboards/keyball/lib/logofont/logofont.c"`
    char ch = 0x80;
    // マウスレイヤーの場合、文字色の白黒を反転させる
    bool isClicklayer = (get_highest_layer(layer_state) == click_layer);

    for (int y = 0; y < 3; y++) {
        oled_write_P(PSTR(" "), isClicklayer);
        for (int x = 0; x < 16; x++) {
            oled_write_char(ch++, isClicklayer);
        }
        oled_write_P(PSTR(" "), isClicklayer);
        oled_advance_page(isClicklayer);
    }
}

void oledkit_render_info_user(void)
{
    keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
//    keyball_oled_render_layerinfo();

    // マウスレイヤーの場合、文字色の白黒を反転させる
    bool isClicklayer = (get_highest_layer(layer_state) == click_layer);
    oled_write_P(PSTR("L\xB6\xB7r\xB1 "), isClicklayer);
    // oled_write(get_u8_str(get_highest_layer(layer_state), ' '), isClicklayer);
    // oled_write_P(PSTR("            "), isClicklayer);

    int layer = get_highest_layer(layer_state);
    for (int i = 0; i < DYNAMIC_KEYMAP_LAYER_COUNT; i++) {
        if (i == click_layer && isClicklayer) {
            oled_write(get_u8_str(i, ' '), true);
        } else {
            if (i == layer) {
                oled_write(get_u8_str(i, ' '), !isClicklayer);
            } else {
                oled_write(get_u8_str(i, ' '), isClicklayer);
            }
        }
    }
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return is_keyboard_left() ? OLED_ROTATION_180 : rotation;
}
#endif


// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  // keymap for default
  [0] = LAYOUT_universal(
    KC_Q     , KC_W     , KC_E     , KC_R     , KC_T     ,                            KC_Y     , KC_U     , KC_I     , KC_O     , KC_P     ,
    KC_A     , KC_S     , KC_D     , KC_F     , KC_G     ,                            KC_H     , KC_J     , KC_K     , KC_L     , KC_MINS  ,
    KC_Z     , KC_X     , KC_C     , KC_V     , KC_B     ,                            KC_N     , KC_M     , KC_COMM  , KC_DOT   , KC_SLSH  ,
    KC_LCTL  , KC_LGUI  , KC_LALT  ,LSFT_T(KC_LNG2),LT(1,KC_SPC),LT(3,KC_LNG1),KC_BSPC,LT(2,KC_ENT),LSFT_T(KC_LNG2),KC_RALT,KC_RGUI, KC_RSFT
  ),

  [1] = LAYOUT_universal(
    KC_F1    , KC_F2    , KC_F3    , KC_F4    , KC_RBRC  ,                            KC_F6    , KC_F7    , KC_F8    , KC_F9    , KC_F10   ,
    KC_F5    , KC_EXLM  , S(KC_6)  ,S(KC_INT3), S(KC_8)  ,                           S(KC_INT1), KC_BTN1  , KC_PGUP  , KC_BTN2  , KC_SCLN  ,
    S(KC_EQL),S(KC_LBRC),S(KC_7)   , S(KC_2)  ,S(KC_RBRC),                            KC_LBRC  , KC_DLR   , KC_PGDN  , KC_BTN3  , KC_F11   ,
    KC_INT1  , KC_EQL   , S(KC_3)  , _______  , _______  , _______  ,      TO(2)    , TO(0)    , _______  , KC_RALT  , KC_RGUI  , KC_F12
  ),

  [2] = LAYOUT_universal(
    KC_TAB   , KC_7     , KC_8     , KC_9     , KC_MINS  ,                            KC_NUHS  , _______  , KC_BTN3  , _______  , KC_BSPC  ,
   S(KC_QUOT), KC_4     , KC_5     , KC_6     ,S(KC_SCLN),                            S(KC_9)  , KC_BTN1  , KC_UP    , KC_BTN2  , KC_QUOT  ,
    KC_SLSH  , KC_1     , KC_2     , KC_3     ,S(KC_MINS),                           S(KC_NUHS), KC_LEFT  , KC_DOWN  , KC_RGHT  , _______  ,
    KC_ESC   , KC_0     , KC_DOT   , KC_DEL   , KC_ENT   , KC_BSPC  ,      _______  , _______  , _______  , _______  , _______  , _______
  ),

  [3] = LAYOUT_universal(
    RGB_TOG  , AML_TO   , AML_I50  , AML_D50  , _______  ,                            RGB_M_P  , RGB_M_B  , RGB_M_R  , RGB_M_SW , RGB_M_SN ,
    RGB_MOD  , RGB_HUI  , RGB_SAI  , RGB_VAI  , SCRL_DVI ,                            RGB_M_K  , RGB_M_X  , RGB_M_G  , RGB_M_T  , RGB_M_TW ,
    RGB_RMOD , RGB_HUD  , RGB_SAD  , RGB_VAD  , SCRL_DVD ,                            CPI_D1K  , CPI_D100 , CPI_I100 , CPI_I1K  , KBC_SAVE ,
    QK_BOOT  , KBC_RST  , _______  , _______  , _______  , _______  ,      _______  , _______  , _______  , _______  , KBC_RST  , QK_BOOT
  ),
  [4] = LAYOUT_universal(
    _______  , _______  , _______  , _______  , _______  ,                            _______  , _______  , _______  , _______  , _______  ,
    _______  , _______  , _______  , _______  , _______  ,                            _______  , _______  , _______  , _______  , _______  ,
    _______  ,KC_MY_BTN2, KC_MY_SCR,KC_MY_BTN1, _______  ,                            _______  ,KC_MY_BTN1, KC_MY_SCR,KC_MY_BTN2, _______  ,
    _______  , _______  , _______  , _______  , _______  , _______  ,      _______  , _______  , _______  , _______  , _______  , _______ 
  )
};
// clang-format on
