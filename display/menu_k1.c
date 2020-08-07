/*
 * menu_k1.c
 *
 *  Created on: 2020年6月2日
 *      Author: Administrator
 */
#include "../general.h"
#include "menu_k1.h"
#ifdef BOARD_S6_6
#define FONT_72X24_OFS_              216
#define START_X_LINE 0
#define START_X_NUM  9
#define START_X_XIN  15
#define START_X_TIP  2
#define DELAY_COMPLETE 500
#define ALARM_COUNT_MAX 100
typedef enum {
    MENU_ITEN_NULL=0,
    MENU_ITEN_ADD_GROUP,
    MENU_ITEN_DELETE_GROUP,
    MENU_ITEN_TERMINAL_TEST, //中断测试
    MENU_ITEN_CLOSE_CTROL,
    MENU_ITEN_OPEN_CTROL,
    MENU_ITEN_CLOSE_CTROL_GROUP,
    MENU_ITEN_OPEN_CTROL_GROUP,
    MENU_ITEN_TERMINAL_UNLOCKING,
    MENU_ITEN_GROUP_UNLOCKING,
    MENU_ITEN_CTROL_CLOSE_BLOCKING,
    MENU_ITEN_CTROL_OPEN_BLOCKING,
    MENU_ITEN_CTROL_POWER_SELECT_HIGH,
    MENU_ITEN_CTROL_POWER_SELECT_MID,
    MENU_ITEN_CTROL_POWER_SELECT_LOW,
    MENU_ITEN_OPEN_GROUP_PREVENT_ESCAPE,
    MENU_ITEN_CLOSE_GROUP_PREVENT_ESCAPE,
    MENU_ITEN_OPEN_TERMINQL_PREVENT_ESCAPE,
    MENU_ITEN_CLOSE_TERMINQL_PREVENT_ESCAPE,
    MENU_ITEN_LOCK_CHECK,
    MENU_ITEN_QUERY_ALARM_INFO,
    MENU_ITEN_SETTING_TIME,
    MENU_ITEM_TIK_GROUP_SUBDUE,
    MENU_ITEM_TIK_FIXED_NUMBER_SUBDUE,
    MENU_ITEM_TIK_ARR_SUBDUE,

    MENU_ITEN_MAX
} memu_item_select_main;

typedef enum {
    KEY_DOING_NULL=0,
    KEY_DOING_ACK=0x01,
    KEY_DOING_DELETE=0x02,
    KEY_DOING_TICK=0x03,
    KEY_DOING_SWITCH=0x04,
    KEY_DOING_DISPLAY,
    KEY_DOING_MAX
} KEY_DOING;



#define MENU_TIEM_NUM_MAX  MENU_ITEN_MAX
static void menu_add_group( );
static void menu_delete_group( );
static void menu_terminal_test_group( );
static void menu_close_ctrol( );
static void menu_close_ctrol_group( );
static void menu_open_ctrol( );
static void menu_open_ctrol_group( );
static void menu_terminal_unlocking( );
static void menu_group_unlocking( );
static void menu_close_blocking( );
static void menu_open_blocking( );
//static void menu_open_prevent_escape( );
//static void menu_close_prevent_escape( );
static void menu_power_select_high( );
static void menu_power_select_mid( );
static void menu_power_select_low( );
static void menu_query_alarm_info();
static void menu_lock_check();
static void menu_setting_time( );

static void menu_open_group_prevent_escape();
static void menu_close_group_prevent_escape();
static void menu_open_terminal_prevent_escape();
static void menu_close_terminal_prevent_escape();
static void menu_tik_fixed_number_subdue();
static void menu_tik_group_subdue();
static void menu_tik_arr_subdue();

static void Display_alarm(uint32_t devicesId, uint8_t type);
MenuMode_t MenuMode[]=
{
    {MENU_ITEN_ADD_GROUP,NULL,menu_add_group},
    {MENU_ITEN_DELETE_GROUP,NULL,menu_delete_group},
    {MENU_ITEN_TERMINAL_TEST,NULL,menu_terminal_test_group},
    {MENU_ITEN_CLOSE_CTROL,NULL,menu_close_ctrol},
    {MENU_ITEN_OPEN_CTROL,NULL,menu_open_ctrol},
    {MENU_ITEN_CLOSE_CTROL_GROUP,NULL,menu_close_ctrol_group},
    {MENU_ITEN_OPEN_CTROL_GROUP,NULL,menu_open_ctrol_group},
    {MENU_ITEN_TERMINAL_UNLOCKING,NULL,menu_terminal_unlocking},
    {MENU_ITEN_GROUP_UNLOCKING,NULL,menu_group_unlocking},
    {MENU_ITEN_CTROL_CLOSE_BLOCKING,NULL,menu_close_blocking},
    {MENU_ITEN_CTROL_OPEN_BLOCKING,NULL,menu_open_blocking},

    //{MENU_ITEN_CTROL_OPEN_PREVENT_ESCAPE,NULL,menu_open_prevent_escape},
    //{MENU_ITEN_CTROL_CLOSE_PREVENT_ESCAPE,NULL,menu_close_prevent_escape},
    {MENU_ITEN_CTROL_POWER_SELECT_HIGH,NULL,menu_power_select_high},
    {MENU_ITEN_CTROL_POWER_SELECT_MID,NULL,menu_power_select_mid},
    {MENU_ITEN_CTROL_POWER_SELECT_LOW,NULL,menu_power_select_low},

    {MENU_ITEN_OPEN_GROUP_PREVENT_ESCAPE,NULL,menu_open_group_prevent_escape},
    {MENU_ITEN_CLOSE_GROUP_PREVENT_ESCAPE,NULL,menu_close_group_prevent_escape},
    {MENU_ITEN_OPEN_TERMINQL_PREVENT_ESCAPE,NULL,menu_open_terminal_prevent_escape},
    {MENU_ITEN_CLOSE_TERMINQL_PREVENT_ESCAPE,NULL,menu_close_terminal_prevent_escape},
    {MENU_ITEN_LOCK_CHECK,NULL,menu_lock_check},
    {MENU_ITEN_QUERY_ALARM_INFO,NULL,menu_query_alarm_info},

    {MENU_ITEN_SETTING_TIME,NULL,menu_setting_time},
    {MENU_ITEM_TIK_GROUP_SUBDUE,NULL,menu_tik_group_subdue},
    {MENU_ITEM_TIK_FIXED_NUMBER_SUBDUE,NULL,menu_tik_fixed_number_subdue},
    {MENU_ITEM_TIK_ARR_SUBDUE,NULL,menu_tik_arr_subdue},

};

MenuModeObject mMenuModeObject;
MenuAlarmObject mMenuAlarmObject[ALARM_COUNT_MAX];
static Calendar calendar;
void power_on_init_key_code(void);
void insertAlarm(uint32_t devicesId,uint8_t Alarmtype)
{
    uint8_t i = 0;
   for(i = 0 ; i < ALARM_COUNT_MAX;i++)
   {
       if(mMenuAlarmObject[i].alarmType == Alarmtype && mMenuAlarmObject[i].devicesId == devicesId)
           goto INSET_RET_TAB;

   }

   for(i = 0; i < ALARM_COUNT_MAX;i++)
   {
     if(mMenuAlarmObject[i].devicesId == 0)
     {
         mMenuAlarmObject[i].alarmType =   Alarmtype;
         mMenuAlarmObject[i].devicesId =   devicesId;
         goto INSET_RET_TAB;
     }
   }

   mMenuAlarmObject[0].alarmType =   Alarmtype;
   mMenuAlarmObject[0].devicesId =   devicesId;

INSET_RET_TAB:
   Sys_event_post(SYS_EVT_ALARM);
}
static uint8_t Alarmcount = 0;
static uint8_t get_alarm_count(void)
{
    uint8_t i = 0 ;
    Alarmcount = 0;
    for(i = 0 ; i < ALARM_COUNT_MAX;i++)
    {
        if(mMenuAlarmObject[i].devicesId != 0)
           Alarmcount++;
        else
          break;
    }
return Alarmcount;
}


void set_alarm_init(void)
{
    //alarmNum = 0;
    //havefindAlarm = 0;
}
uint8_t get_menu_alarmOrSetting(void)
{
  return  mMenuModeObject.alarmOrSetting;
}

