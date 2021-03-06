// RequestMeasurement.cpp
//
// Example program showing how to use RH_NRF24 on Raspberry Pi
// Uses the bcm2835 library to access the GPIO pins to drive the NRF24L01
// Requires bcm2835 library to be already installed
// http://www.airspayce.com/mikem/bcm2835/
// Use the Makefile in this directory:
// cd example/raspi
// make
// sudo ./requestMeasurement address
//
// Creates a RHReliableDatagram manager and listens and prints for reliable datagrams
// sent to it on the default Channel 2.
//
// Contributed by Mike Poublon

#include <bcm2835.h>
#include <unistd.h>

#include <RHReliableDatagram.h>
#include <RH_NRF24.h>

#include <chrono>
#include <thread>
#include <ctime>
#include <iomanip>
#include <iostream>

#define CLIENT_ADDRESS 0xa3

//Main Function
int main(int argc, const char *argv[]) {
  uint8_t server_address = std::stoi(argv[1]);

  if (!bcm2835_init()) {
    std::cout << "\n\nRasPiRH Tester Startup Failed - bcm2835_init failed.\n\n";
    return 1;
  }

  // Create an instance of a driver
  // Chip enable is pin 22
  // Slave Select is pin 24
  RH_NRF24 nrf24(RPI_V2_GPIO_P1_22, RPI_V2_GPIO_P1_24);
  RHReliableDatagram manager(nrf24, CLIENT_ADDRESS);

  /* Begin Reliable Datagram Init Code */
  if (!manager.init()) {
    std::cout << "RHReliableDatagram obj - Init failed.\n";
    return 1;
  }
  /* End Reliable Datagram Init Code */

  uint8_t send_buf[] = "request sensor data";
  uint8_t receive_buf[RH_NRF24_MAX_MESSAGE_LEN];

  float temperature = 0.0;
  float humidity = 0.0;
  float value3 = 0.0;

  // Begin the main body of code
  uint8_t len = sizeof(receive_buf);
  uint8_t from = 0;
  uint8_t message_type;

  auto copy_buffer = [&receive_buf, &message_type, &temperature, &humidity]() {
    memcpy(&message_type, &receive_buf[0], 1);
    memcpy(&temperature, &receive_buf[1], 4);
    memcpy(&humidity, &receive_buf[5], 4);
  };

  /* Begin Reliable Datagram Code */
  if (manager.sendtoWait(send_buf, sizeof(send_buf), server_address)) {
    auto const now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    len = sizeof(receive_buf);
    if (manager.recvfromAckTimeout(receive_buf, &len, 2000, &from)) {

      copy_buffer();
      std::cout << int(server_address) << " "
                << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S")
                << " " << humidity << " " << temperature << std::endl;
    } else {
      // did not receive acknowledgement - send request again
      if (manager.sendtoWait(send_buf, sizeof(send_buf), server_address)) {
        len = sizeof(receive_buf);
        if (manager.recvfromAckTimeout(receive_buf, &len, 2000, &from)) {

          copy_buffer();

          std::cout << int(server_address) << " "
                    << std::put_time(std::localtime(&now_c),
                                     "%Y-%m-%d %H:%M:%S")
                    << " " << humidity << " " << temperature << std::endl;
        }
      }
    }
  }
  /* End Reliable Datagram Code */

  bcm2835_close();
  return EXIT_SUCCESS;
}
