/* 官方底层管脚映射有问题：TIM0的默认引脚是PA9，但是映射不到PB23，因为PB23默认做板载复位按键*/
/* 官方底层管脚映射有问题：TIM3的默认引脚是PB22，可以映射到PA2，他那个文件的注释是错的*/
#include "PWM.h"

/*  RB_PIN_TMR3   -  TMR3:  PB22 ->  PA2
 *  RB_PIN_TMR2   -  TMR2:  PA11 ->  PB11
 *  RB_PIN_TMR1   -  TMR1:  PA10 ->  PB10
 *  RB_PIN_TMR0   -  TMR0:  PA9 ->  PB23 */

 // 默认PA12 - PWM4--->映射PA6
 // 默认PA13 - PWM5--->映射PA7
 // 默认PB0 - PWM6**************不可映射
 // 默认PB4 - PWM7--->映射PB1
 // 默认PB6 - PWM8--->映射PB2
 // 默认PB7 - PWM9--->映射PB3
 // 默认PB14 - PWM10
 //  GPIOB_ModeCfg(GPIO_Pin_23, GPIO_ModeOut_PP_5mA); // PB23 - PWM11 此脚复用为外部复位脚，需要关闭此功能才能开启PWM功能
 //低级PWM是PWM4-到PWM11
 //高级PWM是PWM4-到PWM9

//四个定时器的引脚pwm
PWM_Tim_HandleDef wapwmt0;
PWM_Tim_HandleDef wapwmt1;
PWM_Tim_HandleDef wapwmt2;
PWM_Tim_HandleDef wapwmt3;

//定义8个低级PWM
PWMX_LOW_HandleDef wapwmlow4;
PWMX_LOW_HandleDef wapwmlow5;
PWMX_LOW_HandleDef wapwmlow6;
PWMX_LOW_HandleDef wapwmlow7;
PWMX_LOW_HandleDef wapwmlow8;
PWMX_LOW_HandleDef wapwmlow9;
PWMX_LOW_HandleDef wapwmlow10;
PWMX_LOW_HandleDef wapwmlow11;

//定义6个高级PWM
PWMX_High_HandleDef wapwmhigh4;
PWMX_High_HandleDef wapwmhigh5;
PWMX_High_HandleDef wapwmhigh6;
PWMX_High_HandleDef wapwmhigh7;
PWMX_High_HandleDef wapwmhigh8;
PWMX_High_HandleDef wapwmhigh9;

//声明“厨子函数”
void WA_PWM_TIM_MspInit(PWM_Tim_HandleDef *pwmt);
void WA_PWM_LOW_MspInit(PWMX_LOW_HandleDef *pwmlow);
void WA_PWM_High_MspInit(PWMX_High_HandleDef *pwmhigh);


/*
    定时器引脚的PWM：
    pwmt：地址
    print:管脚映射使能
    function:有效电平
    times:有效输出重复次数
    period:周期（us）
    pwm:占空比
*/
void WA_PWM_TIM_Init(PWM_Tim_HandleDef *pwmt,uint8_t print,uint8_t function,uint8_t times,uint16_t period,uint16_t pwm)
{
    pwmt->print = print;
    pwmt->function = function;
    pwmt->period = period;
    pwmt->times = times;
    pwmt->pwm = pwm;

    //“厨子函数”
    WA_PWM_TIM_MspInit(pwmt);

}

