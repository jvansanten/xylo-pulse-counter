
#include <cassert>
#include <iostream>
#include <iomanip>
#include <vector>

#include <chrono>
#include <thread>

#include "cycfx2dev.h"

enum FPGACommand {
	FPGA_NO_CHANGE = 0,
	// FPGA_CLEAR = 1,
	FPGA_DISABLE = 2,
	FPGA_ENABLE = 4,
	FPGA_GETDATA = 8,
	//TTL FPGA output commands
	FPGA_PULSE = 0x10, //16
	FPGA_TOGGLE = 0x20, //32
	//TTL channels
	FPGA_TTL_1 = 0x40, //64
	FPGA_TTL_2 = 0x80, //128
};

struct count_statistics {
	std::vector<int> counts;
	int cycles;
	
	count_statistics() : counts(4,0), cycles(0) {};
};

class PulseCounter : private CypressFX2Device {
public:
	PulseCounter() {
		struct usb_device *usbdev = USBFindDevice(0x4b4,0x8613);
		assert(usbdev);
		CypressFX2Device::open(usbdev);
		
		CypressFX2Device::FX2Reset(0);
		CypressFX2Device::ProgramIHexFile("xylo_setup.ihx");
		CypressFX2Device::FX2Reset(1);
	}
	PulseCounter(const PulseCounter &) = delete;
	
	count_statistics count() {
		count_statistics stats;
		
		// Write a command for the FPGA to EP2
		{
			unsigned char command = FPGA_ENABLE+FPGA_GETDATA;
			CypressFX2Device::BlockWrite(0x02, &command, 1);
		}
		
		// Check size of packet in output by reading from EP8
		// (not strictly neccessary, as the packet should always be 64 bytes)
		size_t packet_size = 0;
		{
			std::vector<unsigned char> buffer(512, 0);
			int bytes = CypressFX2Device::BlockRead(0x88, buffer.data(), 512, 'B');
			packet_size = int(buffer[0]<<8) + int(buffer[1]);
		}
		if (packet_size == 0)
			return stats;
		
		// Now, read the data from EP6
		std::vector<unsigned char> buffer(512, 0);
		int bytes = CypressFX2Device::BlockRead(0x86, buffer.data(), 512, 'B');
		if (bytes > packet_size)
			return stats;
		
		for (int offset=0; offset < bytes; offset += 64) {
			// first 6 bytes are the clock cycles elapsed since the last request
			int p = 1;
			for(int k = 0; k < 6; k++){
				stats.cycles += (int) p*((unsigned int) buffer[offset+k]);
				p = p<<8; //next byte should be multiplied by a number 2^8 greater
			}
			// next 16 bytes contain accumlated counts in channels 1-4
			for (int j = 0; j < 4; j++) {
				p = 1; //2^(3*8) is t he amount by which the 4th byte needs to be raised
				for(int k = 0; k < 4; k++){
					stats.counts[j] += p*((unsigned int) buffer[offset+6+j*4+k]);
					p=p<<8; //next byte should be multiplied by a number 2^8 greater
				}
			}
			// and some coincidence data that we don't bother to parse for 
		}
		
		return stats;
	}
	
private:
};

void county(int iterations=20, bool quick=false)
{
	PulseCounter counter;
	if (quick)
		return;
	for (int i=0; i < iterations; i++) {
		auto counts = counter.count();
		std::cout << i << ": " << counts.counts[0]/(counts.cycles/48e6) << " in " << counts.cycles << " ("<<counts.cycles/46e6<<" s)" << std::endl;
	    using namespace std::chrono_literals;
	    std::this_thread::sleep_for(1ms);
	}
}

int main (int argc, char const *argv[])
{
	usb_init();
	usb_find_busses();
	usb_find_devices();
	
	// load_firmware();
	for (int i=0; i < 10; i++) {
		for (int j=0; j<40; j++)
			std::cout << "-";
		std::cout << std::endl << i << std::endl;
		for (int j=0; j<40; j++)
			std::cout << "-";
		std::cout << std::endl;
		county(10);
	}

	
	/* code */
	return 0;
}