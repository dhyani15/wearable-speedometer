#include <mbed.h>
SPI spi(PF_9,PF_8,PF_7); // since MOSI ,MISO and CLK are on PF_9,PF_8,PF_7 respectively
DigitalOut cs(PC_1); // since CS is connected to PC_1 pin
//============= SPI protocol=================
// if we want to read, send first bit as 1 else 0 for write
int main() {
  cs =1;
  spi.format(8,3);
  spi.frequency(1000000);
  // put your setup code here, to run once:
  cs =0;
  spi.write(0x20);
  spi.write(0x0F);
  cs=1;
  while(1) {
    // put your main code here, to run repeatedly:
    // int whoami = spi.write(0x00); // two writes makes it read
    
    cs =0;
    spi.write(0xE8);
    int lowbitx = spi.write(0x00);
    int highbitx = spi.write(0x00);
    int x = highbitx<<8 |lowbitx;
    printf(" X 0x%d\t", x);
    wait_us(100000);
    // cs =1;
    // cs =0;
    // spi.write(0xEA);
    int lowbity = spi.write(0x00);
    int highbity = spi.write(0x00);
    int y = highbity<<8 |lowbity;
    printf(" Y 0x%d\t", y);
    wait_us(100000);
    // cs =1;
    // cs =0;
    // spi.write(0xEC);
    int lowbitz = spi.write(0x00);
    int highbitz = spi.write(0x00);
    int z = highbitz<<8 |lowbitz;
    printf(" Z 0x%d\n", z);
    cs =1;
    wait_us(100000);
    wait_us(1000000);
  }
}