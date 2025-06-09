#pragma once

#include "../fmus_config.h"
#include <string>

namespace fmus {
namespace core {

/**
 * @brief Version information for the fmus-embed library
 */
struct FMUS_EMBED_API Version {
    int major;    ///< Major version number
    int minor;    ///< Minor version number
    int patch;    ///< Patch version number

    /**
     * @brief Get the version as a string
     *
     * @return std::string The version string in the format "major.minor.patch"
     */
    std::string toString() const;

    /**
     * @brief Compare versions for equality
     *
     * @param other The version to compare with
     * @return true if the versions are equal
     * @return false otherwise
     */
    bool operator==(const Version& other) const;

    /**
     * @brief Compare versions for inequality
     *
     * @param other The version to compare with
     * @return true if the versions are not equal
     * @return false otherwise
     */
    bool operator!=(const Version& other) const;

    /**
     * @brief Compare versions for less than
     *
     * @param other The version to compare with
     * @return true if this version is less than the other
     * @return false otherwise
     */
    bool operator<(const Version& other) const;

    /**
     * @brief Compare versions for less than or equal
     *
     * @param other The version to compare with
     * @return true if this version is less than or equal to the other
     * @return false otherwise
     */
    bool operator<=(const Version& other) const;

    /**
     * @brief Compare versions for greater than
     *
     * @param other The version to compare with
     * @return true if this version is greater than the other
     * @return false otherwise
     */
    bool operator>(const Version& other) const;

    /**
     * @brief Compare versions for greater than or equal
     *
     * @param other The version to compare with
     * @return true if this version is greater than or equal to the other
     * @return false otherwise
     */
    bool operator>=(const Version& other) const;
};

/**
 * @brief Get the current library version
 *
 * @return Version The current version
 */
FMUS_EMBED_API Version getVersion();

/**
 * @brief Get the current library version as a string
 *
 * @return std::string The version string
 */
FMUS_EMBED_API std::string getVersionString();

/**
 * @brief Get the major version number
 *
 * @return int Major version number
 */
FMUS_EMBED_API int getMajorVersion();

/**
 * @brief Get the minor version number
 *
 * @return int Minor version number
 */
FMUS_EMBED_API int getMinorVersion();

/**
 * @brief Get the patch version number
 *
 * @return int Patch version number
 */
FMUS_EMBED_API int getPatchVersion();

/**
 * @brief Check if the current version is at least the specified version
 *
 * @param major Major version to check
 * @param minor Minor version to check
 * @param patch Patch version to check
 * @return bool True if the current version is at least the specified version
 */
FMUS_EMBED_API bool isVersionAtLeast(int major, int minor, int patch);

} // namespace core
} // namespace fmus
