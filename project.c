#include <LPC17xx.h>
#include <stdio.h>

//3.3V in analog corresponds to 0xfff in digital(=16^3=2^12=4096)
#define Ref_Vtg 3.300
#define Full_Scale 0xfff

unsigned long int temp1, temp2;

#define RS_CTRL (1<<8) //bit mask for Register select ctrl line of LCD, determines if cmd or data, conn. to 9th bit of a port,lsb=0 msb=15
#define EN_CTRL (1<<9)//enable ctrl line conn. to 10th bit of a port
#define DT_CTRL (0xf<<4)//data ctrl line uses 4lsb of port(bits4 -7)

//#define alpha 0.5


//function prototypes
void lcd_init(void);
void wr_cn(void);
void clr_disp(void);
void delay_lcd(unsigned int);
void lcd_com(void);
void wr_dn(void);
void lcd_data(void);
void clear_ports(void);
void lcd_puts(unsigned char *);

int main(void)
{
unsigned long adc_temp, adc_temp1;
unsigned int i;
float in_vtg, distance;
unsigned char vtg[7];
unsigned char Msg3[11] = {"Distance:"};

SystemInit();//initializeslpc1768's system clock,other low-level hardware settings.
SystemCoreClockUpdate();//updates global var SystemCoreClock(Hz).

LPC_SC->PCONP |= (1<<12); //Enable the peripheral ADC. Enable power to ADC
lcd_init();
LPC_PINCON->PINSEL1 = (01 << 14); // Output from the sensor is input to the microcontroller P0.23 is set 1

temp1 = 0x80; //Cursor at the beginning of the first line
lcd_com();
delay_lcd(800);
lcd_puts(&Msg3[0]);


while(1)
{
LPC_ADC->ADCR = (1<<0) | (1<<21) | (1<<24); //Select AD0.0, Power ON, Start conversion now
while((adc_temp = LPC_ADC->ADGDR) == 0x80000000); //Loop till DONE bit is set

adc_temp = (LPC_ADC->ADGDR >> 4) & 0xfff; //Read the digital data. It then clears the DONE flag. The value is now a 12 bit digital data.
// adc_temp1 = adc_temp*alpha + (1-alpha)*adc_temp;
adc_temp1 = adc_temp;
in_vtg = ((float)adc_temp1 * Ref_Vtg) / Full_Scale; //in_vtg is the required analog voltage.
// V analog = V digital * 3.3 / 2^12
distance = 27/in_vtg;
sprintf(vtg, "%3.2f", distance); //Convert the readings into string to display on LCD

for(i=0; i < 2000; i++);

temp1 = 0x8c;
lcd_com();
delay_lcd(800);
lcd_puts(&vtg[0]);

for(i = 0; i < 20000; i++);
//for(j = 0; j < 200; j++);

for(i = 0; i < 7; i++)
vtg[i] = 0x00;
adc_temp = 0;
in_vtg = 0;
}

}

void lcd_init() //initializing LCD module
{
LPC_PINCON->PINSEL0 &= 0xfff000ff;
LPC_GPIO0->FIODIR|=DT_CTRL;
LPC_GPIO0->FIODIR|=RS_CTRL;
LPC_GPIO0->FIODIR|=EN_CTRL;

clear_ports();
delay_lcd(3200);

temp2=(0x30<<0);
wr_cn();
delay_lcd(30000);

temp2=(0x30<<0);
wr_cn();
delay_lcd(30000);

temp2=(0x30<<0);
wr_cn();
delay_lcd(30000);

temp2=(0x20<<0);
wr_cn();
delay_lcd(30000);

temp1=0x28;
lcd_com();
delay_lcd(30000);

temp1=0x0c;
lcd_com();
delay_lcd(30000);

temp1=0x06;
lcd_com();
delay_lcd(800);

temp1=0x01;
lcd_com();
delay_lcd(800);

temp1=0x80;
lcd_com();
delay_lcd(10000);
return;
}

void lcd_com(void)
{
temp2= temp1 & 0xf0;
temp2= temp2 << 0;
wr_cn();
temp2= temp1 & 0x0f;
temp2= temp2 << 4;
wr_cn();
delay_lcd(1000);
return;
}

void wr_cn(void) //writes cmd data to LCD module
{
clear_ports();
LPC_GPIO0->FIOPIN=temp2;
LPC_GPIO0->FIOCLR=RS_CTRL;
LPC_GPIO0->FIOSET=EN_CTRL;
delay_lcd(25);
LPC_GPIO0->FIOCLR=EN_CTRL;
return;
}

void lcd_data(void)
{
temp2= temp1 & 0xf0;
temp2= temp2 << 0;
wr_dn();
temp2= temp1 & 0x0f;
temp2= temp2 << 4;
wr_dn();
delay_lcd(1000);
return;
}

void wr_dn(void)
{
clear_ports();
LPC_GPIO0->FIOPIN=temp2;
LPC_GPIO0->FIOSET=RS_CTRL;
LPC_GPIO0->FIOSET=EN_CTRL;
delay_lcd(25);
LPC_GPIO0->FIOCLR=EN_CTRL;
return;
}

void delay_lcd(unsigned int r1)
{
unsigned int r;
for(r=0;r<r1;r++);
return;
}

void clr_disp(void)
{
temp1=0x01;
lcd_com();
delay_lcd(10000);
return;
}

void clear_ports(void)
{
LPC_GPIO0->FIOCLR=DT_CTRL;
LPC_GPIO0->FIOCLR=RS_CTRL;
LPC_GPIO0->FIOCLR=EN_CTRL;
return;
}

void lcd_puts(unsigned char *buf1)
{
unsigned int i=0;
while(buf1[i]!='\0')
{
temp1=buf1[i];
lcd_data();
i++;
if(i==16)
{
temp1=0xc0;
lcd_com();
}
}
}