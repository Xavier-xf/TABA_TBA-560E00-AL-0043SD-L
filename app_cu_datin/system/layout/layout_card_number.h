#ifndef LAYOUT_CARD_NUMBER_H
#define LAYOUT_CARD_NUMBER_H


typedef enum{
	DELETE_CARD_MODE,
	ADD_CARD_MODE,
}CARD_NUMBER_TRIGGER_MODE;

typedef enum{
	CARD_NUMBER_STATUS_NONE,
	DELETE_CARD_NUMBER_SUCCESS,
	INPUT_CARD_NUMBER_ERROR
}CARD_NUMBER_STATUS;


typedef struct{
	STR_WidgetShow widget_show;
	STR_WidgetDialogBox *dialog_box;
	TouchKeyRegisterCallback key_register;
	CARD_NUMBER_TRIGGER_MODE trigger_mode;
	CARD_NUMBER_STATUS status;
	unsigned char home_id[4];
}STR_CardNumber;


extern STR_CardNumber CardNumberClass;
extern layout layout_card_number;










#endif






