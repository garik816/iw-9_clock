#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>

#define srClk PB0 // serial clock
#define srDataIn PB1	//serial data-in
#define rClk0 PC0 // register clock rank 0
#define rClk1 PC1 // register clock rank 1
#define rClk2 PC2 // register clock rank 2
#define rClk3 PC3 // register clock rank 3

#define delayTime 2

uint8_t digits[] = {
		0b11111101,	//0b00111111, //"0" //������ � �������� �������
		0b01100000,	//0b00000110, //"1"
		0b11011010,	//0b01011011, //"2"
		0b11110010,	//0b01001111, //"3"
		0b01100110,	//0b01100110, //"4"
		0b10110110,	//0b01101101, //"5"
		0b10111111,	//0b01111101, //"6"
		0b11100000,	//0b00000111, //"7"
		0b11111110,	//0b01111111, //"8"
		0b11110110	//0b01101111  //"9"
	};


// ������� ������������� ���� TWI
void I2CInit(void)
{
	TWBR = 2; // ��������� ������� ����
	TWSR = (1 << TWPS1)|(1 << TWPS0); // ������������ �� 64
	TWCR |= (1 << TWEN); // ��������� ������ TWI
}

// ������� �����
void I2CStart(void)
{
	TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWSTA); // �������� ������� �����
	while(!(TWCR & (1 << TWINT))); // �������� ��������� ����� TWINT
}

// ������� ����
void I2CStop(void)
{
	TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWSTO); // �������� ������� ����
	while(TWCR & (1 << TWSTO)); // �������� ���������� �������� ������� ����
}

// ������� ������ ������ �� ����
uint8_t I2CWriteByte(uint8_t data)
{
	TWDR = data; // �������� ������ � TWDR
	TWCR = (1 << TWEN)|(1 << TWINT); // ����� ����� TWINT ��� ������ �������� ������
	while(!(TWCR & (1 << TWINT))); // �������� ��������� ����� TWINT
	// �������� �������
	// ���� ����� DS1307+R � ������� "�������������"(0x18)
	// ��� ����� DS1307+W � ������� "�������������"(0x40)
	// ��� ���������� ���� ������ � ������� "�������������"(0x28)
		if((TWSR & 0xF8) == 0x18 || (TWSR & 0xF8) == 0x40 || (TWSR & 0xF8) == 0x28) return 1; // OK
	else return 0; // ������
}

// ������� ������ ������ �� ����
uint8_t I2CReadByte(uint8_t *data,uint8_t ack)
{
	// ���������� "�������������" ����� ������
	if(ack) TWCR |= (1 << TWEA);
	// ���������� "���������������" ����� ������
	// ������� ���������� �� �������� ������ ������
	// ������ ������������ ��� ������������� ���������� �����
	else TWCR &= ~(1 << TWEA);
	// ���������� ������ ������ ����� ������ TWINT
	TWCR |= (1 << TWINT);
	while(!(TWCR & (1 << TWINT))); // �������� ��������� ����� TWINT
	// �������� �������
	// ���� ������ ���� ������ � ������������ "�������������"(0x50)
	// ��� ������ ���� ������ � ������������ "�����������������"(0x58)
	if((TWSR & 0xF8) == 0x50 || (TWSR & 0xF8) == 0x58)
	{
		*data = TWDR; // ������ ������ �� TWDR
		return 1; // OK
	}
	else return 0; // ������
}

// ������� ������ ������ �� DS1307
uint8_t DS1307Read(uint8_t address,uint8_t *data)
{
	uint8_t res;
	I2CStart(); // �����
	res = I2CWriteByte(0b11010000); // ����� DS1307+W
	if(!res)    return 0; // ������
		// �������� ������ ������������ ��������
		res = I2CWriteByte(address);
	if(!res)    return 0; // ������
		I2CStart(); // ��������� �����
		res = I2CWriteByte(0b11010001); // ����� DS1307+R
	if(!res)    return 0; // ������
		// ������ ������ � "����������������"
		res = I2CReadByte(data,0);
	if(!res)    return 0; // ������
		I2CStop(); // ����
	return 1; // OK
}

