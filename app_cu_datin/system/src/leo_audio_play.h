#ifndef LEO_AUDIO_PLAY_H
#define LEO_AUDIO_PLAY_H



enum ring_index {

    RING_TOUCH_TONE,
	RING_WARN_TONE,
	RING_SLOT_CARD_TONE,
    RING_MAX
};



extern void leo_audio_play_init(void);
void touch_sound_num_play(enum ring_index index);
extern void touch_sound_play(void);
extern void warn_sound_play(void);
extern void swiping_card_sound_play(void);


#endif

