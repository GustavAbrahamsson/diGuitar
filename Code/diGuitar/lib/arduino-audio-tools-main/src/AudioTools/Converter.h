#pragma once
#include "AudioTypes.h"
#include "AudioBasic/Vector.h"
#include "AudioTools/Filter.h"

namespace audio_tools {

/**
 * @brief Converts from a source to a target number with a different type
 * 
 */
class NumberConverter {
    public:
        static int32_t convertFrom24To32(int24_t value)  {
            return value.scale32();
        }

        static int16_t convertFrom24To16(int24_t value)  {
            return value.scale16();
        }

        static float convertFrom24ToFloat(int24_t value)  {
            return value.scaleFloat();
        }

        static int16_t convertFrom32To16(int32_t value)  {
            return static_cast<float>(value) / INT32_MAX * INT16_MAX;
        }

        static int16_t convert16(int value, int value_bits_per_sample){
            return value * NumberConverter::maxValue(16) / NumberConverter::maxValue(value_bits_per_sample);
        }

        static int16_t convert8(int value, int value_bits_per_sample){
            return value * NumberConverter::maxValue(8) / NumberConverter::maxValue(value_bits_per_sample);
        }

        /// provides the biggest number for the indicated number of bits
        static int64_t maxValue(int value_bits_per_sample){
            switch(value_bits_per_sample/8){
                case 8:
                    return 127;
                case 16:
                    return 32767;
                case 24:
                    return 8388607;
                case 32:
                    return 2147483647;

            }
            return 32767;
        }

};


/**
 * @brief Abstract Base class for Converters
 * A converter is processing the data in the indicated array
 * @author Phil Schatzmann
 * @copyright GPLv3
 * @tparam T 
 */
template<typename T>
class BaseConverter {
    public:
        virtual size_t convert(uint8_t *src, size_t size) = 0;
};


/**
 * @brief Dummy converter which does nothing
 * 
 * @tparam T 
 */
template<typename T>
class NOPConverter : public  BaseConverter<T> {
    public:
        virtual size_t convert(uint8_t(*src), size_t size) {return size;};
};

/**
 * @brief Multiplies the values with the indicated factor adds the offset and clips at maxValue. To mute use a factor of 0.0!
 * @author Phil Schatzmann
 * @copyright GPLv3
 * 
 * @tparam T 
 */
template<typename T>
class ConverterScaler : public  BaseConverter<T> {
    public:
        ConverterScaler(float factor, T offset, T maxValue, int channels=2){
            this->factor_value = factor;
            this->maxValue = maxValue;
            this->offset_value = offset;
            this->channels = channels;
        }

        size_t convert(uint8_t*src, size_t byte_count) {
            T *sample = (T*)src;
            int size = byte_count / channels / sizeof(T);
            for (size_t j=0;j<size;j++){
                for (int i=0;i<channels;i++){
                    *sample = (*sample + offset_value) * factor_value;
                    if (*sample>maxValue){
                        *sample = maxValue;
                    } else if (*sample<-maxValue){
                        *sample = -maxValue;
                    }
                    sample++;
                }
            }
            return byte_count;
        }

        /// Defines the factor (volume)
        void setFactor(T factor){
            this->factor_value = factor;
        }

        /// Defines the offset
        void setOffset(T offset) {
            this->offset_value = offset;
        }

        /// Determines the actual factor (volume)
        float factor() {
            return factor_value;
        }

        /// Determines the offset value
        T offset() {
            return offset_value;
        }

    protected:
        int channels;
        float factor_value;
        T maxValue;
        T offset_value;
};

/**
 * @brief Makes sure that the avg of the signal is set to 0
 * 
 * @tparam T 
 */
template<typename T>
class ConverterAutoCenter : public  BaseConverter<T> {
    public:
        ConverterAutoCenter(int channels=2){
            this->channels = channels;
        }

        size_t convert(uint8_t(*src), size_t byte_count) {
            int size = byte_count / channels / sizeof(T);
            T *sample = (T*) src;
            setup((T*)src, size);
            if (is_setup){
                for (size_t j=0; j<size; j++){
                    for (int i=0;i<channels;i++){
                        *sample = *sample - offset;
                        sample++;
                    }
                }
            }
            return byte_count;
        }

    protected:
        T offset;
        float left;
        float right;
        bool is_setup = false;
        int channels;

        void setup(T *src, size_t size){
            if (!is_setup) {
                T *sample = (T*) src;
                for (size_t j=0;j<size;j++){
                    left += *sample++;
                    right += *sample++;
                }
                left = left / size;
                right = right / size;

                if (left>0){
                    offset = left;
                    is_setup = true;
                } else if (right>0){
                    offset = right;
                    is_setup = true;
                }
            }
        }
};

/**
 * @brief Switches the left and right channel
 * @author Phil Schatzmann
 * @copyright GPLv3
 * 
 * @tparam T 
 */
template<typename T>
class ConverterSwitchLeftAndRight : public  BaseConverter<T> {
    public:
        ConverterSwitchLeftAndRight(int channels=2){
            this->channels = channels;
        }

