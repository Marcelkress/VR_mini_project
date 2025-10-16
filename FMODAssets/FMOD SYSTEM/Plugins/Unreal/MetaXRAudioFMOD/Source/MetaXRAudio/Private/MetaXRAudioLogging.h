#pragma once

#define METAXR_AUDIO_LOG(Msg, ...) UE_LOG(LogAudio, Log, TEXT(Msg), ##__VA_ARGS__)
#define METAXR_AUDIO_LOG_WARNING(Msg, ...) UE_LOG(LogAudio, Warning, TEXT(Msg), ##__VA_ARGS__)
#define METAXR_AUDIO_LOG_ERROR(Msg, ...) UE_LOG(LogAudio, Error, TEXT(Msg), ##__VA_ARGS__)
