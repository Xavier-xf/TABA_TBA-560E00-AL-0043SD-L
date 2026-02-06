#include ".\include\msg_event.h"
#include ".\include\interrupt.h"
#include ".\include\kevin_function.h"
#include <stdio.h>
#include "stdlib.h"

#define EVENT_QUEUE_MAX  64

static Event EventQueue[EVENT_QUEUE_MAX];
static int EventQueueIndex = 0;
static int EventQueueCont = 0;
static BOOL home_monitor_sent[HOME_INDEX_TOTAL] = {false};



/*************************************************************************
@Function:    从队列中获取系统事件
@Input:       (Event)    修改获取到的事件
@Output:      (char)
              return  1：获取系统事件成功
		      return -1：获取系统事件失败，队列为空
@Description:
@Author:      kevin
@Date:        2021-08-24
*************************************************************************/
static char msg_event_get(Event* ev)
{
    int index;
    if(EventQueueCont == 0) {
        ev->type = EVENT_TYPE_NONE;
        ev->arg1 = 0;
        ev->arg2 = 0;
        return -1;
    }
    index = EventQueueIndex;
    EventQueueIndex = (EventQueueIndex + 1) % EVENT_QUEUE_MAX;
    EventQueueCont--;
    ev->type = EventQueue[index].type;
    ev->arg1 = EventQueue[index].arg1;
    ev->arg2 = EventQueue[index].arg2;
    return 1;
}


/************************************************************************
@Function:    将系统事件放入队列
@Input:       (Event)    要放入的事件
@Output:      (char)
		      return  0：添加系统事件成功
		      return -1：添加系统事件失败，队列溢出
@Description:
@Author:      kevin
@Date:        2021-08-24
************************************************************************/
char msg_event_post(Event* ev)
{
    int index;

    if (EventQueueCont >= EVENT_QUEUE_MAX) {
        return -1;
    }
    index = (EventQueueIndex + EventQueueCont) % EVENT_QUEUE_MAX;
    EventQueue[index].type = ev->type;
    EventQueue[index].arg1 = ev->arg1;
    EventQueue[index].arg2 = ev->arg2;
    EventQueueCont++;
    return 0;
}

// 判断房号是否被设置过（非默认值）
static uint8_t is_home_id_set(uint8_t* home_id)
{
    // 检查是否为默认值 {0, 0xFF, 0, 0xFF}
    if(home_id[0] == 0 && home_id[1] == 0xFF && home_id[2] == 0 && home_id[3] == 0xFF) {
        return 0; // 默认值，未设置
    }
    return 1; // 已设置
}
/*************************************************************************
@Function:    处理接收的对讲拨号数据
@Input:       (uint8_t) data1 - 拨号数据的千位部分
              (uint8_t) data2 - 拨号数据的百位部分
              (uint8_t) data3 - 拨号数据的十位部分
              (uint8_t) data4 - 拨号数据的个位部分
@Output:      (void)
@Description: 接收外部拨号的四位数据（分拆为四个字节），根据拨码开关状态进行格式转换，
              判断该拨号是否属于当前设备负责的家庭范围，若是则发送确认指令并发布呼叫事件
              兼容两种模式：1.硬件编码模式（房号未设置时） 2.软件编码模式（房号已设置时）
@Author:      Van
@Date:        2025-07-28
*************************************************************************/
static void intercom_receive_dial(uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4)
{
    Event event = {0};
    u16 dial_number;
    u8 i;
		// 计算该房间的房号
    u16 room_number ;
	u8 calculated_base;
	u8 home_index ;
            // 计算拨号号码
    dial_number = data1 * 1000 + data2 * 100 + data3 * 10 + data4;
        for(i = 0; i < 4; i++) {
        uint8_t* home_id;
        switch(i) {
            case 0: home_id = Intercom.home1_id; break;
            case 1: home_id = Intercom.home2_id; break;
            case 2: home_id = Intercom.home3_id; break;
            case 3: home_id = Intercom.home4_id; break;
        }
        

        room_number = home_id[0] * 1000 + home_id[1] * 100 + 
                home_id[2] * 10 + home_id[3];
        
        // 如果设置了软件房号，检查软件房号
        if(is_home_id_set(home_id)) {
            if(dial_number == room_number) {
                // 匹配成功
                send_can_cmd_encode(CMD_ACK, 0x00, CMD_NULL, CMD_NULL, CMD_NULL);
                event.type = EVENT_TYPE_CALL;
                event.arg1 = i + 1;
                msg_event_post(&event);
                return;
            }
        }
    // 如果未设置软件房号，检查硬件房号
    else {
            // 硬件计算逻辑
            u16 raw_input = data1 * 1000 + data2 * 100 + data3 * 10 + data4;
            u16 converted_num = 0;
            
            if (DIP_SW8) {
                converted_num = raw_input;
            } else {
                uint8_t floor = data1 * 10 + data2;
                uint8_t room = data3 * 10 + data4;
                converted_num = (floor - 1) * 10 + room;
            }
            
            calculated_base = (converted_num - 1) / 4;
            home_index = (converted_num - 1) % 4 + 1;
            
            if (calculated_base == base_address && home_index == (i + 1)) {
                // 匹配成功
                send_can_cmd_encode(CMD_ACK, 0x00, CMD_NULL, CMD_NULL, CMD_NULL);
                event.type = EVENT_TYPE_CALL;
                event.arg1 = i + 1;
                msg_event_post(&event);
                return;
            }
        }
    }
}

