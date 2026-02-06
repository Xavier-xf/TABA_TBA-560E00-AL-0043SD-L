#include "ui_api.h"

extern void(*button_down_proc_callback)(void);

static void(*touch_key_proc[KEY_INDEX_TOTAL][KEY_STATE_TOTAL])(void) = {{NULL}};

void key_touch_handle_destory(void)
{
	int i,j;
	for(i = 0 ;i < KEY_INDEX_TOTAL; i ++)
	{
		for(j = 0 ; j < KEY_STATE_TOTAL; j++)
		{
			if( touch_key_proc[i][j] != NULL)
			{
				touch_key_proc[i][j] = NULL;
			}
		}
	}
}


bool key_touch_handle_register(KEY_INDEX key_index,KEY_STATE key_state,void(*key_func)(void))
{
	if((key_index >= KEY_INDEX_TOTAL)||(key_state >= KEY_STATE_TOTAL))
	{
		return false;
	}
	touch_key_proc[key_index][key_state] = key_func;
	return true;
}


//extern bool key_num_flag;
bool key_touch_event_handle(int key_index,int key_state)
{
	if((key_index >= KEY_INDEX_TOTAL)||(key_state >= KEY_STATE_TOTAL))
	{
		return false;
	}
	// if(key_num_flag == true)
	// {
	// 	if(language_get() == 0)
	// 	{
	// 		if(key_state == KEY_STATE_DOWN)
	// 		{
	// 			if((key_index >= KEY_INDEX_0) && (key_index <= KEY_INDEX_9))
	// 			{
	// 				touch_sound_num_play(key_index);
	// 			}
	// 			else
	// 			{
	// 				button_down_proc_callback();
	// 			}
				
	// 		}
	// 	}
	// 	else
	// 	{
	// 		if(key_state == KEY_STATE_DOWN)
	// 		{
	// 			if((key_index >= KEY_INDEX_0) && (key_index <= KEY_INDEX_9))
	// 			{
	// 				touch_sound_num_play(key_index+10);
	// 			}
	// 			else 
	// 			{
	// 				button_down_proc_callback();
	// 			}
				
	// 		}
	// 	}
	// }
	if(key_state == KEY_STATE_DOWN)
	{
		if(button_down_proc_callback != NULL)
		{
			button_down_proc_callback();
		}
	}
	//standby_timer_reset();
	if(touch_key_proc[key_index][key_state] != NULL)
	{
		touch_key_proc[key_index][key_state]();
	}
	return true;
}

