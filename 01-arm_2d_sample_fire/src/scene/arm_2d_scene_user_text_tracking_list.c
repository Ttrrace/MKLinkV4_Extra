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
#define __SIMPLE_LIST_INHERIT__
#define __TEXT_LIST_IMPLEMENT__
#define __USER_SCENE_TEXT_TRACKING_LIST_IMPLEMENT__
#include "arm_2d_scene_user_text_tracking_list.h"
#include "arm_2d_scene_user_button.h"
#include "microlink_board.h"
#if defined(RTE_Acceleration_Arm_2D_Helper_PFB)

#include <stdlib.h>
#include <string.h>
#include "ff.h"
#include "diskio.h"

 
// 定义条目类型结构  
typedef struct {  
    char *name;        // UI 显示字符串（含缩进与 [D]/[F] 前缀）
    char *full_path;   // 完整路径，用于打开/选择
    bool is_directory;
    uint8_t level;     // 层级：0 根，1 次级，...
} file_entry_t;  
#ifndef PATH_MAX
#define PATH_MAX 512
#endif
static file_entry_t *file_entries = NULL;  
static __disp_string_t *c_strListItemStrings = NULL;  
static uint16_t hwFileCount = 0;      // 当前已用条目
static uint16_t hwFileCapacity = 0;   // 当前分配容量

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
}   ARM_2D_FONT_Arial14_A8,
    ARM_2D_FONT_Arial14_A4,
    ARM_2D_FONT_Arial14_A2,
    ARM_2D_FONT_Arial14_A1;

    extern const arm_2d_tile_t c_tilebackground4RGB565;
    extern const arm_2d_tile_t c_tileCircleR24Mask;
/*============================ PROTOTYPES ==============
/*============================ PROTOTYPES ====================================*/
/*============================ LOCAL VARIABLES ===============================*/
// ====================== 内存管理 ======================
static bool ensure_capacity(uint16_t additional) {
    if (additional == 0) return true;
    uint32_t required = (uint32_t)hwFileCount + (uint32_t)additional;
    if (required <= hwFileCapacity) return true;

    // 计算新容量（以 2 的幂增长）
    uint32_t new_cap = (hwFileCapacity == 0) ? 16u : (uint32_t)hwFileCapacity;
    while (new_cap < required) {
        new_cap *= 2u;
        if (new_cap > 65535u) break; // 不超过 uint16_t 最大
    }
    if (new_cap > 65535u) return false;

    // 使用 malloc + memcpy 的方式，避免多次 realloc 回滚问题
    file_entry_t *ne = (file_entry_t *)malloc((size_t)new_cap * sizeof(file_entry_t));
    if (!ne) return false;
    __disp_string_t *ns = (__disp_string_t *)malloc((size_t)new_cap * sizeof(__disp_string_t));
    if (!ns) { free(ne); return false; }

    // 复制已有数据（若有）
    if (file_entries && hwFileCount) {
        memcpy(ne, file_entries, (size_t)hwFileCount * sizeof(file_entry_t));
        memcpy(ns, c_strListItemStrings, (size_t)hwFileCount * sizeof(__disp_string_t));
    }
    // 清零新分配区的剩余部分，避免未初始化指针导致的 double-free/crash
    if (new_cap > hwFileCount) {
        memset(&ne[hwFileCount], 0, (size_t)(new_cap - hwFileCount) * sizeof(file_entry_t));
        memset(&ns[hwFileCount], 0, (size_t)(new_cap - hwFileCount) * sizeof(__disp_string_t));
    }

    // 释放旧内存并替换
    free(file_entries);
    free(c_strListItemStrings);
    file_entries = ne;
    c_strListItemStrings = ns;
    hwFileCapacity = (uint16_t)new_cap;
    return true;
}

