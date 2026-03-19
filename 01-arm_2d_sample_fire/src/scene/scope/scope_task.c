#include "microlink_board.h"
#include "scope_task.h"
#include "arm_2d_scene_user_waveform.h"

static char scope_thread_stack[4096];
static struct rt_thread scope_thread;

#define WINDOW_SIZE 220


extern uint32_t Vbus;
uint16_t  scope_v_buffer[220] = {0};
uint16_t  scope_i_buffer[220] = {0};

extern void arm_2d_scene_waveform_enqueue( user_scene_waveform_t *ptThis, 
                                    int16_t *piSamples, 
                                    uint16_t hwSampleCount);

extern user_scene_waveform_t *ptwaveform;
enum {
    REMAIN,
    SCALE_UP,
    SCALE_DOWN,
};
int16_t sample[2] = {0};
uint8_t scale_flag[2] = {0};

/*
   extern uint8_t scale_flag; 
*/

Scope_cfg_t Scope_cfg = {
	.x_time_step = {5,10,50,100,500,3000,9000},  //1.0s,2s,10s,20s,100s,10min 30min
	.y_cur_step  = {2,5,10,20,50,100,200},//34mA 85mA 170mA 3000mA
	.y_vol_step  = {200,500},	//3V   8V
	.x_level     =  2,
	.index_vol   =  1,
        .index_cur   =  0,
        .upper_limit_vol = 170,
        .upper_limit_cur = 170,
};

Scope_t My_scope_v = 
{
	.tXY_region.tLocation.iX = 10,
	.tXY_region.tLocation.iY = 40,	
	.tXY_region.tSize.iHeight = 170,	
	.tXY_region.tSize.iWidth  = 220,	
};				

Scope_t My_scope_c = 
{
	.tXY_region.tLocation.iX = 10,
	.tXY_region.tLocation.iY = 40,	
	.tXY_region.tSize.iHeight = 170,	
	.tXY_region.tSize.iWidth  = 220,	
};	

typedef struct {
    uint8_t max_value;
    uint16_t index;
} arm_curve_peak_t;


/*
   Plan A
*/

/*
extern bool __arm_loader_io_window_seek(uintptr_t pTarget, void *ptLoader, int32_t offset, int32_t whence);
extern size_t __arm_loader_io_window_read(uintptr_t pTarget, void *ptLoader, uint8_t *pchBuffer, size_t tSize);

bool arm2d_curve_find_max(   uintptr_t ptFifo,
                             uint16_t *pMaxValue,
                             uint16_t *pMaxIndex)
{
    assert(ptFifo != NULL);
    assert(pMaxValue != NULL);
    assert(pMaxIndex != NULL);

    uint16_t value;
    uint16_t index = 0;

 
    if (!__arm_loader_io_window_seek(ptFifo, NULL, 0, SEEK_SET)) {
        return false;   // 空窗口
    }

  
    if (__arm_loader_io_window_read(ptFifo, NULL, &value, 2) != 2) {
        return false;
    }

    uint8_t maxVal = value;
    uint16_t maxIdx = 0;

    index = 1;


    while (__arm_loader_io_window_read(ptFifo, NULL, &value, 2) == 2) {

        if (value > maxVal) {
            maxVal = value;
            maxIdx = index;
        }

        index++;
    }

    *pMaxValue = maxVal;
    *pMaxIndex = maxIdx;

    return true;
}
*/

/*
   Plan B
*/

typedef struct {
    uint16_t value;   // 数据值
    uint16_t index;  // 对应 window 内的位置
} max_node_t;

typedef struct {
    max_node_t buf[WINDOW_SIZE];  // 队列数组
    uint16_t head;                // 队头索引
    uint16_t tail;                // 队尾索引
} max_queue_t;

max_queue_t max_queue_vol;
max_queue_t max_queue_cur;

void max_queue_init(max_queue_t *q)
{
    q->head = 0;
    q->tail = 0;
}

void max_queue_push(max_queue_t *q, uint16_t value, uint16_t index)
{
    // 1. 移除队尾比新值小的元素（保持单调递减）
    while (q->tail != q->head) {
        uint16_t last = (q->tail + WINDOW_SIZE - 1) % WINDOW_SIZE;
        if (q->buf[last].value < value) {   // 推荐用 <
            q->tail = last;
        } else {
            break;
        }
    }

    // 2. 新元素入队
    q->buf[q->tail].value = value;
    q->buf[q->tail].index = index;
    q->tail = (q->tail + 1) % WINDOW_SIZE;

    // 3. 移除窗口外的元素（可能不止一个）
    while (q->head != q->tail &&
           q->buf[q->head].index <= index - WINDOW_SIZE)
    {
        q->head = (q->head + 1) % WINDOW_SIZE;
    }
}


