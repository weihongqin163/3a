#include "3a.h"
#include <string>

#include "agora_audio_processing.h"

using namespace std;

typedef struct _agora_ap_service_impl {
    bool is_initialized;
    _agora_ap_service_config config;
    //ai model resource
    AgoraUAP::AgoraAudioProcessing::AiModelResourceConfig ainsConfig;
    AgoraUAP::AgoraAudioProcessing::AiModelResourceConfig ainsllConfig;  
    AgoraUAP::AgoraAudioProcessing::AiModelResourceConfig ainlpConfig;
    AgoraUAP::AgoraAudioProcessing::AiModelResourceConfig ainlpllConfig;  
} ;


static _agora_ap_service_impl  *g_ap_service_impl = nullptr;
AGORA_API_C_HDL agora_ap_service_create()
{
    //check if g_ap_service_impl is nullptr, if nullptr, create a new one
    if (g_ap_service_impl == nullptr) {
        g_ap_service_impl = new _agora_ap_service_impl();
    }
   return g_ap_service_impl;
}

AGORA_API_C_INT agora_ap_service_initialize(AGORA_API_C_HDL service_handle, const _agora_ap_service_config* config)
{
    if (!service_handle || service_handle != g_ap_service_impl || g_ap_service_impl == nullptr) {
        return -1;
    }
    if (g_ap_service_impl->is_initialized) {
        return 0;
    }
    g_ap_service_impl->config = *config;
    g_ap_service_impl->is_initialized = true;

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
    std::shared_ptr<char> ainsModelDataPtr(NULL, std::default_delete<char[]>());
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
        g_ap_service_impl->ainsConfig.modelDataPtr = ainsModelDataPtr;
        g_ap_service_impl->ainsConfig.modelDataSize = modelDataSize;
        g_ap_service_impl->ainsConfig.modelName = modelName;
        ap->SetAIModelResource(g_ap_service_impl->ainsConfig);
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
        g_ap_service_impl->ainsllConfig.modelDataPtr = ainsllModelDataPtr;
        g_ap_service_impl->ainsllConfig.modelDataSize = modelDataSize;
        g_ap_service_impl->ainsllConfig.modelName = modelName;
        ap->SetAIModelResource(g_ap_service_impl->ainsllConfig);
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
        g_ap_service_impl->ainlpConfig.modelDataPtr = ainlpModeDataPtr;
        g_ap_service_impl->ainlpConfig.modelDataSize = modelDataSize;
        g_ap_service_impl->ainlpConfig.modelName = modelName;
        ap->SetAIModelResource(g_ap_service_impl->ainlpConfig);
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
        g_ap_service_impl->ainlpllConfig.modelDataPtr = ainlpllModeDataPtr;
        g_ap_service_impl->ainlpllConfig.modelDataSize = modelDataSize;
        g_ap_service_impl->ainlpllConfig.modelName = modelName;
        ap->SetAIModelResource(g_ap_service_impl->ainlpllConfig);
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
    config.aec_config = AgoraUAP::AgoraAudioProcessing::AecConfig();
    config.ans_config = AgoraUAP::AgoraAudioProcessing::AnsConfig();
    config.agc_config = AgoraUAP::AgoraAudioProcessing::AgcConfig();
    config.bghvs_config = AgoraUAP::AgoraAudioProcessing::BGHVSCfg();

    // aec config
    config.aec_config.enabled = false;
    config.aec_config.filterLength = AgoraUAP::AgoraAudioProcessing::AecFilterLength::kNormal;
    config.aec_config.aecModelType = AgoraUAP::AgoraAudioProcessing::AecModelType::kLLAIAEC;
    config.aec_config.aiaecSuppressionMode = AgoraUAP::AgoraAudioProcessing::AIAECSuppressionMode::kChatMode;
    config.aec_config.aecSuppressionMode = AgoraUAP::AgoraAudioProcessing::AECSuppressionMode::kAggressiveAEC;

    // ans config
    config.ans_config.enabled = true;
    config.ans_config.ansModelType = AgoraUAP::AgoraAudioProcessing::AnsModelType::kSTDAIANS;  
    config.ans_config.suppressionMode = AgoraUAP::AgoraAudioProcessing::AnsSuppressionMode::kModerate;
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
    config.bghvs_config.bghvsSppMode = AgoraUAP::AgoraAudioProcessing::BghvsSuppressionMode::kBGHVS_Moderate;
 

    return config;
}






