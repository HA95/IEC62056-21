/*
 * Tariff.c
 *
 * Created: 8/21/2019 10:12:13 PM
 * Author : Armin
 */ 

void Read_Request_massage();
unsigned char Selected_mode(unsigned char temp);
void Send_Identity_massage(unsigned char mode);
void Send_Data_massage();
unsigned char Add_Parity(unsigned char temp);
unsigned char Read_Password_command(unsigned char pass);
void Send_Exit_command2();
unsigned int Read_Write_command(unsigned char* data);
void Read_Read_command(unsigned char* data);

#include <avr/io.h>
#define F_CPU 16000000UL
#define delay 2500
#include "util/delay.h"

int main(void)
{
	
	unsigned char temp2;
	unsigned char mode2;
	unsigned char pass = 0x7F;
	unsigned char check;
	unsigned char data_recieved2[1024];
	unsigned int address2 = 0;
	unsigned char exit_flag = 0;
    DDRA = 0x00; /* PA0-7 as data line input at first */
	DDRC = 0x00; /* PC0-7 as input Data */
	DDRD = 0x00; /* PD6-7 as input for Mode selection, PD0 as input to start transmission,...*/
	DDRB = 0x80; /* PB7 as end of transmission signal */

    while (1) 
    {
		while (1) /* check PC0 to start transmission */
		{
			temp2 = PIND;
			temp2 = temp2 & 0x01;
			temp2 = temp2 ^ 0x01;
			if (temp2 == 0x01)
			{
				break;
			}
		}
		temp2 = PIND;
		mode2 = Selected_mode(temp2);
		_delay_ms(10);
		Read_Request_massage();
		Send_Identity_massage(mode2);
		switch (mode2){
			
			case 0x2E: /* Mode A */
				Send_Data_massage();
				while(1){
					DDRA = 0x00;
					temp2 = PINA; /* SOH character --> Programming Command */
					_delay_ms(delay);
					temp2 = PINA;
					_delay_ms(delay);
					switch (temp2){
							
						case 0x42: /* "B" character */
							exit_flag = 1;
							break;
						case 0x50: /* "P" character */
							check = Read_Password_command(pass);
							if (check == 1)
							{
								DDRA = 0xFF;
								PORTA = 0x06; /* ACK message */
								_delay_ms(delay);
								PORTA = 0x00;
								DDRA = 0x00;
								_delay_ms(delay);
							} 
							if (check == 2)
							{
								DDRA = 0xFF;
								PORTA = 0x95; /* NACK message */
								_delay_ms(delay);
								PORTA = 0x00;
								DDRA = 0x00;
								_delay_ms(delay);
							}
							if (check == 0)
							{
								void Send_Exit_command2();
								exit_flag = 1;
							}
							break;
						case 0xD7: /* "W" character */
							address2 = Read_Write_command(data_recieved2);
							break;
						case 0xD2: /* "R" character */
							Read_Read_command(data_recieved2);
							break;
							
					}
					if (exit_flag == 1)
					{
						exit_flag = 0;
						break;
					}
				}
				break;
			case 0x41: /* Mode B */
				break;
			case 0x01: /* Mode C or E*/
				break;
		}
		PORTB = 0x80; /* end of transmission */
		_delay_ms(delay);
		PORTB = 0x00;
    }
}

void Read_Request_massage(){
	
	unsigned char temp;
	temp = PINA; /* "/" character */
	_delay_ms(delay);
	temp = PINA; /* "?" character */
	_delay_ms(delay);
	temp = PINA; /* "!" character */
	_delay_ms(delay);
	temp = PINA; /* CR character */
	_delay_ms(delay);
	temp = PINA; /* LF character */
	_delay_ms(delay);

}

unsigned char Selected_mode(unsigned char temp){
	
	unsigned char mode = 0;
	temp = temp & 0xC0;
	temp = temp ^ 0xC0;
	switch (temp){
		
		case 0x00:
			mode = 0x2E; /* Mode A */
			break;
		case 0x40:
			mode = 0x41; /* Mode B */
			break;
		case 0x80:
			mode = 0x81; /* Mode C */
			break;
		case 0xC0:
			mode = 0x81; /* Mode E */
			break;

	}
	return mode;

}

