#include "3a.h"
#include <string>

#include "agora_audio_processing.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

class APHandler : public AgoraUAP::AgoraAudioProcessingEventHandler {
    public:
    void onEvent(AgoraAudioProcessingEventType event) override {
            
        if (event_handler_ != nullptr) {
            event_handler_->on_event(user_data_, (int)event);
        }
    }
    void onError(int err) override {
        printf("onError: %d\n", err);
        if (event_handler_ != nullptr) {
            event_handler_->on_error(user_data_, (int)err);
        }
    }
    public:
    APHandler(void* user_data, struct _agora_ap_processor_event_handler *event_handler) {
        printf("APHandler constructor\n");
        this->user_data_ = user_data;
        this->event_handler_ = event_handler;
    }
    ~APHandler() {
        printf("APHandler destructor\n");
        user_data_ = nullptr;
    }

    public:
    void* user_data_;
    struct _agora_ap_processor_event_handler *event_handler_;
};

typedef struct _agora_ap_processor_impl {
    AgoraUAP::AgoraAudioProcessing* processor;
    std::shared_ptr<APHandler> handler;
    bool aec_enabled;
    AgoraUAP::AgoraAudioFrame* aec_ref_frame;

    _agora_ap_processor_impl() {
        processor = nullptr;
        handler = nullptr;
        aec_enabled = false;
        aec_ref_frame = nullptr;
    }
} ;

typedef struct _agora_ap_service_impl {
    bool is_initialized;
    _agora_ap_service_config config;

    // global event handler
    struct _agora_ap_processor_event_handler *event_handler;

    //ai model resource
    AgoraUAP::AgoraAudioProcessing::AiModelResourceConfig ains_model_config;
    AgoraUAP::AgoraAudioProcessing::AiModelResourceConfig ainsll_model_config;  
    AgoraUAP::AgoraAudioProcessing::AiModelResourceConfig ainlp_model_config;
    AgoraUAP::AgoraAudioProcessing::AiModelResourceConfig ainlpll_model_config;  
} ;

static _agora_ap_service_impl  *g_ap_service_impl = nullptr;
AGORA_API_C_HDL agora_ap_service_create()
{
    //check if g_ap_service_impl is nullptr, if nullptr, create a new one
    if (g_ap_service_impl == nullptr) {
        g_ap_service_impl = new _agora_ap_service_impl();
        g_ap_service_impl->event_handler = nullptr;
    }
   return g_ap_service_impl;
}

