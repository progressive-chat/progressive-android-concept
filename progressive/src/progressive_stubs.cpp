#include "progressive/progressive.h"

namespace progressive {

std::string version() {
    return std::to_string(PROGRESSIVE_VERSION_MAJOR) + "." +
           std::to_string(PROGRESSIVE_VERSION_MINOR) + "." +
           std::to_string(PROGRESSIVE_VERSION_PATCH);
}

} // namespace progressive
