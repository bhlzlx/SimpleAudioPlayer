#include "audioStream.h"
#include <io/archieve.h>
#include <lame.h>
#include <stdint.h>
#include <assert.h>

namespace Nix
{
	namespace openal
	{

#pragma pack( push, 1 )
		struct Mp3TagV2
		{
			char Header[3];			/*必须为"ID3"否则认为标签不存在*/
			char Ver;				/*版本号 ID3V2.3 就记录 3*/
			char Revision;			/*副版本号此版本记录为 0*/
			char Flag;				/*存放标志的字节,这个版本只定义了三位,稍后详细解说*/
			char size[4];			/*接下来的所有的标签帧的大小,不包括标签头的 10 个字节*/
		};

		struct Mp3DataFrameHeaderCBR
		{
			unsigned int sync : 11;                         //同步信息
			unsigned int version : 2;                       //版本
			unsigned int layer : 2;                         //层
			unsigned int crcValid : 1;						// CRC校验
			unsigned int bitrate_index : 4;					//位率
			unsigned int sampling_frequency : 2;			//采样频率
			unsigned int padding : 1;						//帧长调节
			unsigned int reserve : 1;                       //保留字
			unsigned int channel_mode : 2;                  //声道模式
			unsigned int ext_mode : 2;						//扩充模式
			unsigned int copyright : 1;                     // 版权
			unsigned int original : 1;                      //原版标志
			unsigned int emphasis : 2;						//强调模式
		};

		struct Mp3DataFrameHeader
		{
			union {
				struct
				{
					Mp3DataFrameHeaderCBR
										cbr_header;
					uint8_t				xing_area[36];
					uint8_t				vbr_store_flag[4];
					uint32_t			framecount;
					uint32_t			filesize;
					uint8_t				toc[100];
					uint32_t			raterange;
				};
				char bytes[156];
			};
			bool IsVBR()
			{
				const uint8_t xing[5] = "Xing";
				if (memcmp(&bytes[36], xing, 4) == 0)
				{
					return true;
				}
				if (memcmp(&bytes[21], xing, 4) == 0)
				{
					return true;
				}
				if (memcmp(&bytes[13], xing, 4) == 0)
				{
					return true;
				}
				return false;
			}
		};
#pragma pack(pop)

		static const int MP3_CHUNK_SIZE = 1024;
		static const float MP3_FRAME_TIME_MS = 26.0f;
		static const float MP3_FRAME_TIME_SEC = 0.026f;

		struct Mp3AudioStream : public IAudioStream
		{
			// 文件流
			IFile *		m_blob;
			hip_t		m_hip;
			uint8_t		m_cbr;
			size_t		m_dataframeoffset;
			float		m_timeTotal;
			float		m_frameDur;
			uint32_t		m_currFrame;
			mp3data_struct m_mp3Info;
			Mp3DataFrameHeader m_mp3Header;
			uint8_t		m_mp3Bytes[MP3_CHUNK_SIZE];
			short		m_pcmBytes[2][MP3_CHUNK_SIZE * 4];
			short		m_pcmLastCache[MP3_CHUNK_SIZE * 4 * 2];
			short		m_pcmMixed[MP3_CHUNK_SIZE * 4*2];
			uint32_t	m_pcmLastCacheSize;

			bool		m_eof;

			void mixSamples(size_t _nSamples)
			{
				for (size_t i = 0; i < _nSamples; ++i)
				{
					m_pcmMixed[i * 2] = m_pcmBytes[0][i];
					m_pcmMixed[i * 2+1] = m_pcmBytes[1][i];
				}
			}

			// 通过 IAudioStream 继承
			virtual void rewind() override
			{
				m_eof = false;
				m_pcmLastCacheSize = 0;
				int ret = hip_decode1(m_hip, m_mp3Bytes, 0, m_pcmBytes[0], m_pcmBytes[1]);
				if (ret == -1)
				{

				}
				m_blob->seek( (Nix::SeekFlag)SEEK_SET, m_dataframeoffset);
			}

			virtual float TimeCurr()
			{
				return m_currFrame * m_frameDur;
			}