void set_meun_alarmOrSetting(uint8_t flag)
{
    mMenuModeObject.alarmOrSetting = flag;
    if(flag != 0)
        set_alarm_init();
}
MenuAlarmObject MenuAlarmObjectTemp;
#if 0
void menuc_alarm_main(KEY_CODE_E keyCode)
{
   int8_t i = alarmNum;

   uint32_t deviceIdHex = 0;
   switch(keyCode)
   {
      case _VK_MODE:
          set_meun_alarmOrSetting(0);
          power_on_init_key_code();
          menuc_main(_VK_MODE);
          break;
      case _VK_OK:
          if(MenuAlarmObjectTemp.alarmType != 0 && MenuAlarmObjectTemp.devicesId != 0)
          {
              MenuAlarmObjectTemp.alarmType = 0;
              MenuAlarmObjectTemp.devicesId  = 0;

              mMenuAlarmObject[0].alarmType = 0;
              mMenuAlarmObject[0].devicesId = 0;

              for(i = 0 ; i < ALARM_COUNT_MAX-1;i++)
              {
                  mMenuAlarmObject[i].alarmType = mMenuAlarmObject[i+1].alarmType;
                  mMenuAlarmObject[i].devicesId = mMenuAlarmObject[i+1].devicesId;
              }

              MenuAlarmObjectTemp.alarmType =   mMenuAlarmObject[0].alarmType;
              MenuAlarmObjectTemp.devicesId =   mMenuAlarmObject[0].devicesId;
          }


          if(MenuAlarmObjectTemp.alarmType != 0 &&  MenuAlarmObjectTemp.devicesId != 0)
          {
              deviceIdHex = TransHexToInt(MenuAlarmObjectTemp.devicesId);
              switch(MenuAlarmObjectTemp.alarmType)
              {
                  case ALARM_TYPE_LOW_POWER:
                       Menu_low_power_display(deviceIdHex);
                       break;
                  case ALARM_TYPE_UNWEAR:
                       Menu_not_wearing_well_display(deviceIdHex);
                       break;
                  case ALARM_TYPE_DESTORY:
                      Menu_term_is_destroyed(deviceIdHex);
                       break;
              }
          }
          else
          {
               Menu_term_is_no_arm();
          }

          break;
      case _VK_DISPLAY:
          MenuAlarmObjectTemp.alarmType = 0;
          MenuAlarmObjectTemp.devicesId = 0;

          MenuAlarmObjectTemp.alarmType =   mMenuAlarmObject[0].alarmType;
          MenuAlarmObjectTemp.devicesId =   mMenuAlarmObject[0].devicesId;

          if(MenuAlarmObjectTemp.alarmType != 0 &&  MenuAlarmObjectTemp.devicesId != 0)
          {
              deviceIdHex = TransHexToInt(MenuAlarmObjectTemp.devicesId);
              switch(MenuAlarmObjectTemp.alarmType)
              {
                  case ALARM_TYPE_LOW_POWER:
                       Menu_low_power_display(deviceIdHex);
                       break;
                  case ALARM_TYPE_UNWEAR:
                       Menu_not_wearing_well_display(deviceIdHex);
                       break;
                  case ALARM_TYPE_DESTORY:
                      Menu_term_is_destroyed(deviceIdHex);
                       break;
              }
          }
          else
          {
               Menu_term_is_no_arm();
          }
           break;
      case _VK_DELETE:
#if 0
           if(havefindAlarm)
           {
               for(i = 0 ; i < ALARM_COUNT_MAX;i++)
               {
                   if(mMenuAlarmObject[i].alarmType == MenuAlarmObjectTemp.alarmType && mMenuAlarmObject[i].devicesId == MenuAlarmObjectTemp.devicesId)
                   {
                       mMenuAlarmObject[i].alarmType = 0;
                       mMenuAlarmObject[i].devicesId = 0;
                       break;
                   }
               }

               for(;(i+1) < ALARM_COUNT_MAX;i++)
               {
                   mMenuAlarmObject[i].alarmType = mMenuAlarmObject[i+1].alarmType ;
                   mMenuAlarmObject[i].devicesId = mMenuAlarmObject[i+1].devicesId ;
               }

               Lcd_set_font(36, 24, 1);
               Disp_icon(START_X_TIP,2,ICON_36X24_COMPLETE,1);
               Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,2,ICON_36X24_CLEAR,1);

               if(mMenuAlarmObject[0].alarmType == 0 && mMenuAlarmObject[0].devicesId == 0)
               {
                   power_on_init_key_code();
                   set_meun_alarmOrSetting(0);
                   menuc_main(_VK_COMMAND);
               }

           }
#endif
          break;
      case _VK_COMMAND:
           power_on_init_key_code();
           set_meun_alarmOrSetting(0);
           menuc_main(_VK_COMMAND);
          break;

   }

#if 0
   for(; i < ALARM_COUNT_MAX;i++)
   {
       if(mMenuAlarmObject[i].alarmType != 0 && mMenuAlarmObject[i].devicesId != 0)
       {
           MenuAlarmObjectTemp.alarmType = mMenuAlarmObject[i].alarmType;
           MenuAlarmObjectTemp.devicesId = mMenuAlarmObject[i].devicesId;
           alarmNum = i;
           break;
       }
   }
