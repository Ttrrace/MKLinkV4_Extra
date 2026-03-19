/*
 * Copyright (c) 2009-2024 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*============================ INCLUDES ======================================*/

#include "arm_2d.h"

#if defined(RTE_Acceleration_Arm_2D_Helper_PFB)

#define __USER_SCENE_MENU_IMPLEMENT__
#include "arm_2d_scene_user_menu.h"
#include "arm_2d_scene_user_button.h"
#include "arm_2d_user_navigation.h"
#include "arm_2d_scene_user_waveform.h"
#include "arm_2d_helper.h"
#include "arm_2d_example_controls.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "microlink_board.h"
#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wunknown-warning-option"
#   pragma clang diagnostic ignored "-Wreserved-identifier"
#   pragma clang diagnostic ignored "-Wsign-conversion"
#   pragma clang diagnostic ignored "-Wpadded"
#   pragma clang diagnostic ignored "-Wcast-qual"
#   pragma clang diagnostic ignored "-Wcast-align"
#   pragma clang diagnostic ignored "-Wmissing-field-initializers"
#   pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#   pragma clang diagnostic ignored "-Wmissing-prototypes"
#   pragma clang diagnostic ignored "-Wunused-variable"
#   pragma clang diagnostic ignored "-Wgnu-statement-expression"
#   pragma clang diagnostic ignored "-Wdeclaration-after-statement"
#   pragma clang diagnostic ignored "-Wunused-function"
#   pragma clang diagnostic ignored "-Wmissing-declarations"  
#elif __IS_COMPILER_ARM_COMPILER_5__
#   pragma diag_suppress 64,177
#elif __IS_COMPILER_GCC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wformat="
#   pragma GCC diagnostic ignored "-Wpedantic"
#   pragma GCC diagnostic ignored "-Wunused-function"
#   pragma GCC diagnostic ignored "-Wunused-variable"
#   pragma GCC diagnostic ignored "-Wunused-value"
#   pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#elif __IS_COMPILER_IAR__
#   pragma diag_suppress=Pa089,Pe188,Pe177,Pe174
#endif

/*============================ MACROS ========================================*/

#if __GLCD_CFG_COLOUR_DEPTH__ == 8

#   define c_tileCMSISLogo          c_tileCMSISLogoGRAY8

#elif __GLCD_CFG_COLOUR_DEPTH__ == 16

#   define c_tileCMSISLogo          c_tileCMSISLogoRGB565

#elif __GLCD_CFG_COLOUR_DEPTH__ == 32

#   define c_tileCMSISLogo          c_tileCMSISLogoCCCA8888
#else
#   error Unsupported colour depth!
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
#undef this
#define this (*ptThis)

#define ICON_PROP 1.0f
#define GLCD_WECHAT_GREEN  __RGB( 0X06, 0XC7, 0X63)
#define GLCD_WIN_BLUE      __RGB( 0X63, 0XE3, 0XFF)
#define GLCD_COLOR_DARKER_GREY    __RGB( 0X32, 0X32, 0X32  )
#define GLCD_COLOR_B_GREEN        __RGB( 0X5D, 0XF5, 0XFE  )
#define GLCD_COLOR_2077_YELLOW    __RGB( 0XFE, 0XF6, 0X4D  )
#define GLCD_COLOR_RED_ORANGE     __RGB( 0XFF, 0X56, 0X16  )


#define GLCD_COLOR_FRAME           __RGB( 255, 128, 0    )

#define GLCD_COLOR_ICON            __RGB(   0,   0,   0  )
#define GLCD_COLOR_ICON_UNSELECT   __RGB( 255, 128,   0  )

#define GLCD_COLOR_TEXT_FRAME_SIDE1 __RGB(  0X5D, 0XF5, 0XFE )
#define GLCD_COLOR_TEXT_FRAME_SIDE2 __RGB(   0,0xc4, 0xfb )
#define GLCD_COLOR_TEXT_SIDE1       __RGB(   0,0x0c, 0x18)
#define GLCD_COLOR_TEXT_SIDE2       __RGB(   0,0x0c, 0x18)

#define GLCD_COLOR_BACKGROUND       __RGB(   0,0x0c, 0x18)


/*============================ TYPES =========================================*/
arm_2d_location_t  tStartPoint[4];