AGORA_API_C_INT agora_ap_service_initialize(AGORA_API_C_HDL service_handle, const _agora_ap_service_config* config, _agora_ap_processor_event_handler *handler)
{
    if (!service_handle || service_handle != g_ap_service_impl || g_ap_service_impl == nullptr) {
        return -1;
    }
    if (g_ap_service_impl->is_initialized) {
        return 0;
    }
    g_ap_service_impl->config = *config;
    g_ap_service_impl->is_initialized = true;
    g_ap_service_impl->event_handler = handler;
    

    std::string model_path(config->resource_path);
    if (model_path.back() != '/') {
        model_path += '/';
    }
    


    // fill model resource config
    const char* sourceFile = NULL;
    FILE* binFilePtr = NULL;
    size_t modelDataSize = 0;
    char* modelName = NULL;
 
    std::shared_ptr<void> ainsModelDataPtr = NULL;
    std::shared_ptr<void> ainsllModelDataPtr = NULL;  
    std::shared_ptr<void> ainlpModeDataPtr = NULL;
    std::shared_ptr<void> ainlpllModeDataPtr = NULL;  
  
    modelName = (char*)"ains";
    // sourceFile = "/home/sxb/loganxu/3a_test_demo/3atest/CLDNNWeights.bin";
    std::string ains_model_path = model_path + "CLDNNWeights.bin";
    sourceFile = "./CLDNNWeights.bin";
    binFilePtr = fopen(ains_model_path.c_str(), "rb");
    if (binFilePtr != NULL) {
        printf("open bin file ok!\n");
        fseek(binFilePtr, 0, SEEK_END);
        modelDataSize = ftell(binFilePtr);
        fseek(binFilePtr, 0, SEEK_SET);
        // ainsModelDataPtr.reset(new char[modelDataSize]);
        ainsModelDataPtr.reset(new char[modelDataSize], std::default_delete<char[]>()); 
        fread(ainsModelDataPtr.get(), 1, modelDataSize, binFilePtr);
        fclose(binFilePtr);
        g_ap_service_impl->ains_model_config.modelDataPtr = ainsModelDataPtr;
        g_ap_service_impl->ains_model_config.modelDataSize = modelDataSize;
        g_ap_service_impl->ains_model_config.modelName = modelName;
        //ap->SetAIModelResource(g_ap_service_impl->ains_model_config);
    } else {
        printf("open bin file error!\n");
    }


    modelName = (char*)"ains_ll";
    // sourceFile = "/home/sxb/loganxu/3a_test_demo/3atest/CLDNNWeights.bin";
    std::string ainsll_model_path = model_path + "CLDNNLLWeights.bin";
    sourceFile = "./CLDNNLLWeights.bin";
    binFilePtr = fopen(ainsll_model_path.c_str(), "rb");
    if (binFilePtr != NULL) {
        printf("open bin file ok!\n");
        fseek(binFilePtr, 0, SEEK_END);
        modelDataSize = ftell(binFilePtr);
        fseek(binFilePtr, 0, SEEK_SET);
        ainsllModelDataPtr.reset(new char[modelDataSize], std::default_delete<char[]>());
        fread(ainsllModelDataPtr.get(), 1, modelDataSize, binFilePtr);
        fclose(binFilePtr);
        g_ap_service_impl->ainsll_model_config.modelDataPtr = ainsllModelDataPtr;
        g_ap_service_impl->ainsll_model_config.modelDataSize = modelDataSize;
        g_ap_service_impl->ainsll_model_config.modelName = modelName;
        //ap->SetAIModelResource(g_ap_service_impl->ainsll_model_config);
    } else {
        printf("open bin file error!\n");
    }

    modelName = (char*)"ainlp";
    std::string ainlp_model_path = model_path + "YNetWeights.bin";
    sourceFile = "./YNetWeights.bin";
    binFilePtr = fopen(ainlp_model_path.c_str(), "rb");
    if (binFilePtr != NULL) {
        printf("open bin file ok!\n");
        fseek(binFilePtr, 0, SEEK_END);
        modelDataSize = ftell(binFilePtr);
        fseek(binFilePtr, 0, SEEK_SET);
        ainlpModeDataPtr.reset(new char[modelDataSize], std::default_delete<char[]>());
        fread(ainlpModeDataPtr.get(), 1, modelDataSize, binFilePtr);
        fclose(binFilePtr);
        g_ap_service_impl->ainlp_model_config.modelDataPtr = ainlpModeDataPtr;
        g_ap_service_impl->ainlp_model_config.modelDataSize = modelDataSize;
        g_ap_service_impl->ainlp_model_config.modelName = modelName;
        //ap->SetAIModelResource(g_ap_service_impl->ainlp_model_config);
    } else {
        printf("open bin file error!\n");
    }

    modelName = (char*)"ainlp_ll";
    std::string ainlpll_model_path = model_path + "YNetLLWeights.bin";
    sourceFile = "./YNetLLWeights.bin";
    binFilePtr = fopen(ainlpll_model_path.c_str(), "rb");
    if (binFilePtr != NULL) {
        printf("open bin file ok!\n");
        fseek(binFilePtr, 0, SEEK_END);
        modelDataSize = ftell(binFilePtr);
        fseek(binFilePtr, 0, SEEK_SET);
        ainlpllModeDataPtr.reset(new char[modelDataSize], std::default_delete<char[]>());
        fread(ainlpllModeDataPtr.get(), 1, modelDataSize, binFilePtr);
        fclose(binFilePtr);
        g_ap_service_impl->ainlpll_model_config.modelDataPtr = ainlpllModeDataPtr;
        g_ap_service_impl->ainlpll_model_config.modelDataSize = modelDataSize;
        g_ap_service_impl->ainlpll_model_config.modelName = modelName;
        //ap->SetAIModelResource(g_ap_service_impl->ainlpll_model_config);
    } else {
        printf("open bin file error!\n");
    }  
    return 0;
}