static void cleanup_filenames(void) {
    if (file_entries) {
        for (uint16_t i = 0; i < hwFileCount; i++) {
            if (file_entries[i].name) free(file_entries[i].name);
            if (file_entries[i].full_path) free(file_entries[i].full_path);
        }
        free(file_entries);
        file_entries = NULL;
    }
    if (c_strListItemStrings) {
        // c_strListItemStrings[i] 指针与 file_entries[i].name 指向同一块内存，
        // 所以不要单独 free 它们（已在上面 free 过），这里只释放数组本身。
        free(c_strListItemStrings);
        c_strListItemStrings = NULL;
    }
    hwFileCount = 0;
    hwFileCapacity = 0;
}


// ====================== 工具函数 ======================
// 根据 full_path 计算层级
static uint8_t calc_level_from_path(const char *path) {
    if (!path || path[0] == '\0') return 0;
    if (strcmp(path, "/") == 0) return 0;
    int cnt = 0;
    for (const char *p = path; *p; p++) if (*p == '/') cnt++;
    return (uint8_t)cnt;
}

// ---------- 打印 c_strListItemStrings 调试用 ----------
static void debug_print_c_strList(void) {
    printf("==== UI List Items ====\n");
    for (uint16_t i = 0; i < hwFileCount; i++) {
        const char *s = c_strListItemStrings[i] ? (const char*)c_strListItemStrings[i] : "(null)";
        const char *fp = file_entries[i].full_path ? file_entries[i].full_path : "(no-path)";
        printf("[%02d] lvl=%d  ui=\"%s\"  full=\"%s\"\n", i, file_entries[i].level, s, fp);
    }
    printf("=======================\n");
}

static int find_dir_entry_index(const char *path) {
    if (!path) return -1;
    for (int i = 0; i < (int)hwFileCount; i++) {
        if (file_entries[i].is_directory && file_entries[i].full_path &&
            strcmp(file_entries[i].full_path, path) == 0) {
            return i;
        }
    }
    return -1;
}
// ---------- helper: 检查目录是否已展开（避免重复展开） ----------
static bool dir_already_opened(const char *path, uint8_t base_level) {
    if (!path) return false;

    char prefix[PATH_MAX];
    size_t path_len = strlen(path);
    bool path_is_root = (path_len == 1 && path[0] == '/');

    if (path_is_root) {
        // 对根目录，我们把 prefix 设为 "/" ，再通过 level == base_level+1 来判断直接子级是否存在
        strncpy(prefix, "/", sizeof(prefix));
    } else {
        if (path[path_len - 1] == '/') snprintf(prefix, sizeof(prefix), "%s", path);
        else snprintf(prefix, sizeof(prefix), "%s/", path);
    }
    size_t prefix_len = strlen(prefix);

    for (int i = 0; i < (int)hwFileCount; i++) {
        if (!file_entries[i].full_path) continue;
        // 只看直接子级：level == base_level + 1
        if (file_entries[i].level == (uint8_t)(base_level + 1) &&
            strncmp(file_entries[i].full_path, prefix, prefix_len) == 0) {
            return true;
        }
    }
    return false;
}

