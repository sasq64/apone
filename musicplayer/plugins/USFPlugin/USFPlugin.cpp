
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <types.h>
#include <endian.h>

#include "USFPlugin.h"
#include "../../PSFFile.h"
#include <coreutils/utils.h>
#include <set>

#ifdef ARCH_MIN_ARM_NEON
#include <arm_neon.h>
#endif
extern "C" {
}
#include "lazyusf/misc.h"

int32_t sample_rate;

using namespace std;

namespace chipmachine {

class USFPlayer : public ChipPlayer {
public:

	USFPlayer(const std::string &fileName) {  	
		usf_state = new usf_loader_state;
		usf_state->emu_state = malloc( usf_get_state_size() );
		usf_clear( usf_state->emu_state );
			
		char temp[fileName.length()+1];
		strcpy(temp, fileName.c_str());
					
		if ( psf_load( temp, &psf_file_system, 0x21, usf_loader, usf_state, usf_info, usf_state ) < 0 )
			throw player_exception();
		
		PSFFile psf { fileName };
		if(psf.valid()) {
			auto &tags = psf.tags();

			int seconds = psf.songLength();

			setMeta("composer", tags["artist"],
				"title", tags["title"],
				"game", tags["game"],
				"format", "Nintendo 64",
				"length", seconds
			);
		}

		usf_set_compare( usf_state->emu_state, usf_state->enable_compare );
		usf_set_fifo_full( usf_state->emu_state, usf_state->enable_fifo_full );

		usf_render(usf_state->emu_state, 0, 0, &sample_rate);
	}


	~USFPlayer() {
		usf_shutdown(usf_state->emu_state);
	}

	virtual int getSamples(int16_t *target, int noSamples) override {
		usf_render(usf_state->emu_state, target, noSamples/2, &sample_rate);
		return noSamples;
	}
private:
	usf_loader_state *usf_state;
};

static const set<string> supported_ext { "usf", "miniusf" };

bool USFPlugin::canHandle(const std::string &name) {
	auto ext = utils::path_extension(name);
	return supported_ext.count(utils::path_extension(name)) > 0;
}

ChipPlayer *USFPlugin::fromFile(const std::string &fileName) {
	try {
		return new USFPlayer { fileName };
	} catch(player_exception &e) {
		return nullptr;
	}
};

}