AGORA_API_C_VOID agora_ap_service_release(AGORA_API_C_HDL service_handle)
{
    delete g_ap_service_impl;
    g_ap_service_impl = nullptr;
}

_agora_ap_processor_config agora_ap_processor_config_create()
{
    _agora_ap_processor_config config;
    

    // aec config
    config.aec_config.enabled = false;
    config.aec_config.filterLength = (int)AgoraUAP::AgoraAudioProcessing::AecFilterLength::kNormal;
    config.aec_config.aecModelType = (int)AgoraUAP::AgoraAudioProcessing::AecModelType::kLLAIAEC;
    config.aec_config.aiaecSuppressionMode = (int)AgoraUAP::AgoraAudioProcessing::AIAECSuppressionMode::kChatMode;
    config.aec_config.aecSuppressionMode = (int)AgoraUAP::AgoraAudioProcessing::AECSuppressionMode::kAggressiveAEC;

    // ans config
    config.ans_config.enabled = true;
    config.ans_config.ansModelType = (int)AgoraUAP::AgoraAudioProcessing::AnsModelType::kSTDAIANS;  
    config.ans_config.suppressionMode = (int)AgoraUAP::AgoraAudioProcessing::AnsSuppressionMode::kModerate;
    config.ans_config.speechProtectThreshold = 100;
  
    // agc config
    config.agc_config.enabled = false;
    config.agc_config.maxDigitalGaindB = 12;
    config.agc_config.targetleveldB = 6;
    config.agc_config.curve_slope = 17;
    // bghvs config
    config.bghvs_config.enabled = true;
    config.bghvs_config.bghvsSOSLenInMs = 160;
    config.bghvs_config.bghvsEOSLenInMs = 500;
    config.bghvs_config.bghvsDelayInFrmNums = 12;
    config.bghvs_config.bghvsSppMode = (int)AgoraUAP::AgoraAudioProcessing::BghvsSuppressionMode::kBGHVS_Moderate;
 

    return config;
}
const char* agora_ap_processor_config_get_message(_agora_ap_processor_config *config)
{
    if (config == nullptr) {
        return "empty config";
    }
    // aec config
    //should map to c++ version define
    /*
    printf("[APM_TEST]aecconfig:aec enable = %d\n",static_cast<int>(aecConfig.enabled.value()));
    printf("[APM_TEST]aecconfig:stereoAecEnabled = %d\n",static_cast<int>(aecConfig.stereoAecEnabled.value()));
    printf("[APM_TEST]aecconfig:enableAecAutoReset = %d\n",static_cast<int>(aecConfig.enableAecAutoReset.value()));    
    printf("[APM_TEST]aecconfig:aecStartupMaxSuppressTimeInMs = %d\n",aecConfig.aecStartupMaxSuppressTimeInMs.value());   
    printf("[APM_TEST]aecconfig:filterLength = %d\n",static_cast<int>(aecConfig.filterLength.value())); 
    printf("[APM_TEST]aecconfig:aecModelType = %d\n",static_cast<int>(aecConfig.aecModelType.value())); 
    printf("[APM_TEST]aecconfig:aecSuppressionMode = %d\n",static_cast<int>(aecConfig.aecSuppressionMode.value()));   
    printf("[APM_TEST]aecconfig:aiaecSuppressionMode = %d\n",static_cast<int>(aecConfig.aiaecSuppressionMode.value()));         

    // ans config
    printf("[APM_TEST]ansconfig:ans enable = %d\n",static_cast<int>(ansConfig.enabled.value()));
    printf("[APM_TEST]ansconfig:ansModelType = %d\n",static_cast<int>(ansConfig.ansModelType.value()));  
    printf("[APM_TEST]ansconfig:suppressionMode = %d\n",static_cast<int>(ansConfig.suppressionMode.value()));
    printf("[APM_TEST]ansconfig:aec enable = %d\n",ansConfig.speechProtectThreshold.value());

    // agc config
    printf("[APM_TEST]agcconfig:agc enable = %d\n",static_cast<int>(agcConfig.enabled.value()));
    printf("[APM_TEST]agcconfig:maxDigitalGaindB = %d\n",agcConfig.maxDigitalGaindB.value());  
    printf("[APM_TEST]agcconfig:targetleveldB = %d\n",agcConfig.targetleveldB.value());  
    printf("[APM_TEST]agcconfig:curve_slope = %d\n",agcConfig.curve_slope.value());    

    // bghvs config
    printf("[APM_TEST]bgConfig:bgConfig.enabled = %d\n",static_cast<int>(bgConfig.enabled.value()));
    printf("[APM_TEST]bgConfig:bgConfig.bghvsSOSLenInMs = %d\n",bgConfig.bghvsSOSLenInMs.value());  
    printf("[APM_TEST]bgConfig:bgConfig.bghvsEOSLenInMs = %d\n",bgConfig.bghvsEOSLenInMs.value());  
    printf("[APM_TEST]bgConfig:bgConfig.bghvsDelayInFrmNums = %d\n",bgConfig.bghvsDelayInFrmNums.value()); 
    printf("[APM_TEST]bgConfig:bgConfig.bghvsSppMode = %d\n",static_cast<int>(bgConfig.bghvsSppMode.value()));   
    */

    return "config";
}

