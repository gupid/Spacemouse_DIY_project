#include "I2C.h"

I2C_HandleDef wai2c0;
I2C_HandleDef wai2c1;
I2C_HandleDef wai2c2;
I2C_HandleDef wai2c3;

/*
*********************************************************************************************************
*	�� �� ��: IIC_GPIO_Config
*	����˵��: ����IIC���ߵ�GPIO������ģ��IO�ķ�ʽʵ��
*	��    �Σ���
*	�� �� ֵ: ��
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
    return 1; // ���磬Ĭ�Ϸ��ظߵ�ƽ�����߿��У�
}
/*
*********************************************************************************************************
*	�� �� ��: IIC_Delay
*	����˵��: IIC����λ�ӳ٣����400KHz
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void IIC_Delay(I2C_HandleDef *wai2c)
{
    DelayUs(wai2c->delayus);
}

/*
*********************************************************************************************************
*	�� �� ��: IIC_Start
*	����˵��: CPU����IIC���������ź�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void IIC_Start(I2C_HandleDef *wai2c)
{
    /* ��SCL�ߵ�ƽʱ��SDA����һ�������ر�ʾIIC���������ź� */
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
*	�� �� ��: IIC_Start
*	����˵��: CPU����IIC����ֹͣ�ź�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void IIC_Stop(I2C_HandleDef *wai2c)
{
    /* ��SCL�ߵ�ƽʱ��SDA����һ�������ر�ʾIIC����ֹͣ�ź� */
    IIC_SDA_0(wai2c);
    IIC_SCL_1(wai2c);
    IIC_Delay(wai2c);
    IIC_SDA_1(wai2c);
}

/*
*********************************************************************************************************
*	�� �� ��: IIC_SendByte
*	����˵��: CPU��IIC�����豸����8bit����
*	��    �Σ�_ucByte �� �ȴ����͵��ֽ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void IIC_Send_Byte(I2C_HandleDef *wai2c,uint8_t _ucByte)
{
    uint8_t i;

    /* �ȷ����ֽڵĸ�λbit7 */
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
            IIC_SDA_1(wai2c); // �ͷ�����
        }
        _ucByte <<= 1;	/* ����һ��bit */
        IIC_Delay(wai2c);
    }
}

