#include "oalBuffer.h"
#include <assert.h>

namespace Nix
{
	namespace openal
	{
		void Buffer::initialize()
		{
			CLEAR_AL_ERROR;
			alGenBuffers(1, &m_buff);
			CHECK_AL_ERROR;
		}

		void Buffer::bufferData( const void * _data, uint32_t _size, ALenum _format, ALsizei _freq)
		{
			CLEAR_AL_ERROR;
			alBufferData(m_buff, _format, _data, _size, _freq);
			m_freq = _freq;
			m_size = _size;
			m_format = _format;
			CHECK_AL_ERROR;
		}

		ALboolean Buffer::valid()
		{
			return alIsBuffer(m_buff);
		}

		ALsizei Buffer::size()
		{
			return m_size;
		}

	}
}

