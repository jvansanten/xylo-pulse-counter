
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
	std::vector<unsigned int> counts;
	unsigned int cycles;
	
	count_statistics() : counts(4,0), cycles(0) {};
};

static const char* xylo_setup[] = {
	":03000000020006F5\n",
	":03005F0002000399\n",
	":030003000201AA4D\n",
	":2000620090E680E0FF74085FF0E0FF74025FF090E6007412F00000000090E60174E3F00090\n",
	":2000820000000090E60B7403F00000000090E6027498F00000000090E60374FEF000000027\n",
	":2000A2000090E61274A2F090E61374A0F090E61474E2F090E61574E0F0000000001200D2A0\n",
	":2000C20075A80053D8DF90E680E0FF74F75FF02290E6047480F00000000090E6047482F088\n",
	":2000E2000000000090E6047484F00000000090E6047486F00000000090E6047488F00000D2\n",
	":20010200000090E604E4F00000000090E618E4F00000000090E6497482F00000000090E612\n",
	":20012200497482F00000000090E619E4F00000000090E6497484F00000000090E6497484CD\n",
	":20014200F00000000090E61AE4F00000000090E6487406F00000000090E6487406F00000F9\n",
	":20016200000090E61BE4F00000000090E6487408F00000000090E6487408F0000000009034\n",
	":20018200E6187410F00000000090E6197410F00000000090E61A740CF00000000090E61B57\n",
	":0D01A200740CF0000000002212006280FECC\n",
	":06003500E478FFF6D8FD9F\n",
	":200013007900E94400601B7A009001B3780175A000E493F2A308B8000205A0D9F4DAF275DF\n",
	":02003300A0FF2C\n",
	":20003B007800E84400600A790175A000E4F309D8FC7800E84400600C7900900001E4F0A3C3\n",
	":04005B00D8FCD9FAFA\n",
	":0D0006007581071201AFE58260030200035F\n",
	":0401AF007582002233\n",
	":00000001FF\n"
};

class PulseCounter : private CypressFX2Device {
public:
	PulseCounter() {
		struct usb_device *usbdev = USBFindDevice(0x4b4,0x8613);
		assert(usbdev);
		CypressFX2Device::open(usbdev);
		
		CypressFX2Device::FX2Reset(0);
		CypressFX2Device::ProgramStaticIHex(xylo_setup);
		CypressFX2Device::FX2Reset(1);
	}
	PulseCounter(const PulseCounter &) = delete;

private:
	void request_counts() {
		// Write a command for the FPGA to EP2
		unsigned char command = FPGA_ENABLE+FPGA_GETDATA;
		CypressFX2Device::BlockWrite(0x02, &command, 1);
	}
	
	size_t next_packet_size() {
		std::vector<unsigned char> buffer(512, 0);
		int bytes = CypressFX2Device::BlockRead(0x88, buffer.data(), 512, 'B');
		return (size_t(buffer[0])<<8) + size_t(buffer[1]);
	}
	
	count_statistics read_counts() {
		count_statistics stats;
		
		// Check size of packet in output by reading from EP8
		// (not strictly neccessary, as the packet should always be 64 bytes)
		size_t packet_size = next_packet_size();
		if (packet_size == 0)
			return stats;
		
		// Now, read the data from EP6
		std::vector<unsigned char> buffer(512, 0);
		int bytes = CypressFX2Device::BlockRead(0x86, buffer.data(), 512, 'B');
		if (bytes < packet_size)
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

public:
	count_statistics count() {
		request_counts();
		return read_counts();
	}

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
	    std::this_thread::sleep_for(500ms);
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