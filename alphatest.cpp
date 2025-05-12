#include "agora_audio_processing.h"
#include "agora_uap_base.h"
#include "time.h"
#include <cstdio>
#include <string.h>
#include <iostream>
#include <chrono>
#include <map>
#include <string>

#define SDK_REF_SIG_CHS 1
#define SDK_UPLINK_SAMPLE_FREQ 48000
#define SDK_UPLINK_FRM_SIZE (SDK_UPLINK_SAMPLE_FREQ / 100)
#define SDK_DOWNLINK_FREQ 48000
//#define SDK_DOWNLINK_FREQ 16000
#define SDK_DOWNLINK_FRM_SIZE (SDK_DOWNLINK_FREQ / 100 * SDK_REF_SIG_CHS)

/*
usage:
export LD_LIBRARY_PATH=./
./test.out --nearin <nearIn.wav> --out <output.wav> [--aec <0/1>] [--ans <0/1>] [--agc <0/1>] [--bghvs <0/1>] [--farin <path>]
./test.out --nearin nearin_power.wav --out alpha_out.wav --aec 0 --ans 1 --agc 0 --bghvs 1 --farin farin_power.wav 

*/

unsigned long long getLocalTimeUs()  // us
{
    typedef std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds> microClock_type;
    microClock_type tp = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now());
    return tp.time_since_epoch().count();
}


class apHandler : public AgoraUAP::AgoraAudioProcessingEventHandler {
  void onEvent(AgoraAudioProcessingEventType event) override {
    printf("UapApiTest::onEvent :%d\n", event);
  }

  void onError(int err) override {
    printf("UapApiTest::onError :%d\n", err);
    if (err == AgoraUAP::ErrorCode::kNoServerResponseError) {
      // server_no_resp_ev_->set();
    }
  }
};

static int16_t *uplink_buffer_ = nullptr;
static AgoraUAP::AgoraAudioFrame uplink_frame_;
static int16_t *downlink_buffer_ = nullptr;
static AgoraUAP::AgoraAudioFrame downlink_frame_;
static FILE *uplink_file_ = nullptr;
static FILE *downlink_file_ = nullptr;
static FILE *out_file_ = nullptr;


typedef struct wavfile_header_ {
  char	riff_tag[4];
  int	riff_length;
  char	wave_tag[4];
  char	fmt_tag[4];
  int	fmt_length;
  short	audio_format;
  short	num_channels;
  int	sample_rate;
  int	byte_rate;
  short	block_align;
  short	bits_per_sample;
  char	data_tag[4];
  int	data_length;
} wavfile_header;

    
const static char *APPID = "dfadde3bb5264c32b7829e8be51e9aef";
// const static char *LICENSE ="eyJzaWduIjoibFRxSU00T1JnNklnVTRSTzIwUEppMkpMbjVKUlZhcHNiZGQ0R0h4RzdsZ2p6Y2w1TUJYUXRXTzNKRGFSdTNyaGdQYnpKSHVQa2h3RmFacjFLNWpuKzBtUjBoNVdTbDJMQWt3RFVITlkwUWlzQS9mcElXUnZTclVja3UwajlBR1JIc0s2QU5zNFM2dlpQQmRVT09ZYUJFRW03eU8rS29GeExDbGYwelVTNkR6N0tYSkdvbVBkdU4wV3RkYXhqdUplT1JJcHNKWFZhbmM3b0ZjMmhIdDQ3K0tmTGttNjBSYmJCTEJNdWhLb3dEYXB4aUxHTjVyRUdZQk84aHdoVFl2R2ZpSXo3U0U3U2xqaVdvTkFwK0YwMExTY1JKMFVHRjB3MmMwaXBJbGJ4NzRpdWZteWE4SmtzQUtUQWd4dFc5c0huM2ZuT2xleHE1cUhhTEhVZklOQThRPT0iLCJjdXN0b20iOiI0OUE2MjlGNjU1Q0E0OTc1Qjg1RUJBRjYxMjY0OUJGRiIsImNyZWRlbnRpYWwiOiI0YTVmYTJlZWRkZDJiMmM5YzdjNDE5OWFhZDQ4MjA1YjliMGM5YmIxOWVhZWZkZWE0OTk2YWE5OWY2ZGQ5MjUyIiwiZHVlIjoiMjAyNDEwMjYifQ==";
const static char *LICENSE ="eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE3NTI5MTU3OTksImlhdCI6MTcyMTM3OTc5OSwibGljZW5zZUtleSI6IjIwMjQwNzE5IiwicGlkIjoiOEU5NTk3MUJFMTc4NEJFQjlEQkJFQkVDNEFDQzVCNUQifQ.FK1fcqMwxvTibsjKcTxGuBq0DLqazl8yG8d0n4sFnpZqZjsFdFUDo-mY2CDrqi7QbOqvqx1DCllkN652oDsIeupJ-VefHiEPIGn0dg9DL7cRSNF6WEzSUYyJd2J2_4kg_DlNfV0Ml5zA5eAg2AhKKpGEWz0Jn57Wb-3OYSb0kwnj_ZjZB3Z7HfMIOseVu77bQroBCxdyjj4dzlm1U4GE8bdWGNbouMFJG4GEpYuIAoGDpokNw2t8aN9QKAZfUFymIFbHOMpHiI7x8Z5ONmsco7dy5lgfYkI5VPhMDQ8fxs_W0JGGPgVPo0EyAnZyWlzvd5gEYdY-TCh4JmC811bXsw";

