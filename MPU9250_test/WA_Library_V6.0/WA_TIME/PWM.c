/* �ٷ��ײ�ܽ�ӳ�������⣺TIM0��Ĭ��������PA9������ӳ�䲻��PB23����ΪPB23Ĭ�������ظ�λ����*/
/* �ٷ��ײ�ܽ�ӳ�������⣺TIM3��Ĭ��������PB22������ӳ�䵽PA2�����Ǹ��ļ���ע���Ǵ��*/
#include "PWM.h"

/*  RB_PIN_TMR3   -  TMR3:  PB22 ->  PA2
 *  RB_PIN_TMR2   -  TMR2:  PA11 ->  PB11
 *  RB_PIN_TMR1   -  TMR1:  PA10 ->  PB10
 *  RB_PIN_TMR0   -  TMR0:  PA9 ->  PB23 */

 // Ĭ��PA12 - PWM4--->ӳ��PA6
 // Ĭ��PA13 - PWM5--->ӳ��PA7
 // Ĭ��PB0 - PWM6**************����ӳ��
 // Ĭ��PB4 - PWM7--->ӳ��PB1
 // Ĭ��PB6 - PWM8--->ӳ��PB2
 // Ĭ��PB7 - PWM9--->ӳ��PB3
 // Ĭ��PB14 - PWM10
 //  GPIOB_ModeCfg(GPIO_Pin_23, GPIO_ModeOut_PP_5mA); // PB23 - PWM11 �˽Ÿ���Ϊ�ⲿ��λ�ţ���Ҫ�رմ˹��ܲ��ܿ���PWM����
 //�ͼ�PWM��PWM4-��PWM11
 //�߼�PWM��PWM4-��PWM9

//�ĸ���ʱ��������pwm
PWM_Tim_HandleDef wapwmt0;
PWM_Tim_HandleDef wapwmt1;
PWM_Tim_HandleDef wapwmt2;
PWM_Tim_HandleDef wapwmt3;

//����8���ͼ�PWM
PWMX_LOW_HandleDef wapwmlow4;
PWMX_LOW_HandleDef wapwmlow5;
PWMX_LOW_HandleDef wapwmlow6;
PWMX_LOW_HandleDef wapwmlow7;
PWMX_LOW_HandleDef wapwmlow8;
PWMX_LOW_HandleDef wapwmlow9;
PWMX_LOW_HandleDef wapwmlow10;
PWMX_LOW_HandleDef wapwmlow11;

//����6���߼�PWM
PWMX_High_HandleDef wapwmhigh4;
PWMX_High_HandleDef wapwmhigh5;
PWMX_High_HandleDef wapwmhigh6;
PWMX_High_HandleDef wapwmhigh7;
PWMX_High_HandleDef wapwmhigh8;
PWMX_High_HandleDef wapwmhigh9;

//���������Ӻ�����
void WA_PWM_TIM_MspInit(PWM_Tim_HandleDef *pwmt);
void WA_PWM_LOW_MspInit(PWMX_LOW_HandleDef *pwmlow);
void WA_PWM_High_MspInit(PWMX_High_HandleDef *pwmhigh);


/*
    ��ʱ�����ŵ�PWM��
    pwmt����ַ
    print:�ܽ�ӳ��ʹ��
    function:��Ч��ƽ
    times:��Ч����ظ�����
    period:���ڣ�us��
    pwm:ռ�ձ�
*/
void WA_PWM_TIM_Init(PWM_Tim_HandleDef *pwmt,uint8_t print,uint8_t function,uint8_t times,uint16_t period,uint16_t pwm)
{
    pwmt->print = print;
    pwmt->function = function;
    pwmt->period = period;
    pwmt->times = times;
    pwmt->pwm = pwm;

    //�����Ӻ�����
    WA_PWM_TIM_MspInit(pwmt);

}

