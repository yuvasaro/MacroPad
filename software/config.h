#ifndef CONFIG_H
#define CONFIG_H

#include <filesystem>

#ifdef _WIN32

#elif __APPLE__
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#elif __linux__
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

class Config
{
public:
    static std::filesystem::path getConfigDir();
};

#endif // CONFIG_H
