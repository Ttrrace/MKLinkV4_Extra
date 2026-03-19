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

#ifndef __ARM_2D_SCENE_BUTTON_H__
#define __ARM_2D_SCENE_BUTTON_H__

/*============================ INCLUDES ======================================*/

#if defined(_RTE_)
#   include "RTE_Components.h"
#endif

#if defined(RTE_Acceleration_Arm_2D_Helper_PFB)
#include "arm_2d_example_controls.h"
#include "arm_2d_helper.h"
#include "ref_gui.h"

#ifdef   __cplusplus
extern "C" {
#endif

#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wunknown-warning-option"
#   pragma clang diagnostic ignored "-Wreserved-identifier"
#   pragma clang diagnostic ignored "-Wmissing-declarations"
#   pragma clang diagnostic ignored "-Wpadded"
#elif __IS_COMPILER_ARM_COMPILER_5__
#elif __IS_COMPILER_GCC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wformat="
#   pragma GCC diagnostic ignored "-Wpedantic"
#   pragma GCC diagnostic ignored "-Wpadded"
#endif

/*============================ MACROS ========================================*/

/* OOC header, please DO NOT modify  */
#ifdef __USER_SCENE_BUTTON_IMPLEMENT__
#   define __ARM_2D_IMPL__
#endif
#ifdef __USER_SCENE_BUTTON_INHERIT__
#   define __ARM_2D_INHERIT__
#endif
#include "arm_2d_utils.h"

/*============================ MACROFIED FUNCTIONS ===========================*/

/*!
 * \brief initalize scene_button and add it to a user specified scene player
 * \param[in] __DISP_ADAPTER_PTR the target display adapter (i.e. scene player)
 * \param[in] ... this is an optional parameter. When it is NULL, a new 
 *            user_scene_button_t will be allocated from HEAP and freed on
 *            the deposing event. When it is non-NULL, the life-cycle is managed
 *            by user.
 * \return user_scene_button_t* the user_scene_button_t instance
 */
#define arm_2d_scene_button_init(__DISP_ADAPTER_PTR, ...)                    \
            __arm_2d_scene_button_init((__DISP_ADAPTER_PTR), (NULL, ##__VA_ARGS__))

/*============================ TYPES =========================================*/
/*!
 * \brief a user class for scene button
 */
typedef struct user_scene_button_t user_scene_button_t;

struct user_scene_button_t {
    implement(arm_2d_scene_t);                                                  //! derived from class: arm_2d_scene_t

ARM_PRIVATE(
    /* place your private member here, following two are examples */
    int64_t lTimestamp[6];
    
    uint8_t bUserAllocated      : 1;
    uint8_t bButtonChecked      : 1;
    progress_bar_round_t tProgressBarRound;
    int16_t iProgress[5];
    int8_t  chItemCurrent;
    int8_t  chItemLast;
    console_box_t tConsole;
    int16_t Console_iY;
    int16_t iY;
    uint8_t             chPT;
    uint8_t             chState[5];
    foldable_panel_t    tPanel;

    bool                 Downloading;
    bool                 ShowIsland;
    spin_zoom_widget_t         tText;
    spin_zoom_widget_t         tShadow;
    arm_2d_helper_dirty_region_item_t tDirtyRegionItem[6];
)
    /* place your public member here */
    ref_gui_t *ptGUI;
    bool bScreenProtect;
    bool    Checklog;
    char chFilePath[64];
    bool lock;
    uint16_t downloadprogress;
};

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

ARM_NONNULL(1)
extern
user_scene_button_t *__arm_2d_scene_button_init(   arm_2d_scene_player_t *ptDispAdapter, 
                                        user_scene_button_t *ptScene);

#if defined(__clang__)
#   pragma clang diagnostic pop
#elif __IS_COMPILER_GCC__
#   pragma GCC diagnostic pop
#endif

#undef __USER_SCENE_BUTTON_IMPLEMENT__
#undef __USER_SCENE_BUTTON_INHERIT__

#ifdef   __cplusplus
}
#endif

#endif

#endif
