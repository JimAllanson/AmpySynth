#ifndef _AUDIOOUTPUTMOZZIWAV_H
#define _AUDIOOUTPUTMOZZIWAV_H

#include "AudioOutput.h"

class AudioOutputMozzi : public AudioOutput
{
  public:
    int16_t *_sample;
    AudioOutputMozzi(int16_t *sample) {
        _sample = sample;
    }
    ;
    ~AudioOutputMozzi() {};

    virtual bool begin() override;
    virtual bool ConsumeSample(int16_t sample[2]) override;
    virtual bool stop() override;
  private:
    int count;
};

#endif
