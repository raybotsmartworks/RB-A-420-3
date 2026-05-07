#include "rclib.h"

int Speed;
int Position, current;
int panduan;
int panduan1;
int yanshi;
int ID;
int ii, j;
int c[99];
float a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21;
float b1[99], b2[99], b3[99], b4[99], b5[99], b6[99], b7[99], b8[99];

bool can_driver_installed = false;
unsigned long wait_time = 500;

struct CanPendingRequest
{
    bool active;
    uint8_t motor_id;
    uint32_t expected_rx_id;
    float result;
    unsigned long start_ms;
};

static CanPendingRequest g_req = {false, 0, 0, -99999.0f, 0};

void can_begin_request(uint8_t motor_id, uint8_t cmd, uint32_t rx_id)
{
    if (!can_driver_installed)
        return;

    twai_message_t tx;
    tx.identifier = 0x140 + motor_id;
    tx.flags = 0;
    tx.data_length_code = 2;
    tx.data[0] = cmd;
    tx.data[1] = 0x00;
    twai_transmit(&tx, 0);

    delay(1);

    g_req.active = true;
    g_req.motor_id = motor_id;
    g_req.expected_rx_id = rx_id;
    g_req.result = -99999.0f;
    g_req.start_ms = millis();
}

bool can_poll_response(float *out_result)
{
    if (!g_req.active)
        return false;

    twai_message_t rx;
    if (twai_receive(&rx, 0) == ESP_OK)
    {
        Serial.print("CAN recv id:"); Serial.print(rx.identifier, HEX);
        Serial.print(" expected:"); Serial.print(g_req.expected_rx_id, HEX);
        Serial.print(" active:"); Serial.println(g_req.active);
        if (rx.identifier == g_req.expected_rx_id)
        {
            Serial.print("CAN match! data[4-7]:");
            Serial.print(rx.data[4], HEX); Serial.print(" ");
            Serial.print(rx.data[5], HEX); Serial.print(" ");
            Serial.print(rx.data[6], HEX); Serial.print(" ");
            Serial.print(rx.data[7], HEX); Serial.print(" raw:");
            int32_t raw = *(int32_t *)&rx.data[4];
            Serial.println(raw);
            g_req.result = raw * 0.01f;
            g_req.active = false;
            if (out_result)
                *out_result = g_req.result;
            return true;
        }
    }

    if (millis() - g_req.start_ms > wait_time)
    {
        Serial.println("CAN poll timeout");
        g_req.active = false;
        if (out_result)
            *out_result = -99999.0f;
        return true;
    }
    return false;
}

bool can_is_idle()
{
    return !g_req.active;
}

bool rc_init()
{
#ifdef LILYGO
    pinMode(16, OUTPUT);
    digitalWrite(16, HIGH);
    pinMode(23, OUTPUT);
    digitalWrite(23, LOW);
#endif

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, (gpio_num_t)RX_PIN, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS(); // Look in the api-reference for other speed sets.
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // Install TWAI driver
    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK)
    {
        return false;
    }

    // Start TWAI driver
    if (twai_start() != ESP_OK)
    {
        return false;
    }

    // Reconfigure alerts to detect frame receive, Bus-Off error and RX queue full states
    uint32_t alerts_to_enable = TWAI_ALERT_RX_DATA | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR | TWAI_ALERT_RX_QUEUE_FULL;
    if (twai_reconfigure_alerts(alerts_to_enable, NULL) != ESP_OK)
    {
        return false;
    }

    // TWAI driver is now successfully installed and started
    can_driver_installed = true;
    return true;
}

/*接收数据，通过判断接收的ID来确定是在使用哪个功能，进一步根据返回的数据，来判断显示出当前的状态
注：对于接收的数据需进行一个有效性的判断*/

void run(int id, int speed, int position) // 多圈位置控制
{
    // CAN_frame_t rx_frame;
    if (!can_driver_installed)
        return;

    twai_message_t txbuffer;
    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 8;

    txbuffer.data[0] = 0xA4;
    txbuffer.data[1] = 0x00;
    int16_t *p1 = (int16_t *)&txbuffer.data[2];
    int32_t *p2 = (int32_t *)&txbuffer.data[4];
    *p1 = (int16_t)speed;
    *p2 = (int32_t)100 * position;
    twai_transmit(&txbuffer, 0);
}

