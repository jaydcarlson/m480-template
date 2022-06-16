#pragma once

#include "stdbool.h"
#include "stdint.h"

#define BULK_IN_EP_NUM      1
#define BULK_OUT_EP_NUM     2

#define EP0_MAX_PKT_SIZE    64
#define EP1_MAX_PKT_SIZE    EP0_MAX_PKT_SIZE
#define EP2_MAX_PKT_SIZE    64
#define EP3_MAX_PKT_SIZE    64

void USB_init(void);
void USB_trimTask(void);
bool USB_inEndpointIsBusy();
uint8_t* USB_getInEndpointBuffer();
void USB_sendData(int len);
