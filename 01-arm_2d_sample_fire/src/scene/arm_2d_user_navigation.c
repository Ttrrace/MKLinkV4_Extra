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
#include "arm_2d_user_navigation.h"
#include <stdlib.h>
#include <string.h>
#include "microlink_board.h"
#include "rtthread.h"

/*============================ MACROS ========================================*/
                         
/*============================ MACROFIED FUNCTIONS ===========================*/
#undef this
#define this (*ptThis)
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
extern const arm_2d_tile_t c_tileSmallFlashA4Mask;
extern const arm_2d_tile_t c_tileDisconnectedA4Mask;
extern const arm_2d_tile_t c_tileConnectedA4Mask;
extern const arm_2d_tile_t c_tileIslandFillA4Mask;
extern const arm_2d_tile_t c_tileIslandA4Mask;
extern const arm_2d_tile_t c_tileSlideBarA4Mask;

extern const arm_2d_tile_t c_tileWindowFrameA4Mask;
extern const arm_2d_tile_t c_tileWindowFillA4Mask;

extern 
const
struct {
    implement(arm_2d_user_font_t);
    arm_2d_char_idx_t tUTF8Table2;
}ARM_2D_FONT_Round16_A4,
 ARM_2D_FONT_Round16_A2;


#define GLCD_WECHAT_GREEN       __RGB( 0X06, 0XC7, 0X63)
#define GLCD_COLOR_B_GREEN      __RGB( 0X5D, 0XF5, 0XFE)
#define GLCD_COLOR_BACKGROUND   __RGB(    0, 0x0c, 0x18)

#define RGB_INVERT(r,g,b)  __RGB(255-(r), 255-(g), 255-(b))

#define GLCD_COLOR_ORANGE_INV  RGB_INVERT(255,128,0)


#define c_tileDownloadMask          c_tileSmall_downloadA1Mask
#define c_tileScopeMask             c_tileSmall_scopeA1Mask
#define c_tileInfoMask              c_tileSmall_infoA1Mask


extern const arm_2d_tile_t c_tileDownloadMask;
extern const arm_2d_tile_t c_tileScopeMask;
extern const arm_2d_tile_t c_tileInfoMask;

static
const arm_2d_tile_t *c_pIconTable[] = {
    [ICON_IDX_DOWNLOAD] =   &c_tileDownloadMask,
    [ICON_IDX_SCOPE] =      &c_tileScopeMask,
    [ICON_IDX_INFO] =       &c_tileInfoMask,

};

static const char *c_pIconNameTable[] = {
    [ICON_IDX_DOWNLOAD] =   "Download",
    [ICON_IDX_SCOPE] =      "Scope",
    [ICON_IDX_INFO] =       "DeviceInfo",
};

static const char *c_pComNameTable[] = {
    [com_none] =   "None",
    [com_cmd] =    "CMD",
    [com_uart] =   "UART",
    [com_rs485] =  "485",
};

extern 
const
struct {
    implement(arm_2d_user_font_t);
    arm_2d_char_idx_t tUTF8Table2;
}ARM_2D_FONT_Round12_A4,
 ARM_2D_FONT_Round12_A2;

    arm_2d_region_t Island_region = {
       .tLocation.iX = 190,
       .tLocation.iY = 70,
       .tSize.iWidth  = 40,
       .tSize.iHeight = 120,
    };

    arm_2d_region_t Selected_region = {
       .tLocation.iX = 193,
       .tLocation.iY = 109,
       .tSize.iWidth  = 32,
       .tSize.iHeight = 32,
    };   

IMPL_ARM_2D_REGION_LIST(s_tUserNavDirtyRegionList, static)
    /* a region for the status bar on the bottom of the screen */
    ADD_REGION_TO_LIST(s_tUserNavDirtyRegionList,
        .tSize = {
            .iWidth  = 60,
            .iHeight = 120,
        },
    ),
    ADD_REGION_TO_LIST(s_tUserNavDirtyRegionList,
        .tSize = {
            .iWidth  = 125,
            .iHeight = 30,
        },
    ),
    ADD_REGION_TO_LIST(s_tUserNavDirtyRegionList,
        .tSize = {
            .iWidth  = 125,
            .iHeight = 30,
        },
    ),
    ADD_REGION_TO_LIST(s_tUserNavDirtyRegionList,
        .tSize = {
            .iWidth  = 125,
            .iHeight = 30,
        },
    ),
    ADD_REGION_TO_LIST(s_tUserNavDirtyRegionList,
        .tSize = {
            .iWidth  = 125,
            .iHeight = 30,
        },
    ),

    ADD_LAST_REGION_TO_LIST(s_tUserNavDirtyRegionList,
        .tSize = {
            .iWidth = 240,
            .iHeight = 24,
        },
    ),