uint16_t max_queue_get_max(max_queue_t *q)
{
    if (q->head == q->tail) {
        return 0; // 空队列
    }
    return q->buf[q->head].value;
}

uint16_t max_queue_get_max_index(max_queue_t *q)
{
    if (q->head == q->tail) {
        return 0;
    }
    return q->buf[q->head].index;
}

uint16_t test_val = 0;
uint16_t test_cur = 0;

void adaptive_steplen(float vol, float cur)
{
    static uint16_t voltage = 0;
    static uint16_t current = 0;
    static uint32_t check_cnt = 0;
    
        check_cnt ++;

        voltage = (int16_t)(vol * 0.02);
        current = (int16_t)(cur * 5000);

        max_queue_push(&max_queue_vol,voltage,check_cnt);
        max_queue_push(&max_queue_cur,current,check_cnt);

        test_val = max_queue_get_max(&max_queue_vol);
        test_cur = max_queue_get_max(&max_queue_cur);

        if(voltage > (Scope_cfg.upper_limit_vol)){              //超出上限
               Scope_cfg.index_vol ++;
            if(Scope_cfg.index_vol > 1){
                  Scope_cfg.index_vol = 1; 
            }else{  
              Scope_cfg.upper_limit_vol = 170 * Scope_cfg.y_vol_step[Scope_cfg.index_vol]/Scope_cfg.y_vol_step[0];
              Scope_cfg.bVolUpdate = true;
            }
        }else if(max_queue_get_max(&max_queue_vol) < (Scope_cfg.upper_limit_vol * 0.2)){
            Scope_cfg.index_vol --;
            if(Scope_cfg.index_vol < 0){
               Scope_cfg.index_vol = 0; 
            }else{
               Scope_cfg.upper_limit_vol = 170 * Scope_cfg.y_vol_step[Scope_cfg.index_vol]/Scope_cfg.y_vol_step[0];
               Scope_cfg.bVolUpdate = true;
            }
        }

        if(current > (Scope_cfg.upper_limit_cur)){
               Scope_cfg.index_cur ++;
            if(Scope_cfg.index_cur > 6){
                  Scope_cfg.index_cur = 6; 
            }else{  
              Scope_cfg.upper_limit_cur = 170 * Scope_cfg.y_cur_step[Scope_cfg.index_cur]/Scope_cfg.y_cur_step[0];
              Scope_cfg.bCurUpdate = true;
            }
        }else if(max_queue_get_max(&max_queue_cur) < (Scope_cfg.upper_limit_cur * 0.2)){
            Scope_cfg.index_cur --;
            if(Scope_cfg.index_cur < 0){
               Scope_cfg.index_cur = 0; 
            }else{
               Scope_cfg.upper_limit_cur = 170 * Scope_cfg.y_cur_step[Scope_cfg.index_cur]/Scope_cfg.y_cur_step[0];
               Scope_cfg.bCurUpdate = true;
            }
        }

        sample[0] = voltage;
        sample[1] = current;   


        arm_2d_scene_waveform_enqueue(  ptwaveform,
                                        &sample,
                                        1);

        ptwaveform->bDataUpdate = true;   
	rt_thread_mdelay(Scope_cfg.x_time_step[Scope_cfg.x_level]);	
        	
}

uint16_t val = 0;
uint16_t val1 = 100;

static void scope_thread_entry(void *param)
{	
    while(1){	
            if(Scope_cfg.bshowscope){
              adaptive_steplen(g_tMicroInfo.vcc_voltage,g_tMicroInfo.iref_voltage); 
            }else{
              rt_thread_mdelay(100); 
            }
    } 
}

uint16_t init_zero[220] = {0};

int scope_task_init(void)
{   
    max_queue_init(&max_queue_vol);
    max_queue_init(&max_queue_cur);	

    rt_thread_init(&scope_thread,
                   "scope",
                   scope_thread_entry,
                   RT_NULL,
                   &scope_thread_stack[0],
                   sizeof(scope_thread_stack),
                   24, 50);
    rt_thread_startup(&scope_thread);

    return 0;
}


