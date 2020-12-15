#include <cmath>
#include <cstdio>
#include <iostream>
#include <vector>
#include <fstream>
#include <complex>

// Структура, описывающая заголовок WAV файла.
struct WavHeader {
  // WAV-формат начинается с RIFF-заголовка:

  // Содержит символы "RIFF" в ASCII кодировке
  // (0x52494646 в big-endian представлении)
  char chunk_id[4];

  // 36 + subchunk2Size, или более точно:
  // 4 + (8 + subchunk1Size) + (8 + subchunk2Size)
  // Это оставшийся размер цепочки, начиная с этой позиции.
  // Иначе говоря, это размер файла - 8, то есть,
  // исключены поля chunkId и chunkSize.
  unsigned int chunk_size;

  // Содержит символы "WAVE"
  // (0x57415645 в big-endian представлении)
  char format[4];

  // Формат "WAVE" состоит из двух подцепочек: "fmt " и "data":
  // Подцепочка "fmt " описывает формат звуковых данных:

  // Содержит символы "fmt "
  // (0x666d7420 в big-endian представлении)
  char subchunk1_id[4];

  // 16 для формата PCM.
  // Это оставшийся размер подцепочки, начиная с этой позиции.
  unsigned int subchunk1_size;

  // Аудио формат, полный список можно получить здесь http://audiocoding.ru/wav_formats.txt
  // Для PCM = 1 (то есть, Линейное квантование).
  // Значения, отличающиеся от 1, обозначают некоторый формат сжатия.
  unsigned short audio_format;

  // Количество каналов. Моно = 1, Стерео = 2 и т.д.
  unsigned short num_channels;

  // Частота дискретизации. 8000 Гц, 44100 Гц и т.д.
  unsigned int sample_rate;

  // sampleRate * numChannels * bitsPerSample/8
  unsigned int byte_rate;

  // numChannels * bitsPerSample/8
  // Количество байт для одного сэмпла, включая все каналы.
  unsigned short block_align;

  // Так называемая "глубиная" или точность звучания. 8 бит, 16 бит и т.д.
  unsigned short bits_per_sample;

  // Подцепочка "data" содержит аудио-данные и их размер.

  // Содержит символы "data"
  // (0x64617461 в big-endian представлении)
  char subchunk2_id[4];

  // numSamples * numChannels * bitsPerSample/8
  // Количество байт в области данных.
  unsigned int subchunk2_size;

  // Далее следуют непосредственно Wav данные.
};

// 1 channel, 16 bits per sample
struct WavFile {
  explicit WavFile(std::ifstream &in_stream) {
    in_stream.read(reinterpret_cast<char *>(&header), sizeof(header));
    data.resize(header.subchunk2_size / sizeof(int16_t));
    in_stream.read(reinterpret_cast<char *>(data.data()), header.subchunk2_size);
  }

  void Write(std::ofstream &out_stream) const {
    out_stream.write(reinterpret_cast<const char *>(&header), sizeof(header));
    out_stream.write(reinterpret_cast<const char *>(data.data()), data.size() * sizeof(int16_t));
  }

  void PrintMetadata() const {
    std::cout << header.chunk_id[0] << header.chunk_id[1] << header.chunk_id[2]
              << header.chunk_id[3]
              << '\n';
    std::cout << "Chunk size: " << header.chunk_size << '\n';
    std::cout << header.format[0] << header.format[1] << header.format[2] << header.format[3]
              << '\n';
    std::cout << header.subchunk1_id[0] << header.subchunk1_id[1] << header.subchunk1_id[2]
              << header.subchunk1_id[3] << '\n';
    std::cout << "SubChunkId1: " << header.subchunk1_size << '\n';
    std::cout << "Audio format: " << header.audio_format << '\n';
    std::cout << "Channels: " << header.num_channels << '\n';
    std::cout << "Sample rate: " << header.sample_rate << '\n';
    std::cout << "Bits per sample: " << header.bits_per_sample << '\n';
    std::cout << header.subchunk2_id[0] << header.subchunk2_id[1] << header.subchunk2_id[2]
              << header.subchunk2_id[3] << '\n';

    // Посчитаем длительность воспроизведения в секундах
    double duration_seconds =
        1.0 * header.subchunk2_size / (header.bits_per_sample / 8) / header.num_channels
            / header.sample_rate;
    int duration_minutes = std::floor(duration_seconds / 60.0);
    duration_seconds = duration_seconds - (duration_minutes * 60);
    std::cout << "Duration: " << duration_minutes << ":" << duration_seconds << '\n';
    std::cout << "Data size: " << header.subchunk2_size << '\n';
  }

  WavHeader header{};
  std::vector<int16_t> data;
};

// rounded up
size_t UpperPower2(size_t x) {
  size_t ans = 1;

  while (ans < x)
    ans *= 2;

  return ans;
}

using complex = std::complex<double>;

// a.size() == 2^k
void FFT(std::vector<complex> &a, bool inverse) {
  if (a.size() == 1)
    return;

  size_t n = a.size();
  std::vector<complex> even_half(n / 2);
  std::vector<complex> odd_half(n / 2);

  for (size_t i = 0; i < n; ++i)
    if (i % 2 == 0)
      even_half[i / 2] = a[i];
    else
      odd_half[i / 2] = a[i];

  FFT(even_half, inverse);
  FFT(odd_half, inverse);

  double angle = (inverse ? -1.0 : 1.0) * 2.0 * M_PI / n;
  complex prime_root = {std::cos(angle), std::sin(angle)};
  complex root = 1.0;

  for (size_t i = 0; i < n / 2; ++i, root *= prime_root) {
    a[i] = even_half[i] + root * odd_half[i];
    a[i + n / 2] = even_half[i] - root * odd_half[i];

    // we need to divide the result by a.size() == 2^k, so we divide by 2 on each level of recursion
    if (inverse) {
      a[i] /= 2;
      a[i + n / 2] /= 2;
    }
  }
}

std::vector<complex> ToComplex(const std::vector<int16_t> &a) {
  size_t n = UpperPower2(a.size());
  std::vector<complex> result(n, 0.0);

  for (size_t i = 0; i < a.size(); ++i)
    result[i] = a[i];

  return result;
}

std::vector<int16_t> FromComplex(const std::vector<complex> &a, size_t size) {
  std::vector<int16_t> result(size);

  for (size_t i = 0; i < size; ++i)
    result[i] = a[i].real();

  return result;
}

int main() {
  const char *input_filename = "speech.wav";
  std::ifstream input_file(input_filename);

  if (!input_file) {
    std::cout << "Failed to open input file\n";
    return 1;
  }

  WavFile wav(input_file);

  if (wav.header.bits_per_sample != 16) {
    std::cout << "Wrong bits per sample\n";
    return 1;
  }

  std::cout << "Data is successfully loaded." << '\n';

  std::vector<complex> c_data = ToComplex(wav.data);
  FFT(c_data, false);

  // Обнуляем 80%
  for (size_t i = c_data.size() * 0.2; i < c_data.size(); ++i)
    c_data[i] = 0.0;

  FFT(c_data, true);
  wav.data = FromComplex(c_data, wav.data.size());

  const char *output_filename = "speech_result.wav";
  std::ofstream output_file(output_filename);

  if (!output_file) {
    std::cout << "Failed to open or create output file\n";
    return 1;
  }

  wav.Write(output_file);

  std::cout << "Data transformed successfully\n";

  return 0;
}
