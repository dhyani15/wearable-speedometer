#include "mbed.h"
#include "math.h"
#include "LCD_DISCO_F429ZI.h"
// setup the SPI
// PC1 - CS (chip select for gyro)
// PF7 - SCLK (Serial clock)
// PF8 - MISO (Master in Slave out)
// PF9 - MOSI (Master out Slave in)

LCD_DISCO_F429ZI lcd;
SPI spi(PF_9, PF_8, PF_7); // mosi, miso, sclk
DigitalOut gyro_cs(PC_1);  // chip select
int window_size =3;
float walking_time =0;
int i = 0;
const int sample = 10;
// stores the degree per second values of x,y and z axis for each sample
float dps_x =0;
float dps_y =0;
float dps_z = 0;
// stores the running sume of degrees over the interval of 20s
float dx = 0;
float dy = 0;
float dz = 0;
// noisy variables stores the instantaneous raw values
float noisy_x[400] = {0};
float noisy_y[400] = {0};
float noisy_z[400] = {0};
float noisy_mag[400]={ 0};
int k=0;
int cnt=0; // counter to keep track of number of times i run my get_sample() before printing distance
float s =0; // variable to store distance
float t=0; // running sum of total walking time
float linear_velocity=0;
// variables to store values of angular velocity after low pass filter

// variables used to display text on lcd
uint8_t text[30];
uint8_t text1[30];
uint8_t text2[30];


void display() // function to display raw data from the gyro
{
  printf("\n\r\n\r");
  printf("data = [");
  for (int i=0;i<100;i++){
    printf("%.2f,", noisy_mag[i]);
  }
  printf("]\n");
}

int window_sum(int start){ // helper function that calculates average of 3 gyro samples
    int sum=0;
    for(int i=start; i<start+window_size; i++)
    {
      sum+=abs(noisy_mag[i]);
    }
    return sum;
  }

void find_walkingtime(){ // algorithm to calculate the total walking time for a sample
  
  float counter =0;
  for (int i =0;i<100;i++){
    noisy_mag[i] = sqrt(noisy_x[i]*noisy_x[i]+noisy_y[i]*noisy_y[i]+noisy_z[i]*noisy_z[i]);
  }

  for (int i = 0;i<98;i+=3){
    int sum1 = window_sum(i);
    float avg= sum1/3;
    if(avg<25) {
      counter+=3;
    }
    walking_time = 5 - counter*0.05;
    }
}


void init_LCD(){ // helper function to initialize the lcd
    BSP_LCD_SetFont(&Font20);
  lcd.Clear(LCD_COLOR_RED);
  lcd.SetBackColor(LCD_COLOR_RED);
  lcd.SetTextColor(LCD_COLOR_WHITE);
}


void set_LCD_red() // change the background color of lcd to red and print message
{
 lcd.Clear(LCD_COLOR_RED);
      lcd.SetBackColor(LCD_COLOR_RED);
      lcd.DisplayStringAt(0, LINE(7), (uint8_t *) "Calculating", CENTER_MODE); 
}

void set_LCD_green(){ // change the background color of lcd to green and print message
  lcd.SetBackColor(LCD_COLOR_GREEN);
    lcd.DisplayStringAt(0, LINE(7), (uint8_t *) "  Sampling...", CENTER_MODE);
}


void lcd_display() // print the distance, velocity and total walking time over 20s to lcd
{
    sprintf((char*)text, "Distance = %.2f", s);
    lcd.DisplayStringAt(0, LINE(9), (uint8_t *)&text, CENTER_MODE);
    sprintf((char*)text2, "Walk Time = %.2f", t);
    lcd.DisplayStringAt(0, LINE(11), (uint8_t *)&text2, CENTER_MODE);
    sprintf((char*)text1, "Speed = %.2f", linear_velocity);
    lcd.DisplayStringAt(0, LINE(13), (uint8_t *)&text1, CENTER_MODE);
    
}

