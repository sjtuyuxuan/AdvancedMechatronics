#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include <stdio.h>

// DEVCFG0
#pragma config DEBUG = OFF       // disable debugging
#pragma config JTAGEN = OFF      // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF         // disable flash write protect
#pragma config BWP = OFF         // disable boot write protect
#pragma config CP = OFF          // disable code protect

// DEVCFG1
#pragma config FNOSC = FRCPLL //  use fast frc oscillator with pll
#pragma config FSOSCEN = OFF  // disable secondary oscillator
#pragma config IESO = OFF     // disable switching clocks
#pragma config POSCMOD = OFF   // primary osc disabled
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD       // disable clock switch and FSCM
#pragma config WDTPS = PS1048576    // use largest wdt value
#pragma config WINDIS = OFF         // use non-window mode wdt
#pragma config FWDTEN = OFF         // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz fast rc internal oscillator
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0     // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF  // allow multiple reconfigurations

#define OUT_LEN 255

void Timer1Init (void);
void ButtomInit (void);
void LEDInit (void);
void WriteUART(const char * string);
void ReadUART(char * message, int maxLength);


uint8_t buttom_flag = 0;
char output_str[OUT_LEN];
uint32_t time = 0;

int main() {

    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
 
    U1RXRbits.U1RXR = 0b0000; // Set A2 to U1RX
    RPB3Rbits.RPB3R = 0b0001; // Set B3 to U1TX
    

    U1MODEbits.BRGH = 0;
    U1BRG = ((48000000 / 230400) / 16) - 1;

    U1MODEbits.PDSEL = 0;
    U1MODEbits.STSEL = 0;

    U1STAbits.UTXEN = 1;
    U1STAbits.URXEN = 1;

    U1MODEbits.ON = 1;
    
    Timer1Init();
    ButtomInit();
    LEDInit();

    
    __builtin_enable_interrupts();

    while (1) {
        // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
        // remember the core timer runs at half the sysclk
        if (PORTBbits.RB4 == 0) {
            buttom_flag = 1;
            sprintf (output_str, "Blink! %d th \r\n", time++);
            WriteUART(output_str);
        } // use Timer1

        // using CP0_COUNT
//        LATAbits.LATA4 = 1;  
//        _CP0_SET_COUNT(0);
//        while(_CP0_GET_COUNT()<24000000/2){}
//        LATAbits.LATA4 = 0;
//        _CP0_SET_COUNT(0);
//        while(_CP0_GET_COUNT()<24000000/2){}
//        LATAbits.LATA4 = 1;
//        _CP0_SET_COUNT(0);
//        while(_CP0_GET_COUNT()<24000000/2){}
//        LATAbits.LATA4 = 0;
//        _CP0_SET_COUNT(0);
//        while(_CP0_GET_COUNT()<24000000/2){}
    }
}

void Timer1Init (void) {
    T1CONbits.TCKPS = 0b01;  // pre-scaler 1:8
    PR1 = 5999;              // 1khz
    TMR1 = 0;
    T1CONbits.ON = 1;
    IPC1bits.T1IP = 4;       // priority
    IPC1bits.T1IS = 0;       // sub-priority
    IFS0bits.T1IF = 0;       // int flag
    IEC0bits.T1IE = 1;       // enable int
}

void __ISR(_TIMER_1_VECTOR, IPL4SOFT) Tic(void) {
    static int count = 0;
    if (buttom_flag != 0 && count == 0) {
        count = 2000;
        LATAbits.LATA4 = 1;
    }
    if (count == 1500) LATAbits.LATA4 = 0;
    if (count == 1000) LATAbits.LATA4 = 1;
    if (count == 500) {
        LATAbits.LATA4 = 0;
        buttom_flag = 0;
    }
    if (count > 0) --count;
    IFS0bits.T1IF = 0;
}

void ButtomInit (void) {TRISBbits.TRISB4 = 1;}

void LEDInit (void) {
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 0;
}

void WriteUART(const char * string) {
  while (*string != '\0') {
    while (U1STAbits.UTXBF) {
      ; // wait until tx written
    }
    U1TXREG = *string;
    ++string;
  }
}

void ReadUART(char * message, int maxLength) {
  char data = 0;
  int complete = 0, num_bytes = 0;
  // loop until you get a '\r' or '\n'
  while (!complete) {
    if (U1STAbits.URXDA) { // if data is available
      data = U1RXREG;      // read the data
      if ((data == '\n') || (data == '\r')) {
        complete = 1;
      } else {
        message[num_bytes] = data;
        ++num_bytes;
        // roll over if the array is too small
        if (num_bytes >= maxLength) {
          num_bytes = 0;
        }
      }
    }
  }
  // end the string
  message[num_bytes] = '\0';
}