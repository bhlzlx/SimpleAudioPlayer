#include "audioStream.h"
#include <stdint.h>
#include <io/archieve.h>
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>
#include <vorbis/vorbisenc.h>
#include <assert.h>

namespace Nix
{
	namespace openal
	{
		size_t vorbisRead(void * _pData, size_t _nElement, size_t _nCount, void * _pFile)
		{
			IFile* blob = (IFile*)_pFile;
			return blob->read(_nCount * _nElement, _pData);
		}

		int vorbisSeek(void * _pFile, ogg_int64_t _nOffset, int _flag)
		{
			IFile* blob = (IFile*)_pFile;
			return blob->seek( (Nix::SeekFlag)_flag,_nOffset);
		}

		int vorbisClose(void * _pFile)
		{
			IFile* blob = (IFile*)_pFile;
			blob->release();
			return 1;
		}

		long vorbisTell(void * _pFile)
		{
			IFile* blob = (IFile*)_pFile;
			return blob->tell();
		}

		static ov_callbacks IflibVorbisCallback = {
			(size_t(*)(void *, size_t, size_t, void *))  vorbisRead,
			(int(*)(void *, ogg_int64_t, int))           vorbisSeek,
			(int(*)(void *))                             vorbisClose,
			(long(*)(void *))                            vorbisTell
		};

		struct OggAudioStream : public IAudioStream
		{
			// 文件流
			IFile * blob;
			// vorbis 文件句柄
			OggVorbis_File		m_oggvorbisFile;
			// vorbis信息
			vorbis_info*		m_vorbisInfo;
			// vorbis脚注
			vorbis_comment*		m_vorbisComment;
			//
			bool m_eof;
			// 通过 IAudioStream 继承
			OggAudioStream()
			{
				m_eof = false;
				m_vorbisInfo = nullptr;
				m_vorbisComment = nullptr;
			}

			virtual void rewind() override
			{
				ov_time_seek(&m_oggvorbisFile, 0);
				m_eof = false;
			}

			virtual void seek(double _time) override
			{
				ov_time_seek(&m_oggvorbisFile, _time);
				m_eof = false;
			}

			virtual float TimeCurr()
			{
				return ov_time_tell(&m_oggvorbisFile);
			}

			virtual float TimeTotal()
			{
				return ov_time_total(&m_oggvorbisFile, -1);
			}

			virtual size_t __readChunk(void * _pData) override
			{
				int size = 0;
				int result;
				int section;
				while (size < pcmFramesize)
				{
					result = ov_read(
						&this->m_oggvorbisFile,
						(char*)_pData + size,
						pcmFramesize - size,
						0,
						2,
						1,
						&section
					);
					// 文件尾
					if (result == 0)
					{
						m_eof = true;
						break;
					}
					// 正常读取
					else if (result > 0)
					{
						size = result + size;
					}
					// 发生错误
					else
					{
						assert(result != OV_HOLE && result != OV_EBADLINK && result != OV_EINVAL);
						break;
					}
				}
				return size;
			}

			virtual bool eof() override
			{
				return m_eof;
			}

			virtual void release() override
			{
				ov_clear(&m_oggvorbisFile);
				delete this;
			}
		};

		IAudioStream* IAudioStream::FromOGG(IFile * _blob)
		{
			OggVorbis_File m_oggvorbisFile;
			int32_t error = ov_open_callbacks(_blob, &m_oggvorbisFile, NULL, 0, IflibVorbisCallback);
			if (error != 0)
			{
				return nullptr;
			}
			// 创建OggAudioStream对象
			OggAudioStream * audioStream = new OggAudioStream();
			audioStream->blob = _blob;
			audioStream->m_oggvorbisFile = m_oggvorbisFile;
			audioStream->m_vorbisInfo = ov_info(&audioStream->m_oggvorbisFile, -1);
			audioStream->m_vorbisComment = ov_comment(&audioStream->m_oggvorbisFile, -1);
			/******************************************************
			这里解释一下代码
			PCM数据大小计算 每秒大小 = 采样率 * 位宽 / 8 * 声道数
			位宽是16,声道数是1即
			rate * 16 / 8 * 1
			即 rate * 2 这是一秒的大小
			250ms对应的大小应该是rate / 2
			所以这里就是rate >> 1,位操作右移一位即可实现除2
			但是最后还有个block对齐,这里因为是单声道,16位,两个字节的对齐
			******************************************************/
			audioStream->bytesPerSec = audioStream->m_vorbisInfo->rate * 2;
			switch (audioStream->m_vorbisInfo->channels)
			{
				// 单声道
			case 1:
				audioStream->format = AL_FORMAT_MONO16;
				audioStream->channels = 1;
				//this->m_vorbis.m_nFrameBufferSize;
				break;
				// 双声道
			case 2:
				audioStream->format = AL_FORMAT_STEREO16;
				audioStream->channels = 2;
				audioStream->bytesPerSec *= 2;
				break;
				// 四声道
			case 4:
				audioStream->format = alGetEnumValue("AL_FORMAT_QUAD16");
				audioStream->channels = 4;
				audioStream->bytesPerSec *= 4;
				break;
				// 6声道
			case 6:
				audioStream->format = alGetEnumValue("AL_FORMAT_51CHN16");
				audioStream->channels = 6;
				audioStream->bytesPerSec *= 6;
				break;
			default:
				ov_clear(&audioStream->m_oggvorbisFile);
				delete audioStream;
				return nullptr;
			}
			audioStream->pcmFramesize = audioStream->bytesPerSec / 4;
			audioStream->pcmFramecount = audioStream->bytesPerSec / audioStream->pcmFramesize;
			audioStream->bitsPerSample = audioStream->m_vorbisInfo->bitrate_nominal;
			audioStream->frequency = audioStream->m_vorbisInfo->rate;
			audioStream->sampleDepth = audioStream->bytesPerSec / audioStream->frequency / audioStream->channels;
			//
			for (int i = 0; i < audioStream->pcmFramecount + 1; ++i)
			{
				PcmBuff pcmBuff;
				pcmBuff.data = new uint8_t[audioStream->pcmFramesize];
				pcmBuff.size = 0;
				audioStream->listPcmBuff.push_back(pcmBuff);
			}

			return audioStream;
		}
	}
}