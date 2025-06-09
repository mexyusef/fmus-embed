#include "fmus/core/version.h"
#include <sstream>

namespace fmus {
namespace core {

// Version numbers
static const int MAJOR_VERSION = 0;
static const int MINOR_VERSION = 1;
static const int PATCH_VERSION = 0;

std::string getVersionString() {
    std::stringstream ss;
    ss << MAJOR_VERSION << "." << MINOR_VERSION << "." << PATCH_VERSION;
    return ss.str();
}

int getMajorVersion() {
    return MAJOR_VERSION;
}

int getMinorVersion() {
    return MINOR_VERSION;
}

int getPatchVersion() {
    return PATCH_VERSION;
}

bool isVersionAtLeast(int major, int minor, int patch) {
    if (MAJOR_VERSION > major) {
        return true;
    }
    if (MAJOR_VERSION < major) {
        return false;
    }

    // Major versions are equal, check minor
    if (MINOR_VERSION > minor) {
        return true;
    }
    if (MINOR_VERSION < minor) {
        return false;
    }

    // Major and minor versions are equal, check patch
    return PATCH_VERSION >= patch;
}

} // namespace core
} // namespace fmus