void get_sample(){ // get 10 gyro values with interval between each sample as 0.05s
    for (int j=0;j<10;j++){
      gyro_cs = 0;
      spi.write(232);
      short x_axis_low = spi.write(0x00);
      short x_axis_high = spi.write(0x00);
      short x_axis = x_axis_high << 8 | x_axis_low;

      gyro_cs = 1;

      // Read 2 bytes of angular velocity data around the Y axis (address code: 0xEA)
      gyro_cs = 0;

      spi.write(234);

      short y_axis_low = spi.write(0x00);
      short y_axis_high = spi.write(0x00);
      short y_axis = y_axis_high << 8 | y_axis_low;

      gyro_cs = 1;

      // Read 2 bytes of angular velocity data around the Z axis (address code: 0xEC)
      gyro_cs = 0;

      spi.write(236);
      short z_axis_low = spi.write(0x00);
      short z_axis_high = spi.write(0x00);
      short z_axis = z_axis_high << 8 | z_axis_low;
      gyro_cs = 1;
      // store values of sampled data
      noisy_x[k]=x_axis*0.00875; // degrees per second
      noisy_y[k]=y_axis*0.00875;
      noisy_z[k]=z_axis*0.00875;
      k++;
      
      // degrees per second
      dps_x += abs((x_axis *0.00875)); // multiply by 0.00875 to convert raw values to dps
      dps_y += abs((y_axis *0.00875));
      dps_z += abs((z_axis *0.00875));
      wait_us(50000);
    }

};

void config_gyro(){ // function to wake the gyro so that it starts detecting values around x,y, and z axis
  // Chip must be deselected
  gyro_cs = 1;

  // Setup the SPI for 8 bit data and SPI mode is 3
  spi.format(8, 3);
  // second edge capture, with a 1MHz clock rate
  spi.frequency(8000000);

  // initializing the gyroscope
  // Control register 1 (0x20) set to 00001111 -> 0x0F
  // enables all axes
  // turns on the gyroscope
  gyro_cs = 0;
  wait_us(1000);
  spi.write(32);
  spi.write(15);
  wait_us(1000);
  gyro_cs = 1;
  wait_us(2000000);
  // check if there is no bit in one of the registers where it shows my data is not ready
}




// main logic of device
int main()
{
  float avg_dps_x[sample];
float avg_dps_y[sample];
float avg_dps_z[sample];
  init_LCD(); // set up lcd
  config_gyro(); // set up gyro
  

  while (1){ // infinite loop
    
    set_LCD_green();
    wait_us(1000000);
  
    cnt=cnt+1;
    for (int i=0;i<sample;i++){ // 5 seconds to run
      get_sample(); // 0.5 seconds to run
      avg_dps_x[i] = dps_x/10;
      avg_dps_y[i] = dps_y/10;
      avg_dps_z[i] = dps_z/10;
      dps_x = 0;
      dps_y = 0;
      dps_z = 0;
    }
    find_walkingtime();
    // integrating step, angular velocity over 0.5s 
    for (int j=0;j<sample;j++){
      dx += avg_dps_x[j] * 0.5 * 0.0174533; // multiply by 0.0174533 to convert it into radians
      dy += avg_dps_y[j] * 0.5 * 0.0174533;
      dz += avg_dps_z[j] * 0.5 * 0.0174533;
    }

    float dis = sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2)); // finding the magnitude of x+y+z
    s= s+ (dis * 1.5); // running sum of distance over 5s samples
    t=t+walking_time; // running sum of walking time over 5s samples

    // setting the values to zero for next batch of 5s samples
    avg_dps_x[sample] = {0};
    avg_dps_y[sample] = {0};
    avg_dps_z[sample] = {0};
    dx=0;
    dy=0;
    dz=0;

    if(cnt>3) // after running 4 samples i.e. 20s of sampling, do processing and print values to lcd
    {
      //display(); uncomment me to print the raw values to serial terminal
      k=0;
      cnt=0;
      set_LCD_red();
      wait_us(1000000);

      if(s<0.5)
      {
        s=0.00;  //This is when there is no movement, so it becomes 0/0 
      }
      
      linear_velocity = s/(t); // calculating linear velocity
      printf("%0.2f",s);
      lcd_display(); // display values to lcd
      s=0;
      t=0;
    }
    
  }
}