/*============================ GLOBAL VARIABLES ==============================*/
extern const arm_2d_tile_t c_tilemodelRGB565;
extern const arm_2d_tile_t c_tiletipsrotationA4Mask;
extern const arm_2d_tile_t c_tilepressA4Mask;
extern const arm_2d_tile_t c_tilelongpressA4Mask;

extern const arm_2d_tile_t c_tileTextSetA4Mask;
extern const arm_2d_tile_t c_tileTextDownloadA4Mask;
extern const arm_2d_tile_t c_tileTextObserveA4Mask;
extern const arm_2d_tile_t c_tileTextSerialA4Mask;


extern
const
struct {
    implement(arm_2d_user_font_t);
    arm_2d_char_idx_t tUTF8Table;
} ARM_2D_FONT_LiberationSansRegular14_A4;

extern 
const
struct {
    implement(arm_2d_user_font_t);
    arm_2d_char_idx_t tUTF8Table1;
}ARM_2D_FONT_Round20_A4,
 ARM_2D_FONT_Round20_A2;

extern 
const
struct {
    implement(arm_2d_user_font_t);
    arm_2d_char_idx_t tUTF8Table2;
}ARM_2D_FONT_Round16_A4,
 ARM_2D_FONT_Round16_A2;

arm_2d_size_t tmodel_size = 
{
  .iHeight = 150,
  .iWidth  = 240
};

arm_2d_region_t tText_region = 
{
  .tLocation.iX  = 0,
  .tLocation.iY  = 175,
  .tSize.iHeight = 15,
  .tSize.iWidth  = 240,
};

uint8_t Text_cnt[3] = {0};

enum {
    PANEL_MODEL,
    PANEL_QRCODE,
};

/*============================ PROTOTYPES ====================================*/

/*============================ LOCAL VARIABLES ===============================*/

/*! define dirty regions */
IMPL_ARM_2D_REGION_LIST(s_tDirtyRegions, static)

    /* a dirty region to be specified at runtime*/
    ADD_REGION_TO_LIST(s_tDirtyRegions,
        0  /* initialize at runtime later */
    ),
    
    /* add the last region:
        * it is the top left corner for text display 
        */
    ADD_LAST_REGION_TO_LIST(s_tDirtyRegions,
        .tLocation = {
            .iX = 0,
            .iY = 0,
        },
        .tSize = {
            .iWidth = 0,
            .iHeight = 8,
        },
    ),

END_IMPL_ARM_2D_REGION_LIST(s_tDirtyRegions)


/*============================ IMPLEMENTATION ================================*/


#define ITEM_BG_OPACITY     (255)

static arm_fsm_rt_t __scene_frame_actions(arm_2d_scene_t *ptScene){
    user_scene_menu_t *ptThis = (user_scene_menu_t *)ptScene;
    int16_t iResult[4];
    ARM_2D_UNUSED(ptThis);
    ARM_PT_BEGIN(this.chPT)
    static uint8_t text_cnt;

    switch (this.chState){
          case 0:
                  this.bShowModel = true;
                  this.chState = 1;  
               break;
          case 1: 
               if(this.bShowModel){
                   if(arm_2d_helper_time_half_cos_slider(0, 100, 300, &iResult[0], &this.lTimestamp[1]))
                   { 
                      if(!this.bShowModel){
                         this.chState = 2;
                         this.lTimestamp[1] = 0;
                      }

                      
                      this.iProgress[2]  = 100;
                      break;
                   }else{
                      this.iProgress[2] = iResult[0];    //引脚定义、模型弹出
                   }
                }
               break;
          case 2:
               if(!this.bShowModel) {
                   if(arm_2d_helper_time_half_cos_slider(100, 0, 300, &iResult[0], &this.lTimestamp[1])){
                      this.chState = 1;
                      this.lTimestamp[1] = 0;
                      this.iProgress[2]  = 0;
                      }else{
                        this.iProgress[2] = iResult[0];
                      }
               }
               break;
    }

    ARM_PT_END();

    return arm_fsm_rt_cpl;
}