#endif
}
#endif
void power_on_init_key_code(void)
{
    mMenuModeObject.index =    MENU_ITEN_NULL;
    mMenuModeObject.keyDoing = KEY_DOING_NULL;
    mMenuModeObject.selectIndex = 0;
    mMenuModeObject.numEnter = 0;
    mMenuModeObject.devicesId = 0;
    mMenuModeObject.groudId = 0;
    mMenuModeObject.alarmOrSetting = 0;

}
static void menuModeObject_data_reinit(void)
{
    //mMenuModeObject.devicesId = 0;
    //mMenuModeObject.groudId = 0;
    mMenuModeObject.keyDoing = KEY_DOING_NULL;
    mMenuModeObject.selectIndex = 0;

    //Lcd_clear_screen();

    switch(mMenuModeObject.index)
    {
    case MENU_ITEN_SETTING_TIME:
         mMenuModeObject.numEnter = 0;
         mMenuModeObject.timerefesh = 0;
         calendar =Rtc_get_calendar();
         mMenuModeObject.numEnter = calendar.Year;
         break;
    case MENU_ITEN_ADD_GROUP:
    case MENU_ITEN_DELETE_GROUP:
    case MENU_ITEN_TERMINAL_TEST:
    case MENU_ITEN_CLOSE_CTROL:
    case MENU_ITEN_OPEN_CTROL:
    case MENU_ITEN_TERMINAL_UNLOCKING:
    case MENU_ITEN_CTROL_CLOSE_BLOCKING:
    case MENU_ITEN_CTROL_OPEN_BLOCKING:
    case MENU_ITEN_CTROL_POWER_SELECT_HIGH:
    case MENU_ITEN_CTROL_POWER_SELECT_MID:
    case MENU_ITEN_CTROL_POWER_SELECT_LOW:
    case MENU_ITEN_OPEN_TERMINQL_PREVENT_ESCAPE:
    case MENU_ITEN_CLOSE_TERMINQL_PREVENT_ESCAPE:
    case MENU_ITEM_TIK_FIXED_NUMBER_SUBDUE:
         mMenuModeObject.numEnter = mMenuModeObject.devicesId;
         break;

    case MENU_ITEN_CLOSE_CTROL_GROUP:
    case MENU_ITEN_OPEN_CTROL_GROUP:
    case MENU_ITEN_OPEN_GROUP_PREVENT_ESCAPE:
    case MENU_ITEN_CLOSE_GROUP_PREVENT_ESCAPE:
    case MENU_ITEN_GROUP_UNLOCKING:
    case MENU_ITEM_TIK_ARR_SUBDUE:
           mMenuModeObject.numEnter = mMenuModeObject.groudId;
         break;


    }
    //Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);
}
void menuc_main(KEY_CODE_E keyCode)
{
    KEY_CODE_E keyCodeIn = keyCode;
    //Disp_proc();
    switch(keyCodeIn)
    {
        case _VK_COMMAND://命令
             if((mMenuModeObject.index >= MENU_ITEM_TIK_GROUP_SUBDUE) && (mMenuModeObject.index<MENU_ITEM_TIK_ARR_SUBDUE))
             {
                mMenuModeObject.index++;
             }
            else
            {
                mMenuModeObject.index=MENU_ITEM_TIK_GROUP_SUBDUE;
            }

            menuModeObject_data_reinit();
        break;


        case _VK_ACTIVE://电击
            if(mMenuModeObject.index >=MENU_ITEM_TIK_GROUP_SUBDUE && mMenuModeObject.index < MENU_ITEN_MAX)
                mMenuModeObject.keyDoing = KEY_DOING_TICK;
            else if(mMenuModeObject.index >=MENU_ITEN_ADD_GROUP && mMenuModeObject.index <= MENU_ITEN_SETTING_TIME )
                mMenuModeObject.keyDoing = KEY_DOING_SWITCH;
        break;


        case _VK_DELETE:
            if(mMenuModeObject.index !=MENU_ITEN_NULL && mMenuModeObject.numEnter > 0)
            {
                mMenuModeObject.numEnter=mMenuModeObject.numEnter/10;
                mMenuModeObject.keyDoing = KEY_DOING_DELETE;
            }
        break;


        case _VK_NUM1:
            if(mMenuModeObject.index !=MENU_ITEN_NULL && mMenuModeObject.numEnter < 10000)
            {
                mMenuModeObject.numEnter=mMenuModeObject.numEnter*10+1;
            }
        break;


        case _VK_NUM2:
            if(mMenuModeObject.index !=MENU_ITEN_NULL && mMenuModeObject.numEnter < 10000)
            {
                mMenuModeObject.numEnter=mMenuModeObject.numEnter*10+2;
            }
        break;


        case _VK_NUM3:
            if(mMenuModeObject.index !=MENU_ITEN_NULL && mMenuModeObject.numEnter < 10000)
            {
                mMenuModeObject.numEnter=mMenuModeObject.numEnter*10+3;
            }
        break;


        case _VK_NUM4:
            if(mMenuModeObject.index !=MENU_ITEN_NULL && mMenuModeObject.numEnter < 10000)
            {
                mMenuModeObject.numEnter=mMenuModeObject.numEnter*10+4;
            }
        break;


        case _VK_NUM5:
            if(mMenuModeObject.index !=MENU_ITEN_NULL && mMenuModeObject.numEnter < 10000)
            {
                mMenuModeObject.numEnter=mMenuModeObject.numEnter*10+5;
            }
        break;


        case _VK_NUM6:
            if(mMenuModeObject.index !=MENU_ITEN_NULL && mMenuModeObject.numEnter < 10000)
            {
                mMenuModeObject.numEnter=mMenuModeObject.numEnter*10+6;
            }
        break;


        case _VK_NUM7:
            if(mMenuModeObject.index !=MENU_ITEN_NULL && mMenuModeObject.numEnter < 10000)
            {
                mMenuModeObject.numEnter=mMenuModeObject.numEnter*10+7;
            }
        break;


        case _VK_NUM8:
            if(mMenuModeObject.index !=MENU_ITEN_NULL && mMenuModeObject.numEnter < 10000)
            {
                mMenuModeObject.numEnter=mMenuModeObject.numEnter*10+8;
            }
        break;


        case _VK_NUM9:
            if(mMenuModeObject.index !=MENU_ITEN_NULL && mMenuModeObject.numEnter < 10000)
            {
                mMenuModeObject.numEnter=mMenuModeObject.numEnter*10+9;
            }
        break;


        case _VK_MODE://模式
            if(mMenuModeObject.index == MENU_ITEN_TERMINAL_TEST)
            {
               Lcd_set_font(72, 24, 1);
               Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
            }

           mMenuModeObject.index++;
          if(mMenuModeObject.index >= MENU_ITEM_TIK_GROUP_SUBDUE)
             mMenuModeObject.index= (MENU_ITEN_NULL+1);

            menuModeObject_data_reinit();

            if(mMenuModeObject.index == MENU_ITEN_QUERY_ALARM_INFO)
                mMenuModeObject.keyDoing = KEY_DOING_DISPLAY;

            if(mMenuModeObject.index == MENU_ITEN_TERMINAL_TEST)
            {
               Lcd_set_font(72, 24, 1);
               Disp_icon(START_X_LINE,2,ICON_72X24_CLEAR,1);
               Lcd_set_font(36, 24, 1);
               Disp_icon(START_X_LINE,2,ICON_36X24_VBAT,1);
               Lcd_set_font(8, 24, 1);
               Disp_msg(START_X_NUM,2,"     ",FONT_8X24);
            }
        break;


        case _VK_NUM0:
            if(mMenuModeObject.index !=MENU_ITEN_NULL && mMenuModeObject.numEnter < 1000 && mMenuModeObject.numEnter !=0)
            {
                mMenuModeObject.numEnter=mMenuModeObject.numEnter*10;
            }
        break;


        case _VK_OK://确认
            if(mMenuModeObject.index >MENU_ITEN_NULL && mMenuModeObject.index < MENU_ITEM_TIK_GROUP_SUBDUE)
                mMenuModeObject.keyDoing = KEY_DOING_ACK;
            break;
        case _VK_DISPLAY:
             mMenuModeObject.index = MENU_ITEN_QUERY_ALARM_INFO;
             mMenuModeObject.keyDoing = KEY_DOING_DISPLAY;
             break;
    }
    if(mMenuModeObject.index !=MENU_ITEN_NULL)
    {

           MenuMode[mMenuModeObject.index-1].func();
    }
}

void menu_add_group( )
{
    uint8_t numbuff[10] = {0};
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
#if 1
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_ADD_ARR,1);
           Lcd_set_font(8, 24, 1);

           Disp_msg(START_X_NUM,1,"     ",FONT_8X24);
           Disp_msg(START_X_NUM,2,"     ",FONT_8X24);

           mMenuModeObject.devicesId = mMenuModeObject.numEnter;

           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);

           if(mMenuModeObject.devicesId != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }


           if(mMenuModeObject.groudId != 0)
           {
               sprintf((char*)numbuff,"%d",mMenuModeObject.groudId);
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }



#endif
       }
       else if(mMenuModeObject.selectIndex == 1)
       {
           Lcd_set_font(8, 24, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","     ");
             Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
             mMenuModeObject.groudId = mMenuModeObject.numEnter;
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
           Lcd_set_font(8, 24, 1);
           if(mMenuModeObject.numEnter != 0)
           {
               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
           mMenuModeObject.groudId = mMenuModeObject.numEnter;
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
          if(mMenuModeObject.devicesId != 0 && mMenuModeObject.groudId != 0)
           {
               //mMenuModeObject.groudId = mMenuModeObject.numEnter;
               Lcd_set_font(8, 24, 1);
               //Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
               //Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
               //调用发射窗口
               //display complete
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithNoRespon(RADIO_PRO_CMD_TERM_ADD_TO_GROUP,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);

               Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);
               //mMenuModeObject.numEnter = mMenuModeObject.devicesId;
               //mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               //goto TAB_REPEAT_ADD_ARR;
           }

           //mMenuModeObject.numEnter = 0;

       }
       else if(mMenuModeObject.keyDoing == KEY_DOING_SWITCH)
       {
           if(mMenuModeObject.selectIndex == 0)
           {
               //mMenuModeObject.devicesId = mMenuModeObject.numEnter;
               mMenuModeObject.selectIndex =1;

               Lcd_set_font(8, 24, 1);
               Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
               Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
               mMenuModeObject.numEnter = mMenuModeObject.groudId;
           }
           else if(mMenuModeObject.selectIndex == 1)
           {
               //mMenuModeObject.groudId = mMenuModeObject.numEnter;
               Lcd_set_font(8, 24, 1);
               Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
               Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);

               mMenuModeObject.numEnter = mMenuModeObject.devicesId;
               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
           }
       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }
}

