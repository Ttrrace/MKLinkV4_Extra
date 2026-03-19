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

#define __USER_SCENE_BUTTON_IMPLEMENT__
#include "arm_2d_scene_user_button.h"
#include "arm_2d_scene_user_menu.h"
#include "arm_2d_scene_user_waveform.h"
#include "arm_2d_user_navigation.h"
#include "arm_2d_scene_user_text_tracking_list.h"
#include "microlink_board.h"
#include "pika_config.h"
#if defined(RTE_Acceleration_Arm_2D_Helper_PFB)

#include <stdlib.h>
#include <string.h>

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
#   pragma clang diagnostic ignored "-Wimplicit-int-conversion" 
#elif __IS_COMPILER_ARM_COMPILER_5__
#   pragma diag_suppress 64,177
#elif __IS_COMPILER_IAR__
#   pragma diag_suppress=Pa089,Pe188,Pe177,Pe174
#elif __IS_COMPILER_GCC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wformat="
#   pragma GCC diagnostic ignored "-Wpedantic"
#   pragma GCC diagnostic ignored "-Wunused-function"
#   pragma GCC diagnostic ignored "-Wunused-variable"
#   pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
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

#define GLCD_COLOR_FRAME            __RGB(  255,  128, 0   )
#define GLCD_COLOR_BACKGROUND       __RGB(    0, 0x0c, 0x18)
#define GLCD_COLOR_B_GREEN          __RGB( 0X5D, 0XF5, 0XFE)
#define GLCD_WECHAT_GREEN           __RGB( 0X06, 0XC7, 0X63)
#define GLCD_WECHAT_GREEN_SHADOW    __RGB( 0X06, 0XA7, 0X53)
#define GLCD_SELECTED_GREEN         __RGB( 0X28, 0X93, 0XBD)
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
extern 
const
struct {
    implement(arm_2d_user_font_t);
    arm_2d_char_idx_t tUTF8Table2;
}ARM_2D_FONT_Round16_A4,
 ARM_2D_FONT_Round16_A2;
extern
const
struct {
    implement(arm_2d_user_font_t);
    arm_2d_char_idx_t tUTF8Table;
} ARM_2D_FONT_LiberationSansRegular14_A1,
  ARM_2D_FONT_LiberationSansRegular14_A2,
  ARM_2D_FONT_LiberationSansRegular14_A4,
  ARM_2D_FONT_LiberationSansRegular14_A8;

//extern const arm_2d_tile_t c_tileCMSISLogo;
//  extern const arm_2d_tile_t c_tileBarFrameMask;
  extern const arm_2d_tile_t c_tileBarFrameA4Mask;
  extern const arm_2d_tile_t c_tileTextShadowMask;
  extern const arm_2d_tile_t c_tilemicrolinkMask;

  extern const arm_2d_tile_t c_tilestart_buttonA4Mask;
  extern const arm_2d_tile_t c_tilestart_button_fillMask;

  extern const arm_2d_tile_t c_tileloadingMask;
  extern const arm_2d_tile_t c_tileConsoleFrameA4Mask;
  extern const arm_2d_tile_t c_tileConsoleFrameFillMask;
  extern const arm_2d_tile_t c_tileFileFrameA4Mask;
  extern const arm_2d_tile_t c_tileFileFrameFillMask;
  extern const arm_2d_tile_t c_tileBarFrameFillMask;
  extern const arm_2d_tile_t c_tileDownloadButtonA4Mask;
/*============================ PROTOTYPES ====================================*/
/*============================ LOCAL VARIABLES ===============================*/

enum {
    REGION_IDX_BUTTON = 0,
    REGION_IDX_LINE_EDIT = REGION_IDX_BUTTON,
    REGION_IDX_TEXT_BOX,
    REGION_IDX_TEXT_DOWNLOAD
};

/*! define dirty regions */
IMPL_ARM_2D_REGION_LIST(s_tDirtyRegions, static)
    
    /* add the last region:
        * it is the top left corner for text display 
        */
    ADD_LAST_REGION_TO_LIST(s_tDirtyRegions,
        .tSize = {
            .iWidth = 220,
            .iHeight = 220,
        },
    ),

END_IMPL_ARM_2D_REGION_LIST(s_tDirtyRegions)

/*============================ IMPLEMENTATION ================================*/
static void __console_box_printf(arm_2d_scene_t *ptScene,char ch)
{
    user_scene_button_t *ptThis = (user_scene_button_t *)ptScene;
    ARM_2D_UNUSED(ptThis);
    this.Console_iY = 0;
    console_box_printf(&this.tConsole, "%c",ch);
}

static void __updata_progress_bar(arm_2d_scene_t *ptScene,char progress)
{
    user_scene_button_t *ptThis = (user_scene_button_t *)ptScene;
    ARM_2D_UNUSED(ptThis);
    this.downloadprogress = progress;
}

static void __updata_load_stat(arm_2d_scene_t *ptScene,bool stat)
{
    user_scene_button_t *ptThis = (user_scene_button_t *)ptScene;
    ARM_2D_UNUSED(ptThis);
    this.Downloading = false;
    if(stat == true){
       user_navigation.chDownloadstat = 1;
    }else{
       user_navigation.chDownloadstat = 2;
    }

}

