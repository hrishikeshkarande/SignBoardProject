/*
  RFSniffer
  Usage: ./RFSniffer [<pulseLength>]
  [] = optional
  Hacked from http://code.google.com/p/rc-switch/
  by @justy to provide a handy RF code sniffer
*/

#include "./rc-switch/RCSwitch.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>
#include <array>

using namespace std;
/*  -- SKIP_PEER_VERIFICATION --
* If you want to connect to a site who isn't using a certificate that is
* signed by one of the certs in the CA bundle you have, you can skip the
* verification of the server's certificate. This makes the connection
* A LOT LESS SECURE.
* If you have a CA cert for the server stored someplace else than in the
* default bundle, then the CURLOPT_CAPATH option might come handy for you.
*/
#define SKIP_PEER_VERIFICATION
/* -- SKIP_HOSTNAME_VERIFICATION --
* If the site you're connecting to uses a different host name that what
* they have mentioned in their server certificate's commonName (or
* subjectAltName) fields, libcurl will refuse to connect. You can skip
* this check, but this will make the connection less secure.
*/
#define SKIP_HOSTNAME_VERIFICATION

#define DEBUG false

RCSwitch mySwitch;

// Sends cURL output to /dev/null instead to STDOUT
size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	return size * nmemb;
}

//setELabState
// param: int state
//	1 = open, 2 = closed, 3 = back soon
void setELabState(int state)
{
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, "https://elab-siegen.de");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		switch (state)
		{
		case 1: //set State open
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "open=");
			break;
		case 3: //set State back soon
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "brb=");
			break;
		case 2: //set State closed
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "closed=");
			break;
		default: //don't change on default
			break;
		}
#ifdef SKIP_PEER_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
#ifdef SKIP_HOSTNAME_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
}

int main(int argc, char *argv[])
{
	// This pin is not the first pin on the RPi GPIO header!
	// Consult https://projects.drogon.net/raspberry-pi/wiringpi/pins/
	// for more information.
	int PIN = 15;
	printf("Start reading wiringPi\n");
	if (wiringPiSetup() == -1)
	{
		printf("wiringPiSetup failed, exiting...");
		return 0;
	}
	int pulseLength = 0;
	if (argv[1] != NULL)
		pulseLength = atoi(argv[1]);
	mySwitch = RCSwitch();
	if (pulseLength != 0)
		mySwitch.setPulseLength(pulseLength);
	mySwitch.enableReceive(PIN); // Receiver on interrupt 0 => that is pin #2

	long long int secondsSinceLastReceived = 0;
	long long int threshold = 7200; // ~2 hours when sleeptime is 1 second
	int currentState = -1;
	int received = 0;
	std::array<int, 5> lastReceived;
	for(int i=0; i<lastReceived.size(); i++)
	{
		lastReceived[i] = 0;
	}

	printf("Starting Loop\n");
	while (1)
	{
		usleep(1000000); // 1000 ms / 1 s
		if (mySwitch.available())
		{
			received = mySwitch.getReceivedValue();
			mySwitch.resetAvailable();
			secondsSinceLastReceived = 0;

			if(received != currentState)
			{
				if(DEBUG) printf("State differs old: %i new: %i \n", currentState, received);
				lastReceived[0] = received;
				for(int i=1; i<lastReceived.size(); i++)
				{
					while(!mySwitch.available())
					{
						//WAIT for new received value
						usleep(100000); // 100 ms 
					}
					lastReceived[i] = mySwitch.getReceivedValue();
					if(DEBUG) printf("Received: %i \n", lastReceived[i]);
					mySwitch.resetAvailable();
				}
				// Check if values are equal
				bool equal = true;
				for(int i=1; i<lastReceived.size(); i++)
				{
					//if(DEBUG) printf("C1 %i C2 %i\n", lastReceived[i-1], lastReceived[i]);
					if(lastReceived[i-1] != lastReceived[i])
					{
						equal = false;
					}
				}
				// If they are equal
				if(equal)
				{
					printf("Switchting to state via Curl: %i \n", received);
					setELabState(received);
					currentState = received;
					mySwitch.resetAvailable();
				}
				// If they are not equal
				else
				{
					printf("No new State detected!\n");
				}
			}
		}
		else
		{
			//printf("No new Data available\n");
			secondsSinceLastReceived++;
			if(secondsSinceLastReceived >= threshold)
			{
				printf("Warning: %i seconds since last received value.\n", secondsSinceLastReceived);
			}
		}
	}
	exit(0);
}
