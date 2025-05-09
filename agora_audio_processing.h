//
//  Agora Uplink Audio Processing Library
//
//  Copyright (c) 2023 Agora IO. All rights reserved.
//
//  This program is confidential and proprietary to Agora.io.
//  And may not be copied, reproduced, modified, disclosed to others, published
//  or used, in whole or in part, without the express prior written permission
//  of Agora.io.

#pragma once

#include <memory>

#include "agora_uap_frame.h"
#include "agora_uap_optional.h"

#if defined(_WIN32)
#define AGORA_UAP_API __declspec(dllexport)
#include <windows.h>
#elif defined(__APPLE__) || defined(__ANDROID__)
#define AGORA_UAP_API __attribute__((visibility("default")))
#define WINAPI
#else
#define AGORA_UAP_API __attribute__((visibility("default")))
#define WINAPI
#endif

#define AGORA_MAX_UUID_LENGTH 128

namespace AgoraUAP {

class AgoraAudioProcessingEventHandler;

/**
 * The AgoraAudioProcessing class, which is the basic interface that provides
 * access to Agora uplink audio processing algorithms.
 */
class AgoraAudioProcessing {
 public:
  /**
   * The UAP configuration.
   */
  struct UapConfig {
    /**
     * appId which you can get it from agora console.
     */
    const char* appId;
    /**
     * license for audio processing engine.
     */
    const char* license;
    /**
     * eventHandler.
     */
    AgoraAudioProcessingEventHandler* eventHandler;
    UapConfig(const char* appId, const char* license,
              AgoraAudioProcessingEventHandler* eventHandler)
        : appId(appId), license(license), eventHandler(eventHandler) {}
  };

  using LogOutputFunc = void (*)(const char* log);
  /**
   * The log output option.
   */
  enum LogLevel {
    LS_DEBUG = 0x00,
    LS_INFO = 0x01,
    LS_WARNING = 0x02,
    LS_ERROR = 0x04,
    LS_FATAL = 0x08,
    LS_NONE = 0x0F,
  };
  struct LogOption {
    /**
     * Whether to enable log output.
     * - `true`: Enable log output.
     * - `false`: (Default) Disable log output.
     */
    bool enabled;

    /**
     * Whether to output verbose log.
     * - `true`: Enable log output.
     * - `false`: (Default) Disable log output.
     */
    bool verboseMode;

    /**
     * The log output function pointer. Should be implemented
     * asynchronously in order not to block the calling thread.
     */
    LogOutputFunc func;

    /**
     * The log output level. default is LS_INFO
     */
    LogLevel logLevel;

    /**
     * The log file set by user, null for default log path.
     */
    const char* logPath;

    /**
     * The log size set by user (unit is KB)
     * unit is KB , default is 1MB = 1024 KB
     */
    int logSize;

    LogOption() : enabled(false), verboseMode(false), func(nullptr), logLevel(LS_INFO) {}
    LogOption(bool enable, bool verbose, LogOutputFunc ptr, LogLevel level = LS_INFO,
              const char* path = nullptr, int size = 256)
        : enabled(enable),
          verboseMode(verbose),
          func(ptr),
          logLevel(level),
          logPath(path),
          logSize(size) {}
  };

  /**
   * The PCM dump option.
   */
  struct DumpOption {
    /**
     * Whether to enable PCM dump.
     * - `true`: Enable PCM dump.
     * - `false`: (Default) Disable PCM dump. Must be called
     * to finish dumping otherwise PCM files may be corrupted.
     */
    bool enabled;
    /**
     * The directory to save dump files. Ensure that the
     * directory exists and is writable.
     */
    const char* path;

    DumpOption() : enabled(false), path(nullptr) {}
    DumpOption(bool enable, const char* dir) : enabled(enable), path(dir) {}
  };

  /**
   * The AI Model configuration.
   */
  struct AiModelResourceConfig {
    /**
     * A pointer pointing to the memory location of the model data,
     * which should be provided by the user.
     */
    AgoraUAP::optional<std::shared_ptr<void>> modelDataPtr;
    /**
     * The model name corresponding to the model data.
     * Legal values should be "ainlp"or "ainlp_ll".
     * "ainlp" refers to the standard-latency AIAEC.
     * "ainlp_ll" refers to the low-latency AIAEC.
     */
    AgoraUAP::optional<char*> modelName;
    /**
     * The model size corresponding to the model data.
     * The unit is byte.
     */
    AgoraUAP::optional<size_t> modelDataSize;
  };

