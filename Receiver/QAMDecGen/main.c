/*
 * QAMDecGen.c
 *
 * Created: 20.03.2018 18:32:07
 * Author : Martin Burger
 */ 

//#include <avr/io.h>
#include "avr_compiler.h"
#include "pmic_driver.h"
#include "TC_driver.h"
#include "clksys_driver.h"
#include "sleepConfig.h"
#include "port_driver.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "stack_macros.h"

#include "mem_check.h"

#include "init.h"
#include "utils.h"
#include "errorHandler.h"
#include "NHD0420Driver.h"

#include "qaminit.h"
#include "qamgen.h"
#include "qamdec.h"


extern void vApplicationIdleHook( void );
void vLedBlink(void *pvParameters);
void vGetPeak( void *pvParameters );
void vGetDifference( void *pvParameters);

TaskHandle_t handler;

uint16_t array1[22][32] = {NULL};	// zwischenl�sung f�r 22 Waves und je 32Werte f�r weiterbearbeitung; sollte durch den Queue gef�llt werden
uint16_t array2[22] = {NULL};		// den arrayplatz speichern an welcher Stelle der Peak ist f�r reveserse engineering welcher Bitwert

void vApplicationIdleHook( void )
{	
	
}

int main(void)
{
	resetReason_t reason = getResetReason();

	vInitClock();
	vInitDisplay();
	
	initDAC();			// 2B commented out during real-testing, saving some space and further
	initDACTimer();		// 2B commented out during real-testing, saving some space and further
	initGenDMA();		// 2B commented out during real-testing, saving some space and further
	initADC();
	initADCTimer();
	initDecDMA();
	
	xTaskCreate(vQuamGen, NULL, configMINIMAL_STACK_SIZE + 500, NULL, 2, NULL);		// 2B commented out during real-testing, saving some space and further
	xTaskCreate(vQuamDec, NULL, configMINIMAL_STACK_SIZE + 100, NULL, 1, NULL);
	xTaskCreate(vGetPeak, NULL, configMINIMAL_STACK_SIZE + 250, NULL, 1, NULL);
	xTaskCreate(vGetDifference, NULL, configMINIMAL_STACK_SIZE + 250, NULL, 1, NULL);


	vDisplayClear();
	vDisplayWriteStringAtPos(0,0,"FreeRTOS 10.0.1");
	vDisplayWriteStringAtPos(1,0,"EDUBoard 1.0");
	vDisplayWriteStringAtPos(2,0,"QAMDECGEN-Base");
	vDisplayWriteStringAtPos(3,0,"ResetReason: %d", reason);
	vTaskStartScheduler();
	return 0;
}

void vGetPeak( void *pvParameters ) {												// Peaks aus dem Array mit allen 22 Wellen lesen und diese in einem Weiteren Array abspeichern
	uint16_t actualPeak = 0;														// Zwischenspeicher des h�chsten Werts
	// How to get pointeradress[0] from different Task?
	// Mutex damit diese Daten gesperrt sind w�hrend Bearbeitung
	for (int a = 0; a >= 21; a++) {			// for 22 waves with data
		for (int b = 0; b >= 31; b++) {		// for 32 samples per wave
			if(array1[a][b] > actualPeak) {
				actualPeak = array1[a][b];
				array2[a] = b;
			}
			actualPeak = 0;															// F�r n�chste Runde wider auf 0 damit wieder hochgearbeitet werden kann
		}
		vTaskDelay(100/ portTICK_RATE_MS );
	}
	// Mutex Ende
	// vTaskSuspend;																// Damit keine Resourcen besetzt wenn nicht n�tig
}

void vGetDifference( void *pvParameters ) {
	
	vTaskDelay( 100 / portTICK_RATE_MS );
	//vTaskSuspend;																	// Damit keine Resourcen besetzt wenn nicht n�tig
}
	
/* Aktuell nur zur Speicherung von diversen ideen
speicher[0] = bufferelement[0];
speicher[1] = speicher[0];
speicher[2] = speicher[1];
speicher[3] = speicher[2];
if(speicher[0] > (AD-WERT/1.7)) {	// St�rungen filtern 
	if(speicher[0] > speicher[3]) {	// steigend erkennen und dass langsam interessant wird		// was passiert bei 0 -> direkt peak dann fallend? Geht nicht
						// else if(wenn direkt auf n�he maximum und dann (fallende Flanke?)
		if (bufferelement[a] > posPeakelement) {
			posPeakelement = bufferelement[a];										// peakwert speichern
			posPeak[sigCount] = runner;												// durchlaufz�hlerwert speichern (fehleranf�llig, da mit Task = Zeitproblematik)
						// speichern addresse in Array								// von 32 m�glichen Pl�tzen an x/32
		}
						// fallende Flanke erkennen, posPeak[sigCount] +1 im array f�r n�chsten Peak
	}
}
*/

/*
posPeak[x+1] - posPeak[x] = Abstand => innerhalb +- 8 feldern?
*/