//“厨子函数”
void WA_PWM_TIM_MspInit(PWM_Tim_HandleDef *pwmt)
{
    if(pwmt == &wapwmt0)
    {
        if(pwmt->print == Pin_Enable)
        {
            GPIOB_ModeCfg(GPIO_Pin_23, GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE, RB_PIN_TMR0);
        }
        else 
        {
            GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
        }
        TMR0_PWMInit(pwmt->function, pwmt->times);
        TMR0_PWMCycleCfg(g_1us * pwmt->period); // 周期pwmt->period (us)
        TMR0_PWMActDataWidth(g_1us *pwmt->pwm);  // 占空比 修改占空比必须暂时关闭定时器
        TMR0_PWMEnable();   
        TMR0_Enable();
    }

    if(pwmt == &wapwmt1)
    {
        if(pwmt->print == Pin_Enable)
        {
            GPIOB_ModeCfg(GPIO_Pin_10, GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE, RB_PIN_TMR1);
        }
        else 
        {
            GPIOA_ModeCfg(GPIO_Pin_10, GPIO_ModeOut_PP_5mA);
        }
        TMR1_PWMInit(pwmt->function, pwmt->times);
        TMR1_PWMCycleCfg(g_1us * pwmt->period); // 周期pwmt->period (us)
        TMR1_PWMActDataWidth(g_1us *pwmt->pwm);  // 占空比 修改占空比必须暂时关闭定时器
        TMR1_PWMEnable();   
        TMR1_Enable();
    }

    if(pwmt == &wapwmt2)
    {
        if(pwmt->print == Pin_Enable)
        {
            GPIOB_ModeCfg(GPIO_Pin_11, GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE, RB_PIN_TMR2);
        }
        else
        {
            GPIOA_ModeCfg(GPIO_Pin_11, GPIO_ModeOut_PP_5mA);
        }
        TMR2_PWMInit(pwmt->function, pwmt->times);
        TMR2_PWMCycleCfg(g_1us * pwmt->period); // 周期pwmt->period (us)
        TMR2_PWMActDataWidth(g_1us *pwmt->pwm);  // 占空比 修改占空比必须暂时关闭定时器
        TMR2_PWMEnable();   
        TMR2_Enable();
    }

    if(pwmt == &wapwmt3)
    {
        if(pwmt->print == Pin_Enable)
        {
            GPIOA_ModeCfg(GPIO_Pin_2, GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE, RB_PIN_TMR3);
        }
        else 
        {
           GPIOB_ModeCfg(GPIO_Pin_22, GPIO_ModeOut_PP_5mA);
        }
        TMR3_PWMInit(pwmt->function, pwmt->times);
        TMR3_PWMCycleCfg(g_1us * pwmt->period); // 周期pwmt->period (10us)
        TMR3_PWMActDataWidth(g_1us *pwmt->pwm);  // 占空比 修改占空比必须暂时关闭定时器
        TMR3_PWMEnable();   
        TMR3_Enable();
    }
}

/*
    低级PWM：
    pwm：地址
    print:管脚映射使能
    ***************注意！！！这里的管脚映射一旦打开，便是所有的PWM通道全部
    ***************都会映射，所以，我们初始化的LOW_PWM的这个参数要保持一致
    function:有效电平

    period:周期（us）******这个参数也要保持一致，因为这几个PWM公用一个时钟线
    **************周期的参数只能填
                                PWMX_Cycle_256 = 0, // 256 个PWMX周期
                                PWMX_Cycle_255,     // 255 个PWMX周期
                                PWMX_Cycle_128,     // 128 个PWMX周期
                                PWMX_Cycle_127,     // 127 个PWMX周期
                                PWMX_Cycle_64,      // 64 个PWMX周期
                                PWMX_Cycle_63,      // 63 个PWMX周期

    pwm:占空比
*/
void WA_PWM_LOW_Init(PWMX_LOW_HandleDef *pwmlow,uint8_t print,uint8_t function,uint8_t timfre,uint16_t period,uint16_t pwm)
{
    pwmlow->print = print;
    pwmlow->function = function;
    pwmlow->period = period;
    pwmlow->pwm = pwm;
    pwmlow->timfre = timfre;
    //厨子函数
    WA_PWM_LOW_MspInit(pwmlow);
}

