// ReceiveMeasurements.cpp
//
// Example program showing how to use RH_NRF24 on Raspberry Pi
// Uses the bcm2835 library to access the GPIO pins to drive the NRF24L01
// Requires bcm2835 library to be already installed
// http://www.airspayce.com/mikem/bcm2835/
// Use the Makefile in this directory:
// cd example/raspi
// make
// sudo ./receiveMeasurements
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
#include <fstream>

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

  /* Begin Reliable Datagram Init Code */
  if (!manager.init())
  {
    printf( "Init failed\n" );
	return 1;
  }
  /* End Reliable Datagram Init Code */

  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];

  float temperature = 0.0;
  float humidity = 0.0;
  float value3 = 0.0;

  std::ofstream station3("/home/alarm/data/station3/data.txt", std::ios::app);
  std::ofstream station4("/home/alarm/data/station4/data.txt", std::ios::app);
  std::ofstream station5("/home/alarm/data/station5/data.txt", std::ios::app);
  std::ofstream station6("/home/alarm/data/station6/data.txt", std::ios::app);
  std::ofstream station7("/home/alarm/data/station7/data.txt", std::ios::app);

  //Begin the main body of code
  while (true)
  {
    uint8_t len = sizeof(buf);
    uint8_t from, to, id, flags;

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
			station3 << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " " << humidity << " " << temperature << std::endl;
			//std::cout << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " received humidity: " << humidity << ", temperature: " << temperature << " from station #" << int(from) << std::endl;
			break;
		case 4:
			memcpy(&temperature, &buf[1], 4);
			memcpy(&humidity, &buf[5], 4);
			station4 << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " " << humidity << " " << temperature << std::endl;
			//std::cout << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " received humidity: " << humidity << ", temperature: " << temperature << " from station #" << int(from) << std::endl;
			break;
		case 5:
			memcpy(&temperature, &buf[1], 4);
			station5 << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " " << temperature << std::endl;
			//std::cout << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " received temperature: " << temperature << " from station #" << int(from) << std::endl;
			break;
		case 6:
			memcpy(&temperature, &buf[1], 4);
			memcpy(&humidity, &buf[5], 4);
			memcpy(&value3, &buf[9], 4);
			//station6 << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " " << humidity << " " << temperature << " " << value3 << std::endl;
			std::cout << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " received pressure: " << humidity << ", bmp280_temperature: " << temperature << ", ds1820_temperature: " << value3 << " from station #" << int(from) << std::endl;
			break;
		case 7:
			memcpy(&temperature, &buf[1], 4);
			memcpy(&humidity, &buf[5], 4);
			station7 << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " " << humidity << " " << temperature << std::endl;
			//std::cout << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " received humidity: " << humidity << ", temperature: " << temperature << " from station #" << int(from) << std::endl;
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
    delay(100);
  }
  station3.close();
  station4.close();
  station5.close();
  station6.close();
  station7.close();
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
