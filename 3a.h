#ifndef AGORA_API_3A_H
#define AGORA_API_3A_H

#include "agora_audio_processing.h"


typedef void* AGORA_API_C_HDL;
typedef int AGORA_API_C_INT;
typedef void AGORA_API_C_VOID;

typedef struct _agora_ap_service_config {
    const char* app_id;
    const char* license;
    const char* resource_path;  //like: /user/xx/resource/
} ;

typedef struct _agora_ap_processor_event_handler {
    void (*on_event)(void* context, int event_type, void* event_data);
    void (*on_error)(void* context, int error_code, const char* error_message);
} ;

typedef struct _agora_ap_processor_config {
    // aec
    AgoraUAP::AgoraAudioProcessing::AecConfig aec_config;
    // ns
    AgoraUAP::AgoraAudioProcessing::AnsConfig ans_config;
    // agc
    AgoraUAP::AgoraAudioProcessing::AgcConfig agc_config;
    //bghvs
   AgoraUAP::AgoraAudioProcessing::BGHVSCfg bghvs_config;    
} ;

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
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

AGORA_API_C_HDL agora_ap_processor_create(AGORA_API_C_HDL service_handle, const _agora_ap_processor_config& config);
AGORA_API_C_INT agora_ap_processor_release(AGORA_API_C_HDL processor_handle);
AGORA_API_C_INT agora_ap_processor_process_stream(AGORA_API_C_HDL processor_handle, AgoraUAP::AgoraAudioFrame* frame, AgoraUAP::AgoraAudioFrame* ref_frame);



#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AGORA_API_3A_H