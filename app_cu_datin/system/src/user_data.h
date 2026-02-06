#ifndef USER_DATA_H
#define USER_DATA_H

#define USER_CARD_TOTAL 31000

enum sys_conf
{
	LANGUAGE,
	VOLUME,
	SET_HOME_ID_INDEX,
	UNIT_NUMBER_INDEX,
	ADMIN_CALL_NUMBER,
	UNLOCK_PASSWORD,
	SET_PASSWORD,
	CONFIGURE_MAX
};

typedef struct
{
	int home_id[10000];
	int unit_number[10000];
} STR_UserData;

extern STR_UserData UserData;

extern char *get_card_id_data(int card_number);
extern void card_id_data_save(void);
extern void card_id_data_init(void);
extern void deleteAllCard(void);
void set_card_id_data(unsigned int card_number, char *string);

extern void user_data_init(void);
extern void user_data_save(void);
extern int get_int_conf(enum sys_conf index);
extern int set_int_conf(enum sys_conf index, int value);
extern char *get_string_conf(enum sys_conf index);
extern int set_string_conf(enum sys_conf index, char *value);
extern void user_configure_init(void);
extern void restore_factory_settings(void);

#endif