static void __on_scene_menu_depose(arm_2d_scene_t *ptScene)
{
    user_scene_menu_t *ptThis = (user_scene_menu_t *)ptScene;
    ARM_2D_UNUSED(ptThis);
    /* reset timestamp */
    arm_foreach(int64_t,this.lTimestamp, ptItem) {
        *ptItem = 0;
    }

    arm_2d_helper_dirty_region_remove_items(&this.use_as__arm_2d_scene_t.tDirtyRegionHelper,
                                            &this.tDirtyRegionItem[0],
                                            3);

    do {
        arm_foreach(arm_2d_user_draw_line_descriptor_t, this.data_line, ptLineOP) {
            ARM_2D_OP_DEPOSE(*ptLineOP);
        }
    } while(0);

    ptScene->ptPlayer = NULL;
        /* initialize transform helper */
    qrcode_box_depose(&this.QRCode.tBox);
    arm_foreach (this.tPanel) {
        foldable_panel_depose(_);
    }
    crt_screen_depose(&this.tCRTScreen);
    if (!this.bUserAllocated) {
        __arm_2d_free_scratch_memory(ARM_2D_MEM_TYPE_UNSPECIFIED, ptScene);
    }
}

/*----------------------------------------------------------------------------*
 * Scene 3                                                                    *
 *----------------------------------------------------------------------------*/

static void __on_scene_menu_background_start(arm_2d_scene_t *ptScene)
{
    user_scene_menu_t *ptThis = (user_scene_menu_t *)ptScene;
    ARM_2D_UNUSED(ptThis);

}   

static void __on_scene_menu_background_complete(arm_2d_scene_t *ptScene)
{
    user_scene_menu_t *ptThis = (user_scene_menu_t *)ptScene;
    ARM_2D_UNUSED(ptThis);

}

static void __on_scene_menu_load(arm_2d_scene_t *ptScene)
{
    user_scene_menu_t *ptThis = (user_scene_menu_t *)ptScene;
    ARM_2D_UNUSED(ptThis);
    arm_foreach (this.tPanel) {
        foldable_panel_on_load(_);    
    } 
    this.bFirstShowModel = true;
    crt_screen_on_load(&this.tCRTScreen);
    qrcode_box_on_load(&this.QRCode.tBox);
    arm_2d_helper_dirty_region_add_items(&this.use_as__arm_2d_scene_t.tDirtyRegionHelper,
                                         &this.tDirtyRegionItem[0],
                                         3);

}

static void __on_scene_menu_frame_start(arm_2d_scene_t *ptScene)
{
    user_scene_menu_t *ptThis = (user_scene_menu_t *)ptScene;
    int nResult[3];
    static int16_t iY;

    if (arm_2d_helper_time_liner_slider(0, 300, 200, &nResult[0], &this.lTimestamp[2])) {
        if(this.bFirstShowModel && foldable_panel_status(&this.tPanel[PANEL_MODEL]) == FOLDABLE_PANEL_STATUS_FOLDED){
           foldable_panel_unfold(&this.tPanel[PANEL_MODEL]);
        }
        this.bFirstShowModel = false;
    }

    if(this.bMenuOpened){
       if(arm_2d_helper_time_half_cos_slider(0, 100, 500, &nResult[1], &this.lTimestamp[0])){
          this.bIsMenuSlideEnd = true;
       }else{
           this.iProgress[1] = nResult[1];
           this.bIsMenuSlideEnd = false;
       }
    }else{
       this.bIsMenuSlideEnd = true;
       this.iProgress[1] = 0;
       this.lTimestamp[0] = 0;
    }
    this.iProgress[0] = (int16_t)nResult[0];
    crt_screen_on_frame_start(&this.tCRTScreen);

    arm_foreach (this.tPanel) {
        foldable_panel_on_frame_start(_);
    }

    __scene_frame_actions(ptScene);

    RG_PROCESS_MSG(this.ptGUI, ptMSG) {
        ARM_2D_UNUSED(bIsMessageHandled);
            this.lTimestamp[3] = 0;
            switch(ngy_helper_msg_item_get_id(ptMSG)) {
                case NGY_MSG_GESTURE_EVT_WHEEL:
                do {
                    ngy_msg_gesture_evt_t *ptKeyMSG = (ngy_msg_gesture_evt_t *)ptMSG;
                    if(!user_navigation.SlideOut){
                      if(ptKeyMSG->tRegion.tLocation.iY > iY){
                          if(!this.bShowModel){

                            this.bShowModel = true;
                            this.bShowQR = false;
                          }
                      }else if(ptKeyMSG->tRegion.tLocation.iY < iY){
                         if(this.bShowModel && (this.bFirstShowModel == false)){ //防止过早操作

                             this.bShowModel = false;
                             this.bShowQR = true;
                          }
                      }
                    }else{
                        if(ptKeyMSG->tRegion.tLocation.iY > iY){
                           navi_operate(WHEEL_UP);
                        }else{
                           navi_operate(WHEEL_DOWN);
                        }
                    }

                    iY = ptKeyMSG->tRegion.tLocation.iY;
                    bIsMessageHandled = true;
                }while(0);
                break;
                /* key pressed */
                case NGY_MSG_KEY_EVT_PRESSED:
                   if(user_navigation.SlideOut){
                       navi_operate(SINGLE_PRESS);
                       if(icon_list_get_selected_item_id(&user_navigation.tList) != ICON_IDX_INFO){
                          user_navigation.bSwitchPage = false;
                          arm_2d_scene_player_switch_to_next_scene(ptScene->ptPlayer);
                       }
                    }
                    bIsMessageHandled = true;
                    break;
                case NGY_MSG_KEY_EVT_LONG_PRESSING:     
                //     arm_2d_scene_player_switch_to_next_scene(ptScene->ptPlayer);                     
                    navi_operate(LONG_PRESS);
                    bIsMessageHandled = true;
                    break;             
                default:
                    break;
            }
        
    }
    if(this.bShowModel == true){
      if(foldable_panel_status(&this.tPanel[PANEL_QRCODE]) == FOLDABLE_PANEL_STATUS_UNFOLDED){
           foldable_panel_fold(&this.tPanel[PANEL_QRCODE]);
        }
      if(foldable_panel_status(&this.tPanel[PANEL_MODEL]) == FOLDABLE_PANEL_STATUS_FOLDED){
           foldable_panel_unfold(&this.tPanel[PANEL_MODEL]);
        }
    }else if(this.bShowQR == true){
        if(foldable_panel_status(&this.tPanel[PANEL_QRCODE]) == FOLDABLE_PANEL_STATUS_FOLDED){
          foldable_panel_unfold(&this.tPanel[PANEL_QRCODE]);
         }
        if(foldable_panel_status(&this.tPanel[PANEL_MODEL]) == FOLDABLE_PANEL_STATUS_UNFOLDED){
          foldable_panel_fold(&this.tPanel[PANEL_MODEL]);
         }
    }

}

