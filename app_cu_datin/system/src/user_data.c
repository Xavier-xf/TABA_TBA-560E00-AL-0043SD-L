#include "user_data.h"
#include "iniparser.h"
#include "layout_base.h"
#include "user_data.h"
#include "string.h"

#define USER_CONF_FILE_PATH "/app/data/data.ini"       // 用户配置文件路径（INI格式）
#define FACTORY_CONF_FILE_PATH "/app/data/factory.ini" // 出厂默认配置文件路径
#define DATA_PATH "/app/data"                          // 数据存储根目录
#define USER_DATA_PATH DATA_PATH "/user_data"          // 用户核心数据文件路径（存储单元号等）
#define CARD_ID_DATA_PATH DATA_PATH "/card_data"       // 卡片ID数据文件路径

static dictionary *data_ptr = NULL; // iniparser字典指针，用于解析INI配置文件

static int default_int_data[CONFIGURE_MAX] = {0, 2, 0, 0, 249}; // 默认整型配置数据（索引对应sys_conf枚举）
static int int_data[CONFIGURE_MAX] = {0, 2, 0, 0, 249};         // 当前生效的整型配置数据（如单元号、参数等）

static char *password_string_data[2][32] = {{0}, {0}};             // 字符串配置数据（主要存储解锁密码等）
static char card_id_string_data[USER_CARD_TOTAL][32] = {{0}, {0}}; // 卡片ID数据数组
static char default_string_data[2][32] = {
    // 默认字符串配置（初始密码）
    {"1 2 3 4 5 6"},
    {"1 2 3 4 5 6"},
};

STR_UserData UserData = {
    // 用户数据结构体实例（存储单元号等核心用户数据）
    {-1},
    {-1},
};

/*********************************************************************************************************
 * 函 数 名 : user_data_init
 * 功能说明 : 初始化用户核心数据（从文件加载UserData结构体）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 从USER_DATA_PATH读取数据，读取失败打印错误日志，不影响后续流程
 *********************************************************************************************************/
void user_data_init(void)
{
    FILE *fp = fopen(USER_DATA_PATH, "rb");
    if (fp == NULL)
    {
        LOG_RED("init open %s fail\n\r", USER_DATA_PATH);
        return;
    }
    if (!fread(&UserData, 1, sizeof(STR_UserData), fp))
    {
        LOG_RED("fread %s fail\n\r", USER_DATA_PATH);
        return;
    }
    fclose(fp);
}

/*********************************************************************************************************
 * 函 数 名 : user_data_save
 * 功能说明 : 保存用户核心数据到文件（UserData结构体持久化）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 写入USER_DATA_PATH文件，失败打印错误日志，确保数据持久化
 *********************************************************************************************************/
void user_data_save(void)
{
    LOG_WHITE(" user_data_save start\n");
    FILE *fp = fopen(USER_DATA_PATH, "wb");
    if (fp == NULL)
    {
        LOG_RED("save open %s fail\n\r", USER_DATA_PATH);
        return;
    }
    if (!fwrite(&UserData, sizeof(STR_UserData), 1, fp))
    {
        LOG_RED("write %s fail\n\r", USER_DATA_PATH);
        return;
    }
    fclose(fp);
}

/*********************************************************************************************************
 * 函 数 名 : card_id_data_init
 * 功能说明 : 初始化卡片ID数据（从文件加载card_id_string_data数组）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 从CARD_ID_DATA_PATH读取数据，读取失败打印错误日志
 *********************************************************************************************************/
void card_id_data_init(void)
{
    FILE *fp = fopen(CARD_ID_DATA_PATH, "rb");
    if (fp == NULL)
    {
        LOG_RED("init open %s fail\n\r", CARD_ID_DATA_PATH);
        return;
    }
    if (!fread(&card_id_string_data, 1, sizeof(card_id_string_data), fp))
    {
        LOG_RED("fread %s fail\n\r", CARD_ID_DATA_PATH);
        return;
    }
    fclose(fp);
}

/*********************************************************************************************************
 * 函 数 名 : get_card_id_data
 * 功能说明 : 获取指定卡号对应的卡片ID字符串
 * 形    参 : card_number - 卡片编号（索引）
 * 返 回 值 : char* - 卡片ID字符串指针，越界时返回数组对应位置（未做索引校验）
 * 备    注 : 直接返回数组元素，需确保card_number在USER_CARD_TOTAL范围内
 *********************************************************************************************************/
char *get_card_id_data(int card_number)
{
    return (char *)card_id_string_data[card_number];
}

/*********************************************************************************************************
 * 函 数 名 : deleteAllCard
 * 功能说明 : 删除所有卡片ID数据
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 先打印当前单元号日志，再清空card_id_string_data数组所有元素
 *********************************************************************************************************/
void deleteAllCard(void)
{
    /* *******************↓↓删除之前↓↓******************* */
    for (unsigned char i = 0; i < get_int_conf(UNIT_NUMBER_INDEX); i++)
    {
        LOG_WHITE("user unit number[%d] = %d\n\r", i, UserData.unit_number[i]);
    }
    /* *******************↑↑删除之后↑↑******************* */

    memset(card_id_string_data, 0, sizeof(card_id_string_data));
}

