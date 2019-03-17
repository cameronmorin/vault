#include <avr/io.h>
#include <avr/interrupt.h>
#include <timer.h>
#include "io.c"

//Global Variables
unsigned char correct;
unsigned char wrong;
unsigned char numWrong;
char sm1_output = 0x00;
unsigned char sm2_output = 0x00;
unsigned char sm3_output = 0x00;
unsigned char sm4_output = 0x00;
unsigned char press;
unsigned char doorClosed;
unsigned char lock;
unsigned char tmpA;
unsigned char tmpB = 0x00;

char displayName;
char currentPass;
char inputPass[4] = {'0', '0', '0', '0'};
char testPass[4] = {'0', '0', '0', '0'};
char testTwo[4] = {'6', '9', '6', '9'};

unsigned char cam = 0;
unsigned char bob = 0;

unsigned char cntDelay = 0;

typedef struct _pass {
	char password[4];
	char * userName;
} pass;

/*
static pass passCam, passHong, passHazel;
pass *passwords[] = { passCam, passHong, passHazel};
const unsigned short numUsers = sizeof(passwords)/sizeof(pass*);

passCam.password = "1234";
passCam.userName = "Cam";
passHong.password = "6969";
passHong.userName = "Hong";
passHazel.password = "0101";
passHazel.userName = "Hazel"
*/

//Bit access function
unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}

unsigned char GetBit(unsigned char x, unsigned char k) {
	return ((x & (0x01 << k)) != 0);
}

void checkPassword() {
	//pass *passCam = malloc(sizeof(pass));
	//pass *passwords[] = { passCam, passHong, passHazel};
	//const unsigned short numUsers = sizeof(passwords)/sizeof(pass*);
	//passCam->password[0] = '1';
	//passCam->password[1] = '2';
	//passCam->password[2] = '3';
	//passCam->password[3] = '4';
	//passCam->userName = "Cam";
	//passHong->password = "6969";
	//passHong->userName = "Hong";
	//passHazel->password = "0101";
	//passHazel->userName = "Hazel";

	if (sm1_output == testPass[0] &&
		sm2_output == testPass[1] &&
		sm3_output == testPass[2] &&
		sm4_output == testPass[3]) {
			tmpB = 0x01;
			PORTB = tmpB & 0x09;
			correct = 1;
			wrong = 0;
			cam = 1;
	}
	else if (sm1_output == testTwo[0] &&
			 sm2_output == testTwo[1] &&
			 sm3_output == testTwo[2] &&
			 sm4_output == testTwo[3]) {
		tmpB = 0x01;
		PORTB = tmpB & 0x09;
		correct = 1;
		wrong = 0;
		bob = 1;
	}
	else {
		correct = 0;
		wrong = 1;
		tmpB = 0x08;
		PORTB = tmpB;
	}

	/*
	if (strcmp(test, passCam->password) == 0) {
			strcat(displayName, passCam->userName);
			correct = 1;
			wrong = 0;
			PORTB = 0x01;
	}
	*/
	//PORTB = 0x01;
	/*
	for (unsigned short i = 0; i < 3; i += 1) {
		if (strcmp(inputPass, passwords[i]->password) == 0){
			displayName = "Welcome ";
			strcat(displayName, passwords[i]->userName);
			correct = 1;
			wrong = 0;
		}
	}
	*/
}


enum Keypad_States {Keypad_Start, Keypad_First, Keypad_Second, Keypad_Third, Keypad_Fourth, Keypad_Wait, Keypad_Lock, Keypad_Delay} stateKeypad;

