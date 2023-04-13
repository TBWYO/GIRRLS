#ifndef COMMUNICATIONS
#define COMMUNICATIONS

void SetUpCommunications ();
int ListenOnSerial ();
void WriteOnSerial (int SendOnSerial);
void EstablishConnectionToSoftware ();
void readSerial();
void processIncomingQueue();
#endif