static void intercom_receive_set_home_id(uint8_t home_id, uint8_t data_high, uint8_t data_low,uint8_t brancher_id)
{
    if(brancher_id==base_address){
        uint16_t  home_number;

        home_number = (uint16_t)((data_high << 8) | data_low);
    
        if(home_id == HOME_ID1) {
            UserData.home1_id[0] = home_number/1000;
            UserData.home1_id[1] = home_number%1000/100;
            UserData.home1_id[2] = home_number%100/10;
            UserData.home1_id[3] = home_number%10;
            save_user_data(HOME1_ID_ADDR, UserData.home1_id);
        } else if(home_id == HOME_ID2) {
            UserData.home2_id[0] = home_number/1000;
            UserData.home2_id[1] = home_number%1000/100;
            UserData.home2_id[2] = home_number%100/10;
            UserData.home2_id[3] = home_number%10;
            save_user_data(HOME2_ID_ADDR, UserData.home2_id);
        } else if(home_id == HOME_ID3) {
            UserData.home3_id[0] = home_number/1000;
            UserData.home3_id[1] = home_number%1000/100;
            UserData.home3_id[2] = home_number%100/10;
            UserData.home3_id[3] = home_number%10;
            save_user_data(HOME3_ID_ADDR, UserData.home3_id);
        } else if(home_id == HOME_ID4) {
            UserData.home4_id[0] = home_number/1000;
            UserData.home4_id[1] = home_number%1000/100;
            UserData.home4_id[2] = home_number%100/10;
            UserData.home4_id[3] = home_number%10;
            save_user_data(HOME4_ID_ADDR, UserData.home4_id);
        }
        send_can_cmd_encode(CMD_ACK, 0x00, CMD_NULL, CMD_NULL, CMD_NULL);
    }


}

static void intercom_unlock_check(void)
{
    if(Intercom.unlock_status == UNLOCK_START_WAIT_ACK) {
        Intercom.unlock_status = UNLOCK_BUSY;
    }
    if(Intercom.unlock_status == UNLOCK_END_WAIT_ACK) {
        Intercom.unlock_status = UNLOCK_IDLE;
    }
}

static void intercom_monitor_check(void)
{
    if(Intercom.monitor_status == MONITOR_START_WAIT_ACK) {
        Intercom.monitor_status = MONITOR_BUSY;
    }
    if(Intercom.monitor_status == MONITOR_END_WAIT_ACK) {
        Intercom.monitor_status = MONITOR_IDLE;
    }
}

static void intercom_receive_ack(uint8_t data1)
{
    if(data1 == 0x00) {
        return ;
    }
    intercom_unlock_check();
    intercom_monitor_check();
}


