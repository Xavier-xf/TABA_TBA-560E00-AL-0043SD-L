#include <unistd.h>
#include "leo_audio_play.h"
#include "audio_play_api.h"
#include "rom.h"
#include "gpio_api.h"
#include "intercom.h"
#include "layout_base.h"
#include "gpio_control.h"

void leo_audio_play_init(void)
{
    audio_info audio_group_info[3] =
        {

            {ROM_R_RING_KEY_PCM, ROM_R_RING_KEY_PCM_SIZE, AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_16000, AK_AUDIO_TYPE_PCM},
            {ROM_R_RING_WARN_PCM, ROM_R_RING_WARN_PCM_SIZE, AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_8000, AK_AUDIO_TYPE_PCM},
            {ROM_R_RING_SLOT_CARD_MP3, ROM_R_RING_SLOT_CARD_MP3_SIZE, AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_8000, AK_AUDIO_TYPE_MP3},

        };
    audio_play_init(audio_group_info, 3);
}

static void audio_play_start_func(void)
{

    amplifier_gpio_control(true); // 每次都控制一次GPIO 的话 在播放音频开始会有杂音

    // usleep(1000);
}

static void audio_play_finish_func(void)
{
    // usleep(2 * 1000);
    // amplifier_gpio_control(false);
}

/***
 * 描述: 播放触摸声
 * 日期: 2022-08-15 16:50
 * 作者: 
 ***/
void touch_sound_play(void)
{
    /* befor 45 */
    audio_play(RING_TOUCH_TONE, 45, audio_play_start_func, audio_play_finish_func);
}
void touch_sound_num_play(enum ring_index index)
{
	
	audio_play(index, 65, audio_play_start_func, audio_play_finish_func);
}
/***
 * 描述: 播放警告声
 * 日期: 2022-08-15 16:49
 * 作者: 
 ***/
void warn_sound_play(void)
{   LOG_WHITE("sound play\n");
    audio_play(RING_WARN_TONE, 60, audio_play_start_func, audio_play_finish_func);
}



static void swiping_card_sound_play_start(void)
{
    amplifier_gpio_control(true); 
}

static void swiping_card_sound_play_finish(void)
{
    amplifier_gpio_control(false);
}

/***
 * 描述: 播放刷卡的声音
 * 日期: 2022-08-15 16:49
 * 作者: 
 ***/
void swiping_card_sound_play(void)
{
    audio_play(RING_SLOT_CARD_TONE, 60, swiping_card_sound_play_start, swiping_card_sound_play_finish);
}
