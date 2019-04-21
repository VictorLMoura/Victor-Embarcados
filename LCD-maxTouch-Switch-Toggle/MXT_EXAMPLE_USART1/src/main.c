/**
 * \file
 *
 * \brief Example of usage of the maXTouch component with USART
 *
 * This example shows how to receive touch data from a maXTouch device
 * using the maXTouch component, and display them in a terminal window by using
 * the USART driver.
 *
 * Copyright (c) 2014-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

/**
 * \mainpage
 *
 * \section intro Introduction
 * This simple example reads data from the maXTouch device and sends it over
 * USART as ASCII formatted text.
 *
 * \section files Main files:
 * - example_usart.c: maXTouch component USART example file
 * - conf_mxt.h: configuration of the maXTouch component
 * - conf_board.h: configuration of board
 * - conf_clock.h: configuration of system clock
 * - conf_example.h: configuration of example
 * - conf_sleepmgr.h: configuration of sleep manager
 * - conf_twim.h: configuration of TWI driver
 * - conf_usart_serial.h: configuration of USART driver
 *
 * \section apiinfo maXTouch low level component API
 * The maXTouch component API can be found \ref mxt_group "here".
 *
 * \section deviceinfo Device Info
 * All UC3 and Xmega devices with a TWI module can be used with this component
 *
 * \section exampledescription Description of the example
 * This example will read data from the connected maXTouch explained board
 * over TWI. This data is then processed and sent over a USART data line
 * to the board controller. The board controller will create a USB CDC class
 * object on the host computer and repeat the incoming USART data from the
 * main controller to the host. On the host this object should appear as a
 * serial port object (COMx on windows, /dev/ttyxxx on your chosen Linux flavour).
 *
 * Connect a terminal application to the serial port object with the settings
 * Baud: 57600
 * Data bits: 8-bit
 * Stop bits: 1 bit
 * Parity: None
 *
 * \section compinfo Compilation Info
 * This software was written for the GNU GCC and IAR for AVR.
 * Other compilers may or may not work.
 *
 * \section contactinfo Contact Information
 * For further information, visit
 * <A href="http://www.atmel.com/">Atmel</A>.\n
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include <asf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "conf_board.h"
#include "conf_example.h"
#include "conf_uart_serial.h"
#include "maquina1.h"


#define MAX_ENTRIES        3
#define STRING_LENGTH     70

#define USART_TX_MAX_LENGTH     0xff

typedef struct {
	const uint8_t *data;
	uint16_t width;
	uint16_t height;
	uint8_t dataSize;
} tImage;
#include "icones/lavagens.h"
#include "icones/back.h"

void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq);

const tImage lavagem = { image_data_lavagens, 93, 93,  8 };
const tImage back_arrow = { image_data_back, 93, 93,  8 };


volatile t_ciclo *status_ciclo;
volatile int lock_screen;
volatile int start;
volatile int porta_aberta;
volatile int tranca_porta;
volatile int tela;
volatile uint8_t tempo = 0;
volatile uint8_t minuto = 0;

struct ili9488_opt_t g_ili9488_display_opt;
const uint32_t BUTTON_W = 240;
const uint32_t BUTTON_H = 150;
const uint32_t BUTTON_BORDER = 2;
const uint32_t BUTTON_X = ILI9488_LCD_WIDTH/2;
const uint32_t BUTTON_Y = 200;

/**
 * Inicializa ordem do menu
 * retorna o primeiro ciclo que
 * deve ser exibido.
 */
t_ciclo *initMenuOrder(){
  c_rapido.previous = &c_enxague;
  c_rapido.next = &c_diario;

  c_diario.previous = &c_rapido;
  c_diario.next = &c_pesado;

  c_pesado.previous = &c_diario;
  c_pesado.next = &c_enxague;

  c_enxague.previous = &c_pesado;
  c_enxague.next = &c_centrifuga;

  c_centrifuga.previous = &c_enxague;
  c_centrifuga.next = &c_rapido;

  return(&c_diario);
}


	
static void configure_lcd(void){
	/* Initialize display parameter */
	g_ili9488_display_opt.ul_width = ILI9488_LCD_WIDTH;
	g_ili9488_display_opt.ul_height = ILI9488_LCD_HEIGHT;
	g_ili9488_display_opt.foreground_color = COLOR_CONVERT(COLOR_WHITE);
	g_ili9488_display_opt.background_color = COLOR_CONVERT(COLOR_WHITE);

	/* Initialize LCD */
	ili9488_init(&g_ili9488_display_opt);
}