static void __on_scene_button_load(arm_2d_scene_t *ptScene)
{
    user_scene_button_t *ptThis = (user_scene_button_t *)ptScene;
    ARM_2D_UNUSED(ptThis);
    progress_bar_round_on_load(&this.tProgressBarRound);
    spin_zoom_widget_on_load(&this.tText);
    spin_zoom_widget_on_load(&this.tShadow);
    foldable_panel_on_load(&this.tPanel);
    arm_2d_helper_dirty_region_add_items(&this.use_as__arm_2d_scene_t.tDirtyRegionHelper,
                                         &this.tDirtyRegionItem,
                                         5);

    connect(&tPikaConfig, SIGNAL(pika_console_sig),  ptScene, SLOT(__console_box_printf));  
    connect(&tPikaConfig, SIGNAL(pika_progress_sig), ptScene, SLOT(__updata_progress_bar));
    connect(&tPikaConfig, SIGNAL(pika_loadstat_sig), ptScene, SLOT(__updata_load_stat));
    for(uint32_t i = 0; i<(800 / 8)-(104 / 8);i++){
       console_box_putchar(&this.tConsole, '\n');
    }
}

static void __after_scene_button_switching(arm_2d_scene_t *ptScene)
{
    user_scene_button_t *ptThis = (user_scene_button_t *)ptScene;
    ARM_2D_UNUSED(ptThis);
}

static void __on_scene_button_depose(arm_2d_scene_t *ptScene)
{
    user_scene_button_t *ptThis = (user_scene_button_t *)ptScene;
    ARM_2D_UNUSED(ptThis);
    progress_bar_round_depose(&this.tProgressBarRound);
    console_box_depose(&this.tConsole);
    foldable_panel_depose(&this.tPanel);
    spin_zoom_widget_depose(&this.tText);
    spin_zoom_widget_depose(&this.tShadow);

    arm_2d_helper_dirty_region_remove_items(&this.use_as__arm_2d_scene_t.tDirtyRegionHelper,
                                            this.tDirtyRegionItem,
                                            5);

    disconnect(&tPikaConfig, SIGNAL(pika_console_sig), ptScene, SLOT(__console_box_printf));  
    disconnect(&tPikaConfig, SIGNAL(pika_progress_sig), ptScene, SLOT(__updata_progress_bar));
    disconnect(&tPikaConfig, SIGNAL(pika_loadstat_sig), ptScene, SLOT(__updata_load_stat));
    ptScene->ptPlayer = NULL;
    
    arm_foreach(int64_t,this.lTimestamp, ptItem) {
        *ptItem = 0;
    }

    if (!this.bUserAllocated) {
        __arm_2d_free_scratch_memory(ARM_2D_MEM_TYPE_UNSPECIFIED, ptScene);
    }
}

/*----------------------------------------------------------------------------*
 * Scene button                                                               *
 *----------------------------------------------------------------------------*/

static void __on_scene_button_background_start(arm_2d_scene_t *ptScene)
{
    user_scene_button_t *ptThis = (user_scene_button_t *)ptScene;
    ARM_2D_UNUSED(ptThis);

}

static void __on_scene_button_background_complete(arm_2d_scene_t *ptScene)
{
    user_scene_button_t *ptThis = (user_scene_button_t *)ptScene;
    ARM_2D_UNUSED(ptThis);

}

static arm_fsm_rt_t __scene_frame_actions(arm_2d_scene_t *ptScene){
    user_scene_button_t *ptThis = (user_scene_button_t *)ptScene;
    int16_t iResult[4];
    ARM_2D_UNUSED(ptThis);
    ARM_PT_BEGIN(this.chPT)
    static uint8_t text_cnt;
    switch (this.chState[0]){
          case 0: 
               if(arm_2d_helper_time_half_cos_slider(0, 100, 1000, &iResult[0], &this.lTimestamp[1])){ 
                    this.chState[0] = 1;
                    this.lTimestamp[1] = 0;
              //    this.iProgress[1]  = 100;
                 }else{
                       this.iProgress[1] = iResult[0];    
                 }    
               break;
          case 1:
               if(arm_2d_helper_time_half_cos_slider(100, 0, 1000, &iResult[0], &this.lTimestamp[1])){
                  this.chState[0] = 0;
                  this.lTimestamp[1] = 0;
             //   this.iProgress[1]  = 0;
                }else{
                  this.iProgress[1] = iResult[0];
                }   
               break;
    }
    switch (this.chState[1]){
          case 0: 
               if(this.chItemCurrent == REGION_IDX_LINE_EDIT){
                  if(arm_2d_helper_time_half_cos_slider(0, 255, 300, &iResult[1], &this.lTimestamp[2])){
                    this.iProgress[2]  = 255;
                    arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[0],1);
                  }else{ 
                    arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[0],0);
                    this.iProgress[2] = iResult[1];
                  } 
                }else{
                  this.chState[1] = 1;
                  this.lTimestamp[2] = 0;
                }
               break;
          case 1:
              if(this.chItemCurrent != REGION_IDX_LINE_EDIT){
                 if(arm_2d_helper_time_half_cos_slider(255, 0, 300, &iResult[1], &this.lTimestamp[2])){
                    arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[0],1);
                    this.iProgress[2]  = 0;
                  }else{
                    arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[0],0);
                    this.iProgress[2] = iResult[1];
                  }   
              }else{
                  this.chState[1] = 0;
                  this.lTimestamp[2] = 0;
              }
              break;
     }
    switch (this.chState[2]){
          case 0: 
               if(this.chItemCurrent == REGION_IDX_TEXT_BOX){

                  if(arm_2d_helper_time_half_cos_slider(0, 255, 300, &iResult[2], &this.lTimestamp[3])){
                    arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[1],1);
                    this.iProgress[3]  = 255;
                  }else{ 
                    arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[1],0);
                    this.iProgress[3] = iResult[2];
                  } 
                  if(this.Checklog){
                     this.chState[2] = 2;
                     this.lTimestamp[3] = 0;
                  }

                }else{
                  this.chState[2] = 1;
                  this.lTimestamp[3] = 0;
                }
               break;
          case 1:
              if(this.chItemCurrent != REGION_IDX_TEXT_BOX){
                 if(arm_2d_helper_time_half_cos_slider(255, 0, 300, &iResult[2], &this.lTimestamp[3])){
                    arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[1],1);
                    this.iProgress[3]  = 0;
                  }else{
                    arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[1],0);
                    this.iProgress[3] = iResult[2];
                  }   
              }else{
                  this.chState[2] = 0;
                  this.lTimestamp[3] = 0;
              }
              break;
          case 2:
              if(arm_2d_helper_time_half_cos_slider(255, 0, 300, &iResult[2], &this.lTimestamp[3])){
                    arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[1],1);
                    this.iProgress[3]  = 0;
                  }else{
                    arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[1],0);
                    this.iProgress[3] = iResult[2];
                  }   
              
              if(!this.Checklog){
                 this.chState[2] = 0;
                 this.lTimestamp[3] = 0;
              }
              break;
     }
    switch (this.chState[3]){
          case 0: 
               if(this.chItemCurrent == REGION_IDX_TEXT_DOWNLOAD){
                  if(arm_2d_helper_time_half_cos_slider(0, 255, 300, &iResult[3], &this.lTimestamp[4])){
                    arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[2],1);
                    this.iProgress[4]  = 255;
                  }else{ 
                    arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[2],0);
                    this.iProgress[4] = iResult[3];
                  } 
                }else{
                  this.chState[3] = 1;
                  this.lTimestamp[4] = 0;
                }
               break;
          case 1:
              if(this.chItemCurrent != REGION_IDX_TEXT_DOWNLOAD){
                 if(arm_2d_helper_time_half_cos_slider(255, 0, 300, &iResult[3], &this.lTimestamp[4])){
                    arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[2],1);
                    this.iProgress[4]  = 0;
                  }else{
                    arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[2],0);
                    this.iProgress[4] = iResult[3];
                  }   
              }else{
                  this.chState[3] = 0;
                  this.lTimestamp[4] = 0;
              }
              break;
     }
    if(this.Downloading){
       arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[2],0);
    }

    ARM_PT_END();
    return arm_fsm_rt_cpl;
}

