// CollectMeasurements.cpp
//
// Example program showing how to use RH_NRF24 on Raspberry Pi
// Uses the bcm2835 library to access the GPIO pins to drive the NRF24L01
// Requires bcm2835 library to be already installed
// http://www.airspayce.com/mikem/bcm2835/
// Use the Makefile in this directory:
// cd example/raspi
// make
// sudo ./collectMeasurements
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
#include <thread>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <fstream>

//Function Definitions
void sig_handler(int sig);

#define SERVER_ADDRESS 0xa2
#define CLIENT_ADDRESS 0xa3

// Create an instance of a driver
// Chip enable is pin 22
// Slave Select is pin 24
RH_NRF24 nrf24(RPI_V2_GPIO_P1_22, RPI_V2_GPIO_P1_24);
RHReliableDatagram manager(nrf24, CLIENT_ADDRESS);

//Flag for Ctrl-C
volatile sig_atomic_t flag = 0;

//Main Function
int main(int argc, const char *argv[])
{
  signal(SIGINT, sig_handler);

  if (!bcm2835_init())
  {
    printf("\n\nRasPiRH Tester Startup Failed\n\n");
    return 1;
  }

  /* Begin Reliable Datagram Init Code */
  if (!manager.init())
  {
    printf("Init failed\n");
    return 1;
  }
  /* End Reliable Datagram Init Code */

  uint8_t send_buf[RH_NRF24_MAX_MESSAGE_LEN];
  uint8_t receive_buf[RH_NRF24_MAX_MESSAGE_LEN];

  float temperature = 0.0;
  float humidity = 0.0;
  float value3 = 0.0;

  // Begin the main body of code
  while (true) {
    uint8_t len = sizeof(send_buf);
    uint8_t from, to, id, flags;

    /* Begin Reliable Datagram Code */
    if (manager.sendtoWait(send_buf, sizeof(send_buf), SERVER_ADDRESS)) {
      len = sizeof(receive_buf);
      if (manager.recvfromAck(receive_buf, &len, &from)) {

        auto const now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);

        memcpy(&temperature, &receive_buf[1], 4);
        memcpy(&humidity, &receive_buf[5], 4);
        std::cout << std::put_time(std::localtime(&now_c), "Y-%m-%d %H-%M-%S") << " " << humidity << " " << temperature
                  << std::endl;
      }
    }
    /* End Reliable Datagram Code */

    if (flag) {
      printf("\n---CTRL-C Caught - Exiting---\n");
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    // delay(1000);
  }
  bcm2835_close();
  return 0;
}

void sig_handler(int sig)
{
  flag=1;
}
