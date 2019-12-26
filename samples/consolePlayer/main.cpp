#include <audioStream.h>
#include <oalStreamSource.h>
#include <io/archieve.h>

int main() {
	auto device = Nix::openal::GetDevice(nullptr);
	auto context = device->CreateContext(nullptr);
	context->MakeCurrent();
	Nix::IArchive* archive = Nix::CreateStdArchieve("E:/CloudMusic/");
	Nix::IFile* file = archive->open("山脇宏子 - 银の意志金の翼.mp3");
	//Nix::IFile * file = archive->open("pdh_dh_0001.ogg");
	Nix::openal::IAudioStream* audioStream = Nix::openal::IAudioStream::FromMP3(file);
	Nix::openal::StreamAudioSource* audioSource = new Nix::openal::StreamAudioSource();
	Nix::openal::AudioDecoderService* decodeService = new Nix::openal::AudioDecoderService();
	decodeService->initialize();
	audioSource->initialize(audioStream, decodeService);
	audioSource->play();
	while (true) {
		audioSource->tick();
		auto mTime = std::chrono::milliseconds(2);
		std::this_thread::sleep_for(mTime);
	}
	return 0;
}