/** 
 ** Entrega realizada em parceria com:
 **  - Victor Laperuta de Moura
 **	 -
 **
 **  - https://www.youtube.com/watch?v=nBB7KFQagPM
 **/

#include <asf.h>

//Trocar para o buzzer
#define BUZ_PIO      PIOC
#define BUZ_PIO_ID   ID_PIOC
#define BUZ_IDX      31u
#define BUZ_IDX_MASK (1 << BUZ_IDX)

// Botão
#define BUT_PIO      PIOA
#define BUT_PIO_ID   ID_PIOA
#define BUT_IDX  4u
#define BUT_IDX_MASK (1 << BUT_IDX)

// Botão
#define BUT_TROCA_PIO      PIOA
#define BUT_TROCA_PIO_ID   ID_PIOA
#define BUT_TROCA_IDX  3u
#define BUT_TROCA_IDX_MASK (1 << BUT_IDX)

// defines das notas
#define songspeed1 1.5 //Change to 2 for a slower version of the song, the bigger the number the slower the song
//*****************************************
#define NOTE_C4  262   //Defining note frequency
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988

int notes1[] = {       //Note of the song, 0 is a rest/pulse
   NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, 0, 
   NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, 0, 
   NOTE_C5, NOTE_D5, NOTE_B4, NOTE_B4, 0,
   NOTE_A4, NOTE_G4, NOTE_A4, 0,
   
   NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, 0, 
   NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, 0, 
   NOTE_C5, NOTE_D5, NOTE_B4, NOTE_B4, 0,
   NOTE_A4, NOTE_G4, NOTE_A4, 0,
   
   NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, 0, 
   NOTE_A4, NOTE_C5, NOTE_D5, NOTE_D5, 0, 
   NOTE_D5, NOTE_E5, NOTE_F5, NOTE_F5, 0,
   NOTE_E5, NOTE_D5, NOTE_E5, NOTE_A4, 0,
   
   NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, 0, 
   NOTE_D5, NOTE_E5, NOTE_A4, 0, 
   NOTE_A4, NOTE_C5, NOTE_B4, NOTE_B4, 0,
   NOTE_C5, NOTE_A4, NOTE_B4, 0,

   NOTE_A4, NOTE_A4, 
   //Repeat of first part
   NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, 0, 
   NOTE_C5, NOTE_D5, NOTE_B4, NOTE_B4, 0,
   NOTE_A4, NOTE_G4, NOTE_A4, 0,

   NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, 0, 
   NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, 0, 
   NOTE_C5, NOTE_D5, NOTE_B4, NOTE_B4, 0,
   NOTE_A4, NOTE_G4, NOTE_A4, 0,
   
   NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, 0, 
   NOTE_A4, NOTE_C5, NOTE_D5, NOTE_D5, 0, 
   NOTE_D5, NOTE_E5, NOTE_F5, NOTE_F5, 0,
   NOTE_E5, NOTE_D5, NOTE_E5, NOTE_A4, 0,
   
   NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, 0, 
   NOTE_D5, NOTE_E5, NOTE_A4, 0, 
   NOTE_A4, NOTE_C5, NOTE_B4, NOTE_B4, 0,
   NOTE_C5, NOTE_A4, NOTE_B4, 0,
   //End of Repeat

   NOTE_E5, 0, 0, NOTE_F5, 0, 0,
   NOTE_E5, NOTE_E5, 0, NOTE_G5, 0, NOTE_E5, NOTE_D5, 0, 0,
   NOTE_D5, 0, 0, NOTE_C5, 0, 0,
   NOTE_B4, NOTE_C5, 0, NOTE_B4, 0, NOTE_A4,

   NOTE_E5, 0, 0, NOTE_F5, 0, 0,
   NOTE_E5, NOTE_E5, 0, NOTE_G5, 0, NOTE_E5, NOTE_D5, 0, 0,
   NOTE_D5, 0, 0, NOTE_C5, 0, 0,
   NOTE_B4, NOTE_C5, 0, NOTE_B4, 0, NOTE_A4
};
//*****************************************
int duration1[] = {         //duration of each note (in ms) Quarter Note is set to 250 ms
  125, 125, 250, 125, 125, 
  125, 125, 250, 125, 125,
  125, 125, 250, 125, 125,
  125, 125, 375, 125, 
  
  125, 125, 250, 125, 125, 
  125, 125, 250, 125, 125,
  125, 125, 250, 125, 125,
  125, 125, 375, 125, 
  
  125, 125, 250, 125, 125, 
  125, 125, 250, 125, 125,
  125, 125, 250, 125, 125,
  125, 125, 125, 250, 125,

  125, 125, 250, 125, 125, 
  250, 125, 250, 125, 
  125, 125, 250, 125, 125,
  125, 125, 375, 375,

  250, 125,
  //Rpeat of First Part
  125, 125, 250, 125, 125,
  125, 125, 250, 125, 125,
  125, 125, 375, 125, 
  
  125, 125, 250, 125, 125, 
  125, 125, 250, 125, 125,
  125, 125, 250, 125, 125,
  125, 125, 375, 125, 
  
  125, 125, 250, 125, 125, 
  125, 125, 250, 125, 125,
  125, 125, 250, 125, 125,
  125, 125, 125, 250, 125,

  125, 125, 250, 125, 125, 
  250, 125, 250, 125, 
  125, 125, 250, 125, 125,
  125, 125, 375, 375,
  //End of Repeat
  
  250, 125, 375, 250, 125, 375,
  125, 125, 125, 125, 125, 125, 125, 125, 375,
  250, 125, 375, 250, 125, 375,
  125, 125, 125, 125, 125, 500,

  250, 125, 375, 250, 125, 375,
  125, 125, 125, 125, 125, 125, 125, 125, 375,
  250, 125, 375, 250, 125, 375,
  125, 125, 125, 125, 125, 500
};