//�����Ӻ�����
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
        TMR0_PWMCycleCfg(g_1us * pwmt->period); // ����pwmt->period (us)
        TMR0_PWMActDataWidth(g_1us *pwmt->pwm);  // ռ�ձ� �޸�ռ�ձȱ�����ʱ�رն�ʱ��
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
        TMR1_PWMCycleCfg(g_1us * pwmt->period); // ����pwmt->period (us)
        TMR1_PWMActDataWidth(g_1us *pwmt->pwm);  // ռ�ձ� �޸�ռ�ձȱ�����ʱ�رն�ʱ��
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
        TMR2_PWMCycleCfg(g_1us * pwmt->period); // ����pwmt->period (us)
        TMR2_PWMActDataWidth(g_1us *pwmt->pwm);  // ռ�ձ� �޸�ռ�ձȱ�����ʱ�رն�ʱ��
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
        TMR3_PWMCycleCfg(g_1us * pwmt->period); // ����pwmt->period (10us)
        TMR3_PWMActDataWidth(g_1us *pwmt->pwm);  // ռ�ձ� �޸�ռ�ձȱ�����ʱ�رն�ʱ��
        TMR3_PWMEnable();   
        TMR3_Enable();
    }
}

/*
    �ͼ�PWM��
    pwm����ַ
    print:�ܽ�ӳ��ʹ��
    ***************ע�⣡��������Ĺܽ�ӳ��һ���򿪣��������е�PWMͨ��ȫ��
    ***************����ӳ�䣬���ԣ����ǳ�ʼ����LOW_PWM���������Ҫ����һ��
    function:��Ч��ƽ

    period:���ڣ�us��******�������ҲҪ����һ�£���Ϊ�⼸��PWM����һ��ʱ����
    **************���ڵĲ���ֻ����
                                PWMX_Cycle_256 = 0, // 256 ��PWMX����
                                PWMX_Cycle_255,     // 255 ��PWMX����
                                PWMX_Cycle_128,     // 128 ��PWMX����
                                PWMX_Cycle_127,     // 127 ��PWMX����
                                PWMX_Cycle_64,      // 64 ��PWMX����
                                PWMX_Cycle_63,      // 63 ��PWMX����

    pwm:ռ�ձ�
*/
void WA_PWM_LOW_Init(PWMX_LOW_HandleDef *pwmlow,uint8_t print,uint8_t function,uint8_t timfre,uint16_t period,uint16_t pwm)
{
    pwmlow->print = print;
    pwmlow->function = function;
    pwmlow->period = period;
    pwmlow->pwm = pwm;
    pwmlow->timfre = timfre;
    //���Ӻ���
    WA_PWM_LOW_MspInit(pwmlow);
}

