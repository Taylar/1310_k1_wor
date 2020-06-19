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
#define START_X_NUM  8
#define START_X_XIN  120
#define START_X_TIP  108

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
    MENU_ITEN_CTROL_CLOSE_BLOCKING,
    MENU_ITEN_CTROL_OPEN_BLOCKING,
    MENU_ITEN_CTROL_OPEN_PREVENT_ESCAPE,
    MENU_ITEN_CTROL_CLOSE_PREVENT_ESCAPE,
    MENU_ITEN_CTROL_POWER_SELECT_HIGH,
    MENU_ITEN_CTROL_POWER_SELECT_MID,
    MENU_ITEN_CTROL_POWER_SELECT_LOW,
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
static void menu_close_blocking( );
static void menu_open_blocking( );
static void menu_open_prevent_escape( );
static void menu_close_prevent_escape( );
static void menu_power_select_high( );
static void menu_power_select_mid( );
static void menu_power_select_low( );
static void menu_setting_time( );


static void menu_tik_fixed_number_subdue();
static void menu_tik_group_subdue();
static void menu_tik_arr_subdue();
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
    {MENU_ITEN_CTROL_CLOSE_BLOCKING,NULL,menu_close_blocking},
    {MENU_ITEN_CTROL_OPEN_BLOCKING,NULL,menu_open_blocking},
    {MENU_ITEN_CTROL_OPEN_PREVENT_ESCAPE,NULL,menu_open_prevent_escape},
    {MENU_ITEN_CTROL_CLOSE_PREVENT_ESCAPE,NULL,menu_close_prevent_escape},
    {MENU_ITEN_CTROL_POWER_SELECT_HIGH,NULL,menu_power_select_high},
    {MENU_ITEN_CTROL_POWER_SELECT_MID,NULL,menu_power_select_mid},
    {MENU_ITEN_CTROL_POWER_SELECT_LOW,NULL,menu_power_select_low},
    {MENU_ITEN_SETTING_TIME,NULL,menu_setting_time},
    {MENU_ITEM_TIK_GROUP_SUBDUE,NULL,menu_tik_group_subdue},
    {MENU_ITEM_TIK_FIXED_NUMBER_SUBDUE,NULL,menu_tik_fixed_number_subdue},
    {MENU_ITEM_TIK_ARR_SUBDUE,NULL,menu_tik_arr_subdue},

};

MenuModeObject mMenuModeObject;
static Calendar calendar;
static void menuModeObject_data_reinit(void)
{
    mMenuModeObject.devicesId = 0;
    mMenuModeObject.groudId = 0;
    mMenuModeObject.keyDoing = KEY_DOING_NULL;
    mMenuModeObject.selectIndex = 0;
    mMenuModeObject.numEnter = 0;
    Lcd_clear_screen();
}
void menuc_main(KEY_CODE_E keyCode)
{
    KEY_CODE_E keyCodeIn = keyCode;
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
            mMenuModeObject.index++;
          if(mMenuModeObject.index >= MENU_ITEM_TIK_GROUP_SUBDUE)
             mMenuModeObject.index= (MENU_ITEN_NULL+1);

            menuModeObject_data_reinit();
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
    }
    if(mMenuModeObject.index !=MENU_ITEN_NULL)
    {
        if(mMenuModeObject.index == MENU_ITEN_SETTING_TIME && keyCodeIn == _VK_MODE)
           memset((char*)&calendar,0x00,sizeof(Calendar));

           MenuMode[mMenuModeObject.index-1].func();
    }
}

void menu_add_group( )
{
    uint8_t numbuff[10] = {0};

TAB_REPEAT_ADD_ARR:
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_ADD_ARR,1);
           Lcd_set_font(8, 24, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
             sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);
           Lcd_set_font(8, 24, 1);
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
       }
       else if(mMenuModeObject.selectIndex == 1)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);

           //Disp_icon(0,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_ADD_ARR,1);

           Lcd_set_font(8, 24, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
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
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.numEnter !=0)
           {
               mMenuModeObject.devicesId = mMenuModeObject.numEnter;
               mMenuModeObject.selectIndex =1;

               Lcd_set_font(8, 24, 1);
               Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
               Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
               Lcd_set_font(8, 16, 1);

           }
           else if(mMenuModeObject.selectIndex == 1 && mMenuModeObject.numEnter !=0)
           {
               mMenuModeObject.groudId = mMenuModeObject.numEnter;
               Lcd_set_font(8, 24, 1);
               Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
               Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
               Lcd_set_font(8, 16, 1);
               //调用发射窗口
               //display complete
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithRespon(RADIO_PRO_CMD_TERM_ADD_TO_GROUP,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);
               //Lcd_clear_screen();
               //
               mMenuModeObject.selectIndex = 2;
           }
           else if(mMenuModeObject.selectIndex == 2)
           {
               mMenuModeObject.groudId =   0;
               mMenuModeObject.numEnter =  0;
               mMenuModeObject.devicesId = 0;
               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               Lcd_clear_screen();
               goto TAB_REPEAT_ADD_ARR;
           }
           mMenuModeObject.numEnter = 0;
           mMenuModeObject.keyDoing = KEY_DOING_NULL;
       }
   }


}