static void menu_delete_group( )
{
    uint8_t numbuff[10] = {0};

   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {

           Disp_icon(START_X_LINE,3,ICON_72X24_DELETE_ARR,1);
           Lcd_set_font(8, 24, 1);

           Disp_msg(START_X_NUM,1,"     ",FONT_8X24);
           Disp_msg(START_X_NUM,2,"     ",FONT_8X24);


           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);


           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }


           if(mMenuModeObject.groudId != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.groudId);
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }


           mMenuModeObject.devicesId = mMenuModeObject.numEnter;

       }
       else if(mMenuModeObject.selectIndex == 1)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);

           //Disp_icon(0,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           //Disp_icon(START_X_LINE,3,ICON_72X24_DELETE_ARR,1);


           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","     ");
             Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);

           }

           Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);

           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
           mMenuModeObject.groudId = mMenuModeObject.numEnter;
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {

           if(mMenuModeObject.devicesId != 0 && mMenuModeObject.groudId != 0)
           {
               //mMenuModeObject.groudId = mMenuModeObject.numEnter;
               Lcd_set_font(8, 24, 1);
               //Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
               //Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);

               //调用发射窗口
               //display complete
               //Lcd_clear_screen();
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithNoRespon(RADIO_PRO_CMD_TERM_DELETE_FROM_GROUP,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);
               //
               Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);
               //mMenuModeObject.numEnter = mMenuModeObject.devicesId;
               //mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               //goto TAB_REPEAT_ADD_ARR1;
           }
           //mMenuModeObject.numEnter = 0;
       }
       else if(mMenuModeObject.keyDoing == KEY_DOING_SWITCH )
       {
           if(mMenuModeObject.selectIndex == 0)
           {
               //mMenuModeObject.devicesId = mMenuModeObject.numEnter;
               mMenuModeObject.selectIndex =1;
               Lcd_set_font(8, 24, 1);
               Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
               Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);

               mMenuModeObject.numEnter = mMenuModeObject.groudId;

           }
           else if(mMenuModeObject.selectIndex == 1)
           {
               //mMenuModeObject.groudId = mMenuModeObject.numEnter;
               Lcd_set_font(8, 24, 1);
               Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
               Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);

               mMenuModeObject.numEnter = mMenuModeObject.devicesId;
               mMenuModeObject.selectIndex = 0;
           }

       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }

}
static void menu_terminal_test_group( )
{
    uint8_t numbuff[10] = {0};
    uint8_t numbat[5] = {0};
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           //Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           //Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_TERMINAL_TEST,1);
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);

           Disp_msg(START_X_NUM,1,"     ",FONT_8X24);

           mMenuModeObject.devicesId = mMenuModeObject.numEnter;
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }

           //if(mMenuModeObject.groudId != 0)
           {

               sprintf((char*)numbuff,"%s","-----");
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.devicesId !=0)
           {
               mMenuModeObject.devicesId = mMenuModeObject.numEnter;
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithRespon(RADIO_PRO_CMD_TERM_TEST,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               {

                   Lcd_set_font(8, 24, 1);
                   Disp_msg(START_X_NUM,2,"     ",FONT_8X24);
                   sprintf(numbat," %02d%%",GetTestTermVol());
                   Disp_msg(START_X_NUM,2,numbat,FONT_8X24);
                   Lcd_set_font(36, 24, 1);
                   Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);


               }
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);

               Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);


               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
           }


       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }
}
static void menu_close_ctrol()//关闭制服
{
    uint8_t numbuff[10] = {0};
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           //Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           //Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_CLOSE_CTROL,1);
           Lcd_set_font(8, 24, 1);

           Disp_msg(START_X_NUM,1,"     ",FONT_8X24);

           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);

           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }


           //if(mMenuModeObject.groudId != 0)
           {

               sprintf((char*)numbuff,"%s","-----");
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
           mMenuModeObject.devicesId = mMenuModeObject.numEnter;
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.numEnter !=0)
           {
               mMenuModeObject.devicesId = mMenuModeObject.numEnter;
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithNoRespon(RADIO_PRO_CMD_TERM_CLOSE_CTROL,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);

               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;

               Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);

           }
       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }
}
static void menu_close_ctrol_group( )//关组制服
{
    uint8_t numbuff[10] = {0};

   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           //Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           //Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_CLOSE_GROUP_CTROL,1);
           Lcd_set_font(8, 24, 1);
           Disp_msg(START_X_NUM,2,"     ",FONT_8X24);

           Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);

           Lcd_set_font(8, 24, 1);

           {
              //sprintf((char*)numbuff,"%d",mMenuModeObject.devicesId);
               Disp_msg(START_X_NUM,1,"-----",FONT_8X24);
           }

           if(mMenuModeObject.numEnter != 0)
           {
               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }

           mMenuModeObject.groudId = mMenuModeObject.numEnter;
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 &&  mMenuModeObject.groudId !=0)
           {

               Lcd_set_font(36, 24, 1);
               //send data to devices
               RadioCmdSetWithNoRespon(RADIO_PRO_CMD_GROUP_CLOSE_CTROL,mMenuModeObject.devicesId, mMenuModeObject.groudId);  //群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);

               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;

               Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);
           }


       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }
}
static void menu_open_ctrol( )
{
    uint8_t numbuff[10] = {0};
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           //Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
          // Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_OPEN_CTROL,1);
           Lcd_set_font(8, 24, 1);

           Disp_msg(START_X_NUM,1,"     ",FONT_8X24);

           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);

           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }


           mMenuModeObject.devicesId = mMenuModeObject.numEnter;
           Disp_msg(START_X_NUM,2,"-----",FONT_8X24);

       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.numEnter !=0)
           {
               mMenuModeObject.devicesId = mMenuModeObject.numEnter;
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithNoRespon(RADIO_PRO_CMD_TERM_OPEN_CTROL,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);

               Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);

               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;

           }
           //mMenuModeObject.numEnter = 0;
       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }

}
static void menu_open_ctrol_group( )
{
    uint8_t numbuff[10] = {0};

   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           Disp_icon(START_X_LINE,3,ICON_72X24_OPEN_GROUP_CTROL,1);
           Lcd_set_font(8, 24, 1);

           Disp_msg(START_X_NUM,2,"     ",FONT_8X24);

           Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);

           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }


           mMenuModeObject.groudId = mMenuModeObject.numEnter;
           Disp_msg(START_X_NUM,1,"-----",FONT_8X24);

       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.groudId !=0)
           {
               Lcd_set_font(36, 24, 1);
               //send data to devices
               RadioCmdSetWithNoRespon(RADIO_PRO_CMD_GROUP_OPEN_CTROL,mMenuModeObject.devicesId, mMenuModeObject.groudId);  //群组
               //Task_sleep(1000*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);

               Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);

               mMenuModeObject.selectIndex = 0;
           }


       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }
}
static void menu_terminal_unlocking()
{
    uint8_t numbuff[10] = {0};

   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           //Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           //Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_TERMINAL_UNLOCKING,1);
           Lcd_set_font(8,24, 1);
           Disp_msg(START_X_NUM,1,"     ",FONT_8X24);

           mMenuModeObject.devicesId = mMenuModeObject.numEnter;

           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);

           if(mMenuModeObject.devicesId != 0)
           {
               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }



          Disp_msg(START_X_NUM,2,"-----",FONT_8X24);

       }

   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.devicesId != 0)
           {
               Lcd_set_font(36, 24, 1);

               if(RadioCmdSetWithNoRespon(RADIO_PRO_CMD_TERM_UNLOCKING,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
                 Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
                 Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);

                 Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
                 Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);


           }
       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }


}

static void menu_group_unlocking()
{

    uint8_t numbuff[10] = {0};
    memset(numbuff,0x00,10);
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           Disp_icon(START_X_LINE,3,ICON_72X24_GROUP_UNLOCKING,1);
           Lcd_set_font(8, 24, 1);

           Disp_msg(START_X_NUM,2,"     ",FONT_8X24);


           Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);

           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }



           Disp_msg(START_X_NUM,1,"-----",FONT_8X24);


           mMenuModeObject.groudId = mMenuModeObject.numEnter;
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.groudId !=0)
           {
               Lcd_set_font(36, 24, 1);
               //send data to devices
               RadioCmdSetWithNoRespon(RADIO_PRO_CMD_GROUP_UNLOCKING,mMenuModeObject.devicesId, mMenuModeObject.groudId);  //群组
               //Task_sleep(1000*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);

               Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);


               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
           }
           //mMenuModeObject.numEnter = 0;

       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }
}

static void menu_close_blocking( )
{
    uint8_t numbuff[10] = {0};

   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {

           Disp_icon(START_X_LINE,3,ICON_72X24_CLOSE_BLOCKING,1);
           Lcd_set_font(8, 24, 1);

           Disp_msg(START_X_NUM,1,"     ",FONT_8X24);
           mMenuModeObject.devicesId = mMenuModeObject.numEnter;


           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);

           if(mMenuModeObject.devicesId != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }

           Disp_msg(START_X_NUM,2,"-----",FONT_8X24);

       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.devicesId !=0)
           {
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithNoRespon(RADIO_PRO_CMD_TERM_CLOSE_BLOCKING,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);

               mMenuModeObject.selectIndex = 0;
               Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);

           }

           //mMenuModeObject.numEnter = 0;

       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }



}