/*********************************************************************************************************
 * 函 数 名 : set_card_id_data
 * 功能说明 : 设置指定卡号的卡片ID字符串
 * 形    参 : card_number - 卡片编号（索引）；string - 卡片ID字符串
 * 返 回 值 : 无
 * 备    注 : 先清空目标数组元素（32字节），再拷贝字符串，打印设置日志
 *********************************************************************************************************/
void set_card_id_data(unsigned int card_number, char *string)
{
    LOG_WHITE("card_number:[%d]     string:[%s]\n", card_number, string);
    memset(card_id_string_data[card_number], 0, 32);
    strcpy(card_id_string_data[card_number], string);
}

/*********************************************************************************************************
 * 函 数 名 : card_id_data_save
 * 功能说明 : 保存卡片ID数据到文件（card_id_string_data数组持久化）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 写入CARD_ID_DATA_PATH文件，打印保存开始/结束日志，失败打印错误日志
 *********************************************************************************************************/
void card_id_data_save(void)
{
    LOG_WHITE(" start save card data\n");
    FILE *fp = fopen(CARD_ID_DATA_PATH, "wb");
    if (fp == NULL)
    {
        LOG_RED("save open %s fail\n\r", CARD_ID_DATA_PATH);
        return;
    }

    if (!fwrite(&card_id_string_data, sizeof(card_id_string_data), 1, fp))
    {
        LOG_RED("write %s fail\n\r", CARD_ID_DATA_PATH);
        return;
    }
    fclose(fp);
    LOG_WHITE("save card data over\n");
}

/*********************************************************************************************************
 * 函 数 名 : get_int_conf
 * 功能说明 : 获取指定索引的整型配置项
 * 形    参 : index - 配置项索引（sys_conf枚举）
 * 返 回 值 : int - 成功返回配置值，索引不合法返回-1
 * 备    注 : 校验索引是否为整型配置范围（< UNLOCK_PASSWORD），非法则打印错误日志
 *********************************************************************************************************/
int get_int_conf(enum sys_conf index)
{
    if (index >= UNLOCK_PASSWORD)
    {
        LOG_WHITE("error configure index:%d not int.\n\r", index);
        return -1;
    }
    return int_data[index];
}

/*********************************************************************************************************
 * 函 数 名 : set_int_conf
 * 功能说明 : 设置指定索引的整型配置项（内存+INI文件双更新）
 * 形    参 : index - 配置项索引（sys_conf枚举）；value - 配置值
 * 返 回 值 : int - 成功返回0，索引/值非法返回-1
 * 备    注 : 1. 校验索引（< UNLOCK_PASSWORD）和值（>= -1）；
 *           2. 更新内存int_data数组；
 *           3. 加载INI文件，更新对应配置项并写入文件；
 *           4. 释放iniparser字典资源
 *********************************************************************************************************/
int set_int_conf(enum sys_conf index, int value)
{
    if ((index >= UNLOCK_PASSWORD) || (value < -1))
    {
        LOG_RED("error configure index:%d is invalid.\n\r", index);
        return -1;
    }

    int_data[index] = value;
    char tmp_buff[10];
    sprintf(tmp_buff, "%d", int_data[index]);

    char index_key[24];
    sprintf(index_key, "configure:index%d", index);
    data_ptr = iniparser_load(USER_CONF_FILE_PATH);
    if (iniparser_set(data_ptr, index_key, tmp_buff) == 0)
    {
        FILE *fp = fopen(USER_CONF_FILE_PATH, "w");
        iniparser_dump_ini(data_ptr, fp);
        fclose(fp);
    }
    iniparser_freedict(data_ptr);
    data_ptr = NULL;
    return 0;
}

/*********************************************************************************************************
 * 函 数 名 : get_string_conf
 * 功能说明 : 获取指定索引的字符串配置项（密码类）
 * 形    参 : index - 配置项索引（sys_conf枚举）
 * 返 回 值 : char* - 成功返回配置字符串，索引不合法返回NULL
 * 备    注 : 校验索引是否为字符串配置范围（>= UNLOCK_PASSWORD），非法则打印错误日志
 *********************************************************************************************************/
char *get_string_conf(enum sys_conf index)
{
    if (index < UNLOCK_PASSWORD)
    {
        LOG_RED("error configure index:%d not string.\n\r", index);
        return NULL;
    }
    return (char *)password_string_data[index - UNLOCK_PASSWORD];
}

/*********************************************************************************************************
 * 函 数 名 : set_string_conf
 * 功能说明 : 设置指定索引的字符串配置项（内存+INI文件双更新）
 * 形    参 : index - 配置项索引（sys_conf枚举）；value - 配置字符串
 * 返 回 值 : int - 成功返回0，索引/值非法返回-1
 * 备    注 : 1. 校验索引（>= UNLOCK_PASSWORD）和值（非NULL）；
 *           2. 更新内存password_string_data数组；
 *           3. 加载INI文件，更新对应配置项并写入文件；
 *           4. 释放iniparser字典资源
 *********************************************************************************************************/