			virtual float TimeTotal()
			{
				return m_timeTotal;
			}


			virtual void seek(double _time) override
			{
				// 清空残留数据
				m_pcmLastCacheSize = 0;
				m_eof = false;
				hip_decode1(m_hip, m_mp3Bytes, 0, m_pcmBytes[0], m_pcmBytes[1]);
				//
				size_t offset = 0;
				if (m_cbr)
				{
					if (_time < 0 || _time > m_timeTotal)
					{
						return;
					}
					else
					{
						int index = (int)(_time / m_frameDur);
						if (index >= m_mp3Info.totalframes)
						{
							m_blob->seek((Nix::SeekFlag)SEEK_SET, 0);
							m_currFrame = 0;
							m_eof = true;
						}
						else
						{
							offset = (size_t)((double)(m_blob->size() - m_dataframeoffset) / (double)m_mp3Info.totalframes )* index + m_dataframeoffset;
							m_currFrame = index;
						}
					}
				}
				else
				{
					if (_time > m_timeTotal)
					{
						return;
					}
					size_t index = (size_t)(_time / m_timeTotal * 100);
					offset = m_mp3Header.toc[index] / 256 * m_mp3Header.filesize;
					m_currFrame = index;
				}
				m_blob->seek( (Nix::SeekFlag)SEEK_SET, offset);
			}

			virtual size_t __readChunk(void * _pData) override
			{
				mp3data_struct fh1st;
				pcmFramesize;
				uint32_t nRead = 0;
				uint8_t * wptr = (uint8_t*)_pData;
				if (m_pcmLastCacheSize)
				{
					memcpy(wptr, m_pcmLastCache, m_pcmLastCacheSize);
					wptr += m_pcmLastCacheSize;
					nRead += m_pcmLastCacheSize;
				}
				while (true)
				{
					size_t byteRead = 0;
					while (!m_blob->eof() && (!byteRead))
					{
						byteRead = m_blob->read(MP3_CHUNK_SIZE , m_mp3Bytes);
					}
					++m_currFrame;
					int decodeSample = hip_decode1_headers(m_hip, m_mp3Bytes, byteRead, m_pcmBytes[0], m_pcmBytes[1],&fh1st);
					int decodeBytes = decodeSample * sizeof(short) * channels;
					if (decodeSample == -1)
					{
						m_eof = true;
						m_pcmLastCacheSize = 0;
						return 0;
					}
					if (decodeSample)
					{
						nRead += decodeBytes;
						mixSamples(decodeSample);
						if (nRead < pcmFramesize)
						{
							memcpy(wptr, m_pcmMixed, decodeBytes);
							wptr += decodeBytes;
						}
						else
						{
							size_t bytesW = decodeBytes - (nRead - pcmFramesize);
							size_t bytesC = nRead - pcmFramesize;
							memcpy(wptr, m_pcmMixed, bytesW);

							memcpy(m_pcmLastCache, (uint8_t*)&m_pcmMixed[0] + bytesW, bytesC);
							m_pcmLastCacheSize = bytesC;
							nRead -= bytesC;
							break;
						}
					}					
					if (!byteRead && decodeSample == 0)
					{
						m_eof = true;
						m_currFrame = 0;
						break;
					}
				}

				return nRead;
			}

			virtual bool eof() override
			{
				return m_eof;
			}

			virtual void release() override
			{
				hip_decode_exit(m_hip);
				m_blob->release();
			}
		};

