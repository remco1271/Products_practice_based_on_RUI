#include "rui.h"

extern RUI_I2C_ST rui_i2c_mpu9250;


/**         MPU9250           */

#define MPU9250_ADDR                    0x68 // MPU9250 slave 7 bits address

#define MPU9250_SMPLRT_DIV              0X19 
#define MPU9250_CONFIG                  0X1A 
#define MPU9250_ACCEL_CONFIG            0X1C 
#define MPU9250_ACCEL_CONFIG2           0X1D 

#define MPU9250_GYRO_XOUT_H             0X43 
#define MPU9250_GYRO_XOUT_L             0X44
#define MPU9250_GYRO_YOUT_H             0X45
#define MPU9250_GYRO_YOUT_L             0X46
#define MPU9250_GYRO_ZOUT_H             0X47
#define MPU9250_GYRO_ZOUT_L             0X48

#define MPU9285_SIGNAL_PATH_RESET       0x68
#define MPU9250_PWR_MGMT_1              0X6B 
#define MPU9250_WHO_AM_I                0X75 


typedef enum{
    MPU9250_SUCCESS=0,
    MPU9250_ERROR
} mpu9250_ret_code_enum;

typedef struct{
    int16_t x;
    int16_t y;
    int16_t z;
}mpu9250_gyro_st;


uint32_t mpu9250_read(uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    // Porting i2c read code
    rui_i2c_rw(&rui_i2c_mpu9250, RUI_IF_READ, MPU9250_ADDR, reg_addr, data, len);
    return MPU9250_SUCCESS;
}

uint32_t mpu9250_write(uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    // Porting i2c write code
    rui_i2c_rw(&rui_i2c_mpu9250, RUI_IF_WRITE, MPU9250_ADDR, reg_addr, data, len);
    return MPU9250_SUCCESS;
}

uint8_t mpu9250_read_byte(uint8_t reg_addr)
{
    uint8_t reg_val;

    if (MPU9250_SUCCESS == mpu9250_read(reg_addr, &reg_val, 1))
    {
        return reg_val;
    }
    else
    {
        RUI_LOG_PRINTF("MPU9250 Read err!\r\n");
        return 0;
    }
}

void mpu9250_write_byte(uint8_t reg_addr, uint8_t reg_val)
{
    if (MPU9250_SUCCESS != mpu9250_write(reg_addr, &reg_val, 1))
    {
        RUI_LOG_PRINTF("MPU9250 Read err!\r\n");
    }
}

uint32_t mpu9250_get_gyro(mpu9250_gyro_st *gyro_values)
{
    uint32_t ret_code;
    uint8_t buf[6];
    
    ret_code = mpu9250_read(MPU9250_GYRO_XOUT_H, buf, 6);
    if (ret_code == MPU9250_SUCCESS)
    {
        gyro_values->x = (int16_t)buf[0]<<8 | buf[1];
        gyro_values->y = (int16_t)buf[2]<<8 | buf[3];
        gyro_values->z = (int16_t)buf[4]<<8 | buf[5];
        return MPU9250_SUCCESS;
    }
    else
        return MPU9250_ERROR;
}

uint32_t mpu9250_init(void)
{
    uint32_t ret_code;
    uint8_t id,reg_value;

    // verify device id
    do{
        id = mpu9250_read_byte(MPU9250_WHO_AM_I);
        if (id != 0x71)
        {
            RUI_LOG_PRINTF("ID is 0x%02X.\r\n", id);
            RUI_LOG_PRINTF("mpu9250 init err...\r\n");
        }
    }while (id != 0x71);
    RUI_LOG_PRINTF("ID is %02X.", id);

    mpu9250_write_byte(MPU9250_PWR_MGMT_1, 0x00);   
    mpu9250_write_byte(MPU9285_SIGNAL_PATH_RESET, 0x07);
    mpu9250_write_byte(MPU9250_SMPLRT_DIV, 0x07);
    mpu9250_write_byte(MPU9250_CONFIG, 0x06);
    mpu9250_write_byte(MPU9250_ACCEL_CONFIG, 0x18);
    mpu9250_write_byte(MPU9250_ACCEL_CONFIG2, 0x00);

    return MPU9250_SUCCESS;
}
/************************************************/


RUI_TIMER_ST rui_timer;

uint8_t timer_flag=0;
void timer_callback(void)
{
    timer_flag = 1;
}

void timer_init()
{
    rui_timer.timer_mode = RUI_TIMER_MODE_REPEATED;
    rui_timer_init(&rui_timer, timer_callback);
    rui_timer_setvalue(&rui_timer, 500);
    rui_timer_start(&rui_timer);
}

RUI_I2C_ST rui_i2c_mpu9250;

void i2c_init(void)
{
    rui_i2c_mpu9250.PIN_SDA = 13;
    rui_i2c_mpu9250.PIN_SCL = 12;
    rui_i2c_mpu9250.FREQUENCY = RUI_I2C_FREQ_100K;
    rui_i2c_init(&rui_i2c_mpu9250);
    mpu9250_init();
}

void i2c_running(void)
{
    uint32_t ret_code;
    uint8_t buf[6];
    mpu9250_gyro_st gyro_values;

    if (!timer_flag)
        return ;
    timer_flag = 0;

    ret_code = mpu9250_get_gyro(&gyro_values);
    if (ret_code == MPU9250_SUCCESS)
    {
        RUI_LOG_PRINTF("MPU9250 Gx:%04d Gy:%04d Gz:%04d", gyro_values.x, gyro_values.y, gyro_values.z);
    }
    else
    {
        RUI_LOG_PRINTF("MPU9250 get gyro err...");
    }
}


void main(void)
{
    //system init 
    rui_init();

    //you can add your init code here, like timer, uart, spi...
    timer_init();
    i2c_init();


    while(1)
    {
        //do your work here, then call rui_device_sleep(1) to sleep
        i2c_running();

        //here run system work and do not modify
        rui_running();
    }
}