static void intercom_send_home_id(uint8_t data1,uint8_t data2)
{
    if(data2==base_address){
        uint8_t home_id_high, home_id_low;
        uint8_t home_index = 0;
        u16 converted_num;
            // 获取对应的房号数据
        uint8_t* home_id;
    
        // 确定home索引（1-4，对应HOME_ID1到HOME_ID4）
        if (data1 == HOME_ID1) {
            home_index = 1;
        } else if (data1 == HOME_ID2) {
            home_index = 2;
        } else if (data1 == HOME_ID3) {
            home_index = 3;
        } else if (data1 == HOME_ID4) {
            home_index = 4;
        } else {
            return; // 无效ID，不发送
        }
        
    
        switch(home_index) {
            case 1: home_id = Intercom.home1_id; break;
            case 2: home_id = Intercom.home2_id; break;
            case 3: home_id = Intercom.home3_id; break;
            case 4: home_id = Intercom.home4_id; break;
            default: return;
        }
        
        // 如果房号被设置过，使用软件编码
        if(is_home_id_set(home_id)) {
            u16 home_number = home_id[0] * 1000 + home_id[1] * 100 + 
                              home_id[2] * 10 + home_id[3];
            
    
                // 发送原始数值
                home_id_high = home_number / 100;       // 取高两位
                home_id_low = home_number % 100;        // 取低两位
    
        }
        // 如果房号未设置，使用硬件编码
        else {
            // 计算converted_num（与接收逻辑反向：converted_num = base_address×4 + home_index）
            converted_num = base_address * 4 + home_index;
            
            // 根据DIP_SW8切换发送格式
            if (DIP_SW8) {
                // 模式1：!DIP_SW8，发送原始数值（拆分高位和低位）
                home_id_high = converted_num / 100;       // 取高两位
                home_id_low = converted_num % 100;        // 取低两位
            } else {
                // 模式2：DIP_SW8=1，发送"楼层×10 + 户号"格式
                uint8_t floor = (converted_num - 1) / 10 + 1;  // 楼层（1-25）
                uint8_t room = (converted_num - 1) % 10 + 1;   // 户号（1-10）
                
                home_id_high = floor;  // 楼层作为高位
                home_id_low = room;    // 户号作为低位
            }
        }
        
        // 发送最终结果
        send_can_cmd_encode(CMD_SEND_HOME_ID, home_index, home_id_high, home_id_low, CMD_NULL);
    }

}


static uint8_t get_rand_data(uint8_t range)
{
    uint8_t rand_data;

    srand(cpu_count);
    rand_data = rand() % range + 1;
    return rand_data;
}

static uint8_t send_status_count = 0;
static void send_monitor_status(void)
{

    send_can_cmd_encode(CMD_SEND_MONITOR_STATUS, 0x01, CMD_NULL, CMD_NULL, CMD_NULL);
    if(send_status_count) {
        uint8_t rand_data;
        send_status_count--;
        rand_data = get_rand_data(50);
        set_timer(TIMER3, TID_10mSEC*rand_data, send_monitor_status);
    }
}

static void intercom_send_monitor_status(void)
{

    if(Intercom.monitor_status == MONITOR_BUSY) {
        uint8_t rand_data;
        send_status_count = 1;
        rand_data = get_rand_data(50);
        set_timer(TIMER3, TID_10mSEC*rand_data, send_monitor_status);
    }
}

