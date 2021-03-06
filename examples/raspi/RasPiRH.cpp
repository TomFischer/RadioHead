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

#define SERVER_ADDRESS 2

// Create an instance of a driver
// Chip enable is pin 22
// Slave Select is pin 24
RH_NRF24 nrf24(RPI_V2_GPIO_P1_22, RPI_V2_GPIO_P1_24);
RHReliableDatagram manager(nrf24, SERVER_ADDRESS);

//Flag for Ctrl-C
volatile sig_atomic_t flag = 0;

//Main Function
int main (int argc, const char* argv[] )
{
  signal(SIGINT, sig_handler);

  if (!bcm2835_init())
  {
    printf( "\n\nRasPiRH Tester Startup Failed\n\n" );
    return 1;
  }

  printf( "\nRasPiRH Tester Startup\n\n" );

  /* Begin Driver Only Init Code
  if (!nrf24.init())
    Serial.println("init failed");
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
  if (!nrf24.setChannel(1))
    Serial.println("setChannel failed");
  if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm))
    Serial.println("setRF failed");
  End Driver Only Init Code */

  /* Begin Reliable Datagram Init Code */
  if (!manager.init())
  {
    printf( "Init failed\n" );
  }
  /* End Reliable Datagram Init Code */

  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];

  float temperature = 0.0;
  float humidity = 0.0;
  float value3 = 0.0;

  //Begin the main body of code
  while (true)
  {
    uint8_t len = sizeof(buf);
    uint8_t from, to, id, flags;

    /* Begin Driver Only code
    if (nrf24.available())
    {
      // Should be a message for us now
      //uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
      uint8_t len = sizeof(buf);
      if (nrf24.recv(buf, &len))
      {
        Serial.print("got request: ");
        Serial.println((char*)buf);
        Serial.println("");
      }
      else
      {
        Serial.println("recv failed");
      }
    }
    End Driver Only Code*/

    /* Begin Reliable Datagram Code */
    if (manager.available())
    {
      // Wait for a message addressed to us from the client
      uint8_t len = sizeof(buf);
      uint8_t from;
      if (manager.recvfromAck(buf, &len, &from))
      {
		auto now = std::chrono::system_clock::now();
		std::time_t now_c = std::chrono::system_clock::to_time_t(now);
		switch (from)
		{
		case 3:
			memcpy(&temperature, &buf[1], 4);
			memcpy(&humidity, &buf[5], 4);
			std::cout << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " received humidity: " << humidity << ", temperature: " << temperature << " from station #" << int(from) << std::endl;
			break;
		case 4:
			memcpy(&temperature, &buf[1], 4);
			memcpy(&humidity, &buf[5], 4);
			std::cout << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " received humidity: " << humidity << ", temperature: " << temperature << " from station #" << int(from) << std::endl;
			break;
		case 5:
			memcpy(&temperature, &buf[1], 4);
			std::cout << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " received temperature: " << temperature << " from station #" << int(from) << std::endl;
			break;
		case 6:
			memcpy(&temperature, &buf[1], 4);
			memcpy(&humidity, &buf[5], 4);
			memcpy(&value3, &buf[9], 4);
			std::cout << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " received pressure: " << humidity << ", bmp280_temperature: " << temperature << ", ds1820_temperature: " << value3 << " from station #" << int(from) << std::endl;
			break;
		case 7:
			memcpy(&temperature, &buf[1], 4);
			memcpy(&humidity, &buf[5], 4);
			std::cout << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " received humidity: " << humidity << ", temperature: " << temperature << " from station #" << int(from) << std::endl;
			break;
		default:
			std::cout << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " received message from unknown station #" << int(from) << std::endl;
		}
      }
    }
    /* End Reliable Datagram Code */

    if (flag)
    {
      printf("\n---CTRL-C Caught - Exiting---\n");
      break;
    }
    //sleep(1);
    delay(25);
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