// ������� ������ ������ � DS1307
uint8_t DS1307Write(uint8_t address,uint8_t data)
{
	uint8_t res;
	I2CStart(); // �����
	res = I2CWriteByte(0b11010000); // ����� DS1307+W
	if(!res)    return 0; // ������
		// �������� ������ ������������ ��������
		res = I2CWriteByte(address);
	if(!res)    return 0; // ������
		res = I2CWriteByte(data); // ������ ������
	if(!res)    return 0; // ������
		I2CStop(); // ����
	return 1; // OK
}

// ������� ������ � LCD
#define RS PD0
#define EN PD2
// ������� �������� �������
void lcd_com(unsigned char p)
{
	PORTD &= ~(1 << RS); // RS = 0 (������ ������)
	PORTD |= (1 << EN); // EN = 1 (������ ������ ������� � LCD)
	PORTD &= 0x0F; PORTD |= (p & 0xF0); // ������� ����
	_delay_us(100);
	PORTD &= ~(1 << EN); // EN = 0 (����� ������ ������� � LCD)
	_delay_us(100);
	PORTD |= (1 << EN); // EN = 1 (������ ������ ������� � LCD)
	PORTD &= 0x0F; PORTD |= (p << 4); // ������� ����
	_delay_us(100);
	PORTD &= ~(1 << EN); // EN = 0 (����� ������ ������� � LCD)
	_delay_us(100);
}
// ������� �������� ������
void lcd_data(unsigned char p)
{
	PORTD |= (1 << RS)|(1 << EN); // RS = 1 (������ ������), EN - 1 (������ ������ ������� � LCD)
	PORTD &= 0x0F; PORTD |= (p & 0xF0); // ������� ����
	_delay_us(100);
	PORTD &= ~(1 << EN); // EN = 0 (����� ������ ������� � LCD)
	_delay_us(100);
	PORTD |= (1 << EN); // EN = 1 (������ ������ ������� � LCD)
	PORTD &= 0x0F; PORTD |= (p << 4); // ������� ����
	_delay_us(100);
	PORTD &= ~(1 << EN); // EN = 0 (����� ������ ������� � LCD)
	_delay_us(100);
}

// ������� ������ ������ �� LCD
void lcd_string(unsigned char command, char *string)
{
	lcd_com(0x0C);
	lcd_com(command);
	while(*string != '\0')
	{	lcd_data(*string);
		string++;
	}
}

// ������� ������ ����������
void lcd_num_to_str(unsigned int value, unsigned char nDigit)
{
 switch(nDigit)
 {
  case 4: lcd_data((value/1000)+'0');
  case 3: lcd_data(((value/100)%10)+'0');
  case 2: lcd_data(((value/10)%10)+'0');
  case 1: lcd_data((value%10)+'0');
 }
}

// ������� ������������� LCD
void lcd_init(void)
{
	PORTD = 0x00;
	DDRD = 0xFF;

	_delay_ms(50); // �������� ���������� ��-������

	// ���������������� ����������������� ������
	PORTD |= (1 << PD5);
	PORTD &= ~(1 << PD4);

	// ����������� ����������������� ������
	PORTD |= (1 << EN);
	PORTD &= ~(1 << EN);
	_delay_ms(5);

	lcd_com(0x28); // ���� 4 ���, LCD - 2 ������
	lcd_com(0x08); // ������ ���������� �������
	lcd_com(0x01); // ������� �������
	_delay_us(100);
	lcd_com(0x06); // ����� ������� ������
	lcd_com(0x0C); // ��������� �������, ������ �� �����
}

void output7seg(uint8_t digit, uint8_t rank) {
	PORTC &= ~(1 << rank);

	for (int i = 0; i < 8; i++) {
		uint8_t dataBit = (((digit & (1 << i)) >> i));

		PORTB &= ~(1 << srClk);

		if (dataBit)
			PORTB |= (1 << srDataIn);
		else
			PORTB &= ~(1 << srDataIn);

		PORTB |= (1 << srClk);
		PORTB &= ~(1 << srDataIn);
	}
	PORTC |= (1 << rank);
}

