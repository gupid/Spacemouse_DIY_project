#include "I2C.h"

I2C_HandleDef wai2c0;
I2C_HandleDef wai2c1;
I2C_HandleDef wai2c2;
I2C_HandleDef wai2c3;

/*
*********************************************************************************************************
*	函 数 名: IIC_GPIO_Config
*	功能说明: 配置IIC总线的GPIO，采用模拟IO的方式实现
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void WA_I2C_Init(I2C_HandleDef *wai2c,uint8_t sclgroup,uint32_t sclpin,uint8_t sdagroup,uint32_t sdapin,uint8_t delayus)
{
    wai2c->scl_group = sclgroup;
    wai2c->scl_pin = sclpin;
    wai2c->sda_group = sdagroup;
    wai2c->sda_pin = sdapin;
    wai2c->delayus = delayus;

    if(wai2c->scl_group == GPIOA)
    {
        GPIOA_ModeCfg(wai2c->scl_pin,GPIO_ModeIN_PU);
        GPIOA_ResetBits(wai2c->scl_pin);
    }
       
    if(wai2c->scl_group == GPIOB)
    {
        GPIOB_ModeCfg(wai2c->scl_pin,GPIO_ModeIN_PU);
        GPIOB_ResetBits(wai2c->scl_pin);
    }
        
    if(wai2c->sda_group == GPIOA)
    {
        GPIOA_ModeCfg(wai2c->sda_pin,GPIO_ModeIN_PU);
        GPIOA_ResetBits(wai2c->sda_pin);
    }
       
    if(wai2c->sda_group == GPIOB)
    {
        GPIOB_ModeCfg(wai2c->sda_pin,GPIO_ModeIN_PU);
        GPIOB_ResetBits(wai2c->sda_pin);
    }  
}


void IIC_SCL_1(I2C_HandleDef *wai2c)  /* SCL = 1 */
{
    if(wai2c->scl_group == GPIOA)
        R32_PA_DIR    &= ~wai2c->scl_pin;
    if(wai2c->scl_group == GPIOB)
        R32_PB_DIR    &= ~wai2c->scl_pin;
}	

void IIC_SCL_0(I2C_HandleDef *wai2c)  
{
    if(wai2c->scl_group == GPIOA)
        R32_PA_DIR    |=  wai2c->scl_pin;		/* SCL = 0 */
    if(wai2c->scl_group == GPIOB)
        R32_PB_DIR    |=  wai2c->scl_pin;		/* SCL = 0 */
}

void IIC_SDA_1(I2C_HandleDef *wai2c)  
{
    if(wai2c->sda_group == GPIOA)
        R32_PA_DIR    &= ~wai2c->sda_pin;	/* SDA = 1 */
    if(wai2c->sda_group == GPIOB)
        R32_PB_DIR    &= ~wai2c->sda_pin;	/* SDA = 1 */
}

void IIC_SDA_0(I2C_HandleDef *wai2c)  
{
    if(wai2c->sda_group == GPIOA)
        R32_PA_DIR    |=  wai2c->sda_pin;	/* SDA = 0 */
    if(wai2c->sda_group == GPIOB)
        R32_PB_DIR    |=  wai2c->sda_pin;	/* SDA = 0 */
}

uint32_t IIC_SDA_READ(I2C_HandleDef *wai2c)
{
    if(wai2c->sda_group == GPIOA)
    {
        return GPIOA_ReadPortPin(wai2c->sda_pin);
    }
    else if(wai2c->sda_group == GPIOB)
    {
        return GPIOB_ReadPortPin(wai2c->sda_pin);
    }
    return 1; // 例如，默认返回高电平（总线空闲）
}
/*
*********************************************************************************************************
*	函 数 名: IIC_Delay
*	功能说明: IIC总线位延迟，最快400KHz
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void IIC_Delay(I2C_HandleDef *wai2c)
{
    DelayUs(wai2c->delayus);
}

/*
*********************************************************************************************************
*	函 数 名: IIC_Start
*	功能说明: CPU发起IIC总线启动信号
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void IIC_Start(I2C_HandleDef *wai2c)
{
    /* 当SCL高电平时，SDA出现一个下跳沿表示IIC总线启动信号 */
    IIC_SDA_1(wai2c);
    IIC_SCL_1(wai2c);
    IIC_Delay(wai2c);
    IIC_SDA_0(wai2c);
    IIC_Delay(wai2c);
    IIC_SCL_0(wai2c);
    IIC_Delay(wai2c);
}

