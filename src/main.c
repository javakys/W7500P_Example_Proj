/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "diag/Trace.h"
#include "common.h"
#include "system_W7500x.h"
#include "W7500x_uart.h"
#include "timerHandler.h"
#include "W7500x_wztoe.h"

#include "W7500x_miim.h"
#include "wizchip_conf.h"

// ----------------------------------------------------------------------------
//
// Print a greeting message on the trace device and enter a loop
// to count seconds.
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the DEBUG output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//
// ----------------------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

#define __W7500P__

#define __DEF_USED_MDIO__

#ifdef __DEF_USED_MDIO__ // MDC/MDIO defines
	#ifndef __W7500P__ // W7500
		//#define __DEF_USED_IC101AG__ // PHY initialize for WIZwiki-W7500 Board
		#define W7500x_MDIO    GPIO_Pin_14
		#define W7500x_MDC     GPIO_Pin_15
	#else // W7500P
		#define W7500x_MDIO    GPIO_Pin_15
		#define W7500x_MDC     GPIO_Pin_14
	#endif
#endif

void delay(__IO uint32_t milliseconds); //Notice: used ioLibray
void TimingDelay_Decrement(void);
static void W7500x_WZTOE_Init(void);
void Net_Conf(void);

/* Private variables ---------------------------------------------------------*/
static __IO uint32_t TimingDelay;

int
main (int argc, char* argv[])
{
  // Normally at this stage most of the microcontroller subsystems, including
  // the clock, were initialised by the CMSIS SystemInit() function invoked
  // from the startup file, before calling main().
  // (see system/src/cortexm/_initialize_hardware.c)
  // If further initialisations are required, customise __initialize_hardware()
  // or add the additional initialisation here, for example:
  //
  // HAL_Init();

  // In this sample the SystemInit() function is just a placeholder,
  // if you do not add the real one, the clock will remain configured with
  // the reset value, usually a relatively low speed RC clock (8-12MHz).
	SystemInit_User(CLOCK_SOURCE_EXTERNAL, PLL_SOURCE_12MHz, SYSTEM_CLOCK_48MHz);

    Timer_Configuration();

    S_UART_Init(115200);

    SysTick_Config((GetSystemClock()/1000));


//    printf("PHY Initializing...\r\n");
//#ifdef __W7500P__ // W7500P
	// PB_05, PB_12 pull down
	*(volatile uint32_t *)(0x41003070) = 0x61; // RXDV - set pull down (PB_12)
	*(volatile uint32_t *)(0x41002054) = 0x01; // COL  - PB_05 AFC
	*(volatile uint32_t *)(0x41003054) = 0x61; // COL  - set pull down (PB_05)
	*(volatile uint32_t *)(0x41002058) = 0x01; // DUP  - PB_06 AFC
	*(volatile uint32_t *)(0x41003058) = 0x61; // DUP  - set pull down (PB_06)

	*(volatile uint32_t *)(0x410020D8) = 0x01; // PD 06 AFC[00 : zero / 01 : PD06]
	*(volatile uint32_t *)(0x410030D8) = 0x02; // PD 06 PADCON
	*(volatile uint32_t *)(0x45000004) = 0x40; // GPIOD DATAOUT [PD06 output 1]
	*(volatile uint32_t *)(0x45000010) = 0x40; // GPIOD OUTENSET


	*(volatile uint32_t *)(0x41002048) = 0x01; // PB 02 AFC[00 : zero / 01 : PD06]
	*(volatile uint32_t *)(0x41003048) = 0x02; // PB 02 PADCON
	*(volatile uint32_t *)(0x43000004) = 0x04; // GPIOB DATAOUT [PB02 output 1]
	*(volatile uint32_t *)(0x43000010) = 0x04; // GPIOD OUTENSET

	uint8_t output = 1;

//#endif

#ifdef __DEF_USED_MDIO__
	/* mdio Init */
	mdio_init(GPIOB, W7500x_MDC, W7500x_MDIO);
	mdio_write(GPIOB, PHYREG_CONTROL, PHYReset); // PHY Reset
	mdio_write(GPIOB, PHYREG_CONTROL, FullDuplex10); // PHY Reset
#endif

//	W7500x_WZTOE_Init();
//	Net_Conf();

	// Send a greeting to the trace device (skipped on Release).
//	printf("Hello ARM World!\r\n");

	// At this stage the system clock should have already been configured
	// at high speed.
//	printf("System clock: %u Hz\r\n", SystemCoreClock);


	int seconds = 0;

	setDevtime(seconds);

	uint8_t ch;
	// Infinite loop
	while (1)
	{
		uint8_t tmpseconds;
//		delay_ms(1000);
//		seconds++;
//		if((tmpseconds = getDevtime()) != seconds){
//			seconds = tmpseconds;
//			// Count seconds on the trace device.
////			printf ("Second %d\r\n", seconds);
//			if(output){
////				printf("L");
//				*(volatile uint32_t *)(0x43000004) = 0x00; // GPIOD OUTENSET
//				output = 0;
//			}else{
////				printf("H");
//				*(volatile uint32_t *)(0x43000004) = 0x04; // GPIOD OUTENSET
//				output = 1;
//			}
//		}
		ch = S_UartGetc();
		switch(ch)
		{
		case '0':
			printf("HalfDuplex 10Mbps Mode\r\n");
			mdio_write(GPIOB, PHYREG_CONTROL, PHYReset); // PHY Reset
			mdio_write(GPIOB, PHYREG_CONTROL, HalfDuplex10); // PHY Reset
			break;
		case '1':
			printf("FullDuplex 10Mbps Mode\r\n");
			mdio_write(GPIOB, PHYREG_CONTROL, PHYReset); // PHY Reset
			mdio_write(GPIOB, PHYREG_CONTROL, FullDuplex10); // PHY Reset
			break;
		case '2':
			printf("HalfDuplex 100Mbps Mode\r\n");
			mdio_write(GPIOB, PHYREG_CONTROL, PHYReset); // PHY Reset
			mdio_write(GPIOB, PHYREG_CONTROL, HalfDuplex100); // PHY Reset
			break;
		case '3':
			printf("FullDuplex 100Mbps Mode\r\n");
			mdio_write(GPIOB, PHYREG_CONTROL, PHYReset); // PHY Reset
			mdio_write(GPIOB, PHYREG_CONTROL, FullDuplex100); // PHY Reset
			break;
		default:
			printf("Invalid Opcode. Set AUTO NEGO mode\r\n");
			mdio_write(GPIOB, PHYREG_CONTROL, PHYReset); // PHY Reset
			break;
		}

//*/
	}
	// Infinite loop, never return.
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void delay(__IO uint32_t milliseconds)
{
    TimingDelay = milliseconds;
    while(TimingDelay != 0);
}


/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
    if(TimingDelay != 0x00)
    {
        TimingDelay--;
    }
}

