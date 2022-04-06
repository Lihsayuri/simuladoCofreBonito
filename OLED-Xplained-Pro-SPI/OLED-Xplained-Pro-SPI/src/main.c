#include <asf.h>
#include <time.h>
#include <string.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"


// Definindo tudo do botão 1:
#define BUT1_PIO PIOD
#define BUT1_PIO_ID ID_PIOD
#define BUT1_PIO_IDX 28
#define BUT1_PIO_IDX_MASK (1u << BUT1_PIO_IDX) // esse já está pronto.

// Definindo tudo do LED 1:
#define LED1_PIO           PIOA                 // periferico que controla o LED
// #
#define LED1_PIO_ID        ID_PIOA                 // ID do periférico PIOC (controla LED)
#define LED1_PIO_IDX       0                    // ID do LED no PIO
#define LED1_PIO_IDX_MASK  (1 << LED1_PIO_IDX)   // Mascara para CONTROLARMOS o LED

// Definindo tudo do botão 2:
#define BUT2_PIO PIOC
#define BUT2_PIO_ID ID_PIOC
#define BUT2_PIO_IDX 31
#define BUT2_PIO_IDX_MASK (1u << BUT2_PIO_IDX) // esse já está pronto.

// Definindo tudo do LED 2:
#define LED2_PIO           PIOC                 // periferico que controla o LED
// #
#define LED2_PIO_ID        ID_PIOC                 // ID do periférico PIOC (controla LED)
#define LED2_PIO_IDX       30                    // ID do LED no PIO
#define LED2_PIO_IDX_MASK  (1 << LED2_PIO_IDX)   // Mascara para CONTROLARMOS o LED

// Definindo tudo do botão 3:
#define BUT3_PIO PIOA
#define BUT3_PIO_ID ID_PIOA
#define BUT3_PIO_IDX 19
#define BUT3_PIO_IDX_MASK (1u << BUT3_PIO_IDX) // esse já está pronto.

// Definindo tudo do LED 3:
#define LED3_PIO           PIOB                 // periferico que controla o LED
// #
#define LED3_PIO_ID        ID_PIOB                 // ID do periférico PIOC (controla LED)
#define LED3_PIO_IDX       2                    // ID do LED no PIO
#define LED3_PIO_IDX_MASK  (1 << LED3_PIO_IDX)   // Mascara para CONTROLARMOS o LED


volatile int but1_flag = 0;
volatile int but2_flag = 0;
volatile int but3_flag = 0;
volatile char bloqueado = 0;
volatile char senha[6];
volatile int counter = 0;

void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq);
static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource);

void but1_callback(void) {
	but1_flag = 1;
}

void but2_callback(void) {
	but2_flag = 1;
}

void but3_callback(void) {
	but3_flag = 1;
}

void pin_toggle(Pio *pio, uint32_t mask) {
	if(pio_get_output_data_status(pio, mask))
	pio_clear(pio, mask);
	else
	pio_set(pio,mask);
}


void TC1_Handler(void) {
	volatile uint32_t status = tc_get_status(TC0, 1);

	/** Muda o estado do LED (pisca) **/
	pin_toggle(LED1_PIO, LED1_PIO_IDX_MASK);
	pin_toggle(LED2_PIO, LED2_PIO_IDX_MASK);
	pin_toggle(LED3_PIO, LED3_PIO_IDX_MASK);
}


void RTT_Handler(void) {
	uint32_t ul_status;

	/* Get RTT status - ACK */
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		bloqueado = 0;
		tc_stop(TC0, 1);
		gfx_mono_generic_draw_filled_rect(0, 0, 127, 31, GFX_PIXEL_CLR);
	}
	
	/* IRQ due to Time has changed */
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
		
	}

}

void configure_output(Pio *p_pio, const pio_type_t ul_type, const uint32_t ul_mask, const uint32_t ul_attribute, uint32_t ul_id ){
	pmc_enable_periph_clk(ul_id);
	pio_configure(p_pio, ul_type, ul_mask, ul_attribute);
}

