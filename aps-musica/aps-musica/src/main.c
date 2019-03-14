/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>

//Trocar para o buzzer
#define BUZ_PIO      PIOC
#define BUZ_PIO_ID   ID_PIOC
#define BUZ_IDX      8
#define BUZ_IDX_MASK (1 << BUZ_IDX)

// Botão
#define BUT_PIO      PIOA
#define BUT_PIO_ID   ID_PIOA
#define BUT_IDX  11
#define BUT_IDX_MASK (1 << BUT_IDX)


// Inicializa botao SW0 do kit
void init(void){
	// Initialize the board clock
	sysclk_init();
	
	// Configura o buzzer
	pmc_enable_periph_clk(BUZ_PIO_ID);
	pio_configure(BUZ_PIO, PIO_OUTPUT_0, BUZ_IDX_MASK, PIO_DEFAULT);
	
	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP);
	
	//Inicializa PC8 como saída
	pio_set_output(BUZ_PIO, BUZ_IDX_MASK, 0, 0, 0);
}


int main(void)
{
	/* Initialize the SAM system */
	SystemInit();

	/* Replace with your application code */
	while (1)
	{
		if(pio_get(BUZ_PIO, PIO_INPUT,BUZ_IDX_MASK)){ //botao inicio pressionado
			pio_set(BUZ_PIO, BUZ_IDX_MASK);
		}
		
		if(pio_get(BUZ_PIO, BUZ_IDX_MASK)){ //botao pause pressionado
			// coloca 0 no pino do LED
			pio_clear(BUZ_PIO, BUZ_IDX_MASK);
		}
	}
}