/*停止*/
void stop(int id)
{
    // CAN_frame_t rx_frame;
    if (!can_driver_installed)
        return;

    twai_message_t txbuffer;
    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 8;
    txbuffer.data[0] = 0x81;
    txbuffer.data[1] = 0x00;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    txbuffer.data[4] = 0x00;
    txbuffer.data[5] = 0x00;
    txbuffer.data[6] = 0x00;
    txbuffer.data[7] = 0x00;
    twai_transmit(&txbuffer, 0);
}

/*发送指令读取电机运行时的数据
 */
int read_rd(int id, int suoyin)
{
    if (!can_driver_installed)
        return -99999.0f;

    float res = -99999.0f;

    twai_message_t txbuffer;
    twai_message_t rxbuffer;

    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 1;
    txbuffer.data[0] = suoyin;
    twai_transmit(&txbuffer, 0);

    unsigned long startMillis = millis();
    while (millis() - startMillis < wait_time)
    {

        if (twai_receive(&rxbuffer, 1) == ESP_OK)
        {
            if (rxbuffer.identifier == 0x240 + id)
            {
                int32_t raw = *(int32_t *)&rxbuffer.data[4];
                res = raw * 0.01f;
                break;
            }
        }
    }
    return res;
}

void set_zero(int id) // 设置零点
{
    // CAN_frame_t rx_frame;
    if (!can_driver_installed)
        return;

    twai_message_t txbuffer;
    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 8;
    txbuffer.data[0] = 0x64;
    txbuffer.data[1] = 0x00;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    txbuffer.data[4] = 0x00;
    txbuffer.data[5] = 0x00;
    txbuffer.data[6] = 0x00;
    txbuffer.data[7] = 0x00;
    twai_transmit(&txbuffer, 0);
}
void re_set(int id) // 复位
{
    // CAN_frame_t rx_frame;
    if (!can_driver_installed)
        return;

    twai_message_t txbuffer;
    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 8;
    txbuffer.data[0] = 0x76;
    txbuffer.data[1] = 0x00;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    txbuffer.data[4] = 0x00;
    txbuffer.data[5] = 0x00;
    txbuffer.data[6] = 0x00;
    txbuffer.data[7] = 0x00;
    twai_transmit(&txbuffer, 0);
}
///////////////////////////////////////////////////////////////////V3