END_IMPL_ARM_2D_REGION_LIST(s_tUserNavDirtyRegionList)

void disp_adapter0_user_navigator_init(void);
user_navigation_t user_navigation;

void navi_operate(uint8_t stat)
{
   if(!user_navigation.SlideOut){
      if(stat == LONG_PRESS){
         user_navigation.operation = stat;
      }
   }else if(stat <= 5){
      user_navigation.operation = stat;
   }
}

static arm_fsm_rt_t __scene_frame_actions(user_navigation_t *user_navigation){
    user_navigation_t *ptThis = (user_navigation_t *)user_navigation;
    int16_t iResult[12];
    ARM_2D_UNUSED(ptThis);
    ARM_PT_BEGIN(this.chPT)
    static uint8_t text_cnt;
    /*****TEST*******/
     //if(user_navigation->operation == SINGLE_PRESS){
     //   this.New_window_in = 2;
     //   this.New_message_type = CHOOSE_FIEMWARE0;
     //   user_navigation->operation = NONE;
     //   for(uint8_t i = 0;i < 5;i ++){
     //      this.float_window[i].iscompelete = false;
     //   }
     //}


    /****************/
    switch (this.chState[0]){
          case 0:
                 if(user_navigation->operation == LONG_PRESS){  //长按弹出
                    this.chState[0] = 1;
                  }
               break;
          case 1:
                 if(arm_2d_helper_time_half_cos_slider(0, 40, 500, &iResult[0], &this.lTimestamp[0]))
                 { 
                    this.chState[0] = 2;
                    this.lTimestamp[0] = 0;
                    this.iProgress[0]  = 40;
                    user_navigation->SlideOut  = true;
                    user_navigation->operation = NONE;
                    break;
                 }else{
                    this.iProgress[0] = iResult[0];         //
                 }
               break;
          //case 2: 
          //     if(user_navigation->operation == WAIT){
          //         if(arm_2d_helper_time_half_cos_slider(40, 0, 500, &iResult[0], &this.lTimestamp[0])){
          //            this.chState[0] = 0;
          //            this.lTimestamp[0] = 0;
          //            this.iProgress[0]  = 0;
          //            user_navigation->operation = NONE;
          //            user_navigation->SlideOut = false;
          //            }else{
          //               this.iProgress[0] = iResult[0];
          //            }  
          //      }else if(user_navigation->operation == WHEEL_UP){
          //          icon_list_move_selection(&this.tList, 1, 200);
          //          this.bRedrawLabel = true;
          //          user_navigation->operation = NONE;
          //      }else if(user_navigation->operation == WHEEL_DOWN){
          //          icon_list_move_selection(&this.tList,-1, 200);
          //          user_navigation->operation = NONE;
          //          this.bRedrawLabel = true;
          //      }else if(user_navigation->operation == SINGLE_PRESS){
          //           user_navigation->operation = WAIT;
          //           this.bSwitchPage = true;
          //      }
          //      break;
          case 2: 
                if(user_navigation->operation == WAIT){
                   this.chState[0] = 3;

                 }else if(user_navigation->operation == WHEEL_DOWN){
                    icon_list_move_selection(&this.tList, 1, 200);
                    this.bRedrawLabel = true;
                    user_navigation->operation = NONE;
                }else if(user_navigation->operation == WHEEL_UP){
                    icon_list_move_selection(&this.tList,-1, 200);
                    user_navigation->operation = NONE;
                    this.bRedrawLabel = true;
                }else if(user_navigation->operation == SINGLE_PRESS){
                     user_navigation->operation = WAIT;
                     this.bSwitchPage = true;
                }
                break;
           case 3:
                 if(arm_2d_helper_time_half_cos_slider(40, 0, 500, &iResult[0], &this.lTimestamp[0])){
                    this.chState[0] = 0;
                    this.lTimestamp[0] = 0;
                    this.iProgress[0]  = 0;
                    user_navigation->operation = NONE;
                    user_navigation->SlideOut = false;
                    }else{
                       this.iProgress[0] = iResult[0];
                    }  

                break;

    }


      //if(this.bWindowCancel){
      //  this.bWindowCancel = false;
      //}
        for(uint8_t i = 0;i < 4;i ++){
             if(this.float_window[i].state && this.float_window[i].lifetime){
                this.float_window[i].lifetime --;
                //if(this.float_window[i].lifetime == 0){
                //   this.float_window[i]
                //}
             }
 
           switch (this.float_window[i].state){
                  case 0:
                       this.float_window[i].iscompelete = true;
                       if(this.New_window_in == 2){    //消息未显示时，该浮窗弹出
                          this.float_window[i].state = 1;
                          this.float_window[i].opacity = 255;
                          this.float_window[i].lifetime = 60;
                          this.New_window_in = 1;      //置1，其他浮窗不重复显示，但需要移动   //fall through
                          this.float_window[i].type = this.New_message_type;
                         }else{
                             break;                            
                         }

                  case 1: 
                  if(arm_2d_helper_time_half_cos_slider(0, 95, 200, &iResult[2 + i], &this.lTimestamp[2 + i])){
                       this.iProgress[2 + i] = 95;
                      if(this.float_window[i].lifetime == 0) {
                         this.lTimestamp[2 + i] = 0;
                         this.float_window[i].state = 4;
                         this.float_window[i].iscompelete = true;
                      }else if(this.New_window_in){
                        this.lTimestamp[2 + i] = 0;
                        this.float_window[i].state = 2;
                        this.float_window[i].opacity = 192;
                        this.float_window[i].iscompelete = true;
                      }
                          
                         
                    }else{
                       this.iProgress[2 + i] = iResult[2 + i];
                    }  
                  break;
                  case 2:
                  if(arm_2d_helper_time_half_cos_slider(0, 95, 200, &iResult[7 + i], &this.lTimestamp[2 + i])){
                       this.iProgress[7 + i] = 95;
                      if(this.float_window[i].lifetime == 0) {
                         this.lTimestamp[2 + i] = 0;
                         this.float_window[i].state = 4;
                         this.float_window[i].iscompelete = true;
                      }else if(this.New_window_in){
                       this.lTimestamp[2 + i] = 0;
                       this.float_window[i].state = 3;
                       this.float_window[i].opacity = 128;
                       this.float_window[i].iscompelete = true;
                      }
                    }else{
                       this.iProgress[7 + i] = iResult[7 + i];
                    }                        
                  break;
                  case 3:
                  if(arm_2d_helper_time_half_cos_slider(95, 180, 200, &iResult[7 + i], &this.lTimestamp[2 + i])){
                      // this.lTimestamp[1] = 0;
                       this.iProgress[7 + i] = 180;
                       
                      if(this.float_window[i].lifetime == 0) {
                         this.lTimestamp[2 + i] = 0;
                         this.float_window[i].state = 4;
                         this.float_window[i].iscompelete = true;
                      }else if(this.New_window_in){
                       this.float_window[i].state = 4;
                       this.lTimestamp[2 + i] = 0;
                       this.float_window[i].iscompelete = true;
                      }
                    }else{
                       this.iProgress[7 + i] = iResult[7 + i];
                    }                        
                  break;
                  case 4:
                  if(arm_2d_helper_time_half_cos_slider(95, 0, 200, &iResult[2 + i], &this.lTimestamp[2 + i])){
                      // this.lTimestamp[1] = 0;
                        this.iProgress[2 + i] = 0;
                      //  if(this.New_window_in) {
                        this.float_window[i].state = 0;
                        this.lTimestamp[2 + i] = 0;
                        user_navigation->float_window[i].position.iX = 240;
                        user_navigation->float_window[i].position.iY = 155;
                        this.iProgress[2 + i] = 0;
                        this.iProgress[7 + i] = 0;
                        this.float_window[i].iscompelete = true;
                        break;
                      // }
                    }else{
                       this.iProgress[2 + i] = iResult[2 + i];
                    }   
                  break;
           }       
          
      }
      if(user_navigation->float_window[0].iscompelete && user_navigation->float_window[1].iscompelete
      && user_navigation->float_window[2].iscompelete && user_navigation->float_window[3].iscompelete){
         this.New_window_in = 0;
      }
    
    ARM_PT_END();
    return arm_fsm_rt_cpl;
}