static void menu_delete_group( )
{
    uint8_t numbuff[10] = {0};

TAB_REPEAT_ADD_ARR1:
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_DELETE_ARR,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
       }
       else if(mMenuModeObject.selectIndex == 1)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);

           //Disp_icon(0,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_DELETE_ARR,1);

           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.numEnter !=0)
           {
               mMenuModeObject.devicesId = mMenuModeObject.numEnter;
               mMenuModeObject.selectIndex =1;
               Lcd_set_font(8, 24, 1);
               Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
               Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
               Lcd_set_font(8, 16, 1);


           }
           else if(mMenuModeObject.selectIndex == 1 && mMenuModeObject.numEnter !=0)
           {
               mMenuModeObject.groudId = mMenuModeObject.numEnter;
               Lcd_set_font(8, 24, 1);
               Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
               Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
               Lcd_set_font(8, 16, 1);
               //调用发射窗口
               //display complete
               //Lcd_clear_screen();
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithRespon(RADIO_PRO_CMD_TERM_DELETE_FROM_GROUP,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);
               //
               mMenuModeObject.selectIndex = 2;
           }
           else if(mMenuModeObject.selectIndex == 2)
           {
               mMenuModeObject.groudId =   0;
               mMenuModeObject.numEnter =  0;
               mMenuModeObject.devicesId = 0;
               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               Lcd_clear_screen();
               goto TAB_REPEAT_ADD_ARR1;
           }
           mMenuModeObject.numEnter = 0;
           mMenuModeObject.keyDoing = KEY_DOING_NULL;
       }
   }

}
static void menu_terminal_test_group( )
{
    uint8_t numbuff[10] = {0};

TAB_REPEAT_ADD_ARR2:
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_TERMINAL_TEST,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
       }
       else if(mMenuModeObject.selectIndex == 1)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);

           //Disp_icon(0,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_TERMINAL_TEST,1);

           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.numEnter !=0)
           {
               mMenuModeObject.devicesId = mMenuModeObject.numEnter;
               mMenuModeObject.selectIndex =2;
               Lcd_set_font(8, 16, 1);
               //Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1)
               //Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithRespon(RADIO_PRO_CMD_TERM_TEST,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);
                              //
               mMenuModeObject.selectIndex = 2;

           }
           else if(mMenuModeObject.selectIndex == 1 && mMenuModeObject.numEnter !=0)
           {
               mMenuModeObject.groudId = mMenuModeObject.numEnter;
               Lcd_set_font(8, 24, 1);
               Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
               Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
               Lcd_set_font(8, 16, 1);
               //调用发射窗口
               //display complete
               //Lcd_clear_screen();
               RadioCmdSetWithNoRespon(RADIO_PRO_CMD_GROUP_TEST,mMenuModeObject.devicesId, mMenuModeObject.groudId);
               Task_sleep(1000*CLOCK_UNIT_MS);
               Lcd_set_font(36, 24, 1);
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               //
               mMenuModeObject.selectIndex = 2;
           }
           else if(mMenuModeObject.selectIndex == 2)
           {
               mMenuModeObject.groudId =   0;
               mMenuModeObject.numEnter =  0;
               mMenuModeObject.devicesId = 0;
               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               Lcd_clear_screen();
               goto TAB_REPEAT_ADD_ARR2;
           }
           mMenuModeObject.numEnter = 0;
           mMenuModeObject.keyDoing = KEY_DOING_NULL;
       }
   }
}
static void menu_close_ctrol()//关闭制服
{
    uint8_t numbuff[10] = {0};

TAB_REPEAT_ADD_ARR3:
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_CLOSE_CTROL,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
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
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithRespon(RADIO_PRO_CMD_TERM_CLOSE_CTROL,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);
               mMenuModeObject.selectIndex = 2;
           }
           else if(mMenuModeObject.selectIndex == 2)
           {
               mMenuModeObject.groudId =   0;
               mMenuModeObject.numEnter =  0;
               mMenuModeObject.devicesId = 0;
               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               Lcd_clear_screen();
               goto TAB_REPEAT_ADD_ARR3;
           }
           mMenuModeObject.numEnter = 0;
           mMenuModeObject.keyDoing = KEY_DOING_NULL;
       }
   }
}
static void menu_close_ctrol_group( )//关组制服
{
    uint8_t numbuff[10] = {0};

TAB_REPEAT_ADD_ARR4:
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_CLOSE_GROUP_CTROL,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.numEnter !=0)
           {
               mMenuModeObject.groudId = mMenuModeObject.numEnter;
               Lcd_set_font(36, 24, 1);
               //send data to devices
               RadioCmdSetWithNoRespon(RADIO_PRO_CMD_GROUP_CLOSE_CTROL,mMenuModeObject.devicesId, mMenuModeObject.groudId);  //群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               mMenuModeObject.selectIndex = 2;
           }
           else if(mMenuModeObject.selectIndex == 2)
           {
               mMenuModeObject.groudId =   0;
               mMenuModeObject.numEnter =  0;
               mMenuModeObject.devicesId = 0;
               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               Lcd_clear_screen();
               goto TAB_REPEAT_ADD_ARR4;
           }
           mMenuModeObject.numEnter = 0;
           mMenuModeObject.keyDoing = KEY_DOING_NULL;
       }
   }
}
static void menu_open_ctrol( )
{
    uint8_t numbuff[10] = {0};

TAB_REPEAT_ADD_ARR5:
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_OPEN_CTROL,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
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
               if(RadioCmdSetWithRespon(RADIO_PRO_CMD_TERM_OPEN_CTROL,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);
               mMenuModeObject.selectIndex = 2;
           }
           else if(mMenuModeObject.selectIndex == 2)
           {
               mMenuModeObject.groudId =   0;
               mMenuModeObject.numEnter =  0;
               mMenuModeObject.devicesId = 0;
               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               Lcd_clear_screen();
               goto TAB_REPEAT_ADD_ARR5;
           }
           mMenuModeObject.numEnter = 0;
           mMenuModeObject.keyDoing = KEY_DOING_NULL;
       }
   }

}
static void menu_open_ctrol_group( )
{
    uint8_t numbuff[10] = {0};

TAB_REPEAT_ADD_ARR6:
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_OPEN_GROUP_CTROL,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.numEnter !=0)
           {
               mMenuModeObject.groudId = mMenuModeObject.numEnter;
               Lcd_set_font(36, 24, 1);
               //send data to devices
               RadioCmdSetWithNoRespon(RADIO_PRO_CMD_GROUP_OPEN_CTROL,mMenuModeObject.devicesId, mMenuModeObject.groudId);  //群组
               //Task_sleep(1000*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               mMenuModeObject.selectIndex = 2;
           }
           else if(mMenuModeObject.selectIndex == 2)
           {
               mMenuModeObject.groudId =   0;
               mMenuModeObject.numEnter =  0;
               mMenuModeObject.devicesId = 0;
               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               Lcd_clear_screen();
               goto TAB_REPEAT_ADD_ARR6;
           }
           mMenuModeObject.numEnter = 0;
           mMenuModeObject.keyDoing = KEY_DOING_NULL;
       }
   }
}
static void menu_terminal_unlocking()
{
    uint8_t numbuff[10] = {0};

TAB_REPEAT_ADD_ARR7:
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_TERMINAL_UNLOCKING,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
       }
       else if(mMenuModeObject.selectIndex == 1)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);

           Disp_icon(0,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_TERMINAL_UNLOCKING,1);

           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.numEnter !=0)
           {
               mMenuModeObject.devicesId = mMenuModeObject.numEnter;
               //mMenuModeObject.selectIndex =1;
               //Lcd_set_font(8, 16, 1);
               //Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1)
               //Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
               //调用发射窗口
               //display complete
               //Lcd_clear_screen();
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithRespon(RADIO_PRO_CMD_TERM_UNLOCKING,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);
               mMenuModeObject.selectIndex = 2;

           }
           else if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.numEnter ==0)
           {
               mMenuModeObject.selectIndex =1;
               mMenuModeObject.devicesId = 0;
               mMenuModeObject.groudId = 0;
               Lcd_clear_screen();
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               goto TAB_REPEAT_ADD_ARR7;
           }
           else if(mMenuModeObject.selectIndex == 1 && mMenuModeObject.numEnter !=0)
           {
               mMenuModeObject.groudId = mMenuModeObject.numEnter;
               Lcd_set_font(8, 16, 1);
               //Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1)
               //Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
               //调用发射窗口
               //display complete
               //Lcd_clear_screen();
               RadioCmdSetWithNoRespon(RADIO_PRO_CMD_GROUP_UNLOCKING,mMenuModeObject.devicesId, mMenuModeObject.groudId);  //群组
               //Task_sleep(1000*CLOCK_UNIT_MS);
               Lcd_set_font(36, 24, 1);
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               //
               mMenuModeObject.selectIndex = 2;
           }
           else if(mMenuModeObject.selectIndex == 1 && mMenuModeObject.numEnter ==0)
           {
               mMenuModeObject.selectIndex =0;
               mMenuModeObject.devicesId = 0;
               mMenuModeObject.groudId = 0;
               Lcd_clear_screen();
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               goto TAB_REPEAT_ADD_ARR7;
           }
           else if(mMenuModeObject.selectIndex == 2)
           {
               mMenuModeObject.groudId =   0;
               mMenuModeObject.numEnter =  0;
               mMenuModeObject.devicesId = 0;
               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               Lcd_clear_screen();
               goto TAB_REPEAT_ADD_ARR7;
           }
           mMenuModeObject.numEnter = 0;
           mMenuModeObject.keyDoing = KEY_DOING_NULL;
       }
   }


}

