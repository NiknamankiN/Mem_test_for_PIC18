#define F_CPU 10000000/64
#define I2C_SPEED 400000

float temp;
int fe = 0;
unsigned char Device_Address = 0b10101110;
int memory_size = 256;
unsigned char old_portA;
unsigned char data_U;
char k,k2,k3;
char rc_data[10];
short f1=0,f2=0,f3=0;
int t;
int i = 0;
unsigned char RxByte = 0;
unsigned char TxArray[15] = "perf_11!";
unsigned char RxArray[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned char ErrArray[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned char I2CByte = 0b00000000;

void I2C_Init()
{
   SSPSTAT = 0b10000000;
   SSPCON1 = 0b00111000;
   SSPCON2 = 0b01000000;
   TRISC = 0b00011101;
   TRISC.B6=0;
   TRISC.B7=1;
   SSPADD  = ((F_CPU*64/4)/I2C_SPEED) - 1;
}

void I2C_Start() {
    SSPCON2.SEN = 1;
    while(!PIR1.SSPIF);
    PIR1.SSPIF = 0;
}

void I2C_ReStart() {
    SSPCON2.RSEN = 1;
    while(!PIR1.SSPIF);
    PIR1.SSPIF = 0;
}

void I2C_Stop(void)
{
    SSPCON2.PEN = 1;
    while(!PIR1.SSPIF);
    PIR1.SSPIF = 0;
}

short I2C_Send(unsigned char datar)
{
  SSPBUF=datar;
  while(!PIR1.SSPIF);
  PIR1.SSPIF = 0;
  return SSPCON2.ACKSTAT;
}

void I2C_Send_ACK(void)
{
    SSPCON2.ACKDT = 0;
    SSPCON2.ACKEN = 1;
    while(!PIR1.SSPIF);
    PIR1.SSPIF = 0;
}
void I2C_Send_NACK(void)
{
    SSPCON2.ACKDT = 1;
    SSPCON2.ACKEN = 1;
    while(!PIR1.SSPIF);
    PIR1.SSPIF = 0;
}

unsigned char I2C_Receive_byte()
{
  SSPCON2.RCEN = 1;
  while(!PIR1.SSPIF);
  PIR1.SSPIF = 0;
  return SSPBUF;
}

 void Write_Page_I2C(unsigned char Address,unsigned char* pData,unsigned char NoOfBytes)
{
    unsigned int i;
    I2C_Start();
    while(I2C_Send(Device_Address + 0) == 1)
    {
        I2C_ReStart();
    }
    I2C_Send(Address);
    for(i=0; i<NoOfBytes; i++){
        I2C_Send(pData[i]);
        Delay_5ms();
        }
    I2C_Stop();
}

void Read_Bytes_I2C(unsigned char Address, unsigned char* pData, unsigned int NoOfBytes)
{
    unsigned int i;
    I2C_Start();

    while(I2C_Send(Device_Address + 0) == 1)
    {
        I2C_ReStart();
    }

    I2C_Send(Address);
    I2C_ReStart();

    I2C_Send(Device_Address + 1);
    pData[0] = I2C_Receive_byte();
    for(i=1; i<NoOfBytes; i++)
    {
        I2C_Send_ACK();
        pData[i] = I2C_Receive_byte();
        Delay_ms(2);
    }
    I2C_Send_NACK();
    I2C_Stop();
}

void Clear()
{   int i,j;
    I2CByte = 0b00000000;
    for(i = 0; i < memory_size; ++i)
    {
        for(j = 0; j < 8; ++j)
        {
            I2CByte = (I2CByte>>1) + 0b10000000;
            Write_Page_I2C(i,&I2CByte, 1);
        }
        I2CByte = 0b00000000;
    }
}

void Read_1_up(char error)
{
    int i,j;
    I2CByte = 0b11111111;
    for(i = 0; i < memory_size; ++i)
    {
        Read_Bytes_I2C(i,RxArray,1);
        if(RxArray[0] != I2CByte)
        {
            I2CByte = 0b10000000;
            for(j = 0; j < 8; ++j)
            {
                if(RxArray[0]&I2CByte == 0)
                {
                    ErrArray[0] = i/110+10;
                    ErrArray[1] = i%110+10;
                    ErrArray[2] = j+10;
                    ErrArray[3] = error;
                    UART_Write_Text(ErrArray);
                }
                I2CByte = (I2CByte>>1);
            }
        }
        I2CByte = 0b11111111;
    }
}

void Read_1_Set_0_up(char error)
{
    int i,j;
    I2CByte = 0b11111111;
    for(i = 0; i < memory_size; ++i)
    {
        for(j = 0; j < 8; ++j)
        {
            if(RxArray[0]&(0b10000000>>j) != I2CByte&(0b10000000>>j))
            {
                ErrArray[0] = i/110+10;
                ErrArray[1] = i%110+10;
                ErrArray[2] = j+10;
                ErrArray[3] = error;
                UART_Write_Text(ErrArray);
            }
            I2CByte = (I2CByte>>1);
            if(i == 12)
            {
                PORTD.RD0 = 1;
                Delay_ms(10);
            }
            if (i == 13)
                PORTD.RD0 = 0;

            Write_Page_I2C(i,&I2CByte, 1);
        }
        I2CByte = 0b11111111;
    }
}
void Read_1_Set_0_down(char error)
{
    int i,j;
    I2CByte = 0b11111111;
    for(i = memory_size-1; i > -1; --i)
    {
        for(j = 7; j > -1; --j)
        {
            Read_Bytes_I2C(i,RxArray,1);
            if(RxArray[0]&(0b10000000>>j) != I2CByte&(0b10000000>>j))
            {
                ErrArray[0] = i/110+10;
                ErrArray[1] = i%110+10;
                ErrArray[2] = j+10;
                ErrArray[3] = error;
                UART_Write_Text(ErrArray);
            }
            I2CByte = (I2CByte<<1);
            Write_Page_I2C(i,&I2CByte, 1);
        }
        I2CByte = 0b11111111;
    }
}

void Read_0_down(char error)
{
    int i,j;
    for(i = memory_size-1; i >-1; --i)
    {
        Read_Bytes_I2C(i,RxArray,1);
        if(RxArray[0] != 0b00000000)
        {
            I2CByte = 0b00000001;
            for(j = 7; j > -1; --j)
            {
                if(RxArray[0]&I2CByte)
                {
                    ErrArray[0] = i/110+10;
                    ErrArray[1] = i%110+10;
                    ErrArray[2] = j+10;
                    ErrArray[3] = error;
                    UART_Write_Text(ErrArray);
                    Delay_ms(60);
                }
                I2CByte = (I2CByte<<1);
            }
        }
    }
}

void Read_0_up(char error)
{
    int i,j;
    I2CByte = 0b00000000;
    for(i = 0; i < memory_size; ++i)
    {
        Read_Bytes_I2C(i,RxArray,1);
        if(RxArray[0] != I2CByte)
        {
            I2CByte = 0b10000000;
            for(j = 0; j < 8; ++j)
            {
                if(RxArray[0]&I2CByte == 1)
                {
                    ErrArray[0] = i/110+10;
                    ErrArray[1] = i%110+10;
                    ErrArray[2] = j+10;
                    ErrArray[3] = error;
                    UART_Write_Text(ErrArray);
                }
                I2CByte = (I2CByte>>1);
            }
        }
        I2CByte = 0b00000000;
    }
}

void Read_1_Set_0_Read_0_up(char error1, char error2)
{
    int i,j;
    I2CByte = 0b11111111;
    for(i = 0; i < memory_size; ++i)
    {
        for(j = 0; j < 8; ++j)
        {
            Read_Bytes_I2C(i,RxArray,1);
            if(RxArray[0]&(0b10000000>>j) != I2CByte&(0b10000000>>j))
            {
                ErrArray[0] = i/110+10;
                ErrArray[1] = i%110+10;
                ErrArray[2] = j+10;
                ErrArray[3] = error1;
                UART_Write_Text(ErrArray);
            }
            I2CByte = (I2CByte>>1);
            Write_Page_I2C(i,&I2CByte, 1);
            Read_Bytes_I2C(i,RxArray,1);
            if(RxArray[0]&(0b10000000>>j) != I2CByte&(0b10000000>>j))
            {
                ErrArray[0] = i/110+10;
                ErrArray[1] = i%110+10;
                ErrArray[2] = j+10;
                ErrArray[3] = error2;
                UART_Write_Text(ErrArray);
            }

        }
        I2CByte = 0b11111111;
    }
}

void Read_1_Set_0_Read_0_down(char error1, char error2)
{
    int i,j;
    I2CByte = 0b11111111;
    for(i = memory_size-1; i > -1; --i)
    {
        for(j = 7; j > -1; --j)
        {
            Read_Bytes_I2C(i,RxArray,1);
            if(RxArray[0]&(0b10000000>>j) != I2CByte&(0b10000000>>j))
            {
                ErrArray[0] = i/110+10;
                ErrArray[1] = i%110+10;
                ErrArray[2] = j+10;
                ErrArray[3] = error1;
                UART_Write_Text(ErrArray);
            }
            I2CByte = (I2CByte<<1);
            Write_Page_I2C(i,&I2CByte, 1);
            Read_Bytes_I2C(i,RxArray,1);
            if(RxArray[0]&(0b10000000>>j) != I2CByte&(0b10000000>>j))
            {
                ErrArray[0] = i/110+10;
                ErrArray[1] = i%110+10;
                ErrArray[2] = j+10;
                ErrArray[3] = error2;
                UART_Write_Text(ErrArray);
            }

        }
        I2CByte = 0b11111111;
    }
}

void Test_1()
{
    //E;/\(R1,P);\/R0
    //March Test
    Clear();
    Read_1_Set_0_up('S');
    Read_0_down('O');
}

void Test_2()
{
    Clear();
    Read_1_Set_0_Read_0_up('S','O');
    Read_0_down('R');
}

void Test_3()
{
    Clear();
    Read_1_Set_0_up('S');
    Clear();
    Read_1_Set_0_Read_0_down('T','O');
    Read_0_up('R');
}

void Flash_March()
{
    Clear();
    Read_1_Set_0_up('S');
    Read_0_up('O');
    Clear();
    Read_1_Set_0_down('T');
    Read_0_down('D');
}

void March_FT()
{
    Clear();
    Read_1_Set_0_Read_0_up('S','D');
    Read_0_up('R');
    Clear();
    Read_1_Set_0_Read_0_down('T','O');
    Read_0_down('W');

}

void Param_test()
{
    //Param. testing
    //SCL- SDA+ WP-
    LATD = 0b00110000;
    LATB.RB0 = 1;
    //SCL_amp
    Delay_ms(10);
    UART1_write('1');
    while(!UART1_Data_Ready());
    UART1_Read();

    LATD = 0b11010000;
    LATB.RB0 = 1;
    //WP_amp
    Delay_ms(10);
    UART1_write('2');
    while(!UART1_Data_Ready());
    UART1_Read();

    //SCL+ SDA+ WP-
    LATD = 0b00110010;
    LATB.RB0 = 1;
    //SCL_amp
    Delay_ms(10);
    UART1_write('3');
    while(!UART1_Data_Ready());
    UART1_Read();

    //SCL+ SDA- WP+
    LATD = 0b10001101;
    LATB.RB0 = 0;
    //SDA_amp
    Delay_ms(10);
    UART1_write('4');
    while(!UART1_Data_Ready());
    UART1_Read();

    //SCL+ SDA+ WP+
    LATD = 0b10001111;
    LATB.RB0 = 0;
    //SDA_amp
    Delay_ms(10);
    UART1_write('5');
    while(!UART1_Data_Ready());
    UART1_Read();
    LATD = 0x0;

}


void main()
{
  unsigned int i;
  unsigned int j;
  fe = 0;
  TRISA = 0b11111111;
  TRISB = 0b00000000;
  LATA = 0x00;
  LATB = 0x00;
  ADCON1 = 0b1111;
  PORTA = 0x00;
  PORTB = 0x00;
  //USART_Init(19200);
  Delay_ms(100);
  T0CON = 0b000000010;
  INTCON = 0b10100000;
  PIR1.RCIF = 1;

  PIR1.TXIF = 1;
  UART1_Init(19200);

  TRISD = 0b00000000;
  PORTD = 0x0;
  I2C_Init();
  f3 = 1;
  k2 = 32;
  k3 = 'R';
  memset(&TxArray[0],0,sizeof(TxArray));
  while(1){
           if(UART1_Data_Ready())
           {
                 TxArray[0] = UART1_Read();
                 if(TxArray[0] >='0' && TxArray[0] < '9')
                 {
                     while(!UART1_Data_Ready());
                     TxArray[1] = UART1_Read();
                 }
           }

           if(TxArray[0] == 'L'){

               strcpy(RxArray,"Pem320");
               memset(&TxArray[0],0,sizeof(TxArray));
               Write_Page_I2C(0,RxArray,1);
               Read_Bytes_I2C(0,TxArray,1);

               if(TxArray[0] =='P'){
                   strcpy(RxArray,"Mem320");
                   UART1_Write_Text(RxArray);
               }
               else
               {
                   strcpy(RxArray,"Mem001");
                   UART1_Write_Text(RxArray);
               }
               memset(&RxArray[0],0,sizeof(RxArray));
               k = '0';
           }
           switch (TxArray[0]) {
           case '0':
               Test_1();
               UART1_Write('F');
               break;
           case '1':
               Test_2();
               break;
           case '2':
               Test_3();
               break;
           case '3':
               Flash_March();
               break;
           case '4':
               March_FT();
               break;
           }
           TxArray[0] = 'e';
           if(TxArray[1] == '0')
           {
               Param_test();
               TxArray[1] = '1';
           }



  }
}