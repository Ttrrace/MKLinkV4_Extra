#include "microlink_board.h"
#include "my_scope.h"

#undef  this
#define this (*scope)

#define MY_COLOR_ORANGE       __RGB( 239, 148, 0    )
#define SWITCH_BYTE(x)       (x << 8) + (x >> 8)

static	arm_2d_region_t tXY_region = 
	{
	  .tLocation.iX = 10,
	  .tLocation.iY = 40,
	  .tSize.iHeight = 170,	
	  .tSize.iWidth  = 220,			
	};		
	
void Draw_Grid(Scope_t *scope, const arm_2d_tile_t *ptSource )
{
	arm_2d_region_t tHorizon_Line = 
	 {
		.tLocation = {
		   .iX = 10,
		   .iY = 40,
		},
		.tSize = {
			.iWidth  = 220,
			.iHeight = 1,
		},
	 };			
	
	arm_2d_fill_colour_with_opacity(
                                         ptSource, 
                                         &tHorizon_Line,	
                                         (__arm_2d_color_t){MY_COLOR_ORANGE},
	                                 255 - 155
						 );	
	ARM_2D_OP_WAIT_ASYNC();		
						 
	tHorizon_Line.tLocation.iY += 170;		

	arm_2d_fill_colour_with_opacity(
                                         ptSource, 
                                         &tHorizon_Line,	
                                         (__arm_2d_color_t){MY_COLOR_ORANGE},
                                         192
						 );							 
        ARM_2D_OP_WAIT_ASYNC();
						 
	arm_2d_region_t tVertical_Line = 
	 {
		.tLocation = {
		   .iX = 10,
		   .iY = 40,
		},
		.tSize = {
			.iWidth  = 1,
			.iHeight = 170,
		},
	 };							 
						 
	arm_2d_fill_colour_with_opacity(
						 ptSource, 
						 &tVertical_Line,	
						 (__arm_2d_color_t){MY_COLOR_ORANGE},
	                     192
						 );							 
	ARM_2D_OP_WAIT_ASYNC();	
						 
	tVertical_Line.tLocation.iX += 220;							 
	arm_2d_fill_colour_with_opacity(
						 ptSource, 
						 &tVertical_Line,	
						 (__arm_2d_color_t){MY_COLOR_ORANGE},
	                     255 - 155
						 );							 
							 
						 
	for(uint8_t i = 0; i < 16 ; i++ ){
		tHorizon_Line.tLocation.iY -= 10;
		ARM_2D_OP_WAIT_ASYNC();
		arm_2d_fill_colour_with_opacity(
						 ptSource, 
						 &tHorizon_Line,	
						 (__arm_2d_color_t){MY_COLOR_ORANGE},
	                     255 - 200
						 );		
	
	}

	for(uint8_t i = 0; i < 15 ; i++ ){
		tVertical_Line.tLocation.iX -= 20;
		ARM_2D_OP_WAIT_ASYNC();
		arm_2d_fill_colour_with_opacity(
						 ptSource, 
						 &tVertical_Line,	
						 (__arm_2d_color_t){MY_COLOR_ORANGE},
	                     255 - 200
						 );			
			
	}	

}
uint16_t cnt = 0;
float Yoffset = 0;
arm_2d_location_t   tlocation;
void Draw_Line(Scope_t *scope, const arm_2d_tile_t *ptSource,uint_fast16_t Colour)
{
 
	static int16_t last_y = 0;
	int16_t max_dot = 170;
	cnt = 440;
        peek_bytes_queue(&this.source_data_queue,this.value,cnt);
	arm_2d_location_t   tlocation;
        
	for(uint16_t i  = 0;i < cnt/2;i ++){
                Yoffset = (this.value[i] * scope->k);
		tlocation.iX = this.tXY_region.tLocation.iX + i; 
		tlocation.iY = this.tXY_region.tLocation.iY + this.tXY_region.tSize.iHeight
                               - (uint16_t)Yoffset; 
		
		if(tlocation.iY < max_dot){
		   max_dot = tlocation.iY;
		}
		
		if(tlocation.iY != 170){
		   arm_2dp_rgb565_draw_point(&this.tpoint[0],ptSource,tlocation,Colour,255);
		}	
		int16_t declay = tlocation.iY - last_y;
		if(i){
			if((declay) <= - 2){
                            arm_2d_region_t tVertical_Line = 
                             {
                                .tLocation = {
                                   .iX = tlocation.iX,
                                   .iY = tlocation.iY,
                                },
                                .tSize = {
                                        .iWidth  = 1,
                                        .iHeight = (-1) * declay,
                                },
                             };				
                            arm_2dp_rgb16_fill_colour(
                                                       NULL,
                                                       ptSource, 
                                                       &tVertical_Line,	
                                                       Colour								
                                                       );	
                            ARM_2D_OP_WAIT_ASYNC();
				 
			}else if((declay) >= 2 ){
				arm_2d_region_t tVertical_Line = 
				 {
                                    .tLocation = {
                                       .iX = tlocation.iX,
                                       .iY = last_y,
                                    },
                                    .tSize = {
                                            .iWidth  = 1,
                                            .iHeight = declay,
                                    },
				 };				
				arm_2dp_rgb16_fill_colour(
                                                         NULL,
                                                         ptSource, 
                                                         &tVertical_Line,	
                                                         Colour								
                                                         );	
				ARM_2D_OP_WAIT_ASYNC();
			}		
	    }
		last_y = tlocation.iY;
	}
	//    if(Control_block.start_flag){   //仅在数据更新时调节
                if(max_dot < this.tXY_region.tLocation.iY){ //有一个点大于总量程，增大单位长度
                   scope->steplen_up = true;
                }else if(max_dot > 140){
                   scope->steplen_down = true;  //所有点小于25%量程，减小单位长度
                }
        //    }	
	    scope->draw_line_over = true;
}