static void menu_close_blocking( )
{
    uint8_t numbuff[10] = {0};

TAB_REPEAT_ADD_ARR21:
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_CLOSE_BLOCKING,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
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
               if(RadioCmdSetWithRespon(RADIO_PRO_CMD_TERM_CLOSE_BLOCKING,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);
               mMenuModeObject.selectIndex = 2;
           }
           else if(mMenuModeObject.selectIndex == 2)
           {
               mMenuModeObject.groudId =   0;
               mMenuModeObject.numEnter =  0;
               mMenuModeObject.devicesId = 0;
               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               Lcd_clear_screen();
               goto TAB_REPEAT_ADD_ARR21;
           }
           mMenuModeObject.numEnter = 0;
           mMenuModeObject.keyDoing = KEY_DOING_NULL;
       }
   }



}

static void menu_open_blocking( )
{

    uint8_t numbuff[10] = {0};

TAB_REPEAT_ADD_ARR20:
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_OPEN_BLOCKING,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
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
               if(RadioCmdSetWithRespon(RADIO_PRO_CMD_TERM_OPEN_BLOCKING,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);
               mMenuModeObject.selectIndex = 2;
           }
           else if(mMenuModeObject.selectIndex == 2)
           {
               mMenuModeObject.groudId =   0;
               mMenuModeObject.numEnter =  0;
               mMenuModeObject.devicesId = 0;
               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               Lcd_clear_screen();
               goto TAB_REPEAT_ADD_ARR20;
           }
           mMenuModeObject.numEnter = 0;
           mMenuModeObject.keyDoing = KEY_DOING_NULL;
       }
   }

}
static void menu_open_prevent_escape( )
{

   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_ALL_FIBID_ESCAPE,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_NOT_NEED_INPUT,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_OPEN_ESCAPE,1);
           Lcd_set_font(8, 16, 1);
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {

           {

               if(RadioCmdSetWithRespon(RADIO_PRO_CMD_OPEN_PREVENT_ESCAPE,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);
               mMenuModeObject.selectIndex = 2;
           }
           mMenuModeObject.numEnter = 0;
           mMenuModeObject.keyDoing = KEY_DOING_NULL;
       }
   }

}
static void menu_close_prevent_escape( )
{
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_ALL_FIBID_ESCAPE,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_NOT_NEED_INPUT,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_CLOSE_ESCAPE,1);
           Lcd_set_font(8, 16, 1);
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {

           {

               if(RadioCmdSetWithRespon(RADIO_PRO_CMD_CLOSE_PREVENT_ESCAPE,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);
               mMenuModeObject.selectIndex = 2;
           }
           mMenuModeObject.numEnter = 0;
           mMenuModeObject.keyDoing = KEY_DOING_NULL;
       }
   }

}
static void menu_power_select_high()
{
    uint8_t numbuff[10] = {0};

TAB_REPEAT_ADD_ARR8:
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_POWER_HIGH,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
       }
       else if(mMenuModeObject.selectIndex == 1)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);

           Disp_icon(0,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_POWER_HIGH,1);

           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.numEnter !=0)
           {
               mMenuModeObject.devicesId = mMenuModeObject.numEnter;
               //mMenuModeObject.selectIndex =1;
               //Lcd_set_font(8, 16, 1);
               //Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1)
               //Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
               //调用发射窗口
               //display complete
               //Lcd_clear_screen();
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithRespon(RADIO_PRO_CMD_TERM_POWER_HIGH,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);
               mMenuModeObject.selectIndex = 2;

           }
           else if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.numEnter ==0)
           {
               mMenuModeObject.selectIndex =1;
               mMenuModeObject.devicesId = 0;
               mMenuModeObject.groudId = 0;
               Lcd_clear_screen();
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               goto TAB_REPEAT_ADD_ARR8;
           }
           else if(mMenuModeObject.selectIndex == 1 && mMenuModeObject.numEnter !=0)
           {
               mMenuModeObject.groudId = mMenuModeObject.numEnter;
               Lcd_set_font(8, 16, 1);
               //Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1)
               //Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
               //调用发射窗口
               //display complete
               //Lcd_clear_screen();
               RadioCmdSetWithNoRespon(RADIO_PRO_CMD_GROUP_POWER_HIGH,mMenuModeObject.devicesId, mMenuModeObject.groudId);
               //Task_sleep(1000*CLOCK_UNIT_MS);
               Lcd_set_font(36, 24, 1);
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               //
               mMenuModeObject.selectIndex = 2;
           }
           else if(mMenuModeObject.selectIndex == 1 && mMenuModeObject.numEnter ==0)
           {
               mMenuModeObject.selectIndex =0;
               mMenuModeObject.devicesId = 0;
               mMenuModeObject.groudId = 0;
               Lcd_clear_screen();
               goto TAB_REPEAT_ADD_ARR8;
           }
           else if(mMenuModeObject.selectIndex == 2)
           {
               mMenuModeObject.groudId =   0;
               mMenuModeObject.numEnter =  0;
               mMenuModeObject.devicesId = 0;
               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               Lcd_clear_screen();
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               goto TAB_REPEAT_ADD_ARR8;
           }
           mMenuModeObject.numEnter = 0;
           mMenuModeObject.keyDoing = KEY_DOING_NULL;
       }
   }

}
static void menu_power_select_mid()
{
    uint8_t numbuff[10] = {0};

TAB_REPEAT_ADD_ARR9:
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_POWER_MID,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
       }
       else if(mMenuModeObject.selectIndex == 1)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);

           Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_POWER_MID,1);

           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.numEnter !=0)
           {
               mMenuModeObject.devicesId = mMenuModeObject.numEnter;
               //mMenuModeObject.selectIndex =1;
               //Lcd_set_font(8, 16, 1);
               //Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1)
               //Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
               //调用发射窗口
               //display complete
               //Lcd_clear_screen();
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithRespon(RADIO_PRO_CMD_TERM_POWER_MID,mMenuModeObject.devicesId, mMenuModeObject.groudId))
                  Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               else
                   Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);



               mMenuModeObject.selectIndex = 2;

           }
           else if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.numEnter ==0)
           {
               mMenuModeObject.selectIndex =1;
               mMenuModeObject.devicesId = 0;
               mMenuModeObject.groudId = 0;
               Lcd_clear_screen();
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               goto TAB_REPEAT_ADD_ARR9;
           }
           else if(mMenuModeObject.selectIndex == 1 && mMenuModeObject.numEnter !=0)
           {
               mMenuModeObject.groudId = mMenuModeObject.numEnter;
               Lcd_set_font(8, 16, 1);
               //Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1)
               //Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
               //调用发射窗口
               //display complete
               //Lcd_clear_screen();
               RadioCmdSetWithNoRespon(RADIO_PRO_CMD_GROUP_POWER_MID,mMenuModeObject.devicesId, mMenuModeObject.groudId);

               Lcd_set_font(36, 24, 1);
               Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
               //
               mMenuModeObject.selectIndex = 2;
           }
           else if(mMenuModeObject.selectIndex == 1 && mMenuModeObject.numEnter ==0)
           {
               mMenuModeObject.selectIndex =0;
               mMenuModeObject.devicesId = 0;
               mMenuModeObject.groudId = 0;
               Lcd_clear_screen();
               goto TAB_REPEAT_ADD_ARR9;
           }
           else if(mMenuModeObject.selectIndex == 2)
           {
               mMenuModeObject.groudId =   0;
               mMenuModeObject.numEnter =  0;
               mMenuModeObject.devicesId = 0;
               mMenuModeObject.selectIndex = 0;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               mMenuModeObject.keyDoing = KEY_DOING_NULL;
               Lcd_clear_screen();
               goto TAB_REPEAT_ADD_ARR9;
           }
           mMenuModeObject.numEnter = 0;
           mMenuModeObject.keyDoing = KEY_DOING_NULL;
       }
   }

}
static void menu_power_select_low()
{
    uint8_t numbuff[10] = {0};

 TAB_REPEAT_ADD_ARR10:
    if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
    {

        Lcd_set_font(72, 24, 1);
        if(mMenuModeObject.selectIndex == 0)
        {
            //numToBuff(numbuff,mMenuModeObject.numEnter);
            Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
            Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
            Disp_icon(START_X_LINE,3,ICON_72X24_POWER_LOW,1);
            Lcd_set_font(8, 16, 1);
            if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
            {
                sprintf((char*)numbuff,"%s","    ");
              Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
            }
            Lcd_set_font(8, 24, 1);
            Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
            Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);
            Lcd_set_font(8, 16, 1);
            if(mMenuModeObject.numEnter != 0)
            {

                sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
                Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
            }
        }
        else if(mMenuModeObject.selectIndex == 1)
        {
            //numToBuff(numbuff,mMenuModeObject.numEnter);

            Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
            Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
            Disp_icon(START_X_LINE,3,ICON_72X24_POWER_LOW,1);

            Lcd_set_font(8, 16, 1);
            if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
            {
                sprintf((char*)numbuff,"%s","    ");
              Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
            }
            Lcd_set_font(8, 24, 1);
            Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
            Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
            Lcd_set_font(8, 16, 1);
            if(mMenuModeObject.numEnter != 0)
            {

                sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
                Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
            }
        }
    }
    else
    {
        if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
        {
            if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.numEnter !=0)
            {
                mMenuModeObject.devicesId = mMenuModeObject.numEnter;
                //mMenuModeObject.selectIndex =1;
                //Lcd_set_font(8, 16, 1);
                //Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1)
                //Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
                //调用发射窗口
                //display complete
                //Lcd_clear_screen();
               // RadioCmdSetWithRespon(RADIO_PRO_CMD_TERM_POWER_LOW,mMenuModeObject.devicesId, mMenuModeObject.groudId);
                Lcd_set_font(36, 24, 1);
                if(RadioCmdSetWithRespon(RADIO_PRO_CMD_TERM_POWER_LOW,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
                Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
                else
                Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);
                mMenuModeObject.selectIndex = 2;

            }
            else if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.numEnter ==0)
            {
                mMenuModeObject.selectIndex =1;
                mMenuModeObject.devicesId = 0;
                mMenuModeObject.groudId = 0;
                Lcd_clear_screen();
                mMenuModeObject.keyDoing = KEY_DOING_NULL;
                goto TAB_REPEAT_ADD_ARR10;
            }
            else if(mMenuModeObject.selectIndex == 1 && mMenuModeObject.numEnter !=0)
            {
                mMenuModeObject.groudId = mMenuModeObject.numEnter;
                Lcd_set_font(8, 16, 1);
                //Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1)
                //Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
                //调用发射窗口
                //display complete
                //Lcd_clear_screen();
                RadioCmdSetWithNoRespon(RADIO_PRO_CMD_GROUP_POWER_LOW,mMenuModeObject.devicesId, mMenuModeObject.groudId);

                Lcd_set_font(36, 24, 1);
                Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
                //
                mMenuModeObject.selectIndex = 2;
            }
            else if(mMenuModeObject.selectIndex == 1 && mMenuModeObject.numEnter ==0)
            {
                mMenuModeObject.selectIndex =0;
                mMenuModeObject.devicesId = 0;
                mMenuModeObject.groudId = 0;
                Lcd_clear_screen();
                mMenuModeObject.keyDoing = KEY_DOING_NULL;
                goto TAB_REPEAT_ADD_ARR10;
            }
            else if(mMenuModeObject.selectIndex == 2)
            {
                mMenuModeObject.groudId =   0;
                mMenuModeObject.numEnter =  0;
                mMenuModeObject.devicesId = 0;
                mMenuModeObject.selectIndex = 0;
                mMenuModeObject.keyDoing = KEY_DOING_NULL;
                Lcd_clear_screen();
                goto TAB_REPEAT_ADD_ARR10;
            }
            mMenuModeObject.numEnter = 0;
            mMenuModeObject.keyDoing = KEY_DOING_NULL;
        }
    }


}

