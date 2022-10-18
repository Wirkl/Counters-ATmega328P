// MACROS
#define F_CPU 16000000UL // 16MHz FRECUENCY

#define programbutton (PINC & (1<<PC3))>>PC3		// PROGRAM BUTTON
#define resetbutton (PINC & (1<<PC4))>>PC4			// RESET BUTTON
#define startpausebutton (PINC & (1<<PC5))>>PC5		// START/PAUSE BUTTON
#define timedownbutton (PINC & (1<<PC4))>>PC4		// TIME DOWN BUTTON
#define timeupbutton (PINC & (1<<PC5))>>PC5			// TIME UP BUTTON
#define stopbutton (PINC & (1<<PC3))>>PC3			// STOP BUTTON

#define TIME 5 // MULTIPLEXING TIME

// LIBRARYS
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

//FUNCTIONS AND SUBROUTINES
void startpause(bool &); // TO START/PAUSE THE COUNTER
void reset(unsigned int &, unsigned int); // TO RESET THE COUNTER
bool stop(); // TO STOP THE COUNTER
void program(unsigned int); // TO PROGRAM T_ON AND T_OFF
void mostrar_numero(unsigned int); // TO MULTIPLEXING THE COUNTER IN 4 DISPLAYS
void changetime(unsigned int &); // TO INCREASE AND DECREASE THE COUNTER IN PROGRAM MODE
bool program(); // TO SET T_ON AND T_OFF COUNTER TIME
void delayms(unsigned int);	// DELAY

//NUMBER DECODER VECTOR
int code[10] ={
	0x00,  // 0
	0x02,  // 1
	0x04,  // 2
	0x06,  // 3
	0x08,  // 4
	0x0A,  // 5
	0x0C,  // 6
	0x0E,  // 7
	0x10,  // 8
	0x12}; // 9

//MAIN FUNCTION
int main(void){
	DDRB |= 0b00011111; //PB0-PB4 AS OUTPUT OTHERS AS INPUT
	DDRD |= 0b11100000; //PD5-PD7 AS OUTPUT OTHERS AS INPUT
	DDRC |= 0b00000111; //PC0-PC2 AS OUTPUT OTHERS AS INPUT
	
	//LOCAL VARIABLES
	unsigned int T_on = 0000, T_off = 0000, state=1, aux1=0, aux2=0; 
	bool pause = false; 
    
	//LOOP
	while (1){
		pause = false;
		
		switch (state){
			case 1:
				mostrar_numero(T_on); //SHOW T_on VALUE IN DISPLAYS
				changetime(T_on); //INCREASE OR DECREASE THE T_on VALUE
				PORTC = (1<<PC1); //TOGGLE ON T_ON GREEN LED
				if(T_on!=0) if(program()){ //WHEN T_on!=0 ALLOWS SETUP T_on
					state = 2;
					aux1=T_on;
				}
			break;
			
			case 2:
				mostrar_numero(T_off); //SHOW T_off VALUE IN DISPLAYS
				changetime(T_off); //INCREASE OR DECREASE THE T_off VALUE
				PORTC = (1<<PC0); //TOGGLE ON T_ff RED LED
				if(T_off!=0) if(program()){ //WHEN T_off!=0 ALLOWS SETUP T_off
					state = 3;
					aux2=T_off;
				}
			break;	
			
			case 3:
				PORTC = (1<<PC1); //TOGGLE ON T_off GREEN LED
				for(int i=0;i<(1000/(TIME*4));i++){ //1000ms COUNTER WITHOUT APPLYING MORE DELAYS
					startpause(pause); //CHANGE pause TRUE OR FALSE VALUE 
					reset(aux1, T_on); //RESET aux1 TO T_on VALUE
					if (stop()) aux1 = 0; //WHEN STOP=TRUE >> aux1=0
					mostrar_numero(aux1); //SHOW aux1 VALUE IN DISPLAYS
					if(pause) i--; //WHEN pause=TRUE, STOPS THE 1000ms COUNTER
				}
				if(aux1==0){
					PORTC |= (1<<PC2); //TOGGLE ON THE RELAY
					state = 4;
				}
				aux1--;
			break;
			
			case 4:
				PORTC = (1<<PC2) | (1<<PC0); //TOGGLE ON T_off RED LED AND RELAY
				for(int i=0;i<(1000/(TIME*4));i++){ //1000ms COUNTER WITHOUT APPLYING MORE DELAYS
					startpause(pause); //CHANGE pause TRUE OR FALSE VALUE 
					reset(aux2, T_off); //RESET aux2 TO T_off VALUE
					if (stop()) aux2 = 0; //WHEN STOP=TRUE >> aux2=0
					mostrar_numero(aux2); //SHOW aux2 VALUE IN DISPLAYS
					if(pause) i--; //WHEN pause=TRUE, STOPS THE 1000ms COUNTER
				}
				if(aux2==0){
					PORTC = (0<<PC2); //TOGGLE OFF THE RELAY
					state = 1;
				}
				aux2--;
			break;
		}
    }
}

//START AND PAUSE FUNCTION
void startpause(bool &pause){
	static uint16_t buttondebounce = 0;
	buttondebounce = (buttondebounce<<1) | startpausebutton | 0xe000;
	if (buttondebounce==0xf000) pause = !pause;
}

//STOP FUNCTION
//RETURNS BOOL VALUE
bool stop(){
	static uint16_t buttondebounce = 0;
	buttondebounce = (buttondebounce<<1) | stopbutton | 0xe000;
	if (buttondebounce==0xf000) return true;
	else return false;
}

//RESET FUNCTION
void reset(unsigned int &valoractual, unsigned int valorinicial){
	if(resetbutton == 1) valoractual = valorinicial;
}

//CHANGE TIME FUNCTION
void changetime(unsigned int &numero){
	static uint16_t button1debounce = 0, button2debounce = 0;
	 
	button1debounce = (button1debounce<<1) | timeupbutton | 0xe000;
	if (button1debounce==0xf000){
		if(numero==9999) numero=0;
		else if(numero<9999) numero++;
	}
	button2debounce = (button2debounce<<1) | timedownbutton | 0xe000;
	if (button2debounce==0xf000){
		if(numero==0) numero = 9999;
		else if(numero>0) numero--;
	}
}

//PROGRAM FUNCTION 
//RETURNS BOOL VALUE
bool program(){
	static uint16_t buttondebounce = 0;
	buttondebounce = (buttondebounce<<1) | programbutton | 0xe000;
	if (buttondebounce==0xf000) return true;
	else return false;
}

//SHOW NUMBER IN DISPLAYS
void mostrar_numero(unsigned int numero){
	unsigned int unidades = numero%10;
	unsigned int decenas = numero/10%10;
	unsigned int centenas = numero/100%10;
	unsigned int miles = numero/1000%10;
	
	PORTB = code[unidades] | 0b00000001;
	delayms(TIME);
	
	PORTD = 0b10000000;
	PORTB = code[decenas];
	delayms(TIME);

	PORTD = 0b01000000;
	PORTB = code[centenas];
	delayms(TIME);
	
	PORTD = 0b00100000;
	PORTB = code[miles];
	delayms(TIME);
	PORTD = 0x00;
}

//DELAY FUNCTION
void delayms(unsigned int n) {
	while(n--) {
		_delay_ms(1);
	}
}