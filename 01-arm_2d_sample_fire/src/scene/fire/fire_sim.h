#ifndef __FIRE_SIM_H__
#define __FIRE_SIM_H__
#include "stdint.h"


// 最大旋涡数量
#define MAX_SWIRLS 60

// 场类型枚举
enum {
	U_FIELD = 0,    // X方向速度场
	V_FIELD,        // Y方向速度场
	T_FIELD,        // 温度场
};
// 流体模拟结构体
typedef struct {
	int numX;           // X方向网格数（含边界）
	int numY;           // Y方向网格数（含边界）
	int numCells;       // 总网格数
	float h;            // 网格大小（物理单位）
	
	float *u;           // X方向速度
	float *v;           // Y方向速度
	float *newU;        // 下一时间步X方向速度
	float *newV;        // 下一时间步Y方向速度
	uint8_t *s;           // 固体障碍物标记（1=流体，0=固体）
	float *t;           // 温度场
	float *newT;        // 下一时间步温度场
	
	int numSwirls;              // 当前旋涡数量
	float swirlGlobalTime;       // 全局旋涡时间
	float swirlX[MAX_SWIRLS];   // 旋涡X坐标
	float swirlY[MAX_SWIRLS];   // 旋涡Y坐标
	float swirlOmega[MAX_SWIRLS];// 旋涡角速度
	float swirlRadius[MAX_SWIRLS];// 旋涡半径
	float swirlTime[MAX_SWIRLS]; // 旋涡剩余寿命
} Fluid;
// 场景配置结构体
typedef struct {
	float gravity;           // 重力加速度
	float dt;                // 时间步长
	int numIters;            // 压力求解迭代次数
	int frameNr;             // 帧计数
	float obstacleX;         // 障碍物X坐标
	float obstacleY;         // 障碍物Y坐标
	float obstacleRadius;    // 障碍物半径
	int burningObstacle;     // 是否启用燃烧环
	int burningFloor;        // 是否启用燃烧地板
	int paused;              // 是否暂停
	int showObstacle;        // 是否显示障碍物
	int showSwirls;          // 是否显示旋涡
	float swirlProbability;  // 旋涡生成概率
	float swirlMaxRadius;    // 旋涡最大半径
	Fluid *fluid;            // 流体模拟实例指针
} Scene;
extern Scene scene;  // 全局场景实例
extern int fire_init(void) ;
extern uint16_t getFireColor_RGB565_Q12(float val);
extern void fire_sim_update(void);
extern uint16_t getFireColor_RGB565_Q12_gpt_version(float val);
#endif