int main(void) {

	DDRB |= ((1 << srDataIn) | (1 << srClk));
	DDRC |= ((1 << rClk0) | (1 << rClk1) | (1 << rClk2) | (1 << rClk3));
	DDRC &= (~((1<<PC4) | (1<<PC5)));
	PORTB = 0xFF;
	PORTC |= ((1 << PC4) | (1 << PC5));

	lcd_init(); // ������������� LCD
	I2CInit(); // ������������� ���� I2C

	lcd_string(0x81, "clock DS1307"); // ���� �� DS1307
	lcd_string(0xC4, "  :  :  ");

	// ��������� ��� �����
	uint8_t temp;
	DS1307Read(0x00,&temp);
	temp &= ~(1 << 7); // �������� 7 ���
	DS1307Write(0x00,temp);

	while(1) {

//		//�������
//		for (int i=0; i<10; i++) {
//			output7seg(digits[i],rClk0);
//			_delay_ms(delayTime);
//			for (int i=0; i<10; i++) {
//				output7seg(digits[i],rClk1);
//				_delay_ms(delayTime);
//				for (int i=0; i<10; i++) {
//					output7seg(digits[i],rClk2);
//					_delay_ms(delayTime);
//					for (int i=0; i<10; i++) {
//						output7seg(digits[i],rClk3);
//						_delay_ms(delayTime);
//					}
//				}
//			}
//		}
//		//end �������


		unsigned char hour, minute, second, temp;
		// ������ ������ � ����������� �� BCD � �������� �������
		DS1307Read(0x00,&temp); // ������ �������� ������
		second = (((temp & 0xF0) >> 4)*10)+(temp & 0x0F);
		DS1307Read(0x01,&temp); // ������ �������� �����
		minute = (((temp & 0xF0) >> 4)*10)+(temp & 0x0F);
		DS1307Read(0x02,&temp); // ������ �������� �����
		hour = (((temp & 0xF0) >> 4)*10)+(temp & 0x0F);

		lcd_com(0xC4);
		lcd_num_to_str(hour, 2); // ������� �� ����� ����
		lcd_com(0xC7);
		lcd_num_to_str(minute, 2); // ������� �� ����� ������
		lcd_com(0xCA);
		lcd_num_to_str(second, 2); // ������� �� ����� �������

//		case 4: lcd_data((value/1000)+'0');
//		case 3: lcd_data(((value/100)%10)+'0');
//		case 2: lcd_data(((value/10)%10)+'0');
//		case 1: lcd_data((value%10)+'0');

		output7seg(digits[hour/10%10],rClk3);
		output7seg(digits[hour%10],rClk2);
		output7seg(digits[minute/10%10],rClk1);
		output7seg(digits[minute%10],rClk0);

//		if((PINC & (1 << PC0)) == 0) // ���� ������ ������
//		{
//			while((PINC & (1 << PC0)) == 0){} // ���� ���������� ������
//			hour++; // ����������� ���� �� 1
//			if(hour > 23) hour = 0;
//			// ����������� �� �������� ������� � BCD � ���������� � DS1307
//			uint8_t temp;
//			temp = ((hour/10) << 4)|(hour%10);
//			DS1307Write(0x02, temp);
//			_delay_ms(100);
//		}
//
//		if((PINC & (1 << PC1)) == 0) // ���� ������ ������
//		{
//			while((PINC & (1 << PC1)) == 0){} // ���� ���������� ������
//			minute++; // ����������� ������ �� 1
		if(minute > 59) minute = 0;{
			// ����������� �� �������� ������� � BCD � ���������� � DS1307
			uint8_t temp;
			temp = ((minute/10) << 4)|(minute%10);
			DS1307Write(0x01, temp);
			_delay_ms(100);
		}
	}

	return 0;
}