  /**
   * The AEC linear filter length.
   */
  enum AecFilterLength {
    /**
     * kNormal: (Default) 48ms for normal environment.
     */
    kNormal,
    /**
     * kLong: 200ms for reverberant environment.
     */
    kLong,
    /**
     * kLongest: 480ms for highly reverberant environment.
     */
    kLongest
  };

  /**
   * The AEC Model Type.
   */
  enum AecModelType {
    /**
     * kTRAEC: (Default) traditional AEC model.
     */
    kTRAEC,
    /**
     * kLLAIAEC: low-latency AIAEC algorithm.
     */
    kLLAIAEC,
    /**
     * kSTDAIAEC: standard-latency AIAEC algorithm.
     */
    kSTDAIAEC
  };

  /**
   * The AIAEC Suppression Mode.
   */
  enum AIAECSuppressionMode {
    /**
     * kChatMode: (Default) AIAEC chat mode, which
     * has a aggressive suppressione level.
     */
    kChatMode,
    /**
     * kSingMode: AIAEC sing mode, which has a mild
     * suppressione level.
     */
    kSingMode,
    /**
     * kSupperSingMode: AIAEC sing mode, which has a
     * super mild suppressione level.
     */
    kSupperSingMode,
    /**
     * kPerfectSingMode: AIAEC sing mode, which has
     * a minor suppressione level.
     */
    kPerfectSingMode
  };

  /**
   * The AEC Suppression Level.
   */
  enum AECSuppressionMode {
    /**
     * kMildAEC: mild AEC suppressione level.
     */
    kMildAEC,
    /**
     * kNormalAEC: normal AEC suppressione level.
     */
    kNormalAEC,
    /**
     * kAggressiveAEC: (Default) AEC suppressione level.
     */
    kAggressiveAEC,
    /**
     * kSuperAggressiveAEC: super AEC suppressione level.
     */
    kSuperAggressiveAEC,
    /**
     * kExtremeAEC: extream AEC suppressione level.
     */
    kExtremeAEC
  };

  /**
   * The AEC configuration.
   */
  struct AecConfig {
    /**
     * Whether to enable AEC.
     * - `true`: Enable AEC.
     * - `false`: (Default) Disable AEC.
     */
    AgoraUAP::optional<bool> enabled;
    /**
     * Whether to enable stereo AEC.
     * - `true`: Enable stereo AEC.
     * - `false`: (Default) Disable stereo AEC.
     */
    AgoraUAP::optional<bool> stereoAecEnabled;
    /**
     * Whether to enable AEC auto-reset when AEC is unavailable.
     * - `true`: Enable AEC auto-reset.
     * - `false`: (Default) Disable AEC auto-reset.
     */
    AgoraUAP::optional<bool> enableAecAutoReset;
    /**
     * when aec is in startup state, an extreme aec suppression is
     * needed. aecStartupMaxSuppressTimeInMs, measured in milliseconds,
     * is the max time for extreme aec suppression operation.
     * The value range is [0, (1 << 30) - 1], and default value is (1 << 30) - 1.
     */
    AgoraUAP::optional<int> aecStartupMaxSuppressTimeInMs;
    /**
     * The AEC linear filter length. See #AecFilterLength.
     */
    AgoraUAP::optional<AecFilterLength> filterLength;
    /**
     * The AEC Model Type. See #AecModelType.
     */
    AgoraUAP::optional<AecModelType> aecModelType;
    /**
     * The AEC suppression level. See #AECSuppressionMode.
     */
    AgoraUAP::optional<AECSuppressionMode> aecSuppressionMode;
    /**
     * The AIAEC suppression mode. See #AIAECSuppressionMode.
     */
    AgoraUAP::optional<AIAECSuppressionMode> aiaecSuppressionMode;
  };

  /**
   * The ANS noise suppression mode.
   */
  enum AnsSuppressionMode {
    /**
     * kMild: Low suppression level
     */
    kMild,
    /**
     * kModerate: (Default) Moderate suppression level.
     */
    kModerate,
    /**
     * kAggressive: High suppression level.
     */
    kAggressive,
    /**
     * kExtreme: extream suppression level.
     */
    kExtreme   
  };