static void sys_intercome_check(void)
{
    uint8_t cmd, data1, data2, data3, data4;

    if(receive_can_cmd_decode(&cmd, &data1, &data2, &data3, &data4) == 1) {
        switch(cmd) {
        case CMD_DIAL:
            intercom_receive_dial(data1, data2, data3, data4);
            break;
        case CMD_SET_HOME_ID:
            intercom_receive_set_home_id(data1, data2, data3,data4);
            break;
        case CMD_READ_HOME_ID:
            intercom_send_home_id(data1,data2);
            break;
        case CMD_READ_MONITOR_STATUS:
            intercom_send_monitor_status();
            break;
        case CMD_ACK:
            intercom_receive_ack(data1);
            break;
        }
    }
}
static LEVEL pre_status = HIGH_LEVEL;
static uint8_t debounce_timer = 0;      // 防抖定时器ID
static uint8_t debounce_flag = 0;       // 防抖处理标志
static void unlock_debounce_timeout(void);
/***********************************************************************
@Function:    开锁事件检测（带100ms防抖）
@Input:       (void)
@Output:      (void)
@Description: 检测UNLOCK_GPIO是否被拉低，低电平有开锁，高电平无开锁
              增加100ms防抖处理，防止按键抖动误触发
              只对低电平（开锁开始）进行防抖，高电平（开锁结束）立即响应
@Author:      kevin
@Date:        2021-08-24
@Modified:    2025-12-7 增加防抖功能
************************************************************************/
static void sys_unlock_check(void)
{
    Event event = {0};
    
    // 读取当前GPIO状态
    LEVEL current_gpio = (UNLOCK_GPIO == 0) ? LOW_LEVEL : HIGH_LEVEL;
    
    // 处理高电平 -> 开锁结束（立即响应，不需要防抖）
    if((current_gpio == HIGH_LEVEL) && (Intercom.unlock_status == UNLOCK_BUSY)) {
        // 取消可能存在的防抖定时器
        if(debounce_flag) {
            kill_timer(TIMER0);
            debounce_flag = 0;
        }
        
        pre_status = HIGH_LEVEL;
        event.type = EVENT_TYPE_UNLOCK_END;
        msg_event_post(&event);
        return;
    }
    
    // 处理低电平 -> 开锁开始（需要防抖）
    if((current_gpio == LOW_LEVEL) && (pre_status == HIGH_LEVEL) && 
       (Intercom.unlock_status == UNLOCK_IDLE)) {
        if(!debounce_flag) {
            // 第一次检测到低电平，启动防抖定时器
            debounce_flag = 1;
            set_timer(TIMER0, TID_100mSEC, unlock_debounce_timeout);
        }
    } else if(current_gpio == HIGH_LEVEL) {
        // 当前是高电平，取消防抖
        if(debounce_flag) {
            kill_timer(TIMER0);
            debounce_flag = 0;
        }
    }
}

/***********************************************************************
@Function:    开锁防抖超时处理
@Input:       (void)
@Output:      (void)
@Description: 防抖定时器超时后的处理，确认开锁事件
@Author:      kevin
@Date:        2023-11-15
************************************************************************/
static void unlock_debounce_timeout(void)
{
    Event event = {0};
    
    // 防抖定时器超时，确认开锁事件
    if(debounce_flag) {
        // 检查当前GPIO状态是否仍为低电平
        if((UNLOCK_GPIO == 0) && (pre_status == HIGH_LEVEL) && 
           (Intercom.unlock_status == UNLOCK_IDLE)) {
            // 确认开锁开始
            pre_status = LOW_LEVEL;
            event.type = EVENT_TYPE_UNLOCK_START;
            msg_event_post(&event);
        }
        
        // 重置防抖标志
        debounce_flag = 0;
    }
    
    // 清除定时器
    kill_timer(TIMER0);
}



static void home1_monitor_check(void)
{
    Event event = {0};

    if(!POWER_HOME1_GPIO && home_monitor_sent[HOME_INDEX_1]==false) {
        event.type = EVENT_TYPE_MONITOR_START;
        event.arg1 = HOME_ID1;
        msg_event_post(&event);
    }

//    if(!POWER_HOME1_GPIO && !AUDIO_VIDEO_CH1_GPIO) {
//        event.type = EVENT_TYPE_OPEN_AUDIO_VIDEO;
//        event.arg1 = HOME_ID1;
//        msg_event_post(&event);
//    } else if(POWER_HOME1_GPIO && AUDIO_VIDEO_CH1_GPIO) {
//        event.type = EVENT_TYPE_CLOSE_AUDIO_VIDEO;
//        event.arg1 = HOME_ID1;
//        msg_event_post(&event);
//    }
}

static void home2_monitor_check(void)
{
    Event event = {0};

    if(!POWER_HOME2_GPIO && home_monitor_sent[HOME_INDEX_2]==false) {
        event.type = EVENT_TYPE_MONITOR_START;
        event.arg1 = HOME_ID2;
        msg_event_post(&event);
    }

//    if(!POWER_HOME2_GPIO && !AUDIO_VIDEO_CH2_GPIO) {
//        event.type = EVENT_TYPE_OPEN_AUDIO_VIDEO;
//        event.arg1 = HOME_ID2;
//        msg_event_post(&event);
//    } else if(POWER_HOME2_GPIO && AUDIO_VIDEO_CH2_GPIO) {
//        event.type = EVENT_TYPE_CLOSE_AUDIO_VIDEO;
//        event.arg1 = HOME_ID2;
//        msg_event_post(&event);
//    }
}