//厨子函数
void WA_PWM_LOW_MspInit(PWMX_LOW_HandleDef *pwmlow)
{
    PWMX_CLKCfg(pwmlow->timfre);//0.06us                                   // cycle = 4/Fsys
    PWMX_CycleCfg(pwmlow->period);                     // 周期 = 64*cycle
    if(pwmlow == &wapwmlow4)
    {   
        //映射
        if(pwmlow->print == Pin_Enable)
        {
            GPIOA_ModeCfg(GPIO_Pin_6,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //默认
        {
            GPIOA_ModeCfg(GPIO_Pin_12,GPIO_ModeOut_PP_5mA);
        }
        PWMX_ACTOUT(CH_PWM4, pwmlow->pwm, pwmlow->function, ENABLE);
    }

    if(pwmlow == &wapwmlow5)
    {   
        //映射
        if(pwmlow->print == Pin_Enable)
        {
            GPIOA_ModeCfg(GPIO_Pin_7,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //默认
        {
            GPIOA_ModeCfg(GPIO_Pin_13,GPIO_ModeOut_PP_5mA);
        }
        PWMX_ACTOUT(CH_PWM5, pwmlow->pwm, pwmlow->function, ENABLE);
    }

    if(pwmlow == &wapwmlow6)
    {    
        GPIOB_ModeCfg(GPIO_Pin_0,GPIO_ModeOut_PP_5mA);
        PWMX_ACTOUT(CH_PWM6, pwmlow->pwm, pwmlow->function, ENABLE);
    }

    if(pwmlow == &wapwmlow7)
    {   
        //映射
        if(pwmlow->print == Pin_Enable)
        {
            GPIOB_ModeCfg(GPIO_Pin_1,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //默认
        {
            GPIOB_ModeCfg(GPIO_Pin_4,GPIO_ModeOut_PP_5mA);
        }
        PWMX_ACTOUT(CH_PWM7, pwmlow->pwm, pwmlow->function, ENABLE);
    }

    if(pwmlow == &wapwmlow8)
    {   
        //映射
        if(pwmlow->print == Pin_Enable)
        {
            GPIOB_ModeCfg(GPIO_Pin_2,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //默认
        {
            GPIOB_ModeCfg(GPIO_Pin_6,GPIO_ModeOut_PP_5mA);
        }
        PWMX_ACTOUT(CH_PWM8, pwmlow->pwm, pwmlow->function, ENABLE);
    }

    if(pwmlow == &wapwmlow9)
    {   
        //映射
        if(pwmlow->print == Pin_Enable)
        {
            GPIOB_ModeCfg(GPIO_Pin_3,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //默认
        {
            GPIOB_ModeCfg(GPIO_Pin_7,GPIO_ModeOut_PP_5mA);
        }
        PWMX_ACTOUT(CH_PWM9, pwmlow->pwm, pwmlow->function, ENABLE);
    }

    if(pwmlow == &wapwmlow10)
    {   
        
        GPIOB_ModeCfg(GPIO_Pin_14,GPIO_ModeOut_PP_5mA);
        PWMX_ACTOUT(CH_PWM10, pwmlow->pwm, pwmlow->function, ENABLE);
    }
    if(pwmlow == &wapwmlow11)
    {   
       
        GPIOB_ModeCfg(GPIO_Pin_23,GPIO_ModeOut_PP_5mA);
        PWMX_ACTOUT(CH_PWM11, pwmlow->pwm, pwmlow->function, ENABLE);
    }
}

/*
    高级PWM：
    pwm：地址
    print:管脚映射使能
    ***************注意！！！这里的管脚映射一旦打开，便是所有的PWM通道全部
    ***************都会映射，所以，我们初始化的LOW_PWM的这个参数要保持一致
    function:有效电平
    timfre:基准时钟 = timfre/系统时钟主频62400000  (单位s)
    period:周期（us******这个参数也要保持一致，因为这几个PWM公用一个时钟线
    pwm:占空比
*/
void WA_PWM_High_Init(PWMX_High_HandleDef *pwmhigh,uint8_t print,uint8_t function,uint8_t timfre,uint16_t period,uint16_t pwm)
{
    pwmhigh->print = print;
    pwmhigh->function = function;
    pwmhigh->period = period;
    pwmhigh->pwm = pwm;
    pwmhigh->timfre = timfre;
    //"厨子函数"
    WA_PWM_High_MspInit(pwmhigh);

}

//厨子函数
void WA_PWM_High_MspInit(PWMX_High_HandleDef *pwmhigh)
{
    PWMX_CLKCfg(pwmhigh->timfre);                                   // cycle = 4/Fsys
    PWMX_16bit_CycleCfg(pwmhigh->period - 1);                    
    if(pwmhigh == &wapwmhigh4)
    {   
        //映射
        if(pwmhigh->print == Pin_Enable)
        {
            GPIOA_ModeCfg(GPIO_Pin_6,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //默认
        {
            GPIOA_ModeCfg(GPIO_Pin_12,GPIO_ModeOut_PP_5mA);
        }
        PWMX_16bit_ACTOUT(CH_PWM4, pwmhigh->pwm, pwmhigh->function, ENABLE);
    }

    if(pwmhigh == &wapwmhigh5)
    {   
        //映射
        if(pwmhigh->print == Pin_Enable)
        {
            GPIOA_ModeCfg(GPIO_Pin_7,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //默认
        {
            GPIOA_ModeCfg(GPIO_Pin_13,GPIO_ModeOut_PP_5mA);
        }
        PWMX_16bit_ACTOUT(CH_PWM5, pwmhigh->pwm, pwmhigh->function, ENABLE);
    }

    if(pwmhigh == &wapwmhigh6)
    {    
        GPIOB_ModeCfg(GPIO_Pin_0,GPIO_ModeOut_PP_5mA);
        PWMX_16bit_ACTOUT(CH_PWM6, pwmhigh->pwm, pwmhigh->function, ENABLE);
    }

    if(pwmhigh == &wapwmhigh7)
    {   
        //映射
        if(pwmhigh->print == Pin_Enable)
        {
            GPIOB_ModeCfg(GPIO_Pin_1,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //默认
        {
            GPIOB_ModeCfg(GPIO_Pin_4,GPIO_ModeOut_PP_5mA);
        }
        PWMX_16bit_ACTOUT(CH_PWM7, pwmhigh->pwm, pwmhigh->function, ENABLE);
    }

    if(pwmhigh == &wapwmhigh8)
    {   
        //映射
        if(pwmhigh->print == Pin_Enable)
        {
            GPIOB_ModeCfg(GPIO_Pin_2,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //默认
        {
            GPIOB_ModeCfg(GPIO_Pin_6,GPIO_ModeOut_PP_5mA);
        }
        PWMX_16bit_ACTOUT(CH_PWM8, pwmhigh->pwm, pwmhigh->function, ENABLE);
    }

    if(pwmhigh == &wapwmhigh9)
    {   
        //映射
        if(pwmhigh->print == Pin_Enable)
        {
            GPIOB_ModeCfg(GPIO_Pin_3,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //默认
        {
            GPIOB_ModeCfg(GPIO_Pin_7,GPIO_ModeOut_PP_5mA);
        }
        PWMX_16bit_ACTOUT(CH_PWM9, pwmhigh->pwm, pwmhigh->function, ENABLE);
    }
}

/*
    实时更新PWM占空比函数
*/
void PWM_ALL_UpData(void *pwmall,uint16_t percent)
{

    //高级定时器4，5，6，7，8，9
    if(pwmall == &wapwmhigh4)
    {   
        R8_PWM_OUT_EN &= ~(CH_PWM4);
        PWMX_16bit_ACTOUT(CH_PWM4,percent, wapwmhigh4.function, ENABLE);
    }

    if(pwmall == &wapwmhigh5)
    {   
        R8_PWM_OUT_EN &= ~(CH_PWM5);
        PWMX_16bit_ACTOUT(CH_PWM5,percent, wapwmhigh5.function, ENABLE);
    }

    if(pwmall == &wapwmhigh6)
    {   
        R8_PWM_OUT_EN &= ~(CH_PWM6);
        PWMX_16bit_ACTOUT(CH_PWM6,percent, wapwmhigh6.function, ENABLE);
    }

    if(pwmall == &wapwmhigh7)
    {   
        R8_PWM_OUT_EN &= ~(CH_PWM7);
        PWMX_16bit_ACTOUT(CH_PWM7,percent, wapwmhigh7.function, ENABLE);
    }


    if(pwmall == &wapwmhigh8)
    {   
        R8_PWM_OUT_EN &= ~(CH_PWM8);
        PWMX_16bit_ACTOUT(CH_PWM8,percent, wapwmhigh8.function, ENABLE);
    }

    if(pwmall == &wapwmhigh9)
    {   
        R8_PWM_OUT_EN &= ~(CH_PWM9);
        PWMX_16bit_ACTOUT(CH_PWM9,percent, wapwmhigh9.function, ENABLE);
    }

    //低级定时器4，5，6，7，8，9，10，11
    if(pwmall == &wapwmlow4)
    {   
        R8_PWM_OUT_EN &= ~(CH_PWM4);
        PWMX_ACTOUT(CH_PWM4,percent, wapwmlow4.function, ENABLE);
    }

    if(pwmall == &wapwmlow5)
    {   
        R8_PWM_OUT_EN &= ~(CH_PWM5);
        PWMX_ACTOUT(CH_PWM5,percent, wapwmlow5.function, ENABLE);
    }

    if(pwmall == &wapwmlow6)
    {   
        R8_PWM_OUT_EN &= ~(CH_PWM6);
        PWMX_ACTOUT(CH_PWM6,percent, wapwmlow6.function, ENABLE);
    }

    if(pwmall == &wapwmlow7)
    {   
        R8_PWM_OUT_EN &= ~(CH_PWM7);
        PWMX_ACTOUT(CH_PWM7,percent, wapwmlow7.function, ENABLE);
    }

    if(pwmall == &wapwmlow8)
    {   
        R8_PWM_OUT_EN &= ~(CH_PWM8);
        PWMX_ACTOUT(CH_PWM8,percent, wapwmlow8.function, ENABLE);
    }

    if(pwmall == &wapwmlow9)
    {   
        R8_PWM_OUT_EN &= ~(CH_PWM9);
        PWMX_ACTOUT(CH_PWM9,percent, wapwmlow9.function, ENABLE);
    }

    if(pwmall == &wapwmlow10)
    {   
        R8_PWM_OUT_EN &= ~(CH_PWM10);
        PWMX_ACTOUT(CH_PWM10,percent, wapwmlow10.function, ENABLE);
    }

    if(pwmall == &wapwmlow11)
    {   
        R8_PWM_OUT_EN &= ~(CH_PWM11);
        PWMX_ACTOUT(CH_PWM11,percent, wapwmlow11.function, ENABLE);
    }

    //定时器PWM
    if(pwmall == &wapwmt0)
    {
        TMR0_Disable();
        TMR0_PWMActDataWidth(g_1us * percent);
        TMR0_Enable();
    }

    if(pwmall == &wapwmt1)
    {
        TMR1_Disable();
        TMR1_PWMActDataWidth(g_1us * percent);
        TMR1_Enable();
    }

    if(pwmall == &wapwmt2)
    {
        TMR2_Disable();
        TMR2_PWMActDataWidth(g_1us * percent);
        TMR2_Enable();
    }

    if(pwmall == &wapwmt3)
    {
        TMR3_Disable();
        TMR3_PWMActDataWidth(g_1us * percent);
        TMR3_Enable();
    }
}