static void menu_tik_fixed_number_subdue()
{

    uint8_t numbuff[10] = {0};

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
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_ARROW,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_DISPLAY_CLEAR,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_TICK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.numEnter !=0)
           {
               mMenuModeObject.devicesId = mMenuModeObject.numEnter;
               Lcd_set_font(36, 24, 1);
               //send data to devices
               if(RadioCmdSetWithRespon(RADIO_PRO_CMD_FIXED_TERM_SUBDUE_START,mMenuModeObject.devicesId, mMenuModeObject.groudId))  //添加群组
               Disp_icon(START_X_TIP,3,ICON_36X24_SUBDUE,1);
               else
               Disp_icon(START_X_TIP,3,ICON_36X24_FAIL,1);

               Task_sleep(1000*CLOCK_UNIT_MS);
               Lcd_set_font(72, 24, 1);
               Disp_icon(START_X_LINE,3,ICON_72X24_FIXED_PEOPLE_STOP,1);
               Lcd_set_font(36, 24, 1);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);
               mMenuModeObject.selectIndex = 1;
           }
           else if(mMenuModeObject.selectIndex == 1)
           {
               Lcd_set_font(72, 24, 1);
               Disp_icon(START_X_LINE,3,ICON_72X24_FIXED_PEOPLE_STOP,1);
               //send data to devices
               Lcd_set_font(36, 24, 1);
               if(RadioCmdSetWithRespon(RADIO_PRO_CMD_FIXED_TERM_SUBDUE_STOP,mMenuModeObject.devicesId, mMenuModeObject.groudId))
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
           mMenuModeObject.keyDoing = KEY_DOING_NULL;
       }
   }
}
static void menu_tik_arr_subdue()
{
    uint8_t numbuff[10] = {0};

//TAB_REPEAT_ADD_ARR12:
   if((!mMenuModeObject.keyDoing) || (mMenuModeObject.keyDoing ==KEY_DOING_DELETE))
   {

       Lcd_set_font(72, 24, 1);
       //if(mMenuModeObject.selectIndex == 0)
       {
           //numToBuff(numbuff,mMenuModeObject.numEnter);
           Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
           Disp_icon(START_X_LINE,2,ICON_72X24_GROUP_NUM,1);
           Disp_icon(START_X_LINE,3,ICON_72X24_ARR_SUBDUE,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
           {
               sprintf((char*)numbuff,"%s","    ");
             Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
           }
           Lcd_set_font(8, 24, 1);
           Disp_icon(START_X_XIN,1,ICON_8X24_DISPLAY_CLEAR,1);
           Disp_icon(START_X_XIN,2,ICON_8X24_ARROW,1);
           Lcd_set_font(8, 16, 1);
           if(mMenuModeObject.numEnter != 0)
           {

               sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
               Disp_msg(START_X_NUM,2,numbuff,FONT_8X24);
           }
       }
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_TICK )
       {
           if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.numEnter !=0)
           {
               mMenuModeObject.devicesId = mMenuModeObject.numEnter;
               Lcd_set_font(36, 24, 1);
               //send data to devices
               RadioCmdSetWithNoRespon(RADIO_PRO_CMD_GROUP_SUBDUE_START,mMenuModeObject.devicesId, mMenuModeObject.groudId);
               Disp_icon(START_X_TIP,3,ICON_36X24_SUBDUE,1);
               Task_sleep(1000*CLOCK_UNIT_MS);
               Lcd_set_font(72, 24, 1);
               Disp_icon(START_X_LINE,3,ICON_72X24_ARR_STOP,1);
               Lcd_set_font(36, 24, 1);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);
               mMenuModeObject.selectIndex = 1;
           }
           else if(mMenuModeObject.selectIndex == 1)
           {
               Lcd_set_font(72, 24, 1);
               Disp_icon(START_X_LINE,3,ICON_72X24_ARR_STOP,1);
               //send data to devices
               RadioCmdSetWithRespon(RADIO_PRO_CMD_GROUP_SUBDUE_STOP,mMenuModeObject.devicesId, mMenuModeObject.groudId);
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
           mMenuModeObject.keyDoing = KEY_DOING_NULL;
       }
   }
}
static void menu_tik_group_subdue()
{
    //uint8_t numbuff[10] = {0};
    uint8_t numbuff[10] = {0};
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

       }
       mMenuModeObject.keyDoing = KEY_DOING_NULL;
   }
   else
   {
       if(mMenuModeObject.keyDoing == KEY_DOING_TICK )
       {
           if(mMenuModeObject.selectIndex == 0)
           {
               Lcd_set_font(36, 24, 1);
               //send data to devices
               RadioCmdSetWithNoRespon(RADIO_PRO_CMD_ALL_SUBDUE_START,mMenuModeObject.devicesId, mMenuModeObject.groudId);
               Disp_icon(START_X_TIP,3,ICON_36X24_ALL_SHOCK,1);
               Task_sleep(1000*CLOCK_UNIT_MS);
               Disp_icon(START_X_TIP,3,ICON_36X24_CLEAR,1);
               mMenuModeObject.selectIndex = 0;
           }
           mMenuModeObject.keyDoing = KEY_DOING_NULL;
       }
   }
}

