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

#define __USER_SCENE_FILTERS_IMPLEMENT__
#include "arm_2d_scene_user_scope.h"
#include "arm_2d_scene_user_menu.h"
#include "scope_task.h"

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

/*============================ MACROFIED FUNCTIONS ===========================*/
#undef this
#define this (*ptThis)

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/

#define GLCD_COLOR_LIGHT_BLUE   __RGB(   50,   50, 255  )
#define GLCD_COLOR_PINK         __RGB(  255,   50, 200  )
#define GLCD_COLOR_BRIGHT_GREEN __RGB(    0,  255, 10   )
#define GLCD_COLOR_B_GREEN      __RGB( 0X5D, 0XF5, 0XFE )
extern Scope_t My_scope_v;
extern Scope_t My_scope_c;

extern 
const
struct {
    implement(arm_2d_user_font_t);
    arm_2d_char_idx_t tUTF8Table2;
}ARM_2D_FONT_Round12_A4,
 ARM_2D_FONT_Round12_A2;
/*============================ PROTOTYPES ====================================*/


/*============================ LOCAL VARIABLES ===============================*/
enum {
    LIST_ITEM_ID_REFERENCE,
    LIST_ITEM_ID_GLASS_BAR,
    LIST_ITEM_ID_REVERSE_COLOUR,
    LIST_ITEM_ID_IIR_BLUR,
};
static const char *UnitTable[] = {
    [0] =      "V",
    [1] =      "A",
    [2] =      "W",	
    [3] =      "ms",		
    [4] =      "s",		
    [5] =      "m",
    [6] =      "h",		
};
	
static const char *UnitTitle[] = {
    [0] =      "U = ",
    [1] =      "I = ",
    [2] =      "Time = ",	
};

static const char *TimestepTable[] = {
    [0] =  "200ms",
    [1] =  "1s",
    [2] =  "2s",
    [3] =  "10s",
    [4] =  "100s",
    [5] =  "10min",
    [6] =  "30min",
};	
	
arm_2d_region_t tScale_value_region = 
{
  .tLocation.iX  =  10,
  .tLocation.iY  =  180,
  .tSize.iHeight =  40,	
  .tSize.iWidth  =  100,			
};		

/*============================ IMPLEMENTATION ================================*/


static void __on_scene_filters_load(arm_2d_scene_t *ptScene)
{
    user_scene_filters_t *ptThis = (user_scene_filters_t *)ptScene;
    ARM_2D_UNUSED(ptThis);

}

static void __after_scene_filters_switching(arm_2d_scene_t *ptScene)
{
    user_scene_filters_t *ptThis = (user_scene_filters_t *)ptScene;
    ARM_2D_UNUSED(ptThis);

}

static void __on_scene_filters_depose(arm_2d_scene_t *ptScene)
{
    user_scene_filters_t *ptThis = (user_scene_filters_t *)ptScene;
    ARM_2D_UNUSED(ptThis);
    
    ARM_2D_OP_DEPOSE(this.tBlurOP);
    
    arm_foreach(int64_t,this.lTimestamp, ptItem) {
        *ptItem = 0;
    }

    ptScene->ptPlayer = NULL;
    if (!this.bUserAllocated) {
        __arm_2d_free_scratch_memory(ARM_2D_MEM_TYPE_UNSPECIFIED, ptScene);
    }
}

/*----------------------------------------------------------------------------*
 * Scene filters                                                                    *
 *----------------------------------------------------------------------------*/

static void __on_scene_filters_background_start(arm_2d_scene_t *ptScene)
{
    user_scene_filters_t *ptThis = (user_scene_filters_t *)ptScene;
    ARM_2D_UNUSED(ptThis);

}

static void __on_scene_filters_background_complete(arm_2d_scene_t *ptScene)
{
    user_scene_filters_t *ptThis = (user_scene_filters_t *)ptScene;
    ARM_2D_UNUSED(ptThis);

}


