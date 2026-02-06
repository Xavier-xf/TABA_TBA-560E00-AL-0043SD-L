#include "ui_api.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>

static point touch_down_point_xy = {0};
static point current_touch_point = {0};

static int button_listens_index = 0;
static const button* pbutton_listens_group[BUTTON_MAX];

static enum language  cur_language = language_english;

void(*button_down_proc_callback)(void) = NULL;

static touch_func_callback touch_outside_of_all_button_func = NULL;
static void(*touch_slide_function)(char)  = NULL;

void touch_other_down_register(touch_func_callback down_callback)
{
	touch_outside_of_all_button_func = down_callback;
}

void touch_slide_callback_register(touch_slide_func_callback slide_callback)
{
    touch_slide_function = slide_callback;
}

bool button_init(button*btn,const position* pos,touch_func_callback up)
{
	btn->pos = *pos;
	btn->bg_color = 0x00;
	btn->erase = false;

	btn->icon_align = CENTER_MIDDLE;
	btn->icon_offset.x = btn->icon_offset.y = 0;
	btn->resource.id = btn->resource.size = 0;

	btn->font_size = 0;
	btn->font_color = 0xFFFFFF;
	btn->font_align = CENTER_MIDDLE;
	btn->font_offset.x = btn->font_offset.y = 0;
	btn->string = NULL;

	btn->down = NULL;
	btn->leave_down = NULL;
	btn->up = up;
	btn->motion = NULL;
	btn->long_down = NULL;
	return true;
}


bool button_icon_init(button* btn,resource*resource)
{
	btn->resource = *resource;
	return true;
}


bool button_text_init(button* btn,void* string,int font_size)
{
	btn->string = string;
	btn->font_size = font_size;
	return true;
}


bool button_display(button*btn)
{

	if(btn->erase == true)
    {
		gui_erase(&(btn->pos), 0x000000);
	}
	else if(btn->bg_color&0xFF000000)
    {
		draw_rect(&(btn->pos), btn->bg_color);
	}
	
	if(btn->resource.id && btn->resource.size)
    {
		icon icon;
		icon_init(&icon, &btn->pos, &btn->resource);
		icon.align = btn->icon_align;
		icon.offset = btn->icon_offset;
		icon_display(&icon);
	}
	char** string = (char**)btn->string;
	if((btn->string != NULL)&&(string[cur_language] != NULL))
    {

		text text;
		text_init(&text, &btn->pos, btn->font_size);
		text.font_color = btn->font_color;
		text.align = btn->font_align;
		text.offset = btn->font_offset;
		text_display(&text, string[cur_language]);
	}
	return true;
}


static bool button_listen_self_check(const button* pbtn,int* index)
{
    int i = 0;
	for( i = 0 ; i < button_listens_index ; i++)
    {
		if(pbutton_listens_group[i] == pbtn)
        {
			*index = i;
			return true;
		}
	}
	return false;
}


bool button_disable(const button* pbtn)
{
	int index = 0 ; 
	if(button_listen_self_check(pbtn, &index) == true)
    {
		if(index < (button_listens_index - 1))
        {
			memcpy(&pbutton_listens_group[index],&pbutton_listens_group[index+1],sizeof(button*)*(button_listens_index - index));
		}
		button_listens_index--;
		return true;
	}
	return false;
}

bool button_enable(const button* pbtn)
{
	if(button_listens_index >= BUTTON_MAX)
    {
		return false;
	}

	int index = 0 ; 
	if(button_listen_self_check(pbtn, &index) == true)
    {
		return false;
	}
	pbutton_listens_group[button_listens_index++] = pbtn;
	return true;
}


bool button_listens_copy(button** pbtn_group,int* index){

	*index = button_listens_index;
	memcpy(pbtn_group,pbutton_listens_group,sizeof(button*)*button_listens_index);
	return true;
}

bool button_listens_recovery(button** pbtn_group,int index){

	button_listens_index = index;
	memcpy(pbutton_listens_group,pbtn_group,sizeof(button*)*index);
	return true;
}


bool button_listens_destroy(void)
{
	button_listens_index = 0;
	memset(pbutton_listens_group,0,sizeof(button*)*BUTTON_MAX);
	return true;
}



const point* touch_down_point_get(void){

	return &touch_down_point_xy;
}


const point* touch_current_point(void){

	return &current_touch_point;
}


void language_set(enum language lang)
{
    cur_language = lang;
}

enum language language_get(void)
{
    return cur_language;
}

static bool is_point_in_scope(const point* point,const position* rect){

    if((point->x < rect->point.x)||(point->y < rect->point.y)){
        return false;
    }
    if((point->x >(rect->point.x + rect->vector.width)) ||(point->y > (rect->point.y + rect->vector.height))){
        return false;
    }
    return true;
}
static int get_touched_button_index(const point* touched_xy){
    for( int i = 0 ; i < button_listens_index ; i++)
	{
        if(is_point_in_scope(touched_xy,&pbutton_listens_group[i]->pos) == true)
		{
            return i;
        }
    }
    return -1;
}

