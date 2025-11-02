#include "DebugNotification.h"
#include <RE/Skyrim.h>
#include <REL/Relocation.h>

namespace RE {
    void DebugNotification(const char* a_notification, const char* a_soundToPlay, bool a_cancelIfAlreadyQueued) {
        // Exact copy from old CommonLib Misc.cpp
        using func_t = decltype(&DebugNotification);

        // NOTE: For Skyrim AE 1.6.1170, offset ID 52933 works (not 52050)
        // ID 52050 exists but points to the wrong address/function
        static REL::Relocation<func_t> func{REL::ID(52933)};

        return func(a_notification, a_soundToPlay, a_cancelIfAlreadyQueued);
    }
}