        size_t convert(uint8_t*src, size_t byte_count) {
            if (channels==2){
                int size = byte_count / channels / sizeof(T);
                T *sample = (T*) src;
                for (size_t j=0;j<size;j++){
                    T temp = *sample;
                    *sample = *(sample+1)
                    *(sample+1) = temp;
                    sample+=2;
                }
            }
            return byte_count;
        }
    protected:
        int channels=2;
};


enum FillLeftAndRightStatus {Auto, LeftIsEmpty, RightIsEmpty};

/**
 * @brief Make sure that both channels contain any data
 * @author Phil Schatzmann
 * @copyright GPLv3
 * 
 * @tparam T 
 */


template<typename T>
class ConverterFillLeftAndRight : public  BaseConverter<T> {
    public:
        ConverterFillLeftAndRight(FillLeftAndRightStatus config=Auto, int channels=2){
            this->channels = channels;
            switch(config){
                case LeftIsEmpty:
                    left_empty = true;
                    right_empty = false;
                    is_setup = true;
                    break;
                case RightIsEmpty:
                    left_empty = false;
                    right_empty = true;
                    is_setup = true;
                    break;
                case Auto:
                    is_setup = false;
                    break;
            }
        }

        size_t convert(uint8_t*src, size_t byte_count) {
            if (channels==2){
                int size = byte_count / channels / sizeof(T);
                setup((T*)src, size);
                if (left_empty && !right_empty){
                    T *sample = (T*)src;
                    for (size_t j=0;j<size;j++){
                        *sample = *(sample+1);
                        sample+=2;
                    }
                } else if (!left_empty && right_empty) {
                    T *sample = (T*)src;
                    for (size_t j=0;j<size;j++){
                        *(sample+1) = *sample;
                        sample+=2;
                    }
                }
            }
            return byte_count;
        }

    private:
        bool is_setup = false;
        bool left_empty = true;
        bool right_empty = true; 
        int channels;

        void setup(T *src, size_t size) {
            if (!is_setup) {
                T *sample = (T*) src;
                for (int j=0;j<size;j++) {
                    if (*src!=0) {
                        left_empty = false;
                        break;
                    }
                    src+=2;
                }
                sample = src+1;
                for (int j=0;j<size-1;j++) {
                    if (*(src)!=0) {
                        right_empty = false;
                        break;
                    }
                    src+=2;
                }
                // stop setup as soon as we found some data
                if (!right_empty || !left_empty) {
                    is_setup = true;
                }
            }
        }

};

/**
 * @brief special case for internal DAC output, the incomming PCM buffer needs 
 *  to be converted from signed 16bit to unsigned
 * @author Phil Schatzmann
 * @copyright GPLv3
 * 
 * @tparam T 
 */
template<typename T>
class ConverterToInternalDACFormat : public  BaseConverter<T> {
    public:
        ConverterToInternalDACFormat(int channels=2){
            this->channels = channels;
        }

        size_t convert(uint8_t*src, size_t byte_count) {
            int size = byte_count / channels / sizeof(T);
            T *sample = (T*) src;
            for (int i=0; i<size; i++) {
                for (int j=0;j<channels;j++){
                    *sample = *sample + 0x8000;
                    sample++;
                }
            }
            return byte_count;
        }
    protected:
        int channels;
};

/**
 * @brief We combine a datastream which consists of multiple channels into less channels. E.g. 2 to 1
 * The last target channel will contain the combined values of the exceeding source channels.
 * 
 * @tparam T 
 */
template<typename T>
class ChannelReducer : public BaseConverter<T> {
    public:
        ChannelReducer() = default;

        ChannelReducer(int channelCountOfTarget, int channelCountOfSource){
            from_channels = channelCountOfSource;
            to_channels = channelCountOfTarget;
        }

        void setSourceChannels(int channelCountOfSource) {
            from_channels = channelCountOfSource;
        }

        void setTargetChannels(int channelCountOfTarget) {
            to_channels = channelCountOfTarget;
        }

        size_t convert(uint8_t*src, size_t size) {
            return convert(src,src,size);
        }