// ====================== 关闭目录 ======================
// 删除该目录下所有子文件和子文件夹
static void close_directory(const char *path) {
    if (!path || !file_entries || hwFileCount == 0) return;

    size_t path_len = strlen(path);
    bool path_is_root = (path_len == 1 && path[0] == '/');

    char prefix[PATH_MAX];
    if (path_is_root) {
        strncpy(prefix, "/", sizeof(prefix));
    } else {
        if (path[path_len - 1] == '/') snprintf(prefix, sizeof(prefix), "%s", path);
        else snprintf(prefix, sizeof(prefix), "%s/", path);
    }
    size_t prefix_len = strlen(prefix);
    uint8_t base_level = calc_level_from_path(path);

    // 找到需要删除的范围：第一个 level > base_level 且 full_path 以 prefix 开头的条目
    uint16_t start = UINT16_MAX;
    for (uint16_t i = 0; i < hwFileCount; i++) {
        if (!file_entries[i].full_path) continue;
        if (file_entries[i].level > base_level &&
            strncmp(file_entries[i].full_path, prefix, prefix_len) == 0) {
            start = i;
            break;
        }
    }
    if (start == UINT16_MAX) return; // 没有子项

    uint16_t end = start;
    while (end < hwFileCount &&
           file_entries[end].full_path &&
           file_entries[end].level > base_level &&
           strncmp(file_entries[end].full_path, prefix, prefix_len) == 0) {
        end++;
    }

    // 释放资源（只释放 name/full_path）
    for (uint16_t i = start; i < end; i++) {
        if (file_entries[i].name) {
            free(file_entries[i].name);
            file_entries[i].name = NULL;
        }
        if (file_entries[i].full_path) {
            free(file_entries[i].full_path);
            file_entries[i].full_path = NULL;
        }
        c_strListItemStrings[i] = NULL;
    }

    // 移动剩余条目
    uint16_t tail_count = hwFileCount - end;
    if (tail_count > 0) {
        memmove(&file_entries[start], &file_entries[end], (size_t)tail_count * sizeof(file_entry_t));
        memmove(&c_strListItemStrings[start], &c_strListItemStrings[end], (size_t)tail_count * sizeof(__disp_string_t));
    }

    // 清理尾部（可选，避免悬挂指针）
    for (uint16_t k = start + tail_count; k < hwFileCount; k++) {
        file_entries[k].name = NULL;
        file_entries[k].full_path = NULL;
        file_entries[k].is_directory = false;
        file_entries[k].level = 0;
        c_strListItemStrings[k] = NULL;
    }

    hwFileCount -= (end - start);
}

