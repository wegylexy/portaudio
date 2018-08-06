#include<cstdio>
#include<functional>
#include<portaudiocpp/AutoSystem.hxx>
#include<portaudiocpp/CppFunCallbackStream.hxx>
#include<portaudiocpp/Device.hxx>
#include<portaudiocpp/HostApi.hxx>
#include<portaudiocpp/StreamParameters.hxx>
#include<portaudiocpp/System.hxx>
#include<portaudiocpp/SystemDeviceIterator.hxx>
#ifdef _MSC_VER
#include<Windows.h>
#endif

using namespace portaudio;

int main()
{
#ifdef _MSC_VER
	SetConsoleOutputCP(CP_UTF8);
#endif
	static float buffer[480];
	for (;;)
	{
		const AutoSystem paAuto;
		auto &paInstance = System::instance();
		printf("All devices via %s:\n", paInstance.defaultHostApi().name());
		for (auto i = paInstance.devicesBegin(), e = paInstance.devicesEnd(); i != e; ++i)
			printf("%d: %s\n    capture: %d, render: %d, default: %s\n", i->index() + 1, i->name(), i->maxInputChannels(), i->maxOutputChannels(),
				i->isSystemDefaultInputDevice() || i->isSystemDefaultOutputDevice() ? "yes" : "no");
		PaDeviceIndex c, r;
		printf("Capturing device (0 for default): ");
		scanf_s("%d", &c);
		printf("Rendering device (0 for default): ");
		scanf_s("%d", &r);
		const auto &capture = c ? paInstance.deviceByIndex(c - 1) : paInstance.defaultInputDevice(),
			&render = r ? paInstance.deviceByIndex(r - 1) : paInstance.defaultOutputDevice();
		printf("Capturing with %s\n", capture.name());
		printf("Rendering with %s\n", render.name());
		FunCallbackStream fcsC{ StreamParameters{
			DirectionSpecificStreamParameters{ capture, 1, SampleDataFormat::FLOAT32, true, capture.defaultLowInputLatency(), nullptr },
			DirectionSpecificStreamParameters::null(), capture.defaultSampleRate(), 480, paClipOff
		}, [](const void *const inputBuffer, void *const outputBuffer, unsigned long numFrames, const PaStreamCallbackTimeInfo *const timeInfo, const PaStreamCallbackFlags statusFlags, void *const userData)
			{
				memcpy(buffer, inputBuffer, sizeof(float) * 480);
				return static_cast<int>(paContinue);
			}, nullptr
		}, fcsR{ StreamParameters{
			DirectionSpecificStreamParameters::null(),
			DirectionSpecificStreamParameters{ render, 1, SampleDataFormat::FLOAT32, true, render.defaultLowOutputLatency(), nullptr },
			render.defaultSampleRate(), 480, paNoFlag
		}, [](const void *const inputBuffer, void *const outputBuffer, unsigned long numFrames, const PaStreamCallbackTimeInfo *const timeInfo, const PaStreamCallbackFlags statusFlags, void *const userData)
			{
				memcpy(outputBuffer, buffer, sizeof(float) * 480);
				return static_cast<int>(paContinue);
			}, nullptr
		};
		fcsC.start();
		fcsR.start();
		Pa_Sleep(1000);
		puts("Press Enter to restart");
		getchar();
		getchar();
		fcsR.stop();
		fcsC.stop();
	}
	return 0;
}