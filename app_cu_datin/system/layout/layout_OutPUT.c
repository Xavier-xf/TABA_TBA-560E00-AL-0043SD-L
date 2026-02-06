#include "os_sys_api.h"
#include "layout_base.h"
#include "ui_api.h"
#include "rom.h"
#include "string.h"

static void ouput_dialog_clear(ENUM_OUTPUT_DIALOG_BOX cur_focus)
{
    OutPUTClass.dialog_box[cur_focus]->cursor.index = 0;
    memset(OutPUTClass.dialog_box[cur_focus]->font.string1, 0, 10);
}
static void output_dialog_display(void)
{
    widget_dialog_box_display(OutPUTClass.dialog_box[OutPUTClass.cur_focus.dialog_box]);
}

static void output_icon_display(void)
{
    taba_btn_display();
}
static void OUT_font_display(void)
{
    position pos = {{40, 30}, {80, 40}};
    text system_set;

    text_init(&system_set, &pos, 26);
    system_set.align = LEFT_TOP;
    text_display(&system_set, font_str(STR_HOME_ID_OUT_ID));
}
static void UNIT_font_display(void)
{
    position pos = {{40, 80}, {80, 40}};
    text system_set;

    text_init(&system_set, &pos, 26);
    system_set.align = LEFT_TOP;
    text_display(&system_set, font_str(STR_HOME_ID_UNIT_SET));
}

static void Guard_font_display(void)
{
    position pos = {{40, 80}, {80, 40}};
    text system_set;

    text_init(&system_set, &pos, 26);
    system_set.align = LEFT_TOP;
    gui_erase(&pos, 0X00);
    text_display(&system_set, font_str(STR_HOME_ID_GUARD));
}
static void Save_font_display(void)
{
    position pos = {{310, 80}, {80, 40}};
    text system_set;

    text_init(&system_set, &pos, 26);
    system_set.align = LEFT_TOP;
    text_display(&system_set, font_str(STR_HOME_ID_SAVE));
}

static void Error_font_display(int y)
{
    position pos = {{310, y}, {150, 40}};
    text system_set;

    text_init(&system_set, &pos, 26);
    system_set.align = LEFT_TOP;
    text_display(&system_set, font_str(STR_HOME_ID_ERROR));
}
static void Error_font_clear(int y)
{
    position pos = {{310, y}, {150, 40}};
    gui_erase(&pos, 0X00);
}
static void output_font_display(void)
{
    OUT_font_display();
    UNIT_font_display();
}
static void input_out_dialog_box_init(void)
{
    icon box;
    STR_DialogFont font = {{0}, {0}, {0}, NULL, NULL, NULL};
    STR_Cursor cursor = {{{0, 0}, {0, 0}}, 0, 0, 2};
    static char string[20];

    position box_pos = {{120, 30}, {100, 40}};
    icon_init(&box, &box_pos, NULL);
    box.res.id = 0;
    position pos = {{120, 30}, {100, 40}};
    text_init(&font.text1, &pos, 26);
    font.string1 = string;
    font.text1.align = LEFT_TOP;

    widget_dialog_box_init(OutPUTClass.dialog_box[ENUM_OUTPUT_DIALOG_BOX1],
                           &box,
                           &font,
                           &cursor);
}
static void input_out_dialog_box2_init(void)
{
    icon box;
    STR_DialogFont font = {{0}, {0}, {0}, NULL, NULL, NULL};
    STR_Cursor cursor = {{{0, 0}, {0, 0}}, 0, 0, 3};
    static char string[20];

    position box_pos = {{120, 80}, {100, 40}};
    icon_init(&box, &box_pos, NULL);
    box.res.id = 0;
    position pos = {{120, 80}, {100, 40}};
    text_init(&font.text1, &pos, 26);
    font.string1 = string;
    font.text1.align = LEFT_TOP;

    widget_dialog_box_init(OutPUTClass.dialog_box[ENUM_OUTPUT_DIALOG_BOX2],
                           &box,
                           &font,
                           &cursor);
}
static void clear_prev_output_focus(void)
{

    position pos = {{270, 30}, {32, 32}};
    pos.point.y = 30 + 50 * OutPUTClass.cur_focus.main;
    gui_erase(&pos, 0x00);
}