  /**
   * The AEC Model Type.
   */
  enum AnsModelType {
    /**
     * kTRANS: (Default) traditional ANS model.
     */
    kTRANS,
    /**
     * kLLAIAEC: low-latency AIANS algorithm.
     */
    kLLAIANS,
    /**
     * kSTDAIAEC: standard-latency AIANS algorithm.
     */
    kSTDAIANS
  };

  /**
   * The BGHVS mode.
   */
  enum BghvsSuppressionMode {
    /**
     * kMild:  Low suppression level.
     */
    kBGHVS_Mild,
    /**
     * kModerate: (Default)  Moderate suppression level.
     */
    kBGHVS_Moderate,
    /**
     * kAggressive: High suppression level.
     */
    kBGHVS_Aggressive
  };

  /**
   * The ANS configuration.
   */
  struct AnsConfig {
    /**
     * Whether to enable ANS.
     * - `true`: Enable ANS.
     * - `false`: (Default) Disable ANS.
     */
    AgoraUAP::optional<bool> enabled;
    /**
     * The ANS noise suppression mode. See #AnsSuppressionMode.
     */
    AgoraUAP::optional<AnsSuppressionMode> suppressionMode;
    /**
     * The ANS Model Type. See #AnsModelType.
     */
    AgoraUAP::optional<AnsModelType> ansModelType;
    /**
     * The ANS speech protect threshold, which is used when
     * possibly speech damage happened by ANS. The value range
     * is [0, 100], the default value is 100, and we recommend
     * 50 when you intended to protect speech.
     */
    AgoraUAP::optional<int> speechProtectThreshold;
  };

  /**
   * The AGC configuration.
   */
  struct AgcConfig {
    /**
     * Whether to enable AGC.
     * - `true`: Enable AGC.
     * - `false`: (Default) Disable AGC.
     */
    AgoraUAP::optional<bool> enabled;
    /**
     * Whether to use analog AGC working mode.
     * - `true`: Use anglog mode. Analog AGC is only available for
     * platforms with an analog volume control on the capture device.
     * The SetStreamAnalogLevel and StreamAnalogLevel methods provide
     * coupling between OS analog volume control and AGC.
     * - `false`: (Default) Use digital mode.
     */
    AgoraUAP::optional<bool> useAnalogMode;
    /**
     * Maximum digital AGC gain in dB.
     * Recommended values should be within [6,30],
     * and default value is 12.
     **/
    AgoraUAP::optional<int> maxDigitalGaindB;
    /**
     * target digital AGC level in dB.
     * Recommended values should be within [18,0],
     * and default value is 6.
     **/
    AgoraUAP::optional<int> targetleveldB;
    /**
     * digital AGC slope
     * Recommended values should be within [3,30],
     * and default value is 17.
     **/
    AgoraUAP::optional<int> curve_slope;    
  };

  /**
   * Agora uplink audio processing state.
   */
  struct BGHVSCfg{
    /**
     * Whether to enable sessCtrl and BGHVS.
     * - `true`: Enable BGHVS.
     * - `false`: (Default) Disable BGHVS.
     */
    AgoraUAP::optional<bool> enabled; 

    AgoraUAP::optional<int> bghvsSOSLenInMs;     //how long to trigger SOS,in ms;

    AgoraUAP::optional<int> bghvsEOSLenInMs;     //how long to trigger EOS,in ms;

    AgoraUAP::optional<BghvsSuppressionMode> bghvsSppMode;       //bghvs aggressive level;

    AgoraUAP::optional<int> bghvsDelayInFrmNums; //bghvs algorithm delay,frm number,10ms per frame;

    //AgoraUAP::optional<bool> bghvsDump;         // whether open dump

    //AgoraUAP::optional<char*> bghvsDumpPath;    // bghvs dump path
  };

  struct State {
    /**
     * algorithmLatency refers to the relative delay between the
     * input data and the output data, measured in milliseconds,
     * when the input data is processed by the SDK and the
     * output data is obtained
     */
    AgoraUAP::optional<unsigned int> algorithmLatency;
    /**
     * algorithmLatency_sp refers to the relative delay between the
     * input data and the output data, measured in sample_points,
     * when the input data is processed by the SDK and the
     * output data is obtained
     */
    AgoraUAP::optional<unsigned int> algorithmLatency_sp;
    /**
     * aecEstimatedDelay refers to the relative delay between
     * the echo reference signal and the microphone-received signal.
     * measured in milliseconds.
     */
    AgoraUAP::optional<unsigned int> aecEstimatedDelay;
  };

