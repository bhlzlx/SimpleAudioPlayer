#pragma once
#include <openal/al.h>
#include <openal/alc.h>
#include <cstdint>

#ifdef _DEBUG
#define CLEAR_AL_ERROR \
	alGetError();
#define CHECK_AL_ERROR \
assert ( alGetError() == AL_NO_ERROR );
#else
#define CLEAR_AL_ERROR
#define CHECK_AL_ERROR
#endif

namespace Nix
{
	namespace openal
	{
		//static const ALint MAXBUFFQUEUED = 3;
		typedef ALuint ALbuff;
		typedef ALuint ALsource;

		enum eFormat :ALenum
		{
			FormatMono8 = AL_FORMAT_MONO8,
			FormatMono16 = AL_FORMAT_MONO16,
			FormatStereo8 = AL_FORMAT_STEREO8,
			FormatStereo16 = AL_FORMAT_STEREO16,
		};
	}
}