// 读取PID参数,0x01:电流环 KP， 0x02:电流环 KI, 0x03:电流环 KD, 0x04:速度环 KP, 0x05:速度环 KI, 0x06:速度环 KD, 0x07:位置环 KD, 0x08:位置环 KP, 0x09:位置环 KI
float read_PID(int id, int suoyin)
{
    if (!can_driver_installed)
        return -99999.0f;

    float res = -99999.0f;

    twai_message_t txbuffer;
    twai_message_t rxbuffer;

    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 1;
    txbuffer.data[0] = 0x30;
    txbuffer.data[1] = suoyin;
    twai_transmit(&txbuffer, 0);

    unsigned long startMillis = millis();
    while (millis() - startMillis < wait_time)
    {

        if (twai_receive(&rxbuffer, 1) == ESP_OK)
        {
            if (rxbuffer.identifier == 0x240 + id)
            {
                res = *(float *)&rxbuffer.data[4];
                break;
            }
        }
    }
    return res;
}
// 写入PID参数到RAM,0x01:电流环 KP， 0x02:电流环 KI, 0x03:电流环 KD, 0x04:速度环 KP, 0x05:速度环 KI, 0x06:速度环 KD, 0x07:位置环 KD, 0x08:位置环 KP, 0x09:位置环 KI
void write_PID_RAM(int id, int suoyin, float shuju)
{
    // CAN_frame_t rx_frame;
    if (!can_driver_installed)
        return;

    twai_message_t txbuffer;
    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 8;

    txbuffer.data[0] = 0x31;
    txbuffer.data[1] = suoyin;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    float *p1 = (float *)&txbuffer.data[4];
    *p1 = shuju;
    twai_transmit(&txbuffer, 0);
}
// 写入PID参数到ROM,0x01:电流环 KP， 0x02:电流环 KI, 0x03:电流环 KD, 0x04:速度环 KP, 0x05:速度环 KI, 0x06:速度环 KD, 0x07:位置环 KD, 0x08:位置环 KP, 0x09:位置环 KI
void write_PID_ROM(int id, int suoyin, float shuju)
{
    // CAN_frame_t rx_frame;
    if (!can_driver_installed)
        return;

    twai_message_t txbuffer;
    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 8;

    txbuffer.data[0] = 0x32;
    txbuffer.data[1] = suoyin;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    float *p1 = (float *)&txbuffer.data[4];
    *p1 = shuju;
    twai_transmit(&txbuffer, 0);
}
float read_a(int id, int suoyin) // 读取加速度, 0x00:位置规划加速度, 0x01:位置规划减速度, 0x02:速度规划加速度, 0x03:速度规划减速度
{
    if (!can_driver_installed)
        return -99999.0f;

    float res = -99999.0f;

    twai_message_t txbuffer;
    twai_message_t rxbuffer;

    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 1;
    txbuffer.data[0] = 0x42;
    txbuffer.data[1] = suoyin;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    txbuffer.data[4] = 0x00;
    txbuffer.data[5] = 0x00;
    txbuffer.data[6] = 0x00;
    txbuffer.data[7] = 0x00;
    twai_transmit(&txbuffer, 0);

    unsigned long startMillis = millis();
    while (millis() - startMillis < wait_time)
    {

        if (twai_receive(&rxbuffer, 1) == ESP_OK)
        {
            if (rxbuffer.identifier == 0x240 + id)
            {
                res = *(float *)&rxbuffer.data[4];
                break;
            }
        }
    }
    return res;
}
// 写入加减速度到ROM和RAM, 0x00:位置规划加速度, 0x01:位置规划减速度, 0x02:速度规划加速度, 0x03:速度规划减速度
void write_a(int id, int suoyin, int shuju)
{
    // CAN_frame_t rx_frame;
    if (!can_driver_installed)
        return;

    twai_message_t txbuffer;
    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 8;

    txbuffer.data[0] = 0x43;
    txbuffer.data[1] = suoyin;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    int *p1 = (int *)&txbuffer.data[4];
    *p1 = shuju;
    twai_transmit(&txbuffer, 0);
}

