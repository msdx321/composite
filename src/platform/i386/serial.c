#define ENABLE_SERIAL

#include "string.h"
#include "chal/io.h"
#include "isr.h"
#include "kernel.h"

void serial_puts(const char *s);

enum serial_ports
{
	SERIAL_PORT_A = 0x3F8,
	SERIAL_PORT_B = 0x2F8,
	SERIAL_PORT_C = 0x3E8,
	SERIAL_PORT_D = 0x2E8
};

static inline char
serial_recv(void)
{
	if ((inb(SERIAL_PORT_A + 5) & 1) == 0) return '\0';
	return inb(SERIAL_PORT_A);
}

static inline void
serial_send(char out)
{
	while ((inb(SERIAL_PORT_A + 5) & 0x20) == 0) {
		/* wait for port to be ready to send */
	}
	outb(SERIAL_PORT_A, out);
}

void
serial_puts(const char *s)
{
#ifdef ENABLE_SERIAL
	for (; *s != '\0'; s++) serial_send(*s);
#endif
}

void
chal_serial_putb(const void *d, int len)
{
#ifdef ENABLE_SERIAL
	int i = 0;

	for (; i < len; i++) serial_send(*(char *)(d+i));
#endif
}

int
serial_handler(struct pt_regs *r)
{
	char serial;
	int  preempt = 1;

	lapic_ack();

	serial = serial_recv();

	/*
	 * Fix the serial input assuming it is ascii
	 */
	switch (serial) {
	case '\0':
		return preempt;
	case 127:
		serial = 0x08;
		break;
	case 13:
		serial = '\n';
		break;
	case 3: /* FIXME: Obviously remove this once we have working components */
		die("Break\n");
	case 'o':
		hpet_set(HPET_ONESHOT, 50000000);
		hpet_set(HPET_ONESHOT, 50000000);
		break;
	case 'p':
		hpet_set(HPET_PERIODIC, 100000000);
		hpet_set(HPET_PERIODIC, 100000000);
		break;
	default:
		break;
	}

//	PRINTK("Serial: %c\n", serial);

	return preempt;
}

void
serial_init(void)
{
	printk_register_handler(PRINTK_SERIAL, serial_puts);

	/* We will initialize the first serial port */
	outb(SERIAL_PORT_A + 1, 0x00);
	outb(SERIAL_PORT_A + 3, 0x80); /* Enable divisor mode */
	//outb(SERIAL_PORT_A + 0, 0x03); /* Div Low:  03 Set the port to 38400 bps */
	outb(SERIAL_PORT_A + 0, 0x01); /* Div Low:  01 Set the port to 115200 bps */
	outb(SERIAL_PORT_A + 1, 0x00); /* Div High: 00 */
	outb(SERIAL_PORT_A + 3, 0x03);
	outb(SERIAL_PORT_A + 2, 0xC7);
	outb(SERIAL_PORT_A + 4, 0x0B);

	outb(SERIAL_PORT_A + 1, 0x01); /* Enable interrupts on receive */
	printk("Enabling serial I/O\n");
}

void
serial_late_init(void)
{
//	chal_irq_enable(HW_SERIAL, 0);
	chal_irq_enable(HW_KEYBOARD, 0);
}
