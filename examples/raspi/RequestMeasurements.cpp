// RasPiRH.cpp
//
// Example program showing how to use RH_NRF24 on Raspberry Pi
// Uses the bcm2835 library to access the GPIO pins to drive the NRF24L01
// Requires bcm2835 library to be already installed
// http://www.airspayce.com/mikem/bcm2835/
// Use the Makefile in this directory:
// cd example/raspi
// make
// sudo ./RasPiRH
//
// Creates a RHReliableDatagram manager and listens and prints for reliable datagrams
// sent to it on the default Channel 2.
//
// Contributed by Mike Poublon

#include <bcm2835.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <RHReliableDatagram.h>
#include <RH_NRF24.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

//Function Definitions
void sig_handler(int sig);
void printbuffer(uint8_t buff[], int len);

#define SERVER_ADDRESS 1

//Flag for Ctrl-C
volatile sig_atomic_t flag = 0;

//Main Function
int main (int argc, const char* argv[] )
{
    // Create an instance of a driver
    // Chip enable is pin 22
    // Slave Select is pin 24
    RH_NRF24 nrf24(RPI_V2_GPIO_P1_22, RPI_V2_GPIO_P1_24);
    RHReliableDatagram manager(nrf24, SERVER_ADDRESS);

	signal(SIGINT, sig_handler);

  if (!bcm2835_init())
  {
    printf( "\n\nRasPiRH Tester Startup Failed\n\n" );
    return 1;
  }
  printf( "\nRasPiRH Tester Startup successful\n\n" );

  /* Begin Reliable Datagram Init Code */
  if (!manager.init())
  {
    printf( "Init failed\n" );
    return 1;
  }
  /* End Reliable Datagram Init Code */

  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN] = "t";
  uint8_t len = sizeof(buf);
  uint8_t from;

  /* Begin Reliable Datagram Code */
  	// request temperature from client 10
  	if (manager.sendtoWait(buf, sizeof(buf), 10))
	{
		std::cout << "manager.sendtoWait(" << buf << ", " << sizeof(buf) << ", 10) successful\n";
	}
	else
	{
		std::cout << "manager.sendtoWait(" << buf << ", " << sizeof(buf) << ", 10) not successful\n";
	}

	if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
	{
		std::cout << "got reply from " << from << ": " << buf << std::endl;
	}
	else
	{
		std::cout << "no reply" << std::endl;
	}
    /* End Reliable Datagram Code */

    if (flag)
    {
      printf("\n---CTRL-C Caught - Exiting---\n");
	  return EXIT_SUCCESS;
    }
  printf( "\nRasPiRH Tester Ending\n" );
  bcm2835_close();
  return 0;
}

void sig_handler(int sig)
{
  flag=1;
}

void printbuffer(uint8_t buff[], int len)
{
  for (int i = 0; i< len; i++)
  {
    printf(" %2X", buff[i]);
  }
}
