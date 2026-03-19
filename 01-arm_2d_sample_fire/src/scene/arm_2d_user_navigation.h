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

#ifndef  __ARM_2D_USER_NAVIGAYION_H__
#define  __ARM_2D_USER_NAVIGAYION_H__
#endif
/*============================ INCLUDES ======================================*/
#include "arm_2d_helper.h"
#include "./icon_list.h"
#include "__arm_2d_helper_common.h"
#ifdef   __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/
#include "arm_2d_utils.h"
#include "ref_gui.h"
/*============================ MACROFIED FUNCTIONS ===========================*/
typedef struct user_navigation_t user_navigation_t;

enum {
    NONE = 0,
    WHEEL_UP,
    WHEEL_DOWN,
    SINGLE_PRESS,
    LONG_PRESS,
    WAIT,           //静置
};

enum {
    DOWNLOAD_SUCCEED = 0,
    DOWNLOAD_FAILED,
    CHOOSE_FIEMWARE,
    MCU_CONNECTED,
    MCU_DISCONNECTED,
    COM_NONE,
    COM_CMD,
    COM_UART,
    COM_RS485,
};

enum {
    ICON_IDX_DOWNLOAD = 0,
    ICON_IDX_SCOPE,
    ICON_IDX_INFO,
};

enum {
    SLIDE_IN = 0,
    SLIDE_UP_1,
    SLIDE_UP_2,
    SLIDE_OUT,
};

typedef struct float_window_t{
   arm_2d_location_t position;
   uint8_t index;
   uint8_t state;
   uint8_t type;
   uint32_t lifetime;
   bool    isfull;
   bool    iscompelete;
   uint16_t opacity;
}float_window_t;

struct user_navigation_t {
    int64_t lTimestamp[6];
    uint8_t bUserAllocated  : 1;
    uint8_t bRedrawLabel    : 1;
    int16_t iProgress[12];
    //icon_list_t tList; 
    bool SlideOut           ;
    uint8_t chPT            ;
    uint8_t chState[2]         ;
    arm_2d_helper_dirty_region_item_t tDirtyRegionItems[1];
    /* place your public member here */
    ref_gui_t *ptGUI ;
    uint8_t operation;
    icon_list_t tList;
    float_window_t float_window[4];
    uint8_t Target_page;
    uint8_t New_window_in;
    uint8_t New_message_type;

    bool    bWindowCancel;
    bool    bSwitchPage;

    uint8_t chMcuConnected;
    uint8_t chDownloadstat;
    uint8_t chNofileTips;

};

extern
void disp_adapter0_user_navigator_init(void);

extern
user_navigation_t user_navigation;

extern
void navi_operate(uint8_t stat);
#ifdef   __cplusplus
}
#endif