static void __on_scene_filters_frame_start(arm_2d_scene_t *ptScene)
{
    user_scene_filters_t *ptThis = (user_scene_filters_t *)ptScene;
    ARM_2D_UNUSED(ptThis);

    //if (LIST_ITEM_ID_IIR_BLUR == list_view_get_selected_item_id(&this.tListView)) {
    //    if (arm_2d_helper_is_time_out(6000, &this.lTimestamp[1])) {
    //        list_view_move_selection(&this.tListView, 1, 1000);
    //        this.lTimestamp[1] = 0;
    //    }
    //} else {
    //    if (arm_2d_helper_is_time_out(3000, &this.lTimestamp[1])) {
    //        list_view_move_selection(&this.tListView, 1, 1000);
    //        this.lTimestamp[1] = 0;
    //    }
    //}

    if (LIST_ITEM_ID_IIR_BLUR == list_view_get_selected_item_id(&this.tListView) 
    &&  !__arm_2d_list_core_is_list_moving(&this.tListView.use_as____arm_2d_list_core_t) ) {
        int32_t nResult;
        if (arm_2d_helper_time_cos_slider(  0, 
                                            (255-16),
                                            5000,
                                            0,
                                            &nResult, 
                                            &this.lTimestamp[0])) {
        }
        this.chBlurDegree = nResult;
    } else {
        this.chBlurDegree = 0;
        this.lTimestamp[0] = 0;
    }
    
}

static void __on_scene_filters_frame_complete(arm_2d_scene_t *ptScene)
{
    user_scene_filters_t *ptThis = (user_scene_filters_t *)ptScene;
    ARM_2D_UNUSED(ptThis);

    arm_2dp_filter_iir_blur_depose(&this.tBlurOP);
    static int16_t iY;
#if 0
    /* switch to next scene after 3s */
    if (arm_2d_helper_is_time_out(3000, &this.lTimestamp[0])) {
        arm_2d_scene_player_switch_to_next_scene(ptScene->ptPlayer);
    }
#endif
    RG_PROCESS_MSG(this.ptGUI, ptMSG) {
        ARM_2D_UNUSED(bIsMessageHandled);
        switch(ngy_helper_msg_item_get_id(ptMSG)) {
            case NGY_MSG_GESTURE_EVT_WHEEL:
            do {
                ngy_msg_gesture_evt_t *ptKeyMSG = (ngy_msg_gesture_evt_t *)ptMSG;
                if(ptKeyMSG->tRegion.tLocation.iY > iY){
                    Scope_cfg.x_level += 1;
                }else{
                    Scope_cfg.x_level -= 1;
                }

                if(Scope_cfg.x_level > 6){
                   Scope_cfg.x_level = 0;
                }
                if(Scope_cfg.x_level < 0){
                   Scope_cfg.x_level = 6;
                }

                iY = ptKeyMSG->tRegion.tLocation.iY;
                this.lTimestamp[0] = 0;
                bIsMessageHandled = true;
            }while(0);
            break;
            /* key pressed */
            case NGY_MSG_KEY_EVT_PRESSED:
                break;
            case NGY_MSG_KEY_EVT_LONG_PRESSING:                          
                arm_2d_scene_player_switch_to_next_scene(ptScene->ptPlayer);
                break; 
            default:
                break;
        }
    }
}

static void __before_scene_filters_switching_out(arm_2d_scene_t *ptScene)
{
    user_scene_filters_t *ptThis = (user_scene_filters_t *)ptScene;
    ARM_2D_UNUSED(ptThis);
    user_scene_menu_t *ptMenu
        =  arm_2d_scene_menu_init(this.use_as__arm_2d_scene_t.ptPlayer);
    assert(NULL != ptMenu);
    ptMenu->ptGUI = &g_tMyGUI;
}