float read_Mc_encoder_p(int id) // 读取多圈编码器位置
{
    if (!can_driver_installed)
        return -99999.0f;

    float res = -99999.0f;

    twai_message_t txbuffer;
    twai_message_t rxbuffer;

    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 1;
    txbuffer.data[0] = 0x60;
    txbuffer.data[1] = 0x00;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    txbuffer.data[4] = 0x00;
    txbuffer.data[5] = 0x00;
    txbuffer.data[6] = 0x00;
    txbuffer.data[7] = 0x00;
    twai_transmit(&txbuffer, 0);

    unsigned long startMillis = millis();
    while (millis() - startMillis < wait_time)
    {

        if (twai_receive(&rxbuffer, 1) == ESP_OK)
        {
            if (rxbuffer.identifier == 0x240 + id)
            {
                res = *(float *)&rxbuffer.data[4];
                break;
            }
        }
    }
    return res;
}
float read_Mc_encoder_op(int id) // 读取多圈编码器原始位置
{
    if (!can_driver_installed)
        return -99999.0f;

    float res = -99999.0f;

    twai_message_t txbuffer;
    twai_message_t rxbuffer;

    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 1;
    txbuffer.data[0] = 0x61;
    txbuffer.data[1] = 0x00;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    txbuffer.data[4] = 0x00;
    txbuffer.data[5] = 0x00;
    txbuffer.data[6] = 0x00;
    txbuffer.data[7] = 0x00;
    twai_transmit(&txbuffer, 0);

    unsigned long startMillis = millis();
    while (millis() - startMillis < wait_time)
    {

        if (twai_receive(&rxbuffer, 1) == ESP_OK)
        {
            if (rxbuffer.identifier == 0x240 + id)
            {
                res = *(float *)&rxbuffer.data[4];
                break;
            }
        }
    }
    return res;
}
float read_Mc_encoder_zerobd(int id) // 读取多圈编码器零偏数据
{
    if (!can_driver_installed)
        return -99999.0f;

    float res = -99999.0f;

    twai_message_t txbuffer;
    twai_message_t rxbuffer;

    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 1;
    txbuffer.data[0] = 0x62;
    txbuffer.data[1] = 0x00;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    txbuffer.data[4] = 0x00;
    txbuffer.data[5] = 0x00;
    txbuffer.data[6] = 0x00;
    txbuffer.data[7] = 0x00;
    twai_transmit(&txbuffer, 0);

    unsigned long startMillis = millis();
    while (millis() - startMillis < wait_time)
    {

        if (twai_receive(&rxbuffer, 1) == ESP_OK)
        {
            if (rxbuffer.identifier == 0x240 + id)
            {
                res = *(float *)&rxbuffer.data[4];
                break;
            }
        }
    }
    return res;
}
void Write_Mc_encoder_ztoROM(int id, float shuju) // 写入编码器多圈值到ROM作为零点
{
    // CAN_frame_t rx_frame;
    if (!can_driver_installed)
        return;

    twai_message_t txbuffer;
    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 8;

    txbuffer.data[0] = 0x63;
    txbuffer.data[1] = 0x00;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    float *p1 = (float *)&txbuffer.data[4];
    *p1 = shuju;
    twai_transmit(&txbuffer, 0);
}
void Write_Mc_encoder_ztoRAM(int id) // 写入编码器当前多圈位置到ROM作为零点
{
    // CAN_frame_t rx_frame;
    if (!can_driver_installed)
        return;

    twai_message_t txbuffer;
    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 8;

    txbuffer.data[0] = 0x64;
    txbuffer.data[1] = 0x00;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    txbuffer.data[4] = 0x00;
    txbuffer.data[5] = 0x00;
    txbuffer.data[6] = 0x00;
    txbuffer.data[7] = 0x00;
    twai_transmit(&txbuffer, 0);
}
float read_Sc_encoder_p(int id) // 读取单圈编码器
{
    if (!can_driver_installed)
        return -99999.0f;

    float res = -99999.0f;

    twai_message_t txbuffer;
    twai_message_t rxbuffer;

    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 1;
    txbuffer.data[0] = 0x90;
    txbuffer.data[1] = 0x00;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    txbuffer.data[4] = 0x00;
    txbuffer.data[5] = 0x00;
    txbuffer.data[6] = 0x00;
    txbuffer.data[7] = 0x00;
    twai_transmit(&txbuffer, 0);

    unsigned long startMillis = millis();
    while (millis() - startMillis < wait_time)
    {

        if (twai_receive(&rxbuffer, 1) == ESP_OK)
        {
            if (rxbuffer.identifier == 0x240 + id)
            {
                res = *(float *)&rxbuffer.data[4];
                break;
            }
        }
    }
    return res;
}
float read_Mc_angle(int id) // 读取多圈角度
{
    if (!can_driver_installed)
        return -99999.0f;

    float res = -99999.0f;

    twai_message_t txbuffer;
    twai_message_t rxbuffer;

    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 1;
    txbuffer.data[0] = 0x92;
    txbuffer.data[1] = 0x00;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    txbuffer.data[4] = 0x00;
    txbuffer.data[5] = 0x00;
    txbuffer.data[6] = 0x00;
    txbuffer.data[7] = 0x00;
    twai_transmit(&txbuffer, 0);

    unsigned long startMillis = millis();
    while (millis() - startMillis < wait_time)
    {

        if (twai_receive(&rxbuffer, 1) == ESP_OK)
        {
            if (rxbuffer.identifier == 0x240 + id)
            {
                Serial.print("id:"); Serial.print(id);
                Serial.print(" data[4-7]:");
                Serial.print(rxbuffer.data[4], HEX); Serial.print(" ");
                Serial.print(rxbuffer.data[5], HEX); Serial.print(" ");
                Serial.print(rxbuffer.data[6], HEX); Serial.print(" ");
                Serial.print(rxbuffer.data[7], HEX); Serial.print(" | raw:");
                int32_t raw = *(int32_t *)&rxbuffer.data[4];
                Serial.println(raw);
                res = raw * 0.01f;
                break;
            }
        }
    }
    return res;
}
float read_Sc_angle(int id) // 读取单圈角度
{
    if (!can_driver_installed)
        return -99999.0f;

    float res = -99999.0f;

    twai_message_t txbuffer;
    twai_message_t rxbuffer;

    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 1;
    txbuffer.data[0] = 0x94;
    txbuffer.data[1] = 0x00;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    txbuffer.data[4] = 0x00;
    txbuffer.data[5] = 0x00;
    txbuffer.data[6] = 0x00;
    txbuffer.data[7] = 0x00;
    twai_transmit(&txbuffer, 0);

    unsigned long startMillis = millis();
    while (millis() - startMillis < wait_time)
    {

        if (twai_receive(&rxbuffer, 1) == ESP_OK)
        {
            if (rxbuffer.identifier == 0x240 + id)
            {
                res = *(float *)&rxbuffer.data[4];
                break;
            }
        }
    }
    return res;
}
float read_motor_status_1(int id, int suoyin) // 读取电机状态1和错误标志命令
{
    if (!can_driver_installed)
        return -99999.0f;

    float res = -99999.0f;

    twai_message_t txbuffer;
    twai_message_t rxbuffer;

    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 1;
    txbuffer.data[0] = 0x9A;
    txbuffer.data[1] = suoyin;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    txbuffer.data[4] = 0x00;
    txbuffer.data[5] = 0x00;
    txbuffer.data[6] = 0x00;
    txbuffer.data[7] = 0x00;
    twai_transmit(&txbuffer, 0);

    unsigned long startMillis = millis();
    while (millis() - startMillis < wait_time)
    {

        if (twai_receive(&rxbuffer, 1) == ESP_OK)
        {
            if (rxbuffer.identifier == 0x240 + id)
            {
                res = *(float *)&rxbuffer.data[4];
                break;
            }
        }
    }
    return res;
}
float read_motor_status_2(int id, int suoyin) // 读取电机状态2命令
{
    if (!can_driver_installed)
        return -99999.0f;

    float res = -99999.0f;

    twai_message_t txbuffer;
    twai_message_t rxbuffer;

    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 1;
    txbuffer.data[0] = 0x9C;
    txbuffer.data[1] = suoyin;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    txbuffer.data[4] = 0x00;
    txbuffer.data[5] = 0x00;
    txbuffer.data[6] = 0x00;
    txbuffer.data[7] = 0x00;

    twai_transmit(&txbuffer, 0);

    unsigned long startMillis = millis();
    while (millis() - startMillis < wait_time)
    {

        if (twai_receive(&rxbuffer, 1) == ESP_OK)
        {
            if (rxbuffer.identifier == 0x240 + id)
            {
                res = *(float *)&rxbuffer.data[4];
                break;
            }
        }
    }
    return res;
}
float read_motor_status_3(int id) // 读取电机状态3命令
{
    if (!can_driver_installed)
        return -99999.0f;

    float res = -99999.0f;

    twai_message_t txbuffer;
    twai_message_t rxbuffer;

    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 1;
    txbuffer.data[0] = 0x9D;
    txbuffer.data[1] = 0x00;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    txbuffer.data[4] = 0x00;
    txbuffer.data[5] = 0x00;
    txbuffer.data[6] = 0x00;
    txbuffer.data[7] = 0x00;
    twai_transmit(&txbuffer, 0);

    unsigned long startMillis = millis();
    while (millis() - startMillis < wait_time)
    {

        if (twai_receive(&rxbuffer, 1) == ESP_OK)
        {
            if (rxbuffer.identifier == 0x240 + id)
            {
                res = *(float *)&rxbuffer.data[4];
                break;
            }
        }
    }
    return res;
}
void shutdown(int id) // 电机关闭命令
{
    // CAN_frame_t rx_frame;
    if (!can_driver_installed)
        return;

    twai_message_t txbuffer;
    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 8;
    txbuffer.data[0] = 0x80;
    txbuffer.data[1] = 0x00;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    txbuffer.data[4] = 0x00;
    txbuffer.data[5] = 0x00;
    txbuffer.data[6] = 0x00;
    txbuffer.data[7] = 0x00;
    twai_transmit(&txbuffer, 0);
}
// void stop(int id) // 电机停止命令
// {
//     // CAN_frame_t rx_frame;
//     if (!can_driver_installed)
//         return;