void Send_Identity_massage(unsigned char mode){

	DDRA = 0xFF; /* PA0-7 as data line output at first */
	PORTA = 0xAF; /* "/" character */
	_delay_ms(delay);
	PORTA = 0x0A; /* "X" character */
	_delay_ms(delay);
	PORTA = 0x0A; /* "X" character */
	_delay_ms(delay);
	PORTA = 0x0A; /* "X" character */
	_delay_ms(delay);
	PORTA = mode; /* "Z" character */
	_delay_ms(delay);
	PORTA = 0x8D; /* CR character */
	_delay_ms(delay);
	PORTA = 0x0A; /* LF character */
	_delay_ms(delay);

}

void Send_Data_massage(){
	
	unsigned char temp;
	unsigned char count = 0;
	while (1){
		temp = PINC;
		temp = temp ^ 0xFF;
		temp = Add_Parity(temp);
		PORTA = temp;
		_delay_ms(delay);
		if (temp == 0x21) /* "!" character */
		{
			break;
		}
		count = count + 1;
		if (count == 78)
		{
			PORTA = 0xA9; /* ")" character */
			_delay_ms(delay);
			PORTA = 0x8D; /* CR character */
			_delay_ms(delay);
			PORTA = 0x0A; /* LF character */
			_delay_ms(delay);	
			count = 0;
		}
	}
	PORTA = 0x8D; /* CR character */
	_delay_ms(delay);
	PORTA = 0x0A; /* LF character */
	_delay_ms(delay);
	PORTA = 0x00;
	DDRA = 0x00;
	_delay_ms(delay);

}

unsigned char Read_Password_command(unsigned char pass){
	
	unsigned char temp;
	unsigned char Password = 0;
	unsigned char count = 0;
	unsigned char BCC = 0x50; /* "P" character */
	temp = PINA; /* 1 character */
	BCC = BCC ^ temp;
	_delay_ms(delay);
	temp = PINA; /* STX character */
	BCC = BCC ^ temp;
	_delay_ms(delay);
	temp = PINA; /* "(" character */
	BCC = BCC ^ temp;
	_delay_ms(delay);
	while (1){
		
		temp = PINA; 
		BCC = BCC ^ temp;
		_delay_ms(delay);
		if (temp == 0xA9)
		{
			break;
		}
		temp = temp & 0x7F;
		if (count == 0)
		{
			Password = temp;
		}
		count = count + 1;

	}
	temp = PINA; /* ETX character */
	BCC = BCC ^ temp;
	_delay_ms(delay); 
	BCC = BCC & 0x7F;
	BCC = Add_Parity(BCC);
	temp = PINA; /* BCC character */ 
	_delay_ms(delay); 
	if (BCC != temp)
	{
		return 2;
	}
	if (Password == pass)
	{
		return 1;
	} 
	else
	{
		return 0;
	}
	
}

void Send_Exit_command2(){
	
	unsigned char BCC;
	DDRA = 0xFF;
	PORTA = 0x81; /* SOH character */
	_delay_ms(delay);
	PORTA = 0x42; /* "B" character */
	BCC = 0x42;
	_delay_ms(delay);
	PORTA = 0x00; /* 0 character */
	BCC = BCC ^ 0x00;
	_delay_ms(delay);
	PORTA = 0x03; /* ETX character */
	BCC = BCC ^ 0x03;
	_delay_ms(delay);
	BCC = BCC & 0x7F;
	BCC = Add_Parity(BCC);
	PORTA = BCC; /* BCC character */

}

unsigned int Read_Write_command(unsigned char* data){

	unsigned char temp;
	unsigned char BCC = 0xD7;
	unsigned char count = 3;
	unsigned int address;
	unsigned char check = 0; /* ACK or NACK flag */
	temp = PINA; /* 1 character */
	BCC = BCC ^ temp;
	_delay_ms(delay);
	temp = PINA; /* STX character */
	BCC = BCC ^ temp;
	_delay_ms(delay);
	temp = PINA; /* upper 8 bits of address */
	BCC = BCC ^ temp;
	temp = temp & 0x7F;
	address = temp << 8;
	_delay_ms(delay);
	temp = PINA; /* lower 8 bits of address */
	BCC = BCC ^ temp;
	temp = temp & 0x7F;
	address = address + temp;
	_delay_ms(delay);
	temp = PINA; /* "(" character */
	BCC = BCC ^ temp;
	_delay_ms(delay);
	if (temp != 0x28)
	{
		check = 1;
	}
	while (1){
		count = count + 1;
		temp = PINA;
		BCC = BCC ^ temp;
		_delay_ms(delay);
		if (temp == 0xA9) /* ")" character */
		{
			break;
		}
		if (count == 78)
		{
			check = 1;
			break;
		}
		temp = temp & 0x7F; /* setting parity bit to zero --> right data */
		data[address] = temp;
		address = address + 1;
	}
	temp = PINA; /* ETX character */
	BCC = BCC ^ temp;
	_delay_ms(delay);
	if (temp != 0x03)
	{
		check = 1;
	}
	temp = PINA; /* BCC character */
	_delay_ms(delay);
	BCC = BCC & 0x7F;
	BCC = Add_Parity(BCC);
	if (temp != BCC)
	{
		check = 1;
	}
	DDRA = 0xFF; /* PA0-7 as data line output */
	if (check == 0)
	{
		PORTA = 0x06; /* ACK message */
	}
	else{
		PORTA = 0x95; /* NACK message */
	}
	_delay_ms(delay);
	PORTA = 0x00;
	DDRA = 0x00;
	_delay_ms(delay);
	return address;

}