int set_string_conf(enum sys_conf index, char *value)
{
    if ((index < UNLOCK_PASSWORD) || (value == NULL))
    {
        LOG_RED("error configure index:%d not string.\n\r", index);
        return -1;
    }
    int str_array_index = (int)(index - UNLOCK_PASSWORD);

    sprintf((char *)password_string_data[str_array_index], "%s", value);

    char index_key[32];
    sprintf(index_key, "configure:index_s%d", str_array_index);
    data_ptr = iniparser_load(USER_CONF_FILE_PATH);
    if (iniparser_set(data_ptr, index_key, value) == 0)
    {
        FILE *fp = fopen(USER_CONF_FILE_PATH, "w");
        iniparser_dump_ini(data_ptr, fp);
        fclose(fp);
    }
    iniparser_freedict(data_ptr);
    data_ptr = NULL;
    return 0;
}

/*********************************************************************************************************
 * 函 数 名 : create_data_file
 * 功能说明 : 创建数据存储根目录
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 通过system调用执行mkdir -p命令，确保目录存在（多级目录自动创建）
 *********************************************************************************************************/
static void create_data_file(void)
{
    system("mkdir -p " DATA_PATH);
}

/*********************************************************************************************************
 * 函 数 名 : create_user_configure_file
 * 功能说明 : 创建空的用户配置文件（INI格式）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 创建USER_CONF_FILE_PATH文件并写入[configure]段头，失败打印错误日志
 *********************************************************************************************************/
static void create_user_configure_file(void)
{
    FILE *fp = fopen(USER_CONF_FILE_PATH, "w");
    if (fp == NULL)
    {
        LOG_RED("open %s fail.\n\r", USER_CONF_FILE_PATH);
    }
    else
    {
        fprintf(fp, "[configure]\n\r");
        fclose(fp);
    }
}

/*********************************************************************************************************
 * 函 数 名 : user_configure_init
 * 功能说明 : 初始化用户配置（目录检查+配置文件加载+数据初始化）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 1. 检查数据目录，不存在则创建；
 *           2. 检查出厂/用户配置文件，不存在则创建空配置或复制出厂配置；
 *           3. 加载INI配置文件，初始化整型/字符串配置数据（无配置则用默认值）；
 *           4. 释放iniparser字典资源
 *********************************************************************************************************/
void user_configure_init(void)
{
    if (access(DATA_PATH, F_OK) != 0)
    {
        LOG_WHITE(" -------- NO_DATA_PATH \n\r");
        create_data_file();
    }

    if ((access(FACTORY_CONF_FILE_PATH, F_OK) != 0) && (access(USER_CONF_FILE_PATH, F_OK)) != 0)
    {
        LOG_WHITE(" -------- FACTORY_CONF_FILE_PATH \n\r");
        create_user_configure_file();
    }

    if (access(USER_CONF_FILE_PATH, F_OK) != 0)
    {
        LOG_WHITE(" -------- USER_CONF_FILE_PATH \n\r");
        system("cp " FACTORY_CONF_FILE_PATH " " USER_CONF_FILE_PATH);
    }

    data_ptr = iniparser_load(USER_CONF_FILE_PATH);
    if (data_ptr == NULL)
    {
        LOG_RED("load %s fail.\n\r", USER_CONF_FILE_PATH);
    }
    else
    {
        iniparser_dump(data_ptr, stderr);
    }

    int tmp_int_conf;
    char index_key[32];
    for (int i = 0; i < UNLOCK_PASSWORD; ++i)
    {
        sprintf(index_key, "configure:index%d", i);
        tmp_int_conf = iniparser_getint(data_ptr, index_key, -1);
        if (tmp_int_conf >= 0)
        {
            int_data[i] = tmp_int_conf;
        }
        else
        {
            int_data[i] = default_int_data[i];
        }
    }

    char *tmp_str_conf;
    for (int i = 0; i < (CONFIGURE_MAX - UNLOCK_PASSWORD); ++i)
    {
        sprintf(index_key, "configure:index_s%d", i);
        tmp_str_conf = iniparser_getstring(data_ptr, index_key, NULL);
        if (tmp_str_conf != NULL)
        {
            sprintf((char *)password_string_data[i], "%s", tmp_str_conf);
        }
        else
        {
            sprintf((char *)password_string_data[i], "%s", default_string_data[i]);
        }
    }
    iniparser_freedict(data_ptr);
    data_ptr = NULL;
}

/*************************************************************************
 * @brief  恢复出厂设置/清空所有用户数据
 * @date   2022-11-03 10:34
 * @author xiaoele
 **************************************************************************/
/*********************************************************************************************************
 * 函 数 名 : restore_factory_settings
 * 功能说明 : 恢复出厂设置（重置用户配置文件并重新初始化）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 1. 删除现有用户配置文件；
 *           2. 从出厂配置文件复制新的用户配置；
 *           3. 重新初始化用户配置（加载新配置到内存）
 *********************************************************************************************************/
void restore_factory_settings(void)
{
    system("rm -rf " USER_CONF_FILE_PATH);
    system("cp " FACTORY_CONF_FILE_PATH " " USER_CONF_FILE_PATH);
    user_configure_init();
}