void configure_input(Pio *p_pio, const pio_type_t ul_type, const uint32_t ul_mask, const uint32_t ul_attribute, uint32_t ul_id){
	pmc_enable_periph_clk(ul_id);
	pio_configure(p_pio, ul_type, ul_mask, ul_attribute);
	pio_set_debounce_filter(p_pio, ul_mask, 60);
}

void configure_interruption(Pio *p_pio, uint32_t ul_id, uint32_t ul_mask, uint32_t ul_attr, void (*p_handler) (uint32_t, uint32_t)){
	pio_handler_set(p_pio, ul_id, ul_mask, ul_attr, p_handler);
	pio_enable_interrupt(p_pio, ul_mask);
	pio_get_interrupt_status(p_pio);
	NVIC_EnableIRQ(ul_id);
	NVIC_SetPriority(ul_id, 4);
}


void io_init(void) {
	// Configura led
	configure_output(LED1_PIO, PIO_OUTPUT_0, LED1_PIO_IDX_MASK, PIO_DEFAULT, LED1_PIO_ID);
	configure_output(LED2_PIO, PIO_OUTPUT_0, LED2_PIO_IDX_MASK, PIO_DEFAULT, LED2_PIO_ID);
	configure_output(LED3_PIO, PIO_OUTPUT_0, LED3_PIO_IDX_MASK, PIO_DEFAULT, LED2_PIO_ID);


	// Inicializa clock do periférico PIO responsavel pelo botao
	
	configure_input(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE, BUT1_PIO_ID);
	configure_input(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE, BUT2_PIO_ID);
	configure_input(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE, BUT3_PIO_ID);
	
	configure_interruption(BUT1_PIO, BUT1_PIO_ID, BUT1_PIO_IDX_MASK, PIO_IT_FALL_EDGE, but1_callback);
	configure_interruption(BUT2_PIO, BUT2_PIO_ID, BUT2_PIO_IDX_MASK, PIO_IT_FALL_EDGE, but2_callback);
	configure_interruption(BUT3_PIO, BUT3_PIO_ID, BUT3_PIO_IDX_MASK, PIO_IT_FALL_EDGE, but3_callback);
}

void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq){
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	/* Configura o PMC */
	pmc_enable_periph_clk(ID_TC);

	/** Configura o TC para operar em  freq hz e interrupçcão no RC compare */
	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	/* Configura NVIC*/
	NVIC_SetPriority(ID_TC, 4);
	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);
}


static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource) {

	uint16_t pllPreScale = (int) (((float) 32768) / freqPrescale);
	
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	if (rttIRQSource & RTT_MR_ALMIEN) {
		uint32_t ul_previous_time;
		ul_previous_time = rtt_read_timer_value(RTT);
		while (ul_previous_time == rtt_read_timer_value(RTT));
		rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);
	}

	/* config NVIC */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 4);
	NVIC_EnableIRQ(RTT_IRQn);

	/* Enable RTT interrupt */
	if (rttIRQSource & (RTT_MR_RTTINCIEN | RTT_MR_ALMIEN))
	rtt_enable_interrupt(RTT, rttIRQSource);
	else
	rtt_disable_interrupt(RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN);
	
}

int senha_correta(char senha_definida [6]){
	for (int j = 0; j < 6; j++){
		if (senha[j] != senha_definida[j]){
			return 0;
		}
	}
	
	return 1;
}