static void mxt_init(struct mxt_device *device)
{
	enum status_code status;

	/* T8 configuration object data */
	uint8_t t8_object[] = {
		0x0d, 0x00, 0x05, 0x0a, 0x4b, 0x00, 0x00,
		0x00, 0x32, 0x19
	};

	/* T9 configuration object data */
	uint8_t t9_object[] = {
		0x8B, 0x00, 0x00, 0x0E, 0x08, 0x00, 0x80,
		0x32, 0x05, 0x02, 0x0A, 0x03, 0x03, 0x20,
		0x02, 0x0F, 0x0F, 0x0A, 0x00, 0x00, 0x00,
		0x00, 0x18, 0x18, 0x20, 0x20, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x02,
		0x02
	};

	/* T46 configuration object data */
	uint8_t t46_object[] = {
		0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x03,
		0x00, 0x00
	};
	
	/* T56 configuration object data */
	uint8_t t56_object[] = {
		0x02, 0x00, 0x01, 0x18, 0x1E, 0x1E, 0x1E,
		0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E,
		0x1E, 0x1E, 0x1E, 0x1E, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00
	};

	/* TWI configuration */
	twihs_master_options_t twi_opt = {
		.speed = MXT_TWI_SPEED,
		.chip  = MAXTOUCH_TWI_ADDRESS,
	};

	status = (enum status_code)twihs_master_setup(MAXTOUCH_TWI_INTERFACE, &twi_opt);
	Assert(status == STATUS_OK);

	/* Initialize the maXTouch device */
	status = mxt_init_device(device, MAXTOUCH_TWI_INTERFACE,
			MAXTOUCH_TWI_ADDRESS, MAXTOUCH_XPRO_CHG_PIO);
	Assert(status == STATUS_OK);

	/* Issue soft reset of maXTouch device by writing a non-zero value to
	 * the reset register */
	mxt_write_config_reg(device, mxt_get_object_address(device,
			MXT_GEN_COMMANDPROCESSOR_T6, 0)
			+ MXT_GEN_COMMANDPROCESSOR_RESET, 0x01);

	/* Wait for the reset of the device to complete */
	delay_ms(MXT_RESET_TIME);

	/* Write data to configuration registers in T7 configuration object */
	mxt_write_config_reg(device, mxt_get_object_address(device,
			MXT_GEN_POWERCONFIG_T7, 0) + 0, 0x20);
	mxt_write_config_reg(device, mxt_get_object_address(device,
			MXT_GEN_POWERCONFIG_T7, 0) + 1, 0x10);
	mxt_write_config_reg(device, mxt_get_object_address(device,
			MXT_GEN_POWERCONFIG_T7, 0) + 2, 0x4b);
	mxt_write_config_reg(device, mxt_get_object_address(device,
			MXT_GEN_POWERCONFIG_T7, 0) + 3, 0x84);

	/* Write predefined configuration data to configuration objects */
	mxt_write_config_object(device, mxt_get_object_address(device,
			MXT_GEN_ACQUISITIONCONFIG_T8, 0), &t8_object);
	mxt_write_config_object(device, mxt_get_object_address(device,
			MXT_TOUCH_MULTITOUCHSCREEN_T9, 0), &t9_object);
	mxt_write_config_object(device, mxt_get_object_address(device,
			MXT_SPT_CTE_CONFIGURATION_T46, 0), &t46_object);
	mxt_write_config_object(device, mxt_get_object_address(device,
			MXT_PROCI_SHIELDLESS_T56, 0), &t56_object);

	/* Issue recalibration command to maXTouch device by writing a non-zero
	 * value to the calibrate register */
	mxt_write_config_reg(device, mxt_get_object_address(device,
			MXT_GEN_COMMANDPROCESSOR_T6, 0)
			+ MXT_GEN_COMMANDPROCESSOR_CALIBRATE, 0x01);
}