void Read_Read_command(unsigned char* data){

	unsigned char BCC = 0xD2;
	unsigned char temp, address_low, address_high;	
	unsigned int address;
	unsigned char num_of_units;
	unsigned char check = 0; /* ACK or NACK flag */
	temp = PINA; /* 1 character */
	BCC = BCC ^ temp;
	_delay_ms(delay);
	temp = PINA; /* STX character */
	BCC = BCC ^ temp;
	_delay_ms(delay);
	temp = PINA; /* upper 8 bits of address */
	BCC = BCC ^ temp;
	temp = temp & 0x7F;
	address_high = temp;
	address = temp << 8;
	_delay_ms(delay);
	temp = PINA; /* lower 8 bits of address */
	BCC = BCC ^ temp;
	temp = temp & 0x7F;
	address_low = temp;
	address = address + temp;
	_delay_ms(delay);
	temp = PINA; /* "(" character */
	BCC = BCC ^ temp;
	_delay_ms(delay);
	if (temp != 0x28)
	{
		check = 1;
	}
	temp = PINA;
	BCC = BCC ^ temp;
	temp = temp & 0x7F;
	num_of_units = temp; /* number of characters to be read */
	_delay_ms(delay);
	temp = PINA; /* ")" character */
	BCC = BCC ^ temp;
	_delay_ms(delay);
	if (temp != 0xA9)
	{
		check = 1;
	}
	temp = PINA; /* ETX character */
	BCC = BCC ^ temp;
	BCC = BCC & 0x7F;
	BCC = Add_Parity(BCC);
	_delay_ms(delay);
	if (temp != 0x03)
	{
		check = 1;
	}
	temp = PINA; /* BCC character */
	_delay_ms(delay);
	if (temp != BCC)
	{
		check = 1;
	}
	if (check == 0)
	{
		DDRA = 0xFF; /* PA0-7 as data line output */
		PORTA = 0x82; /* STX character */
		_delay_ms(delay);
		PORTA = address_high; /* data character */
		BCC = address_high;
		_delay_ms(delay);
		PORTA = address_low; /* data character */
		BCC = BCC ^ address_low;
		_delay_ms(delay);
		PORTA = 0x28; /* "(" character */
		BCC = BCC ^ 0x28;
		_delay_ms(delay);
		for (temp = 0; temp < num_of_units; temp++)
		{
			temp = data[address];
			temp = Add_Parity(temp);
			PORTA = temp; /* data character */
			BCC = BCC ^ data[address];
			address = address + 1;
			_delay_ms(delay);
		}
		PORTA = 0xA9; /* ")" character */
		BCC = BCC ^ 0xA9;
		_delay_ms(delay);
		PORTA = 0x03; /* ETX character */
		BCC = BCC ^ 0x03;
		BCC = BCC & 0x7F;
		BCC = Add_Parity(BCC);
		_delay_ms(delay);
		PORTA = BCC; /* BCC character */
		_delay_ms(delay);
		PORTA = 0x00;
		DDRA = 0x00;
		_delay_ms(delay);
	} 
	else
	{
		DDRA = 0xFF; /* PA0-7 as data line output */
		PORTA = 0x95; /* NACK character */
		_delay_ms(delay);
		PORTA = 0x00;
		DDRA = 0x00;
		_delay_ms(delay);
	}

}

unsigned char Add_Parity(unsigned char temp){

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

