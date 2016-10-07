#include "HardwareSerial.h"

HardwareSerial Serial(NULL, NULL, NULL, NULL, NULL, NULL);

HardwareSerial::HardwareSerial(
	volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
	volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
	volatile uint8_t *ucsrc, volatile uint8_t *udr) :
	_ubrrh(ubrrh), _ubrrl(ubrrl),
	_ucsra(ucsra), _ucsrb(ucsrb), _ucsrc(ucsrc),
	_udr(udr),
	_rx_buffer_head(0), _rx_buffer_tail(0),
	_tx_buffer_head(0), _tx_buffer_tail(0)
{

}

void HardwareSerial::begin(unsigned long, uint8_t)
{

}
void HardwareSerial::end()
{

}
int HardwareSerial::available(void)
{
	return 0;
}
int HardwareSerial::peek(void)
{
	return 0;
}
int HardwareSerial::read(void)
{
	return 0;
}
int HardwareSerial::availableForWrite(void)
{
	return 0;
}
void HardwareSerial::flush(void)
{

}
size_t HardwareSerial::write(uint8_t)
{
	return 0;
}
void HardwareSerial::_rx_complete_irq(void)
{

}
void HardwareSerial::_tx_udr_empty_irq(void)
{

}
