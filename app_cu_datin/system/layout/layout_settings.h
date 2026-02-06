#ifndef LAYOUT_SETTINGS_H
#define LAYOUT_SETTINGS_H



typedef enum{
	SYSTEM_SET_FOCUS,
	UNLOCK_SET_FOCUS,
	PASSWORD_SET_FOCUS,
	FLOOR_SET_FOCUS,
	CARD_SET_FOCUS,
	TOTAL_SET_FOCUS
}SETTING_FOUCUS;



typedef struct{
	STR_WidgetShow widget_show;
	TouchKeyRegisterCallback key_register;
	SETTING_FOUCUS cur_focus;
}STR_SettingsClass;


extern STR_SettingsClass SettingsClass;
extern layout layout_settings;










#endif