/*
*********************************************************************************************************
*	函 数 名: IIC_Start
*	功能说明: CPU发起IIC总线停止信号
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void IIC_Stop(I2C_HandleDef *wai2c)
{
    /* 当SCL高电平时，SDA出现一个上跳沿表示IIC总线停止信号 */
    IIC_SDA_0(wai2c);
    IIC_SCL_1(wai2c);
    IIC_Delay(wai2c);
    IIC_SDA_1(wai2c);
}

/*
*********************************************************************************************************
*	函 数 名: IIC_SendByte
*	功能说明: CPU向IIC总线设备发送8bit数据
*	形    参：_ucByte ： 等待发送的字节
*	返 回 值: 无
*********************************************************************************************************
*/
void IIC_Send_Byte(I2C_HandleDef *wai2c,uint8_t _ucByte)
{
    uint8_t i;

    /* 先发送字节的高位bit7 */
    for (i = 0; i < 8; i++)
    {
        if (_ucByte & 0x80)
        {
            IIC_SDA_1(wai2c);
        }
        else
        {
            IIC_SDA_0(wai2c);
        }
        IIC_Delay(wai2c);
        IIC_SCL_1(wai2c);
        IIC_Delay(wai2c);
        IIC_SCL_0(wai2c);
        if (i == 7)
        {
            IIC_SDA_1(wai2c); // 释放总线
        }
        _ucByte <<= 1;	/* 左移一个bit */
        IIC_Delay(wai2c);
    }
}

/*
*********************************************************************************************************
*	函 数 名: IIC_ReadByte
*	功能说明: CPU从IIC总线设备读取8bit数据
*	形    参：无
*	返 回 值: 读到的数据
*********************************************************************************************************
*/
uint8_t IIC_Read_Byte(I2C_HandleDef *wai2c,uint8_t ack)
{
    uint8_t i;
    uint8_t value;

    /* 读到第1个bit为数据的bit7 */
    value = 0;
    for (i = 0; i < 8; i++)
    {
        value <<= 1;
        IIC_SCL_1(wai2c);
        IIC_Delay(wai2c);
        if (IIC_SDA_READ(wai2c))
        {
            value++;
        }
        IIC_SCL_0(wai2c);
        IIC_Delay(wai2c);
    }
    if(ack==0)
        IIC_NAck(wai2c);
    else
        IIC_Ack(wai2c);
    return value;
}

/*
*********************************************************************************************************
*	函 数 名: IIC_WaitAck
*	功能说明: CPU产生一个时钟，并读取器件的ACK应答信号
*	形    参：无
*	返 回 值: 返回0表示正确应答，1表示无器件响应
*********************************************************************************************************
*/
uint8_t IIC_Wait_Ack(I2C_HandleDef *wai2c)
{
    uint8_t re;

    IIC_SDA_1(wai2c);	/* CPU释放SDA总线 */
    IIC_Delay(wai2c);
    IIC_SCL_1(wai2c);	/* CPU驱动SCL = 1, 此时器件会返回ACK应答 */
    IIC_Delay(wai2c);
    if (IIC_SDA_READ(wai2c))	/* CPU读取SDA口线状态 */
    {
        re = 1;
    }
    else
    {
        re = 0;
    }
    IIC_SCL_0(wai2c);
    IIC_Delay(wai2c);
    return re;
}