static bool load_directory_append(const char *path) {
    if (!path) return false;

    FRESULT fr;
    DIR dir;
    FILINFO fno;
    uint16_t entry_count = 0;

    // 1) 统计条目数（排除以 . 开头和 "System Volume Information"）
    fr = f_opendir(&dir, path);
    if (fr != FR_OK) return false;
    while (1) {
        fr = f_readdir(&dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0) break;
        if (fno.fname[0] != '.' && strcmp(fno.fname, "System Volume Information") != 0) entry_count++;
    }
    f_closedir(&dir);
    if (entry_count == 0) return true; // 无新条目，视为成功

    // 2) 插入位置：父目录索引 + 1，或尾部
    uint8_t base_level = calc_level_from_path(path);
    if (dir_already_opened(path, base_level)) {
        close_directory(path);
        return true; // 已展开，退出
    }

    int dir_idx = find_dir_entry_index(path);
    uint16_t insert_idx = (dir_idx >= 0) ? (uint16_t)(dir_idx + 1) : hwFileCount;

    // 3) 确保容量
    if (!ensure_capacity(entry_count)) return false;

    // 4) 将尾部数据向后移动，给新条目腾出空间
    uint16_t tail_count = hwFileCount - insert_idx;
    if (tail_count > 0) {
        memmove(&file_entries[insert_idx + entry_count],
                &file_entries[insert_idx],
                (size_t)tail_count * sizeof(file_entry_t));
        memmove(&c_strListItemStrings[insert_idx + entry_count],
                &c_strListItemStrings[insert_idx],
                (size_t)tail_count * sizeof(__disp_string_t));
    }

    // 初始化新插入区，避免部分失败时 free 未初始化指针
    for (uint16_t k = insert_idx; k < insert_idx + entry_count; k++) {
        file_entries[k].name = NULL;
        file_entries[k].full_path = NULL;
        file_entries[k].is_directory = false;
        file_entries[k].level = 0;
        c_strListItemStrings[k] = NULL;
    }

    // 5) 重新遍历并填充
    fr = f_opendir(&dir, path);
    if (fr != FR_OK) {
        // 回滚：把尾部移回原位
        if (tail_count > 0) {
            memmove(&file_entries[insert_idx],
                    &file_entries[insert_idx + entry_count],
                    (size_t)tail_count * sizeof(file_entry_t));
            memmove(&c_strListItemStrings[insert_idx],
                    &c_strListItemStrings[insert_idx + entry_count],
                    (size_t)tail_count * sizeof(__disp_string_t));
            // 清空尾部（可选）
            for (uint16_t k = insert_idx + tail_count; k < insert_idx + tail_count + entry_count; k++) {
                file_entries[k].name = NULL;
                file_entries[k].full_path = NULL;
                c_strListItemStrings[k] = NULL;
            }
        }
        return false;
    }

    uint16_t filled = 0;
    uint16_t idx = insert_idx;
    size_t path_len = strlen(path);
    bool path_is_root = (path_len == 1 && path[0] == '/');

    while (filled < entry_count) {
        fr = f_readdir(&dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0) break;
        if (fno.fname[0] == '.' || strcmp(fno.fname, "System Volume Information") == 0) continue;

        // full_path 长度计算 (包含末尾的 '\0')
        size_t name_len = strlen(fno.fname);
        size_t full_len = path_len + (path_is_root ? 0 : 1) + name_len + 1;
        char *full = (char *)malloc(full_len);
        if (!full) {
            // 回滚：释放已填充的新条目（仅释放那些非 NULL 的）
            for (uint16_t j = insert_idx; j < idx; j++) {
                if (file_entries[j].name) free(file_entries[j].name);
                if (file_entries[j].full_path) free(file_entries[j].full_path);
                file_entries[j].name = NULL;
                file_entries[j].full_path = NULL;
            }
            // 把尾部数据移回原位
            if (tail_count > 0) {
                memmove(&file_entries[insert_idx],
                        &file_entries[insert_idx + entry_count],
                        (size_t)tail_count * sizeof(file_entry_t));
                memmove(&c_strListItemStrings[insert_idx],
                        &c_strListItemStrings[insert_idx + entry_count],
                        (size_t)tail_count * sizeof(__disp_string_t));
            }
            f_closedir(&dir);
            return false;
        }
        if (path_is_root) snprintf(full, full_len, "/%s", fno.fname);
        else snprintf(full, full_len, "%s/%s", path, fno.fname);

        // 生成显示字符串（level = base_level + 1）
        uint8_t level = (uint8_t)(base_level + 1);         // <-- 关键：必须是 base_level + 1
        size_t indent = (level > 1) ? ((size_t)(level - 1) * 1) : 0;
        const char *prefix = (fno.fattrib & AM_DIR) ? "壹" : "贰";
        size_t prefix_len = strlen(prefix);
        size_t disp_len = indent + prefix_len + name_len + 1;
        char *disp = malloc(disp_len);
        if (!disp) {
            free(full);
            for (uint16_t j = insert_idx; j < idx; j++) {
                if (file_entries[j].name) free(file_entries[j].name);
                if (file_entries[j].full_path) free(file_entries[j].full_path);
                file_entries[j].name = NULL;
                file_entries[j].full_path = NULL;
            }
            if (tail_count > 0) {
                memmove(&file_entries[insert_idx],
                        &file_entries[insert_idx + entry_count],
                        (size_t)tail_count * sizeof(file_entry_t));
                memmove(&c_strListItemStrings[insert_idx],
                        &c_strListItemStrings[insert_idx + entry_count],
                        (size_t)tail_count * sizeof(__disp_string_t));
            }
            f_closedir(&dir);
            return false;
        }

        // 填充 disp（先空格，再 [D]/[F] 再名字）
        memset(disp, '\0', disp_len);
        memset(disp, ' ', indent);
        memcpy(disp + indent, prefix, prefix_len);
        memcpy(disp + indent + prefix_len, fno.fname, name_len + 1);

        // 填入结构体（保证指针非 NULL）
        file_entries[idx].name = disp;
        file_entries[idx].full_path = full;
        file_entries[idx].is_directory = (fno.fattrib & AM_DIR) != 0;
        file_entries[idx].level = level;
        c_strListItemStrings[idx] = (__disp_string_t)disp;

        idx++;
        filled++;
    }

    f_closedir(&dir);

    // 更新计数
    hwFileCount += filled;
    return true;
}


