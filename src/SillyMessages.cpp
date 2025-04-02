#pragma once

#include "SillyMessages.h"

SillyMessages& SillyMessages::instance() {
    static SillyMessages instance;
    return instance;
}
