#include "RtAudio.h"
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <bitset>

int record(void* output_buffer,
	void* input_buffer,
	unsigned int n_buffer_frames,
	double stream_time,
	RtAudioStreamStatus status,
	void * user_data
) {
	(void)output_buffer;
	(void)input_buffer;
	(void)stream_time;
	FILE* out = (FILE*)user_data;
	if (status) {
		std::cerr << "Stream overflow detected!" << std::endl;
	}
	float output = 0;
	float * real_input_buffer = (float *)input_buffer;
	for (unsigned int i = 0; i < n_buffer_frames; i++) {
		output += real_input_buffer[i];
		// if (real_input_buffer[i] > output) {
		// 	output = real_input_buffer[i];
		// }
	}
	output /= n_buffer_frames;
	fprintf(out, "%f\n", output);
	return 0;
}

void print_all_info(const RtAudio::DeviceInfo& dev) {
	std::cout << dev.name << "\n";
	std::cout << "\tout:\t" << dev.outputChannels << "\tin:\t"  << dev.inputChannels << "\tdup:\t" << dev.duplexChannels << "\n";
	std::cout << "Sample Rates\n";
	for (const unsigned int i : dev.sampleRates) {
		std::cout << i << "\n";
	}
	static const RtAudioFormat formats[] = {
		RTAUDIO_SINT8,
		RTAUDIO_SINT16,
		RTAUDIO_SINT24,
		RTAUDIO_SINT32,
		RTAUDIO_FLOAT32,
		RTAUDIO_FLOAT64,
	};
	static const char * format_names[] = {
		"SINT8",
		"SINT16",
		"SINT24",
		"SINT32",
		"FLOAT32",
		"FLOAT64",
	};
	for (size_t i = 0; i < sizeof(formats) / sizeof(RtAudioFormat); i++) {
		if (formats[i] & dev.nativeFormats) {
			std::cout << format_names[i] << "\n";
		}
	}
}

int main() {
	RtAudio adc;
	if (adc.getDeviceCount() < 1) {
		std::cout << "\nNo audio devices found!\n";
		return 1;
	}

	unsigned int devices = adc.getDeviceCount();
	RtAudio::DeviceInfo info;

	RtAudio::StreamParameters parameters;
	parameters.deviceId = 1;
	parameters.nChannels = 2;
	parameters.firstChannel = 0;
	unsigned int sample_rate = 44100;
	unsigned int buffer_frames = 16;

	for (unsigned int i = 0; i < devices; i++) {
		std::cout << i << ": ";
		print_all_info(adc.getDeviceInfo(i));
	}

	FILE * out = fopen("output.raw", "wb");
	try {
		adc.openStream(NULL, &parameters, RTAUDIO_FLOAT32, sample_rate, &buffer_frames, record, out);
		adc.startStream();
	} catch (RtAudioError& e) {
		e.printMessage();
		return 1;
	}

	char input;
	std::cin.get(input);

	try {
		adc.stopStream();
	} catch (RtAudioError& e) {
		e.printMessage();
	}

	if (adc.isStreamOpen()) {
		adc.closeStream();
	}

	return 0;
}