static void home3_monitor_check(void)
{
    Event event = {0};

    if(!POWER_HOME3_GPIO && home_monitor_sent[HOME_INDEX_3]==false) {
        event.type = EVENT_TYPE_MONITOR_START;
        event.arg1 = HOME_ID3;
        msg_event_post(&event);
    }

//    if(!POWER_HOME3_GPIO && !AUDIO_VIDEO_CH3_GPIO) {
//        event.type = EVENT_TYPE_OPEN_AUDIO_VIDEO;
//        event.arg1 = HOME_ID3;
//        msg_event_post(&event);
//    } else if(POWER_HOME3_GPIO && AUDIO_VIDEO_CH3_GPIO) {
//        event.type = EVENT_TYPE_CLOSE_AUDIO_VIDEO;
//        event.arg1 = HOME_ID3;
//        msg_event_post(&event);
//    }
}

static void home4_monitor_check(void)
{
    Event event = {0};

    if(!POWER_HOME4_GPIO && home_monitor_sent[HOME_INDEX_4]==false) {
        event.type = EVENT_TYPE_MONITOR_START;
        event.arg1 = HOME_ID4;
        msg_event_post(&event);
    }

//    if(!POWER_HOME4_GPIO && !AUDIO_VIDEO_CH4_GPIO) {
//        event.type = EVENT_TYPE_OPEN_AUDIO_VIDEO;
//        event.arg1 = HOME_ID4;
//        msg_event_post(&event);
//    } else if(POWER_HOME4_GPIO && AUDIO_VIDEO_CH4_GPIO) {
//        event.type = EVENT_TYPE_CLOSE_AUDIO_VIDEO;
//        event.arg1 = HOME_ID4;
//        msg_event_post(&event);
//    }
}

static void home_monitor_end_check(void)
{
    Event event = {0};
    int i;
    if(Intercom.monitor_status != MONITOR_BUSY)
        return ;
    if(POWER_HOME1_GPIO && POWER_HOME2_GPIO && POWER_HOME3_GPIO && POWER_HOME4_GPIO) {
        for(i = HOME_INDEX_1; i < HOME_INDEX_TOTAL; i++) {
            home_monitor_sent[i] = false;
        }
        event.type = EVENT_TYPE_MONITOR_END;
        msg_event_post(&event);
        Intercom.monitor_status = MONITOR_END_WAIT_ACK;
    }
    POWER_HOME1_GPIO ? (home_monitor_sent[HOME_INDEX_1] = false) : 0,
    POWER_HOME2_GPIO ? (home_monitor_sent[HOME_INDEX_2] = false) : 0,
    POWER_HOME3_GPIO ? (home_monitor_sent[HOME_INDEX_3] = false) : 0,
    POWER_HOME4_GPIO ? (home_monitor_sent[HOME_INDEX_4] = false) : 0;
}


/***********************************************************************
@Function:    监控事件检测
@Input:       (void)
@Output:      (void)
@Description: 每个楼层分支器可以连接4个住宅home1-home4
			  当有住宅监控时POWER_HOME_GPIO(home1-4)被拉低
@Author:      kevin
@Date:        2021-08-24
************************************************************************/
static void sys_monitor_check(void)
{
    home1_monitor_check();
    home2_monitor_check();
    home3_monitor_check();
    home4_monitor_check();
    home_monitor_end_check();
}