static void __on_scene_button_frame_start(arm_2d_scene_t *ptScene)
{
    user_scene_button_t *ptThis = (user_scene_button_t *)ptScene;
    ARM_2D_UNUSED(ptThis);
    int32_t iResult[2];
    if (arm_2d_helper_time_half_cos_slider(0, 100, 1000, &iResult[0], &this.lTimestamp[0])) {
        this.iProgress[0] = 0;
        this.lTimestamp[0] = 0;
        foldable_panel_unfold(&this.tPanel);
    } else {
        this.iProgress[0] = (uint16_t)iResult[0];
    }
    __scene_frame_actions(ptScene);

    //if(this.downloadtimes < g_tMicroInfo.OffLineLoadNum){
    //   this.Downloading = false;
    //}
    if(this.Downloading || this.Checklog){
       arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[1],0);
    }

     spin_zoom_widget_on_frame_start_xy(&this.tText, 0,0.8,0.8);                                //Microlink logo
     if(!this.Downloading){
        spin_zoom_widget_on_frame_start(&this.tShadow, 0,(float)(this.iProgress[1]/500.0 + 0.5));  //阴影缩放
     }else{
        spin_zoom_widget_on_frame_start(&this.tShadow, 0,(float)(0.5));  //阴影缩放
     }


    RG_PROCESS_MSG(this.ptGUI, ptMSG) {
        this.lTimestamp[5] = 0;
        ARM_2D_UNUSED(bIsMessageHandled);
        switch(ngy_helper_msg_item_get_id(ptMSG)) {
            case NGY_MSG_GESTURE_EVT_WHEEL:
            do {
                ngy_msg_gesture_evt_t *ptKeyMSG = (ngy_msg_gesture_evt_t *)ptMSG;
                 if(!user_navigation.SlideOut && !this.Checklog){
                    if(ptKeyMSG->tRegion.tLocation.iY > this.iY){
                         if(--this.chItemCurrent <= REGION_IDX_LINE_EDIT){
                            this.chItemCurrent = REGION_IDX_LINE_EDIT;
                         }
                         if(this.bButtonChecked == true && this.chItemCurrent == REGION_IDX_TEXT_BOX){
 
                         }
                    }else{
                       
                         if(++this.chItemCurrent >= REGION_IDX_TEXT_DOWNLOAD){
                            this.chItemCurrent = REGION_IDX_TEXT_DOWNLOAD;

                         }
                         if(this.bButtonChecked == true && this.chItemCurrent == REGION_IDX_TEXT_BOX){
                   
                         }
                    }
                  }else{
                      if(ptKeyMSG->tRegion.tLocation.iY > this.iY){  //悬浮菜单
                         navi_operate(WHEEL_UP);
                      }else{
                         navi_operate(WHEEL_DOWN);
                      }
                  }
                if(this.Checklog){
                   this.Console_iY += (ptKeyMSG->tRegion.tLocation.iY - this.iY);
                }
                this.iY = ptKeyMSG->tRegion.tLocation.iY;
                bIsMessageHandled = true;
            }while(0);
            break;
            /* touch up */
            case NGY_MSG_KEY_EVT_PRESSED: 
                if(!user_navigation.SlideOut){
                    if(this.bButtonChecked == false){
                        this.bButtonChecked = true;
                    }else{
                       this.bButtonChecked = false;
                    }
                    if(this.chItemCurrent == REGION_IDX_LINE_EDIT){
                        arm_2d_scene_player_switch_to_next_scene(ptScene->ptPlayer);
                    }
                    if(this.chItemCurrent == REGION_IDX_TEXT_BOX){
                        if(this.Checklog){
                           this.Checklog = false;
                        }else{
                           this.Checklog = true;
                        }
                    }

                    if(this.chItemCurrent == REGION_IDX_TEXT_DOWNLOAD){
                      if(this.chFilePath[0] != '\0'){
                        user_scene_button_t *ptButton = ptThis;
                        this.Downloading = true;
                        publish(&g_tMicroInfo.tSubPub, __MSG_TOPIC(download_topic), (char *)&ptButton->chFilePath);         
                        }else{    
                           user_navigation.chNofileTips = true;
                        }
                    }

                }else{
                   navi_operate(SINGLE_PRESS);
                   if(icon_list_get_selected_item_id(&user_navigation.tList) != ICON_IDX_DOWNLOAD){
                      arm_2d_scene_player_switch_to_next_scene(ptScene->ptPlayer);
                   }
                }
                    bIsMessageHandled = true;
                break;
            case NGY_MSG_KEY_EVT_LONG_PRESSING:  
                 this.bButtonChecked = false;
                 bIsMessageHandled = true;
                 navi_operate(LONG_PRESS);
               //arm_2d_scene_player_switch_to_next_scene(ptScene->ptPlayer);
                break; 
            default:
             if(arm_2d_helper_is_time_out(3000)){  
             }
                 bIsMessageHandled = false;
                break;
        }
    }
    foldable_panel_on_frame_start(&this.tPanel);
    progress_bar_round_on_frame_start(&this.tProgressBarRound);
    console_box_on_frame_start(&this.tConsole);
}