static void display_current_output_focus(void)
{
    icon focus;
    position pos = {{270, 30}, {32, 32}};

    pos.point.y = 30 + 50 * OutPUTClass.cur_focus.main;
    resource res = resource_get(ROM_R_IMG_SET_PAGE_FOCUS_PNG);
    icon_init(&focus, &pos, &res);
    icon_display(&focus);
}
static void output_focus_display(void)
{
    clear_prev_output_focus();
    display_current_output_focus();
}

static void layout_OutPUT_init(void)
{
    input_out_dialog_box_init();
    input_out_dialog_box2_init();
}

static void layout_OutPUT_enter(void)
{
    OutPUTClass.widget_show.icon();
    OutPUTClass.widget_show.font(); // 显示文字
    OutPUTClass.widget_show.focus();
    OutPUTClass.key_register(); // 注册按键
}

static void layout_OutPUT_quit(void)
{
    LOG_WHITE(">>> quit layout output\n\r");

    OutPUTClass.dialog_box[OutPUTClass.cur_focus.dialog_box]->cursor.index = 0;
    memset(OutPUTClass.dialog_box[OutPUTClass.cur_focus.dialog_box]->font.string1, 0, 10);

    OutPUTClass.auth_info.output_status_count = 0;
    OutPUTClass.cur_focus.dialog_box = ENUM_OUTPUT_DIALOG_BOX1;
    OutPUTClass.cur_focus.main = OUPUT_OUT_FOCUS;
    OutPUTClass.cur_focus.status = OUTPUT_STATUS_NONE;
}

static void OutPUT_dialog_box_font_change(void)
{
    // 1. 获取当前输入框类型和光标索引
    ENUM_OUTPUT_DIALOG_BOX cur_dialog = OutPUTClass.cur_focus.dialog_box;
    unsigned char cursor_idx = OutPUTClass.dialog_box[cur_dialog]->cursor.index;
    char *str_buf = OutPUTClass.dialog_box[cur_dialog]->font.string1;

    // 2. 根据输入框类型，选择读取分支器序号或管理员ID
    if (cur_dialog == ENUM_OUTPUT_DIALOG_BOX1)
    {
        // 输入框1：读取分支器序号（brancher_seq[3]，索引0-2）
        switch (cursor_idx)
        {
        case 0:
            memset(str_buf, 0, 10);
            break;
        case 1:
            sprintf(str_buf, "%d", OutPUTClass.auth_info.brancher_seq[0]);
            break;
        case 2:
            sprintf(str_buf, "%d %d", OutPUTClass.auth_info.brancher_seq[0], OutPUTClass.auth_info.brancher_seq[1]);
            break;
        case 3:
            sprintf(str_buf, "%d %d %d", OutPUTClass.auth_info.brancher_seq[0], OutPUTClass.auth_info.brancher_seq[1], OutPUTClass.auth_info.brancher_seq[2]);
            break;
        }
    }
    else
    {
        // 其他输入框：读取管理员ID（admin_id[4]，索引0-3）
        switch (cursor_idx)
        {
        case 0:
            memset(str_buf, 0, 10);
            break;
        case 1:
            sprintf(str_buf, "%d", OutPUTClass.auth_info.admin_id[0]);
            break;
        case 2:
            sprintf(str_buf, "%d %d", OutPUTClass.auth_info.admin_id[0], OutPUTClass.auth_info.admin_id[1]);
            break;
        case 3:
            sprintf(str_buf, "%d %d %d", OutPUTClass.auth_info.admin_id[0], OutPUTClass.auth_info.admin_id[1], OutPUTClass.auth_info.admin_id[2]);
            break;
        case 4:
            sprintf(str_buf, "%d %d %d %d", OutPUTClass.auth_info.admin_id[0], OutPUTClass.auth_info.admin_id[1], OutPUTClass.auth_info.admin_id[2], OutPUTClass.auth_info.admin_id[3]);
            break;
        }
    }
}