static void __on_scene_menu_frame_switching_out(arm_2d_scene_t *ptScene)
{
    user_scene_menu_t *ptThis = (user_scene_menu_t *)ptScene;
    ARM_2D_UNUSED(ptThis);

    arm_2d_scene_player_set_switching_mode( 
                    &DISP0_ADAPTER,
                    ARM_2D_SCENE_SWITCH_MODE_SLIDE_DOWN);    
    arm_2d_scene_player_set_switching_period(
            &DISP0_ADAPTER, 
            500);	    

  //  this.bFirstShowModel = true;
    if(!this.bScreenProtect){
          switch(icon_list_get_selected_item_id(&user_navigation.tList)){
          case 0:  
              user_scene_button_t *ptButton = 
              arm_2d_scene_button_init(this.use_as__arm_2d_scene_t.ptPlayer);
              assert(NULL != ptButton);
              ptButton->ptGUI = &g_tMyGUI;  
            break;
          case 1:
              extern user_scene_waveform_t *ptwaveform;
              ptwaveform = 
              arm_2d_scene_waveform_init(this.use_as__arm_2d_scene_t.ptPlayer);
              assert(NULL != ptwaveform);
              ptwaveform->ptGUI = &g_tMyGUI;  
            break;
          case 2:
            break; 
         }
    }
}

static void __on_scene_menu_frame_complete(arm_2d_scene_t *ptScene)
{
    user_scene_menu_t *ptThis = (user_scene_menu_t *)ptScene;
    ARM_2D_UNUSED(ptThis);


    arm_foreach (this.tPanel) {
        foldable_panel_on_frame_complete(_);
    }
        crt_screen_on_frame_complete(&this.tCRTScreen);


    if(foldable_panel_status(&this.tPanel[1]) == FOLDABLE_PANEL_STATUS_FOLDING 
     ||foldable_panel_status(&this.tPanel[1]) == FOLDABLE_PANEL_STATUS_UNFOLDING){
        arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[1],0);
      }else{
        arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[1],1);
      }

     if(this.iProgress[2] == 100 || this.iProgress[2] == 0){
     //   arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[2],1);
     }else{
        arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[2],0);
     }

}