static void logout(const char *log) { printf("%s\n", log); }

int audio_process_config(AgoraUAP::AgoraAudioProcessing *ap, bool aec_enable, bool ns_enable, bool agc_enable, bool bghvs_enable) {
  if (!ap) {
    return -1;
  }
  const char* sourceFile = NULL;
  FILE* binFilePtr = NULL;
  size_t modelDataSize = 0;
  char* modelName = NULL;
  AgoraUAP::AgoraAudioProcessing::AiModelResourceConfig ainsConfig;
  AgoraUAP::AgoraAudioProcessing::AiModelResourceConfig ainsllConfig;  
  std::shared_ptr<void> ainsModelDataPtr = NULL;
  std::shared_ptr<void> ainsllModelDataPtr = NULL;  
  AgoraUAP::AgoraAudioProcessing::AiModelResourceConfig ainlpConfig;
  AgoraUAP::AgoraAudioProcessing::AiModelResourceConfig ainlpllConfig;  
  std::shared_ptr<void> ainlpModeDataPtr = NULL;
  std::shared_ptr<void> ainlpllModeDataPtr = NULL;  
  // std::shared_ptr<char> ainsModelDataPtr(NULL, std::default_delete<char[]>());
  modelName = (char*)"ains";
  // sourceFile = "/home/sxb/loganxu/3a_test_demo/3atest/CLDNNWeights.bin";
  sourceFile = "./CLDNNWeights.bin";
  binFilePtr = fopen(sourceFile, "rb");
  if (binFilePtr != NULL) {
      printf("open bin file ok!\n");
      fseek(binFilePtr, 0, SEEK_END);
      modelDataSize = ftell(binFilePtr);
      fseek(binFilePtr, 0, SEEK_SET);
      // ainsModelDataPtr.reset(new char[modelDataSize]);
      ainsModelDataPtr.reset(new char[modelDataSize], std::default_delete<char[]>()); 
      fread(ainsModelDataPtr.get(), 1, modelDataSize, binFilePtr);
      fclose(binFilePtr);
      ainsConfig.modelDataPtr = ainsModelDataPtr;
      ainsConfig.modelDataSize = modelDataSize;
      ainsConfig.modelName = modelName;
      ap->SetAIModelResource(ainsConfig);
  } else {
      printf("open bin file error!\n");
  }


  modelName = (char*)"ains_ll";
  // sourceFile = "/home/sxb/loganxu/3a_test_demo/3atest/CLDNNWeights.bin";
  sourceFile = "./CLDNNLLWeights.bin";
  binFilePtr = fopen(sourceFile, "rb");
  if (binFilePtr != NULL) {
      printf("open bin file ok!\n");
      fseek(binFilePtr, 0, SEEK_END);
      modelDataSize = ftell(binFilePtr);
      fseek(binFilePtr, 0, SEEK_SET);
      ainsllModelDataPtr.reset(new char[modelDataSize], std::default_delete<char[]>());
      fread(ainsllModelDataPtr.get(), 1, modelDataSize, binFilePtr);
      fclose(binFilePtr);
      ainsllConfig.modelDataPtr = ainsllModelDataPtr;
      ainsllConfig.modelDataSize = modelDataSize;
      ainsllConfig.modelName = modelName;
      ap->SetAIModelResource(ainsllConfig);
  } else {
      printf("open bin file error!\n");
  }

  modelName = (char*)"ainlp";
  sourceFile = "./YNetWeights.bin";
  binFilePtr = fopen(sourceFile, "rb");
  if (binFilePtr != NULL) {
      printf("open bin file ok!\n");
      fseek(binFilePtr, 0, SEEK_END);
      modelDataSize = ftell(binFilePtr);
      fseek(binFilePtr, 0, SEEK_SET);
      ainlpModeDataPtr.reset(new char[modelDataSize], std::default_delete<char[]>());
      fread(ainlpModeDataPtr.get(), 1, modelDataSize, binFilePtr);
      fclose(binFilePtr);
      ainlpConfig.modelDataPtr = ainlpModeDataPtr;
      ainlpConfig.modelDataSize = modelDataSize;
      ainlpConfig.modelName = modelName;
      ap->SetAIModelResource(ainlpConfig);
  } else {
      printf("open bin file error!\n");
  }

  modelName = (char*)"ainlp_ll";
  sourceFile = "./YNetLLWeights.bin";
  binFilePtr = fopen(sourceFile, "rb");
  if (binFilePtr != NULL) {
      printf("open bin file ok!\n");
      fseek(binFilePtr, 0, SEEK_END);
      modelDataSize = ftell(binFilePtr);
      fseek(binFilePtr, 0, SEEK_SET);
      ainlpllModeDataPtr.reset(new char[modelDataSize], std::default_delete<char[]>());
      fread(ainlpllModeDataPtr.get(), 1, modelDataSize, binFilePtr);
      fclose(binFilePtr);
      ainlpllConfig.modelDataPtr = ainlpllModeDataPtr;
      ainlpllConfig.modelDataSize = modelDataSize;
      ainlpllConfig.modelName = modelName;
      ap->SetAIModelResource(ainlpllConfig);
  } else {
      printf("open bin file error!\n");
  }  

//set aec parameter
  AgoraUAP::AgoraAudioProcessing::AecConfig aecConfig;
  aecConfig.enabled = aec_enable;
  aecConfig.filterLength = AgoraUAP::AgoraAudioProcessing::AecFilterLength::kNormal;
  aecConfig.aecModelType = AgoraUAP::AgoraAudioProcessing::AecModelType::kLLAIAEC;
  aecConfig.aiaecSuppressionMode = AgoraUAP::AgoraAudioProcessing::AIAECSuppressionMode::kChatMode;
  aecConfig.aecSuppressionMode = AgoraUAP::AgoraAudioProcessing::AECSuppressionMode::kAggressiveAEC;
  // aecConfig.aecStartupMaxSuppressTimeInMs = 0;
  int ret = ap->SetAecConfiguration(aecConfig);
  printf("[APM_TEST]aecconfig:aec enable = %d\n",static_cast<int>(aecConfig.enabled.value()));
  printf("[APM_TEST]aecconfig:stereoAecEnabled = %d\n",static_cast<int>(aecConfig.stereoAecEnabled.value()));
  printf("[APM_TEST]aecconfig:enableAecAutoReset = %d\n",static_cast<int>(aecConfig.enableAecAutoReset.value()));    
  printf("[APM_TEST]aecconfig:aecStartupMaxSuppressTimeInMs = %d\n",aecConfig.aecStartupMaxSuppressTimeInMs.value());   
  printf("[APM_TEST]aecconfig:filterLength = %d\n",static_cast<int>(aecConfig.filterLength.value())); 
  printf("[APM_TEST]aecconfig:aecModelType = %d\n",static_cast<int>(aecConfig.aecModelType.value())); 
  printf("[APM_TEST]aecconfig:aecSuppressionMode = %d\n",static_cast<int>(aecConfig.aecSuppressionMode.value()));   
  printf("[APM_TEST]aecconfig:aiaecSuppressionMode = %d\n",static_cast<int>(aecConfig.aiaecSuppressionMode.value()));         

//set ans parameter
  AgoraUAP::AgoraAudioProcessing::AnsConfig ansConfig;
  ansConfig.enabled = ns_enable;
  ansConfig.ansModelType = AgoraUAP::AgoraAudioProcessing::AnsModelType::kSTDAIANS;  
  ansConfig.suppressionMode = AgoraUAP::AgoraAudioProcessing::AnsSuppressionMode::kModerate;
  ansConfig.speechProtectThreshold = 100;
  ret = ap->SetAnsConfiguration(ansConfig);
  printf("[APM_TEST]ansconfig:ans enable = %d\n",static_cast<int>(ansConfig.enabled.value()));
  printf("[APM_TEST]ansconfig:ansModelType = %d\n",static_cast<int>(ansConfig.ansModelType.value()));  
  printf("[APM_TEST]ansconfig:suppressionMode = %d\n",static_cast<int>(ansConfig.suppressionMode.value()));
  printf("[APM_TEST]ansconfig:aec enable = %d\n",ansConfig.speechProtectThreshold.value());

 //set agc parameter 
  AgoraUAP::AgoraAudioProcessing::AgcConfig agcConfig;
  agcConfig.enabled = agc_enable;
  agcConfig.maxDigitalGaindB = 12;
  agcConfig.targetleveldB = 6;
  agcConfig.curve_slope = 17;
  ret = ap->SetAgcConfiguration(agcConfig);
  printf("[APM_TEST]agcconfig:agc enable = %d\n",static_cast<int>(agcConfig.enabled.value()));
  printf("[APM_TEST]agcconfig:maxDigitalGaindB = %d\n",agcConfig.maxDigitalGaindB.value());  
  printf("[APM_TEST]agcconfig:targetleveldB = %d\n",agcConfig.targetleveldB.value());  
  printf("[APM_TEST]agcconfig:curve_slope = %d\n",agcConfig.curve_slope.value());      


//set session contorl
AgoraUAP::AgoraAudioProcessing::BGHVSCfg bgConfig;
bgConfig.enabled = bghvs_enable;
bgConfig.bghvsSOSLenInMs = 160;
bgConfig.bghvsEOSLenInMs = 500;
bgConfig.bghvsDelayInFrmNums = 12;
bgConfig.bghvsSppMode = AgoraUAP::AgoraAudioProcessing::BghvsSuppressionMode::kBGHVS_Moderate;
ret =ap->SetBGHVSConfiguration(bgConfig);

printf("[APM_TEST]bgConfig:bgConfig.enabled = %d\n",static_cast<int>(bgConfig.enabled.value()));
printf("[APM_TEST]bgConfig:bgConfig.bghvsSOSLenInMs = %d\n",bgConfig.bghvsSOSLenInMs.value());  
printf("[APM_TEST]bgConfig:bgConfig.bghvsEOSLenInMs = %d\n",bgConfig.bghvsEOSLenInMs.value());  
printf("[APM_TEST]bgConfig:bgConfig.bghvsDelayInFrmNums = %d\n",bgConfig.bghvsDelayInFrmNums.value()); 
printf("[APM_TEST]bgConfig:bgConfig.bghvsSppMode = %d\n",static_cast<int>(bgConfig.bghvsSppMode.value()));   


  AgoraUAP::AgoraAudioProcessing::DumpOption dumpOption(true, "./dump/"); //directory
  ret = ap->EnableDataDump(dumpOption);
  return ret;
}

