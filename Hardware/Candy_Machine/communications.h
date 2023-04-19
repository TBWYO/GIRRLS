#ifndef COMMUNICATIONS
#define COMMUNICATIONS

void SetUpCommunications ();
int ListenOnSerial ();
void WriteOnSerial (char* SendOnSerialArray, int length);
void EstablishConnectionToSoftware ();
void readSerial();
void processIncomingQueue();
bool ResetToggle ();
#endif