static bool touch_point_motion_flag = false;
static int selected_button_index = -1;
static point touched_point = {0,0};
static point released_point = {0,0};

static void touched_point_tmp_store(const point* touched){
    touched_point.x = touched->x;
    touched_point.y = touched->y;
}
static void released_point_tmp_store(const point* released){
    released_point.x = released->x;
    released_point.y = released->y;
}
static enum touch_slide_direction check_sliding_direction(void)
{
    int absolute_x = abs(released_point.x - touched_point.x);
    int absolute_y = abs(released_point.y - touched_point.y);
    if ((absolute_x - absolute_y) > 20)
	{ /* x axis movement*/
        if (released_point.x - touched_point.x > 20)
		{
            return SLIDE_RIGHT;
        }
        else if (touched_point.x - released_point.x > 20){
            return SLIDE_LEFT;
        }
    }
	else if ((absolute_y - absolute_x) > 20) 
	{ /* y axis movement*/
        if (released_point.y - touched_point.y > 20)
		{
            return SLIDE_DOWN;
        }
		else if (touched_point.y - released_point.y > 20)
		{
            return SLIDE_UP;
        }
    }
    return INVALID_MOVE;
}
static void button_touch_down_callback(int button_index)
{
	if(button_index > -1)
	{
        if(button_down_proc_callback != NULL)
		{
            button_down_proc_callback();
        }
        if(pbutton_listens_group[button_index]->down != NULL)
		{
            pbutton_listens_group[button_index]->down(pbutton_listens_group[button_index]);
        }
        selected_button_index = button_index;
    }
}
static bool button_touch_release_callback(int button_index,const point *touch_point)
{
	if(selected_button_index > -1)
	{
		if(pbutton_listens_group[selected_button_index]->leave_down != NULL)
		{
			pbutton_listens_group[selected_button_index]->leave_down
			(
				pbutton_listens_group[selected_button_index]);
		}
		//selected_button_index = -1;
	}

	if((button_index > -1) && (selected_button_index == button_index))
	{
        if(pbutton_listens_group[button_index]->up != NULL)
		{
            pbutton_listens_group[button_index]->up(pbutton_listens_group[button_index]);
            return true;
        }
    }
	else if(touch_outside_of_all_button_func != NULL)
	{
        touch_outside_of_all_button_func(touch_point);
        return true;
    }
    return false;
}
static void button_touch_motion_callback(int button_index){
    if(button_index > -1)
	{
        if(selected_button_index == button_index)
		{
            if(pbutton_listens_group[button_index]->motion != NULL)
			{
                pbutton_listens_group[button_index]->motion(pbutton_listens_group[button_index]);
            }
        }
		else if(selected_button_index > -1)
		{
            if(pbutton_listens_group[selected_button_index]->leave_down != NULL)
			{

                pbutton_listens_group[selected_button_index]->leave_down(pbutton_listens_group[selected_button_index]);
            }
            selected_button_index = -1;
        }
    }
	else if(selected_button_index > -1)
	{
        if(pbutton_listens_group[selected_button_index]->leave_down != NULL){

            pbutton_listens_group[selected_button_index]->leave_down(pbutton_listens_group[selected_button_index]);
        }
        selected_button_index = -1;
    }
}
static void button_touch_long_press(int button_index){
    if(button_index > -1)
	{
        if(pbutton_listens_group[button_index]->long_down != NULL)
		{
            pbutton_listens_group[button_index]->long_down(pbutton_listens_group[button_index]);
        }
    }
}
static bool button_touch_slide_callback(void)
{
    enum touch_slide_direction direction = check_sliding_direction();
	
    if(direction != INVALID_MOVE )
	{
        if(touch_slide_function != NULL)
		{
            touch_slide_function(direction);
            return true;
        }
    }
    return false;
}
bool button_touch_event_handle(char state,const point* touch_point)
{
    int button_index = get_touched_button_index(touch_point);
	
	current_touch_point = *touch_point;
	
    if(state == TOUCH_STATE_DOWN)
	{
        touch_down_point_xy = *touch_point;

        button_touch_down_callback(button_index);
        touched_point_tmp_store(touch_point);
        touch_point_motion_flag = false;
    }
	else if(state == TOUCH_STATE_UP)
	{
        //int ret;
        //ret = 
        button_touch_release_callback(button_index, touch_point);
        if(touch_point_motion_flag)
		{
            touch_point_motion_flag = false;
            released_point_tmp_store(touch_point);
            //if(ret == false)
            {
                //ret = 
                button_touch_slide_callback();
            }
        }
        selected_button_index = -1;
    }
	else if(state == TOUCH_STATE_MOTION)
	{
        button_touch_motion_callback(button_index);
        touch_point_motion_flag = true;
    }
	else if(state == TOUCH_STATE_LONG_DOWN)
	{
        button_touch_long_press(button_index);
    }
    return true;
}