static void menu_open_blocking( )
{

    uint8_t numbuff[10] = {0};

   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           //Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           //Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_OPEN_BLOCKING,1);
           Lcd_set_font(8, 24, 1);
           Disp_msg(START_X_NUM,1,"     ",FONT_8X24);

           mMenuModeObject.devicesId = mMenuModeObject.numEnter;

           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);

           if(mMenuModeObject.devicesId != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
           Disp_msg(START_X_NUM,2,"-----",FONT_8X24);

       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.devicesId != 0)
           {
               Lcd_set_font(36, 24, 1);
               if(mMenuModeObject.numEnter != 0)
               {
                   if(RadioCmdSetWithNoRespon(RADIO_PRO_CMD_TERM_OPEN_BLOCKING,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
                   Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
                   else
                   Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);

                   Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
                   Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);

               }
               mMenuModeObject.selectIndex = 0;

           }

       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }

}
#if 0
static void menu_open_prevent_escape( )
{
    uint8_t numbuff[10] = {0};

   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_ALL_FIBID_ESCAPE,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_NOT_NEED_INPUT,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_OPEN_ESCAPE,1);

       }
       Lcd_set_font(8, 24, 1);
       sprintf((char*)numbuff,"%s","     ");
       Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
       Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {

           {
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithNoRespon(RADIO_PRO_CMD_OPEN_PREVENT_ESCAPE,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);

           }

           Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
           Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);

           mMenuModeObject.numEnter = 0;
           mMenuModeObject.groudId = 0;
           mMenuModeObject.devicesId = 0;
           mMenuModeObject.keyDoing = KEY_DOING_NULL;
       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }

}
static void menu_close_prevent_escape( )
{
    uint8_t numbuff[10] = {0};

   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           //Disp_icon(START_X_LINE,1,ICON_72X24_ALL_FIBID_ESCAPE,1);
           //Disp_icon(START_X_LINE,2,ICON_72X24_NOT_NEED_INPUT,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_CLOSE_ESCAPE,1);

       }
       Lcd_set_font(8, 24, 1);
       sprintf((char*)numbuff,"%s","     ");
       Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
       Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {

           {
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithNoRespon(RADIO_PRO_CMD_CLOSE_PREVENT_ESCAPE,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);
               mMenuModeObject.selectIndex = 2;
           }

           Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
           Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);

           mMenuModeObject.numEnter = 0;
           mMenuModeObject.groudId = 0;
           mMenuModeObject.devicesId = 0;
           mMenuModeObject.keyDoing = KEY_DOING_NULL;
       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }

}
#endif
static void menu_power_select_high()
{
    uint8_t numbuff[10] = {0};

   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_POWER_HIGH,1);
           Lcd_set_font(8, 24, 1);

           Disp_msg(START_X_NUM,1,"     ",FONT_8X24);

           mMenuModeObject.devicesId = mMenuModeObject.numEnter;

           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);

           if(mMenuModeObject.devicesId != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }

           Disp_msg(START_X_NUM,2,"-----",FONT_8X24);

       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.devicesId != 0)
           {
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithNoRespon(RADIO_PRO_CMD_TERM_POWER_HIGH,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);

               Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);

           }

       }

       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }

}
static void menu_power_select_mid()
{
    uint8_t numbuff[10] = {0};

   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           //Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           //Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_POWER_MID,1);
           Lcd_set_font(8, 24, 1);

           Disp_msg(START_X_NUM,1,"     ",FONT_8X24);


           mMenuModeObject.devicesId = mMenuModeObject.numEnter;

           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);

           if(mMenuModeObject.devicesId != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.devicesId);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }



           Disp_msg(START_X_NUM,2,"-----",FONT_8X24);

       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.devicesId != 0)
           {


               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithNoRespon(RADIO_PRO_CMD_TERM_POWER_MID,mMenuModeObject.devicesId, mMenuModeObject.groudId))
                  Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
                   Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);

               Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);

           }
       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }

}
static void menu_power_select_low()
{
    uint8_t numbuff[10] = {0};

    if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
    {

        Lcd_set_font(72, 24, 1);
        if(mMenuModeObject.selectIndex == 0)
        {
            Disp_icon(START_X_LINE,3,ICON_72X24_POWER_LOW,1);
            Lcd_set_font(8, 24, 1);
            Disp_msg(START_X_NUM,1,"     ",FONT_8X24);

            mMenuModeObject.devicesId = mMenuModeObject.numEnter;

            Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
            Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);

            if(mMenuModeObject.devicesId != 0)
            {

                sprintf((char*)numbuff,"%d",mMenuModeObject.devicesId);
                Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
            }

            Disp_msg(START_X_NUM,2,"-----",FONT_8X24);

        }
    }
    else
    {
        if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
        {
            if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.devicesId!= 0)
            {
                Lcd_set_font(36, 24, 1);
                if(RadioCmdSetWithNoRespon(RADIO_PRO_CMD_TERM_POWER_LOW,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
                Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
                else
                Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);


                Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
                Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);
            }

            else if(mMenuModeObject.selectIndex == 1 && mMenuModeObject.numEnter !=0)
            {
                mMenuModeObject.groudId = mMenuModeObject.numEnter;
                Lcd_set_font(8, 16, 1);
                RadioCmdSetWithNoRespon(RADIO_PRO_CMD_GROUP_POWER_LOW,mMenuModeObject.devicesId, mMenuModeObject.groudId);

                Lcd_set_font(36, 24, 1);
                Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);

                Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
                Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);
            }

            mMenuModeObject.keyDoing = KEY_DOING_NULL;
        }

#if 0
        else if(mMenuModeObject.keyDoing == KEY_DOING_SWITCH)
        {

            mMenuModeObject.keyDoing = KEY_DOING_NULL;
            if(mMenuModeObject.selectIndex == 0)
            {
                mMenuModeObject.selectIndex =1;
                mMenuModeObject.numEnter = mMenuModeObject.groudId;
                Lcd_set_font(8, 24, 1);
                Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
                Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);

            }
            else if(mMenuModeObject.selectIndex == 1)
            {
                mMenuModeObject.selectIndex =0;
                mMenuModeObject.numEnter = mMenuModeObject.devicesId;
                Lcd_set_font(8, 24, 1);
                Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);
                Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);

            }
        }
#endif
        mMenuModeObject.keyDoing = KEY_DOING_NULL;
    }


}

static void menu_lock_check()
{
    uint8_t numbuff[10] = {0};
    uint8_t numbat[5] = {0};
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           //Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           //Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_LOCK_CHECK,1);
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);

           Disp_msg(START_X_NUM,1,"     ",FONT_8X24);

           mMenuModeObject.devicesId = mMenuModeObject.numEnter;
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }

           //if(mMenuModeObject.groudId != 0)
           {

               sprintf((char*)numbuff,"%s","-----");
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.devicesId !=0)
           {
               mMenuModeObject.devicesId = mMenuModeObject.numEnter;
               Lcd_set_font(36, 24, 1);
               RadioCmdSetWithNoRespon(RADIO_PRO_CMD_MOTO_RUN,mMenuModeObject.devicesId, mMenuModeObject.groudId);  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);

               Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);

               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
           }


       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }

}

static void menu_open_group_prevent_escape()
{
    uint8_t numbuff[10] = {0};

   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           Disp_icon(START_X_LINE,3,ICON_72X24_OPEN_GROUP_ESCAPE,1);
           Lcd_set_font(8, 24, 1);

           Disp_msg(START_X_NUM,2,"     ",FONT_8X24);

           mMenuModeObject.groudId = mMenuModeObject.numEnter;
           Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);

           if(mMenuModeObject.groudId != 0)
           {
               sprintf((char*)numbuff,"%d",mMenuModeObject.groudId);
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }

           Disp_msg(START_X_NUM,1,"-----",FONT_8X24);

       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.groudId !=0)
           {
               Lcd_set_font(36, 24, 1);
               //send data to devices
               RadioCmdSetWithNoRespon(RADIO_PRO_CMD_OPEN_GROUP_PREVENT_ESCAPE,mMenuModeObject.devicesId, mMenuModeObject.groudId);  //群组
               //Task_sleep(1000*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);

               Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);


               mMenuModeObject.selectIndex = 0;
           }
           //mMenuModeObject.numEnter = 0;

       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }
}