static void output_input_add_number(unsigned char number)
{
    if (OutPUTClass.cur_focus.status != OUTPUT_STATUS_NONE)
        return;
    ENUM_OUTPUT_DIALOG_BOX cur_dialog = OutPUTClass.cur_focus.dialog_box;
    unsigned char index = OutPUTClass.dialog_box[cur_dialog]->cursor.index;

    // 校验光标索引不超过最大值（分支器3位→max_index=2；管理员4位→max_index=3）
    if (index <= OutPUTClass.dialog_box[OutPUTClass.cur_focus.dialog_box]->cursor.max_index)
    {
        // 根据输入框类型，保存数字到对应数组
        if (cur_dialog == ENUM_OUTPUT_DIALOG_BOX1)
        {
            // 输入框1：保存到分支器序号
            OutPUTClass.auth_info.brancher_seq[index] = number;
        }
        else
        {
            // 其他输入框：保存到管理员ID
            OutPUTClass.auth_info.admin_id[index] = number;
        }
        // 光标后移，更新显示
        OutPUTClass.dialog_box[OutPUTClass.cur_focus.dialog_box]->cursor.index++;
        OutPUT_dialog_box_font_change();
        OutPUTClass.widget_show.dialog_box();
    }
}

static void output_input_del_number(void)
{
    unsigned char index = OutPUTClass.dialog_box[OutPUTClass.cur_focus.dialog_box]->cursor.index;
    if (index <= 0)
    {
        return;
    }
    else
    {
        OutPUTClass.dialog_box[OutPUTClass.cur_focus.dialog_box]->cursor.index--;
        OutPUT_dialog_box_font_change();
        gui_erase(&OutPUTClass.dialog_box[OutPUTClass.cur_focus.dialog_box]->box.pos, 0XFF20428A);
        OutPUTClass.widget_show.dialog_box();
    }
}
static int cal_culate_brancher_id(void)
{
    // 健壮性检查
    if (OutPUTClass.auth_info.brancher_seq == NULL)
    {
        return -1;
    }

    // 核心计算逻辑
    int brancher_id = OutPUTClass.auth_info.brancher_seq[0] * 100 +
                      OutPUTClass.auth_info.brancher_seq[1] * 10 +
                      OutPUTClass.auth_info.brancher_seq[2] * 1;

    LOG_WHITE("输入的brancher_id数  %d\n", brancher_id);
    return brancher_id;
}
static bool validity_check_brancher_id(void)
{
    unsigned int brancher_seq; // 对应输入的那个 四位数 [千 百 十 个]
    brancher_seq = cal_culate_brancher_id();

    /* 这里的 home id 同时又是输入的数字 */
    LOG_WHITE("pk的brancher_seq数  %d\n", brancher_seq);

    if (brancher_seq >= 0 && brancher_seq < 128)
    {
        LOG_WHITE("return true\n");
        return true;
    }
    return false;
}
static int cal_culate_admin_id(void)
{
    // 健壮性检查
    if (OutPUTClass.auth_info.admin_id == NULL)
    {
        return -1;
    }

    // 核心计算逻辑
    int admin_id = OutPUTClass.auth_info.admin_id[0] * 1000 +
                   OutPUTClass.auth_info.admin_id[1] * 100 +
                   OutPUTClass.auth_info.admin_id[2] * 10 +
                   OutPUTClass.auth_info.admin_id[3];

    LOG_WHITE("输入的admin_id数  %d\n", admin_id);
    return admin_id;
}