void Tick_Keypad() {
	//Transition Actions
	switch(stateKeypad)	{
		case Keypad_Start:
			stateKeypad = Keypad_First;
			break;
		case Keypad_First:
			if (press) {
				inputPass[0] = sm1_output;
				stateKeypad = Keypad_Second;
			}
			break;
		case Keypad_Second:
			if (press) {
				inputPass[1] = sm2_output;
				stateKeypad = Keypad_Third;
			}
			break;
		case Keypad_Third:
			if (press) {
				inputPass[2] = sm3_output;
				stateKeypad = Keypad_Fourth;
			}
			break;
		case Keypad_Fourth:
			if (press) {
				inputPass[3] = sm4_output;
				stateKeypad = Keypad_Wait;
				press = 0;
			}
			break;
		case Keypad_Wait:
			if (correct && !wrong) {
				stateKeypad = Keypad_Lock;
				press = 0;
			}
			if (!correct && wrong) {
				stateKeypad = Keypad_First;
				press = 0;
			}
			break;
		case Keypad_Lock:
			if (doorClosed && lock) {
				lock = 0;
				stateKeypad = Keypad_Delay;
			}
			break;
		case Keypad_Delay:
			if (cntDelay >= 10) {
				stateKeypad = Keypad_First;
			}
		default:
			stateKeypad = Keypad_Start;
			break;
	}
	
	//State Actions
	switch(stateKeypad) {
		case Keypad_Start:
			break;
		case Keypad_First:
			press = 0;
			PORTC = 0xEF; // Enable col 4 with 0, disable others with 1�s
    		asm("nop"); // add a delay to allow PORTC to stabilize before checking
    		if (GetBit(PINC,0)==0) { sm1_output = '1'; press = 1; }
    		if (GetBit(PINC,1)==0) { sm1_output = '2'; press = 1; }
    		if (GetBit(PINC,2)==0) { sm1_output = '3'; press = 1; }
    		if (GetBit(PINC,3)==0) { sm1_output = 'A'; press = 1; }

    		// Check keys in col 2
    		PORTC = 0xDF; // Enable col 5 with 0, disable others with 1�s
    		asm("nop"); // add a delay to allow PORTC to stabilize before checking
    		if (GetBit(PINC,0)==0) { sm1_output = '4'; press = 1; }
			if (GetBit(PINC,1)==0) { sm1_output = '5'; press = 1; }
			if (GetBit(PINC,2)==0) { sm1_output = '6'; press = 1; }
			if (GetBit(PINC,3)==0) { sm1_output = 'B'; press = 1; }

    		// Check keys in col 3
   			PORTC = 0xBF; // Enable col 6 with 0, disable others with 1�s
    		asm("nop"); // add a delay to allow PORTC to stabilize before checking
			if (GetBit(PINC,0)==0) { sm1_output = '7'; press = 1; }
			if (GetBit(PINC,1)==0) { sm1_output = '8'; press = 1; }
			if (GetBit(PINC,2)==0) { sm1_output = '9'; press = 1; }
			if (GetBit(PINC,3)==0) { sm1_output = 'C'; press = 1; }
    		
    		// Check keys in col 4
			PORTC = 0x7F; // Enable col 6 with 0, disable others with 1�s
			asm("nop"); // add a delay to allow PORTC to stabilize before checking
			if (GetBit(PINC,0)==0) { sm1_output = '*'; press = 1; }
			if (GetBit(PINC,1)==0) { sm1_output = '0'; press = 1; }
			if (GetBit(PINC,2)==0) { sm1_output = '#'; press = 1; }
			if (GetBit(PINC,3)==0) { sm1_output = 'D'; press = 1; }
			break;
		case Keypad_Second:
			press = 0;
			PORTC = 0xEF; // Enable col 4 with 0, disable others with 1�s
    		asm("nop"); // add a delay to allow PORTC to stabilize before checking
    		if (GetBit(PINC,0)==0) { sm2_output = '1'; press = 1; }
    		if (GetBit(PINC,1)==0) { sm2_output = '2'; press = 1; }
    		if (GetBit(PINC,2)==0) { sm2_output = '3'; press = 1; }
    		if (GetBit(PINC,3)==0) { sm2_output = 'A'; press = 1; }

    		// Check keys in col 2
    		PORTC = 0xDF; // Enable col 5 with 0, disable others with 1�s
    		asm("nop"); // add a delay to allow PORTC to stabilize before checking
    		if (GetBit(PINC,0)==0) { sm2_output = '4'; press = 1; }
			if (GetBit(PINC,1)==0) { sm2_output = '5'; press = 1; }
			if (GetBit(PINC,2)==0) { sm2_output = '6'; press = 1; }
			if (GetBit(PINC,3)==0) { sm2_output = 'B'; press = 1; }

    		// Check keys in col 3
   			PORTC = 0xBF; // Enable col 6 with 0, disable others with 1�s
    		asm("nop"); // add a delay to allow PORTC to stabilize before checking
			if (GetBit(PINC,0)==0) { sm2_output = '7'; press = 1; }
			if (GetBit(PINC,1)==0) { sm2_output = '8'; press = 1; }
			if (GetBit(PINC,2)==0) { sm2_output = '9'; press = 1; }
			if (GetBit(PINC,3)==0) { sm2_output = 'C'; press = 1; }
    		
    		// Check keys in col 4
			PORTC = 0x7F; // Enable col 6 with 0, disable others with 1�s
			asm("nop"); // add a delay to allow PORTC to stabilize before checking
			if (GetBit(PINC,0)==0) { sm2_output = '*'; press = 1; }
			if (GetBit(PINC,1)==0) { sm2_output = '0'; press = 1; }
			if (GetBit(PINC,2)==0) { sm2_output = '#'; press = 1; }
			if (GetBit(PINC,3)==0) { sm2_output = 'D'; press = 1; }
			break;
		case Keypad_Third:
		press = 0;
			PORTC = 0xEF; // Enable col 4 with 0, disable others with 1�s
    		asm("nop"); // add a delay to allow PORTC to stabilize before checking
    		if (GetBit(PINC,0)==0) { sm3_output = '1'; press = 1; }
    		if (GetBit(PINC,1)==0) { sm3_output = '2'; press = 1; }
    		if (GetBit(PINC,2)==0) { sm3_output = '3'; press = 1; }
    		if (GetBit(PINC,3)==0) { sm3_output = 'A'; press = 1; }

    		// Check keys in col 2
    		PORTC = 0xDF; // Enable col 5 with 0, disable others with 1�s
    		asm("nop"); // add a delay to allow PORTC to stabilize before checking
    		if (GetBit(PINC,0)==0) { sm3_output = '4'; press = 1; }
			if (GetBit(PINC,1)==0) { sm3_output = '5'; press = 1; }
			if (GetBit(PINC,2)==0) { sm3_output = '6'; press = 1; }
			if (GetBit(PINC,3)==0) { sm3_output = 'B'; press = 1; }

    		// Check keys in col 3
   			PORTC = 0xBF; // Enable col 6 with 0, disable others with 1�s
    		asm("nop"); // add a delay to allow PORTC to stabilize before checking
			if (GetBit(PINC,0)==0) { sm3_output = '7'; press = 1; }
			if (GetBit(PINC,1)==0) { sm3_output = '8'; press = 1; }
			if (GetBit(PINC,2)==0) { sm3_output = '9'; press = 1; }
			if (GetBit(PINC,3)==0) { sm3_output = 'C'; press = 1; }
    		
    		// Check keys in col 4
			PORTC = 0x7F; // Enable col 6 with 0, disable others with 1�s
			asm("nop"); // add a delay to allow PORTC to stabilize before checking
			if (GetBit(PINC,0)==0) { sm3_output = '*'; press = 1; }
			if (GetBit(PINC,1)==0) { sm3_output = '0'; press = 1; }
			if (GetBit(PINC,2)==0) { sm3_output = '#'; press = 1; }
			if (GetBit(PINC,3)==0) { sm3_output = 'D'; press = 1; }
			break;
		case Keypad_Fourth:
			press = 0;
			PORTC = 0xEF; // Enable col 4 with 0, disable others with 1�s
			asm("nop"); // add a delay to allow PORTC to stabilize before checking
			if (GetBit(PINC,0)==0) { sm4_output = '1'; press = 1; }
			if (GetBit(PINC,1)==0) { sm4_output = '2'; press = 1; }
			if (GetBit(PINC,2)==0) { sm4_output = '3'; press = 1; }
			if (GetBit(PINC,3)==0) { sm4_output = 'A'; press = 1; }

			// Check keys in col 2
			PORTC = 0xDF; // Enable col 5 with 0, disable others with 1�s
			asm("nop"); // add a delay to allow PORTC to stabilize before checking
			if (GetBit(PINC,0)==0) { sm4_output = '4'; press = 1; }
			if (GetBit(PINC,1)==0) { sm4_output = '5'; press = 1; }
			if (GetBit(PINC,2)==0) { sm4_output = '6'; press = 1; }
			if (GetBit(PINC,3)==0) { sm4_output = 'B'; press = 1; }

			// Check keys in col 3
			PORTC = 0xBF; // Enable col 6 with 0, disable others with 1�s
			asm("nop"); // add a delay to allow PORTC to stabilize before checking
			if (GetBit(PINC,0)==0) { sm4_output = '7'; press = 1; }
			if (GetBit(PINC,1)==0) { sm4_output = '8'; press = 1; }
			if (GetBit(PINC,2)==0) { sm4_output = '9'; press = 1; }
			if (GetBit(PINC,3)==0) { sm4_output = 'C'; press = 1; }
			
			// Check keys in col 4
			PORTC = 0x7F; // Enable col 6 with 0, disable others with 1�s
			asm("nop"); // add a delay to allow PORTC to stabilize before checking
			if (GetBit(PINC,0)==0) { sm4_output = '*'; press = 1; }
			if (GetBit(PINC,1)==0) { sm4_output = '0'; press = 1; }
			if (GetBit(PINC,2)==0) { sm4_output = '#'; press = 1; }
			if (GetBit(PINC,3)==0) { sm4_output = 'D'; press = 1; }
			break;
		case Keypad_Wait:
			break;
		case Keypad_Lock:
			PORTC = 0xEF; // Enable col 4 with 0, disable others with 1�s
			asm("nop"); // add a delay to allow PORTC to stabilize before checking
			if (GetBit(PINC,0)==0) { sm4_output = '1'; lock = 0; }
			if (GetBit(PINC,1)==0) { sm4_output = '2'; lock = 0; }
			if (GetBit(PINC,2)==0) { sm4_output = '3'; lock = 0; }
			if (GetBit(PINC,3)==0) { sm4_output = 'A'; lock = 0; }

			// Check keys in col 2
			PORTC = 0xDF; // Enable col 5 with 0, disable others with 1�s
			asm("nop"); // add a delay to allow PORTC to stabilize before checking
			if (GetBit(PINC,0)==0) { sm4_output = '4'; lock = 0; }
			if (GetBit(PINC,1)==0) { sm4_output = '5'; lock = 0; }
			if (GetBit(PINC,2)==0) { sm4_output = '6'; lock = 0; }
			if (GetBit(PINC,3)==0) { sm4_output = 'B'; lock = 0; }

			// Check keys in col 3
			PORTC = 0xBF; // Enable col 6 with 0, disable others with 1�s
			asm("nop"); // add a delay to allow PORTC to stabilize before checking
			if (GetBit(PINC,0)==0) { sm4_output = '7'; lock = 0; }
			if (GetBit(PINC,1)==0) { sm4_output = '8'; lock = 0; }
			if (GetBit(PINC,2)==0) { sm4_output = '9'; lock = 0; }
			if (GetBit(PINC,3)==0) { sm4_output = 'C'; lock = 0; }
			
			// Check keys in col 4
			PORTC = 0x7F; // Enable col 6 with 0, disable others with 1�s
			asm("nop"); // add a delay to allow PORTC to stabilize before checking
			if (GetBit(PINC,0)==0) { sm4_output = '*'; lock = 0; }
			if (GetBit(PINC,1)==0) { sm4_output = '0'; lock = 0; }
			if (GetBit(PINC,2)==0) { sm4_output = '#'; lock = 1; }
			if (GetBit(PINC,3)==0) { sm4_output = 'D'; lock = 0; }
			break;
		case Keypad_Delay:
			break;
		default:
			break;
	}
}