//     twai_message_t txbuffer;
//     txbuffer.identifier = 0x140 + id;
//     txbuffer.flags = 0;
//     txbuffer.data_length_code = 8;
//     txbuffer.data[0] = 0x81;
//     txbuffer.data[1] = 0x00;
//     txbuffer.data[2] = 0x00;
//     txbuffer.data[3] = 0x00;
//     txbuffer.data[4] = 0x00;
//     txbuffer.data[5] = 0x00;
//     txbuffer.data[6] = 0x00;
//     txbuffer.data[7] = 0x00;
//     twai_transmit(&txbuffer, 0);
// }

void Torque_control(int id, float AA) // 转矩闭环控制命令
{                                     // CAN_frame_t rx_frame;
    if (!can_driver_installed)
        return;

    twai_message_t txbuffer;
    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 8;

    txbuffer.data[0] = 0xA1;
    txbuffer.data[1] = 0x00;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    int32_t *p2 = (int32_t *)&txbuffer.data[4];
    *p2 = (int32_t)100 * AA;
    twai_transmit(&txbuffer, 0);
}

void Speed_control(int id, int speed) // 速度闭环控制命令
{                                     // CAN_frame_t rx_frame;
    if (!can_driver_installed)
        return;

    twai_message_t txbuffer;
    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 8;

    txbuffer.data[0] = 0xA2;
    txbuffer.data[1] = 0x00;
    int16_t *p1 = (int16_t *)&txbuffer.data[2];
    *p1 = (int16_t)speed;
    twai_transmit(&txbuffer, 0);
    // CAN_frame_t rx_frame;
}
void Single_position_control(int id, int speed, int position) // 单圈位置控制命令
{                                                             // CAN_frame_t rx_frame;
    if (!can_driver_installed)
        return;

    twai_message_t txbuffer;
    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 8;

    txbuffer.data[0] = 0xA6;
    txbuffer.data[1] = 0x00;
    int16_t *p1 = (int16_t *)&txbuffer.data[2];
    int32_t *p2 = (int32_t *)&txbuffer.data[4];
    *p1 = (int16_t)speed;
    *p2 = (int32_t)100 * position;
    twai_transmit(&txbuffer, 0);
}
void Incremental_position_control(int id, int speed, int position) // 增量位置闭环控制命令
{                                                                  // CAN_frame_t rx_frame;
    if (!can_driver_installed)
        return;

    twai_message_t txbuffer;
    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 8;

    txbuffer.data[0] = 0xA8;
    txbuffer.data[1] = 0x00;
    int16_t *p1 = (int16_t *)&txbuffer.data[2];
    int32_t *p2 = (int32_t *)&txbuffer.data[4];
    *p1 = (int16_t)speed;
    *p2 = (int32_t)100 * position;
    twai_transmit(&txbuffer, 0);
}

void reset(int id) // 系统复位指令
{
    // CAN_frame_t rx_frame;
    if (!can_driver_installed)
        return;

    twai_message_t txbuffer;
    txbuffer.identifier = 0x140 + id;
    txbuffer.flags = 0;
    txbuffer.data_length_code = 8;
    txbuffer.data[0] = 0x76;
    txbuffer.data[1] = 0x00;
    txbuffer.data[2] = 0x00;
    txbuffer.data[3] = 0x00;
    txbuffer.data[4] = 0x00;
    txbuffer.data[5] = 0x00;
    txbuffer.data[6] = 0x00;
    txbuffer.data[7] = 0x00;
    twai_transmit(&txbuffer, 0);
}

/*接收数据，通过判断接收的ID来确定是在使用哪个功能，进一步根据返回的数据，来判断显示出当前的状态
注：对于接收的数据需进行一个有效性的判断*/