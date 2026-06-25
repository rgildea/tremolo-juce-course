#pragma once

namespace tremolo {
class Tremolo {
public:
  Tremolo() {
    lfo.setFrequency( 5.0f /* Hz */, true);
  }
  void prepare(double sampleRate, int expectedMaxFramesPerBlock) {
    const juce::dsp::ProcessSpec processSpec {
      .sampleRate = sampleRate,
      .maximumBlockSize = static_cast<juce::uint32>(expectedMaxFramesPerBlock),
      .numChannels = 1u,
    };
    lfo.prepare(processSpec);
  }

  void process(juce::AudioBuffer<float>& buffer) noexcept {
    // for each frame
    for (const auto frameIndex : std::views::iota(0, buffer.getNumSamples())) {
      const auto lfoValue = lfo.processSample(0.f);
      constexpr auto modulationDepth = 0.4f;
      // instead of range [-1, 1], we want to use the range [0,1].
      // shift the lfoValue up to [0,2] by adding 1, and then scale by half to get a value between and 1.
      const auto lfoValueScaled = (lfoValue +1) * 0.5f;
      // if the lfoValue is 0, we do nothing. if it's 1, we do maximum modulation, scaled by the modulationDepth.
      const auto modulationValue = 1.0f - (modulationDepth * lfoValueScaled);

      // for each channel sample in the frame
      for (const auto channelIndex :
           std::views::iota(0, buffer.getNumChannels())) {
        // get the input sample
        const auto inputSample = buffer.getSample(channelIndex, frameIndex);

        // modulate the sample
        const auto outputSample = inputSample * modulationValue;

        // set the output sample
        buffer.setSample(channelIndex, frameIndex, outputSample);
      }
    }
  }

  void reset() noexcept {
    lfo.reset();
  }

private:
  juce::dsp::Oscillator<float> lfo {[](auto phase){ return std::sin(phase); }};

};
}  // namespace tremolo