static 
arm_fsm_rt_t my_arm_2d_text_list_draw_list_core_item( 
                                      arm_2d_list_item_t *ptItem,
                                      const arm_2d_tile_t *ptTile,
                                      bool bIsNewFrame,
                                      arm_2d_list_item_param_t *ptParam)
{
    text_list_t *ptThis = (text_list_t *)ptItem->ptListView;
    __simple_list_cfg_t *ptCFG = &this.use_as____simple_list_t.tSimpleListCFG;

    ARM_2D_UNUSED(ptItem);
    ARM_2D_UNUSED(bIsNewFrame);
    ARM_2D_UNUSED(ptTile);
    ARM_2D_UNUSED(ptParam);

    arm_2d_canvas(ptTile, __top_container) {
        arm_lcd_text_set_font(ptCFG->ptFont);

        arm_lcd_text_set_colour(ptCFG->tFontColour, ptCFG->tBackgroundColour);
        //arm_lcd_text_set_display_mode(ARM_2D_DRW_PATN_MODE_COPY);
        
        arm_lcd_text_set_target_framebuffer((arm_2d_tile_t *)ptTile);
        arm_lcd_text_set_scale(0.0f);
        arm_lcd_text_set_opacity(255);

        /* print numbers */
        __simple_list_item_printf(
                &this.use_as____simple_list_t,
                 &__top_container,
                 this.use_as____simple_list_t.tSimpleListCFG.tTextAlignment,
                 this.tTextListCFG.pchFormatString,
                 text_list_get_item_string(ptThis, ptItem->hwID));
    
        arm_lcd_text_set_target_framebuffer(NULL);
    }
    
    return arm_fsm_rt_cpl;
}

void initialize_text_list(arm_2d_scene_t *ptScene,const char *path)
{
    user_scene_text_tracking_list_t *ptThis = (user_scene_text_tracking_list_t *)ptScene;
    ARM_2D_UNUSED(ptThis);
    /* ------------   initialize members of user_scene_text_tracking_list_t begin ---------------*/

    /* initialize text list */
    do {
        if (!load_directory_append(path)) {  
            // 加载失败的处理，可以使用默认列表或显示错误信息  
            return;  
        }  
        arm_2d_helper_pi_slider_cfg_t tpidCFG = {
            .fProportion = 0.3,
            .fIntegration = 0,
            .nInterval = 20
        };          
        text_tracking_list_cfg_t tCFG = {
            //.tSettings = {
                .bIndicatorAutoSize = true,
                .tIndicator.tColour = GLCD_COLOR_NIXIE_TUBE,
            //},

            .use_as__text_list_cfg_t = {
                .ptStrings = (__disp_string_t *)c_strListItemStrings,

                .use_as____simple_list_cfg_t = {
                    .hwCount = hwFileCount,
                    
                    .tFontColour = GLCD_COLOR_WHITE,
                    .tBackgroundColour = GLCD_COLOR_BLACK,
                    .bIgnoreBackground = true,
                    .bUseMonochromeMode = false,
                    .bShowScrollingBar = true,
                    .chScrollingBarAutoDisappearTimeX100Ms = 1,
                    .ScrollingBar.tColour = GLCD_COLOR_NIXIE_TUBE,
                    .bUsePIMode = true,
                    .fnOnDrawListItem = my_arm_2d_text_list_draw_list_core_item,
                    .bDisableRingMode = true,     /* you can disable the list ring mode here */
                    .ptPISliderCFG = &tpidCFG,
                    .chNextPadding = 1,
                    .chPreviousPadding = 1,
                    .tListSize = {
                        .iHeight = 0,           /* automatically set the height */
                        .iWidth = 0,            /* automatically set the width */
                    },
                    .tTextAlignment = ARM_2D_ALIGN_MIDDLE_LEFT,

                    .ptFont = (arm_2d_font_t *)&ARM_2D_FONT_Round16_A4,

                    .bUseDirtyRegion = false,
                    .ptTargetScene = &this.use_as__arm_2d_scene_t,
                },
            },
        };
        text_tracking_list_init( 
            &this.tList, 
            &tCFG,
            &ARM_2D_LIST_CALCULATOR_NORMAL_FIXED_SIZED_ITEM_NO_STATUS_CHECK_VERTICAL);

        //text_tracking_list_move_selection(&this.tList, 0, 0);

    } while(0);

    /* ------------   initialize members of user_scene_text_tracking_list_t end   ---------------*/
}

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