 public:
  /**
   * Init the AgoraAudioProcessing object.
   *
   * @note Agora recommends calling this method after creating
   *  AgoraAudioProcessing object.
   *
   * @param config The uap config. See #UapConfig.
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int Init(const UapConfig& config) = 0;

  /**
   * Release the AgoraAudioProcessing object.
   */
  virtual void Release() = 0;

  /**
   * Set log output option.
   *
   * @note Agora recommends calling this method to enable
   * log output before creating AgoraAudioProcessing object.
   *
   * a. If the external log output function LogOutputFunc is set, the external function will be
   * called for output, and no local log file will be written. b. If you need to write logs to a
   * separate log file, set LogOutputFunc to nullptr. c. The log file name is agora3Asdk.log. When
   * the path is empty by default, it will be placed in the internal directory of the installation
   * package. For example, on Android, it will be located at
   * /storage/emulated/0/Android/data/app_package_name/logs/agora3Asdk.log. If the user specifies a
   * directory, the output will go to the specified path. If the directory does not exist, the SDK
   * will attempt to create it, but it is important to check for permissions. The return value can
   * be used to determine if the call was successful. d. The default LogLevel is LS_INFO. Setting it
   * to LS_NONE will suppress all output.
   *
   * @param option The log output option. See #LogOption.
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  AGORA_UAP_API static int WINAPI EnableLogOutput(const LogOption option);

  /**
   * Set PCM dump option.
   *
   * @note If dump is needed, Agora recommends calling this
   * method to enable PCM dump before starting process capture
   * and reverse stream.
   *
   * @param option The PCM dump option. See #DumpOption.
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int EnableDataDump(const DumpOption option) = 0;

  /**
   * Set AEC configuration.
   *
   * @param config The AEC configuration. See #AecConfig.
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int SetAecConfiguration(const AecConfig config) = 0;

  /**
   * Set ANS configuration.
   *
   * @param config The ANS configuration. See #AnsConfig.
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int SetAnsConfiguration(const AnsConfig config) = 0;

  /**
   * Set AGC configuration.
   *
   * @param config The AGC configuration. See #AgcConfig.
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int SetAgcConfiguration(const AgcConfig config) = 0;
  
  /**
   * Set BGHVS configuration.
   *
   * @param config The AGC configuration. See #AgcConfig.
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int SetBGHVSConfiguration(const BGHVSCfg config) = 0;
  /**
   *  Set BGHVS data dump,with open dump and set dump path;
  */
  virtual int EnableBGHVSDataDump(const DumpOption option) = 0;

