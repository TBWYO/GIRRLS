#ifndef COMMUNICATIONS
#define COMMUNICATIONS

void setWatchForCandyDispensed (bool newValue);
bool getWatchForCandyDispensed ();
void setWatchForCandyTaken (bool newValue);
bool getWatchForCandyTaken ();
void SetUpCommunications ();
int ListenOnSerial ();
void WriteArrayOnSerial (char* SendOnSerialArray, int length);
void EstablishConnectionToSoftware ();
void readSerial();
void processIncomingQueue();
void ProcessOutgoingQueue ();
//bool ResetToggle();
void DetermineCommTypes ();
void WriteOutgoingBuffer (char* ByteArray, int length);
#endif
