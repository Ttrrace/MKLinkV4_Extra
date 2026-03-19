#ifndef __SCOPE_TASK_H__
#define __SCOPE_TASK_H__

#include "rtthread.h"
#include "my_scope.h"

typedef struct
{       
   uint16_t x_time_step[9];
   uint32_t y_vol_step[7];	
   uint32_t y_cur_step[7];	
	
   int8_t  index_vol;
   int8_t  index_cur;	

   int16_t  upper_limit_vol;
   int16_t  upper_limit_cur;

   int8_t   x_level;
   bool     bshowscope;
   bool     bCurUpdate;
   bool     bVolUpdate;

}Scope_cfg_t;
extern Scope_cfg_t Scope_cfg;
extern Scope_t My_scope_v;
extern Scope_t My_scope_c;

extern int scope_task_init(void);
#endif