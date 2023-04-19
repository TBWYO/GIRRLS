#ifndef COMMUNICATIONS
#define COMMUNICATIONS

void SetUpCommunications ();
int ListenOnSerial ();
void WriteArrayOnSerial (char* SendOnSerialArray, int length);
void EstablishConnectionToSoftware ();
void readSerial();
void processIncomingQueue();
bool ResetToggle ();
void DetermineCommTypes ();
void WriteOutgoingBuffer ();
#endif
