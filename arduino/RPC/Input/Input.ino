//Include if necessary

//#include<cyclops.h>

/*
	Global Variables
*/

volatile byte Proc1 = 0;
volatile byte Proc2 = 0;
extern volatile byte FLAG = 0;
bool READY = 0;
unsigned int EEPROM_Start = 0;
unsigned int EEPROM_Ene = 0;

int Nsets =0 ;// number of additional Argument blocks global for debug 
byte StartChar =0 ; // global for debug
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
 my motiavtions for limiting this are debatable
The argumets can be extracted using some functions below or by just memcpy the section of the array onto a structure 
*/
byte Arguments[64]; 


/*
RPC FORMAT ::
1 Start Byte :: Arbitrary
2 Procedure Bytes
5 Number of arguments Bytes(4 Bytes for Arguments / 1  byte = no. more argument blocks of 16 bytes)
1 Stop Byte :: Arbitrary 

EXTRA ARGUMENTS FORMAT :: 
16 Bytes (16 bytes For Arguments will read in multiples of num sets specified above)

This is designed around 1 byte long standard UART implementations  
*/


/* 	Setup and Loop
		Sticking to The arduino format In order to keep it simple
	Setup::	Please add all module initialisation functions here or implement procedure
			calls for the same
	Void :: This is the main function Waits and parses through the input buffer and
			appropriately loades the new procedure
*/	

/*
To Test the Code pass the following via any serial monitor code and varify the output ascii values
"R0112340S" == R -- proc1 = 0 -- proc2 == 1 -- args = 1234 -- 0 add 16 byte arg blocks -- S
"R0112341SABCDEFGHIJKLMNOP" == R -- proc1 = 0 -- proc2 == 1 -- args = 1234 -- 0 add 16 byte arg blocks -- S -- 1 blck args == ABCDEFG..OP
*/

void setup(){
	Serial.begin(115200);
}

void loop(){
	if(Serial.available()>=9){ // a good flag for avaislable serial will any way modify isr to signal end of recieved input
		StartChar = Serial.read();
		if((char)StartChar == 'R'){ // Checcking for 
			Nsets = ParseInput();
			Nsets =	Nsets -48; // For Temp DBG with MONITOR TEXT ENTRY
			int i=0;
			while(i<Nsets){
				if(Serial.available()>=16){
					ParseArguments(i);
					i++;
					}
				}
			/*
			// Debug to varify the integrity of the read data
			for(int i=0;i<(4+(16*Nsets));i++){
				Serial.print("  ");
				Serial.print(Arguments[i]);
				}
			Serial.println();
			*/
			if (Nsets!=-1) READY = 1;
			// Flag to signify correct input and ready to execute= 1;
			}
		// this is here in order to help make the code human readable will print for every invalid byte :P
		else{
			Serial.println("Format :: \"R\'Proc1\'\'Proc2\'\'Arg Bytes 0-4\'\'num blocks of add bytes\'S");
			}
		}
	if(READY){
		READY = 0; //  since we have started handling the exe req
		Serial.println("checking Validity of procedure call id :: ");
		// this If Else will be replaced with a neat funtion array indexd implemenataion :)
		//FP[Proc1][proc2](Arguments); <- like this 
		if((Proc1 == '0') && (Proc2 == '0')){
			EchoInput(Arguments); // test ipt :: "R0012340S" 
			}
		else if((Proc1 == '0') && (Proc2 == '1')){
			EchoLong(Arguments); // test ipt :: "R0112340S"
			}
		else if((Proc1 == '0') && (Proc2 == '2')){
			EchoDouble(Arguments); // test ipt :: "R0212340S" 
			}
		else if((Proc1 == '0') && (Proc2 == '3')){
			Echo5Longs(Arguments); // test ipt :: "R0312341SABCDEFGHIJKLMNOP"
			}
		
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
	if((char)Stop == 'S') return AddArgs; // misc Error Checking :P
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
	return (int)((*Pointer<<8)+(*Pointer));
}

unsigned int byte2uint(byte * Pointer){ // combines two bytes to a 16 bit unsigned integer 
	return (unsigned int)((*Pointer<<8)+(*Pointer));
} 

long byte2long(byte * Pointer){ // combines two bytes to a 16 bit integer 
	long temp;
	memcpy(&temp , Pointer,sizeof(temp));
	return temp ;// TODO
}

unsigned long byte2ulong(byte * Pointer){ // combines two bytes to a 16 bit integer 
	unsigned long temp ;
	memcpy(&temp , Pointer,sizeof(temp));
	return temp; // TODO cp from above case
}

double byte2double(byte* Pointer){
	double temp ;
	memcpy(&temp , Pointer,sizeof(temp));
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
	return (Serial.available()>=9); // the size of an input packet
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

/*	Sapmle Function Implementations 
	Procedure calls grouped together for convenience and modularity(by type / utility)
	Note ::  please add your function to the function pointer array to enable calling it
*/

// General Utility Functions 
	/*
		calls to setup utils for the Dac and other modules here etc..
	*/

FunctionSet Gen_Util[] = {};

// General Debug Functions
	/*
		EchoUART
		TestWave2Dac
		Stop
		EchoInput
		EchoDouble
		EchoLong
		Echo5Longs
	*/

void EchoInput(byte* inputs){
	for(int i=0;i<(4+(16*Nsets));i++){
		Serial.print("  ");
		Serial.print(inputs[i]);
		}
	Serial.println();			
	}

void EchoLong(byte * inputs){
	Serial.println(byte2long(inputs));
}

void EchoDouble(byte * inputs){
	Serial.println(byte2double(inputs));	
}

void Echo5Longs(byte * inputs){
	Serial.println(byte2long(inputs));
	Serial.println(byte2long(inputs+4));
	Serial.println(byte2long(inputs+8));
	Serial.println(byte2long(inputs+12));
	Serial.println(byte2long(inputs+16));
}

FunctionSet Gen_Debug[] = {EchoInput,
						   EchoDouble,
						   EchoLong,
						   Echo5Longs};

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
	memcpy (&sinewave,inputs,sizeof(sinewave));
	//starttime = get time;
	/*
	loop(check timeout || trrmination){
		//output part of the sin wave;
	
	}
	*/
	return ; 
}

void SquareWave(byte* inputs){

}

void Triangle(byte* inputs){
	
}

void SigPWM(byte* inputs){

}

void SigMEM1(byte* inputs){
// devide the 1kb space into 2 segments to save 2 waves i can adda bit to just save one large wave also
}

void SigMEM2(byte* inputs){

}

FunctionSet Sig_Simple[] = { Sinousoid,
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

void SigAM(byte* inputs){

}

void SigFM(byte* inputs){

}

void SigPCM(byte* inputs){

}

void TDM_AM(byte* inputs){

}

FunctionSet Sig_Cplx[] = {SigAM,
						  SigFM,
						  SigPCM,
						  TDM_AM
                          };

// Utility to Read from and store sample waveform to the EEPROM

void WriteEEPROMwave(byte* inputs){

}
void EEPROM_Status(byte* inputs){

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