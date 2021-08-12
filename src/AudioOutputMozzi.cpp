#include "AudioOutputMozzi.h"

bool AudioOutputMozzi::begin()
{
  return true;
}

bool AudioOutputMozzi::ConsumeSample(int16_t sample[2])
{
  _sample = sample;

  return true;
}


bool AudioOutputMozzi::stop()
{
  return true;
}