static void menu_close_group_prevent_escape()
{
    uint8_t numbuff[10] = {0};

   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           Disp_icon(START_X_LINE,3,ICON_72X24_CLOSE_GROUP_ESCAPE,1);
           Lcd_set_font(8, 24, 1);

           Disp_msg(START_X_NUM,2,"     ",FONT_8X24);

           mMenuModeObject.groudId = mMenuModeObject.numEnter;

           Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);

           if(mMenuModeObject.groudId != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.groudId);
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }



           Disp_msg(START_X_NUM,1,"-----",FONT_8X24);
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.groudId !=0)
           {
               Lcd_set_font(36, 24, 1);
               //send data to devices
               RadioCmdSetWithNoRespon(RADIO_PRO_CMD_CLOSE_GROUP_PREVENT_ESCAPE,mMenuModeObject.devicesId, mMenuModeObject.groudId);  //群组
               //Task_sleep(1000*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);

               Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);


               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
           }
           //mMenuModeObject.numEnter = 0;

       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }
}
static void menu_open_terminal_prevent_escape()
{    uint8_t numbuff[10] = {0};

   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           //Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           //Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_OPEN_ESCAPE,1);
           Lcd_set_font(8, 24, 1);

           Disp_msg(START_X_NUM,1,"     ",FONT_8X24);

           mMenuModeObject.devicesId = mMenuModeObject.numEnter;

           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);

           if(mMenuModeObject.devicesId != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.devicesId);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
               Disp_msg(START_X_NUM,2,"-----",FONT_8X24);

       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.devicesId !=0)
           {
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithNoRespon(RADIO_PRO_CMD_OPEN_TERMINAL_PREVENT_ESCAPE,mMenuModeObject.devicesId, mMenuModeObject.groudId))
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);

               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;

               Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);


           }
           mMenuModeObject.numEnter = 0;

       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }

}
static void menu_close_terminal_prevent_escape()
{
    uint8_t numbuff[10] = {0};

   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           //Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           //Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_CLOSE_ESCAPE,1);
           Lcd_set_font(8, 24, 1);
           Disp_msg(START_X_NUM,1,"     ",FONT_8X24);

           mMenuModeObject.devicesId = mMenuModeObject.numEnter;

           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);

           if(mMenuModeObject.devicesId != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.devicesId);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }



               Disp_msg(START_X_NUM,2,"-----",FONT_8X24);
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.devicesId !=0)
           {
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithNoRespon(RADIO_PRO_CMD_CLOSE_TERMINAL_PREVENT_ESCAPE,mMenuModeObject.devicesId, mMenuModeObject.groudId))
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);

               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;

               Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);


           }


       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }

}
static void menu_tik_fixed_number_subdue()
{

    uint8_t numbuff[10] = {0};
    uint32_t  eventId,timeCnt = 0;
//TAB_REPEAT_ADD_ARR11:
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
      // if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_FIXED_PEOPLE_SUBDUE,1);
           Lcd_set_font(8, 24, 1);
           Disp_msg(START_X_NUM,1,"     ",FONT_8X24);

           mMenuModeObject.devicesId = mMenuModeObject.numEnter;

           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);

           if(mMenuModeObject.devicesId != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.devicesId);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }

           Disp_msg(START_X_NUM,2,"-----",FONT_8X24);
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_TICK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.devicesId !=0)
           {

               Lcd_set_font(36, 24, 1);
               //send data to devices
               if(RadioCmdSetWithNoRespon(RADIO_PRO_CMD_FIXED_TERM_SUBDUE_START,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_SUBDUE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);

               Task_sleep(1000*CLOCK_UNIT_MS);
               Lcd_set_font(72, 24, 1);
               Disp_icon(START_X_LINE,3,ICON_72X24_FIXED_PEOPLE_STOP,1);
               Lcd_set_font(36, 24, 1);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);

               Event_pend(systemAppEvtHandle, 0, SYS_EVT_KEY_SCAN, BIOS_NO_WAIT);
               KeyScanFxn();
               while(timeCnt < 70)
               {
                   eventId = Event_pend(systemAppEvtHandle, 0, SYS_EVT_KEY_SCAN, 100*CLOCK_UNIT_MS);
                   if(eventId & SYS_EVT_KEY_SCAN){
                       KeyScanFxn();
                       // clear SYS_EVT_KEY_SCAN event
                       Event_pend(systemAppEvtHandle, 0, SYS_EVT_KEY_SCAN, BIOS_NO_WAIT);
                       if(Key_get() == _VK_OK)
                       {
                           Event_pend(systemAppEvtHandle, 0, SYSTEMAPP_EVT_KEY, BIOS_NO_WAIT);
                           //Lcd_set_font(72, 24, 1);
                           //Disp_icon(START_X_LINE,3,ICON_72X24_FIXED_PEOPLE_STOP,1);
                           //send data to devices
                           RadioCmdSetWithNoRespon(RADIO_PRO_CMD_FIXED_TERM_SUBDUE_STOP,mMenuModeObject.devicesId, mMenuModeObject.groudId);
                           Lcd_set_font(36, 24, 1);
                           Disp_icon(START_X_TIP,3,ICON_36X24_STOP,1);
                           Task_sleep(1000*CLOCK_UNIT_MS);
                           break;
                       }
                       else if((Key_get() == _VK_SELECT) || (Key_get() == _VK_COMMAND))
                       {
                             break;
                       }

                   }

                   timeCnt++;
               }


               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);
               Lcd_set_font(72, 24, 1);
               Disp_icon(START_X_LINE,3,ICON_72X24_FIXED_PEOPLE_SUBDUE,1);

               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;


           }
           else if(mMenuModeObject.selectIndex == 1)
           {
               Lcd_set_font(72, 24, 1);
               Disp_icon(START_X_LINE,3,ICON_72X24_FIXED_PEOPLE_STOP,1);
               //send data to devices
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithNoRespon(RADIO_PRO_CMD_FIXED_TERM_SUBDUE_STOP,mMenuModeObject.devicesId, mMenuModeObject.groudId))
               Disp_icon(START_X_TIP,3,ICON_36X24_STOP,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);

               Task_sleep(1000*CLOCK_UNIT_MS);

               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);
               Lcd_set_font(72, 24, 1);
               Disp_icon(START_X_LINE,3,ICON_72X24_FIXED_PEOPLE_SUBDUE,1);

               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;

           }
           //mMenuModeObject.numEnter = 0;

       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }
}
static void menu_tik_arr_subdue()
{
    uint8_t numbuff[10] = {0};
    uint32_t    eventId,timeCnt = 0;

   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       //if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_ARR_SUBDUE,1);
           Lcd_set_font(8, 24, 1);
           Disp_msg(START_X_NUM,2,"     ",FONT_8X24);

           mMenuModeObject.groudId = mMenuModeObject.numEnter;

           Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);

           if(mMenuModeObject.groudId != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.groudId);
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }

           Disp_msg(START_X_NUM,1,"-----",FONT_8X24);
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_TICK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.groudId !=0)
           {

               Lcd_set_font(36, 24, 1);
               //send data to devices
               RadioCmdSetWithNoRespon(RADIO_PRO_CMD_GROUP_SUBDUE_START,mMenuModeObject.devicesId, mMenuModeObject.groudId);
               Disp_icon(START_X_TIP,3,ICON_36X24_SUBDUE,1);
               Task_sleep(1000*CLOCK_UNIT_MS);
               Lcd_set_font(72, 24, 1);
               Disp_icon(START_X_LINE,3,ICON_72X24_ARR_STOP,1);
               Lcd_set_font(36, 24, 1);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);

               Event_pend(systemAppEvtHandle, 0, SYS_EVT_KEY_SCAN, BIOS_NO_WAIT);
               KeyScanFxn();
               while(timeCnt < 70)
               {
                   eventId = Event_pend(systemAppEvtHandle, 0, SYS_EVT_KEY_SCAN, 100*CLOCK_UNIT_MS);
                   if(eventId & SYS_EVT_KEY_SCAN){
                       KeyScanFxn();
                       // clear SYS_EVT_KEY_SCAN event
                       Event_pend(systemAppEvtHandle, 0, SYS_EVT_KEY_SCAN, BIOS_NO_WAIT);
                       if(Key_get() == _VK_OK)
                       {
                           Event_pend(systemAppEvtHandle, 0, SYSTEMAPP_EVT_KEY, BIOS_NO_WAIT);
                           //Lcd_set_font(72, 24, 1);
                           //Disp_icon(START_X_LINE,3,ICON_72X24_FIXED_PEOPLE_STOP,1);
                           //send data to devices
                           RadioCmdSetWithNoRespon(RADIO_PRO_CMD_GROUP_SUBDUE_STOP,mMenuModeObject.devicesId, mMenuModeObject.groudId);
                           Lcd_set_font(36, 24, 1);
                           Disp_icon(START_X_TIP,3,ICON_36X24_STOP,1);
                           Task_sleep(1000*CLOCK_UNIT_MS);
                           break;
                       }
                       else if((Key_get() == _VK_SELECT) || (Key_get() == _VK_COMMAND))
                       {
                             break;
                       }

                   }

                   timeCnt++;
               }

               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);
               Lcd_set_font(72, 24, 1);
               Disp_icon(START_X_LINE,3,ICON_72X24_ARR_SUBDUE,1);

               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;

           }
           else if(mMenuModeObject.selectIndex == 1)
           {
               Lcd_set_font(72, 24, 1);
               Disp_icon(START_X_LINE,3,ICON_72X24_ARR_STOP,1);
               //send data to devices
               RadioCmdSetWithNoRespon(RADIO_PRO_CMD_GROUP_SUBDUE_STOP,mMenuModeObject.devicesId, mMenuModeObject.groudId);
               Lcd_set_font(36, 24, 1);
               Disp_icon(START_X_TIP,3,ICON_36X24_STOP,1);
               Task_sleep(1000*CLOCK_UNIT_MS);

               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);
               Lcd_set_font(72, 24, 1);
               Disp_icon(START_X_LINE,3,ICON_72X24_ARR_SUBDUE,1);

               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;

           }
           //mMenuModeObject.numEnter = 0;

       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }
}
static void menu_tik_group_subdue()
{
    //uint8_t numbuff[10] = {0};

//TAB_REPEAT_ADD_ARR13:
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       //if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_GROUP_SUBDUE,1);
           Lcd_set_font(8, 24, 1);
           Disp_msg(START_X_NUM,1,"-----",FONT_8X24);
           Disp_msg(START_X_NUM,2,"-----",FONT_8X24);
       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_TICK )
       {
           if(mMenuModeObject.selectIndex == 0)
           {
               mMenuModeObject.selectIndex = 1;
           }
           else if(mMenuModeObject.selectIndex == 1)
           {
               Lcd_set_font(36, 24, 1);
               //send data to devices
               RadioCmdSetWithNoRespon(RADIO_PRO_CMD_ALL_SUBDUE_START,mMenuModeObject.devicesId, mMenuModeObject.groudId);
               Disp_icon(START_X_TIP,3,ICON_36X24_ALL_SHOCK,1);
               Task_sleep(1000*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);
               mMenuModeObject.selectIndex = 0;
           }

       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }
}

