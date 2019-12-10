#ifndef _PS2MS_H_
#define _PS2MS_H_

#include <mbed.h>

// #define DEBUG_TEST

#ifdef DEBUG_TEST
#define DEBUG_MESSAGE(...) printf(__VA_ARGS__)
#else
#define DEBUG_MESSAGE(...)
#endif

class PS2MS
{
public:
    /**
     * @brief Construct a new ps2ms init object
     * 
     * @param clk_pin : Clock pin
     * @param dat_pin : Data pin
     */
    PS2MS(PinName clk_pin, PinName dat_pin);

    /**
     * @brief Destroy the ps2ms init object
     * 
     */
    ~PS2MS();

    /**
     * @brief write a byte and read acknowledge
     * 
     * @param data : a byte to write
     */

    /**
     * @brief write a byte and read acknowledge
     * 
     * @param data : a byte to write
     * @return int16_t : data return if negative is error
     */
    int16_t writeAndReadAck(uint8_t data);

    /**
     * @brief read data
     * 
     * @return int16_t : data return if negative is error
     */
    int16_t read();

private:
    DigitalInOut clk_;
    DigitalInOut dat_;
    static const int MAX_RETRY = 1000000;

    /**
     * @brief Send a byte data
     * 
     * @param data : a character 
     * @return int16_t : Nagative value is an error number
     */
    int16_t send_(uint8_t data);

    /**
     * @brief Receive a byte data
     * 
     * @return int16_t : A data, Negative value is an error number 
     */
    int16_t recv_(void);

    /**
     * @brief Wait for clock state change
     * 
     * @return true : if wait done
     */
    bool waitForClockState_(int expectedState);

    /**
     * @brief Set pin to input mode
     * 
     * @param pin : address of DigitalInOut pin
     */
    void goHi_(DigitalInOut *pin);

    /**
     * @brief Set pin to output mode
     * 
     * @param pin : address of DigitalInOut pin
     */
    void goLo_(DigitalInOut *pin);
};

PS2MS::PS2MS(PinName clk_pin, PinName dat_pin) : clk_(clk_pin), dat_(dat_pin)
{
    goHi_(&clk_);
    goHi_(&dat_);
    wait_us(300);
}

PS2MS::~PS2MS()
{
}

int16_t PS2MS::writeAndReadAck(uint8_t data)
{
    int16_t ret = send_(data);

    if (ret < 0)
    {
        DEBUG_MESSAGE("Send data error.\n");
        return -1;
    }

    ret = recv_();

    if (ret < 0)
    {
        DEBUG_MESSAGE("Recv data error.\n");
        return -2;
    }

    return ret & 0xFF;
}

int16_t PS2MS::read()
{
    int16_t ret = recv_();

    if (ret < 0)
    {
        DEBUG_MESSAGE("Recv data error.\n");
        return -2;
    }

    return ret & 0xFF;
}

int16_t PS2MS::send_(uint8_t data)
{
    goHi_(&dat_);
    goHi_(&clk_);
    wait_us(200);

    goLo_(&clk_);
    wait_us(200);
    goLo_(&dat_);
    wait_us(10);
    goHi_(&clk_);
    wait_us(10);

    if (!waitForClockState_(0))
        return -1;

    uint8_t parity = 1;
    for (int i = 0; i < 10; i++)
    {
        if (i < 8)
        {
            /* Data bit */
            if (data & 0x01)
                dat_.write(1);
            else
                dat_.write(0);
        }
        else if (i == 8)
        {
            /* Parity bit */
            if (parity)
                dat_.write(1);
            else
                dat_.write(0);
        }
        else if (i == 9)
        {
            /* Stop bit */
            goHi_(&dat_);
            wait_us(50);
        }

        if (!waitForClockState_(1))
            return -2;
        if (!waitForClockState_(0))
            return -3;

        parity ^= (data & 0x01);
        data = data >> 1;
    }

    while (dat_.read() != 0)
    {
        DEBUG_MESSAGE("Send Error: data line is high.\n");
        return -4;
    }
    if (!waitForClockState_(1))
        return -5;
    goLo_(&clk_);

    return 0;
}

int16_t PS2MS::recv_(void)
{
    int16_t data = 0;
    int parcnt = 0;

    goHi_(&clk_);
    goHi_(&dat_);
    wait_us(50);

    for (int i = 0; i < 11; i++)
    {
        if (!waitForClockState_(0))
            return -1;
        if (i == 0)
        {
            if (dat_.read() != 0)
            {
                DEBUG_MESSAGE("Recv Error: First bit is not zero.\n");
                return -2;
            }
        }
        else if ((i > 0) && (i < 9))
        {
            if (dat_.read() == 0)
            {
                data &= ~(1 << (i - 1));
            }
            else
            {
                data |= (1 << (i - 1));
                parcnt++;
            }
        }
        else if (i == 9)
        {
            if (dat_.read() == 0)
            {
                if ((parcnt % 2) != 1)
                {
                    return -3;
                }
            }
            else
            {
                if ((parcnt % 2) != 0)
                {
                    return -4;
                }
            }
        }
        else if (i == 10)
        {
            if (dat_.read() != 1)
            {
                return -5;
            }
        }
        if (!waitForClockState_(1))
        {
            return -6;
        }
    }

    goLo_(&clk_);
    return data;
}

bool PS2MS::waitForClockState_(int expectedState)
{
    int cnt = 0;
    while (clk_.read() != expectedState)
    {
        cnt++;
        if (MAX_RETRY < cnt)
        {
            DEBUG_MESSAGE("Timeout: to wait for clock state.\n");
            return false;
        }
        wait_us(1);
    }
    return true;
}

void PS2MS::goHi_(DigitalInOut *pin)
{
    pin->input();
    pin->write(1);
}
void PS2MS::goLo_(DigitalInOut *pin)
{
    pin->output();
    pin->write(0);
}

#endif