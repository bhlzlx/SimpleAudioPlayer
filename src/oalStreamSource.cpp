#include "oalStreamSource.h"
#include <assert.h>
#include <Windows.h>

namespace Nix
{
	namespace openal
	{
		StreamAudioSource::StreamAudioSource()
			: m_audioStream(nullptr)
			, m_decodeService(nullptr)
			, m_decodeQueue(8)
			, m_loop(true)
		{
		}

		void StreamAudioSource::initialize(IAudioStream * _audioStream, AudioDecoderService* _decodeService )
		{
			m_audioStream = _audioStream;
			m_decodeService = _decodeService;
			//
			if (listBuff.size())
			{
				int frameCount = _audioStream->pcmFramecount;
				int listBuffCount = listBuff.size();
				Source::play();
				while (true)
				{
					Sleep(100);
					if (processedBufferCount() == listBuffCount)
					{
						break;
					}
				}
				// ���ڵ�Pcm framecount��֮ǰ�� stream��Ӧ�Ŀ��ܲ�һ��,�������ڴ�Ļ���֮ǰ��framecount
				if (listBuffCount > frameCount)
				{
					for (size_t i = 0; i < listBuffCount - frameCount; ++i)
					{
						std::shared_ptr<Buffer> buffer;
						unqueBuffer(buffer);
					}
					listBuffCount = frameCount;
				}
				for (int32_t i = 0; i < listBuffCount; ++i)
				{
					const void * chunk = nullptr;
					size_t size = _audioStream->readChunk(&chunk);
					std::shared_ptr<Buffer> buffer; 
					unqueBuffer(buffer);
					buffer->bufferData(chunk, size, m_audioStream->format, m_audioStream->frequency);
					queueBuffer(buffer);
					if (_audioStream->eof())
					{
						break;
					}
				}
				while(listBuffCount < frameCount)
				{
					const void * chunk = nullptr;
					size_t size = _audioStream->readChunk(&chunk);
					std::shared_ptr<Buffer> buffer(new Buffer(), Buffer::Dt);
					buffer->initialize();
					buffer->bufferData(chunk, size, m_audioStream->format, m_audioStream->frequency);
					queueBuffer(buffer);
					if (_audioStream->eof())
					{
						break;
					}
					++listBuffCount;
				}

				if (!_audioStream->eof())
				{
					while (listBuffCount < frameCount)
					{
						const void * chunk = nullptr;
						size_t size = _audioStream->readChunk(&chunk);
						std::shared_ptr<Buffer> buffer(new Buffer(), Buffer::Dt);
						buffer->initialize();
						buffer->bufferData(chunk, size, m_audioStream->format, m_audioStream->frequency);
						queueBuffer(buffer);
						if (_audioStream->eof())
						{
							break;
						}
						++listBuffCount;
					}
				}
			}
			else
			{
				for (int32_t i = 0; i < _audioStream->pcmFramecount; ++i)
				{
					const void * chunk = nullptr;
					size_t size = _audioStream->readChunk(&chunk);
					std::shared_ptr<Buffer> buffer(new Buffer(), Buffer::Dt);
					buffer->initialize();
					buffer->bufferData(chunk, size, m_audioStream->format, m_audioStream->frequency);
					queueBuffer(buffer);
					if (_audioStream->eof())
					{
						break;
					}
				}
			}

		}

		bool StreamAudioSource::tick()
		{
			if (state != Playing)
				return true;
			if (!m_audioStream->eof())
			{
				ALint idle = processedBufferCount();
				ALint queuedSize = queuedBufferCount();

				std::function<int(StreamAudioSource * _audioSource, std::shared_ptr<Buffer>& _buffer)> queueBufferFunc = [this](StreamAudioSource* _audioSource, std::shared_ptr<Buffer>& _buffer)->int {
					const void* dataChunk;
					size_t size = _audioSource->m_audioStream->readChunk(&dataChunk);
					_buffer->bufferData(dataChunk, size, _audioSource->m_audioStream->format, _audioSource->m_audioStream->frequency);
					m_decodeQueue.push(_buffer);
					return 0;
				};

				for (ALint i = 0; i < idle; ++i)
				{
					// Version 1
					std::shared_ptr<Buffer> buff;
					unqueBuffer(buff);
					m_decodeService->addTask(std::bind(queueBufferFunc, this, std::move(buff)));
					/*
					// Version 2
					std::shared_ptr< Buffer > buff;
					const void * dataChunk;
					size_t size;
					size = audioStream->ReadChunk(&dataChunk);
					ReloadHead(dataChunk, size, audioStream->format, audioStream->frequency);
					*/
					if (m_audioStream->eof())
					{
						break;
					}
				}
				std::shared_ptr<Buffer> buffer;
				while(m_decodeQueue.pop(buffer)) {
					queueBuffer(buffer);
				}
				return true;
			}
			else
			{
				auto queued = queuedBufferCount();
				auto played = processedBufferCount();
				if (queued == played)
				{
					if (m_loop)
					{
						rewind();
					}
					else
					{
						return false;
					}
				}
				return true;
			}
		}

		void StreamAudioSource::play()
		{
			Source::play();
			state = Playing;
		}

		void StreamAudioSource::pause()
		{
			Source::pause();
			state = Paused;
		}

		void StreamAudioSource::rewind()
		{
			Source::stop();
			seek(0.0);
			Source::play();
			state = Playing;
		}

		void StreamAudioSource::seek(double _time)
		{
			m_audioStream->seek(_time);
			// buffȫ��ȡ����������
			std::vector< std::shared_ptr<Buffer> > vecBuff;
			size_t buffCnt = queuedBufferCount();
			for (size_t i = 0; i < buffCnt; ++i)
			{
				std::shared_ptr<Buffer> buff;
				unqueBuffer(buff);
				vecBuff.push_back(buff);
			}
			// ��buff
			for (auto& buff : vecBuff)
			{
				const void * dataChunk;
				size_t size;
				size = m_audioStream->readChunk(&dataChunk);
				buff->bufferData(dataChunk, size, m_audioStream->format, m_audioStream->frequency);
				queueBuffer(buff);
				if (m_audioStream->eof())
				{
					break;
				}
			}
		}

		size_t StreamAudioSource::getPcmSample(uint8_t * data_, size_t _samples)
		{
			int processed = processedBufferCount();
			int offset = GET_BYTE_OFFSET();
			return m_audioStream->getPcmSample(processed, offset, _samples, data_);
		}

		void StreamAudioSource::stop()
		{
			Source::stop();
			seek(0.0);
			state = Stopped;
		}
	}
}