/*
*********************************************************************************************************
*	�� �� ��: IIC_ReadByte
*	����˵��: CPU��IIC�����豸��ȡ8bit����
*	��    �Σ���
*	�� �� ֵ: ����������
*********************************************************************************************************
*/
uint8_t IIC_Read_Byte(I2C_HandleDef *wai2c,uint8_t ack)
{
    uint8_t i;
    uint8_t value;

    /* ������1��bitΪ���ݵ�bit7 */
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
*	�� �� ��: IIC_WaitAck
*	����˵��: CPU����һ��ʱ�ӣ�����ȡ������ACKӦ���ź�
*	��    �Σ���
*	�� �� ֵ: ����0��ʾ��ȷӦ��1��ʾ��������Ӧ
*********************************************************************************************************
*/
uint8_t IIC_Wait_Ack(I2C_HandleDef *wai2c)
{
    uint8_t re;

    IIC_SDA_1(wai2c);	/* CPU�ͷ�SDA���� */
    IIC_Delay(wai2c);
    IIC_SCL_1(wai2c);	/* CPU����SCL = 1, ��ʱ�����᷵��ACKӦ�� */
    IIC_Delay(wai2c);
    if (IIC_SDA_READ(wai2c))	/* CPU��ȡSDA����״̬ */
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
*	�� �� ��: IIC_Ack
*	����˵��: CPU����һ��ACK�ź�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void IIC_Ack(I2C_HandleDef *wai2c)
{
    IIC_SDA_0(wai2c);	/* CPU����SDA = 0 */
    IIC_Delay(wai2c);
    IIC_SCL_1(wai2c);	/* CPU����1��ʱ�� */
    IIC_Delay(wai2c);
    IIC_SCL_0(wai2c);
    IIC_Delay(wai2c);
    IIC_SDA_1(wai2c);	/* CPU�ͷ�SDA���� */
}

/*
*********************************************************************************************************
*	�� �� ��: IIC_NAck
*	����˵��: CPU����1��NACK�ź�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void IIC_NAck(I2C_HandleDef *wai2c)
{
    IIC_SDA_1(wai2c);	/* CPU����SDA = 1 */
    IIC_Delay(wai2c);
    IIC_SCL_1(wai2c);	/* CPU����1��ʱ�� */
    IIC_Delay(wai2c);
    IIC_SCL_0(wai2c);
    IIC_Delay(wai2c);
}

//IIC����д
//addr:������ַ
//reg:�Ĵ�����ַ
//len:д�볤��
//buf:������
//����ֵ:0,����
//    ����,�������
uint8_t WA_Write_Len(I2C_HandleDef *wai2c,uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf )
{
    uint8_t i;
    uint16_t t = 0;
    uint8_t dev_write;
    
    dev_write = ((addr<<1)|0);

    MPU_IIC_Start(wai2c);
    MPU_IIC_Send_Byte(wai2c,dev_write);//������7bit
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
        //     MPU_IIC_Stop(wai2c);	//����һ��ֹͣ����
        //     dev_write = addr & 0xFE;
            
        //     MPU_IIC_Start(wai2c);
        //     MPU_IIC_Send_Byte(wai2c,dev_write);//������7bit
        //     MPU_IIC_Wait_Ack(wai2c);
        //     break;
        // }   
    }
    
    MPU_IIC_Send_Byte(wai2c,reg);	//д�Ĵ�����ַ
    if(MPU_IIC_Wait_Ack(wai2c))
    {
        PRINT("reg address no ack \n");
        return 1;
    }		//�ȴ�Ӧ��

    for(i=0; i<len; i++)
    {
        MPU_IIC_Send_Byte(wai2c,buf[i]);	//��������
        if(MPU_IIC_Wait_Ack(wai2c))		//�ȴ�ACK
        {
            MPU_IIC_Stop(wai2c);
            return 1;
        }
    }
    MPU_IIC_Stop(wai2c);
    return 0;
}
//IIC������
//addr:������ַ
//reg:Ҫ��ȡ�ļĴ�����ַ
//len:Ҫ��ȡ�ĳ���
//buf:��ȡ�������ݴ洢��
//����ֵ:0,����
//    ����,�������
uint8_t WA_Read_Len(I2C_HandleDef *wai2c,uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf )
{
    uint8_t t;
    uint8_t dev_write,dev_read;
   
    dev_write = ((addr<<1)|0);
    dev_read = ((addr<<1)|1);
    MPU_IIC_Start(wai2c);
    MPU_IIC_Send_Byte(wai2c,dev_write);//������7bit
    while(MPU_IIC_Wait_Ack(wai2c))
    {
        t++;
        if(t == 30)
        {
            MPU_IIC_Stop(wai2c);	//����һ��ֹͣ����
            // flag = 1;
            dev_write = addr & 0xFE;
            dev_read  = (addr | 0x01);
            MPU_IIC_Start(wai2c);
            MPU_IIC_Send_Byte(wai2c,dev_write);//������7bit
            MPU_IIC_Wait_Ack(wai2c);
            break;
        }   
    }
    MPU_IIC_Send_Byte(wai2c,reg);	//д�Ĵ�����ַ
    MPU_IIC_Wait_Ack(wai2c);		//�ȴ�Ӧ��
    MPU_IIC_Start(wai2c);
    MPU_IIC_Send_Byte(wai2c,dev_read);//����������ַ+������
    MPU_IIC_Wait_Ack(wai2c);		//�ȴ�Ӧ��
    while(len)
    {
        if(len==1)*buf=MPU_IIC_Read_Byte(wai2c,0);//������,����nACK
        else *buf=MPU_IIC_Read_Byte(wai2c,1);		//������,����ACK
        len--;
        buf++;
    }
    MPU_IIC_Stop(wai2c);	//����һ��ֹͣ����
    return 0;
}