int audio_process(AgoraUAP::AgoraAudioProcessing *ap,
 AgoraUAP::AgoraAudioFrame &uplink_frame, AgoraUAP::AgoraAudioFrame &downlink_frame) {
  int ret = 0;
  ret = ap->SetStreamDelayMs(60);
  ret = ap->SetStreamAnalogLevel(0);
  
  

  ap->ProcessReverseStream(&downlink_frame);

  ap->ProcessStream(&uplink_frame);
  // ret = ap->GetStreamAnalogLevel(level);
  return 0;
}

// Command line argument parser
std::map<std::string, std::string> parseCommandLineArgs(int argc, char* argv[]) {
    std::map<std::string, std::string> args;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.substr(0, 2) == "--") {
            std::string key = arg.substr(2);
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                args[key] = argv[i + 1];
                i++; // Skip the value in next iteration
            } else {
                args[key] = "true"; // Flag without value
            }
        }
    }
    return args;
}

// to execute: ./3atest_config nearIn.wav farIn.wav output.wav
// change to ./3atest_config nearIn.wav  output.wav {aec:0/1} {ans: 0/1} {agc: 0/1} {bghvs: 0/1}  {farin.wav: null/path}
const char *usage = "Usage: ./3atest_config nearIn.wav  output.wav {aec:0/1} {ans: 0/1} {agc: 0/1} {bghvs: 0/1}  {farin.wav: null/path}\n and aec,ans,agc,bghvs are optional,but should have at least one to be 1, default is 0\n";
int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Usage: %s --nearin <nearIn.wav> --out <output.wav> [--aec <0/1>] [--ans <0/1>] [--agc <0/1>] [--bghvs <0/1>] [--farin <path>]\n", argv[0]);
    return 0;
  }

  // Parse command line arguments
  auto args = parseCommandLineArgs(argc, argv);
  
  // Get required arguments
  if (args.find("out") == args.end()) {
    printf("Error: --out parameter is required\n");
    return -1;
  }

  char* nearInFile = const_cast<char*>(args["nearin"].c_str());
  
  char* outputFile = const_cast<char*>(args["out"].c_str());

  // auto -generate pcm outfile
  std::string strPcmOutFile = outputFile;
  strPcmOutFile += ".pcm";
  const char* pcmOutFile = strPcmOutFile.c_str();
  
  // Get optional arguments with defaults
  bool aec_enable = args.find("aec") != args.end() ? std::stoi(args["aec"]) : false;
  bool ns_enable = args.find("ans") != args.end() ? std::stoi(args["ans"]) : false;
  bool agc_enable = args.find("agc") != args.end() ? std::stoi(args["agc"]) : false;
  bool bghvs_enable = args.find("bghvs") != args.end() ? std::stoi(args["bghvs"]) : false;
  
  // Get farin file if provided
  char* farInFile = args.find("farin") != args.end() ? const_cast<char*>(args["farin"].c_str()) : nullptr;
  
  printf("Config Parameters are:\n");
  printf("  Near input file: %s\n", nearInFile);  
  printf("  Output file: %s\n", outputFile);
  printf("  AEC enabled: %d\n", aec_enable);
  printf("  ANS enabled: %d\n", ns_enable);
  printf("  AGC enabled: %d\n", agc_enable);
  printf("  BGHVS enabled: %d\n", bghvs_enable);
  if (farInFile) {
    printf("  Far input file: %s\n", farInFile);
  }

  

  
  int tmp = 0;
  int uplink_sample_rate = 16000;
  int downlink_sample_rate = 16000;
  int uplink_channels = 1;
  int downlink_channels = 1;
  int uplink_frame_size = 160;  //default 10ms and 16 bits
  int downlink_frame_size = 160;


  wavfile_header wavHeader;

   // read wav file to get wavformat and fill downlink_frame_ and uplink_frame_

  uplink_file_ = fopen(nearInFile, "rb");  // TODO: modify as needed
  if (!uplink_file_) {
    printf("%s(%d): uplink_file_ nullptr!\n", __FUNCTION__, __LINE__);
    return -1;
  }

  //read wav header
  fseek(uplink_file_, 0, SEEK_SET);
  fread(&wavHeader, sizeof(wavfile_header), 1, uplink_file_);
  int bytesRead = wavHeader.data_length;
  uplink_sample_rate = wavHeader.sample_rate;
  uplink_channels = wavHeader.num_channels;
  uplink_frame_size = uplink_sample_rate/100;
  //modify bytesRead to file size
  fseek(uplink_file_, 0, SEEK_END);
  bytesRead = ftell(uplink_file_) - sizeof(wavfile_header);
  fseek(uplink_file_, sizeof(wavfile_header), SEEK_SET);

  // allocate buffer for uplink_buffer_ and downlink_buffer_
  uplink_buffer_ = new int16_t[uplink_frame_size];  // 10ms one frame in int16,i.e 16bits pcm
  uplink_frame_.buffer = uplink_buffer_;
  uplink_frame_.sampleRate = uplink_sample_rate;  // TODO: modify as needed
  uplink_frame_.channels = uplink_channels;
  uplink_frame_.samplesPerChannel = uplink_frame_size;
  

  // if enable aec and farin file is provided, read farin file; else set to nullptr
  if (aec_enable && farInFile) {
    downlink_file_ = fopen(farInFile, "rb");
    if (!downlink_file_) {
      printf("%s(%d): downlink_file_ nullptr!, aec_enable = %d, farInFile = %s\n", __FUNCTION__, __LINE__, aec_enable, farInFile?farInFile:"nullptr");
      return -1;
    }
    // set aec to ture
    aec_enable = true;
  } else {
    downlink_file_ = nullptr;
    aec_enable = false;
  }

  // force to null ptr for downlink_file_
  downlink_frame_.buffer = nullptr;

  if (downlink_file_) {
    fread(&wavHeader, sizeof(wavfile_header), 1, downlink_file_);
    // bytesRead is not used, set to farin file, ie downlink_file_ should be same length as nearin_file_
    //bytesRead = wavHeader.data_length; 
    downlink_sample_rate = wavHeader.sample_rate;
    downlink_channels = wavHeader.num_channels;
    printf("-- from farin file: downlink_sample_rate: %d, downlink_channels: %d\n", downlink_sample_rate, downlink_channels);
  
  }
  else { // set same to uplink_sample_rate and uplink_channels
    downlink_sample_rate = uplink_sample_rate;
    downlink_channels = uplink_channels;
  }
  downlink_frame_size = downlink_sample_rate/100;
 
  // allocate buffer for downlink_buffer_
  downlink_buffer_ = new int16_t[downlink_frame_size];  // 10ms one frame in int16,i.e 16bits pcm
  downlink_frame_.buffer = downlink_buffer_;
  downlink_frame_.sampleRate = downlink_sample_rate;  // TODO: modify as needed
  downlink_frame_.channels = downlink_channels;
  downlink_frame_.samplesPerChannel = downlink_frame_size;

  if (!aec_enable) {
    // set downlink_frame_ to dummy frame
    memset(downlink_frame_.buffer, 0, sizeof(int16_t) * downlink_frame_.samplesPerChannel);
  }

  printf("uplink_sample_rate: %d, downlink_sample_rate: %d\n", uplink_sample_rate, downlink_sample_rate);
  printf("uplink_channels: %d, downlink_channels: %d\n", uplink_channels, downlink_channels); 
  printf("uplink_frame_size: %d, downlink_frame_size: %d\n", uplink_frame_size, downlink_frame_size);
  printf("bytesRead: %d\n", bytesRead);
  printf("After Config Parameters are:\n");
  printf("  Near input file: %s\n", nearInFile);  
  printf("  Output file: %s\n", outputFile);
  printf("  AEC enabled: %d\n", aec_enable);
  printf("  ANS enabled: %d\n", ns_enable);
  printf("  AGC enabled: %d\n", agc_enable);
  printf("  BGHVS enabled: %d\n", bghvs_enable);
  if (farInFile) {
    printf("  Far input file: %s\n", farInFile);
  }
  getchar();

  const char* version = GetSdkVersion();
  printf("3ASDK Version: %s\n", version);

  AgoraUAP::AgoraAudioProcessing::LogOption option(true, true, logout);
  AgoraUAP::AgoraAudioProcessing::EnableLogOutput(option);
  apHandler handle;
  char uuid[AGORA_MAX_UUID_LENGTH];
  GetAgoraDeviceUUID(APPID, uuid, AGORA_MAX_UUID_LENGTH);
  printf("TestNormal uuid :%s\n", uuid);
  auto ap = CreateAgoraAudioProcessing();
  printf("the ap is %p \n", ap);

  
  
  

  //  ap->Init(AgoraUAP::AgoraAudioProcessing::UapConfig(APPID, "", &handle));
  int ret = ap->Init(
      AgoraUAP::AgoraAudioProcessing::UapConfig(APPID, LICENSE, &handle));

  // if (ap) {
  //   ap->Release();
  //   ap = nullptr;
  // }

  // printf("the ret is %d \n", ret);

  // return 0;

  // printf("the ret is %d \n", ret);

  // uplink_buffer_ = new int16_t[160];
  // uplink_frame_.buffer = uplink_buffer_;
  // uplink_frame_.sampleRate = 16000;  // TODO: modify as needed
  // uplink_frame_.channels = 1;
  // uplink_frame_.samplesPerChannel = 160;

  // downlink_buffer_ = new int16_t[160];
  // downlink_frame_.buffer = downlink_buffer_;
  // downlink_frame_.sampleRate = 16000;
  // downlink_frame_.channels = 1;
  // downlink_frame_.samplesPerChannel = 160;

 

  audio_process_config(ap, aec_enable, ns_enable, agc_enable, bghvs_enable); 

  

  getchar();
  //get 3a lib algorithm Latency
  AgoraUAP::AgoraAudioProcessing::State state_;
  ap->GetState(state_,uplink_sample_rate);
  int latency = state_.algorithmLatency.value();
  int frame_latency = latency/10;
  int sp_latency = state_.algorithmLatency_sp.value();
  int aec_delay = state_.aecEstimatedDelay.value();
  printf("[APM_TEST]:get 3a lib latency = %d ms, frame_latency = %d ,sp_latency = %d , aec_delay = %d\n",latency,frame_latency,sp_latency,aec_delay);

  int validfrmSize = bytesRead/sizeof(int16_t)/uplink_frame_size;
  int frmSize = validfrmSize + frame_latency;

  int16_t* dowlink_buff_tmp = (int16_t*)malloc(sizeof(int16_t) * frmSize * downlink_frame_size);
  memset(dowlink_buff_tmp, 0, sizeof(int16_t) * frmSize * downlink_frame_size);
  if (downlink_file_ && aec_enable) {
    fread(dowlink_buff_tmp, 1, bytesRead, downlink_file_);
  }

  int16_t* uplink_buff_tmp = (int16_t*)malloc(sizeof(int16_t) * frmSize * uplink_frame_size);
  memset(uplink_buff_tmp, 0, sizeof(int16_t) * frmSize * uplink_frame_size);
  if (uplink_file_) {
    fread(uplink_buff_tmp, 1, bytesRead, uplink_file_);
  }

  out_file_ = fopen(outputFile,"wb");
  //write wav header
  wavHeader.data_length = sizeof(int16_t) * frmSize * uplink_frame_size;
  fwrite(&wavHeader, sizeof(wavfile_header), 1, out_file_);

  FILE* pcm_out_file_ = fopen(pcmOutFile, "wb");

  // start process
  int hopSize = uplink_frame_size;

  int process_frame_avarge_time = 0;
  int process_time = 0;
  uint64_t begin_time = 0;
  uint64_t end_time = 0;
  int file_time = validfrmSize *10;


  for(int i = 0; i < frmSize; i++) {
    // if(i%500 == 0){
    //   ap->Reset();
    //   printf("[APM_TEST]:reset apm!!!\n");
    // }
    if(i < validfrmSize){
      memcpy(uplink_buffer_, uplink_buff_tmp + i * hopSize, sizeof(int16_t) * hopSize);
      memcpy(downlink_buffer_, dowlink_buff_tmp + i * hopSize, sizeof(int16_t) * hopSize);
    }else{
      memset(uplink_buffer_, 0, sizeof(int16_t) * hopSize);
      memset(downlink_buffer_, 0, sizeof(int16_t) * hopSize);
    }

    begin_time = getLocalTimeUs();
    audio_process(ap, uplink_frame_, downlink_frame_);
    end_time = getLocalTimeUs();
    process_time += end_time - begin_time;
   
    fwrite(uplink_buffer_, sizeof(int16_t), uplink_frame_size, out_file_);   
    if (pcm_out_file_) {
      fwrite(uplink_buffer_, sizeof(int16_t), uplink_frame_size, pcm_out_file_);
    }
    
  }
  getchar();
  process_frame_avarge_time = process_time/frmSize;
  process_time = process_time/1000;
  printf("[APM_TEST]:process_frame_avarge_time = %d us, process_total_time = %dms,file_time = %d ms\n",
    process_frame_avarge_time,process_time,file_time);
  ap->GetState(state_,downlink_sample_rate);
  latency = state_.algorithmLatency.value();
  frame_latency = (int)(latency/10);
  sp_latency = state_.algorithmLatency_sp.value();
  aec_delay = state_.aecEstimatedDelay.value();
  printf("[APM_TEST]:get 3a lib latency = %d ms, frame_latency = %d ,sp_latency = %d , aec_delay = %d\n",latency,frame_latency,sp_latency,aec_delay);
  // //clock_t start_time, end_time;
  // while((fread(downlink_buffer_, sizeof(int16_t), 160, downlink_file_) == 160 )&& (fread(uplink_buffer_, sizeof(int16_t), 160, uplink_file_) == 160) )
  // {
  //   //start_time = clock();
  //   audio_process(ap);
  //   //end_time = clock();
  //   //double times = (double)(end_time - start_time) / CLOCKS_PER_SEC;
  //   //printf("per frame time is %f seconds\n", times);
  //   fwrite(uplink_buffer_, sizeof(int16_t), 160, out_file_);
  // }

  if(uplink_buffer_) {
    delete[] uplink_buffer_;
    uplink_buffer_ = NULL;
  }

  if(uplink_buff_tmp) {
    free(uplink_buff_tmp);
    uplink_buff_tmp = NULL;
  }

  if(downlink_buffer_) {
    delete[] downlink_buffer_;
    downlink_buffer_ = NULL;
  }

  if(dowlink_buff_tmp) {
    free(dowlink_buff_tmp);
    dowlink_buff_tmp = NULL;
  }

  if(uplink_file_){
    fclose(uplink_file_);
    uplink_file_ = NULL;
  }

  if(downlink_file_){
    fclose(downlink_file_);
    downlink_file_ = NULL;
  }

  if (ap) {
    ap->Release();
    ap = nullptr;
  }

  printf("the ret is %d \n", ret);

  return 0;
}

