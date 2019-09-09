 void Send_Request_massage();
 unsigned char Read_Identity_massage();
 unsigned char Specify_mode(unsigned char temp);
 unsigned int Read_Data_massage(unsigned char* data, unsigned int init_address);
 unsigned char Specify_Func(unsigned char temp);
 void Send_Exit_command();
 void Send_Password_command();
 unsigned char Add_Parity2(unsigned char temp);
 unsigned char Rean_Answer_to_PWcommand();
 void Send_Write_command();
 void Send_Read_message();
 unsigned int Read_Answer_to_Rcommand(unsigned char* data, unsigned int address);

#include <avr/io.h>
#define F_CPU 16000000UL
#define delay 2500
#include "util/delay.h"

int main(void)
{

	unsigned char temp;
	unsigned char mode;
	unsigned int address = 0;
	unsigned char decide_flag;
	unsigned char data_recieved[1024];
	unsigned char exit_flag2 = 0;
	DDRB = 0x00; /* PB6-7 as input to determine functionality, PB0 as input to start transmission,...*/
	DDRC = 0xFF; /* PC0-7 as data line output */
	DDRA = 0x00; /* PA0-7 as input Data */
	DDRD = 0x80; /* PD7 as end of transmission signal */
	

    while (1) 
    {
		while (1) /* check PB0 to start transmission */
		{
			temp = PINB;
			temp = temp & 0x01;
			temp = temp ^ 0x01;
			if (temp == 0x01)
			{
				break;
			}
		}
		Send_Request_massage();
		temp = Read_Identity_massage(); 
		mode = Specify_mode(temp);
		switch (mode){
			
			case 0x41: /* Mode A */
				PORTD = 0x80;
				address = Read_Data_massage(data_recieved, address);
				while(1){
					temp = PINB;
					temp = temp & 0xC0;
					temp = temp ^ 0xC0;
					temp = Specify_Func(temp);
					switch (temp){
					
						case 0x04: /* Exit command */
							Send_Exit_command();
							exit_flag2 = 1;
							break;
						case 0x01: /* Password command */
							while (1){
								Send_Password_command();
								decide_flag = Rean_Answer_to_PWcommand();
								if (decide_flag == 0) /* ACK message */
								{
									break;
								}
								if (decide_flag == 1) /* NACK message */
								{
									continue;
								}
								if (decide_flag == 2) /* Break message */
								{
									exit_flag2 = 1;
									break;
								}
							}
							break;
						case 0x02: /* Write command */
							while (1){
								Send_Write_command();
								decide_flag = Rean_Answer_to_PWcommand();
								if (decide_flag == 0) /* ACK message */
								{
									break;
								}
								if (decide_flag == 1) /* NACK message */
								{
									continue;
								}
							}
							break;
						case 0x03: /* Read command */
							Send_Read_message();
							address = Read_Answer_to_Rcommand(data_recieved, address);
							break;
					}
					if (exit_flag2 == 1)
					{
						exit_flag2 = 0;
						break;
					}
				}
				break;
			case 0x42: /* Mode B */
				break;
			case 0x43: /* Mode C or E*/
				break;
			
		}	
		PORTD = 0x80; /* end of transmission */
		_delay_ms(delay);
		PORTD = 0x00;
    }
}

void Send_Request_massage(){

	PORTC = 0xAF; /* "/" character */
	_delay_ms(delay);
	PORTC = 0x3F; /* "?" character */
	_delay_ms(delay);
	PORTC = 0x21; /* "!" character */
	_delay_ms(delay);
	PORTC = 0x8D; /* CR character */
	_delay_ms(delay);
	PORTC = 0x0A; /* LF character */
	_delay_ms(delay);
	PORTC = 0x00;
	DDRC = 0x00;
	_delay_ms(delay);

}

unsigned char Read_Identity_massage(){
	
	unsigned char temp;
	unsigned char mode;
	DDRC = 0x00; /* PC0-7 as data line input */
	temp = PINC; /* "/" character */
	_delay_ms(delay);
	temp = PINC; /* "X" character */
	_delay_ms(delay);
	temp = PINC; /* "X" character */
	_delay_ms(delay);
	temp = PINC; /* "X" character */
	_delay_ms(delay);
	mode = PINC; /* "Z" character */
	_delay_ms(delay);
	temp = PINC; /* CR character */
	_delay_ms(delay);
	temp = PINC; /* LF character */
	return mode;
	
}

unsigned char Specify_mode(unsigned char temp){

	unsigned char mode;

	switch (temp){
		
		case 0x41: /* "A" character */
		case 0x42: /* "B" character */
		case 0xC3: /* "C" character */
		case 0x44: /* "D" character */
		case 0xC5: /* "E" character */
		case 0xC6: /* "F" character */
			mode = 0x42; /* Mode B */
			break;
		case 0x81:
		case 0x82:
		case 0x03:
		case 0x84:
		case 0x05:
		case 0x06:
			mode = 0x43; /* Mode C or E */
			break;
		default:
			mode = 0x41; /* Mode A */
			break;
	}

	return mode;

}