static void __on_scene_button_frame_complete(arm_2d_scene_t *ptScene)
{
    user_scene_button_t *ptThis = (user_scene_button_t *)ptScene;
    ARM_2D_UNUSED(ptThis);
    static bool fold_dr_flag = false;
    progress_bar_round_on_frame_complete(&this.tProgressBarRound);
    spin_zoom_widget_on_frame_complete(&this.tText);
    spin_zoom_widget_on_frame_complete(&this.tShadow);
    foldable_panel_on_frame_complete(&this.tPanel);

    if(foldable_panel_status(&this.tPanel) == FOLDABLE_PANEL_STATUS_FOLDING 
     ||foldable_panel_status(&this.tPanel) == FOLDABLE_PANEL_STATUS_UNFOLDING){
        arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[4],0);
    }

    if(foldable_panel_status(&this.tPanel) == FOLDABLE_PANEL_STATUS_FOLDED 
     ||foldable_panel_status(&this.tPanel) == FOLDABLE_PANEL_STATUS_UNFOLDED){
        //arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[4],1);
    }
    //  arm_2d_helper_dirty_region_item_suspend_update(&this.tDirtyRegionItem[4],1);   //没处理好
      
    
      

#if 0
    /* switch to next scene after 3s */
    if (arm_2d_helper_is_time_out(3000, &this.lTimestamp[0])) {
        arm_2d_scene_player_switch_to_next_scene(ptScene->ptPlayer);
    }
#endif
}

static void __before_scene_button_switching_out(arm_2d_scene_t *ptScene)
{
    user_scene_button_t *ptThis = (user_scene_button_t *)ptScene;
    ARM_2D_UNUSED(ptThis);

    arm_2d_scene_player_set_switching_mode( 
                    &DISP0_ADAPTER,
                    ARM_2D_SCENE_SWITCH_MODE_SLIDE_UP);    

    arm_2d_scene_player_set_switching_period(
            &DISP0_ADAPTER, 
            500);	

     if(this.chItemCurrent == REGION_IDX_LINE_EDIT && !user_navigation.SlideOut){
        user_scene_text_tracking_list_t *ptTextTrackingList = 
            arm_2d_scene_text_tracking_list_init(this.use_as__arm_2d_scene_t.ptPlayer);
        assert(NULL != ptTextTrackingList);
        ptTextTrackingList->ptGUI = &g_tMyGUI;
     }else if(user_navigation.SlideOut){
            switch(icon_list_get_selected_item_id(&user_navigation.tList)){
            case 0:
              break;
            case 1:
                extern user_scene_waveform_t *ptwaveform;
                ptwaveform = 
                arm_2d_scene_waveform_init(this.use_as__arm_2d_scene_t.ptPlayer);
                assert(NULL != ptwaveform);
                ptwaveform->ptGUI = &g_tMyGUI;  
              break;
            case 2:
                user_scene_menu_t *ptMenu = 
                arm_2d_scene_menu_init(this.use_as__arm_2d_scene_t.ptPlayer);
                assert(NULL != ptMenu);
                ptMenu->ptGUI = &g_tMyGUI;  
              break; 
           }
      }
      
}     

void draw_Label(arm_2d_tile_t *ptTile,arm_2d_region_t *ptRegion,const char *pchString,COLOUR_INT tColour,uint8_t chOpacity)
{
    arm_2d_container(ptTile, __label, ptRegion) {
        arm_lcd_text_set_target_framebuffer((arm_2d_tile_t *)&__label);
        arm_lcd_text_set_font((arm_2d_font_t *)&ARM_2D_FONT_Round16_A4);
        arm_lcd_text_set_opacity(chOpacity);
        arm_lcd_text_set_colour(tColour, GLCD_COLOR_BLACK);
        arm_print_banner(pchString, __label_canvas);
    }
}