/*
*********************************************************************************************************
*	函 数 名: IIC_Ack
*	功能说明: CPU产生一个ACK信号
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void IIC_Ack(I2C_HandleDef *wai2c)
{
    IIC_SDA_0(wai2c);	/* CPU驱动SDA = 0 */
    IIC_Delay(wai2c);
    IIC_SCL_1(wai2c);	/* CPU产生1个时钟 */
    IIC_Delay(wai2c);
    IIC_SCL_0(wai2c);
    IIC_Delay(wai2c);
    IIC_SDA_1(wai2c);	/* CPU释放SDA总线 */
}

/*
*********************************************************************************************************
*	函 数 名: IIC_NAck
*	功能说明: CPU产生1个NACK信号
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void IIC_NAck(I2C_HandleDef *wai2c)
{
    IIC_SDA_1(wai2c);	/* CPU驱动SDA = 1 */
    IIC_Delay(wai2c);
    IIC_SCL_1(wai2c);	/* CPU产生1个时钟 */
    IIC_Delay(wai2c);
    IIC_SCL_0(wai2c);
    IIC_Delay(wai2c);
}

//IIC连续写
//addr:器件地址
//reg:寄存器地址
//len:写入长度
//buf:数据区
//返回值:0,正常
//    其他,错误代码
uint8_t WA_Write_Len(I2C_HandleDef *wai2c,uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf )
{
    uint8_t i;
    uint16_t t = 0;
    uint8_t dev_write;
    
    dev_write = ((addr<<1)|0);

    MPU_IIC_Start(wai2c);
    MPU_IIC_Send_Byte(wai2c,dev_write);//假设是7bit
    while(MPU_IIC_Wait_Ack(wai2c))
    {
        t++;
        if(t ==30)
        {
            PRINT("reg address no ack \n");
            return 1;
        }
        // if(t == 30)
        // {
        //     MPU_IIC_Stop(wai2c);	//产生一个停止条件
        //     dev_write = addr & 0xFE;
            
        //     MPU_IIC_Start(wai2c);
        //     MPU_IIC_Send_Byte(wai2c,dev_write);//假设是7bit
        //     MPU_IIC_Wait_Ack(wai2c);
        //     break;
        // }   
    }
    
    MPU_IIC_Send_Byte(wai2c,reg);	//写寄存器地址
    if(MPU_IIC_Wait_Ack(wai2c))
    {
        PRINT("reg address no ack \n");
        return 1;
    }		//等待应答

    for(i=0; i<len; i++)
    {
        MPU_IIC_Send_Byte(wai2c,buf[i]);	//发送数据
        if(MPU_IIC_Wait_Ack(wai2c))		//等待ACK
        {
            MPU_IIC_Stop(wai2c);
            return 1;
        }
    }
    MPU_IIC_Stop(wai2c);
    return 0;
}
//IIC连续读
//addr:器件地址
//reg:要读取的寄存器地址
//len:要读取的长度
//buf:读取到的数据存储区
//返回值:0,正常
//    其他,错误代码
uint8_t WA_Read_Len(I2C_HandleDef *wai2c,uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf )
{
    uint8_t t;
    uint8_t dev_write,dev_read;
   
    dev_write = ((addr<<1)|0);
    dev_read = ((addr<<1)|1);
    MPU_IIC_Start(wai2c);
    MPU_IIC_Send_Byte(wai2c,dev_write);//假设是7bit
    while(MPU_IIC_Wait_Ack(wai2c))
    {
        t++;
        if(t == 30)
        {
            MPU_IIC_Stop(wai2c);	//产生一个停止条件
            // flag = 1;
            dev_write = addr & 0xFE;
            dev_read  = (addr | 0x01);
            MPU_IIC_Start(wai2c);
            MPU_IIC_Send_Byte(wai2c,dev_write);//假设是7bit
            MPU_IIC_Wait_Ack(wai2c);
            break;
        }   
    }
    MPU_IIC_Send_Byte(wai2c,reg);	//写寄存器地址
    MPU_IIC_Wait_Ack(wai2c);		//等待应答
    MPU_IIC_Start(wai2c);
    MPU_IIC_Send_Byte(wai2c,dev_read);//发送器件地址+读命令
    MPU_IIC_Wait_Ack(wai2c);		//等待应答
    while(len)
    {
        if(len==1)*buf=MPU_IIC_Read_Byte(wai2c,0);//读数据,发送nACK
        else *buf=MPU_IIC_Read_Byte(wai2c,1);		//读数据,发送ACK
        len--;
        buf++;
    }
    MPU_IIC_Stop(wai2c);	//产生一个停止条件
    return 0;
}

