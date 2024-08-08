#ifndef SLSOUND_H
#define SLSOUND_H

#include <SoL/soloud.h>
#include <SoL/soloud_wav.h>
#include <SoL/soloud_speech.h>

#include <iostream>
#include <chrono>
#include <thread>

namespace SoL
{
	class SLSound
	{
	private:
		SoLoud::Soloud soLoud;
		SoLoud::Queue soQueue;
		SoLoud::Speech sp;
		SoLoud::Wav mScreenSound;
		
	public:
		bool enableDebug;

		void init();
		void playMScreen();
		void cleanUp();
	};
}
#endif