static void draw_buttom(const arm_2d_tile_t *ptTile, 
                        arm_2d_region_t *ptRegion,
                        const char *pchString,
                        COLOUR_INT tColour,
                        uint8_t chOpacity,
                        uint8_t bButtonChecked)
{
    arm_2d_container(ptTile, __button, ptRegion) {

        draw_round_corner_box(&__button, NULL, GLCD_COLOR_WHITE, chOpacity);
        ARM_2D_OP_WAIT_ASYNC();
        if (bButtonChecked) {
            draw_round_corner_border(   &__button, 
                                        NULL, 
                                        GLCD_COLOR_WHITE, 
                                        (arm_2d_border_opacity_t)
                                            {255-64, 255-64, 32, 32},
                                        (arm_2d_corner_opacity_t)
                                            {255-64, 255-64, 32, 0});
        } else {
            draw_round_corner_border(   &__button, 
                                        NULL, 
                                        GLCD_COLOR_WHITE, 
                                        (arm_2d_border_opacity_t)
                                            {32, 32, 255-64, 255-64},
                                        (arm_2d_corner_opacity_t)
                                            {0, 128, 128, 128});
        }
        ARM_2D_OP_WAIT_ASYNC();
        arm_lcd_text_set_target_framebuffer((arm_2d_tile_t *)&__button);
        arm_lcd_text_set_font((arm_2d_font_t *)&ARM_2D_FONT_Round16_A4);
        arm_lcd_text_set_opacity(chOpacity);
        arm_lcd_text_set_colour(tColour, GLCD_COLOR_BLACK);
        arm_print_banner(pchString, __button_canvas);
        arm_lcd_text_set_opacity(255);
        
    }
}
static const char *get_filename(const char *path) {
    const char *filename = path;
    while (*path) {
        if (*path == '/' || *path == '\\')
            filename = path + 1;
        path++;
    }
    return filename;
}
static
IMPL_PFB_ON_DRAW(__pfb_draw_scene_button_handler)
{
    ARM_2D_PARAM(pTarget);
    ARM_2D_PARAM(ptTile);
    ARM_2D_PARAM(bIsNewFrame);
    user_scene_button_t *ptThis = (user_scene_button_t *)pTarget;
    arm_2d_size_t tScreenSize = ptTile->tRegion.tSize;
    ARM_2D_UNUSED(tScreenSize);
    arm_2d_canvas(ptTile, __top_canvas) {

    arm_2d_region_t  __Download_region = {
      .tLocation.iX  = 10,
      .tLocation.iY  = 180,
      .tSize.iWidth  = 220, 
      .tSize.iHeight = 60,
    };

           arm_2d_helper_dirty_region_update_item(&this.tDirtyRegionItem[2],
                                                  (arm_2d_tile_t *)ptTile,
                                                  NULL,
                                                  &__Download_region);  

          arm_2d_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                      &__Download_region,
                                                      &c_tileBarFrameA4Mask,
                                                      (arm_2d_color_rgb565_t){GLCD_COLOR_ORANGE},
                                                      192); 

          arm_2d_region_t  __progress_region = {
            .tLocation.iX  = __Download_region.tLocation.iX,
            .tLocation.iY  = __Download_region.tLocation.iY,
            .tSize.iWidth  = this.downloadprogress * 2.2, 
            .tSize.iHeight = __Download_region.tSize.iHeight,
          };

          arm_2d_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                          &__progress_region,
                                                          &c_tileBarFrameA4Mask,
                                                          (arm_2d_color_rgb565_t){GLCD_COLOR_B_GREEN},
                                                          255); 




          arm_2d_rgb565_fill_colour_with_4pts_alpha_gradient_mask_and_opacity(ptTile,
                                                                              &__Download_region,
                                                                              &c_tileBarFrameFillMask,
                                                                              (arm_2d_color_rgb565_t){GLCD_COLOR_ORANGE},
                                                                              192,
                                                                              (arm_2d_alpha_samples_4pts_t)
                                                                              {{this.iProgress[4] ,this.iProgress[4]/2,
                                                                                this.iProgress[4]/3, this.iProgress[4]/4}}
                                                                              );
    /*-----------------------draw the foreground begin-----------------------*/
        /* following code is just a demo, you can remove them */
        arm_2d_align_centre(__top_canvas, s_tDirtyRegions[REGION_IDX_BUTTON].tRegion.tSize ) { 
            arm_2d_layout(__centre_region) {
               __item_line_vertical(218,45,0,0,14,0) {
               arm_2d_helper_dirty_region_update_item(&this.tDirtyRegionItem[0],
                                                      (arm_2d_tile_t *)ptTile,
                                                      NULL,
                                                      &__item_region);   

                    arm_2d_rgb565_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                                    &__item_region,
                                                                    &c_tileFileFrameA4Mask,
                                                                    (arm_2d_color_rgb565_t){GLCD_COLOR_ORANGE},
                                                                    192); 
                    arm_2d_rgb565_fill_colour_with_4pts_alpha_gradient_mask_and_opacity(ptTile,
                                                                                        &__item_region,
                                                                                        &c_tileFileFrameFillMask,
                                                                                        (arm_2d_color_rgb565_t){GLCD_COLOR_ORANGE},
                                                                                        192,
                                                                                        (arm_2d_alpha_samples_4pts_t)
                                                                                        {{this.iProgress[2] ,this.iProgress[2]/2,
                                                                                          this.iProgress[2]/3, this.iProgress[2]/4}}
                                                                                        );
                    arm_lcd_text_set_target_framebuffer(ptTile);
                    arm_lcd_text_set_font((arm_2d_font_t *)&ARM_2D_FONT_Round16_A4);
                    arm_lcd_text_set_draw_region(&__item_region);
                    
                    if(this.chFilePath[0] == '\0'){
                        arm_lcd_text_set_opacity(192);
                        arm_lcd_text_set_colour(GLCD_COLOR_ORANGE, GLCD_COLOR_BLACK);
                        arm_lcd_printf_label(ARM_2D_ALIGN_CENTRE,"选择烧录文件"); 
                    }else{
                        const char *filename = get_filename(this.chFilePath);
                        arm_lcd_text_set_opacity(192);
                        arm_lcd_text_set_colour(GLCD_COLOR_B_GREEN, GLCD_COLOR_BLACK);
                        arm_lcd_printf_label(ARM_2D_ALIGN_CENTRE,filename);       
                    }
               }  

              __item_line_vertical(220, 125,0,0,-7,0) {
                   arm_2d_helper_dirty_region_update_item(&this.tDirtyRegionItem[1],
                                                          (arm_2d_tile_t *)ptTile,
                                                          NULL,
                                                          &__item_region);   

                    arm_2d_rgb565_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                                    &__item_region,
                                                                    &c_tileConsoleFrameA4Mask,
                                                                    (arm_2d_color_rgb565_t){GLCD_COLOR_B_GREEN},
                                                                    192); 
                    arm_2d_rgb565_fill_colour_with_4pts_alpha_gradient_mask_and_opacity(ptTile,
                                                                                        &__item_region,
                                                                                        &c_tileConsoleFrameFillMask,
                                                                                         (arm_2d_color_rgb565_t){GLCD_SELECTED_GREEN},
                                                                                        192,
                                                                                        (arm_2d_alpha_samples_4pts_t)
                                                                                        {{this.iProgress[3] ,this.iProgress[3]/2,
                                                                                          this.iProgress[3]/3, this.iProgress[3]/4}}
                                                                                        );

                  arm_2d_dock_with_margin(__item_region, 10) {
                     arm_2d_container(ptTile, __console, &__dock_region) {
                        static int16_t offset = 0;
                        arm_2d_region_t tConsoleBox = {
                            .tLocation.iY = (offset + this.Console_iY) * 8 - 800 + __dock_region.tSize.iHeight,
                            .tSize = {200, 800},
                        };
                        if(tConsoleBox.tLocation.iY < (- 800 + __dock_region.tSize.iHeight)){
                            offset = -this.Console_iY;
                        }
                        console_box_show(&this.tConsole,
                                &__console,
                                &tConsoleBox,
                                bIsNewFrame,
                                255); 

                        arm_2d_region_t  __text_region = {
                          .tLocation.iX  = __dock_region.tLocation.iX,
                          .tLocation.iY  = __dock_region.tLocation.iY,
                          .tSize.iWidth  = __dock_region.tSize.iWidth,
                          .tSize.iHeight = __dock_region.tSize.iHeight,
                        };

                         if(!this.Downloading){
                            __text_region.tLocation.iY = __dock_region.tLocation.iY + this.iProgress[1]/10 - 10;
                         }

                        arm_2d_region_t  __shadow_region = {
                          .tLocation.iX  = __dock_region.tLocation.iX,
                          .tLocation.iY  = __dock_region.tLocation.iY + 20,
                          .tSize.iWidth  = __dock_region.tSize.iWidth,
                          .tSize.iHeight = __dock_region.tSize.iHeight,
                        };

                        arm_2d_region_t  __panel_region = {
                          .tLocation.iX  = __dock_region.tLocation.iX + 10,
                          .tLocation.iY  = __dock_region.tLocation.iY + 15,
                          .tSize.iWidth  = __dock_region.tSize.iWidth -  30,
                          .tSize.iHeight = __dock_region.tSize.iHeight - 25,
                        };

                        arm_2d_region_t  __dirty_region = {
                          .tLocation.iX  = __panel_region.tLocation.iX,
                          .tLocation.iY  = __panel_region.tLocation.iY,
                          .tSize.iWidth  = __panel_region.tSize.iWidth, 
                          .tSize.iHeight = __panel_region.tSize.iHeight + 20,
                        };

                        arm_2d_helper_dirty_region_update_item(&this.tDirtyRegionItem[4],
                                                               ptTile,
                                                               NULL,
                                                               &__dirty_region); 
 
                        arm_2d_tile_t *ptPanel = 
                            foldable_panel_show(&this.tPanel,
                                                ptTile, 
                                                &__panel_region,
                                                bIsNewFrame);

                        ARM_2D_OP_WAIT_ASYNC();
                        assert(NULL != ptPanel);
                        arm_2d_canvas(ptPanel, __top_canvas) {
                            arm_2d_align_centre(__top_canvas, __text_region.tSize) {
                              spin_zoom_widget_show(&this.tText,   ptTile, &__text_region,NULL,32);

                                if(!this.Downloading){
                                  spin_zoom_widget_show(&this.tShadow, ptTile, &__shadow_region,NULL,12 + this.iProgress[1]/2);
                                }else{
                                  spin_zoom_widget_show(&this.tShadow, ptTile, &__shadow_region,NULL,32);
                                }
                            }
                        }
                     }
                  }                     
               }
              




              __item_line_vertical(240,80,0,0,-6,0) {
                   arm_2d_layout(__item_region) {



                    ARM_2D_OP_WAIT_ASYNC();
                    __item_line_horizontal(45,45, 4, 2, 4, 2){  
                    arm_2d_rgb565_fill_colour_with_mask_and_opacity(ptTile,
                                                                    &__item_region,
                                                                    &c_tilestart_button_fillMask,
                                                                    (arm_2d_color_rgb565_t){GLCD_COLOR_BACKGROUND},
                                                                    192); 


                    arm_2d_rgb565_fill_colour_with_4pts_alpha_gradient_mask_and_opacity(ptTile,
                                                                                        &__item_region,
                                                                                        &c_tilestart_button_fillMask,
                                                                                        (arm_2d_color_rgb565_t){GLCD_COLOR_BACKGROUND},
                                                                                        192,
                                                                                        (arm_2d_alpha_samples_4pts_t)
                                                                                        {{this.iProgress[4]     ,this.iProgress[4]/2,
                                                                                          this.iProgress[4]/2   ,this.iProgress[4]}}
                                                                                        );
                    arm_2d_rgb565_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                                    &__item_region,
                                                                    &c_tilestart_buttonA4Mask,
                                                                    (arm_2d_color_rgb565_t){GLCD_COLOR_B_GREEN},
                                                                    192); 

                    arm_2d_region_t Arrow_region = {
                      .tLocation.iX = __item_region.tLocation.iX + 10,
                      .tLocation.iY = __item_region.tLocation.iY + 5,
                      .tSize.iWidth  = c_tileDownloadButtonA4Mask.tRegion.tSize.iWidth,
                      .tSize.iHeight = c_tileDownloadButtonA4Mask.tRegion.tSize.iHeight,
                    };
                    arm_2d_region_t Process_region = {
                      .tLocation.iX = __item_region.tLocation.iX + 5,
                      .tLocation.iY = __item_region.tLocation.iY + 10,
                      .tSize.iWidth  = 30,
                      .tSize.iHeight = 16,
                    };

                      if(this.Downloading == false){
                      arm_2d_rgb565_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                                      &Arrow_region,
                                                                      &c_tileDownloadButtonA4Mask,
                                                                      (arm_2d_color_rgb565_t){GLCD_COLOR_B_GREEN},
                                                                      192);

                      }else{
                        arm_lcd_text_set_target_framebuffer(ptTile);
                        arm_lcd_text_set_colour(GLCD_COLOR_B_GREEN, GLCD_COLOR_BLACK);
                        arm_lcd_text_set_opacity(192);
                        arm_lcd_text_set_font(&ARM_2D_FONT_LiberationSansRegular14_A2);
                        arm_lcd_text_set_display_mode(ARM_2D_DRW_PATN_MODE_COPY);
                        arm_lcd_text_set_draw_region(&Process_region);
                        arm_lcd_printf("%d%%",this.downloadprogress);   
                      }

                    }
                    __item_line_horizontal(160,18, 0, 2, 24, 40){ 

                    arm_lcd_text_set_target_framebuffer(ptTile);
                    arm_lcd_text_set_colour(GLCD_COLOR_ORANGE, GLCD_COLOR_BLACK);
                    arm_lcd_text_set_opacity(192);
                    arm_lcd_text_set_font(&ARM_2D_FONT_Round16_A4);
                    arm_lcd_text_set_display_mode(ARM_2D_DRW_PATN_MODE_COPY);
                    arm_lcd_text_set_draw_region(&__item_region);


                    arm_2d_region_t num_region = {
                       .tLocation.iX = 157,
                       .tLocation.iY = 202,
                       .tSize.iWidth  = 45,
                       .tSize.iHeight = 16,
                    };

                    arm_2d_helper_dirty_region_update_item( &this.tDirtyRegionItem[3],
                                                          (arm_2d_tile_t *)ptTile,
                                                          &__centre_region,
                                                          &num_region);   

                    __item_line_dock_vertical(20, 20, 0, 17, 0) {
                           arm_lcd_text_set_draw_region(&__item_region);                        
                           arm_lcd_text_set_colour(GLCD_COLOR_ORANGE,GLCD_COLOR_BLACK);                                                                                                                                      
                           arm_lcd_printf("烧录次数: %d",g_tMicroInfo.OffLineLoadNum);   
                     
                     }

                    }
                  }
              }     
               
        }
       
    /*-----------------------draw the foreground end  -----------------------*/
       }
    ARM_2D_OP_WAIT_ASYNC();
    }
    return arm_fsm_rt_cpl;
}

