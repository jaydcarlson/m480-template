/***************************************************************************//**
 * @file     descriptors.c
 * @brief    M480 series USBD driver source file
 * @version  2.0.0
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
/*!<Includes */
#include "NuMicro.h"
#include "usbd.h"
#include "usb_vendor.h"

#define USBD_VID        0x0416
#define USBD_PID        0x502D

#define VENDOR_STRING   "VIC"
#define PRODUCT_STRING  "Template Project"

/*-------------------------------------------------------------*/
/* Define EP maximum packet size */


#define SETUP_BUF_BASE      0
#define SETUP_BUF_LEN       8
#define EP0_BUF_BASE        (SETUP_BUF_BASE + SETUP_BUF_LEN)
#define EP0_BUF_LEN         EP0_MAX_PKT_SIZE
#define EP1_BUF_BASE        (SETUP_BUF_BASE + SETUP_BUF_LEN)
#define EP1_BUF_LEN         EP1_MAX_PKT_SIZE
#define EP2_BUF_BASE        (EP1_BUF_BASE + EP1_BUF_LEN)
#define EP2_BUF_LEN         EP2_MAX_PKT_SIZE
#define EP3_BUF_BASE        (EP2_BUF_BASE + EP2_BUF_LEN)
#define EP3_BUF_LEN         EP3_MAX_PKT_SIZE


/* Define the interrupt In EP number */



/* Define Descriptor information */
#define USBD_SELF_POWERED               0
#define USBD_REMOTE_WAKEUP              0
#define USBD_MAX_POWER                  50  /* The unit is in 2mA. ex: 50 * 2mA = 100mA */



#define LE16(x)  (x & 0x00FF), ((x & 0xFF00) >> 8)

// Private module functions
void USB_dataReceived(uint8_t* data, int len);

/*----------------------------------------------------------------------------*/
/*!<USB Device Descriptor */
uint8_t gu8DeviceDescriptor[] =
{
    LEN_DEVICE,     /* bLength */
    DESC_DEVICE,    /* bDescriptorType */
    LE16(0x0200),     /* bcdUSB */ 
    0xFF,           /* bDeviceClass */
    0x00,           /* bDeviceSubClass */
    0x00,           /* bDeviceProtocol */
    64,   /* bMaxPacketSize0 */
    /* idVendor */
    LE16(USBD_VID),
    /* idProduct */
    LE16(USBD_PID),
    0x00, 0x03,     /* bcdDevice */
    0x01,           /* iManufacture */
    0x02,           /* iProduct */
    0x03,           /* iSerialNumber - no serial */
    0x01            /* bNumConfigurations */
};

/*!<USB Configure Descriptor */
uint8_t gu8ConfigDescriptor[] =
{
    LEN_CONFIG,     /* bLength              */
    DESC_CONFIG,    /* bDescriptorType      */
    LE16(32),     /* wTotalLength         */
    0x01,           /* bNumInterfaces       */
    0x01,           /* bConfigurationValue  */
    0x00,           /* iConfiguration       */
    0xC0,           /* bmAttributes         */
    0x32,           /* MaxPower             */

    /* INTERFACE descriptor */
    LEN_INTERFACE,  /* bLength              */
    DESC_INTERFACE, /* bDescriptorType      */
    0x00,           /* bInterfaceNumber     */
    0x00,           /* bAlternateSetting    */
    0x02,           /* bNumEndpoints        */
    0xFF,           /* bInterfaceClass      */
    0xFF,           /* bInterfaceSubClass   */
    0xFF,           /* bInterfaceProtocol   */
    0x00,           /* iInterface           */

    /* ENDPOINT descriptor */
    LEN_ENDPOINT,                   /* bLength          */
    DESC_ENDPOINT,                  /* bDescriptorType  */
    (EP_INPUT | BULK_IN_EP_NUM),    /* bEndpointAddress */
    EP_BULK,                        /* bmAttributes     */
    EP2_MAX_PKT_SIZE, 0x00,         /* wMaxPacketSize   */
    0x00,                           /* bInterval        */

    /* ENDPOINT descriptor */
    LEN_ENDPOINT,                   /* bLength          */
    DESC_ENDPOINT,                  /* bDescriptorType  */
    (EP_OUTPUT | BULK_OUT_EP_NUM),  /* bEndpointAddress */
    EP_BULK,                        /* bmAttributes     */
    EP3_MAX_PKT_SIZE, 0x00,         /* wMaxPacketSize   */
    0x00,                           /* bInterval        */
};

