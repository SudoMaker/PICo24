/*
    This file is part of PICo24 SDK.

    Copyright (C) 2021 ReimuNotMoe <reimu@sudomaker.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdint.h>

#include "mini_unistd.h"

#include <PICo24_Board.h>
#include <PICo24/Peripherals/USB/Device/usb_deluxe_device.h>
#include <PICo24/Peripherals/UART/UART.h>

int STDIN_FILENO = -1;
int STDOUT_FILENO = -1;
int STDERR_FILENO = -1;

#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_ACM
static ssize_t write_cdc_acm(int fd, const void *buf, size_t count) {
	fd &= 0xf;
	uint8_t cdc_nr = 0;

	for (size_t i=0; i<usb_device_driver_ctx.size; i++) {
		if (USBDeluxe_DeviceGetDriverContext(i)->func == USB_FUNC_CDC_ACM) {
			if (fd == cdc_nr) {
				return USBDeluxeDevice_CDC_ACM_Write(USBDeluxe_DeviceGetDriverContext(i)->drv_ctx, (uint8_t *)buf, count);
			}

			cdc_nr++;
		}
	}

	return -1;
}

static ssize_t read_cdc_acm(int fd, void *buf, size_t count) {
	fd &= 0xf;
	uint8_t cdc_nr = 0;

	for (size_t i=0; i<usb_device_driver_ctx.size; i++) {
		if (USBDeluxe_DeviceGetDriverContext(i)->func == USB_FUNC_CDC_ACM) {
			if (fd == cdc_nr) {
				return USBDeluxeDevice_CDC_ACM_Read(USBDeluxe_DeviceGetDriverContext(i)->drv_ctx, (uint8_t *) buf, count);
			}

			cdc_nr++;
		}
	}

	return -1;
}
#endif

#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_NCM
static ssize_t write_cdc_ncm(int fd, const void *buf, size_t count) {
	fd &= 0xf;
	uint8_t cdc_nr = 0;

	for (size_t i=0; i<usb_device_driver_ctx.size; i++) {
		if (USBDeluxe_DeviceGetDriverContext(i)->func == USB_FUNC_CDC_NCM) {
			if (fd == cdc_nr) {
				return USBDeluxeDevice_CDC_NCM_Write(USBDeluxe_DeviceGetDriverContext(i)->drv_ctx, (uint8_t *)buf, count);
			}

			cdc_nr++;
		}
	}

	return -1;
}

static ssize_t read_cdc_ncm(int fd, void *buf, size_t count) {
	fd &= 0xf;
	uint8_t cdc_nr = 0;

	for (size_t i=0; i<usb_device_driver_ctx.size; i++) {
		if (USBDeluxe_DeviceGetDriverContext(i)->func == USB_FUNC_CDC_NCM) {
			if (fd == cdc_nr) {
				return USBDeluxeDevice_CDC_NCM_Read(USBDeluxe_DeviceGetDriverContext(i)->drv_ctx, (uint8_t *) buf, count);
			}

			cdc_nr++;
		}
	}

	return -1;
}
#endif

#ifdef PICo24_Enable_Peripheral_UART
static ssize_t write_uart(int fd, const void *buf, size_t count) {
	switch (fd) {
#ifdef PINFUNC_UART1_TX
		case UART1_FILENO:
			UART_Transmit(&huart1, (uint8_t *) buf, count);
			return count;
#endif
#ifdef PINFUNC_UART2_TX
		case UART2_FILENO:
			UART_Transmit(&huart2, (uint8_t *) buf, count);
			return count;
#endif
#ifdef PINFUNC_UART3_TX
		case UART3_FILENO:
			UART_Transmit(&huart3, (uint8_t *) buf, count);
			return count;
#endif
		default:
			break;
	}

	return -1;
}

static ssize_t read_uart(int fd, void *buf, size_t count) {
	switch (fd) {
#ifdef PINFUNC_UART1_TX
		case UART1_FILENO:
			return UART_Receive(&huart1, (uint8_t *)buf, count);
#endif
#ifdef PINFUNC_UART2_TX
		case UART2_FILENO:
			return UART_Receive(&huart2, (uint8_t *)buf, count);
#endif
#ifdef PINFUNC_UART3_TX
		case UART3_FILENO:
			return UART_Receive(&huart3, (uint8_t *)buf, count);
#endif
		default:
			return -1;
	}
}
#endif

ssize_t __attribute__((__section__(".libc"))) write(int fd, const void *buf, size_t count) {

	switch (fd) {
		case STDOUT_FILENO_STD:
			if (STDOUT_FILENO > 0) {
				fd = STDOUT_FILENO;
			}
			break;
		case STDERR_FILENO_STD:
			if (STDERR_FILENO > 0) {
				fd = STDERR_FILENO;
			}
			break;
		default:
			break;
	}

#ifdef PICo24_Enable_Peripheral_UART
	if (fd & UART_FILENO_MASK) {
		return write_uart(fd, buf, count);
	}
#endif

#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_ACM
	if (fd & CDC_ACM_FILENO_MASK) {
		return write_cdc_acm(fd, buf, count);
	}
#endif

#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_NCM
	if (fd & CDC_NCM_FILENO_MASK) {
		return write_cdc_ncm(fd, buf, count);
	}
#endif

	return -1;
}



ssize_t __attribute__((__section__(".libc"))) read(int fd, void *buf, size_t count) {
	switch (fd) {
		case STDIN_FILENO_STD:
			if (STDIN_FILENO > 0) {
				fd = STDIN_FILENO;
			}
			break;
		default:
			break;
	}

#ifdef PICo24_Enable_Peripheral_UART
	if (fd & UART_FILENO_MASK) {
		return read_uart(fd, buf, count);
	}
#endif

#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_ACM
	if (fd & CDC_ACM_FILENO_MASK) {
		return read_cdc_acm(fd, buf, count);
	}
#endif

#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_NCM
	if (fd & CDC_NCM_FILENO_MASK) {
		return read_cdc_ncm(fd, buf, count);
	}
#endif

	return -1;
}

void usleep(uint64_t usec) {
	Delay_Microseconds(usec);
}

void msleep(uint64_t msec) {
	Delay_Milliseconds(msec);
}

void sleep(uint32_t sec) {
	Delay_Milliseconds(sec * 1000);
}