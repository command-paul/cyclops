//Include if necessary

#include<cyclops>

/*
	Global Variables
*/

volatile byte Proc1 = ;
volatile byte Proc2 = ;

volatile byte EXECUTING;
extern volatile byte FLAG = ;

unsigned int EEPROM_Start = 0;
unsigned int EEPROM_Ene = 0;
//Global Variables End

/*
	Structs / Type Defs
*/

typedef void (*FunctionSet)(byte*);

typedef struct SimpleWave { // size limited to 4 Bytes to be done without an arguments segment
	unsigned int frequency;
	byte timeout;
	byte amplitude;
}SimpleWave;

typedef struct wave {  
	unsigned int frequency;
	unsigned int timeout;
	unsigned int amplitude;
}wave;
// Typedefs end


/*
The Main Arguments Buffer
the arduino has a 64 byte buffer :P can be increased in the board config

The argumets can be extracted using some functions below or by just memcpy the section of the array onto a structure 
*/
byte Arguments[64]; 


/*
RPC FORMAT ::
1 Start Byte :: Arbitrary
2 Procedure Bytes
5 Number of arguments Bytes(4 Bytes for Arguments / 1  byte = no. more argument blocks)
1 Stop Byte :: Arbitrary 

EXTRA ARGUMENTS FORMAT :: 
17 Bytes (16 bytes For Arguments / 1 Byte =  More Arguments)

This is designed around 1 byte long standard UART implementations  
*/


/* 	Setup and Loop
		Sticking to The arduino format In order to keep it simple
	Setup::	Please add all module initialisation functions here or implement procedure
			calls for the same
	Void :: This is the main function Waits and parses through the input buffer and
			appropriately loades the new procedure
*/	

void setup(){
	Serial.begin(115200);
}

void loop(){
	if(Serial.available()){ // a good flag for avaislable serial will any way modify isr to signal end of recieved input
		if(Serial.read() == 'R'){ // Checcking for 
			int Nsets = ParseInput();
			int 1=0;
			while(i<nsets){
				ParseArguments(i);
			}
		Ready = 1;
		}
	}
	if(!EXECUTING && INPUTREADY){
		FP[OpCall](Arguments , Nargs);
		// call function and pass in arguments 
	}
}

/* 	The main Parse input Function
	breaks down the Input into the format specified above
	Returns if additional inputs true
*/
int ParseInput(){
	Proc1 = Serial.read();
	Proc2 = Serial.read();
	for(int i=0;i<4;i++){
		Arguments[i] = Serial.read(); 
	}
	byte AddArgs = Serial.read();
	byte Stop = Serial.read();
	if(Stop == 'S') return AddArgs; // misc Error Checking :P
	else return -1;
}
	
/* 
	Parses throught the input stream and appends arguments to the arguments array
*/

int ParseArguments(int set){
	int i=0;
	for(i=0;i<16;i++){
		Arguments[4+(set*16)+i] = Serial.read(); 
	}
}

/* 
	Functions to parse input and convert big endian byte stream to required types 
	these can be called inside functions to convert sections of the argument array 
	to known datatypes
	Alternate Modular approach :: declare a structure and simply memcopy the argument 
	array over the structure
*/
int byte2int(byte * Pointer){ // combines two bytes to a 16 bit integer 
	return (int)((*Pointer<<8)+(*pointer));
}

unsigned int byte2uint(byte * Pointer){ // combines two bytes to a 16 bit unsigned integer 
	return (unsigned int)((*Pointer<<8)+(*pointer));
} 

long byte2long(byte * Pointer){ // combines two bytes to a 16 bit integer 
	return (long) Pointer // TODO
}

unsigned long byte2ulong(byte * Pointer){ // combines two bytes to a 16 bit integer 
	return (unsigned long) Pointer // TODO cp from above case
}

double byte2double(byte* Pointer){
	double temp ;
	memcpy(temp , byte2long(Pointer),4)
	return temp;
}

// End of parse functions

/* 
	Functions to handle termination 
	1	New Availabe inst ::	Check if enough bytes have loaded for a new instruction 
								(ISR/Read stop byte approaches will not work without changing 
								core Standard libs - these changes as they cant be done by an
								average user - path abandoned) Danger Critical section will 
								cause this bit to return when the result is not completly available
	2	Time out enable  + Timeout check

*/
bool ChkTerminate(){
	// check if the previous input has been bufered by looking for the top of the stack  
	// all protected variables cant expect new users to modify them / approach of making a new board config will not work sinnce that bit is common to all boards
	return (Serial.available()>=PSIZE);
}

long Timer;
void StartTimer(){
	Timer = millis();
} 
bool ChkTimeout(long int TOUT){
	return ((millis()-Timer)>=TOUT);
}	
// End of Termination Functions


/*	Critical Section app for Reliability improvement
	Disables interrupts till EndCriticalSection() is called
	in order to make code more human readable  can be expanded in the future when we can 
	implement an RTOS and multithread
	Dangerous :: may Result in dropped packets (and a bit of edge latence in the exec of 
	the next call)
*/
void StartCrtiticalSection(){ noInterrupts(); }
void EndCriticalSection(){ interrupts(); }
// End of Critical Section Functions

/*	Sample Function Implementations 
	Procedure calls grouped together for convenience and modularity(by type / utility)
	Note ::  please add your function to the function pointer array to enable calling it
*/

// Simple Wave Functions
/*
	Sine wave
	Triangulat rave
	Squre wave
	Slur rate and slew rate control
	priscision in the output signal
*/
void Sinousoid(byte* inputs){
	SimpleWave sinewave;
	// section that parses input for relevant data usinng the memcpy approach for general funtions
	memcpy (inputs sinewave,sizeof(sinewave));
	//starttime = get time;
	loop(check timeout || trrmination){
		output part of the sin wave;
	}
	return ; 
}
void SquareWave(){

}

void Triangle(){
	
}

void SigPWM(){

}

void SigMEM1(){
// devide the 1kb space into 2 segments to save 2 waves i can adda bit to just save one large wave also
}

void SigMEM2(){

}

FunctionSet SignalGen[] = { Sinousoid,
							SquareWave,
							Triangle,
							SigPWM,
							SigMEM1,
							SigMEM2
							};

// Complex Waveforms [Signal Modulation toolkit ] (ones that take an additional argumetns buffer)
/*
AM 
FM
PWM
*/

void SigAM(){

}

void SigFM(){

}

void SigPCM(){

}

void TDM_AM(){

}

FunctionSet SIG_CPLX[] = {SigAM,
						  SigFM,
						  SigPCM,
						  TDM_AM
                          };

// Utility to Read from and store sample waveform to the EEPROM

void WriteEEPROMwave(byte* inputs){

}
void EEPROM_Status(){

}

FunctionSet EEPROM_Util[] = { WriteEEPROMwave,
							  EEPROM_Status
							  };

// 


//End of Sample Function implementations                          


/*	Problems / sols / Comments / misc
// Looks like all the buffers and counters in the serial library are protected and hence cant be accessed without modifying the librbray
// work around by either modifying the isr or by adding a last read/ flag variable
// Suggest writing new isr and making a new arduino uno - cyclops board config file
*/