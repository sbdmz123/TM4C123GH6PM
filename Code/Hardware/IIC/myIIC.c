/**
 * IIC:
 *  - SCL - PB2
 *  - SDA - PB3
 */
#include "IIC/myIIC.h"

#include "inc/hw_gpio.h"
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
void myIICInit(void) {
  //   // enable i2c0 module
  //   SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);

  //   // reset module
  //   SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);

  //   // enable GPIO that contains I2C0
  //   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

  //   // Configure the pin muxing for I2C0 functions on port B2 and B3.
  //   GPIOPinConfigure(GPIO_PB2_I2C0SCL);
  //   GPIOPinConfigure(GPIO_PB3_I2C0SDA);

  //   // Select the I2C function for these pins.
  //   GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
  //   GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

  //   I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);

  //   // clear I2C FIFOs
  //   HWREG(I2C0_BASE + I2C_O_FIFOCTL) = 80008000;
  SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

  GPIOPinConfigure(GPIO_PB2_I2C0SCL);
  GPIOPinConfigure(GPIO_PB3_I2C0SDA);

  GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_3);

  GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_STRENGTH_8MA,
                   GPIO_PIN_TYPE_STD_WPU);
  GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_3, GPIO_STRENGTH_8MA,
                   GPIO_PIN_TYPE_OD);

  I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(),
                      false);  // false:100kbps true:400kbps

  I2CMasterEnable(I2C0_BASE);
}

void myIICWriteLen(unsigned char addr, unsigned char headByte,
                   unsigned char length, unsigned char *data_t) {
  I2CMasterSlaveAddrSet(I2C0_BASE, addr, false);
  I2CMasterDataPut(I2C0_BASE, headByte);
  if (length == 0) {
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while (I2CMasterBusy(I2C0_BASE))
      ;
    return;
  }
  // Initialize sending data from the MCU
  I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
  // wait while MCU is done transferring
  while (I2CMasterBusy(I2C0_BASE))
    ;
  while (length > 1) {
    I2CMasterDataPut(I2C0_BASE, *data_t);
    // Initialize sending data from the MCU
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
    // wait while MCU is done transferring
    while (I2CMasterBusy(I2C0_BASE))
      ;
    data_t++;
    length--;
  }
  I2CMasterDataPut(I2C0_BASE, *data_t);
  // Initialize sending data from the MCU
  I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
  // wait while MCU is done transferring
  while (I2CMasterBusy(I2C0_BASE))
    ;
}

void myIICReadLen(unsigned char addr, unsigned char headByte,
                  unsigned char length, unsigned char *data_t) {
  I2CMasterSlaveAddrSet(I2C0_BASE, addr,
                        false);  // set dia chi slave, che do write
  I2CMasterDataPut(I2C0_BASE,
                   headByte);  // dat dia chi thanh ghi slave vao thanh ghi data
  I2CMasterControl(I2C0_BASE,
                   I2C_MASTER_CMD_SINGLE_SEND);  // goi tin hieu send data
  while (I2CMasterBusy(I2C0_BASE))
    ;  // cho goi xong

  if (length == 1) {
    I2CMasterSlaveAddrSet(I2C0_BASE, addr, true);  // cai dat read slave
    I2CMasterControl(I2C0_BASE,
                     I2C_MASTER_CMD_SINGLE_RECEIVE);  // tell master read data
    while (I2CMasterBusy(I2C0_BASE))
      ;  // cho truyen xong
    data_t[0] = I2CMasterDataGet(I2C0_BASE);
  } else {
    I2CMasterSlaveAddrSet(I2C0_BASE, addr, true);  // cai dat read slave
    I2CMasterControl(
        I2C0_BASE,
        I2C_MASTER_CMD_BURST_RECEIVE_START);  // tell master read data
    while (I2CMasterBusy(I2C0_BASE))
      ;
    *data_t = I2CMasterDataGet(I2C0_BASE);
    data_t++;
    length--;
    while (length > 1) {
      I2CMasterSlaveAddrSet(I2C0_BASE, addr, true);  // cai dat read slave
      I2CMasterControl(
          I2C0_BASE,
          I2C_MASTER_CMD_BURST_RECEIVE_CONT);  // tell master read data
      while (I2CMasterBusy(I2C0_BASE))
        ;
      *data_t = I2CMasterDataGet(I2C0_BASE);
      data_t++;
      length--;
    }
    I2CMasterSlaveAddrSet(I2C0_BASE, addr, true);  // cai dat read slave
    I2CMasterControl(
        I2C0_BASE,
        I2C_MASTER_CMD_BURST_RECEIVE_FINISH);  // tell master read data
    while (I2CMasterBusy(I2C0_BASE))
      ;
    *data_t = I2CMasterDataGet(I2C0_BASE);
  }
}
