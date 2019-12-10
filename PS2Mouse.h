#ifndef _PS2MOUSE_H_
#define _PS2MOUSE_H_

#include <mbed.h>
#include "PS2MS.h"

// #define DEBUG_TEST

#ifdef DEBUG_TEST
#define DEBUG_MESSAGE(...) printf(__VA_ARGS__)
#else
#define DEBUG_MESSAGE(...)
#endif

class PS2Mouse
{
public:
    enum Stand
    {
        STAND_PS2 = 0x00,
        STAND_INTELLIMOUSE = 0x03,
        STAND_INTELLIMOUSE_5BTN = 0x04,
    };
    enum Res
    {
        RES_1_COUNT_PER_MM = 0x00,
        RES_2_COUNT_PER_MM,
        RES_4_COUNT_PER_MM,
        RES_8_COUNT_PER_MM,
    };
    enum Sam
    {
        SAM_10_PER_SEC = 10,
        SAM_20_PER_SEC = 20,
        SAM_40_PER_SEC = 40,
        SAM_60_PER_SEC = 60,
        SAM_80_PER_SEC = 80,
        SAM_100_PER_SEC = 100,
        SAM_200_PER_SEC = 200,
    };

    typedef struct
    {
        bool btnLeft;
        bool btnCenter;
        bool btnRight;
        int moveX;
        int moveY;
        int moveZ;
    } mouse_event_t;

    mouse_event_t info;

    PS2Mouse(PinName clk_pin, PinName dat_pin);
    ~PS2Mouse();
    bool begin();
    void update();

    void setResolution(uint8_t resolution);
    void setSampleRate(uint8_t samplerate);

private:
    enum Commands
    {
        RESET = 0xFF,
        RESEND = 0xFE,
        SET_DEFAULTS = 0xF6,
        DISABLE_DATA_REPORTING = 0xF5,
        ENABLE_DATA_REPORTING = 0xF4,
        SET_SAMPLE_RATE = 0xF3,
        GET_DEVICE_ID = 0xF2,
        SET_REMOTE_MODE = 0xF0,
        SET_WRAP_MODE = 0xEE,
        RESET_WRAP_MODE = 0xEC,
        READ_DATA = 0xEB,
        SET_STREAM_MODE = 0xEA,
        STATUS_REQUEST = 0xE9,
        SET_RESOLUTION = 0xE8,
    };

    typedef struct
    {
        union {
            uint8_t byte;
            struct
            {
                uint8_t btnLeft : 1;
                uint8_t btnRight : 1;
                uint8_t btnCenter : 1;
                uint8_t always1 : 1;
                uint8_t signX : 1;
                uint8_t signY : 1;
                uint8_t overflowX : 1;
                uint8_t overflowY : 1;
            } bit;
        } byte1;
        union {
            uint8_t byte;
        } byte2;
        union {
            uint8_t byte;
        } byte3;
        union {
            uint8_t byte;
            struct
            {
                uint8_t value : 7;
                uint8_t signZ : 1;
            } bit;
            struct
            {
                uint8_t value : 3;
                uint8_t signZ : 1;
                uint8_t btn4th : 1;
                uint8_t btn5th : 1;
                uint8_t always0 : 2;
            } bit45;
        } byte4;
    } mouse_info_t;
    mouse_info_t mi_;

    PS2MS ps2ms_;
};

PS2Mouse::PS2Mouse(PinName clk_pin, PinName dat_pin) : ps2ms_(clk_pin, dat_pin)
{
}

PS2Mouse::~PS2Mouse()
{
}

bool PS2Mouse::begin()
{
    // char txdat[11] = {0xFF, 0xFF, 0xF5, 0xF6, 0xE8, 0x03, 0xF3, 0x0A, 0xE9, 0xE7, 0xF0};
    // char txdat[5] = {0xFF, 0xFF, 0xF5, 0xF6, 0xF0};

    // char txdat[] = {0xFF, 0xE9, 0xF6, 0xE9, 0xF0, 0xE9, 0xEB};
    char txdat[] = {RESET, STATUS_REQUEST, SET_DEFAULTS, STATUS_REQUEST, SET_REMOTE_MODE, STATUS_REQUEST, READ_DATA};
    const int n = sizeof(txdat);
    Timer t;
    t.start();
    for (int i = 0; i < n; i++)
    {
        if (ps2ms_.writeAndReadAck(txdat[i]) != 0xFA)
            return false;

        switch (txdat[i])
        {
        case 0xFF:
            DEBUG_MESSAGE("Self-test -> %X\n", ps2ms_.read());
            DEBUG_MESSAGE("Device ID -> %X\n", ps2ms_.read());
            break;
        case 0xEB:
            DEBUG_MESSAGE("Byte1 -> %X\n", ps2ms_.read());
            DEBUG_MESSAGE("Byte2 -> %X\n", ps2ms_.read());
            DEBUG_MESSAGE("Byte3 -> %X\n", ps2ms_.read());
            break;
        case 0xE9:
            DEBUG_MESSAGE("Byte1 -> %X\n", ps2ms_.read());
            DEBUG_MESSAGE("Byte2 -> %X\n", ps2ms_.read());
            DEBUG_MESSAGE("Byte3 -> %X\n", ps2ms_.read());
            break;
        default:
            break;
        }
        DEBUG_MESSAGE("\n");
    }
    update();
    return true;
}

void PS2Mouse::update()
{
    ps2ms_.writeAndReadAck(Commands::READ_DATA);

    mi_.byte1.byte = ps2ms_.read();
    mi_.byte2.byte = ps2ms_.read();
    mi_.byte3.byte = ps2ms_.read();

    info.btnLeft = mi_.byte1.bit.btnLeft ? true : false;
    info.btnCenter = mi_.byte1.bit.btnCenter ? true : false;
    info.btnRight = mi_.byte1.bit.btnRight ? true : false;

    info.moveX = mi_.byte1.bit.signX ? (-256 + mi_.byte2.byte) : mi_.byte2.byte;
    info.moveY = mi_.byte1.bit.signY ? (-256 + mi_.byte3.byte) : mi_.byte3.byte;
    info.moveZ = mi_.byte4.bit.signZ ? (-128 + mi_.byte4.bit.value) : mi_.byte4.bit.value;
}

void PS2Mouse::setResolution(uint8_t resolution)
{
    // ps2ms_.writeAndReadAck(Commands::SET_RESOLUTION);
    // ps2ms_.writeAndReadAck(resolution);
}
void PS2Mouse::setSampleRate(uint8_t samplerate)
{
    // ps2ms_.writeAndReadAck(Commands::SET_SAMPLE_RATE);
    // ps2ms_.writeAndReadAck(samplerate);
}
#endif