enum LCD_States {LCD_Start, LCD_First, LCD_Second, LCD_Third, LCD_Fourth, LCD_Wait, LCD_Welcome, LCD_Error, LCD_Locked, LCD_Delay} stateLCD;

void Tick_LCD() {
	//Transition Actions
	switch(stateLCD)	{
		case LCD_Start:
			LCD_ClearScreen();
			LCD_DisplayString(1, "Password: ");
			stateLCD = LCD_First;
			break;
		case LCD_First:
			if (press) {
				LCD_Cursor(11);
				LCD_WriteData('*');
				//LCD_DisplayString(1, "*");
				//strcat(test, sm1_output);
				stateLCD = LCD_Second;
			}
			break;
		case LCD_Second:
			if (press) {
				LCD_Cursor(12);
				LCD_WriteData('*');
				//LCD_DisplayString(1, "**");
				//strcat(test, sm2_output);
				stateLCD = LCD_Third;
			}
			break;
		case LCD_Third:
			if (press) {
				LCD_Cursor(13);
				LCD_WriteData('*');
				//LCD_DisplayString(1, "***");
				//strcat(test, sm3_output);
				stateLCD = LCD_Fourth;
			}
			break;
		case LCD_Fourth:
			if (press) {
				LCD_Cursor(14);
				LCD_WriteData('*');
				//LCD_DisplayString(1, "****");
				//strcat(test, sm4_output);
				checkPassword();
				stateLCD = LCD_Wait;
			}
			break;
		case LCD_Wait:
			/*
			LCD_ClearScreen();
			PORTB = 0;
			LCD_DisplayString(1, "Hello");
			stateLCD = LCD_Welcome;
			*/
			if (correct == 1 && wrong == 0) {
				//LCD_DisplayString(1, displayName);
				stateLCD = LCD_Welcome;
			}
			else if (correct == 0 && wrong == 1) {
				//LCD_DisplayString(1, "ERROR");
				stateLCD = LCD_Error;
			}
			else {
				//LCD_DisplayString(1, "0000");
			}
			break;
		case LCD_Welcome:
			if (cam) {
				LCD_DisplayString(1, "Welcome Cam!");	
			}
			else {
				LCD_DisplayString(1, "Welcome Bob!");
			}
			numWrong = 0;
			stateLCD = LCD_Locked;
			break;
		case LCD_Error:
			LCD_DisplayString(1, "Access Denied");
			//set_PWM(261.63);
			//Write7Seg(SS_1);
			cntDelay = 0;
			numWrong += 1;
			stateLCD = LCD_Delay;
			break;
		case LCD_Locked:
			if (doorClosed && lock) {
				LCD_DisplayString(1, "Goodbye!");
				tmpB = 0x00;
				PORTB = tmpB & 0x08;
				cntDelay = 0;
				stateLCD = LCD_Delay;
			}
			break;
		case LCD_Delay:
			if (cntDelay >= 10) {
				cam = 0;
				bob = 0;
				tmpB = 0x00;
				PORTB = tmpB;
				//set_PWM(0);
				stateLCD = LCD_Start;
			}
			break;
		default:
			stateLCD = LCD_Start;
			break;
	}
	
	//State Actions
	switch(stateLCD) {
		case LCD_Start:
			break;
		/*
		case LCD_Running:
			switch (sm1_output) {
    			case '\0': break; // All 5 LEDs on
    			case '1': LCD_DisplayString(1, "1"); break; // hex equivalent
    			case '2': LCD_DisplayString(1, "2"); break;
				case '3': LCD_DisplayString(1, "3"); break;
				case '4': LCD_DisplayString(1, "4"); break;
				case '5': LCD_DisplayString(1, "5"); break;
				case '6': LCD_DisplayString(1, "6"); break;
				case '7': LCD_DisplayString(1, "7"); break;
				case '8': LCD_DisplayString(1, "8"); break;
				case '9': LCD_DisplayString(1, "9"); break;
				case 'A': LCD_DisplayString(1, "A"); break;
				case 'B': LCD_DisplayString(1, "B"); break;
				case 'C': LCD_DisplayString(1, "C"); break;
    			case 'D': LCD_DisplayString(1, "D"); break;
    			case '*': LCD_DisplayString(1, "*"); break;
    			case '0': LCD_DisplayString(1, "0"); break;
    			case '#': LCD_DisplayString(1, "#"); break;
    			default: LCD_DisplayString(1, "!"); break;
			}
			break;
		*/
		case LCD_First:
			break;
		case LCD_Second:
			break;
		case LCD_Third:
			break;
		case LCD_Fourth:
			break;
		case LCD_Wait:
			//FIXME: Boolean Logic here	
			//strcat(currentPass, sm1_output);
			//strcat(currentPass, sm2_output);
			//strcat(currentPass, sm3_output);
			//strcat(currentPass, sm4_output);
			
			//checkPassword(currentPass);
			//currentPass = "";
			break;
		case LCD_Welcome:
			//LCD_DisplayString(1, "Yeet");
			break;
		case LCD_Error:
		
			break;
		case LCD_Locked:
			
			break;
		case LCD_Delay:
			cntDelay ++;
		default:
			break;
	}
}

