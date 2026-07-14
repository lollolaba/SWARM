#pragma once
#include <string>
#include "../events/event.h"
#include "../../environment/channel/acoustic_channel.h"
#include "../../algorithms/localization/localization_stack.h"
std::string EventTypeToString(EventType type);
std::string LocalizationModeToString(LocalizationMode mode);
std::string ChannelModeToString(ChannelMode mode);