void Menu_low_power_display(uint32_t devicesId)
{
    uint8_t numbuff[10] = {0};
    Lcd_set_font(72, 24, 1);
    Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
    Lcd_set_font(8, 24, 1);
    Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
    Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);
    Disp_msg(START_X_NUM,1,"     ",FONT_8X24);
    Disp_msg(START_X_NUM,2,"     ",FONT_8X24);
    sprintf((char*)numbuff,"%d",devicesId);
    Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
    Lcd_set_font(72, 24, 1);
    Disp_icon(START_X_LINE,2,ICON_72X24_LOW_POWER,1);
    //Disp_icon(START_X_LINE,3,ICON_72X24_CLEAR,1);
}
void Menu_not_wearing_well_display(uint32_t devicesId)
{
    uint8_t numbuff[10] = {0};
    Lcd_set_font(72, 24, 1);
    Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
    Lcd_set_font(8, 24, 1);
    Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
    Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);
    Disp_msg(START_X_NUM,1,"     ",FONT_8X24);
    Disp_msg(START_X_NUM,2,"     ",FONT_8X24);
    sprintf((char*)numbuff,"%d",devicesId);
    Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
    Lcd_set_font(72, 24, 1);
    Disp_icon(START_X_LINE,2,ICON_72X24_NOT_WEARING_WELL,1);
    //Disp_icon(START_X_LINE,3,ICON_72X24_CLEAR,1);

}

void Menu_term_is_destroyed(uint32_t devicesId)
{
    uint8_t numbuff[10] = {0};
    //Lcd_clear_screen();
    Lcd_set_font(72, 24, 1);
    Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
    Lcd_set_font(8, 24, 1);
    Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
    Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);
    Disp_msg(START_X_NUM,1,"     ",FONT_8X24);
    Disp_msg(START_X_NUM,2,"     ",FONT_8X24);
    sprintf((char*)numbuff,"%d",devicesId);
    Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
    Lcd_set_font(72, 24, 1);
    Disp_icon(START_X_LINE,2,ICON_72X24_DESTORYED,1);
    //Disp_icon(START_X_LINE,3,ICON_72X24_CLEAR,1);

}
static void Display_alarm(uint32_t devicesId, uint8_t type)
{

    uint8_t numbuff[10] = {0};
    //Lcd_clear_screen();
    Lcd_set_font(72, 24, 1);
    Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
    Lcd_set_font(8, 24, 1);
    Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
    Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);
    Disp_msg(START_X_NUM,1,"     ",FONT_8X24);
    Disp_msg(START_X_NUM,2,"     ",FONT_8X24);
    sprintf((char*)numbuff,"%d",devicesId);
    Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
    Lcd_set_font(72, 24, 1);
    switch(type)
    {
    case ALARM_TYPE_LOW_POWER:
           Disp_icon(START_X_LINE,2,ICON_72X24_LOW_POWER,1);
          break;
    case ALARM_TYPE_UNWEAR:
           Disp_icon(START_X_LINE,2,ICON_72X24_NOT_WEARING_WELL,1);
          break;
    case ALARM_TYPE_DESTORY:
           Disp_icon(START_X_LINE,2,ICON_72X24_DESTORYED,1);
          break;
    case ALARM_TYPE_ESCAPE:
           Disp_icon(START_X_LINE,2,ICON_72X24_ESCAPE_ALARM,1);
          break;
    }

}
void Menu_term_is_no_arm()
{

    //Lcd_clear_screen();
    Lcd_set_font(72, 24, 1);
    Disp_icon(START_X_LINE,1,ICON_72X24_CLEAR,1);
    Disp_icon(START_X_LINE,2,ICON_72X24_CLEAR,1);
    Lcd_set_font(8, 24, 1);
    Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
    Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);
    Disp_msg(START_X_NUM,1,"     ",FONT_8X24);
    Disp_msg(START_X_NUM,2,"     ",FONT_8X24);

    Lcd_set_font(36, 24, 1);
    Disp_icon(START_X_LINE,1,ICON_36X24_NO_ALARM,1);
    Lcd_set_font(72, 24, 1);

    //Disp_icon(START_X_LINE,3,ICON_72X24_CLEAR,1);

}

static void menu_query_alarm_info()
{
    uint32_t deviceIdHex = 0;
    uint8_t i = 0;
    uint8_t buff[10] = {0};
    if(mMenuModeObject.keyDoing == KEY_DOING_DISPLAY)
    {
        MenuAlarmObjectTemp.alarmType = 0;
        MenuAlarmObjectTemp.devicesId = 0;

        MenuAlarmObjectTemp.alarmType =   mMenuAlarmObject[0].alarmType;
        MenuAlarmObjectTemp.devicesId =   mMenuAlarmObject[0].devicesId;
        Lcd_set_font(72, 24, 1);
        Disp_icon(START_X_LINE,3,ICON_72X24_ALARM_INFO_NUM,1);
        if(MenuAlarmObjectTemp.alarmType != 0 &&  MenuAlarmObjectTemp.devicesId != 0)
          {
             deviceIdHex = TransHexToInt(MenuAlarmObjectTemp.devicesId);

             Display_alarm(deviceIdHex,MenuAlarmObjectTemp.alarmType);

          }
          else
          {
             Menu_term_is_no_arm();
          }
    }
    else if(mMenuModeObject.keyDoing == KEY_DOING_ACK)
    {

        if(MenuAlarmObjectTemp.alarmType != 0 && MenuAlarmObjectTemp.devicesId != 0)
        {
            MenuAlarmObjectTemp.alarmType = 0;
            MenuAlarmObjectTemp.devicesId  = 0;

            mMenuAlarmObject[0].alarmType = 0;
            mMenuAlarmObject[0].devicesId = 0;

            for(i = 0 ; i < ALARM_COUNT_MAX-1;i++)
            {
                mMenuAlarmObject[i].alarmType = mMenuAlarmObject[i+1].alarmType;
                mMenuAlarmObject[i].devicesId = mMenuAlarmObject[i+1].devicesId;
            }

            MenuAlarmObjectTemp.alarmType =   mMenuAlarmObject[0].alarmType;
            MenuAlarmObjectTemp.devicesId =   mMenuAlarmObject[0].devicesId;
        }


        if(MenuAlarmObjectTemp.alarmType != 0 &&  MenuAlarmObjectTemp.devicesId != 0)
        {
            deviceIdHex = TransHexToInt(MenuAlarmObjectTemp.devicesId);
            Display_alarm(deviceIdHex,MenuAlarmObjectTemp.alarmType);
        }
        else
        {
             Menu_term_is_no_arm();
        }

    }
    Lcd_set_font(8, 24, 1);
    sprintf(buff,"%d",get_alarm_count());
    Disp_msg(START_X_XIN-1,3,buff,FONT_8X24);
    mMenuModeObject.keyDoing = KEY_DOING_NULL;
}