void draw_screen(void) {
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
	ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, ILI9488_LCD_HEIGHT-1);
}

void draw_button() { //Primeira tela do display
	tela = 1;
	
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_TOMATO));
	ili9488_draw_filled_rectangle((BUTTON_X-BUTTON_W/2+BUTTON_BORDER)+30, BUTTON_Y+BUTTON_BORDER, (BUTTON_X+BUTTON_W/2-BUTTON_BORDER)-30, BUTTON_Y+BUTTON_H/2-BUTTON_BORDER);
		
	char buff[32];
	sprintf(buff, status_ciclo);
	
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
	ili9488_draw_filled_rectangle(100, BUTTON_Y+BUTTON_BORDER+100,  300,   BUTTON_Y+BUTTON_BORDER+200 );

	
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
	ili9488_draw_string((BUTTON_X-BUTTON_W/2+BUTTON_BORDER)+30+55, BUTTON_Y+BUTTON_BORDER+15, buff);
	
	sprintf(buff, "-- SELECT --");
	ili9488_draw_string((BUTTON_X-BUTTON_W/2+BUTTON_BORDER)+30+15, BUTTON_Y+BUTTON_BORDER+15+25, buff);

	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_TOMATO));
	sprintf(buff, ">>>");
	ili9488_draw_string((BUTTON_X-BUTTON_W/2+BUTTON_BORDER)+30+15+180, BUTTON_Y+BUTTON_BORDER+15+25, buff);
	
	sprintf(buff, "<<<");
	ili9488_draw_string((BUTTON_X-BUTTON_W/2+BUTTON_BORDER)+30+15-70, BUTTON_Y+BUTTON_BORDER+15+25, buff);
	
	// ------------- Desenhar no LCD as características desse ciclo ------------- //
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
	sprintf(buff, "Tempo de enxague: %d" ,status_ciclo->enxagueTempo);
	ili9488_draw_string(15, BUTTON_Y+BUTTON_BORDER+100, buff);
	sprintf(buff, "Qtd de enxagues:  %d" ,status_ciclo->enxagueQnt);
	ili9488_draw_string(15, BUTTON_Y+BUTTON_BORDER+100+17, buff);
	sprintf(buff, "Vel centrifugacao: %d" ,status_ciclo->centrifugacaoRPM);
	ili9488_draw_string(15, BUTTON_Y+BUTTON_BORDER+100+17+17, buff);
	sprintf(buff, "Tempo centrifugacao: %d" ,status_ciclo->centrifugacaoTempo);
	ili9488_draw_string(15, BUTTON_Y+BUTTON_BORDER+100+17+17+17, buff);
	
	if (status_ciclo->heavy == 1){sprintf(buff, "Modo pesado: ON");}
	else if (status_ciclo->heavy == 0){sprintf(buff, "Modo pesado: OFF");}
	ili9488_draw_string(15, BUTTON_Y+BUTTON_BORDER+100+17+17+17+17, buff);
	
	if (status_ciclo->bubblesOn == 1){sprintf(buff, "Bubbles: ON");}
	else if (status_ciclo->bubblesOn == 0){sprintf(buff, "Bubbles: OFF");}
	ili9488_draw_string(15, BUTTON_Y+BUTTON_BORDER+100+17+17+17+17+17, buff);	

	// ---------- LOCK SCREEEN ------------//
	char stringLCD[32];
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_TOMATO));
	ili9488_draw_filled_rectangle(190,20,295,45);
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
	if (lock_screen == 0){sprintf(stringLCD, "Lock screen: ON/(OFF)");}
	else if (lock_screen == 1){sprintf(stringLCD, "Lock screen: (ON)/OFF");}
	ili9488_draw_string(40, 25, stringLCD);
	sprintf(stringLCD, "___________________________");
	ili9488_draw_string(0, 45, stringLCD);
	// ---------- LOCK SCREEEN ------------//
	
	/***		Desenhando ícone			***/
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
	// desenha imagem lavagem na posicao X=80 e Y=150
	ili9488_draw_pixmap(0, 90, lavagem.width, lavagem.height, lavagem.data);
	/***					***/
	
	sprintf(stringLCD, "Selecione o ciclo");
	ili9488_draw_string(100, 135, stringLCD);

}