//���Ӻ���
void WA_PWM_LOW_MspInit(PWMX_LOW_HandleDef *pwmlow)
{
    PWMX_CLKCfg(pwmlow->timfre);//0.06us                                   // cycle = 4/Fsys
    PWMX_CycleCfg(pwmlow->period);                     // ���� = 64*cycle
    if(pwmlow == &wapwmlow4)
    {   
        //ӳ��
        if(pwmlow->print == Pin_Enable)
        {
            GPIOA_ModeCfg(GPIO_Pin_6,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //Ĭ��
        {
            GPIOA_ModeCfg(GPIO_Pin_12,GPIO_ModeOut_PP_5mA);
        }
        PWMX_ACTOUT(CH_PWM4, pwmlow->pwm, pwmlow->function, ENABLE);
    }

    if(pwmlow == &wapwmlow5)
    {   
        //ӳ��
        if(pwmlow->print == Pin_Enable)
        {
            GPIOA_ModeCfg(GPIO_Pin_7,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //Ĭ��
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
        //ӳ��
        if(pwmlow->print == Pin_Enable)
        {
            GPIOB_ModeCfg(GPIO_Pin_1,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //Ĭ��
        {
            GPIOB_ModeCfg(GPIO_Pin_4,GPIO_ModeOut_PP_5mA);
        }
        PWMX_ACTOUT(CH_PWM7, pwmlow->pwm, pwmlow->function, ENABLE);
    }

    if(pwmlow == &wapwmlow8)
    {   
        //ӳ��
        if(pwmlow->print == Pin_Enable)
        {
            GPIOB_ModeCfg(GPIO_Pin_2,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //Ĭ��
        {
            GPIOB_ModeCfg(GPIO_Pin_6,GPIO_ModeOut_PP_5mA);
        }
        PWMX_ACTOUT(CH_PWM8, pwmlow->pwm, pwmlow->function, ENABLE);
    }

    if(pwmlow == &wapwmlow9)
    {   
        //ӳ��
        if(pwmlow->print == Pin_Enable)
        {
            GPIOB_ModeCfg(GPIO_Pin_3,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //Ĭ��
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
    �߼�PWM��
    pwm����ַ
    print:�ܽ�ӳ��ʹ��
    ***************ע�⣡��������Ĺܽ�ӳ��һ���򿪣��������е�PWMͨ��ȫ��
    ***************����ӳ�䣬���ԣ����ǳ�ʼ����LOW_PWM���������Ҫ����һ��
    function:��Ч��ƽ
    timfre:��׼ʱ�� = timfre/ϵͳʱ����Ƶ62400000  (��λs)
    period:���ڣ�us******�������ҲҪ����һ�£���Ϊ�⼸��PWM����һ��ʱ����
    pwm:ռ�ձ�
*/
void WA_PWM_High_Init(PWMX_High_HandleDef *pwmhigh,uint8_t print,uint8_t function,uint8_t timfre,uint16_t period,uint16_t pwm)
{
    pwmhigh->print = print;
    pwmhigh->function = function;
    pwmhigh->period = period;
    pwmhigh->pwm = pwm;
    pwmhigh->timfre = timfre;
    //"���Ӻ���"
    WA_PWM_High_MspInit(pwmhigh);

}

//���Ӻ���
void WA_PWM_High_MspInit(PWMX_High_HandleDef *pwmhigh)
{
    PWMX_CLKCfg(pwmhigh->timfre);                                   // cycle = 4/Fsys
    PWMX_16bit_CycleCfg(pwmhigh->period - 1);                    
    if(pwmhigh == &wapwmhigh4)
    {   
        //ӳ��
        if(pwmhigh->print == Pin_Enable)
        {
            GPIOA_ModeCfg(GPIO_Pin_6,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //Ĭ��
        {
            GPIOA_ModeCfg(GPIO_Pin_12,GPIO_ModeOut_PP_5mA);
        }
        PWMX_16bit_ACTOUT(CH_PWM4, pwmhigh->pwm, pwmhigh->function, ENABLE);
    }

    if(pwmhigh == &wapwmhigh5)
    {   
        //ӳ��
        if(pwmhigh->print == Pin_Enable)
        {
            GPIOA_ModeCfg(GPIO_Pin_7,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //Ĭ��
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
        //ӳ��
        if(pwmhigh->print == Pin_Enable)
        {
            GPIOB_ModeCfg(GPIO_Pin_1,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //Ĭ��
        {
            GPIOB_ModeCfg(GPIO_Pin_4,GPIO_ModeOut_PP_5mA);
        }
        PWMX_16bit_ACTOUT(CH_PWM7, pwmhigh->pwm, pwmhigh->function, ENABLE);
    }

    if(pwmhigh == &wapwmhigh8)
    {   
        //ӳ��
        if(pwmhigh->print == Pin_Enable)
        {
            GPIOB_ModeCfg(GPIO_Pin_2,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //Ĭ��
        {
            GPIOB_ModeCfg(GPIO_Pin_6,GPIO_ModeOut_PP_5mA);
        }
        PWMX_16bit_ACTOUT(CH_PWM8, pwmhigh->pwm, pwmhigh->function, ENABLE);
    }

    if(pwmhigh == &wapwmhigh9)
    {   
        //ӳ��
        if(pwmhigh->print == Pin_Enable)
        {
            GPIOB_ModeCfg(GPIO_Pin_3,GPIO_ModeOut_PP_5mA);
            GPIOPinRemap(ENABLE,RB_PIN_PWMX);
        }
        else //Ĭ��
        {
            GPIOB_ModeCfg(GPIO_Pin_7,GPIO_ModeOut_PP_5mA);
        }
        PWMX_16bit_ACTOUT(CH_PWM9, pwmhigh->pwm, pwmhigh->function, ENABLE);
    }
}

/*
    ʵʱ����PWMռ�ձȺ���
*/
void PWM_ALL_UpData(void *pwmall,uint16_t percent)
{

    //�߼���ʱ��4��5��6��7��8��9
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

    //�ͼ���ʱ��4��5��6��7��8��9��10��11
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

    //��ʱ��PWM
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