static bool validity_check_unit_admin(void)
{
    unsigned int admin_id; // 对应输入的那个 四位数 [千 百 十 个]
    admin_id = cal_culate_admin_id();

    /* 这里的 home id 同时又是输入的数字 */
    LOG_WHITE("pk的admin_id数  %d\n", admin_id);

    if (admin_id > 0 && admin_id < USER_CARD_TOTAL / 10)
    {
        LOG_WHITE("return true\n");
        return true;
    }
    return false;
}
static void OutPUT_home_id_adjust(void)
{
    unsigned char i, j;
    unsigned char index = OutPUTClass.dialog_box[OutPUTClass.cur_focus.dialog_box]->cursor.index;
    unsigned char max_index = OutPUTClass.dialog_box[OutPUTClass.cur_focus.dialog_box]->cursor.max_index;
    LOG_WHITE("index %d,max_index %d\n", index, max_index);
    for (i = index; i <= max_index; i++)
    {
        for (j = max_index; j > 0; j--)
        {
            if (OutPUTClass.cur_focus.dialog_box == ENUM_OUTPUT_DIALOG_BOX1)
            {
                OutPUTClass.auth_info.brancher_seq[j] = OutPUTClass.auth_info.brancher_seq[j - 1];
            }
            else
            {
                OutPUTClass.auth_info.admin_id[j] = OutPUTClass.auth_info.admin_id[j - 1];
            }
        }
        if (OutPUTClass.cur_focus.dialog_box == ENUM_OUTPUT_DIALOG_BOX1)
        {
            OutPUTClass.auth_info.brancher_seq[0] = 0;
        }
        else
        {
            OutPUTClass.auth_info.admin_id[0] = 0;
        }
    }
}

static void goto_next_output_focus(void)
{
    (OutPUTClass.cur_focus.main == OUPUT_UNIT_FOCUS) ? (OutPUTClass.cur_focus.main = OUPUT_OUT_FOCUS) : (OutPUTClass.cur_focus.main = OUPUT_UNIT_FOCUS);
}
static void output_key0_up(void)
{
    output_input_add_number(0);
}
static void output_key1_up(void)
{
    output_input_add_number(1);
}
static void output_key2_up(void)
{
    output_input_add_number(2);
}
static void output_key3_up(void)
{
    output_input_add_number(3);
}
static void output_key4_up(void)
{
    output_input_add_number(4);
}
static void output_key5_up(void)
{
    output_input_add_number(5);
}
static void output_key6_up(void)
{
    output_input_add_number(6);
}
static void output_key7_up(void)
{
    output_input_add_number(7);
}
static void output_key8_up(void)
{
    output_input_add_number(8);
}
static void output_key9_up(void)
{
    output_input_add_number(9);
}

static void output_key_down_up(void)
{

    clear_prev_output_focus();
    goto_next_output_focus();
    display_current_output_focus();
    Guard_font_display();
}

static void output_key_star_up(void)
{
    if (OutPUTClass.dialog_box[OutPUTClass.cur_focus.dialog_box]->cursor.index == 0)
    {
        os_layout_goto(&layout_settings);
    }
    else
    {
        output_input_del_number();
    }
}

