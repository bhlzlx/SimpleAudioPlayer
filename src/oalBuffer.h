#pragma once
#include "oalDef.h"

namespace Nix
{
    namespace openal
    {        
        struct Buffer
        {
            ALuint		m_buff;
            ALsizei		m_freq;
            ALsizei		m_size;
			ALenum		m_format;
            /*
            AL_FREQUENCY
            AL_BITS
            AL_CHANNELS
            AL_SIZE
            AL_DATA*/

			Buffer()
			{
			}

			void initialize();

			void bufferData(const void * _data, uint32_t _size, ALenum _format, ALsizei _freq);

			ALboolean valid();

			ALsizei size();

			static void Dt(Buffer * _buffer)
			{
				if (_buffer->m_buff)
				{
					alDeleteBuffers(1, &_buffer->m_buff);
				}
				delete _buffer;
			}
        };
    }
}
