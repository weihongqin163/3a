#ifndef AGORA_API_3A_H
#define AGORA_API_3A_H



#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

typedef void* AGORA_API_C_HDL;
typedef int AGORA_API_C_INT;
typedef void AGORA_API_C_VOID;

typedef struct _agora_ap_service_config {
    const char* app_id;
    const char* license;
    const char* resource_path;  //like: /user/xx/resource/
} ;

typedef struct _agora_ap_processor_event_handler {
    //context is the user data, user can set it to anything
    void (*on_event)(void* user_data, int event_type);
    void (*on_error)(void* user_data, int error_code);
} ;

typedef struct _agora_ap_aec_config {
    bool enabled;
    bool stereoAecEnabled;
    bool enableAecAutoReset;
    int aecStartupMaxSuppressTimeInMs;
    int filterLength;
    int aecModelType;
    int aiaecSuppressionMode;
    int aecSuppressionMode;
} ;

typedef struct _agora_ap_ans_config {
    bool enabled;
    /**
     * The ANS noise suppression mode. See #AnsSuppressionMode.
     */
    int suppressionMode;
    /**
     * The ANS Model Type. See #AnsModelType.
     */
    int ansModelType;
    /**
     * The ANS speech protect threshold, which is used when
     * possibly speech damage happened by ANS. The value range
     * is [0, 100], the default value is 100, and we recommend
     * 50 when you intended to protect speech.
     */
    int speechProtectThreshold;
} ;

typedef struct _agora_ap_agc_config {
     /**
     * Whether to enable AGC.
     * - `true`: Enable AGC.
     * - `false`: (Default) Disable AGC.
     */
    bool enabled;
    /**
     * Whether to use analog AGC working mode.
     * - `true`: Use anglog mode. Analog AGC is only available for
     * platforms with an analog volume control on the capture device.
     * The SetStreamAnalogLevel and StreamAnalogLevel methods provide
     * coupling between OS analog volume control and AGC.
     * - `false`: (Default) Use digital mode.
     */
    bool useAnalogMode;
    /**
     * Maximum digital AGC gain in dB.
     * Recommended values should be within [6,30],
     * and default value is 12.
     **/
    int maxDigitalGaindB;
    /**
     * target digital AGC level in dB.
     * Recommended values should be within [18,0],
     * and default value is 6.
     **/
    int targetleveldB;
    /**
     * digital AGC slope
     * Recommended values should be within [3,30],
     * and default value is 17.
     **/
    int curve_slope;    
} ;

typedef struct _agora_ap_bghvs_config {
     /**
     * Whether to enable sessCtrl and BGHVS.
     * - `true`: Enable BGHVS.
     * - `false`: (Default) Disable BGHVS.
     */
    bool enabled; 

    int bghvsSOSLenInMs;     //how long to trigger SOS,in ms;

    int bghvsEOSLenInMs;     //how long to trigger EOS,in ms;

    int bghvsSppMode;       //bghvs aggressive level;

    int bghvsDelayInFrmNums; //bghvs algorithm delay,frm number,10ms per frame;
} ;

typedef struct _agora_ap_processor_config {
    // aec
    struct _agora_ap_aec_config aec_config;
    // ns
    struct _agora_ap_ans_config ans_config;
    // agc
    struct _agora_ap_agc_config agc_config;
    //bghvs
    struct _agora_ap_bghvs_config bghvs_config;    
} ;

typedef struct _agora_ap_audio_frame {
  /**
   * Audio frame types.
   */


  /**
   * The audio frame type. See #AudioFrameType.， default is kPcm0, 0
   */
  int type;
  /**
   * The number of samples per channel in the audio frame, should
   * be 8000, 16000, 24000, 32000 , 44100 or 48000.
   */
  int sampleRate;
  /**
   * The number of audio channels (interleaved if stereo).
   * - 1: Mono.
   * - 2: Stereo.
   */
  int channels;
  /**
   * The number of samples per channel in this frame, should
   * be (sampleRate / 100) corresponding to 10ms.
   */
  int samplesPerChannel;
  /**
   * The number of bytes per sample. See #BytesPerSample, default is 2
   */
  int bytesPerSample;
  /**
   * The data buffer of the audio frame. When the audio frame uses a stereo
   * channel, the data buffer is interleaved.
   *
   * Buffer size in bytes = channels * samplesPerChannel * bytesPerSample.
   */
  void* buffer;
};


AGORA_API_C_HDL agora_ap_service_create();

/**
 * @ANNOTATION:GROUP:agora_service
 */
AGORA_API_C_INT agora_ap_service_initialize(AGORA_API_C_HDL service_handle, const _agora_ap_service_config* config);

/**
 * @ANNOTATION:GROUP:agora_service
 */
AGORA_API_C_VOID agora_ap_service_release(AGORA_API_C_HDL service_handle);


// ap processor
// return a default config
_agora_ap_processor_config agora_ap_processor_config_create();
const char* agora_ap_processor_config_get_message(_agora_ap_processor_config *config);

AGORA_API_C_HDL agora_ap_processor_create(AGORA_API_C_HDL service_handle, const _agora_ap_processor_config& config);
AGORA_API_C_INT agora_ap_processor_release(AGORA_API_C_HDL processor_handle);
AGORA_API_C_INT agora_ap_processor_process_stream(AGORA_API_C_HDL processor_handle, _agora_ap_audio_frame* frame, _agora_ap_audio_frame* ref_frame);



#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AGORA_API_3A_H