/*!<USB Language String Descriptor */
uint8_t gu8StringLang[4] =
{
    4,              /* bLength */
    DESC_STRING,    /* bDescriptorType */
    0x09, 0x04
};



/*!<USB BOS Descriptor */
const uint8_t gu8BOSDescriptor[] =
{
    LEN_BOS,        /* bLength */
    DESC_BOS,       /* bDescriptorType */
    /* wTotalLength */
    0x0C & 0x00FF,
    ((0x0C & 0xFF00) >> 8),
    0x01,           /* bNumDeviceCaps */

    /* Device Capability */
    0x7,            /* bLength */
    DESC_CAPABILITY,/* bDescriptorType */
    CAP_USB20_EXT,  /* bDevCapabilityType */
    0x02, 0x00, 0x00, 0x00  /* bmAttributes */
};

char serialNumber[64];

char const* string_descriptors [] =
{
  (const char[]) { 0x09, 0x04 }, // 0: is supported language is English (0x0409)
  VENDOR_STRING,                         // 1: Manufacturer
  PRODUCT_STRING,                // 2: Product
  serialNumber,
  NULL
};

uint8_t *gu8UsbHidReport[3] =
{
    NULL,
    NULL,
    NULL,
};

uint32_t gu32UsbHidReportLen[3] =
{
    0,
    0,
    0,
};

uint32_t gu32ConfigHidDescIdx[3] =
{
    0,
    0,
    0,
};

const S_USBD_INFO_T gsInfo =
{
    (uint8_t *)gu8DeviceDescriptor,
    (uint8_t *)gu8ConfigDescriptor,
    (uint8_t **)string_descriptors,
    (uint8_t **)gu8UsbHidReport,
    (uint8_t *)gu8BOSDescriptor,
    (uint32_t *)gu32UsbHidReportLen,
    (uint32_t *)gu32ConfigHidDescIdx
};



/**************************************************************************//**
 * @file     vcom_serial.c
 * @brief    M480 series USBD driver Sample file
 * @version  2.0.0
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

/*!<Includes */
#include <string.h>
#include "NuMicro.h"

// crystalless trim stuff
#define TRIM_INIT           (SYS_BASE+0x10C)
uint32_t u32TrimInit;

uint32_t volatile g_u32OutToggle = 0;


static bool inEndpointBusy = 0;

