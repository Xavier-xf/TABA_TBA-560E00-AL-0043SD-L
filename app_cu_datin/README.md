# Anyka3760E-APP-SDK



## goto next
```C
(C.focus == M1) ? (C.focus = M2) :(C.focus == M2) ? (C.focus = M3) :(C.focus == M3) ? (C.focus = M4) :(C.focus =  M1);	


M1	M2	M3	M4
```
## 锁 while



## save room number 

```C
UserData.home_id[0] = 101
UserData.home_id[1] = 102
UserData.home_id[2] = 103
UserData.home_id[3] = 104
UserData.home_id[4] = 105
UserData.home_id[5] = 106
UserData.home_id[5] = 107

读取分支器上的 	原数字		beforNumber
[M1]	[M2]	[M3]	[M4]
101		 102	  103		104

新保存的数字		newNumber
[M1]	[M2]	[M3]	[M4]
111		 222	  333		444

获取原数字对应的下标
for(i = 0; i < get_int_conf(SET_HOME_ID_INDEX); i++){
	if(UserData.home_id[i] == beforNumber){
		LOG_WHITE("UserData.home_id[%d]:%d is exist\n\r",i, UserData.home_id[i]);
		return i;
	}
}

取代原来的位置
UserData.home_id[i] = newNumber;


id = data1*1000 + data2*100+data3		// 1123
home = id%1000			// 123
m = id - home		// 1


```


## status

```C
CallingClass.call_object = //	CALL_USER,CALL_GUARD



```





## 通信

关闭正在监控的室内机


先获取当前状态
Intercom.status =？


大厅机判断 是否在通话 
在通话时，不能按下[Call]按钮


```
室内机监控
	按下监控      monitor start
							receive ack
	退出监控      monitor end
							receive ack 

```


```C
static void goto_prev_settings_focus(void){


	(SystemSetClass.cur_focus.main == TIME_SET_FOCUS)   ? (SystemSetClass.cur_focus.main = SYSTEM_SET_RESET_FOCUS) :
	(SystemSetClass.cur_focus.main == LANGUAGE_SET_FOCUS)   ? (SystemSetClass.cur_focus.main = TIME_SET_FOCUS) :
	(SystemSetClass.cur_focus.main == VOLUME_SET_FOCUS) ? (SystemSetClass.cur_focus.main = LANGUAGE_SET_FOCUS) :
	(SystemSetClass.cur_focus.main == SYSTEM_SET_RESET_FOCUS)    ? (SystemSetClass.cur_focus.main = VOLUME_SET_FOCUS) ;

}

static void goto_next_settings_focus(void){

	(SystemSetClass.cur_focus.main == TIME_SET_FOCUS)	? (SystemSetClass.cur_focus.main = LANGUAGE_SET_FOCUS) :
	(SystemSetClass.cur_focus.main == LANGUAGE_SET_FOCUS)	? (SystemSetClass.cur_focus.main = VOLUME_SET_FOCUS) :
	(SystemSetClass.cur_focus.main == VOLUME_SET_FOCUS) ? (SystemSetClass.cur_focus.main = SYSTEM_SET_RESET_FOCUS) :
	(SystemSetClass.cur_focus.main == SYSTEM_SET_RESET_FOCUS)	? (SystemSetClass.cur_focus.main = TIME_SET_FOCUS) ;

}
```

## 输入框现在能输入四位数，我们仅需3位数
```C
// FIXME 修改这里 或者 修改 判断条件 '<='
// FIXME  cursor.max_index = 3;  
```

EG 修改 判断条件 `<=`:  
```C
if(index <= DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]->cursor.max_index)			// befor

if(index < DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]->cursor.max_index)			// later
```

## STR_DialClass
```C
typedef struct{
	STR_WidgetShow widget_show;
	STR_WidgetDialogBox **dialog_box;
	TouchKeyRegisterCallback key_register;
	unsigned char home_id[4];
}STR_DialClass;
```

## home_id_set_key_star_up
修改前  
```C
    os_layout_goto(&layout_settings);
```
修改后  
```C
static void home_id_set_key_star_up(void){

	if(HomeIdSetClass.dialog_box[HomeIdSetClass.cur_focus]->cursor.index == 0){
		os_layout_goto(&layout_settings);
	}else{
		home_id_set_sub_number();
	}

}
```