static
IMPL_PFB_ON_DRAW(__pfb_draw_scene_menu_background_handler)
{
    user_scene_menu_t *ptThis = (user_scene_menu_t *)pTarget;
    ARM_2D_UNUSED(ptTile);
    ARM_2D_UNUSED(bIsNewFrame);
    /*-----------------------draw back ground begin-----------------------*/



    /*-----------------------draw back ground end  -----------------------*/
    ARM_2D_OP_WAIT_ASYNC();

    return arm_fsm_rt_cpl;
}

static  
IMPL_PFB_ON_DRAW(__pfb_draw_scene_menu_handler)
{
    user_scene_menu_t *ptThis = (user_scene_menu_t *)pTarget;
    ARM_2D_UNUSED(ptTile);
    ARM_2D_UNUSED(bIsNewFrame);

    /*-----------------------draw the foreground begin-----------------------*/
    arm_2d_canvas(ptTile, __top_canvas) {
        arm_2d_layout(__top_canvas, LEFT_TO_RIGHT) {
            __item_line_dock_horizontal(240, 0, 0, 25, 40) {
                arm_2d_align_centre(__item_region,240, 150) {
                    arm_2d_tile_t *ptModelPanel = 
                        foldable_panel_show(&this.tPanel[0],
                                            ptTile, 
                                            &__item_region,
                                            bIsNewFrame);

                    arm_2d_helper_dirty_region_update_item(&this.tDirtyRegionItem[0],
                                                           ptTile,
                                                           NULL,
                                                           &__item_region);   
                    ARM_2D_OP_WAIT_ASYNC();

                    assert(NULL != ptModelPanel);
                    arm_2d_canvas(ptModelPanel, __item_region){
                        arm_2d_align_centre(__item_region, tmodel_size) {
                           arm_2d_rgb16_tile_copy_only( &c_tilemodelRGB565, 
                                                        ptModelPanel, 
                                                        &__centre_region); 
                        }
                    }

                    arm_2d_user_draw_line_api_params_t tParam_1 = {
                        .tStart.iX = 140,
                        .tStart.iY = 60,
                        .tEnd.iX   = 160,
                        .tEnd.iY   = 20,
                    };
                    arm_2d_user_draw_line_api_params_t tParam_2 = {
                        .tStart.iX = 160,
                        .tStart.iY = 20,
                        .tEnd.iX   = 180,
                        .tEnd.iY   = 20,
                    };
                    arm_2d_user_draw_line_api_params_t tParam_3 = {
                        .tStart.iX = 140,
                        .tStart.iY = 70,
                        .tEnd.iX   = 180,
                        .tEnd.iY   = 70,
                    };
                    arm_2d_user_draw_line_api_params_t tParam_4 = {
                        .tStart.iX = 140,
                        .tStart.iY = 80,
                        .tEnd.iX   = 160,
                        .tEnd.iY   = 120,
                    };
                    arm_2d_user_draw_line_api_params_t tParam_5 = {
                        .tStart.iX = 160,
                        .tStart.iY = 120,
                        .tEnd.iX   = 180,
                        .tEnd.iY   = 120,
                    };

                    arm_lcd_text_set_target_framebuffer(ptModelPanel);
                    arm_lcd_text_set_colour(GLCD_COLOR_ORANGE, GLCD_COLOR_BLACK);
                    arm_lcd_text_set_opacity(192);
                    arm_lcd_text_set_font(&ARM_2D_FONT_Round16_A4);
                    arm_lcd_text_set_display_mode(ARM_2D_DRW_PATN_MODE_COPY);
                    arm_lcd_text_set_draw_region(&__item_region);

                    arm_2dp_rgb565_user_draw_line(
                      &this.data_line[0],
                      ptModelPanel,
                      NULL,
                      &tParam_1,
                      (arm_2d_color_rgb565_t){GLCD_COLOR_B_GREEN},
                      128);

                    ARM_2D_OP_WAIT_ASYNC();

                    arm_2dp_rgb565_user_draw_line(
                      &this.data_line[1],
                      ptModelPanel,
                      NULL,
                      &tParam_2,
                      (arm_2d_color_rgb565_t){GLCD_COLOR_B_GREEN},
                      128);

                    ARM_2D_OP_WAIT_ASYNC();

                    arm_2d_region_t tTips_1 = 
                    {
                      .tLocation.iX  = tParam_2.tEnd.iX,
                      .tLocation.iY  = tParam_2.tEnd.iY - 5,
                      .tSize.iHeight = 30,
                      .tSize.iWidth  = 30,
                    };     
    
                    arm_2d_rgb565_fill_colour_with_mask_and_opacity(ptModelPanel,
                                                        &tTips_1,
                                                        &c_tiletipsrotationA4Mask,
                                                        (arm_2d_color_rgb565_t){GLCD_COLOR_B_GREEN},
                                                        192);

                  // tTips_1.tLocation.iY -= 15;
                    arm_2d_region_t tText_1 = 
                    {
                      .tLocation.iX  = tTips_1.tLocation.iX + 15,
                      .tLocation.iY  = tTips_1.tLocation.iY - 15,
                      .tSize.iHeight = 50,
                      .tSize.iWidth  = 50,
                    }; 
                      arm_lcd_text_set_draw_region(&tText_1);
                      arm_lcd_printf_label(ARM_2D_ALIGN_TOP_LEFT,"旋转");
                      tText_1.tLocation.iY += 15;

                      arm_lcd_text_set_draw_region(&tText_1);
                      arm_lcd_printf_label(ARM_2D_ALIGN_TOP_LEFT,"切换");

   
                    arm_2dp_rgb565_user_draw_line(
                      &this.data_line[2],
                      ptModelPanel,
                      NULL,
                      &tParam_3,
                      (arm_2d_color_rgb565_t){GLCD_COLOR_B_GREEN},
                      128);

                    arm_2d_region_t tTips_2 = 
                    {
                      .tLocation.iX  = tParam_3.tEnd.iX,
                      .tLocation.iY  = tParam_3.tEnd.iY - 5,
                      .tSize.iHeight = 30,
                      .tSize.iWidth  = 30,
                    };     
    
                    arm_2d_rgb565_fill_colour_with_mask_and_opacity(ptModelPanel,
                                                        &tTips_2,
                                                        &c_tilepressA4Mask,
                                                        (arm_2d_color_rgb565_t){GLCD_COLOR_B_GREEN},
                                                        192);

                    arm_2d_region_t tText_2 = 
                    {
                      .tLocation.iX  = tTips_2.tLocation.iX + 15,
                      .tLocation.iY  = tTips_2.tLocation.iY - 15,
                      .tSize.iHeight = 50,
                      .tSize.iWidth  = 50,
                    }; 
                      arm_lcd_text_set_draw_region(&tText_2);
                      arm_lcd_printf_label(ARM_2D_ALIGN_TOP_LEFT,"短按");
                      tText_2.tLocation.iY += 15;

                      arm_lcd_text_set_draw_region(&tText_2);
                      arm_lcd_printf_label(ARM_2D_ALIGN_TOP_LEFT,"确认");
  

                    arm_2dp_rgb565_user_draw_line(
                      &this.data_line[3],
                      ptModelPanel,
                      NULL,
                      &tParam_4,
                      (arm_2d_color_rgb565_t){GLCD_COLOR_B_GREEN},
                      128);

                    arm_2dp_rgb565_user_draw_line(
                      &this.data_line[4],
                      ptModelPanel,
                      NULL,
                      &tParam_5,
                      (arm_2d_color_rgb565_t){GLCD_COLOR_B_GREEN},
                      128);

                    arm_2d_region_t tTips_3 = 
                    {
                      .tLocation.iX  = tParam_5.tEnd.iX,
                      .tLocation.iY  = tParam_5.tEnd.iY - 5,
                      .tSize.iHeight = 30,
                      .tSize.iWidth  = 30,
                    };     
    
                    arm_2d_rgb565_fill_colour_with_mask_and_opacity(ptModelPanel,
                                                        &tTips_3,
                                                        &c_tilelongpressA4Mask,
                                                        (arm_2d_color_rgb565_t){GLCD_COLOR_B_GREEN},
                                                        192);

                    arm_2d_region_t tText_3 = 
                    {
                      .tLocation.iX  = tTips_3.tLocation.iX + 15,
                      .tLocation.iY  = tTips_3.tLocation.iY - 10,
                      .tSize.iHeight = 50,
                      .tSize.iWidth  = 50,
                    }; 

                        arm_lcd_text_set_draw_region(&tText_3);
                        arm_lcd_printf_label(ARM_2D_ALIGN_TOP_LEFT,"长按");
                        tText_3.tLocation.iY += 15;
                        arm_lcd_text_set_draw_region(&tText_3);
                        arm_lcd_printf_label(ARM_2D_ALIGN_TOP_LEFT,"菜单");
                 
                  }

        int16_t iQRCodePixelSize = qrcode_box_get_size(&this.QRCode.tBox);

        arm_2d_align_centre(__top_canvas, iQRCodePixelSize + 14, iQRCodePixelSize + 14) {
            arm_2d_tile_t *ptPanel = 
                foldable_panel_show(&this.tPanel[PANEL_QRCODE],
                                    ptTile, 
                                    &__centre_region,
                                    bIsNewFrame);

           arm_2d_helper_dirty_region_update_item(&this.tDirtyRegionItem[1],
                                                  ptTile,
                                                  NULL,
                                                  &__centre_region);   

            arm_2d_canvas(ptPanel, __qrcode_panel) {
                qrcode_box_show(&this.QRCode.tBox, 
                                ptPanel,
                                NULL,
                                GLCD_COLOR_B_GREEN, 
                                255);
            }
        }

            }
        }
        arm_lcd_text_set_target_framebuffer((arm_2d_tile_t *)ptTile);
        arm_lcd_text_set_colour(GLCD_COLOR_ORANGE, GLCD_COLOR_BLACK);
        arm_lcd_text_set_opacity(192);
        arm_lcd_text_set_font(&ARM_2D_FONT_6x8.use_as__arm_2d_font_t);
        arm_lcd_text_set_display_mode(ARM_2D_DRW_PATN_MODE_COPY);

        tText_region.tLocation.iY = 260 - this.iProgress[2] * 0.75;
        tText_region.tSize.iHeight = 240 - tText_region.tLocation.iY;

         arm_2d_helper_dirty_region_update_item(&this.tDirtyRegionItem[2],
                                                ptTile,
                                                NULL,
                                                &tText_region);   

        arm_2d_layout(tText_region, LEFT_TO_RIGHT) {
            __item_line_dock_vertical(6, 0, 0, 6, 0) {
              arm_lcd_text_set_draw_region(&__item_region);
              arm_lcd_printf_label(ARM_2D_ALIGN_TOP_LEFT, "5V  TX  RST TDO GND CLK DIO TDI  NC VREF");       
            }
              arm_lcd_text_set_colour(GLCD_COLOR_B_GREEN, GLCD_COLOR_BLACK);
            __item_line_dock_vertical(6, 0, 0, 6, 0) {
              arm_lcd_text_set_draw_region(&__item_region);
              arm_lcd_printf_label(ARM_2D_ALIGN_TOP_LEFT, "19  17  15  13  11   9   7   5   3   1");   
            }
             __item_line_dock_vertical(6, 0, 0, 0, 0) {
              arm_lcd_text_set_draw_region(&__item_region);
              arm_lcd_printf_label(ARM_2D_ALIGN_TOP_LEFT, "----------------------------------------");       
            }     
            __item_line_dock_vertical(6, 0, 100, 0, 0) {
              arm_lcd_text_set_draw_region(&__item_region);
              arm_lcd_printf_label(ARM_2D_ALIGN_TOP_LEFT, "20  18  16  14  12   10  8   6   4   2");       
            }   
            arm_lcd_text_set_colour(GLCD_COLOR_ORANGE, GLCD_COLOR_BLACK);      
            __item_line_dock_vertical(6, 0, 0, 6, 0) {
              arm_lcd_text_set_draw_region(&__item_region);
              arm_lcd_printf_label(ARM_2D_ALIGN_TOP_LEFT, "GND RX GND 485A 485B GND GND GND GND VCC");       
            }         
        }
    }
    /*-----------------------draw the foreground end  -----------------------*/
    ARM_2D_OP_WAIT_ASYNC();

    return arm_fsm_rt_cpl;
}

