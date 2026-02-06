#ifndef LAYOUT_PASSWORD_H
#define LAYOUT_PASSWORD_H

typedef enum
{
	INPUT_UNLOCK,
	INPUT_OLD_UNLOCK,
	INPUT_NEW_UNLOCK,
	INPUT_NEW_UNLOCK_AGAIN,
	INPUT_SETTING,
	INPUT_OLD_SETTING,
	INPUT_NEW_SETTING,
	INPUT_NEW_SETTING_AGAIN,
	TOTAL_PASSWORD_TRIGGER_MODE
} PASSWORD_TRIGGER_MODE;

typedef enum
{
	PASSWORD_STATUS_NONE,
	PASSWORD_ERROR,
	PASSWORD_OLD_ERROR,
	PASSWORD_SUCCESS,
	PASSWORD_WELLCOME
} PASSWORD_STATUS_SHOW;

typedef enum
{
	ENUM_PASSWORD_DIALOG_BOX1 = 0,
	ENUM_PASSWORD_DIALOG_BOX2,
	ENUM_PASSWORD_DIALOG_BOX_TOTAL
} ENUM_PASSWORD_DIALOG_BOX;

typedef struct
{
	STR_WidgetShow widget_show;
	STR_WidgetDialogBox **dialog_box;
	TouchKeyRegisterCallback key_register;
	PASSWORD_TRIGGER_MODE trigger_mode;
	PASSWORD_STATUS_SHOW password_status;
	ENUM_PASSWORD_DIALOG_BOX cur_focus;
	char new_password[15];
} STR_PasswordClass;

extern STR_PasswordClass PasswordClass;
extern layout layout_password;

#endif