static void __on_user_navigation_frame_compelete(user_navigation_t *ptNavigation)
{
    user_navigation_t *ptThis = (user_navigation_t *)ptNavigation;
    ARM_2D_UNUSED(ptThis);
    static int16_t iY;

}

static void __on_user_navigation_frame_start(user_navigation_t *ptNavigation)
{
     user_navigation_t *ptThis = (user_navigation_t *)ptNavigation;
     static uint8_t com_stat = 0; 
     static uint8_t mcu_stat = 0; 
     static uint8_t download_stat = 0; 

     if(user_navigation.operation == NONE){
        if(arm_2d_helper_is_time_out(3000,&this.lTimestamp[1])){
           this.lTimestamp[1] = 0;
           user_navigation.operation = WAIT;
        }
      }else{
           this.lTimestamp[1] = 0;
      }

     if(download_stat != this.chDownloadstat){
        user_navigation.New_window_in = 2;
        if(this.chDownloadstat == 1){
           user_navigation.New_message_type = DOWNLOAD_SUCCEED;      
        }else if(this.chDownloadstat == 2){
           user_navigation.New_message_type = DOWNLOAD_FAILED;  
        }
        download_stat = 0;
        this.chDownloadstat = 0;
     }else if(mcu_stat != this.chMcuConnected){    //每帧仅触发一种
           user_navigation.New_window_in = 2;
         if(this.chMcuConnected == 0){
            user_navigation.New_message_type = MCU_DISCONNECTED;      
         }else if(this.chMcuConnected == 1){
            user_navigation.New_message_type = MCU_CONNECTED;      
         }
         mcu_stat = this.chMcuConnected;
    }else if(com_stat != g_tMicroInfo.tComType){
       if(g_tMicroInfo.tComType < 4){
          user_navigation.New_window_in = 2;
          user_navigation.New_message_type = g_tMicroInfo.tComType + 5;      
       }      
       com_stat = g_tMicroInfo.tComType;
    }else if(this.chNofileTips){
          this.chNofileTips = 0;
          user_navigation.New_window_in = 2;
          user_navigation.New_message_type = CHOOSE_FIEMWARE;         
    }

     __scene_frame_actions(ptNavigation);



    icon_list_on_frame_start(&this.tList);  
     
}