// WinUSB stuff
#define	GET_MS_DESCRIPTOR		  0xEE			///< Standard setup request for Microsoft
#define EXTENDED_COMPAT_ID		  0x04
#define EXTENDED_PROPERTIES		  0x05
uint8_t msCompatDesc[] = {
	0x28, 0x00, 00, 0x00,
	0x00, 0x01,
	0x04, 0x00,
	0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
	0x01,
	0x57, 0x49, 0x4E, 0x55, 0x53, 0x42, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

typedef struct _MS_EXT_PROPERTY_FEATURE_DESC {
	uint32_t dwLength;
	uint16_t bcdVersion;
	uint16_t wIndex;
	uint16_t wCount;
	uint32_t dwSize;
	uint32_t dwPropertyDataType;
	uint16_t wPropertyNameLength;
	uint16_t bPropertyName[20];
	uint32_t dwPropertyDataLength;
	uint16_t bPropertyData[39];
} MS_EXT_PROPERTY_FEATURE_DESC;

uint8_t ExtPropertyFeatureDescriptor[] = {
	//----------Header Section--------------
	0x92, 0x00, 0x00, 0x00,//dwLength
	0x00, 0x01,//bcdVersion = 1.00
	0x05, 0x00,//wIndex
	0x01, 0x00,//wCount - 0x0001 "Property Sections" implemented in this descriptor
	//----------Property Section 1----------
	0x88, 0x00, 0x00, 0x00,//dwSize - 136 bytes in this Property Section
	0x07, 0x00, 0x00, 0x00 ,//dwPropertyDataType (Unicode REG_MULTI_SZ string)
	0x2a, 0x00,//wPropertyNameLength - 40 bytes in the bPropertyName field
	'D',0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00, 'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00,'s', 0x00, 0x00, 0x00,
	//bPropertyName - "DeviceInterfaceGUID"
	0x50, 0x00, 0x00, 0x00,//dwPropertyDataLength - 78 bytes in the bPropertyData field (GUID value in UNICODE formatted string, with braces and dashes)
	'{', 0x00, '7', 0x00, 'B', 0x00, '3', 0x00, '4', 0x00, 'B', 0x00, '3', 0x00, '8', 0x00, 'B', 0x00, '-', 0x00, 'F', 0x00, '4', 0x00, 'C', 0x00, 'D', 0x00, '-', 0x00, '4', 0x00, '9', 0x00, 'C', 0x00, '3', 0x00, '-', 0x00, 'B', 0x00, '2', 0x00, 'B', 0x00, 'B', 0x00, '-', 0x00, '6', 0x00, '0', 0x00, 'E', 0x00, '4', 0x00, '7', 0x00, 'A', 0x00, '4', 0x00, '3', 0x00, 'E', 0x00, '1', 0x00, '2', 0x00, 'D', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00//bPropertyData - this is the actual GUID value.  Make sure this matches the PC application code trying to connect to the device.
};


/*--------------------------------------------------------------------------*/
void USBD_IRQHandler(void)
{
    uint32_t u32IntSts = USBD_GET_INT_FLAG();
    uint32_t u32State = USBD_GET_BUS_STATE();

//------------------------------------------------------------------
    if (u32IntSts & USBD_INTSTS_FLDET)
    {
        // Floating detect
        USBD_CLR_INT_FLAG(USBD_INTSTS_FLDET);

        if (USBD_IS_ATTACHED())
        {
            /* USB Plug In */
            USBD_ENABLE_USB();
        }
        else
        {
            /* USB Un-plug */
            USBD_DISABLE_USB();
        }
    }

//------------------------------------------------------------------
    if (u32IntSts & USBD_INTSTS_BUS)
    {
        /* Clear event flag */
        USBD_CLR_INT_FLAG(USBD_INTSTS_BUS);

        if (u32State & USBD_STATE_USBRST)
        {
            /* Bus reset */
            USBD_ENABLE_USB();
            USBD_SwReset();
        }
        if (u32State & USBD_STATE_SUSPEND)
        {
            /* Enable USB but disable PHY */
            USBD_DISABLE_PHY();
        }
        if (u32State & USBD_STATE_RESUME)
        {
            /* Enable USB and enable PHY */
            USBD_ENABLE_USB();
        }
    }

//------------------------------------------------------------------
    if(u32IntSts & USBD_INTSTS_WAKEUP)
    {
        /* Clear event flag */
        USBD_CLR_INT_FLAG(USBD_INTSTS_WAKEUP);
    }

    if (u32IntSts & USBD_INTSTS_USB)
    {
        // USB event
        if (u32IntSts & USBD_INTSTS_SETUP)
        {
            // Setup packet
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_SETUP);

            /* Clear the data IN/OUT ready flag of control end-points */
            USBD_STOP_TRANSACTION(EP0);
            USBD_STOP_TRANSACTION(EP1);

            USBD_ProcessSetupPacket();
        }

        // EP events
        if (u32IntSts & USBD_INTSTS_EP0)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP0);

            // control IN
            USBD_CtrlIn();
        }

        if (u32IntSts & USBD_INTSTS_EP1)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP1);

            // control OUT
            USBD_CtrlOut();
        }

        if (u32IntSts & USBD_INTSTS_EP2)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP2);
            // Bulk IN
            inEndpointBusy = FALSE;
        }

        if (u32IntSts & USBD_INTSTS_EP3)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP3);
            // Bulk Out
            int rxSize = USBD_GET_PAYLOAD_LEN(EP3);
            uint8_t* rxBuf = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP3));
            USB_dataReceived(rxBuf, rxSize);
            USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);
        }

        if (u32IntSts & USBD_INTSTS_EP4)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP4);
        }

        if (u32IntSts & USBD_INTSTS_EP5)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP5);
        }

        if (u32IntSts & USBD_INTSTS_EP6)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP6);
        }

        if (u32IntSts & USBD_INTSTS_EP7)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP7);
        }
    }
}