unsigned int Read_Data_massage(unsigned char* data, unsigned int init_address){
	
	unsigned char temp;
	unsigned int address = init_address;
	unsigned char flag1 = 0;
	unsigned char flag2 = 0;
	while (1){
		_delay_ms(delay);
		temp = PINC; /* upper 8 bits of address */
		temp = temp & 0x7F;
		address = temp << 8;
		_delay_ms(delay);
		temp = PINC; /* lower 8 bits of address */
		temp = temp & 0x7F;
		address = address + temp;
		_delay_ms(delay);
		temp = PINC;  /* "(" character */
		temp = temp & 0x7F;
		_delay_ms(delay);
		while(1){
			temp = PINC;
			temp = temp & 0x7F;
			if (temp == 0x29) /* ")" character */
			{
				_delay_ms(delay);
				temp = PINC; 
				temp = temp & 0x7F;
				switch (temp){

					case 0x0D : /* CR character */
						_delay_ms(delay);
						temp = PINC; /* LF character */
						flag1 = 1;
						break;
					case 0x21 : /* "!" character */
						_delay_ms(delay);
						temp = PINC; /* CR character */
						_delay_ms(delay);
						temp = PINC; /* LF character */
						_delay_ms(delay);
						flag1 = 1;
						flag2 = 1;
						break;
					default:
						address = temp << 8;
						_delay_ms(delay);
						temp = PINC; /* lower 8 bits of address */
						temp = temp & 0x7F;
						address = address + temp;
						_delay_ms(delay);
						temp = PINC;  /* "(" character */
						_delay_ms(delay);
						break;

				}
			}
			else{
				data[address] = temp;
				address = address + 1;
				_delay_ms(delay);
			}
			if (flag1 == 1)
			{
				flag1 = 0;
				break;
			}

		}
		if (flag2 == 1)
		{
			flag2 = 0;
			break;
		}		
	}
	PORTD = 0x00;
	return address;

 }

 unsigned char Specify_Func(unsigned char temp){

	unsigned char Func = 0;
	switch (temp){

		case 0x00:
			Func = 0x01; /* Password command */
			break;
		case 0x40:
			Func = 0x02; /* Write command */
			break;
		case 0x80:
			Func = 0x03; /* Read command */
			break;
		case 0xC0:
			Func = 0x04; /* Exit command */
			break;

	}
	return Func;

 }

 void Send_Exit_command(){
	
	unsigned char BCC = 0x00;
	DDRC = 0xFF;
	PORTC = 0x81; /* SOH character */
	_delay_ms(delay);
	PORTC = 0x42; /* "B" character */
	BCC = 0x42;
	_delay_ms(delay);
	PORTC = 0x00; /* 0 character */
	BCC = BCC ^ 0x00;
	_delay_ms(delay);
	PORTC = 0x03; /* ETX character */
	BCC = BCC ^ 0x03;
	_delay_ms(delay);
	BCC = BCC & 0x7F;
	BCC = Add_Parity2(BCC);
	PORTC = BCC; /* BCC character */	

 }

  void Send_Password_command(){

	unsigned char temp;
	unsigned char count = 0;
	unsigned char BCC;
	DDRC = 0xFF;
	PORTC = 0x81; /* SOH character */
	_delay_ms(delay);
	PORTC = 0x50; /* "P" character */
	BCC = 0x50;
	_delay_ms(delay);
	PORTC = 0x81; /* 1 character */
	BCC = BCC ^ 0x81;
	_delay_ms(delay);
	PORTC = 0x82; /* STX character */
	BCC = BCC ^ 0x82;
	_delay_ms(delay);
	PORTC = 0x28; /* "(" character */	
	BCC = BCC ^ 0x28;
	_delay_ms(delay);
	while (1){
		temp = PINA;
		temp = temp ^ 0xFF;
		temp = Add_Parity2(temp);
		BCC = BCC ^ temp;
		PORTC = temp;
		_delay_ms(delay);
		if (temp == 0xA9)
		{
			break;
		}
		if (count == 75)
		{
			PORTC = 0xA9;
			BCC = BCC ^ 0xA9;
			_delay_ms(delay);
			break;
		}
		count = count + 1;
	}
	PORTC = 0x03; /* ETX character */
	BCC = BCC ^ 0x03;
	_delay_ms(delay);
	BCC = BCC & 0x7F;
	BCC = Add_Parity2(BCC);
	PORTC = BCC; /* BCC character */
	_delay_ms(delay);
	PORTC = 0x00;
	DDRC = 0x00;
	_delay_ms(delay);
	
  }

