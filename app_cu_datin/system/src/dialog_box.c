#include "dialog_box.h"

/*********************************************************************************
@Function:    对话框部件初始化
@Input:       widget_dialog_box: 要初始化的对话框
			  box: 对话框框图标
			  font: 对话框字体
			  cursor: 对话框光标
@Output:      (void)
@Description: 将对话框整个部件封装成一个结构体，初始化整个对话框部件，对话框部件
			  主要由框图标、对话框字体(目前最多三个字符串)、光标组成。
@Author:      kevin
@Date:        2021-10-10
*********************************************************************************/
void widget_dialog_box_init(STR_WidgetDialogBox *widget_dialog_box,
							icon *box,
							STR_DialogFont *font,
							STR_Cursor *cursor)
{
	widget_dialog_box->box = *box;
	widget_dialog_box->font = *font;
	widget_dialog_box->cursor = *cursor;
}

/*********************************************************************************
@Function:    对话框部件显示
@Input:	   widget_dialog_box: 要显示的对话框
@Output:	   (void)
@Description: 此函数用于对话框部件的显示，如果对话框部件有图标、字体、光标则显示
			  必须初始化对话框之后才能显示
@Author:	   kevin
@Date: 	   2021-10-10
*********************************************************************************/
void widget_dialog_box_display(STR_WidgetDialogBox *widget_dialog_box)
{

	/**********************************************************************************
	判断对话框是否有图标，如果有就显示图标
	**********************************************************************************/
	if (widget_dialog_box->box.res.id == 0)
	{
		printf("none dialog box icon\n\r");
	}
	else
	{
		gui_erase(&widget_dialog_box->box.pos, 0x00);
		icon_display(&widget_dialog_box->box);
	}

	/**********************************************************************************
	判断对话框是否有字符串，如果有就显示字符串，目前对话框最多可显示三个字符串，
	如果大于三个则需要修改对话框结构体的内容
	**********************************************************************************/
	if (widget_dialog_box->font.string1 == NULL)
	{
		// printf("none dialog box font string1\n\r");
	}
	else
	{
		text_display(&widget_dialog_box->font.text1, widget_dialog_box->font.string1);
	}

	if (widget_dialog_box->font.string2 == NULL)
	{
		// printf("none dialog box font string2\n\r");
	}
	else
	{
		text_display(&widget_dialog_box->font.text2, widget_dialog_box->font.string2);
	}

	if (widget_dialog_box->font.string3 == NULL)
	{
		// printf("none dialog box font string3\n\r");
	}
	else
	{
		text_display(&widget_dialog_box->font.text3, widget_dialog_box->font.string3);
	}

	/**********************************************************************************
	判断对话框是否有光标，如果有就画光标，此处通过判断光标的宽度是否为0确定是否有光标
	**********************************************************************************/
	if (widget_dialog_box->cursor.pos.vector.width == 0)
	{
		// printf("none dialog box cursor\n\r");
	}
	else
	{
		draw_rect(&widget_dialog_box->cursor.pos, widget_dialog_box->cursor.color);
	}
}