// tlv493d.c 或包含 TLV493D_Read_Bytes 定义的文件中

uint8_t TLV493D_Read_Bytes(
    I2C_HandleDef *wai2c,
    uint8_t slave_7bit_addr,
    uint8_t initial_bytes_to_skip,
    uint8_t num_bytes_to_read,
    uint8_t *data_buffer
) {
    uint8_t retry_counter = 0;
    const uint8_t MAX_READ_ADDR_RETRIES = 5; // 您可以调整这个重试次数，例如 5 次
                                            // MAX_ACK_CHECK_RETRIES (30) 可能太高，且用途不同
    uint8_t i2c_device_addr_read;
    uint8_t i;

    // 1. 基本参数校验
    if (wai2c == NULL || data_buffer == NULL || num_bytes_to_read == 0) {
        return WA_I2C_ERR_INVALID_PARAMS;
    }

    // 2. 准备包含读位的设备地址
    i2c_device_addr_read = (slave_7bit_addr << 1) | 0x01; // 地址 + 读位(1)

    // --- 直接开始读取流程 ---

    // 3. 发送START信号、从机地址+读位，并处理ACK（带有效重试）
    while(1) { // 无限循环，直到成功或达到最大重试次数
        MPU_IIC_Start(wai2c);
        MPU_IIC_Send_Byte(wai2c, i2c_device_addr_read);

        if (MPU_IIC_Wait_Ack(wai2c) == 0) { // 如果收到ACK (返回0)
            break; // 成功，跳出重试循环
        }

        // 如果NACK
        MPU_IIC_Stop(wai2c); // 在重试前发送STOP

        retry_counter++;
        if (retry_counter >= MAX_READ_ADDR_RETRIES) {
            // MPU_IIC_Stop(wai2c); // 已在上面发送
            return WA_I2C_ERR_ADDR1_NACK; // 达到最大重试次数后仍然NACK
        }
        DelayUs(10); // 在两次完整的（Start-Addr-Stop）尝试之间稍作延时 (10ms可能偏长，可调整为 DelayUs)
                     // 或者使用您I2C句柄中的 wai2c->delayus * N 来进行微秒级延时
                     // 例如 DelayUs(wai2c->delayus * 100);
    }

    // ---- 地址被成功ACK后，继续后续操作 ----

    // 5. 如果需要，读取并丢弃初始的几个字节
    for (i = 0; i < initial_bytes_to_skip; i++) {
        // 注意: MPU_IIC_Read_Byte 本身是否包含SCL时钟产生和SDA读取？
        // 如果 MPU_IIC_Read_Byte 出错（例如它内部的ACK/NACK处理），这里也可能出问题
        // 但当前错误是地址NACK，所以先解决地址部分
        (void)MPU_IIC_Read_Byte(wai2c, 1); // 读取一个字节并发送ACK（因为后面还有数据）
                                          // 这里的ACK是主机对从机发送数据的ACK
    }

    // 6. 循环读取指定长度的有效数据到data_buffer
    for (i = 0; i < num_bytes_to_read; i++) {
        if (i == (num_bytes_to_read - 1)) {
            data_buffer[i] = MPU_IIC_Read_Byte(wai2c, 0); // 最后一个字节，主机NACK
        } else {
            data_buffer[i] = MPU_IIC_Read_Byte(wai2c, 1); // 非最后一个字节，主机ACK
        }
        // 理想情况下，每次MPU_IIC_Read_Byte后，如果它有错误返回机制，也应该检查
    }

    // 7. 发送STOP信号，结束本次I2C通信
    MPU_IIC_Stop(wai2c);

    return WA_I2C_OK; // 所有操作成功
}