static 
arm_fsm_rt_t __arm_2d_icon_list_draw_list_core_item( 
                                      arm_2d_list_item_t *ptItem,
                                      const arm_2d_tile_t *ptTile,
                                      bool bIsNewFrame,
                                      arm_2d_list_item_param_t *ptParam)
{
    icon_list_t *ptThis = (icon_list_t *)ptItem->ptListView;

    ARM_2D_UNUSED(ptItem);
    ARM_2D_UNUSED(bIsNewFrame);
    ARM_2D_UNUSED(ptTile);
    //ARM_2D_UNUSED(ptParam);

    arm_2d_canvas(ptTile, __item_canvas) {

        do {
            const arm_2d_tile_t *ptileIcon = icon_list_get_item_icon(ptThis, ptItem->hwID);
            if (NULL == ptTile) {
                break;
            } else if ( ptTile->tInfo.bHasEnforcedColour 
                     && ptTile->tInfo.tColourInfo.chScheme != ARM_2D_COLOUR_1BIT) {
                /* we only draw a4 mask by default */
                break;
            }
            arm_2d_align_centre(__item_canvas, ptileIcon->tRegion.tSize) {
              if(ptParam->bIsSelected){
                
                arm_2d_fill_colour_with_a1_mask(ptTile, 
                                            &__centre_region, 
                                            ptileIcon, 
                                            (__arm_2d_color_t){GLCD_COLOR_BLACK});

              }else{
                arm_2d_fill_colour_with_a1_mask(ptTile, 
                                            &__centre_region, 
                                            ptileIcon, 
                                            (__arm_2d_color_t){GLCD_COLOR_ORANGE});
              }
            }

        } while(0);
    }
    
    return arm_fsm_rt_cpl;
}