void Menu_low_power_display(uint32_t devicesId)
{
    uint8_t numbuff[10] = {0};
    Lcd_clear_screen();
    Lcd_set_font(72, 24, 1);
    Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
    Lcd_set_font(8, 16, 1);
    sprintf((char*)numbuff,"%d",devicesId);
    Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
    Lcd_set_font(72, 24, 1);
    Disp_icon(START_X_LINE,2,ICON_72X24_LOW_POWER,1);

}
void Menu_not_wearing_well_display(uint32_t devicesId)
{
    uint8_t numbuff[10] = {0};
    Lcd_clear_screen();
    Lcd_set_font(72, 24, 1);
    Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
    Lcd_set_font(8, 16, 1);
    sprintf((char*)numbuff,"%d",devicesId);
    Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
    Lcd_set_font(72, 24, 1);
    Disp_icon(START_X_LINE,2,ICON_72X24_NOT_WEARING_WELL,1);

}

void Menu_term_is_destroyed(uint32_t devicesId)
{
    uint8_t numbuff[10] = {0};
    Lcd_clear_screen();
    Lcd_set_font(72, 24, 1);
    Disp_icon(START_X_LINE,1,ICON_72X24_TERMINAL_NUM,1);
    Lcd_set_font(8, 16, 1);
    sprintf((char*)numbuff,"%d",devicesId);
    Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
    Lcd_set_font(72, 24, 1);
    Disp_icon(START_X_LINE,2,ICON_72X24_DESTORYED,1);

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
        Disp_icon(START_X_XIN,mMenuModeObject.selectIndex%2+1,ICON_8X24_ARROW,1);



        switch(mMenuModeObject.selectIndex)
        {
        case 0:
            Lcd_set_font(36, 24, 1);
            Disp_icon(START_X_LINE,1,ICON_36X24_YEAR,1);
            Disp_icon(START_X_LINE,2,ICON_36X24_MOTH,1);
            break;
        case 1:
            Lcd_set_font(36, 24, 1);
            Disp_icon(START_X_LINE,1,ICON_36X24_YEAR,1);
            Disp_icon(START_X_LINE,2,ICON_36X24_MOTH,1);

            Lcd_set_font(8, 16, 1);
            sprintf((char*)numbuff,"%d",calendar.Year);
            Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
            break;
        case 2:
            Lcd_set_font(36, 24, 1);
            Disp_icon(START_X_LINE,1,ICON_36X24_DATE,1);
            Disp_icon(START_X_LINE,2,ICON_36X24_HONOR,1);
            break;
        case 3:
            Lcd_set_font(36, 24, 1);
            Disp_icon(START_X_LINE,1,ICON_36X24_DATE,1);
            Disp_icon(START_X_LINE,2,ICON_36X24_HONOR,1);

            Lcd_set_font(8, 16, 1);
            sprintf((char*)numbuff,"%d",calendar.DayOfMonth);
            Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
            break;
        case 4:
            Lcd_set_font(36, 24, 1);
            Disp_icon(START_X_LINE,1,ICON_36X24_MINUTE,1);
            Disp_icon(START_X_LINE,2,ICON_36X24_SECOND,1);
            break;
        case 5:
            Lcd_set_font(36, 24, 1);
            Disp_icon(START_X_LINE,1,ICON_36X24_MINUTE,1);
            Disp_icon(START_X_LINE,2,ICON_36X24_SECOND,1);
            Lcd_set_font(8, 16, 1);
            sprintf((char*)numbuff,"%d",calendar.Minutes);
            Disp_msg(START_X_NUM,1,numbuff,FONT_8X24);
            break;
        case 6:
            break;

        }
        Lcd_set_font(8, 16, 1);
        if(mMenuModeObject.keyDoing ==KEY_DOING_DELETE)
        {
          sprintf((char*)numbuff,"%s","    ");
          Disp_msg(START_X_NUM,mMenuModeObject.selectIndex%2+1,numbuff,FONT_8X24);
        }

        if(mMenuModeObject.numEnter != 0)
        {
            sprintf((char*)numbuff,"%d",mMenuModeObject.numEnter);
            Disp_msg(START_X_NUM,mMenuModeObject.selectIndex%2+1,numbuff,FONT_8X24);

        }

        Lcd_set_font(72, 24, 1);
        Disp_icon(START_X_LINE,3,ICON_72X24_TIME_SETTING,1);

    }
    else
    {
        if(mMenuModeObject.keyDoing == KEY_DOING_ACK )
        {
            if(mMenuModeObject.selectIndex == 0 && mMenuModeObject.numEnter !=0)
            {
                calendar.Year = mMenuModeObject.numEnter;
                Lcd_set_font(36, 24, 1);
                mMenuModeObject.selectIndex = 1;
                mMenuModeObject.numEnter = 0;
                Lcd_clear_screen();
                mMenuModeObject.keyDoing = KEY_DOING_NULL;
                goto TAB_REPEAT_ADD_ARR30;

            }
            else if(mMenuModeObject.selectIndex == 1 && mMenuModeObject.numEnter !=0)
            {
                calendar.Month = mMenuModeObject.numEnter;
                mMenuModeObject.numEnter = 0;
                mMenuModeObject.selectIndex =2;
                Lcd_clear_screen();
                mMenuModeObject.keyDoing = KEY_DOING_NULL;
                goto TAB_REPEAT_ADD_ARR30;
            }
            else if(mMenuModeObject.selectIndex == 2 && mMenuModeObject.numEnter !=0)
            {
                calendar.DayOfMonth = mMenuModeObject.numEnter;
                mMenuModeObject.numEnter = 0;
                mMenuModeObject.selectIndex =3;
                Lcd_clear_screen();
                mMenuModeObject.keyDoing = KEY_DOING_NULL;
                goto TAB_REPEAT_ADD_ARR30;
            }
            else if(mMenuModeObject.selectIndex == 3 && mMenuModeObject.numEnter !=0)
            {
                calendar.Hours = mMenuModeObject.numEnter;
                mMenuModeObject.numEnter = 0;
                mMenuModeObject.selectIndex =4;
                Lcd_clear_screen();
                mMenuModeObject.keyDoing = KEY_DOING_NULL;
                goto TAB_REPEAT_ADD_ARR30;
            }
            else if(mMenuModeObject.selectIndex == 4 && mMenuModeObject.numEnter !=0)
            {
                calendar.Minutes = mMenuModeObject.numEnter;

                mMenuModeObject.numEnter = 0;
                mMenuModeObject.selectIndex =5;
                Lcd_clear_screen();
                mMenuModeObject.keyDoing = KEY_DOING_NULL;
                goto TAB_REPEAT_ADD_ARR30;
            }
            else if(mMenuModeObject.selectIndex == 5 && mMenuModeObject.numEnter !=0)
            {
                calendar.Seconds = mMenuModeObject.numEnter;
                mMenuModeObject.numEnter = 0;
                mMenuModeObject.selectIndex =6;
                mMenuModeObject.keyDoing = KEY_DOING_NULL;
                Lcd_set_font(36, 24, 1);
                Disp_icon(START_X_TIP,3,ICON_36X24_COMPLETE,1);
                //调用设置
                //goto TAB_REPEAT_ADD_ARR30;
                Rtc_set_calendar(&calendar);
            }
            else if(mMenuModeObject.selectIndex == 6)
            {
                mMenuModeObject.selectIndex =0;
                mMenuModeObject.numEnter = 0;
                Lcd_clear_screen();
                mMenuModeObject.keyDoing = KEY_DOING_NULL;
                goto TAB_REPEAT_ADD_ARR30;
            }
            mMenuModeObject.numEnter = 0;
            mMenuModeObject.keyDoing = KEY_DOING_NULL;
        }
    }

}
#endif