__attribute__((weak))
void USB_dataReceived(uint8_t* data, int len)
{
    // do something with the RX data here

}

 extern uint8_t g_usbd_SetupPacket[];

// this should be moved back into usb.c
void handleVendorRequest()
{
    int length;
    if (g_usbd_SetupPacket[0] == 0xC0) {
		if (g_usbd_SetupPacket[4] == EXTENDED_COMPAT_ID) {
			length = 40;
			if (g_usbd_SetupPacket[6] < length) {
				length = g_usbd_SetupPacket[6];
			}
            // length = USBD_Minimum(length, (uint32_t)LEN_DEVICE);
            USBD_PrepareCtrlIn(msCompatDesc, length);
            // USBD_MemCopy((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)), msCompatDesc, length);
            // /* Data stage */
            // USBD_SET_DATA1(EP0);
            // USBD_SET_PAYLOAD_LEN(EP0, length);
            // /* Status stage */
            USBD_PrepareCtrlOut(0,0);

		}

	} else if (g_usbd_SetupPacket[0] == 0xC1) {
		 if (g_usbd_SetupPacket[4] == EXTENDED_PROPERTIES) {
			length = sizeof(ExtPropertyFeatureDescriptor);
			if (g_usbd_SetupPacket[6] < length) {
				length = g_usbd_SetupPacket[6];
			}
            // length = USBD_Minimum(length, (uint32_t)LEN_DEVICE);
            USBD_PrepareCtrlIn(ExtPropertyFeatureDescriptor, length);
            // USBD_MemCopy((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)), ExtPropertyFeatureDescriptor, length);
            // /* Data stage */
            // USBD_SET_DATA1(EP0);
            // USBD_SET_PAYLOAD_LEN(EP0, length);
            // /* Status stage */
            // USBD_PrepareCtrlOut(0,0);
		 }
	}
}

bool USB_inEndpointIsBusy()
{
    return inEndpointBusy;
}

uint8_t* USB_getInEndpointBuffer()
{
    return (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2));
}

void USB_sendData(int len)
{
    inEndpointBusy = TRUE;
    USBD_SET_PAYLOAD_LEN(EP2, len);

}

/*--------------------------------------------------------------------------*/
/**
  * @brief  USBD Endpoint Config.
  * @param  None.
  * @retval None.
  */