/***********************************************************************
@Function:    楼层号事件检测
@Input:       (void)
@Output:      (void)
@Description: 获取楼层分支器房号
@Author:      kevin
@Date:        2021-08-24
***********************************************************************/
static void sys_get_hoom_id(void)
{
    uint8_t i;
    for(i = 0; i < 4; i++) {
        Intercom.home1_id[i] = UserData.home1_id[i];
        Intercom.home2_id[i] = UserData.home2_id[i];
        Intercom.home3_id[i] = UserData.home3_id[i];
        Intercom.home4_id[i] = UserData.home4_id[i];
    }
}
/***********************************************************************
@Function:    更新拨码目标值
@Input:       (void)
@Output:      (void)
@Description:
@Author:      xiang
@Date:        2025-07-25
***********************************************************************/
static void update_branch_target(void)
{
    uint8_t new_base_addr = 0;
    //  读取最新的拨码开关状态
    read_dip_switch();
    new_base_addr = dip_switch_value;

    // 如果分支器地址 发生了改变
    if(new_base_addr != prev_base_address)
    {
        // 恢复4个房间号为默认值 {0, 0xFF, 0, 0xFF}
        UserData.home1_id[0] = 0;    UserData.home1_id[1] = 0xFF;    UserData.home1_id[2] = 0;    UserData.home1_id[3] = 0xFF;
        UserData.home2_id[0] = 0;    UserData.home2_id[1] = 0xFF;    UserData.home2_id[2] = 0;    UserData.home2_id[3] = 0xFF;
        UserData.home3_id[0] = 0;    UserData.home3_id[1] = 0xFF;    UserData.home3_id[2] = 0;    UserData.home3_id[3] = 0xFF;
        UserData.home4_id[0] = 0;    UserData.home4_id[1] = 0xFF;    UserData.home4_id[2] = 0;    UserData.home4_id[3] = 0xFF;

        // 将默认值持久化保存到闪存(IAP)，保证重启后依然生效
        save_user_data(HOME1_ID_ADDR, UserData.home1_id);
        save_user_data(HOME2_ID_ADDR, UserData.home2_id);
        save_user_data(HOME3_ID_ADDR, UserData.home3_id);
        save_user_data(HOME4_ID_ADDR, UserData.home4_id);

        //  更新历史值，避免重复触发恢复逻辑
        prev_base_address = new_base_addr;
    }

    //  更新当前分支器地址
    base_address = new_base_addr;
}

/***********************************************************************
@Function:    系统事件检测
@Input:       (void)
@Output:      (void)
@Description:
@Author:      kevin
@Date:        2021-08-24
***********************************************************************/
static void sys_event_check(void)
{
    sys_intercome_check();
    sys_unlock_check();
    sys_monitor_check();
    sys_timer_check();
    sys_get_hoom_id();
    update_branch_target();   // xiang：更新拨码目标值
}




static void sys_call_process(HOME_ID id)
{
    //open_audio_video_ch(id);
    call_home(id);
}

static BOOL check_home_monitor_sent_reset(HOME_ID id)
{
    if(home_monitor_sent[id] == true) {
        return false;
    }
    return true;

}
static void intercom_wait_ack(void)
{

    if (Intercom.send_count > 0) {
        Intercom.send_count--;
        if(Intercom.monitor_status == MONITOR_START_WAIT_ACK) {
            send_can_cmd_encode(CMD_MONITOR_START, CMD_NULL, CMD_NULL, CMD_NULL, CMD_NULL);
        } else if(Intercom.monitor_status == MONITOR_END_WAIT_ACK) {
            send_can_cmd_encode(CMD_MONITOR_END, CMD_NULL, CMD_NULL, CMD_NULL, CMD_NULL);
        }
        if(Intercom.unlock_status == UNLOCK_START_WAIT_ACK) {
            send_can_cmd_encode(CMD_UNLOCK_START, CMD_NULL, CMD_NULL, CMD_NULL, CMD_NULL);
        } else if(Intercom.unlock_status == UNLOCK_END_WAIT_ACK) {
            send_can_cmd_encode(CMD_UNLOCK_END, CMD_NULL, CMD_NULL, CMD_NULL, CMD_NULL);
        }
        set_timer(TIMER2, TID_800mSEC, intercom_wait_ack);
    } else {
        if((Intercom.monitor_status == MONITOR_START_WAIT_ACK) || (Intercom.monitor_status == MONITOR_END_WAIT_ACK)) {
            Intercom.monitor_status = MONITOR_IDLE;
        }
        if((Intercom.unlock_status == UNLOCK_START_WAIT_ACK) || (Intercom.unlock_status == UNLOCK_END_WAIT_ACK)) {
            Intercom.unlock_status = UNLOCK_IDLE;
        }
    }

}

