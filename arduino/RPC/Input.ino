/*
Include if necessary
#include<cyclops>
*/

/*
	Global Variables
*/
volatile short OpCall = ;
volatile short Nargs = ;
volatile short EXECUTING;
extern volatile short FLAG = ;
//Global Variables End

/*
	Type Defs
*/
typedef void (*FunctionSet)(short*);
// Typedefs end

/*
The Main Arguments Buffer
the arduino has a 64 byte buffer :P can be increased in the board config 
*/
short Arguments[64]; 

/*
RPC FORMAT ::
1 Start Byte :: Arbitrary
2 Procedure Bytes
5 Number of arguments Bytes(4 Bytes for Arguments / 1  byte = more arguments)
1 Stop Byte :: Arbitrary nonprintable char

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
	if(Serial.available()){ // a good flag for available serial will any way modify isr to signal end of recieved input
		while(Serial.read()!='l');// Start Byte - Problem infinite loop for random text input
		ParseInput();
		// toggle flag to ready to execute 
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
void ParseInput(){
	OpCall = Serial.read();
	int i= Nargs = Serial.read();
	while(i){
		Arguments[Nargs-i] = Serial.read(); // Expects the input in a big endian format
		i--;
	}
	while(Serial.read()!='l'); // End of Transmission byte
	// flush bauffer
}

/* 
	Parses throught the input stream and appends arguments to the arguments array
*/

int ParseArguments(){

}

/* 
	Functions to parse input and convert big endian byte stream to required types 
	these can be called inside functions to convert sections of the argument array 
	to known datatypes
	Alternate Modular approach :: declare a structure and simply memcopy the argument 
	array over the structure
*/
int Short2int(short * Pointer){ // combines two bytes to a 16 bit integer 
	return (int)((*Pointer<<8)+(*pointer));
}

uint Short2int(short * Pointer){ // combines two bytes to a 16 bit unsigned integer 
	return (uint)((*Pointer<<8)+(*pointer));
} 

long Short2long(short * Pointer){ // combines two bytes to a 16 bit integer 
	return (long) Pointer
}

unsigned long Short2long(short * Pointer){ // combines two bytes to a 16 bit integer 
	return (unsigned long) Pointer
}

double short2double(short* Pointer){
	double temp ;
	memcpy(temp , Short2long(Pointer),1)
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

// Functions of Infinite exectime

void GenSin100(int a) { 
  Serial.println(a);
  return a;
}

FunctionSet SignalGen[] = {GenSin100,
                           GenSin100,
                           GenSin100,
                           GenSin100,
                           GenSin100,
                           GenSin100,
                           GenSin100
                          };



//Functions of Finite exec time
int function0(int a) {
  Serial.println(a);
  return a;
}
int function1(int a) {
  Serial.println(a + 1);
  return a + 1;
}
int function2(int a) {
  Serial.println(a + 2);
  return a + 2;
}
int function3(int a) {
  Serial.println(a + 3);
  return a + 3;
}
int function4(int a) {
  Serial.println(a + 4);
  return a + 4;
}
int function5(int a) {
  Serial.println(a + 5);
  return a + 5;
}
int function6(int a) {
  Serial.println(a + 6);
  return a + 6;
}
FunctionSet SignalGen[] = {function0,
                           function1,
                           function2,
                           function3,
                           function4,
                           function5,
                           function6
                          };

//End of Sample Function implementations                          


/*	Problems / sols / Comments / misc
// Looks like all the buffers and counters in the serial library are protected and hence cant be accessed without modifying the librbray
// work around by either modifying the isr or by adding a last read/ flag variable
// Suggest writing new isr and making a new arduino uno - cyclops board config file
*/