  /**
   * Set stream delay in ms.
   *
   * @note Must be called if and only if AEC is enabled, in way of
   * before ProcessStream() in each processing cycle.
   *
   * @param delay Approximate time in ms between ProcessReverseStream()
   * receiving a far-end frame and ProcessStream() receiving the near-end
   * frame containing the corresponding echo. The value range is [60, 500].
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int SetStreamDelayMs(int delay) = 0;

  /**
   * Set stream analog level.
   *
   * @note Must be called if and only if analog agc is enabled, in way of
   * before ProcessStream() in each processing cycle.
   *
   * @param level The current analog level from the audio HAL. The value
   * range is [0, 255].
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int SetStreamAnalogLevel(int level) = 0;

  /**
   * Set AI Model Resource.
   *
   * @note Must be called if and only if AIAEC/AINS is on.
   *
   * @param config  The AI Model resource config. See #AiModelResourceConfig.
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int SetAIModelResource(const AiModelResourceConfig config) = 0;

  /**
   * Get new stream analog level suggested by AGC.
   *
   * @note Should be called if and only if analog agc is enabled, in way of
   * after ProcessStream() in each processing cycle.
   *
   * @param level The new analog level suggested by AGC. Should be set to
   * audio HAL if not equal to the level set in SetStreamAnalogLevel. The
   * value range is [0, 255].
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int GetStreamAnalogLevel(int& level) = 0;

  /**
   * Set signal gain.
   *
   * @param gain The gain applied to audio frame. The value range
   * is [0, 400].
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int SetGain(int gain) = 0;

  /**
   * Set private parameter.
   *
   * @param key The parameter string to be set.
   * @param value The value of the corresponding parameter string.
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int SetParameter(const char* key, int value) = 0;

  /**
   * Processes a 10 ms |frame| of the near-end (or captured) audio.
   *
   * @param frame The near-end frame to be processed. See #AgoraUAP::AudioFrame.
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int ProcessStream(AgoraUAP::AgoraAudioFrame* frame) = 0;

  /**
   * Processes a 10 ms |frame| of the far-end (or to be rendered) audio.
   *
   * @param frame The far-end frame to be processed. See #AgoraUAP::AudioFrame.
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int ProcessReverseStream(AgoraUAP::AgoraAudioFrame* frame) = 0;

  /**
   * Get Audio Processing states
   *
   * @param state See #AgoraAudioProcessing::State.
   * @param insamplerate to cal algorithmLatency_sp,support 8000,16000,24000,32000,44100,48000
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int GetState(AgoraAudioProcessing::State& state,int insamplerate) = 0;
  /**
   * Reset internal states and restart audio processing modules.
   *
   * @note Should be called if and only if critical runtime errors reported.
   * Agora uplink audio processing library will try best to recover from
   * critical runtime errors with no other method calling needed.
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int Reset() = 0;

 protected:
  virtual ~AgoraAudioProcessing(){};
};

/**
 * The AgoraAudioProcessingEventHandler class.
 *
 * Agora uses this class to send callback event notifications to the app, and
 * the app inherits methods in this class to retrieve these event notifications.
 *
 * To ensure thread safety, the app must not call AgoraAudioProcessing APIs
 * directly in the callback thread. Dispatch time-consuming tasks to other
 * threads asynchronously instead.
 */
class AgoraAudioProcessingEventHandler {
 public:
  /**
   * The event code of Agora uplink audio processing library.
   *
   * @note Agora reports critical runtime events or errors happened during
   * audio processing. The app should handle these events properly.
   */
  enum AgoraAudioProcessingEventType {
    /**
     * kAecMalfunction: The AEC breaks down.
     *
     * @note The app should call AgoraAudioProcessing::Reset asynchronously
     * to handle this event. See #AgoraAudioProcessing::Reset.
     */
    kAecMalfunction = 0
  };

  virtual ~AgoraAudioProcessingEventHandler() {}

  /**
   * The event callback of the Agora uplink audio processing library.
   *
   * @param event The event code of Agora uplink audio processing library.
   * See #AgoraAudioProcessingEventType.
   */
  virtual void onEvent(AgoraAudioProcessingEventType event) = 0;

  /**
   * The error callback of the Agora uplink audio processing library.
   *
   * @param error The error code of Agora uplink audio processing library.
   * See #ErrorCode.
   */
  virtual void onError(int error) = 0;
};

}  // namespace AgoraUAP

/**
 * Gets the SDK version.
 * @return The version of the current SDK in the string format.
 */
extern "C" AGORA_UAP_API const char* GetSdkVersion();

/**
 * Create an AgoraAudioProcessing object.
 */
extern "C" AGORA_UAP_API AgoraUAP::AgoraAudioProcessing* CreateAgoraAudioProcessing();

/**
 * Get Agora device UUID.
 *
 * @note Used to get license from agora license server.
 *
 * @param appId Input appid, must be the same as set in Init().
 * @param uuid Output uuid.
 * @param uuidBufLen Output uuid buffer length.
 *
 * @return
 * - 0: success.
 * - < 0: Failure.
 */
extern "C" AGORA_UAP_API int GetAgoraDeviceUUID(const char* appId, char* uuid, int uuidBufLen);

/**
 * Set android dir to store data.
 *
 * @note only work for android and should be called before GetAgoraDeviceUUID and
 * AgoraAudioProcessing::Init.
 *
 * @param dir normally the dir should be Context.getCacheDir().getAbsolutePath().
 *
 * @return
 *  * - 0: success.
 *  - < 0: Failure.
 */
extern "C" AGORA_UAP_API int SetAgoraAndroidDataDir(const char* dir);