void USB_init(void)
{
    SYS_UnlockReg();

    /* Select USBD */
    SYS->USBPHY = (SYS->USBPHY & ~SYS_USBPHY_USBROLE_Msk) | SYS_USBPHY_USBEN_Msk | SYS_USBPHY_SBO_Msk;

    /* Enable FMC ISP function */
    FMC_Open();

    uint32_t serial = FMC_ReadUID(0) ^ FMC_ReadUID(1) ^ FMC_ReadUID(2);
    
    // read serial number into serialNumber descriptor string
    sprintf(serialNumber, "%x%x%x%x", 
                                        (uint8_t)(serial & 0xFF),
                                        (uint8_t)(serial >> 8 & 0xFF),
                                        (uint8_t)(serial >> 16 & 0xFF),
                                        (uint8_t)(serial >> 24 & 0xFF));

    USBD_Open(&gsInfo, NULL, NULL);

    /* Init setup packet buffer */
    /* Buffer for setup packet -> [0 ~ 0x7] */
    USBD->STBUFSEG = SETUP_BUF_BASE;

    /*****************************************************/
    /* EP0 ==> control IN endpoint, address 0 */
    USBD_CONFIG_EP(EP0, USBD_CFG_CSTALL | USBD_CFG_EPMODE_IN | 0);
    /* Buffer range for EP0 */
    USBD_SET_EP_BUF_ADDR(EP0, EP0_BUF_BASE);

    /* EP1 ==> control OUT endpoint, address 0 */
    USBD_CONFIG_EP(EP1, USBD_CFG_CSTALL | USBD_CFG_EPMODE_OUT | 0);
    /* Buffer range for EP1 */
    USBD_SET_EP_BUF_ADDR(EP1, EP1_BUF_BASE);

    /*****************************************************/
    /* EP2 ==> Bulk IN endpoint, address 1 */
    USBD_CONFIG_EP(EP2, USBD_CFG_EPMODE_IN | BULK_IN_EP_NUM);
    /* Buffer offset for EP2 */
    USBD_SET_EP_BUF_ADDR(EP2, EP2_BUF_BASE);

    /* EP3 ==> Bulk Out endpoint, address 2 */
    USBD_CONFIG_EP(EP3, USBD_CFG_EPMODE_OUT | BULK_OUT_EP_NUM);
    /* Buffer offset for EP3 */
    USBD_SET_EP_BUF_ADDR(EP3, EP3_BUF_BASE);
    /* trigger receive OUT data */
    USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);

    USBD_SetVendorRequest(handleVendorRequest);

    USBD_Start();
    
    if (((SYS->CSERVER & SYS_CSERVER_VERSION_Msk) == 0x1))
    {
        /* Start USB trim */
        USBD->INTSTS = USBD_INTSTS_SOFIF_Msk;
        while((USBD->INTSTS & USBD_INTSTS_SOFIF_Msk) == 0);
        SYS->HIRCTCTL = 0x1;
        SYS->HIRCTCTL |= SYS_HIRCTCTL_REFCKSEL_Msk;
        /* Backup default trim */
        u32TrimInit = M32(TRIM_INIT);
    }

    
    NVIC_EnableIRQ(USBD_IRQn);
}

void USB_trimTask()
{
    if (((SYS->CSERVER & SYS_CSERVER_VERSION_Msk) == 0x1))
        {
            /* Start USB trim if it is not enabled. */
            if ((SYS->HIRCTCTL & SYS_HIRCTCTL_FREQSEL_Msk) != 1)
            {
                if(USBD->INTSTS & USBD_INTSTS_SOFIF_Msk)
                {
                    /* Clear SOF */
                    USBD->INTSTS = USBD_INTSTS_SOFIF_Msk;

                    /* Re-enable crystal-less */
                    SYS->HIRCTCTL = 0x1;
                    SYS->HIRCTCTL |= SYS_HIRCTCTL_REFCKSEL_Msk;
                }
            }

            /* Disable USB Trim when error */
            if (SYS->HIRCTISTS & (SYS_HIRCTISTS_CLKERRIF_Msk | SYS_HIRCTISTS_TFAILIF_Msk))
            {
                /* Init TRIM */
                M32(TRIM_INIT) = u32TrimInit;

                /* Disable crystal-less */
                SYS->HIRCTCTL = 0;

                /* Clear error flags */
                SYS->HIRCTISTS = SYS_HIRCTISTS_CLKERRIF_Msk | SYS_HIRCTISTS_TFAILIF_Msk;

                /* Clear SOF */
                USBD->INTSTS = USBD_INTSTS_SOFIF_Msk;
            }
        }
}