static void output_key_ring_up(void)
{
    if (OutPUTClass.cur_focus.status != OUTPUT_STATUS_NONE)
        return;
    switch (OutPUTClass.cur_focus.main)
    {
    case OUPUT_OUT_FOCUS:
        OutPUT_home_id_adjust();
        if (cal_culate_brancher_id() == OUTPUT_ADMIN_ID_SET)
        {
            ouput_dialog_clear(ENUM_OUTPUT_DIALOG_BOX1);
            output_key_down_up();
            OutPUTClass.cur_focus.dialog_box = ENUM_OUTPUT_DIALOG_BOX2;
        }
        else
        {
            if (validity_check_brancher_id() == true)
            {
                os_layout_goto(&layout_home_id_set);
            }
            else
            {
                ouput_dialog_clear(ENUM_OUTPUT_DIALOG_BOX1);
                gui_erase(&OutPUTClass.dialog_box[OutPUTClass.cur_focus.dialog_box]->box.pos, 0XFF20428A);
                OutPUTClass.widget_show.dialog_box();
                OutPUTClass.cur_focus.status = OUTPUT_STATUS_ERROR;
            }
        }

        break;
    case OUPUT_UNIT_FOCUS:
        OutPUT_home_id_adjust();
        if (validity_check_unit_admin() == true)
        {
            if (set_int_conf(ADMIN_CALL_NUMBER, cal_culate_admin_id()) == 0)
            {
                LOG_WHITE("设置成功\n");
                OutPUTClass.cur_focus.status = OUTPUT_STATUS_SUCCESS;
            }
            else
            {
                OutPUTClass.cur_focus.status = OUTPUT_STATUS_ERROR;
                LOG_WHITE("设置失败\n");
            }
        }
        else
        {
            ouput_dialog_clear(ENUM_OUTPUT_DIALOG_BOX2);
            gui_erase(&OutPUTClass.dialog_box[OutPUTClass.cur_focus.dialog_box]->box.pos, 0XFF20428A);
            OutPUTClass.widget_show.dialog_box();
            OutPUTClass.cur_focus.status = OUTPUT_STATUS_ERROR;
        }
        break;
    default:
        break;
    }
}
static void output_key_register(void)
{
    key_touch_handle_register(KEY_INDEX_0, KEY_STATE_DOWN, output_key0_up);
    key_touch_handle_register(KEY_INDEX_1, KEY_STATE_DOWN, output_key1_up);
    key_touch_handle_register(KEY_INDEX_2, KEY_STATE_DOWN, output_key2_up);
    key_touch_handle_register(KEY_INDEX_3, KEY_STATE_DOWN, output_key3_up);
    key_touch_handle_register(KEY_INDEX_4, KEY_STATE_DOWN, output_key4_up);
    key_touch_handle_register(KEY_INDEX_5, KEY_STATE_DOWN, output_key5_up);
    key_touch_handle_register(KEY_INDEX_6, KEY_STATE_DOWN, output_key6_up);
    key_touch_handle_register(KEY_INDEX_7, KEY_STATE_DOWN, output_key7_up);
    key_touch_handle_register(KEY_INDEX_8, KEY_STATE_DOWN, output_key8_up);
    key_touch_handle_register(KEY_INDEX_9, KEY_STATE_DOWN, output_key9_up);

    key_touch_handle_register(KEY_INDEX_STAR, KEY_STATE_DOWN, output_key_star_up);
    key_touch_handle_register(KEY_INDEX_POUND, KEY_STATE_DOWN, output_key_ring_up);
}
static void layout_OutPUT_timer(void)
{
    if (OutPUTClass.cur_focus.status != OUTPUT_STATUS_NONE)
    {
        if (OutPUTClass.auth_info.output_status_count == 3)
        {
            if (OutPUTClass.cur_focus.status == OUTPUT_STATUS_SUCCESS)
            {
                Save_font_display();
            }
            else if (OutPUTClass.cur_focus.status == OUTPUT_STATUS_ERROR)
            {
                if (OutPUTClass.cur_focus.dialog_box == ENUM_OUTPUT_DIALOG_BOX1)
                    Error_font_display(30);
                else
                    Error_font_display(80);
            }
        }
        OutPUTClass.auth_info.output_status_count++;
        if (OutPUTClass.auth_info.output_status_count > 40)
        {
            if (OutPUTClass.cur_focus.status == OUTPUT_STATUS_SUCCESS)
            {
                os_layout_goto(&layout_OutPUT);
            }
            OutPUTClass.auth_info.output_status_count = 0;
            OutPUTClass.cur_focus.status = OUTPUT_STATUS_NONE;
            if (OutPUTClass.cur_focus.dialog_box == ENUM_OUTPUT_DIALOG_BOX1)
                Error_font_clear(30);
            else
                Error_font_clear(80);
        }
    }
}
STR_WidgetDialogBox OutPUTDialogBox1;
STR_WidgetDialogBox OutPUTDialogBox2;
STR_WidgetDialogBox *OutPUTDialogBox[ENUM_OUTPUT_DIALOG_BOX_TOTAL] = {
    &OutPUTDialogBox1,
    &OutPUTDialogBox2};
STR_OutPUTClass OutPUTClass = {
    {output_icon_display, output_font_display, output_focus_display, output_dialog_display, NULL},
    OutPUTDialogBox,
    output_key_register,
    {OUPUT_OUT_FOCUS, ENUM_OUTPUT_DIALOG_BOX1, OUTPUT_STATUS_NONE},
    {{0}, {0}, 0}};

layout layout_OutPUT = {
    .init = layout_OutPUT_init,
    .enter = layout_OutPUT_enter,
    .quit = layout_OutPUT_quit,
    .timer = layout_OutPUT_timer};