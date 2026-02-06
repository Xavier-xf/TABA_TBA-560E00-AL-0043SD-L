#ifndef LANGUAGE_H
#define LANGUAGE_H

typedef enum
{
	// -------------------------- 基础提示信息 --------------------------
	STR_RESETTING_PASSWORD, // Resetting password (重置密码)
	STR_FACTORY_SETTING,	// Restoring factory settings (恢复出厂设置)
	STR_COMPLETE,			// complete (完成)

	// -------------------------- 拨号界面文字 --------------------------
	STR_DIAL_PRESS,		 // PRESS (按下)
	STR_DIAL_INPUT_UNIT, // ENTER UNIT NUMBER AND (输入单元号并)
	STR_DIAL_CALL_GUARD, // TO CALL GUARD (呼叫保安)
	STR_DIAL_STAR_CORRE, // Delete (删除/修正)
	STR_DIAL_STAR_CLEAR, // TO CLEAR (清空)
	STR_DIAL_UNIT,		 // UNIT: (单元:)

	// -------------------------- 呼叫界面文字 --------------------------
	STR_CALLING_USER,	   // Calling (正在呼叫用户)
	STR_CALLING_GUARD,	   // Calling Guard ... (正在呼叫保安...)
	STR_CALLING_NO_ANSWER, // No Answer (无人接听)

	// -------------------------- 密码界面文字 --------------------------
	STR_PASSWORD_INPUT_PASSWORD,		   // PASSWORD: (密码:)
	STR_PASSWORD_INPUT_OLD_PASSWORD,	   // Please Input old password (请输入旧密码)
	STR_PASSWORD_INPUT_NEW_PASSWORD,	   // Please Input new password (请输入新密码)
	STR_PASSWORD_INPUT_NEW_PASSWORD_AGAIN, // Please Input the new password again (请再次输入新密码)
	STR_PASSWORD_OLD_PASSWORD_ERROR,	   // Old password Error (旧密码错误)
	STR_PASSWORD_SUCCESS,				   // Success (成功)
	STR_PASSWORD_WELCOME,				   // Welcome (欢迎)
	STR_PASSWORD_ERROR,					   // Password Error (密码错误)

	// -------------------------- 设置主界面文字 --------------------------
	STR_SET_SYSTEM_SET,			 // System Setting (系统设置)
	STR_SET_UNLOCK_SET,			 // Unlock Password (解锁密码)
	STR_SET_PASSWORD_SET,		 // Setting Password (设置密码)
	STR_SET_CONFIGURE_UNIT_ADDR, // OutPut (输出/单元地址配置)
	STR_SET_CARD_MANAGEMENT,	 // NFC (NFC/卡片管理)

	// -------------------------- 系统设置界面文字 --------------------------
	STR_SYSTEM_SET_TIME_SET,   // Time and Date (时间和日期)
	STR_SYSTEM_SET_LANG_SET,   // Language (语言)
	STR_SYSTEM_SET_VOLUME_SET, // Volume setting (音量设置)
	STR_SYSTEM_SET_LANG,	   // English (英语)
	STR_SYSTEM_SET_RESET,	   // Reset Factory (恢复出厂设置)

	// -------------------------- 时间设置界面文字 --------------------------
	STR_TIME_SET_YEAR,	 // Year (年)
	STR_TIME_SET_MONTH,	 // Month (月)
	STR_TIME_SET_DAY,	 // Day (日)
	STR_TIME_SET_HOUR,	 // Hour (时)
	STR_TIME_SET_MINUTE, // Minute (分)

	// -------------------------- 家庭ID/监视器设置界面文字 --------------------------
	STR_HOME_ID_OUT_ID,						  // OUT
	STR_HOME_ID_UNIT_SET,					  // UNIT
	STR_HOME_ID_GUARD,						  // Guard
	STR_HOME_ID_SAVE,						  // Save
	STR_HOME_ID_ERROR,						  // Error
	STR_HOME_ID_SET_M1,						  // M1 (监视器1)
	STR_HOME_ID_SET_M2,						  // M2 (监视器2)
	STR_HOME_ID_SET_M3,						  // M3 (监视器3)
	STR_HOME_ID_SET_M4,						  // M4 (监视器4)
	STR_HOME_ID_SET_SUCCESS,				  // Success (成功)
	STR_HOME_ID_SET_FAIL,					  // Fail (失败)
	STR_HOME_ID_SET_EXISTED,				  // Exists (已存在)
	STR_HOME_ID_SET_SELECT_DIALOG_BOX_TITLE1, // The number has been set (编号已设置)
	STR_HOME_ID_SET_SELECT_DIALOG_BOX_TITLE2, // are you sure you want to set it? (确定要设置吗？)

	// -------------------------- 卡片管理界面文字 --------------------------
	STR_CARD_MANAGE_ADD_CARD,		 // Add new card (添加新卡片)
	STR_CARD_MANAGE_DELETE_CARD,	 // delete card (删除指定房间号卡片)
	STR_CARD_MANAGE_DELETE_ALL_CARD, // delete all the cards (删除所有卡片)
	STR_CARD_MANAGE_ARE_YOU_SURE,	 // Delete all (确认删除所有？)
	STR_CARD_MANAGE_YES,			 // Yes (是)
	STR_CARD_MANAGE_NO,				 // No (否)
	STR_CARD_MANAGE_SUCCESS,		 // Success (成功)
	STR_CARD_MANAGE_UNIT,
	STR_CARD_MANAGE_TAG,
	STR_CARD_MANAGE_ERASE,
	STR_CARD_MANAGE_SAVE,

	// -------------------------- 卡片编号输入界面文字 --------------------------
	STR_CARD_NUMBER_INPUT_UNIT_NUMBER, // Please enter room number (请输入房间号)
	STR_CARD_NUMBER_SUCCESS,		   // Success (成功)
	STR_CARD_NUMBER_UNIT_NUMBER_ERROR, // Room number error (房间号错误/不存在)

	// -------------------------- 刷卡操作界面文字 --------------------------
	STR_SWIPING_CARD_PUT_CARD,	 // Please put your card in the card section (请将卡片放置到刷卡区)
	STR_SWIPING_CARD_SUCCESS,	 // Add Success ! (添加成功！)
	STR_CARD_MANAGE_WAIT_DELETE, // Deleting... (正在删除...)

	STR_TOTAL // 字符串总数（用于数组边界）
} STRING_ID;

extern void *font_str(STRING_ID str_id);

#endif
