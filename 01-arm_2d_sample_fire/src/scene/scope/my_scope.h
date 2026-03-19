#ifndef __MY_SCOPE_H__
#define __MY_SCOPE_H__

#include "arm_2d_helper.h"
#include "microboot.h"

typedef struct
{       
	byte_queue_t    source_data_queue; //
	arm_2d_region_t tXY_region;           //
	arm_2d_op_drw_pt_t  tpoint[2];
	
	int16_t             value[220];

        uint32_t Delta_cur;
        uint32_t Delta_vol;	
	
	float k;
	
	bool  steplen_up;
	bool  steplen_down;	

	bool  draw_line_over;
}Scope_t;

extern void Draw_Grid(Scope_t *scope, const arm_2d_tile_t *ptSource );
extern void Draw_Line(Scope_t *scope, const arm_2d_tile_t *ptSource,uint_fast16_t Colour);

#endif