void draw_button2() { //segunda tela do display
	tela = 2;
	
	// ---------- LOCK SCREEEN ------------//
	char stringLCD[32];
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_TOMATO));
	ili9488_draw_filled_rectangle(190,20,295,45);
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
	if (lock_screen == 0){sprintf(stringLCD, "Lock screen: ON/(OFF)");}
	else if (lock_screen == 1){sprintf(stringLCD, "Lock screen: (ON)/OFF");}
	ili9488_draw_string(40, 25, stringLCD);
	sprintf(stringLCD, "___________________________");
	ili9488_draw_string(0, 45, stringLCD);
	// ---------- LOCK SCREEEN ------------//
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_TOMATO));
	ili9488_draw_filled_rectangle(70,180,250,220);
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_GREENYELLOW));
	ili9488_draw_filled_rectangle(70,120,250,160);
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
	
	if(porta_aberta == 0){
		sprintf(stringLCD, "Abrir a porta");
		ili9488_draw_string(85, 140, stringLCD);
	}
	else if(porta_aberta == 1){
		sprintf(stringLCD, "Fechar a porta");
		ili9488_draw_string(80, 140, stringLCD);
	}

	char buff3[32];
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
	if (start == 0){ //caso em que a lavagem ainda nao começou
		sprintf(buff3, "-- START --");
		ili9488_draw_string(95, 200, buff3);
		sprintf(buff3, "Esperando para comecar");
		ili9488_draw_string(35, 275, buff3);
		
		if (porta_aberta == 1){
			ili9488_set_foreground_color(COLOR_CONVERT(COLOR_RED));
			sprintf(buff3, "PORTA ABERTA !");
			ili9488_draw_string(80, 310, buff3);	
			sprintf(buff3, "Feche para prosseguir");
			ili9488_draw_string(40, 340, buff3);
			ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
		}		
	}
	else if (start == 1){ //caso em que a lavagem ja começou
		sprintf(buff3, "-- STOP --");
		ili9488_draw_string(100, 200, buff3);
		sprintf(buff3, "Lavagem em progresso");
		ili9488_draw_string(35, 275, buff3);
		if (porta_aberta == 1){
			ili9488_set_foreground_color(COLOR_CONVERT(COLOR_RED));
			sprintf(buff3, "PORTA TRANCADA !");
			ili9488_draw_string(70, 310, buff3);
			sprintf(buff3, "Pare a lavagem para abrir");
			ili9488_draw_string(10, 340, buff3);
			ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
		}
		int tempo_restante = (status_ciclo->centrifugacaoTempo+status_ciclo->enxagueTempo)-minuto;
		sprintf(buff3, "%s %d", "Tempo restante:",tempo_restante);
		ili9488_draw_string(45, 395, buff3);
	}
}

void printa_tempo_restante(){
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
	ili9488_draw_filled_rectangle(210,370,400,500);
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
	char buff5[32];
	int tempo_restante = (status_ciclo->centrifugacaoTempo+status_ciclo->enxagueTempo)-minuto;
	sprintf(buff5, "%s %d", "Tempo restante:",tempo_restante);
	ili9488_draw_string(45, 395, buff5);	
}

void printa_fim(){
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
	ili9488_draw_filled_rectangle(0,310,320,480);
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_TOMATO));
	char buff5[32];
	sprintf(buff5, "FIM");
	ili9488_draw_string(140, 395, buff5);
}

uint32_t convert_axis_system_x(uint32_t touch_y) {
	// entrada: 4096 - 0 (sistema de coordenadas atual)
	// saida: 0 - 320
	return ILI9488_LCD_WIDTH - ILI9488_LCD_WIDTH*touch_y/4096;
}