static void __on_scene_text_tracking_list_load(arm_2d_scene_t *ptScene)
{
    user_scene_text_tracking_list_t *ptThis = (user_scene_text_tracking_list_t *)ptScene;
    ARM_2D_UNUSED(ptThis);
    initialize_text_list(ptScene,"/Python");
}

static void __after_scene_text_tracking_list_switching(arm_2d_scene_t *ptScene)
{
    user_scene_text_tracking_list_t *ptThis = (user_scene_text_tracking_list_t *)ptScene;
    ARM_2D_UNUSED(ptThis);

}

static void __on_scene_text_tracking_list_depose(arm_2d_scene_t *ptScene)
{
    user_scene_text_tracking_list_t *ptThis = (user_scene_text_tracking_list_t *)ptScene;
    ARM_2D_UNUSED(ptThis);

    cleanup_filenames();

    text_tracking_list_depose(&this.tList);


    ptScene->ptPlayer = NULL;
    
    arm_foreach(int64_t,this.lTimestamp, ptItem) {
        *ptItem = 0;
    }

    if (!this.bUserAllocated) {
        __arm_2d_free_scratch_memory(ARM_2D_MEM_TYPE_UNSPECIFIED, ptScene);
    }
    
}

/*----------------------------------------------------------------------------*
 * Scene text_tracking_list                                                            *
 *----------------------------------------------------------------------------*/

static void __on_scene_text_tracking_list_background_start(arm_2d_scene_t *ptScene)
{
    user_scene_text_tracking_list_t *ptThis = (user_scene_text_tracking_list_t *)ptScene;
    ARM_2D_UNUSED(ptThis);

}

static void __on_scene_text_tracking_list_background_complete(arm_2d_scene_t *ptScene)
{
    user_scene_text_tracking_list_t *ptThis = (user_scene_text_tracking_list_t *)ptScene;
    ARM_2D_UNUSED(ptThis);

}


static void __on_scene_text_tracking_list_frame_start(arm_2d_scene_t *ptScene)
{
    user_scene_text_tracking_list_t *ptThis = (user_scene_text_tracking_list_t *)ptScene;
    ARM_2D_UNUSED(ptThis);

    text_tracking_list_on_frame_start(&this.tList);
}

