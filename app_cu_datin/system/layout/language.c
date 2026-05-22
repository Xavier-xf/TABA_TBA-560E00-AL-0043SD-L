#include "language.h"
#include "ui_api.h"

const char *multi_lingual[STR_TOTAL][language_total] =
	{
		// info
		{"Resetting password", "بازنشانی رمز عبور"},
		{"Restoring factory settings", "بازیابی تنظیمات کارخانه"},
		{"complete", "کامل"},
		// layout dial font
		{"PRESS", "فشار دهید"},
		{"ENTER UNIT NUMBER AND", "شماره واحد را وارد کنید و"},
		{"TO CALL GUARD", "برای تماس با نگهبان"},
		{"Delete", "تصحیح"},
		{"TO CLEAR", "برای پاک کردن"},
		{"UNIT:", "واحد:"},
		// layout calling font
		{"Calling", "در حال تماس"},
		{"Calling Guard ...", "در حال تماس با نگهبان ..."},
		{"No Answer", "بدون پاسخ"},
		// layout password font
		{"PASSWORD:", "رمز عبور"}, // 10
		{"Enter Old Password:", "رمز قدیم وارد شود"},
		{"Enter New Password:", "رمز جدید وارد شود"},
		{"Please Input the new password again", "رمز جدید دوباره وارد شود"},
		{"Old password Error", "رمز قدیم اشتباه است"},
		{"Success", "موفق"},
		{"Welcome", "خوش آمدی"},
		{"Password Error", "رمز اشتباه است"},
		// layout settings font
		{"System Setting", "تنظیمات سیستم"},
		{"Unlock Password", "تنظیمات رمز درب بازکن"},
		{"Setting Password", "تنظیمات رمز ورودی"},
		{"OutPut", "خروجی"},
		{"NFC", "کارت دسترسی"},
		// layout system set font
		{"Time and Date", "ساعت و تاریخ"},
		{"Language", "زبان"},
		{"Volume setting", "تنظیمات صدا"}, // 25
		{"English", "انگلیسی"},
		{"Reset Factory", "بازگشت به کارخانه"},
		// layout time set font
		{"Year", "سال"},
		{"Month", "ماه"},
		{"Day", "روز"}, // 30
		{"Hour", "ساعت"},
		{"Minute", "دقیقه"},
		// layout home id set font
		{"OUT:", "خروج"},
		{"UNIT:", "واحد"},
		{"Guard:", "نگهبان"},
		{"SAVE", "ذخیره"},
		{"ERROR", "خطا"},
		{"M1", "مانیتور 1"},
		{"M2", "مانیتور 2"},
		{"M3", "مانیتور 3"}, // 35
		{"M4", "مانیتور 4"},
		{"Success", "موفق"},
		{"Fail", "شکست"},
		{"Exists", "تکراری"},
		{"The number has been set", "تعداد تعیین شده است"}, // 40
		{"are you sure you want to set it?", "آیا مطمئن هستید که می خواهید آن را تنظیم کنید؟"},
		// layout card manage font
		{"Add new card", "اضافه کردن کارت"},
		{"delete card", "حذف کارت با شماره اتاق"},
		{"delete all the cards", "حذف همه کارتها"},
		{"Delete all", "آیا مطمئن هستید؟"}, // 45
		{"Yes", "بله"},
		{"No", "خیر"},
		{"Success", "موفق"},
		{"UNIT:", "واحد"},
		{"TAG:", "تگ"},
		{"ERASE:", "پاک کردن "},
		{"SAVE:", "ذخیره"},
		// layout card number font
		{"Please enter room number", "لطفا شماره اتاق را وارد کنید"},
		{"Success", "موفق"}, // 50
		{"Room number error", "شماره اتاق وجود ندارد"},
		// layout swiping number font
		{"Please put your card in the card section", "لطفا کارت را در قسمت کارتخوانی قرار دهید"},
		{"Add Success !", "کارت اضافه شد"},
		{"Deleting...", "در حال حذف"},
		// bottom logo font
		{"TABA", "تابا"},
		{"Electronics", "الکترونیک"},
};

void *font_str(STRING_ID str_id)
{

	return (void *)multi_lingual[str_id][language_get()];
}
