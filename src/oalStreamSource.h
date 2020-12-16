#pragma once
#include "oalDevice.h"
#include "oalBuffer.h"
#include "oalSource.h"
#include "audioStream.h"
#include <vector>
#include <atomic>

#include <condition_variable>
#include <mutex>
#include <thread>
#include <functional>
#include <deque>
#include <LockFreeQueue.h>

namespace Nix
{
	namespace openal
	{

		class AudioDecoderService {
		private:
			std::thread									m_thread;
			std::atomic_bool							m_exit;
			ugi::LockFreeQueue<std::function<int()>>	m_tasks;
			//
			static int DecodeProc(AudioDecoderService* _this) {
				std::function<std::shared_ptr<Buffer>()> task;
				while (!_this->m_exit) {
					std::function<int()> task;
					if (_this->m_tasks.pop(task)) {
						task();
					}
					else {
						auto mTime = std::chrono::milliseconds(20);
						std::this_thread::sleep_for(mTime);
					}
				}
				return 0;
			}
		public:
			AudioDecoderService()
			 : m_tasks(64)
			{
			}

			bool initialize() {
				m_exit = false;
				m_thread = std::move( std::thread(DecodeProc, this));
				return true;
			}

			void addTask(std::function<int()>&& _task) {
				m_tasks.push(_task);
			}

			void terminate() {
				m_exit = true;
			}

		};

		typedef uint8_t byte;
		struct StreamAudioSource:public Source
		{
			enum
			{
				IdleState,
				Playing,
				Paused,
				Stopped,
			} state;
			IAudioStream *									m_audioStream;
			AudioDecoderService*							m_decodeService; 
			ugi::LockFreeQueue< std::shared_ptr<Buffer> >	m_decodeQueue;
			bool											m_loop;
			//
			std::atomic<uint32_t>							m_numBufferQueued;
			// Functions
			StreamAudioSource();
			void initialize(IAudioStream * _audioStream, AudioDecoderService* _decodeService);
			bool tick();
			void seek(double _time);
			size_t getPcmSample(uint8_t * data_, size_t _samples);
			// virtual functions
			virtual void play();
			virtual void pause();
			virtual void stop();
			virtual void rewind();
		};
	}
}