IMPL_PFB_ON_DRAW(__disp_adapter0_user_draw_navigation)
{
    ARM_2D_PARAM(ptTile);
    ARM_2D_PARAM(pTarget);
    ARM_2D_PARAM(bIsNewFrame);

    user_navigation_t *ptThis = (user_navigation_t *)pTarget;
    static uint8_t sort[4] = {0};

    if(bIsNewFrame){
        do {
              __on_user_navigation_frame_compelete(ptThis);
              __on_user_navigation_frame_start(ptThis);
           } while(0);
    }

    arm_lcd_text_set_target_framebuffer((arm_2d_tile_t *)ptTile);
    arm_lcd_text_set_font(&ARM_2D_FONT_Round12_A2);
    arm_lcd_text_set_draw_region(NULL);
    arm_lcd_text_set_opacity(192);
    arm_lcd_text_set_colour(GLCD_COLOR_ORANGE, GLCD_COLOR_WHITE);
    arm_lcd_text_location(0,0);

    arm_2d_region_t info_region = {
       .tLocation.iX = 2,
       .tLocation.iY = 2,
       .tSize.iWidth  = 236,
       .tSize.iHeight = 20,
    };

    draw_round_corner_box(ptTile,
                          &info_region,
                          GLCD_COLOR_BLACK,
                          128,
                          &c_tileWhiteDotMask);

    draw_round_corner_border(ptTile, 
                             &info_region, 
                             GLCD_COLOR_ORANGE, 
                            (arm_2d_border_opacity_t)
                              {0, 0, 0, 0},
                            (arm_2d_corner_opacity_t)
                              {192, 192, 192, 192});


    for(uint8_t i = 0;i < 4;i ++){
      if(this.float_window[i].state == 1){
         sort[3] = i;
      }else if(this.float_window[i].state == 2){
         sort[2] = i;
      }else if(this.float_window[i].state == 3){
         sort[1] = i;
      }else if(this.float_window[i].state == 4){
         sort[0] = i;
      }
    }



   for(uint8_t i = 0;i < 4;i ++){
    arm_2d_region_t window_region0 = {
       .tLocation.iX = this.float_window[sort[i]].position.iX - this.iProgress[2 + sort[i]],
       .tLocation.iY = this.float_window[sort[i]].position.iY - this.iProgress[7 + sort[i]] / 4,
       .tSize.iWidth  = c_tileWindowFrameA4Mask.tRegion.tSize.iWidth,
       .tSize.iHeight = c_tileWindowFrameA4Mask.tRegion.tSize.iHeight,
    };

      s_tUserNavDirtyRegionList[sort[i] + 1].tRegion.tLocation.iX = window_region0.tLocation.iX - 30;
      s_tUserNavDirtyRegionList[sort[i] + 1].tRegion.tLocation.iY = window_region0.tLocation.iY;

      if(this.float_window[sort[i]].state){
      arm_2d_rgb565_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                      &window_region0,
                                                      &c_tileWindowFillA4Mask,
                                                      (arm_2d_color_rgb565_t){GLCD_COLOR_BLACK},
                                                      this.float_window[sort[i]].opacity);     

      arm_lcd_text_set_target_framebuffer(ptTile);
      arm_lcd_text_set_font((arm_2d_font_t *)&ARM_2D_FONT_Round16_A4);


      arm_2d_region_t content_region = {
             .tLocation.iX = window_region0.tLocation.iX,
             .tLocation.iY = window_region0.tLocation.iY - 2,
             .tSize.iWidth  = c_tileWindowFrameA4Mask.tRegion.tSize.iWidth,
             .tSize.iHeight = c_tileWindowFrameA4Mask.tRegion.tSize.iHeight,
      };

      switch (this.float_window[sort[i]].type){
         case DOWNLOAD_SUCCEED:
              content_region.tLocation.iX += 4;

              arm_lcd_text_set_draw_region(&content_region);
              arm_lcd_text_set_opacity(this.float_window[sort[i]].opacity);
              arm_lcd_text_set_colour(GLCD_COLOR_ORANGE, GLCD_COLOR_BLACK);
              arm_lcd_printf_label(ARM_2D_ALIGN_CENTRE,"烧录成功");              
              arm_2d_rgb565_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                              &window_region0,
                                                              &c_tileWindowFrameA4Mask,
                                                              (arm_2d_color_rgb565_t){GLCD_COLOR_B_GREEN},
                                                              this.float_window[sort[i]].opacity); 
         break;
         case DOWNLOAD_FAILED:
              content_region.tLocation.iX += 4;

              arm_lcd_text_set_draw_region(&content_region);
              arm_lcd_text_set_opacity(this.float_window[sort[i]].opacity);
              arm_lcd_text_set_colour(GLCD_COLOR_ORANGE, GLCD_COLOR_BLACK);
              arm_lcd_printf_label(ARM_2D_ALIGN_CENTRE,"烧录失败");              
              arm_2d_rgb565_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                              &window_region0,
                                                              &c_tileWindowFrameA4Mask,
                                                              (arm_2d_color_rgb565_t){GLCD_COLOR_RED},
                                                              this.float_window[sort[i]].opacity); 
         break;
         case CHOOSE_FIEMWARE:
              arm_lcd_text_set_draw_region(&content_region);
              arm_lcd_text_set_opacity(this.float_window[sort[i]].opacity);
              arm_lcd_text_set_colour(GLCD_COLOR_ORANGE, GLCD_COLOR_BLACK);
              arm_lcd_printf_label(ARM_2D_ALIGN_CENTRE,"请选择固件");              
              arm_2d_rgb565_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                              &window_region0,
                                                              &c_tileWindowFrameA4Mask,
                                                              (arm_2d_color_rgb565_t){GLCD_COLOR_YELLOW},
                                                              this.float_window[sort[i]].opacity); 
         break;
         case MCU_CONNECTED:
              arm_lcd_text_set_draw_region(&content_region);
              arm_lcd_text_set_opacity(this.float_window[sort[i]].opacity);
              arm_lcd_text_set_colour(GLCD_COLOR_ORANGE, GLCD_COLOR_BLACK);
              arm_lcd_printf_label(ARM_2D_ALIGN_CENTRE,"MCU已连接");              
              arm_2d_rgb565_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                              &window_region0,
                                                              &c_tileWindowFrameA4Mask,
                                                              (arm_2d_color_rgb565_t){GLCD_COLOR_B_GREEN},
                                                              this.float_window[sort[i]].opacity); 
         break;
         case MCU_DISCONNECTED:
              arm_lcd_text_set_draw_region(&content_region);
              arm_lcd_text_set_opacity(this.float_window[sort[i]].opacity);
              arm_lcd_text_set_colour(GLCD_COLOR_ORANGE, GLCD_COLOR_BLACK);
              arm_lcd_printf_label(ARM_2D_ALIGN_CENTRE,"MCU已断开");              
              arm_2d_rgb565_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                              &window_region0,
                                                              &c_tileWindowFrameA4Mask,
                                                              (arm_2d_color_rgb565_t){GLCD_COLOR_YELLOW},
                                                              this.float_window[sort[i]].opacity); 
         break;
         case COM_NONE:
              arm_lcd_text_set_draw_region(&content_region);
              arm_lcd_text_set_opacity(this.float_window[sort[i]].opacity);
              arm_lcd_text_set_colour(GLCD_COLOR_ORANGE, GLCD_COLOR_BLACK);
              arm_lcd_printf_label(ARM_2D_ALIGN_CENTRE,"COM:NONE");              
              arm_2d_rgb565_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                              &window_region0,
                                                              &c_tileWindowFrameA4Mask,
                                                              (arm_2d_color_rgb565_t){GLCD_COLOR_YELLOW},
                                                              this.float_window[sort[i]].opacity); 
         break;
         case COM_CMD:
              content_region.tLocation.iX -= 2;
              arm_lcd_text_set_draw_region(&content_region);
              arm_lcd_text_set_opacity(this.float_window[sort[i]].opacity);
              arm_lcd_text_set_colour(GLCD_COLOR_ORANGE, GLCD_COLOR_BLACK);
              arm_lcd_printf_label(ARM_2D_ALIGN_CENTRE,"COM:CMD");              
              arm_2d_rgb565_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                              &window_region0,
                                                              &c_tileWindowFrameA4Mask,
                                                              (arm_2d_color_rgb565_t){GLCD_COLOR_B_GREEN},
                                                              this.float_window[sort[i]].opacity); 
         break;
         case COM_UART:
              arm_lcd_text_set_draw_region(&content_region);
              arm_lcd_text_set_opacity(this.float_window[sort[i]].opacity);
              arm_lcd_text_set_colour(GLCD_COLOR_ORANGE, GLCD_COLOR_BLACK);
              arm_lcd_printf_label(ARM_2D_ALIGN_CENTRE,"COM:UART");              
              arm_2d_rgb565_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                              &window_region0,
                                                              &c_tileWindowFrameA4Mask,
                                                              (arm_2d_color_rgb565_t){GLCD_COLOR_B_GREEN},
                                                              this.float_window[sort[i]].opacity); 
         break;
         case COM_RS485:
              content_region.tLocation.iX -= 2;
              arm_lcd_text_set_draw_region(&content_region);
              arm_lcd_text_set_opacity(this.float_window[sort[i]].opacity);
              arm_lcd_text_set_colour(GLCD_COLOR_ORANGE, GLCD_COLOR_BLACK);
              arm_lcd_printf_label(ARM_2D_ALIGN_CENTRE,"COM:485");              
              arm_2d_rgb565_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                              &window_region0,
                                                              &c_tileWindowFrameA4Mask,
                                                              (arm_2d_color_rgb565_t){GLCD_COLOR_B_GREEN},
                                                              this.float_window[sort[i]].opacity); 
         break;

      }
     }
    }



    arm_2d_canvas(ptTile, __top_canvas) {
      arm_2d_layout(__top_canvas, LEFT_TO_RIGHT) {
        //__item_line_horizontal(10,20, 10, 5, 3, 0) {            
        //  arm_2d_rgb565_fill_colour_with_a4_mask_and_opacity(ptTile,
        //                                                  &__item_region,
        //                                                  &c_tileSmallFlashA4Mask,
        //                                                  (arm_2d_color_rgb565_t){GLCD_COLOR_YELLOW},
        //                                                  255);       
        //}
        __item_line_horizontal(20, 20, 10, 0, 4, 0){       
            if(g_tMicroInfo.idcode == 0){                                                                                                          
              arm_2d_rgb565_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                            &__item_region,
                                                            &c_tileDisconnectedA4Mask,
                                                            (arm_2d_color_rgb565_t){GLCD_COLOR_YELLOW},
                                                            255);     
              this.chMcuConnected = 0;
            }else{   
            arm_2d_rgb565_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                            &__item_region,
                                                            &c_tileConnectedA4Mask,
                                                            (arm_2d_color_rgb565_t){GLCD_COLOR_GREEN},
                                                            255); 
              this.chMcuConnected = 1;
            }
        }  

        __item_line_horizontal(120,20, 0, 0, 2, 0){     
          arm_lcd_text_set_font(&ARM_2D_FONT_Round12_A2);
          arm_lcd_text_set_draw_region(&__item_region);     
          arm_lcd_printf("%.3fV I %.3fA",g_tMicroInfo.vcc_voltage /1000, g_tMicroInfo.iref_voltage);          
                                                                                                                                             
          //arm_lcd_printf("Vcc %d.%dv Vref %d.%dv",g_tMicroInfo.vcc_voltage /1000, (g_tMicroInfo.vcc_voltage%1000)/100,
          //                                        g_tMicroInfo.vref_voltage/1000, (g_tMicroInfo.vref_voltage%1000)/100);      
        }
  
        __item_line_horizontal(90,20, 0, 0, 2, 0){     
          arm_lcd_text_set_font(&ARM_2D_FONT_Round12_A2);
          arm_lcd_text_set_draw_region(&__item_region);     
          arm_lcd_printf("I COM:%s",c_pComNameTable[g_tMicroInfo.tComType]);          
                                                                                                                                             
          //arm_lcd_printf("Vcc %d.%dv Vref %d.%dv",g_tMicroInfo.vcc_voltage /1000, (g_tMicroInfo.vcc_voltage%1000)/100,
          //                                        g_tMicroInfo.vref_voltage/1000, (g_tMicroInfo.vref_voltage%1000)/100);      
        }
            


            Island_region.tLocation.iX = 240 - this.iProgress[0];

            s_tUserNavDirtyRegionList[0].tRegion.tLocation.iX = Island_region.tLocation.iX - 10;
            s_tUserNavDirtyRegionList[0].tRegion.tLocation.iY = Island_region.tLocation.iY;


            arm_2d_rgb565_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                            &Island_region,
                                                            &c_tileIslandFillA4Mask,
                                                            (arm_2d_color_rgb565_t){GLCD_COLOR_BACKGROUND},
                                                            192); 

            arm_2d_rgb565_fill_colour_with_a4_mask_and_opacity(ptTile,
                                                            &Island_region,
                                                            &c_tileIslandA4Mask,
                                                            (arm_2d_color_rgb565_t){GLCD_COLOR_ORANGE},
                                                            255); 
 
                    do {
                        if (__arm_2d_list_core_is_list_moving(
                            &this.
                                tList.
                                    use_as____simple_list_t.
                                        use_as____arm_2d_list_core_t)) {
                            break;
                        }

                        if (this.bRedrawLabel) {
                            this.bRedrawLabel = false;
                        }
                    } while(0);

                    
                    arm_2d_region_t List_region = {
                       .tLocation.iX = Island_region.tLocation.iX,
                       .tLocation.iY = 75,
                       .tSize.iWidth  = 40,
                       .tSize.iHeight = 100,
                    };   

                     arm_2d_align_centre(List_region, 32, 32) {     
                        draw_round_corner_box(ptTile,
                                              &__centre_region,
                                              GLCD_COLOR_ORANGE,
                                              192);   
                        draw_round_corner_border(ptTile, 
                                 &__centre_region, 
                                 GLCD_COLOR_YELLOW, 
                                (arm_2d_border_opacity_t)
                                  {0, 0, 0, 0},
                                (arm_2d_corner_opacity_t)
                                  {192, 192, 192, 192});                                                       
                     }

                    while(arm_fsm_rt_cpl != icon_list_show( &this.tList, 
                                                            ptTile, 
                                                            &List_region, 
                                                            bIsNewFrame));

      }
 
    }
    return arm_fsm_rt_cpl;
}