int noteDuration[] = {
	406, 203, 203, 406, 203, 203, 406, 203, 203, 406,
	203, 203, 609, 203, 406, 406, 406, 406, 203, 203,
	203, 203, 609, 203, 406, 203, 203, 609, 203, 406,
	203, 203, 406, 203, 203, 406, 406, 406, 406, 406, 406
};
int rawSequence[] = {
	659, 493, 523, 587, 523, 493, 440, 440, 523,
	659, 587, 523, 493, 523, 587, 659, 523,
	440, 440, 440, 493, 523, 587, 698, 880, 783,
	698, 659, 523, 659, 587, 523, 493, 493,
	523, 587, 659, 523, 440, 440, 0
};


// Inicializa botao SW0 do kit
void init(void){
	// Initialize the board clock
	sysclk_init();
	
	// Desativa watchdog
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	// Configura o buzzer
	pmc_enable_periph_clk(BUZ_PIO_ID);
	pio_configure(BUZ_PIO, PIO_OUTPUT_0, BUZ_IDX_MASK, PIO_DEFAULT);
	
	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pmc_enable_periph_clk(BUT_PIO_ID);
	pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP);
	
	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pmc_enable_periph_clk(BUT_TROCA_PIO_ID);
	pio_configure(BUT_TROCA_PIO, PIO_INPUT, BUT_TROCA_IDX_MASK, PIO_PULLUP);

	//Inicializa PC8 como saída
	pio_set_output(BUZ_PIO, BUZ_IDX_MASK, 0, 0, 0);
}


int main(void)
{
	/* Initialize the SAM system */
	init();
	int troca = 0;
	while (1)
	{
		// Verifica valor do pino que o botão está conectado
		if(!pio_get(BUT_PIO, PIO_INPUT, BUT_IDX_MASK)){
			for (int i=0;i<18;i++){           
				int wait = duration1[i] * songspeed1;
				double voltas = 0;
				while (voltas<wait){
					if(notes1[i]!=0){
						pio_set(BUZ_PIO, BUZ_IDX_MASK);
						delay_us(500000/notes1[i]);
						pio_clear(BUZ_PIO, BUZ_IDX_MASK);
						delay_us(500000/notes1[i]);
						voltas += 1;
					}
					else{
						pio_clear(BUZ_PIO, BUZ_IDX_MASK);
						delay_ms(wait);
						voltas=wait;
					}
				}
			}
			delay_ms(1000);
			for (int i=0;i<40;i++){      
				int wait = noteDuration[i] * 0.5;
				double voltas = 0;
				while (voltas<wait){
					if(rawSequence[i]!=0){
						pio_set(BUZ_PIO, BUZ_IDX_MASK);
						delay_us(500000/rawSequence[i]);
						pio_clear(BUZ_PIO, BUZ_IDX_MASK);
						delay_us(500000/rawSequence[i]);
						voltas += 1;
					}
					else{
						pio_clear(BUZ_PIO, BUZ_IDX_MASK);
						delay_ms(wait);
						voltas=wait;
					}
				}
			}			
		}
	}
}