ARM_NONNULL(1)
user_scene_button_t *__arm_2d_scene_button_init(   arm_2d_scene_player_t *ptDispAdapter, 
                                        user_scene_button_t *ptThis)
{
    bool bUserAllocated = false;
    assert(NULL != ptDispAdapter);

    s_tDirtyRegions[dimof(s_tDirtyRegions)-1].ptNext = NULL;

    /* get the screen region */
    arm_2d_region_t tScreen
        = arm_2d_helper_pfb_get_display_area(
            &ptDispAdapter->use_as__arm_2d_helper_pfb_t);
    
    /* initialise dirty region 0 at runtime
     * this demo shows that we create a region in the centre of a screen(320*240)
     * for a image stored in the tile c_tileCMSISLogoMask
     */
    arm_2d_align_centre(tScreen, s_tDirtyRegions[REGION_IDX_BUTTON].tRegion.tSize) {
        s_tDirtyRegions[REGION_IDX_BUTTON].tRegion = __centre_region;
    }

    if (NULL == ptThis) {
        ptThis = (user_scene_button_t *)
                    __arm_2d_allocate_scratch_memory(   sizeof(user_scene_button_t),
                                                        __alignof__(user_scene_button_t),
                                                        ARM_2D_MEM_TYPE_UNSPECIFIED);
        assert(NULL != ptThis);
        if (NULL == ptThis) {
            return NULL;
        }
    } else {
        bUserAllocated = true;
    }

    memset(ptThis, 0, sizeof(user_scene_button_t));

    *ptThis = (user_scene_button_t){
        .use_as__arm_2d_scene_t = {

            /* the canvas colour */
            .tCanvas = {GLCD_COLOR_BACKGROUND}, 

            /* Please uncommon the callbacks if you need them
             */
            .fnOnLoad       = &__on_scene_button_load,
            .fnScene        = &__pfb_draw_scene_button_handler,
            //.fnAfterSwitch  = &__after_scene_button_switching,

            /* if you want to use predefined dirty region list, please uncomment the following code */
            //.ptDirtyRegion  = (arm_2d_region_list_item_t *)s_tDirtyRegions,
            

            //.fnOnBGStart    = &__on_scene_button_background_start,
            //.fnOnBGComplete = &__on_scene_button_background_complete,
            .fnOnFrameStart = &__on_scene_button_frame_start,
            .fnBeforeSwitchOut = &__before_scene_button_switching_out,
            .fnOnFrameCPL   = &__on_scene_button_frame_complete,
            .fnDepose       = &__on_scene_button_depose,

            .bUseDirtyRegionHelper = true,
        },
        .bUserAllocated = bUserAllocated,
    };

    /* ------------   initialize members of user_scene_button_t begin ---------------*/
    do {
        progress_bar_round_cfg_t tCFG = {
            .ptScene = &ptThis->use_as__arm_2d_scene_t,
            .ValueRange = {
                .iMin = 0,
                .iMax = 100,
            },
            
        };

        progress_bar_round_init(&this.tProgressBarRound, &tCFG);
    } while(0);
    /* ------------   initialize members of user_scene_console_window_t begin ---------------*/
    do {
        static uint8_t s_chInputBuffer[512];
        static uint8_t s_chConsoleBuffer[(200 / 6) * (800 / 8)];
        console_box_cfg_t tCFG = {
            .tBoxSize = {200, 800},

            .pchConsoleBuffer = s_chConsoleBuffer,
            .hwConsoleBufferSize = sizeof(s_chConsoleBuffer),
           
            .pchInputBuffer = s_chInputBuffer,
            .hwInputBufferSize = sizeof(s_chInputBuffer),
            .tColor = GLCD_COLOR_ORANGE,
            .bUseDirtyRegion = false,
        };

        console_box_init(   &this.tConsole, 
                            &this.use_as__arm_2d_scene_t, 
                            &tCFG);
    } while(0);
    /* ------------   initialize members of user_scene_button_t end   ---------------*/
            arm_2d_location_t c_tCentre = {
              .iX = (c_tilemicrolinkMask.tRegion.tSize.iWidth  >> 1),
              .iY = (c_tilemicrolinkMask.tRegion.tSize.iHeight >> 1),
            };

            arm_2d_location_t c_tCentre1 = {
              .iX = (c_tileTextShadowMask.tRegion.tSize.iWidth  >> 1),
              .iY = (c_tileTextShadowMask.tRegion.tSize.iHeight >> 1),
            };

    do {
        spin_zoom_widget_cfg_t tCFG = {
            .Indicator = {
                .LowerLimit = {
                    .fAngleInDegree = 0.0f,
                    .nValue = 0,
                },
                .UpperLimit = {
                    .fAngleInDegree = 360.0f,
                    .nValue = 3600,
                },
                .Step = {
                    .fAngle = 0.0f,  //! 0.0f means very smooth, 1.0f looks like mech watches, 6.0f looks like wall clocks
                },
            },
            .ptTransformMode = &SPIN_ZOOM_MODE_FILL_COLOUR,
            .Source = {
                .ptMask = &c_tilemicrolinkMask,
                .tCentre = c_tCentre,
                .tColourToFill = GLCD_COLOR_B_GREEN,
            },
            .ptScene = (arm_2d_scene_t *)ptThis,
        };
        spin_zoom_widget_init(&this.tText, &tCFG);
    } while(0);

    do {
        spin_zoom_widget_cfg_t tCFG = {
            .Indicator = {
                .LowerLimit = {
                    .fAngleInDegree = 0.0f,
                    .nValue = 0,
                },
                .UpperLimit = {
                    .fAngleInDegree = 360.0f,
                    .nValue = 3600,
                },
                .Step = {
                    .fAngle = 0.0f,  //! 0.0f means very smooth, 1.0f looks like mech watches, 6.0f looks like wall clocks
                },
            },
            .ptTransformMode = &SPIN_ZOOM_MODE_FILL_COLOUR,
            .Source = {
                .ptMask = &c_tileTextShadowMask,
                .tCentre = c_tCentre1,
                .tColourToFill = GLCD_COLOR_B_GREEN,
            },
            .ptScene = (arm_2d_scene_t *)ptThis,
        };
        spin_zoom_widget_init(&this.tShadow, &tCFG);
    } while(0);

    do{
        foldable_panel_cfg_t tCFG = {
            .bShowScanLines = true,
            .ptScene = &this.use_as__arm_2d_scene_t,
            .tLineColour.tColour = GLCD_COLOR_B_GREEN,
            .u12VerticalFoldingTimeInMS   = 300,
            .u12HorizontalFoldingTimeInMS = 300,
        };
        foldable_panel_init(&this.tPanel, &tCFG);
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


