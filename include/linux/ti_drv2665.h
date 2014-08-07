#ifndef _LINUX_TI_DRV2665_H
#define _LINUX_TI_DRV2665_H

#define TI_DRV2665_MAGIC_NUMBER 0xC0FFFEB0
#define NUM_ACTUATORS 1
#define VIBE_OUTPUT_SAMPLE_SIZE 50
#define PLAY_CMD_PIEZO      0x0B 
#define DRV2665_BOARD_NAME "piezo"
#define SPI_HEADER_SIZE 3
#define MODULE_NAME "ti_drv2665"
#define WATCHDOG_TIMEOUT    10  /* 10 timer cycles */
#define MAX_SPI_BUFFER_SIZE (NUM_ACTUATORS * (VIBE_OUTPUT_SAMPLE_SIZE + SPI_HEADER_SIZE))
#define SPI_BUFFER_SIZE (NUM_ACTUATORS * (VIBE_OUTPUT_SAMPLE_SIZE + SPI_HEADER_SIZE))
#define GAIN_50 0x00
#define GAIN_100 0x01
#define GAIN_150 0x02
#define GAIN_200 0x03
#define TIMEOUT_5MS 0x00
#define TIMEOUT_10MS 0x01
#define TIMEOUT_15MS 0x02
#define TIMEOUT_20MS 0x03
#define TIMEOUT_BITSHIFT 0x02
#define DRV2665_GAIN GAIN_150
#define DRV2665_TIMEOUT (TIMEOUT_20MS << TIMEOUT_BITSHIFT)

#define NUM_EXTRA_BUFFERS   1
#define DEVICE_BUS 5
#define DEVICE_ADDR 0x59

struct ti_drv2665_data{
    struct i2c_client* client;
    int8_t buf[VIBE_OUTPUT_SAMPLE_SIZE*3];
    int8_t lastPacket[VIBE_OUTPUT_SAMPLE_SIZE+1];
    int8_t lastPacket_size;
    int timerPeriodMs;
    bool nackDetected;
    char isPlaying;
    char writeBuffer[MAX_SPI_BUFFER_SIZE];
    bool timerStarted;
    struct hrtimer timer;
    ktime_t period; 
    int wdCnt;
    struct semaphore sem;
};
struct buffer_data{
    u_int8_t index; 
    u_int8_t depth;       
    u_int8_t size;   
    u_int8_t dataBuffer[VIBE_OUTPUT_SAMPLE_SIZE];
};

int get_lastPacket(void);

#endif
