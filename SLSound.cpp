#include "SLSound.h"

namespace SoL
{
	void SLSound::init()
	{
		soLoud.init();
		//sound.load("data/win.ogg");

		sp.setText("Skynet online");
		
		//soLoud.play(sp);

		if (enableDebug)
		{
			std::cout << "SkyNet :: sound initialized" << std::endl;
		}

		mScreenSound.load("data/mScreen/matrix.ogg");
		mScreenSound.setLooping(true);
		mScreenSound.setVolume(0.5);
	}

	void SLSound::playMScreen()
	{
		soLoud.play(mScreenSound);
	}

	//void SLSound::play1()
	//{
		//sp.setText("Wire frame enabled");
		//soLoud.play(sp);
	//}

	

	void SLSound::cleanUp()
	{
		soLoud.deinit();
	}
}