        size_t convert(uint8_t*target, uint8_t*src, size_t size) {
            int frame_count = size/(sizeof(T)*from_channels);
            size_t result_size=0;
            T* result = (T*)target;
            T* source = (T*)src;
            int reduceDiv = from_channels-to_channels+1;

            for(int i=0; i < frame_count; i++){
                // copy first to_channels-1 
                for (int j=0;j<to_channels-1;j++){
                    *result++ = *source++;
                    result_size += sizeof(T);
                }
                // commbined last channels
                T total = 0;
                for (int j=to_channels-1;j<from_channels;j++){
                    total += *source++ / reduceDiv;
                }                
                *result++ = total;
                result_size += sizeof(T);
            }
            return result_size;
        }

    protected:
        int from_channels;
        int to_channels;
};



/**
 * @brief Combines multiple converters
 * 
 * @tparam T 
 */
template<typename T>
class MultiConverter : public BaseConverter<T> {
    public:
        MultiConverter(){
        }

        MultiConverter(BaseConverter<T> &c1){
            add(c1);
        }

        MultiConverter(BaseConverter<T> &c1, BaseConverter<T> &c2){
            add(c1);
            add(c2);
        }

        MultiConverter(BaseConverter<T> &c1, BaseConverter<T> &c2, BaseConverter<T> &c3){
            add(c1);
            add(c2);
            add(c3);
        }

        // adds a converter
        void add(BaseConverter<T> &converter){
            converters.push_back(&converter);
        }

        // The data is provided as int24_t tgt[][2] but  returned as int24_t
        size_t convert(uint8_t*src, size_t size) {
            for(int i=0; i < converters.size(); i++){
                converters[i]->convert(src, size);
            }
            return size;
        }

    private:
        Vector<BaseConverter<T>*> converters;

};

/**
 * @brief Converts e.g. 24bit data to the indicated smaller or bigger data type
 * @author Phil Schatzmann
 * @copyright GPLv3
 * 
 * @tparam T 
 */
template<typename FromType, typename ToType>
class FormatConverter {
    public:
        FormatConverter(ToType (*converter)(FromType v)){
            this->convert_ptr = converter;
        }

        FormatConverter( float factor, float clip){
            this->factor = factor;
            this->clip = clip;
        }

        // The data is provided as int24_t tgt[][2] but  returned as int24_t
        size_t convert(uint8_t *src, uint8_t *target, size_t byte_count_src) {
            return convert((FromType *)src, (ToType*)target, byte_count_src );
        }

        // The data is provided as int24_t tgt[][2] but  returned as int24_t
        size_t convert(FromType *src, ToType *target, size_t byte_count_src) {
            int size = byte_count_src / sizeof(FromType);
            FromType *s = src;
            ToType *t = target;
            if (convert_ptr!=nullptr){
                // standard conversion
                for (int i=size; i>0; i--) {
                    *t = (*convert_ptr)(*s);
                    t++;
                    s++;
                }
            } else {
                // conversion using indicated factor
                for (int i=size; i>0; i--) {
                    float tmp = factor * *s;
                    if (tmp>clip){
                        tmp=clip;
                    } else if (tmp<-clip){
                        tmp = -clip;
                    }
                    *t = tmp;
                    t++;
                    s++;
                }
            }
            return size*sizeof(ToType);
        }

    private:
        ToType (*convert_ptr)(FromType v) = nullptr;
        float factor=0;
        float clip=0;

};



/**
 * @brief Reads n numbers from an Arduino Stream
 * 
 */
class NumberReader {
    public:
        NumberReader(Stream &in) {
            stream_ptr = &in;
        }

        NumberReader() {
        }

        bool read(int inBits, int outBits, bool outSigned, int n, int32_t *result){
            bool result_bool = false;
            int len = inBits/8 * n;
            if (stream_ptr!=nullptr && stream_ptr->available()>len){
                uint8_t buffer[len];
                stream_ptr->readBytes((uint8_t*)buffer, n * len);
                result_bool = toNumbers((void*)buffer, inBits, outBits, outSigned, n, result);
            }
            return result_bool;
        }

        /// converts a buffer to a number array
        bool toNumbers(void *bufferIn, int inBits, int outBits, bool outSigned, int n, int32_t *result){
            bool result_bool = false;
            switch(inBits){
                case 8: {
                        int8_t *buffer=(int8_t *)bufferIn;
                        for (int j=0;j<n;j++){
                            result[j] = scale(buffer[j],inBits,outBits,outSigned);
                        }
                        result_bool = true;
                    }
                    break;
                case 16: {
                        int16_t *buffer=(int16_t *)bufferIn;
                        for (int j=0;j<n;j++){
                            result[j] = scale(buffer[j],inBits,outBits,outSigned);
                        }
                        result_bool = true;
                    }
                    break;
                case 32: {
                        int32_t *buffer=(int32_t*)bufferIn;
                        for (int j=0;j<n;j++){
                            result[j] = scale(buffer[j],inBits,outBits,outSigned);
                        }
                        result_bool = true;
                    }
                    break;
            }
            return result_bool;

        }