uint32_t convert_axis_system_y(uint32_t touch_x) {
	// entrada: 0 - 4096 (sistema de coordenadas atual)
	// saida: 0 - 320
	return ILI9488_LCD_HEIGHT*touch_x/4096;
}

 void update_screen(uint32_t tx, uint32_t ty) {
	 if(tela == 1){
		 if (lock_screen == 0){
			if(tx >= 90 && tx <= 265) { //Caso clique no botao se selecionar o ciclo
				if(ty >= 206 && ty <= 275) { 
					draw_screen();
					draw_button2();
				}
			}
		
			if(tx >= 0 && tx <= 65) { //Caso clique no botao se selecionar o ciclo
				if(ty >= 225 && ty <= 285) {
					//Nesse caso, ir para a proxima tela, a de lavagem
					status_ciclo = status_ciclo->previous;
					draw_button();
				}
			}
		
			if(tx >= 270 && tx <= 320) { //Caso clique no botao se selecionar o ciclo
				if(ty >= 225 && ty <= 285) {
					//Nesse caso, ir para a proxima tela, a de lavagem
					status_ciclo = status_ciclo->next;
					draw_button();
				}
			}
		 }
		if(tx >= 180 && tx <= 295) { //Caso clique no botao, bloquear a tela
			if(ty >= 0 && ty <= 30) {
				if(lock_screen == 1)lock_screen = 0;
				else if(lock_screen == 0) lock_screen = 1;
				draw_button();
			}
		}		 
	 }
	 
	if(tela == 2){	
		if(lock_screen == 0){
			if(tx >= 70 && tx <= 275) { //Caso clique no botao, parar a lavagem ou começar
				if(ty >= 170 && ty <= 230) {
					if((start == 0) && (porta_aberta == 0)){
						start = 1;
						TC_init(TC0, ID_TC0, 0, 1); //para contar a cada segundo
						printa_tempo_restante();
					}
					else if(start == 1){
						start = 0;
						tc_stop(TC0, 0);
					}
					draw_screen();
					draw_button2();
				}
			}
			if(tx >= 70 && tx <= 275) { //Caso clique no botao, abrir/fechar a porta
				if(ty >= 100 && ty <= 160) {
					if(porta_aberta == 0)porta_aberta = 1;
					else if(porta_aberta == 1) porta_aberta = 0;
					draw_screen();
					draw_button2();
				}
			}
			
		}
		if(tx >= 180 && tx <= 295) { //Caso clique no botao, bloquear a tela
			if(ty >= 0 && ty <= 30) {
				if(lock_screen == 1)lock_screen = 0;
				else if(lock_screen == 0) lock_screen = 1;
				draw_button2();
			}
		}
	}
}


void mxt_handler(struct mxt_device *device)
{
	/* USART tx buffer initialized to 0 */
	char tx_buf[STRING_LENGTH * MAX_ENTRIES] = {0};
	uint8_t i = 0; /* Iterator */

	/* Temporary touch event data struct */
	struct mxt_touch_event touch_event;

	/* Collect touch events and put the data in a string,
	 * maximum 2 events at the time */
	do {
		/* Temporary buffer for each new touch event line */
		char buf[STRING_LENGTH];
	
		/* Read next next touch event in the queue, discard if read fails */
		if (mxt_read_touch_event(device, &touch_event) != STATUS_OK) {
			continue;
		}
		
		 // eixos trocados (quando na vertical LCD)
		uint32_t conv_x = convert_axis_system_x(touch_event.y);
		uint32_t conv_y = convert_axis_system_y(touch_event.x);
		
		/* Format a new entry in the data string that will be sent over USART */
		sprintf(buf, "Nr: %1d, X:%4d, Y:%4d, Status:0x%2x conv X:%3d Y:%3d\n\r",
				touch_event.id, touch_event.x, touch_event.y,
				touch_event.status, conv_x, conv_y);
		update_screen(conv_x, conv_y);

		/* Add the new string to the string buffer */
		strcat(tx_buf, buf);
		i++;
		
		break;

		/* Check if there is still messages in the queue and
		 * if we have reached the maximum numbers of events */
	} while ((mxt_is_message_pending(device)) & (i < MAX_ENTRIES));

	/* If there is any entries in the buffer, send them over USART */
	if (i > 0) {
		usart_serial_write_packet(USART_SERIAL_EXAMPLE, (uint8_t *)tx_buf, strlen(tx_buf));
	}
}