		IAudioStream* IAudioStream::FromMP3(IFile * _blob)
		{
			size_t dataframeoffset = 0;
			// 尝试读取 TagV2 文件头
			Mp3TagV2 tagV2;
			_blob->read(sizeof(tagV2) ,&tagV2);
			if (memcmp(tagV2.Header, "ID3", 3) != 0)
			{
				_blob->seek((SeekFlag)SEEK_SET, 0);
				dataframeoffset = 0;
			}
			else
			{
				size_t Tagv2FS = (int)(tagV2.size[0] & 0x7F) << 21
					| (int)(tagV2.size[1] & 0x7F) << 14
					| (int)(tagV2.size[2] & 0x7F) << 7
					| (int)(tagV2.size[3] & 0x7F) + 10;
				dataframeoffset = Tagv2FS;
				_blob->seek((SeekFlag)SEEK_CUR, Tagv2FS - sizeof(tagV2));
			}
			uint8_t isCBR = true;
			//
			// 看是CBR还是VBR
			// 尝试读取VBR
			Mp3DataFrameHeader vbrHead;
			size_t rd = _blob->read(sizeof(vbrHead), &vbrHead);
			assert(rd == sizeof(vbrHead));
			if (vbrHead.IsVBR())
			{
				isCBR = false;
			}
			//
			hip_t hip = hip_decode_init();
			uint8_t mp3Bytes[1024];
			short pcmBytesL[1024 * 4];
			short pcmBytesR[1024 * 4];
			_blob->seek((SeekFlag)SEEK_SET, dataframeoffset);
			// 试着解一帧
			mp3data_struct fh1st;
			memset(&fh1st, 0, sizeof(fh1st));
			int ret = 0;
			do
			{
				_blob->read(1024,mp3Bytes);
				ret = hip_decode1_headers(hip, mp3Bytes, MP3_CHUNK_SIZE, pcmBytesL, pcmBytesR, &fh1st);
			} while (ret == 0);
			if (ret == -1)
			{
				hip_decode_exit(hip);
				return nullptr;
			}
			/* =====================================================================================================================
			
				MP3帧长取决于位率和频率，计算公式为：
	 
				. mpeg1.0       layer1 :   帧长= (48000*bitrate)/sampling_freq + padding
								layer2&3: 帧长= (144000*bitrate)/sampling_freq + padding
				. mpeg2.0       layer1 :   帧长= (24000*bitrate)/sampling_freq + padding
				layer2&3 : 帧长= (72000*bitrate)/sampling_freq + padding
				例如：位率为64kbps，采样频率为44.1kHz，padding（帧长调节）为1时，帧长为210字节。
				帧头后面是可变长度的附加信息，对于标准的MP3文件来说，其长度是32字节，紧接其后的是压缩的声音数据，当解码器读到此处时就进行解码了。
			===================================================================================================================== */
			if (!fh1st.totalframes)
			{
				int frameBinSize = (float)(144000 * fh1st.bitrate) / (float)fh1st.samplerate + vbrHead.cbr_header.padding;
				fh1st.totalframes = (_blob->size() - 128 - dataframeoffset) / frameBinSize;
			}

			_blob->seek((SeekFlag)SEEK_SET, dataframeoffset);
			Mp3AudioStream * audioStream = new Mp3AudioStream();
			audioStream->m_cbr = isCBR;
			audioStream->m_hip = hip;
			audioStream->m_dataframeoffset = dataframeoffset;
			audioStream->m_blob = _blob;
			audioStream->m_mp3Info = fh1st;
			audioStream->m_mp3Header = vbrHead;
			// base class
			audioStream->bytesPerSec = fh1st.samplerate * 2;
			audioStream->channels = fh1st.stereo;
			audioStream->m_currFrame = 0;
			switch (audioStream->channels)
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
				delete audioStream;
				return nullptr;
			}
			audioStream->bitsPerSample = fh1st.bitrate;
			audioStream->pcmFramesize = audioStream->bytesPerSec / 4;
			audioStream->frequency = fh1st.samplerate;
			audioStream->pcmFramecount = audioStream->bytesPerSec / audioStream->pcmFramesize / 2 + 1;
			audioStream->sampleDepth = audioStream->bytesPerSec / audioStream->frequency / audioStream->channels;
			audioStream->m_blob = _blob;
			audioStream->m_frameDur = (float)fh1st.framesize / (float)fh1st.samplerate;
			audioStream->m_timeTotal = audioStream->m_frameDur * fh1st.totalframes;
			// audioStream->timeTotal = (float)((_blob->Size() - dataframeoffset) << 23) / (fh1st.bitrate * 1000000.0f);
			//audioStream->pcmFrame = new uint8_t[audioStream->pcmFramesize];
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