/***********************************************************************
@Function:    开锁事件处理
@Input:       (void)
@Output:      (void)
@Description: 发送开锁指令到CAN总线
@Author:      kevin
@Date:        2021-08-26
***********************************************************************/
static void sys_unlock_start_process(void)
{

    Intercom.unlock_status = UNLOCK_START_WAIT_ACK;
    Intercom.send_count = 3;
    send_can_cmd_encode(CMD_UNLOCK_START, CMD_NULL, CMD_NULL, CMD_NULL, CMD_NULL);
    set_timer(TIMER2, TID_800mSEC, intercom_wait_ack);
}


/***********************************************************************
@Function:    关锁事件处理
@Input:       (void)
@Output:      (void)
@Description: 发送关锁指令到CAN总线
@Author:      kevin
@Date:        2021-08-26
***********************************************************************/
static void sys_unlock_end_process(void)
{

    Intercom.unlock_status = UNLOCK_END_WAIT_ACK;
    Intercom.send_count = 3;
    send_can_cmd_encode(CMD_UNLOCK_END, CMD_NULL, CMD_NULL, CMD_NULL, CMD_NULL);
    set_timer(TIMER2, TID_800mSEC, intercom_wait_ack);
}


/***********************************************************************
@Function:    监控事件处理
@Input:       (HOME_ID) 房间监控事件
@Output:      (void)
@Description: 发送开始监控指令到CAN总线
@Author:      kevin
@Date:        2021-08-26
***********************************************************************/
static void sys_monitor_start_process(HOME_ID id)
{
    if(Intercom.monitor_status == MONITOR_IDLE||check_home_monitor_sent_reset(id)) {
        home_monitor_sent[id] = true;
        Intercom.monitor_status = MONITOR_START_WAIT_ACK;
        Intercom.send_count = 3;
        send_can_cmd_encode(CMD_MONITOR_START, CMD_NULL, CMD_NULL, CMD_NULL, CMD_NULL);
        set_timer(TIMER2, TID_800mSEC, intercom_wait_ack);
    }
    //open_audio_video_ch(id);
}


/***********************************************************************
@Function:    监控事件处理
@Input:       (void)
@Output:      (void)
@Description: 发送结束监控指令到CAN总线
@Author:      kevin
@Date:        2021-08-26
***********************************************************************/
static void sys_monitor_end_process(void)
{

    Intercom.send_count = 3;
    //close_audio_video_ch(TOTAL_HOME_ID);
    send_can_cmd_encode(CMD_MONITOR_END, CMD_NULL, CMD_NULL, CMD_NULL, CMD_NULL);
    set_timer(TIMER2, TID_800mSEC, intercom_wait_ack);
}

static void sys_open_audio_video_process(HOME_ID id)
{

    //open_audio_video_ch(id);
}

static void sys_close_audio_video_process(HOME_ID id)
{

    //close_audio_video_ch(id);
}



/***************************************************************
@Function:    系统事件处理
@Input:       (void)
@Output:      (void)
@Description: 当检测到有系统事件发生时做出相对应的事件处理
@Author:      kevin
@Date:        2022-02-17
****************************************************************/
static void msg_event_process(Event* ev)
{
    switch(ev->type) {
    case EVENT_TYPE_TIMER:
        sys_timer_process(ev->arg1);
        break;
    case EVENT_TYPE_CALL:
        sys_call_process(ev->arg1);
        break;
    case EVENT_TYPE_UNLOCK_START:
        sys_unlock_start_process();
        break;
    case EVENT_TYPE_UNLOCK_END:
        sys_unlock_end_process();
        break;
    case EVENT_TYPE_MONITOR_START:
        sys_monitor_start_process(ev->arg1);
        break;
    case EVENT_TYPE_MONITOR_END:
        sys_monitor_end_process();
        break;
    case EVENT_TYPE_OPEN_AUDIO_VIDEO:
        sys_open_audio_video_process(ev->arg1);
        break;
    case EVENT_TYPE_CLOSE_AUDIO_VIDEO:
        sys_close_audio_video_process(ev->arg1);
        break;
    }
}


void msg_run(void)
{
    Event ev;
    while(1) {
        WDTCR |= (0x01<<4);     //清除看门狗
        sys_event_check();
        if(msg_event_get(&ev)) {
            msg_event_process(&ev);
        }
    }
}
