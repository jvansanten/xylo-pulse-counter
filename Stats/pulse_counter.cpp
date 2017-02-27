
#include "StatsDLL.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

// struct pulse_data {
//
//
// };

int main(int argc, char const *argv[]) {
  /* code */
  // std::cout << "USB_Open(): " << USB_Open() << std::endl;
  USB_Open();
  unsigned __int64 data[512];
  // __int64 stats[512];
  const int runs = 1;
  int length = runs*64;
  std::vector<__int64> stats(length,0);
  int err;
  // err = FPGA_Counts(false, FPGA_CLEAR, "data.dat", stats.data(), data, &length, runs);
  FPGA_Counts(false, FPGA_ENABLE+FPGA_GETDATA, NULL, stats.data(), data, &length, 0);
  size_t count0 = stats[1];
  size_t t0 = stats[0];
  //  USB_Close();
  for (int i=0; i < 10; i++) {
    // USB_Open();
    length = runs*64;
    // memset(stats.data(), sizeof(double)*stats.size(), 0);
    err = FPGA_Counts(false, FPGA_GETDATA, "data.dat", stats.data(), data, &length, runs);
    FPGA_Counts(false, FPGA_CLEAR, NULL, stats.data(), data, &length, 0);
    // USB_Close();

    double dt = stats[0]/48e6;
    std::cout << stats[1]-count0 << " Hz " << stats[1] << " " << stats[0]-t0 << std::endl;
    count0 = stats[1];
    t0 = stats[0];
    // std::cout << "In " << stats[0]/48e6 << " clicks: " << stats[1] << "/" << stats[2] << " counts" << std::endl;
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
  }
  // FPGA_Counts(false, FPGA_CLEAR+FPGA_DISABLE, NULL, stats.data(), data, &length, 0);



  // std::cout << "got bytes: " << length << std::endl;
  // for (int i=0; i < length; i++)
    // std::cout << i << ": " << stats[i] << std::endl;
  // length = 512;
  // FPGA_Counts(false, FPGA_GETDATA, NULL, stats, data, &length, runs);
    // std::cout << length << std::endl;


USB_Close();

  return 0;
}