AgoraUAP::AgoraAudioProcessing::AecConfig  mapaecconfig(const _agora_ap_processor_config& config)
{
    AgoraUAP::AgoraAudioProcessing::AecConfig aecConfig;
    aecConfig.enabled = config.aec_config.enabled;
    aecConfig.stereoAecEnabled = config.aec_config.stereoAecEnabled;
    aecConfig.enableAecAutoReset = config.aec_config.enableAecAutoReset;
    aecConfig.aecStartupMaxSuppressTimeInMs = config.aec_config.aecStartupMaxSuppressTimeInMs;
    aecConfig.filterLength = config.aec_config.filterLength;
    aecConfig.aecModelType = config.aec_config.aecModelType;
    aecConfig.aiaecSuppressionMode = config.aec_config.aiaecSuppressionMode;
    aecConfig.aiaecSuppressionMode = config.aec_config.aiaecSuppressionMode;

    return aecConfig;

}

AGORA_API_C_HDL agora_ap_processor_create(AGORA_API_C_HDL service_handle, const _agora_ap_processor_config& config)
{
    if (service_handle == nullptr || service_handle != g_ap_service_impl || g_ap_service_impl == nullptr) {
        return nullptr;
    }

    _agora_ap_service_impl* service_impl = static_cast<_agora_ap_service_impl*>(service_handle);
    if (service_impl->is_initialized == false) {
        return nullptr;
    }

    // create processor
    _agora_ap_processor_impl* processor_impl = new _agora_ap_processor_impl();
    AgoraUAP::AgoraAudioProcessing* processor =  CreateAgoraAudioProcessing();
    std::shared_ptr<APHandler> handler = std::make_shared<APHandler>(processor_impl, service_impl->event_handler);
    if (processor == nullptr) {

        //should close

        return nullptr;
    }
    processor_impl->processor = processor;
    processor_impl->handler = handler;

    const char* APPID = service_impl->config.app_id;
    const char* LICENSE = service_impl->config.license;
   

    // init processor
    int ret = processor->Init(AgoraUAP::AgoraAudioProcessing::UapConfig(APPID, LICENSE, handler.get()));
    
    // set ai model resource
    processor->SetAIModelResource(service_impl->ains_model_config);
    processor->SetAIModelResource(service_impl->ainsll_model_config);
    processor->SetAIModelResource(service_impl->ainlp_model_config);
    processor->SetAIModelResource(service_impl->ainlpll_model_config);

    // Must map c type to c++
    AgoraUAP::AgoraAudioProcessing::AecConfig aec_config;
    AgoraUAP::AgoraAudioProcessing::AgcConfig agc_config;
    AgoraUAP::AgoraAudioProcessing::AnsConfig ans_config;
    AgoraUAP::AgoraAudioProcessing::BGHVSCfg bghvs_config;



    // set processor config
    ret = processor->SetAecConfiguration(config.aec_config);
    ret = processor->SetAnsConfiguration(config.ans_config);
    ret = processor->SetAgcConfiguration(config.agc_config);
    ret = processor->SetBGHVSConfiguration(config.bghvs_config);

    //dump option
    AgoraUAP::AgoraAudioProcessing::DumpOption dumpOption(true, "./dump/"); //directory
    ret = processor->EnableDataDump(dumpOption);


    return processor_impl;
}
AGORA_API_C_INT agora_ap_processor_release(AGORA_API_C_HDL processor_handle)
{
    if (processor_handle == nullptr ) {
        return -1;
    }
    _agora_ap_processor_impl* processor_impl = static_cast<_agora_ap_processor_impl*>(processor_handle);
    if (processor_impl->processor == nullptr) {
        return -2;
    }
    processor_impl->processor->Release();
    processor_impl->processor = nullptr;
    processor_impl->handler = nullptr;
    if (processor_impl->aec_ref_frame)
    {
        if (processor_impl->aec_ref_frame->buffer) {
            delete[] processor_impl->aec_ref_frame->buffer;
            processor_impl->aec_ref_frame->buffer = nullptr;
        }
        delete processor_impl->aec_ref_frame;
        processor_impl->aec_ref_frame = nullptr;
    }
    delete processor_impl;
    processor_impl = nullptr;
    return 0;
}