static
IMPL_PFB_ON_DRAW(__pfb_draw_scene_filters_handler)
{
    ARM_2D_PARAM(pTarget);
    ARM_2D_PARAM(ptTile);
    ARM_2D_PARAM(bIsNewFrame);

    user_scene_filters_t *ptThis = (user_scene_filters_t *)pTarget;
    arm_2d_size_t tScreenSize = ptTile->tRegion.tSize;

    ARM_2D_UNUSED(tScreenSize);

    arm_2d_canvas(ptTile, __top_canvas) {
    /*-----------------------draw the foreground begin-----------------------*/
         bool bIsNewFrame = arm_2d_target_tile_is_new_frame(pTarget);
         if(bIsNewFrame){
            Draw_Grid(&My_scope_v, ptTile);
	  }
            Draw_Line(&My_scope_v, ptTile,GLCD_COLOR_B_GREEN);
            ARM_2D_OP_WAIT_ASYNC();
            Draw_Line(&My_scope_c, ptTile,GLCD_COLOR_YELLOW);    

	arm_2d_region_t tVal_step = 
	{
	  .tLocation.iX = 10,
	  .tLocation.iY = 25,
	  .tSize.iHeight = 22,	
	  .tSize.iWidth  = 60,			
	};		
	
        arm_lcd_text_set_target_framebuffer(ptTile);
	arm_lcd_text_set_font(&ARM_2D_FONT_Round12_A2);

         arm_2d_layout(__top_canvas, LEFT_TO_RIGHT) {
            __item_line_horizontal(60,12, 10, 5, 25, 0) {    
            arm_lcd_text_set_draw_region(&__item_region);         
               arm_lcd_text_set_colour(GLCD_COLOR_B_GREEN,GLCD_COLOR_BLACK);                                                                                                                               
               arm_lcd_printf("U %dmV",Scope_cfg.y_vol_step[Scope_cfg.index_vol]);   
            }
            __item_line_horizontal(60,12, 10, 5, 25, 0) {    
            arm_lcd_text_set_draw_region(&__item_region);        
               arm_lcd_text_set_colour(GLCD_COLOR_YELLOW,GLCD_COLOR_BLACK);                                                                                                                                      
               arm_lcd_printf("I %dmA",Scope_cfg.y_cur_step[Scope_cfg.index_cur]);   
            }
         }
          arm_2d_layout(__top_canvas, BOTTOM_UP) {
            __item_line_horizontal(100,12, 10, 5, 5, 10) {    
            arm_lcd_text_set_draw_region(&__item_region);        
               arm_lcd_text_set_colour(GLCD_COLOR_ORANGE,GLCD_COLOR_BLACK);                                                                                                                                      
               arm_lcd_printf("Time %s",TimestepTable[Scope_cfg.x_level]);   
            }
          }
    /*-----------------------draw the foreground end  -----------------------*/
    }
    ARM_2D_OP_WAIT_ASYNC();

    return arm_fsm_rt_cpl;
}

ARM_NONNULL(1)
user_scene_filters_t *__arm_2d_scene_filters_init(   arm_2d_scene_player_t *ptDispAdapter, 
                                        user_scene_filters_t *ptThis)
{
    bool bUserAllocated = false;
    assert(NULL != ptDispAdapter);

    if (NULL == ptThis) {
        ptThis = (user_scene_filters_t *)
                    __arm_2d_allocate_scratch_memory(   sizeof(user_scene_filters_t),
                                                        __alignof__(user_scene_filters_t),
                                                        ARM_2D_MEM_TYPE_UNSPECIFIED);
        assert(NULL != ptThis);
        if (NULL == ptThis) {
            return NULL;
        }
    } else {
        bUserAllocated = true;
    }

    memset(ptThis, 0, sizeof(user_scene_filters_t));

    *ptThis = (user_scene_filters_t){
        .use_as__arm_2d_scene_t = {

            /* the canvas colour */
            .tCanvas = {GLCD_COLOR_BLACK}, 

            /* Please uncommon the callbacks if you need them
             */
            .fnOnLoad       = &__on_scene_filters_load,
            .fnScene        = &__pfb_draw_scene_filters_handler,
            //.fnAfterSwitch  = &__after_scene_filters_switching,

            /* if you want to use predefined dirty region list, please uncomment the following code */
            //.ptDirtyRegion  = (arm_2d_region_list_item_t *)s_tDirtyRegions,
            

            //.fnOnBGStart    = &__on_scene_filters_background_start,
            //.fnOnBGComplete = &__on_scene_filters_background_complete,
            .fnOnFrameStart = &__on_scene_filters_frame_start,
            .fnBeforeSwitchOut = &__before_scene_filters_switching_out,
            .fnOnFrameCPL   = &__on_scene_filters_frame_complete,
            .fnDepose       = &__on_scene_filters_depose,

            .bUseDirtyRegionHelper = true,
        },
        .bUserAllocated = bUserAllocated,
    };

    /* ------------   initialize members of user_scene_filters_t begin ---------------*/
    ARM_2D_OP_INIT(this.tBlurOP);

    do {
        static const arm_2d_helper_pi_slider_cfg_t c_tPIHelperCFG = {
            .fProportion = 0.040f,
            .fIntegration = 0.00300f,
            .nInterval = 10,
        };

    } while(0);

    /* ------------   initialize members of user_scene_filters_t end   ---------------*/

    arm_2d_scene_player_append_scenes(  ptDispAdapter, 
                                        &this.use_as__arm_2d_scene_t, 
                                        1);

    return ptThis;
}


#if defined(__clang__)
#   pragma clang diagnostic pop
#endif

#endif