void disp_adapter0_user_navigator_init(void)
{
  user_navigation.lTimestamp[0] = 0;
  user_navigation.lTimestamp[1] = 0;
  user_navigation.lTimestamp[2] = 0;

  user_navigation.float_window[0].position.iX = 240;
  user_navigation.float_window[0].position.iY = 155;

  user_navigation.float_window[1].position.iX = 240;
  user_navigation.float_window[1].position.iY = 155;

  user_navigation.float_window[2].position.iX = 240;
  user_navigation.float_window[2].position.iY = 155;

  user_navigation.float_window[3].position.iX = 240;
  user_navigation.float_window[3].position.iY = 155;

  user_navigation.float_window[4].position.iX = 240;
  user_navigation.float_window[4].position.iY = 155;

  s_tUserNavDirtyRegionList[0].tRegion.tLocation.iX = 195;
  s_tUserNavDirtyRegionList[0].tRegion.tLocation.iY = 80;

    do {
        arm_2d_helper_pi_slider_cfg_t tpidCFG = {
            .fProportion = 0.08,
            .fIntegration = 0.02,
            .nInterval = 5
        };
        icon_list_cfg_t tCFG = {
            .Icon = {
                .pptileMasks = c_pIconTable,
                .hwCount = dimof(c_pIconTable),
            },

            .use_as____simple_list_cfg_t = {
                .hwCount = dimof(c_pIconTable),
                
                .tFontColour = GLCD_COLOR_ORANGE,
                .tBackgroundColour = GLCD_COLOR_BLACK,
                .bIgnoreBackground = true,
                
                .bUseMonochromeMode = true,
                .bShowScrollingBar = false,
                .chScrollingBarAutoDisappearTimeX100Ms = 10,
                .ScrollingBar.tColour = GLCD_COLOR_ORANGE,
                .bPlaceScrollingBarOnTopOrLeft = true,
                
                .bUsePIMode = true,
                .ptPISliderCFG = &tpidCFG,

                //.bDisableRingMode = true,     /* you can disable the list ring mode here */
                //.chNextPadding = 1,
                //.chPreviousPadding = 1,
                .tListSize = {
                    .iHeight = 0,           /* automatically set the height */
                    .iWidth = 0,            /* automatically set the width */
                },
                .tItemSize = {
                    .iHeight = 32,
                    .iWidth = 32,
                },
                .tTextAlignment = ARM_2D_ALIGN_MIDDLE_LEFT,
                .fnOnDrawListItem = &__arm_2d_icon_list_draw_list_core_item,

                .bUseDirtyRegion = false,
                .hwSwitchingPeriodInMs = 30,
                .ptTargetScene = NULL,
            }
        };
         __icon_list_init(&user_navigation.tList, &tCFG, &ARM_2D_LIST_CALCULATOR_MIDDLE_ALIGNED_FIXED_SIZED_ITEM_NO_STATUS_CHECK_VERTICAL);
         icon_list_move_selection(&user_navigation.tList, 0, 0);
    } while(0);

  



  arm_2d_scene_player_register_on_draw_navigation_event_handler(
              &DISP0_ADAPTER,
              __disp_adapter0_user_draw_navigation,
              &user_navigation,
              (arm_2d_region_list_item_t *)s_tUserNavDirtyRegionList);
}





#if defined(__clang__)
#   pragma clang diagnostic pop
#endif