AGORA_API_C_INT agora_ap_processor_process_stream(AGORA_API_C_HDL processor_handle, _agora_ap_audio_frame* frame, _agora_ap_audio_frame* ref_frame)
{
    if (processor_handle == nullptr || frame == nullptr || ref_frame == nullptr) {
        return -1;
    }
    _agora_ap_processor_impl* processor_impl = static_cast<_agora_ap_processor_impl*>(processor_handle);
    if (processor_impl->processor == nullptr) {
        return -2;
    }

    int ret = 0;
    ret = processor_impl->processor->SetStreamDelayMs(60);
    ret = processor_impl->processor->SetStreamAnalogLevel(0);
  
  
    //check ref_frame is nullptr, use mute frame as ref_frame
    if (ref_frame == nullptr || processor_impl->aec_enabled == false) {
        //check and create mute frame
        // compare current frame with aec_ref_frame  if not same parameter, create mute frame
        int sample_rate = frame->sampleRate;
       //ref_frame = AgoraUAP::AgoraAudioFrame::CreateMuteFrame(frame->sampleRate, frame->numChannels, frame->numSamples);
    }
    // directly memory address map do not do copy!!,but should ensure the memory layout is same to c++ defined
    // 直接进行内存地址映射，不进行拷贝，但需要确保内存布局与C++定义的相同
    
    AgoraUAP::AgoraAudioFrame* agora_frame = reinterpret_cast<AgoraUAP::AgoraAudioFrame*>(frame);
    AgoraUAP::AgoraAudioFrame* agora_ref_frame = reinterpret_cast<AgoraUAP::AgoraAudioFrame*>(ref_frame);

    ret = processor_impl->processor->ProcessReverseStream(agora_ref_frame);
    ret = processor_impl->processor->ProcessStream(agora_frame);
    return ret;
}




#ifdef __cplusplus
}
#endif // __cplusplus