enum MagnetStates {Mag_Start, Mag_Read} stateMag;
	
void Tick_Magnet() {
	//Transition Actions
	switch(stateMag) {
		case Mag_Start:
			stateMag = Mag_Read;
			break;
		case Mag_Read:
			break;
		default:
			stateMag = Mag_Start;
			break;
	}
	
	//State Actions
	switch(stateMag) {
		case Mag_Start:
			stateMag = Mag_Read;
			break;
		case Mag_Read:
			tmpA = ~PINA;
			doorClosed = GetBit(tmpA, 2);
			break;
		default:
			break;
	}
}

/*
enum SpeakerStates {Spk_Start, Spk_Running, Spk_Delay} stateSpk;

void set_PWM(double frequency) {
	static double current_frequency; // Keeps track of the currently set frequency
	// Will only update the registers when the frequency changes, otherwise allows
	// music to play uninterrupted.
	if (frequency != current_frequency) {
		if (!frequency) { TCCR0B &= 0x08; } //stops timer/counter
		else { TCCR0B |= 0x03; } // resumes/continues timer/counter
		
		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) { OCR0A = 0xFFFF; }
		
		// prevents OCR0A from underflowing, using prescaler 64					// 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) { OCR0A = 0x0000; }
		
		// set OCR3A based on desired frequency
		else { OCR0A = (short)(8000000 / (128 * frequency)) - 1; }

		TCNT0 = 0; // resets counter
		current_frequency = frequency; // Updates the current frequency
	}
}

void PWM_on() {
	TCCR0A = (1 << COM0A0);
	// COM3A0: Toggle PB3 on compare match between counter and OCR0A
	TCCR0B = (1 << WGM02) | (1 << CS01) | (1 << CS00);
	// WGM02: When counter (TCNT0) matches OCR0A, reset counter
	// CS01 & CS30: Set a prescaler of 64
	set_PWM(0);
}

void PWM_off() {
	TCCR0A = 0x00;
	TCCR0B = 0x00;
}

void Tick_Spk(){
	//Transition Actions
	switch(stateSpk){
		case Spk_Start:
			tmpB = SetBit(tmpB, 3, 0);
			stateSpk = Spk_Running;
			break;
		case Spk_Running:
			break;
		case Spk_Delay:
			if (cntDelay >= 10) {
				tmpB = SetBit(tmpB, 3, 0);
				PORTB = tmpB;
				numWrong = 0;
				set_PWM(0);
				stateSpk = Spk_Running;
			}
		default:
			break;
	}

	//State Actions
	switch(stateSpk){
		case Spk_Start:
			break;
		case Spk_Running:
			if (wrong){
				//set_PWM(261.63);     //Set music note frequency to C_4
				tmpB = SetBit(tmpB, 3, 1);
				PORTB = tmpB;
				stateSpk = Spk_Delay;
			}
			else {
				//set_PWM(0);	    //Speak makes no sound
				break;
			}
			break;
		case Spk_Delay:
			break;
		default:
			break;
	}
	PORTB = tmpB & 0xF7;
}
*/

//Temporary variable used to hold input

unsigned char bA0;   //Variable to track the 1 bit input A0



int main(void){
	DDRA = 0xFB; PORTA = 0x04; // Configure port A's 8 pins as inputs, initialize to 1s
	DDRB = 0xFF; PORTB = 0x00; // Configure port B's 8 pins as outputs, initialize to 0s
    DDRC = 0xF0; PORTC = 0x0F; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines
	PORTB = 0; // Initialize outputs
	
	press = 0;
	tmpA = 0;
	correct = 0;
	wrong = 0;
	numWrong = 0;
	
	//PWM_on();
	TimerSet(25);
	TimerOn();
	LCD_init();
	
	while(1){
		//Tick Functions
		Tick_Keypad();
		Tick_LCD();
		Tick_Magnet();
		//Tick_Spk();
		//set_PWM(100);
		//set_PWM(261.63);
		
		while(!TimerFlag) {}
		TimerFlag = 0;
	}
}