// tlv493d.c ����� TLV493D_Read_Bytes ������ļ���

uint8_t TLV493D_Read_Bytes(
    I2C_HandleDef *wai2c,
    uint8_t slave_7bit_addr,
    uint8_t initial_bytes_to_skip,
    uint8_t num_bytes_to_read,
    uint8_t *data_buffer
) {
    uint8_t retry_counter = 0;
    const uint8_t MAX_READ_ADDR_RETRIES = 5; // �����Ե���������Դ��������� 5 ��
                                            // MAX_ACK_CHECK_RETRIES (30) ����̫�ߣ�����;��ͬ
    uint8_t i2c_device_addr_read;
    uint8_t i;

    // 1. ��������У��
    if (wai2c == NULL || data_buffer == NULL || num_bytes_to_read == 0) {
        return WA_I2C_ERR_INVALID_PARAMS;
    }

    // 2. ׼��������λ���豸��ַ
    i2c_device_addr_read = (slave_7bit_addr << 1) | 0x01; // ��ַ + ��λ(1)

    // --- ֱ�ӿ�ʼ��ȡ���� ---

    // 3. ����START�źš��ӻ���ַ+��λ��������ACK������Ч���ԣ�
    while(1) { // ����ѭ����ֱ���ɹ���ﵽ������Դ���
        MPU_IIC_Start(wai2c);
        MPU_IIC_Send_Byte(wai2c, i2c_device_addr_read);

        if (MPU_IIC_Wait_Ack(wai2c) == 0) { // ����յ�ACK (����0)
            break; // �ɹ�����������ѭ��
        }

        // ���NACK
        MPU_IIC_Stop(wai2c); // ������ǰ����STOP

        retry_counter++;
        if (retry_counter >= MAX_READ_ADDR_RETRIES) {
            // MPU_IIC_Stop(wai2c); // �������淢��
            return WA_I2C_ERR_ADDR1_NACK; // �ﵽ������Դ�������ȻNACK
        }
        DelayUs(10); // �����������ģ�Start-Addr-Stop������֮��������ʱ (10ms����ƫ�����ɵ���Ϊ DelayUs)
                     // ����ʹ����I2C����е� wai2c->delayus * N ������΢�뼶��ʱ
                     // ���� DelayUs(wai2c->delayus * 100);
    }

    // ---- ��ַ���ɹ�ACK�󣬼����������� ----

    // 5. �����Ҫ����ȡ��������ʼ�ļ����ֽ�
    for (i = 0; i < initial_bytes_to_skip; i++) {
        // ע��: MPU_IIC_Read_Byte �����Ƿ����SCLʱ�Ӳ�����SDA��ȡ��
        // ��� MPU_IIC_Read_Byte �����������ڲ���ACK/NACK����������Ҳ���ܳ�����
        // ����ǰ�����ǵ�ַNACK�������Ƚ����ַ����
        (void)MPU_IIC_Read_Byte(wai2c, 1); // ��ȡһ���ֽڲ�����ACK����Ϊ���滹�����ݣ�
                                          // �����ACK�������Դӻ��������ݵ�ACK
    }

    // 6. ѭ����ȡָ�����ȵ���Ч���ݵ�data_buffer
    for (i = 0; i < num_bytes_to_read; i++) {
        if (i == (num_bytes_to_read - 1)) {
            data_buffer[i] = MPU_IIC_Read_Byte(wai2c, 0); // ���һ���ֽڣ�����NACK
        } else {
            data_buffer[i] = MPU_IIC_Read_Byte(wai2c, 1); // �����һ���ֽڣ�����ACK
        }
        // ��������£�ÿ��MPU_IIC_Read_Byte��������д��󷵻ػ��ƣ�ҲӦ�ü��
    }

    // 7. ����STOP�źţ���������I2Cͨ��
    MPU_IIC_Stop(wai2c);

    return WA_I2C_OK; // ���в����ɹ�
}