static void W7500x_WZTOE_Init(void)
{
    ////////////////////////////////////////////////////
    // W7500x WZTOE (Hardwired TCP/IP core) Initialize
    ////////////////////////////////////////////////////

    /* Set Network Configuration: HW Socket Tx/Rx buffer size */
    uint8_t tx_size[8] = { 4, 2, 2, 2, 2, 2, 2, 0 }; // default: { 2, 2, 2, 2, 2, 2, 2, 2 }
    uint8_t rx_size[8] = { 4, 2, 2, 2, 2, 2, 2, 0 }; // default: { 2, 2, 2, 2, 2, 2, 2, 2 }

#ifdef _MAIN_DEBUG_
    uint8_t i;
#endif

    /* Software reset the WZTOE(Hardwired TCP/IP core) */
    wizchip_sw_reset();

    /* Set WZ_100US Register */
    setTIC100US((GetSystemClock()/10000));
#ifdef _MAIN_DEBUG_
    printf("\r\n >> WZTOE Settings ==========================\r\n");
    printf(" - getTIC100US: %X, (%X) \r\n", getTIC100US(), *(uint32_t *)WZTOE_TIC100US); // for debugging
#endif

    /* Set Network Configuration */
    wizchip_init(tx_size, rx_size);

#ifdef _MAIN_DEBUG_
    printf(" - WZTOE H/W Socket Buffer Settings (kB)\r\n");
    printf("   [Tx] ");
    for(i = 0; i < _WIZCHIP_SOCK_NUM_; i++) printf("%d ", getSn_TXBUF_SIZE(i));
    printf("\r\n");
    printf("   [Rx] ");
    for(i = 0; i < _WIZCHIP_SOCK_NUM_; i++) printf("%d ", getSn_RXBUF_SIZE(i));
    printf("\r\n");
#endif
}

void Net_Conf(void)
{
	uint8_t mac[6] = {0x00, 0x08, 0xdc, 0x11, 0x22, 0x33};
	uint8_t ip[4] = {192, 168, 0, 110};
	uint8_t gw[4] = {192, 168, 0, 1};
	uint8_t sn[4] = {255, 255, 255, 0};
	uint8_t dns[4] = {8, 8, 8, 8};

	wiz_NetInfo gWIZNETINFO;

	/* wizchip netconf */
	memcpy(gWIZNETINFO.mac, mac, sizeof(uint8_t)*6);
	memcpy(gWIZNETINFO.ip, ip, sizeof(uint8_t)*4);
	memcpy(gWIZNETINFO.gw, gw, sizeof(uint8_t)*4);
	memcpy(gWIZNETINFO.sn, sn, sizeof(uint8_t)*4);
	memcpy(gWIZNETINFO.dns, dns, sizeof(uint8_t)*4);
	gWIZNETINFO.dhcp = NETINFO_STATIC;

	ctlnetwork(CN_SET_NETINFO, (void*) &gWIZNETINFO);
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