void defina_senha(char senha_definida[6]){
	int x = 0;
	int i = 0;
	gfx_mono_draw_string("Defina senha", 0,5, &sysfont);
	while (i < 6){
		if (but1_flag){
			but1_flag = 0;
			senha_definida[i] = 1;
			x+=10;
			i+= 1;
			gfx_mono_draw_filled_circle(x, 25, 2, GFX_PIXEL_SET, GFX_WHOLE);
		}
		if (but2_flag){
			but2_flag = 0;
			senha_definida[i] = 2;
			i+=1;
			x+=10;
			gfx_mono_draw_filled_circle(x, 25, 2, GFX_PIXEL_SET, GFX_WHOLE);
		}
		if (but3_flag){
			but3_flag = 0;
			senha_definida[i] = 3;
			i+=1;
			x+=10;
			gfx_mono_draw_filled_circle(x, 25, 2, GFX_PIXEL_SET, GFX_WHOLE);
		}
		
	}
	
	gfx_mono_generic_draw_filled_rect(0, 0, 127, 31, GFX_PIXEL_CLR);
			
}


int main (void)
{
	board_init();
	sysclk_init();
	delay_init();
	io_init();
	gfx_mono_ssd1306_init();
	
	gfx_mono_draw_string("Cofre Closed", 0,5, &sysfont);

	int x = 0;
	
	WDT->WDT_MR = WDT_MR_WDDIS;


  // Init OLED
	gfx_mono_ssd1306_init();

	/* Leitura do valor atual do RTC */
	char senha_definida[6];
	char cofre_aberto = 0;
	int errou_pela_primeira = 0;
	
	defina_senha(senha_definida);

	while(1) {
		if (cofre_aberto){
			for (int i = 0; i <= 10000000; i++){
				if((!pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK)) && (i == 10000000)){
					gfx_mono_draw_string("Cofre Closed", 0,5, &sysfont);
					cofre_aberto = 0;
					but1_flag = 0;
				}
			}
		}
			
		if (!bloqueado && (!cofre_aberto)){
			pio_clear(LED1_PIO, LED1_PIO_IDX_MASK);
			pio_clear(LED2_PIO, LED2_PIO_IDX_MASK);
			pio_clear(LED3_PIO, LED3_PIO_IDX_MASK);
			gfx_mono_draw_string("Cofre Closed", 0,5, &sysfont);
				
			if (but1_flag){
				but1_flag = 0;
				senha[counter] = 1;
				counter+=1;
				x+=10;
				gfx_mono_draw_filled_circle(x, 25, 2, GFX_PIXEL_SET, GFX_WHOLE);
			}
			if (but2_flag){
				but2_flag = 0;
				senha[counter] = 2;
				counter+=1;
				x+=10;
				gfx_mono_draw_filled_circle(x, 25, 2, GFX_PIXEL_SET, GFX_WHOLE);
			}
			if (but3_flag){
				but3_flag = 0;
				senha[counter] = 3;
				counter+=1;
				x+=10;
				gfx_mono_draw_filled_circle(x, 25, 2, GFX_PIXEL_SET, GFX_WHOLE);
			}
				
		}
			
		
		
		but1_flag = 0;
		but2_flag = 0;
		but3_flag = 0;
		
		if (counter >= 6){
			gfx_mono_generic_draw_filled_rect(0, 0, 127, 31, GFX_PIXEL_CLR);
			counter = 0;
			x = 0;
			int conf = senha_correta(senha_definida);
			
			if (conf == 0) {
				gfx_mono_draw_string("Bloqueado", 0,5, &sysfont);
				bloqueado = 1;
				TC_init(TC0, ID_TC1, 1, 4);
				tc_start(TC0, 1);

				if (errou_pela_primeira == 0){
					RTT_init(4, 16, RTT_MR_ALMIEN);
					errou_pela_primeira = 1;
				} else{
					RTT_init(1, 10, RTT_MR_ALMIEN);
				}

			} else{
				gfx_mono_draw_string("Cofre aberto", 0,5, &sysfont);
				errou_pela_primeira = 0;

				pio_set(LED1_PIO, LED1_PIO_IDX_MASK);
				pio_set(LED2_PIO, LED2_PIO_IDX_MASK);
				pio_set(LED3_PIO, LED3_PIO_IDX_MASK);
				cofre_aberto = 1;
			}
		}
		

		
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);

		
	}  
				
}
	

			