void mxt_debouce(struct mxt_device *device)
{
	/* USART tx buffer initialized to 0 */
	char tx_buf[STRING_LENGTH * MAX_ENTRIES] = {0};
	uint8_t i = 0; /* Iterator */

	/* Temporary touch event data struct */
	struct mxt_touch_event touch_event;

	/* Collect touch events and put the data in a string,
	 * maximum 2 events at the time */
	do {
		/* Temporary buffer for each new touch event line */
		char buf[STRING_LENGTH];
	
		/* Read next next touch event in the queue, discard if read fails */
		if (mxt_read_touch_event(device, &touch_event) != STATUS_OK) {
			continue;
		}

		i++;

		/* Check if there is still messages in the queue and
		 * if we have reached the maximum numbers of events */
	} while ((mxt_is_message_pending(device)) & (i < MAX_ENTRIES));

	/* If there is any entries in the buffer, send them over USART */
	if (i > 0) {
		usart_serial_write_packet(USART_SERIAL_EXAMPLE, (uint8_t *)tx_buf, strlen(tx_buf));
	}
}

/**
*  Interrupt handler for TC1 interrupt.
*/
void TC0_Handler(void){
	volatile uint32_t ul_dummy;

	/****************************************************************
	* Devemos indicar ao TC que a interrupção foi satisfeita.
	******************************************************************/
	ul_dummy = tc_get_status(TC0, 0);

	/* Avoid compiler warning */
	UNUSED(ul_dummy);
	
	tempo+=1; //em segundos
	if (tempo == 1){
		minuto += 1;
		tempo = 0;
		printa_tempo_restante();
	}
	if (((status_ciclo->centrifugacaoTempo+status_ciclo->enxagueTempo)-minuto) <= 0){
		start = 0;
		minuto = 0;
		tempo = 0;
		//draw_screen();
		printa_fim();
		tc_stop(TC0, 0);
	}
}

/**
* Configura TimerCounter (TC) para gerar uma interrupcao no canal (ID_TC e TC_CHANNEL)
* na taxa de especificada em freq.
*/
void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq){
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	uint32_t channel = 1;

	/* Configura o PMC */
	/* O TimerCounter é meio confuso
	o uC possui 3 TCs, cada TC possui 3 canais
	TC0 : ID_TC0, ID_TC1, ID_TC2
	TC1 : ID_TC3, ID_TC4, ID_TC5
	TC2 : ID_TC6, ID_TC7, ID_TC8
	*/
	pmc_enable_periph_clk(ID_TC);

	/** Configura o TC para operar em  4Mhz e interrupçcão no RC compare */
	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	/* Configura e ativa interrupçcão no TC canal 0 */
	/* Interrupção no C */
	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);

	/* Inicializa o canal 0 do TC */
	tc_start(TC, TC_CHANNEL);
}


int main(void)
{
	status_ciclo = initMenuOrder();
	
	struct mxt_device device; /* Device data container */

	/* Initialize the USART configuration struct */
	const usart_serial_options_t usart_serial_options = {
		.baudrate     = USART_SERIAL_EXAMPLE_BAUDRATE,
		.charlength   = USART_SERIAL_CHAR_LENGTH,
		.paritytype   = USART_SERIAL_PARITY,
		.stopbits     = USART_SERIAL_STOP_BIT
	};
	sysclk_init(); /* Initialize system clocks */
	board_init();  /* Initialize board */
	configure_lcd();
	draw_screen();
	draw_button();
	/* Initialize the mXT touch device */
	mxt_init(&device);
	
	/* Initialize stdio on USART */
	stdio_serial_init(USART_SERIAL_EXAMPLE, &usart_serial_options);

	printf("\n\rmaXTouch data USART transmitter\n\r");
		

	char stringLCD[32];
	lock_screen = 0;
	while (true) {
		/* Check for any pending messages and run message handler if any
		 * message is found in the queue */
		if (mxt_is_message_pending(&device)) {
			mxt_handler(&device);
			delay_s(1);
			mxt_debouce(&device);
		}
		if (mxt_is_message_pending(&device)){ 
					mxt_debouce(&device);
		}
	} 

	return 0;
}
