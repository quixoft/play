// Test the way the ADC behaves with various configurations
// Julian Lewis 6th April 2016

#define CHUNK  100		// Number of ADC values to read each interrupt

uint16_t adc0[CHUNK];	// ADC values
uint16_t adc1[CHUNK]; 

static char txt[132];	// Serial out buffer
static int iflag = 0;	// Interrupt flag
static int chunk = 0;	// Chunk of data
static int wpsr  = 0;	// Write protect status register

#define INTERRUPT_PIN 8
#define CONVERT_PIN 11

void Trig_ISR() {
int i;
int val, ch;

	// Read a CHUNK of data values from both channels

	if (iflag == 0) {
		for (i=0; i<CHUNK; i++) {

			// We have to wait on both channels 0 and 1 individualy
			// and then read ONE value
			// We dont know however whats in LCDR yet channel 0 or 1
			// But thats OK because the channel is TAGged
			// If you want to use CDR[0] or CDR[1], you must only read
			// if the ISR bit for that channel is set, otherwise you get 
			// holes in your data. (Very tricky !!!)

			// Wait for channel 0 conversion

			digitalWrite(CONVERT_PIN,HIGH);
			while((ADC->ADC_ISR & 0x01)==0);
			
			// Read one value - it may not be for channel zero though !

			val = ADC->ADC_LCDR;
			ch = (val & 0x0F000) >> 12;	// The value is tagged with the channel
			if (ch) adc1[i]=val;		// But just because we waited for bit 0
			else    adc0[i]=val;		// doesn't ensure whats sitting in LCDR
			
			// Wait for channel 1 conversion

			while((ADC->ADC_ISR & 0x02)==0);
			
			// Read another value from the other channel
			// this is guaranteed because of the sequence defined in SEQR1/2

			val = ADC->ADC_LCDR;
			ch = (val & 0x0F000) >> 12;
			if (ch) adc1[i]=val;
			else    adc0[i]=val;

			digitalWrite(CONVERT_PIN,LOW);
#if 0
			while((ADC->ADC_ISR & 0x03)==0);// Bug - makes holes in the data
			adc0[i] = ADC->ADC_CDR[0];	// This wont work, see above
			adc1[i] = ADC->ADC_CDR[1];
#endif
		}
		iflag = 1;
	}
}

void setup() {        

	Serial.begin(9600);  

	// So because I have used USEQ and SEQRx I would have expected
	// to see 16 values pipe lined - in fact this is not the case
	// I have tried your old settings, they don't work, and I have
	// experimented with every config I can think of - this works!

	wpsr = ADC->ADC_WPSR;		// Clear errors
	ADC->ADC_WPMR  = 0x41444300;	// Write protect off
	ADC->ADC_MR    = 0x90080080;	// USEQ + FREERUN + Things to fiddle with
	ADC->ADC_EMR   = 0x01000000; 	// TAG On
	ADC->ADC_CHER  = 0x0000FFFF; 	// Enable all SEQR (Its not channels because of USEQ)
	ADC->ADC_SEQR1 = 0x01010101;	// 0,1,0,1.... conversion sequence
	ADC->ADC_SEQR2 = 0x01010101;	// ditto
	wpsr = ADC->ADC_WPSR;		// Check this is zero

	pinMode(CONVERT_PIN,OUTPUT);
	attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN),Trig_ISR,RISING);
}

void loop() {
int j, ch, val;

	if (iflag) {
		
		// Dump data to the serial line, the channel numbers
		// should correspond because the data is TAGged

		for (j=0; j<CHUNK; j++) {
			val = adc0[j];
			ch  = (val & 0x0F000) >> 12;
			sprintf(txt,"CH%X[%2d]:%d\n",ch,j,val & 0xFFF);
			Serial.print(txt);
		}
		Serial.print("\n");
		for (j=0; j<CHUNK; j++) {
			val = adc1[j];
			ch  = (val & 0x0F000) >> 12;
			sprintf(txt,"CH%X[%2d]:%d\n",ch,j,val & 0xFFF);
			Serial.print(txt);
		}
		Serial.println("EOF");	// For Python

		iflag = 0;		// Keep repeating
	}
}