    protected:
        Stream *stream_ptr=nullptr;

        /// scale the value
        int32_t scale(int32_t value, int inBits, int outBits, bool outSigned=true){
            int32_t result = static_cast<float>(value) / NumberConverter::maxValue(inBits) * NumberConverter::maxValue(outBits);
            if (!outSigned){
                result += (NumberConverter::maxValue(outBits) / 2);
            }
            return result;
        }

};


/**
 * @brief Converter for 1 Channel which applies the indicated Filter
 * @author pschatzmann
 * @tparam T
 */
template <typename T>
class Converter1Channel : public BaseConverter<T> {
 public:
  Converter1Channel(Filter<T> &filter) { this->p_filter = &filter; }

  size_t convert(uint8_t *src, size_t size) {
    T *data = (T *)src;
    for (size_t j = 0; j < size; j++) {
      data[j] = p_filter->process(data[j]);
    }
    return size;
  }

 protected:
  Filter<T> *p_filter = nullptr;
};

/**
 * @brief Converter for n Channels which applies the indicated Filter
 * @author pschatzmann
 * @tparam T
 */
template <typename T, typename FT>
class ConverterNChannels : public BaseConverter<T> {
 public:
  /// Default Constructor
  ConverterNChannels(int channels) {
    this->channels = channels;
    filters = new Filter<FT> *[channels];
    // make sure that we have 1 filter per channel
    for (int j = 0; j < channels; j++) {
      filters[j] = nullptr;
    }
  }

  /// Destrucotr
  ~ConverterNChannels() {
    for (int j = 0; j < channels; j++) {
      if (filters[j]!=nullptr){
        delete filters[j];
      }
    }
    delete[] filters;
    filters = 0;
  }

  /// defines the filter for an individual channel - the first channel is 0
  void setFilter(int channel, Filter<FT> *filter) {
    if (channel<channels){
      if (filters[channel]!=nullptr){
        delete filters[channel];
      }
      filters[channel] = filter;
    } else {
      LOGE("Invalid channel nummber %d - max channel is %d", channel, channels-1);
    }
  }

  // convert all samples for each channel separately
  size_t convert(uint8_t *src, size_t size) {
    int count = size / channels / sizeof(T);
    T *sample = (T *)src;
    for (size_t j = 0; j < count; j++) {
      for (int channel = 0; channel < channels; channel++) {
        if (filters[channel]!=nullptr){
          *sample = filters[channel]->process(*sample);
        }
        sample++;
      }
    }
    return size;
  }

 protected:
  Filter<FT> **filters = nullptr;
  int channels;
};

/**
 * @brief Removes any silence from the buffer that is longer then n samples with a amplitude
 * below the indicated threshhold. If you process multiple channels you need to multiply the
 * channels with the number of samples to indicate n
 * 
 * @tparam T 
 */

template <typename T>
class SilenceRemovalConverter : public BaseConverter<T>  {
 public:

  SilenceRemovalConverter(int n = 8, int aplidudeLimit = 2) { 
  	set(n, aplidudeLimit);
  }

  virtual size_t convert(uint8_t *data, size_t size) override {
    if (!active) {
      // no change to the data
      return size;
    }
    size_t sample_count = size / sizeof(T);
    size_t write_count = 0;
    T *audio = (T *)data;

    // find relevant data
    T *p_buffer = (T *)data;
    for (int j = 0; j < sample_count; j++) {
      int pos = findLastAudioPos(audio, j);
      if (pos < n) {
        write_count++;
        *p_buffer++ = audio[j];
      }
    }
    
    // write audio data w/o silence
    size_t write_size = write_count * sizeof(T);
    LOGI("filtered silence from %d -> %d", (int)size, (int)write_size);

    // number of empty samples of prior buffer
    priorLastAudioPos =  findLastAudioPos(audio, sample_count - 1);

    // return new data size
    return write_size;
  }
  
 protected:
  bool active = false;
  const uint8_t *buffer = nullptr;
  int n;
  int priorLastAudioPos = 0;
  int amplidude_limit = 0;

  void set(int n = 5, int aplidudeLimit = 2) {
    LOGI("begin(n=%d, aplidudeLimit=%d", n, aplidudeLimit);
    this->n = n;
    this->amplidude_limit = aplidudeLimit;
    this->priorLastAudioPos = n+1;  // ignore first values
    this->active = n > 0;
  }


  // find last position which contains audible data
  int findLastAudioPos(T *audio, int pos) {
    for (int j = 0; j < n; j++) {
      // we are before the start of the current buffer
      if (pos - j <= 0) {
        return priorLastAudioPos;
      }
      // we are in the current buffer
      if (abs(audio[pos - j]) > amplidude_limit) {
        return j;
      }
    }
    return n + 1;
  }
};


}