ARM_NONNULL(1)
user_scene_menu_t *__arm_2d_scene_menu_init(   arm_2d_scene_player_t *ptDispAdapter, 
                                        user_scene_menu_t *ptThis)
{
    bool bUserAllocated = false;
    assert(NULL != ptDispAdapter);

    if (NULL == ptThis) {
        ptThis = (user_scene_menu_t *)
                    __arm_2d_allocate_scratch_memory(   sizeof(user_scene_menu_t),
                                                        __alignof__(user_scene_menu_t),
                                                        ARM_2D_MEM_TYPE_UNSPECIFIED);
        assert(NULL != ptThis);
        if (NULL == ptThis) {
            return NULL;
        }
    } else {
        memset(ptThis, 0, sizeof(user_scene_menu_t));
        bUserAllocated = true;
    }
    
    *ptThis = (user_scene_menu_t){
        .use_as__arm_2d_scene_t = {

        /* the canvas colour */
        .tCanvas = {GLCD_COLOR_BLACK}, 
        
        /* Please uncommon the callbacks if you need them
         */
        //.fnBackground   = &__pfb_draw_scene_menu_background_handler,
        .fnOnLoad       = &__on_scene_menu_load,
        .fnScene        = &__pfb_draw_scene_menu_handler,
        //.ptDirtyRegion  = (arm_2d_region_list_item_t *)s_tDirtyRegions,
        
        //.fnOnBGStart    = &__on_scene_menu_background_start,
        //.fnOnBGComplete = &__on_scene_menu_background_complete,
        .fnOnFrameStart = &__on_scene_menu_frame_start,
        .fnBeforeSwitchOut = &__on_scene_menu_frame_switching_out,
        .fnOnFrameCPL   = &__on_scene_menu_frame_complete,
        .fnDepose       = &__on_scene_menu_depose,
        .bUseDirtyRegionHelper = false,
        },
        .bUserAllocated = bUserAllocated,
    };

    do{
        foldable_panel_cfg_t tCFG = {
            .bShowScanLines = true,
            .ptScene = &this.use_as__arm_2d_scene_t,
            .tLineColour.tColour = GLCD_COLOR_ORANGE,
            .u12HorizontalFoldingTimeInMS = 200,
        };
        foldable_panel_init(&this.tPanel[PANEL_MODEL], &tCFG);
    } while(0);

    do {
        foldable_panel_cfg_t tCFG = {
            .ptScene = &this.use_as__arm_2d_scene_t,
            .bShowScanLines = true,
            .bAlignTimeline = true,
            .u12HorizontalFoldingTimeInMS = 200,
            .u12VerticalFoldingTimeInMS = 200,
        };
        foldable_panel_init(&this.tPanel[PANEL_QRCODE], &tCFG);
    } while(0);

    /* CRT Screen */
    do {
        crt_screen_cfg_t tCRTScreenCFG = {
            .ptScene = &this.use_as__arm_2d_scene_t,

            .ptImageBoxCFG = (image_box_cfg_t []) {{
                .ptilePhoto = &c_tilemodelRGB565,
                .tScreenColour.tColour = GLCD_COLOR_GREEN,
            }},

            .tScanBarColour.tColour = GLCD_COLOR_WHITE,
            .chWhiteNoiseRatio = 32,
            .chNoiseLasts = 32,
            .bStrongNoise = true,
            .bShowScanningEffect = true,
        };
        crt_screen_init(&this.tCRTScreen, &tCRTScreenCFG);
    } while(0);

    /* initialize QRcode box */
    do {
        static const char c_chURL[] = {"https://microboot.readthedocs.io/zh-cn/latest/tools/microlink/microlink/"};
        qrcode_box_cfg_t tCFG = {
            .bIsString = true,
            .pchString = c_chURL,
            .hwInputSize = sizeof(c_chURL),
            .pchBuffer = (uint8_t *)this.QRCode.chBuffer,
            .hwQRCodeBufferSize = sizeof(this.QRCode.chBuffer),
            .chSquarePixelSize = 2,
          
            .u2ECCLevel = qrcodegen_Ecc_HIGH,
        };

        arm_2d_err_t tResult = qrcode_box_init(&this.QRCode.tBox, &tCFG);
        assert(tResult == ARM_2D_ERR_NONE);

    } while(0);

    do {
            arm_foreach(arm_2d_user_draw_line_descriptor_t, this.data_line, ptLineOP) {
            ARM_2D_OP_INIT(*ptLineOP);
        }
    } while(0);
    this.bScreenProtect = 0;
    arm_2d_scene_player_append_scenes(  ptDispAdapter, 
                                        &this.use_as__arm_2d_scene_t, 
                                        1);
    return ptThis;
}


#if defined(__clang__)
#   pragma clang diagnostic pop
#endif

#endif