static void __on_scene_text_tracking_list_frame_complete(arm_2d_scene_t *ptScene)
{
    user_scene_text_tracking_list_t *ptThis = (user_scene_text_tracking_list_t *)ptScene;
    ARM_2D_UNUSED(ptThis);
    static int16_t iY;
#if 0
    /* switch to next scene after 7s */
    if (arm_2d_helper_is_time_out(7000, &this.lTimestamp[0])) {                 
          arm_2d_scene_player_switch_to_next_scene(ptScene->ptPlayer);
    }
#endif
    /* message handling */
    /* message handling */
    RG_PROCESS_MSG(this.ptGUI, ptMSG) {
        ARM_2D_UNUSED(bIsMessageHandled);
         
        switch(ngy_helper_msg_item_get_id(ptMSG)) {
            case NGY_MSG_GESTURE_EVT_WHEEL:
            do {
                ngy_msg_gesture_evt_t *ptKeyMSG = (ngy_msg_gesture_evt_t *)ptMSG;
                if(ptKeyMSG->tRegion.tLocation.iY > iY){
                    text_tracking_list_move_selection(&this.tList, -1, 10);
                }else{
                    text_tracking_list_move_selection(&this.tList, 1, 10);
                }
                iY = ptKeyMSG->tRegion.tLocation.iY;
                bIsMessageHandled = true;
            }while(0);
            break;
            /* key pressed */
            case NGY_MSG_KEY_EVT_PRESSED:
                
                do {
                    // 获取当前选中项目的ID  
                    this.selected_id = text_tracking_list_get_selected_item_id(&this.tList);  
                    // 检查ID是否有效  
                    if (this.selected_id < hwFileCount && file_entries != NULL) {  
                        // 检查是否为文件夹  
                        bool is_directory = file_entries[this.selected_id].is_directory; 
                        if(is_directory){
                            text_tracking_list_depose(&this.tList);
                            initialize_text_list(ptScene,file_entries[this.selected_id].full_path);
                            text_tracking_list_request_redraw_list(&this.tList);
                            text_tracking_list_move_selection(&this.tList,this.selected_id,1);
                        }else{
                            printf("selected: %s \n", file_entries[this.selected_id].full_path); 
                            arm_2d_scene_player_switch_to_next_scene(ptScene->ptPlayer); 
                        }
                        //debug_print_c_strList();
                    }
                    bIsMessageHandled = true;
                } while(0);
                break;
            default:
                break;
        }
    }
}

static void __before_scene_text_tracking_list_switching_out(arm_2d_scene_t *ptScene)
{
    user_scene_text_tracking_list_t *ptThis = (user_scene_text_tracking_list_t *)ptScene;
    ARM_2D_UNUSED(ptThis);
    user_scene_button_t *ptButtonOffLine = 
        arm_2d_scene_button_init(this.use_as__arm_2d_scene_t.ptPlayer);
    assert(NULL != ptButtonOffLine);
    ptButtonOffLine->ptGUI = &g_tMyGUI;
    memset(ptButtonOffLine->chFilePath, 0, sizeof(ptButtonOffLine->chFilePath));
    if(file_entries != NULL && this.selected_id < hwFileCount && file_entries[this.selected_id].full_path != NULL){
        strncpy(ptButtonOffLine->chFilePath, file_entries[this.selected_id].full_path,sizeof(ptButtonOffLine->chFilePath)-1);
        ptButtonOffLine->chFilePath[sizeof(ptButtonOffLine->chFilePath) - 1] = '\0';
    }
}