static void menu_setting_time( )
{
    uint8_t numbuff[10] = {0};

 TAB_REPEAT_ADD_ARR30:
    if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
    {
        if(mMenuModeObject.selectIndex > 5)
        {
            return;
        }

        Lcd_set_font(8, 24, 1);
        Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
        Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);
        Disp_icon(START_X_XIN-1,3,ICON_8X24_DISPLAY_CLEAR,1);
        Disp_icon(START_X_XIN,mMenuModeObject.selectIndex%2+1,ICON_8X24_ARROW,1);
        switch(mMenuModeObject.selectIndex)
        {
        case 0:
            if(!mMenuModeObject.timerefesh)
            {
                Lcd_set_font(72, 24, 1);
                Disp_icon(START_X_LINE,1,ICON_72X24_CLEAR,1);
                Lcd_set_font(36, 24, 1);
                Disp_icon(START_X_LINE,1,ICON_36X24_YEAR,1);
                Lcd_set_font(72, 24, 1);
                Disp_icon(START_X_LINE,2,ICON_72X24_CLEAR,1);
                Lcd_set_font(36, 24, 1);
                Disp_icon(START_X_LINE,2,ICON_36X24_MOTH,1);
                sprintf((char*)numbuff,"%s","     ");
                Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
                Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);

                Lcd_set_font(72, 24, 1);
                Disp_icon(START_X_LINE,3,ICON_72X24_TIME_SETTING,1);
                Lcd_set_font(8, 24, 1);
                sprintf((char*)numbuff,"%d",calendar.Month);
                Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);

                mMenuModeObject.timerefesh = 1;
            }
            if(mMenuModeObject.numEnter > 2100/* || mMenuModeObject.numEnter < 2020*/)
                mMenuModeObject.numEnter= mMenuModeObject.numEnter/10;
            calendar.Year = mMenuModeObject.numEnter;
            break;
        case 1:
            Lcd_set_font(8, 24, 1);
            sprintf((char*)numbuff,"%d",calendar.Year);
            Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
            if(mMenuModeObject.numEnter > 12)
                mMenuModeObject.numEnter= mMenuModeObject.numEnter/10;

            calendar.Month = mMenuModeObject.numEnter;
            break;
        case 2:
            if(!mMenuModeObject.timerefesh)
            {
                Lcd_set_font(72, 24, 1);
                Disp_icon(START_X_LINE,1,ICON_72X24_CLEAR,1);
                Lcd_set_font(36, 24, 1);
                Disp_icon(START_X_LINE,1,ICON_36X24_DATE,1);
                Lcd_set_font(72, 24, 1);
                Disp_icon(START_X_LINE,2,ICON_72X24_CLEAR,1);
                Lcd_set_font(36, 24, 1);
                Disp_icon(START_X_LINE,2,ICON_36X24_HONOR,1);
                sprintf((char*)numbuff,"%s","     ");
                Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
                Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
                mMenuModeObject.timerefesh = 1;

                Lcd_set_font(8, 24, 1);
                sprintf((char*)numbuff,"%d",calendar.Hours);
                Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
            }
            if(mMenuModeObject.numEnter > 31)
                mMenuModeObject.numEnter= mMenuModeObject.numEnter/10;
            calendar.DayOfMonth = mMenuModeObject.numEnter;
            break;
        case 3:
            Lcd_set_font(8, 16, 1);
            sprintf((char*)numbuff,"%d",calendar.DayOfMonth);
            Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
            if(mMenuModeObject.numEnter > 23)
                mMenuModeObject.numEnter= mMenuModeObject.numEnter/10;

            calendar.Hours = mMenuModeObject.numEnter;
            break;
        case 4:
            if(!mMenuModeObject.timerefesh)
            {
                Lcd_set_font(72, 24, 1);
                Disp_icon(START_X_LINE,1,ICON_72X24_CLEAR,1);
                Lcd_set_font(36, 24, 1);
                Disp_icon(START_X_LINE,1,ICON_36X24_MINUTE,1);
                Lcd_set_font(72, 24, 1);
                Disp_icon(START_X_LINE,2,ICON_72X24_CLEAR,1);
                Lcd_set_font(36, 24, 1);
                Disp_icon(START_X_LINE,2,ICON_36X24_SECOND,1);
                sprintf((char*)numbuff,"%s","     ");
                Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
                Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);

                Lcd_set_font(8, 24, 1);
                sprintf((char*)numbuff,"%d",calendar.Seconds);
                Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);

                mMenuModeObject.timerefesh = 1;
            }
            if(mMenuModeObject.numEnter > 60)
                mMenuModeObject.numEnter= mMenuModeObject.numEnter/10;
            calendar.Minutes = mMenuModeObject.numEnter;
            break;
        case 5:
            Lcd_set_font(8, 16, 1);
            sprintf((char*)numbuff,"%d",calendar.Minutes);
            Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
            if(mMenuModeObject.numEnter > 60)
                mMenuModeObject.numEnter= mMenuModeObject.numEnter/10;
            calendar.Seconds = mMenuModeObject.numEnter;
            break;
        case 6:
            break;

        }
        Lcd_set_font(8, 24, 1);
        if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
        {
          sprintf((char*)numbuff,"%s","     ");
          Disp_msg(START_X_NUM,mMenuModeObject.selectIndex%2+1,numbuff,FONT_8X24);
        }

        if(mMenuModeObject.numEnter != 0)
        {
            sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
            Disp_msg(START_X_NUM,mMenuModeObject.selectIndex%2+1,numbuff,FONT_8X24);

        }



    }
    else
    {
        if(mMenuModeObject.keyDoing == KEY_DOING_SWITCH)
        {
            if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.numEnter !=0)
            {
                calendar.Year = mMenuModeObject.numEnter;
                Lcd_set_font(36, 24, 1);
                mMenuModeObject.selectIndex = 1;
                mMenuModeObject.numEnter = calendar.Month;
                //Lcd_clear_screen();

                mMenuModeObject.keyDoing = KEY_DOING_NULL;
                goto TAB_REPEAT_ADD_ARR30;

            }
            else if(mMenuModeObject.selectIndex == 1 && mMenuModeObject.numEnter !=0)
            {
                calendar.Month = mMenuModeObject.numEnter;
                mMenuModeObject.numEnter = calendar.DayOfMonth;
                mMenuModeObject.selectIndex =2;
                //Lcd_clear_screen();

                mMenuModeObject.timerefesh = 0;
                mMenuModeObject.keyDoing = KEY_DOING_NULL;
                goto TAB_REPEAT_ADD_ARR30;
            }
            else if(mMenuModeObject.selectIndex == 2 && mMenuModeObject.numEnter !=0)
            {
                calendar.DayOfMonth = mMenuModeObject.numEnter;
                mMenuModeObject.numEnter = calendar.Hours;
                mMenuModeObject.selectIndex =3;
                //Lcd_clear_screen();
                mMenuModeObject.keyDoing = KEY_DOING_NULL;
                goto TAB_REPEAT_ADD_ARR30;
            }
            else if(mMenuModeObject.selectIndex == 3 && mMenuModeObject.numEnter !=0)
            {
                calendar.Hours = mMenuModeObject.numEnter;
                mMenuModeObject.numEnter = calendar.Minutes;
                mMenuModeObject.selectIndex =4;
                //Lcd_clear_screen();

                mMenuModeObject.timerefesh = 0;
                mMenuModeObject.keyDoing = KEY_DOING_NULL;
                goto TAB_REPEAT_ADD_ARR30;
            }
            else if(mMenuModeObject.selectIndex == 4 && mMenuModeObject.numEnter !=0)
            {
                calendar.Minutes = mMenuModeObject.numEnter;

                mMenuModeObject.numEnter = calendar.Seconds;
                mMenuModeObject.selectIndex =5;
                mMenuModeObject.keyDoing = KEY_DOING_NULL;
                goto TAB_REPEAT_ADD_ARR30;
            }
            else if(mMenuModeObject.selectIndex == 5 && mMenuModeObject.numEnter !=0)
            {
                calendar.Seconds = mMenuModeObject.numEnter;
                mMenuModeObject.numEnter = calendar.Year;
                mMenuModeObject.selectIndex =0;
                mMenuModeObject.keyDoing = KEY_DOING_NULL;
                mMenuModeObject.timerefesh = 0;
                goto TAB_REPEAT_ADD_ARR30;
            }
            else if(mMenuModeObject.selectIndex == 6)
            {
                mMenuModeObject.selectIndex =0;
                mMenuModeObject.numEnter = 0;
                mMenuModeObject.keyDoing = KEY_DOING_NULL;
                goto TAB_REPEAT_ADD_ARR30;
            }
        }
        else if(mMenuModeObject.keyDoing == KEY_DOING_ACK)
        {
            Lcd_set_font(36, 24, 1);

            if(calendar.Year >=2000)
            {
                Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
                Rtc_set_calendar(&calendar);
                Disp_proc();
                Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
                Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
                Lcd_set_font(36, 24, 1);
                Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);
            }
            else
            {
                Lcd_set_font(36, 24, 1);
                Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);
                Task_sleep(DELAY_COMPLETE*CLOCK_UNIT_MS);
                Lcd_set_font(36, 24, 1);
                Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);

                mMenuModeObject.numEnter = calendar.Year;
                mMenuModeObject.selectIndex =0;
                mMenuModeObject.timerefesh = 0;
                mMenuModeObject.keyDoing = KEY_DOING_NULL;
            }
        }
        mMenuModeObject.keyDoing = KEY_DOING_NULL;
    }

}

#endif
