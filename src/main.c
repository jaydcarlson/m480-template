#include <stdio.h>
#include "M480.h"
#include "NuMicro.h"
#include "bpwm.h"
#include "gpio.h"
#include "usb_vendor.h"

void SYS_Init(void)
{
    // from clock config tool

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* If the macros do not exist in your project, please refer to the related clk.h in Header folder of the tool package */
    /* Set XT1_OUT(PF.2) and XT1_IN(PF.3) to input mode */
    PF->MODE &= ~(GPIO_MODE_MODE2_Msk | GPIO_MODE_MODE3_Msk);

    SYS->GPA_MFPL = (SYS->GPA_MFPL & ~SYS_GPA_MFPL_PA0MFP_Msk) | SYS_GPA_MFPL_PA0MFP_BPWM0_CH0;
    SYS->GPA_MFPL = (SYS->GPA_MFPL & ~SYS_GPA_MFPL_PA1MFP_Msk) | SYS_GPA_MFPL_PA1MFP_BPWM0_CH1;

    /* Enable clock source */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk|CLK_PWRCTL_HIRC48MEN_Msk);

    /* Waiting for clock source ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk|CLK_STATUS_HIRC48MSTB_Msk);

    /* Disable PLL first to avoid unstable when setting PLL */
    CLK_DisablePLL();

    /* Set PLL frequency */
    CLK->PLLCTL = (CLK->PLLCTL & ~(0x000FFFFFUL)) | 0x0008421EUL;

    /* Waiting for PLL ready */
    CLK_WaitClockReady(CLK_STATUS_PLLSTB_Msk);

    /* Set HCLK clock */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_PLL, CLK_CLKDIV0_HCLK(1));

    /* Set PCLK-related clock */
    CLK->PCLKDIV = (CLK_PCLKDIV_PCLK0DIV1 | CLK_PCLKDIV_PCLK1DIV1);

    /* Enable IP clock */
    CLK_EnableModuleClock(FMCIDLE_MODULE);
    CLK_EnableModuleClock(ISP_MODULE);
    CLK_EnableModuleClock(USBD_MODULE);

        /* Enable BPWM0 clock source */
    CLK_EnableModuleClock(BPWM0_MODULE);

    /* Select BPWM module clock source */
    CLK_SetModuleClock(BPWM0_MODULE, CLK_CLKSEL2_BPWM0SEL_PCLK0, 0);

    CLK_SetModuleClock(USBD_MODULE, CLK_CLKSEL0_USBSEL_RC48M, CLK_CLKDIV0_USB(1));

}
volatile uint32_t msTicks = 0;

void SysTick_Handler()
{
    msTicks++;
}

int main (void)
{
    SYS_Init();

    USB_init();

    SysTick_Config(192000000 / 1000);


    BPWM_ConfigOutputChannel(BPWM0, 0, 20000, 0);
    BPWM_ConfigOutputChannel(BPWM0, 1, 20000, 0);
    BPWM_EnableOutput(BPWM0, BIT0|BIT1);
    BPWM_Start(BPWM0, 0);

    // GPIO_SetMode(PH, BIT0,  GPIO_MODE_OUTPUT);
    // PH->MODE = 1;
    // GPIO_SetMode(PA, BIT0 | BIT1,  GPIO_MODE_OUTPUT);

    while(1) {
        BPWM_ConfigOutputChannel(BPWM0, 0, 20000, 50);
        // PA0 = 1;
        // PA1 = 0;
        msTicks = 0;
        while(msTicks < 1000);
        BPWM_ConfigOutputChannel(BPWM0, 0, 20000, 0);
        msTicks = 0;
        while(msTicks < 1000);
        // PA1 = 1;
        // PA0 = 0;
    }
}