unsigned char Rean_Answer_to_PWcommand(){

	unsigned char temp;
	DDRC = 0x00;
	temp = PINC;
	_delay_ms(delay);
	switch (temp){

		case 0x06: /* ACK message */
			return 0;
		case 0x95: /* NACK message */
			return 1;
		case 0x81: /* SOH character */
			return 2;
		default:
			return 1;

	}

}

void Send_Write_command(){

	unsigned char BCC;
	unsigned char temp;
	unsigned char count = 0;
	DDRC = 0xFF;
	PORTC = 0x81; /* SOH character */
	_delay_ms(delay);
	PORTC = 0xD7; /* "W" character */
	BCC = 0xD7;
	_delay_ms(delay);
	PORTC = 0x81; /* 1 character */
	BCC = BCC ^ 0x81;
	_delay_ms(delay);
	PORTC = 0x82; /* STX character */
	BCC = BCC ^ 0x82;
	_delay_ms(delay);
	while (1){
		temp = PINA;
		temp = temp ^ 0xFF;
		temp = Add_Parity2(temp);
		BCC = BCC ^ temp;
		PORTC = temp;
		_delay_ms(delay);
		if (temp == 0xA9)
		{
			break;
		}
		if (count == 76)
		{
			PORTC = 0xA9;
			BCC = BCC ^ 0xA9;
			_delay_ms(delay);
			break;
		}
		count = count + 1;
	}
	PORTC = 0x03; /* ETX character */
	BCC = BCC ^ 0x03;
	BCC = BCC & 0x7F;
	BCC = Add_Parity2(BCC);
	_delay_ms(delay);
	PORTC = BCC; /* BCC character */
	_delay_ms(delay);
	PORTC = 0x00;
	DDRC = 0x00;
	_delay_ms(delay);

}

void Send_Read_message(){

	unsigned char temp;
	unsigned char BCC;
	DDRC = 0xFF;
	PORTC = 0x81; /* SOH character */
	_delay_ms(delay);
	PORTC = 0xD2; /* "R" character */
	BCC = 0xD2;
	_delay_ms(delay);
	PORTC = 0x81; /* 1 character */
	BCC = BCC ^ 0x81;
	_delay_ms(delay);
	PORTC = 0x82; /* STX character */
	BCC = BCC ^ 0x82;
	_delay_ms(delay);
	temp = PINA; /* upper 8 bits of address */
	temp = temp ^ 0xFF;
	temp = Add_Parity2(temp);
	BCC = BCC ^ temp;
	PORTC = temp;
	_delay_ms(delay);
	temp = PINA; /* lower 8 bits of address */
	temp = temp ^ 0xFF;
	temp = Add_Parity2(temp);
	BCC = BCC ^ temp;
	PORTC = temp;
	_delay_ms(delay);
	PORTC = 0x28; /* "(" character */
	BCC = BCC ^ 0x28;
	_delay_ms(delay);
	temp = PINA; /* number of characters to be read */
	temp = temp ^ 0xFF;
	temp = Add_Parity2(temp);
	BCC = BCC ^ temp;
	PORTC = temp;
	_delay_ms(delay);
	PORTC = 0xA9; /* ")" character */
	BCC = BCC ^ 0xA9;
	_delay_ms(delay);
	PORTC = 0x03; /* ETX character */
	BCC = BCC ^ 0x03;
	_delay_ms(delay);
	BCC = BCC & 0x7F;
	BCC = Add_Parity2(BCC);
	PORTC = BCC; /* BCC character */
	_delay_ms(delay);
	PORTC = 0x00;
	DDRC = 0x00;
	_delay_ms(delay);

}

unsigned int Read_Answer_to_Rcommand(unsigned char* data, unsigned int address){

	unsigned char temp;
	while (1){
		DDRC = 0x00;
		temp = PINC;
		if (temp == 0x82) /* STX character */
		{
			_delay_ms(delay);
			temp = PINC; /* useless data */
			_delay_ms(delay);
			temp = PINC; /* useless data */
			_delay_ms(delay);
			temp = PINC; /* "(" character */
			_delay_ms(delay);
			while (1){
				temp = PINC; /* data character */
				_delay_ms(delay);
				if (temp == 0xA9) /* ")" character */
				{
					break;
				}
				temp = temp & 0x7F;
				data[address] = temp;
				address = address + 1;
			}
			temp = PINC; /* ETX character */
			_delay_ms(delay);
			temp = PINC; /* BCC character */
			_delay_ms(delay);
			return address;
		}
		if (temp == 0x95) /* NACK character */
		{
			_delay_ms(delay);
			Send_Read_message();
		}
	}
}

unsigned char Add_Parity2(unsigned char temp){

	  unsigned char count = 0;
	  unsigned char temp2 = temp;
	  unsigned char x ;
	  for (x = 0; x < 8; x++){
		  if (temp & 0x01)
		  {
			  count = count + 1;
		  }
		  temp = temp >> 1;
	  }
	  if (count & 0x01)
	  {
		  temp2 = temp2 | 0x80;
	  }
	  return temp2;

}
