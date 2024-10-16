#include "USB_VCP_Task.hpp"
#include "EKF_fusion.hpp"
#include "LSM6DSO_Task.hpp"


USB_VCPTask Usb;

/*USB接收线程*/
void USB_VCP_RX_Task(void *argument)
{
    while (1)
    {
        // cprintf(&huart3, "usb rx ok\n");
        vTaskDelay(1000);
    }
    
}

/*USB发送线程*/
void USB_VCP_TX_Task(void *argument)
{
    while (1)
    {
        // Usb.imu_angle_send(EKF.Angle_fused);
        vTaskDelay(100);         //1khz
        // HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        EKF.plot_angle();
        // IMU.plot_data();
        // IMU.print_data();
        // EKF.print_angle();
    }
    
}

//USB接收回调函数
void CDC_Receive_CallBack(uint8_t* pbuf, uint32_t *Len)
{
    uint8_t instruc_buf[32];
    uint8_t instruc[32];        //提取到的指令
    uint8_t len = *Len;
    //帧头帧尾索引
    uint8_t head_index = 0;
    uint8_t tail_index = 0;
    //解析到的命令ld


    //把pbuf的前len个字节拷贝到instruc_buf
    memcpy(instruc_buf, pbuf, len);
    instruc_buf[len] = '\0';

    // 寻找帧头尾
    for(uint8_t i = 0; i < len; i++)
    {
        if(instruc_buf[i] == 0x5A) head_index = i;  //帧头
        if(instruc_buf[i] == 0x9C) tail_index = i;  //帧尾
    }
    //无效帧返回
    if(tail_index == 0) return;

    //提取指令
    memcpy(instruc, instruc_buf + head_index, tail_index - head_index + 1);

    Usb.instruct_decode(instruc, tail_index - head_index + 1);
}

void USB_VCPTask::instruct_decode(uint8_t *buf, uint8_t len)
{
    Frame_freq_type imu_set_frame;
    Frame_pwm_type pwm_set_frame;
    Frame_laser_type laser_set_frame;
    Frame_hardware_info_type hardware_info_frame;

    //根据ID解析指令
    switch (buf[1])
    {
    case 0x02: // CMD_IMU_SET_FREQ
        memcpy(&imu_set_frame, buf, len);
        //输出imu_set_frame全部信息
        cprintf(&huart3, "head:%x, length:%d, cmd:%x, freq:%d, end:%x\n"
                , imu_set_frame.head, imu_set_frame.length, imu_set_frame.cmd
                , imu_set_frame.freq, imu_set_frame.end);

        //todo:设置IMU发送频率
        break;

    case 0x03: // CMD_PWM_SET
        memcpy(&pwm_set_frame, buf, len);
        //输出pwm_set_frame全部信息
        cprintf(&huart3, "head:%x, length:%d, cmd:%x, pwm:%d, %d, %d, %d, end:%x\n"
                , pwm_set_frame.head, pwm_set_frame.length, pwm_set_frame.cmd
                , pwm_set_frame.pwm[0], pwm_set_frame.pwm[1], pwm_set_frame.pwm[2], pwm_set_frame.pwm[3], pwm_set_frame.end);

        break;

    case 0x04: // CMD_LASER_SET
        memcpy(&laser_set_frame, buf, len);
        //输出laser_set_frame全部信息
        cprintf(&huart3, "head:%x, length:%d, cmd:%x, laser:%d, end:%x\n"
                , laser_set_frame.head, laser_set_frame.length, laser_set_frame.cmd
                , laser_set_frame.laser, laser_set_frame.end);

        
        break;
        
    case 0x05: // CMD_HARDWARE_INFO
        //todo:响应查询
        break;
    default:
        break;
    }
}

void USB_VCPTask::imu_angle_send(float *angle)
{
    frame.yaw = angle[0];
    frame.pitch = angle[1];
    frame.roll = angle[2];
    frame.length = sizeof(frame);
    CDC_Transmit_FS((uint8_t *)&frame, sizeof(frame));
}