static
IMPL_PFB_ON_DRAW(__pfb_draw_scene_text_tracking_list_handler)
{
    ARM_2D_PARAM(pTarget);
    ARM_2D_PARAM(ptTile);
    ARM_2D_PARAM(bIsNewFrame);

    user_scene_text_tracking_list_t *ptThis = (user_scene_text_tracking_list_t *)pTarget;
    arm_2d_size_t tScreenSize = ptTile->tRegion.tSize;
    ARM_2D_UNUSED(tScreenSize);

    arm_2d_canvas(ptTile, __top_canvas) {
        arm_2d_layout(__top_canvas) {

            __item_line_dock_vertical(24,0,0,15,0) {

                arm_2d_fill_colour_with_opacity(ptTile, &__item_region,  (arm_2d_color_rgb565_t){GLCD_COLOR_BLACK},160);

                /* print label */
                arm_lcd_text_set_target_framebuffer((arm_2d_tile_t *)ptTile);
                arm_lcd_text_set_font(&ARM_2D_FONT_Round16_A4);
                arm_lcd_text_set_draw_region(&__item_region);
                arm_lcd_text_set_display_mode(ARM_2D_DRW_PATN_MODE_COPY);
                
                arm_lcd_printf_label(ARM_2D_ALIGN_MIDDLE_LEFT, " 文件");
            }

                __item_line_dock_vertical(4,    /* left margin */
                                          4,    /* right margin */
                                          2,    /* top margin */
                                          2) {  /* bottom margin */

                /* draw text list */
                arm_2d_dock_with_margin(__item_region, 
                                        0,  /* left margin */
                                        0,  /* right margin */
                                        0,  /* top margin */
                                        0) {/* bottom margin */
                    arm_2d_fill_colour_with_opacity(ptTile, &__item_region,  (arm_2d_color_rgb565_t){GLCD_COLOR_BLACK},160);
                    ARM_2D_OP_WAIT_ASYNC();
         //           arm_lcd_text_set_display_mode(ARM_2D_DRW_PATH_MODE_COMP_FG_COLOUR);
                    while(arm_fsm_rt_cpl != text_tracking_list_show( &this.tList, 
                                                    ptTile, 
                                                    &__dock_region, 
                                                    bIsNewFrame));
                }
            }
        }
    }
    ARM_2D_OP_WAIT_ASYNC();

    return arm_fsm_rt_cpl;
}

ARM_NONNULL(1)
user_scene_text_tracking_list_t *__arm_2d_scene_text_tracking_list_init(
                                        arm_2d_scene_player_t *ptDispAdapter, 
                                        user_scene_text_tracking_list_t *ptThis)
{
    bool bUserAllocated = false;
    assert(NULL != ptDispAdapter);

    if (NULL == ptThis) {
        ptThis = (user_scene_text_tracking_list_t *)
                    __arm_2d_allocate_scratch_memory(   sizeof(user_scene_text_tracking_list_t),
                                                        __alignof__(user_scene_text_tracking_list_t),
                                                        ARM_2D_MEM_TYPE_UNSPECIFIED);
        assert(NULL != ptThis);
        if (NULL == ptThis) {
            return NULL;
        }
    } else {
        bUserAllocated = true;
    }

    memset(ptThis, 0, sizeof(user_scene_text_tracking_list_t));

    *ptThis = (user_scene_text_tracking_list_t){
        .use_as__arm_2d_scene_t = {

            /* the canvas colour */
            .tCanvas = {GLCD_COLOR_BLACK}, 

            /* Please uncommon the callbacks if you need them
             */
            .fnOnLoad       = &__on_scene_text_tracking_list_load,
            .fnScene        = &__pfb_draw_scene_text_tracking_list_handler,
            .fnAfterSwitch  = &__after_scene_text_tracking_list_switching,

            /* if you want to use predefined dirty region list, please uncomment the following code */
            //.ptDirtyRegion  = (arm_2d_region_list_item_t *)s_tDirtyRegions,
            

            //.fnOnBGStart    = &__on_scene_text_tracking_list_background_start,
            //.fnOnBGComplete = &__on_scene_text_tracking_list_background_complete,
            .fnOnFrameStart = &__on_scene_text_tracking_list_frame_start,
            .fnBeforeSwitchOut = &__before_scene_text_tracking_list_switching_out,
            .fnOnFrameCPL   = &__on_scene_text_tracking_list_frame_complete,
            .fnDepose       = &__on_scene_text_tracking_list_depose,

            .bUseDirtyRegionHelper = false,
        },
        .bUserAllocated = bUserAllocated,
    };

    arm_2d_scene_player_append_scenes(  ptDispAdapter, 
                                        &this.use_as__arm_2d_scene_t, 
                                        1);
    return ptThis;
}


#if defined(__clang__)
#   pragma clang diagnostic pop
#endif

#endif