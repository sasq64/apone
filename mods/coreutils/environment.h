#pragma once

#include <string>
#include <mutex>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ERROR
#else
#include <unistd.h>
#endif

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

class Environment
{
public:
    /** $HOME dir */
    static fs::path const& getHomeDir()
    {
        std::lock_guard<std::mutex> lock(m);
        if (homeDir.empty()) {
#ifdef _WIN32
            homeDir = getenv("USERPROFILE");
#else
            homeDir = getenv("HOME");
#endif
        }
        return homeDir;
    }

    /** Directory of the running executable */
    static fs::path const& getExeDir()
    {
        std::lock_guard<std::mutex> lock(m);

        if (exeDir.empty()) {
            char buf[1024];
#if defined _WIN32
            GetModuleFileName(nullptr, buf, sizeof(buf) - 1);
            exeDir = fs::path(buf).parent_path();
#elif defined __APPLE__
            uint32_t size = sizeof(buf);
            if (_NSGetExecutablePath(buf, &size) == 0) {
                exeDir = fs::path(buf).parent_path();
            }
#else
            int rc = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
            if (rc >= 0) {
                buf[rc] = 0;
                exeDir = fs::path(buf).parent_path();
            }
#endif
        }
        return exeDir;
    }

    /** User specific writable cache dir, normally $HOME/.config/$APPNAME */
    static fs::path const& getCacheDir()
    {
		const auto& homeDir = getHomeDir();
        std::lock_guard<std::mutex> lock(m);
        if (cacheDir.empty()) {
            cacheDir = homeDir / ".cache" / appName;
            if (!fs::exists(cacheDir)) fs::create_directories(cacheDir);
        }
        return cacheDir;
    }

    /** User specific config dir, normally $HOME/.config/$APPNAME */
    static fs::path const& getConfigDir()
    {
		const auto& homeDir = getHomeDir();
        std::lock_guard<std::mutex> lock(m);
        if (configDir.empty()) {
            configDir = homeDir / ".config" / appName;
            if (!fs::exists(configDir)) fs::create_directories(configDir);
        }
        return configDir;
    }

    /** The application data directory.
     * Normally $EXEDIR/../Resources on OSX or /usr/share/$APPNAME on *nix */
    static fs::path getAppDir()
    {
		const auto& exeDir = getExeDir();
		std::lock_guard<std::mutex> lock(m);
        if (appDir.empty()) {
#ifdef __APPLE__
            appDir = exeDir / ".." / "Resources";
#elif (defined _WIN32)
            appDir = exeDir;
#else
            if (appName.empty())
                appDir = exeDir;
            else
                appDir = "/usr/share/" + appName;
#endif
			LOGD("APPDIR %s", appDir);
			if(fs::exists(appDir))
				appDir = fs::canonical(appDir);
			else
				appDir = exeDir;
        }
        return appDir;
    }

    static void setAppName(std::string const& aname) { appName = aname; }

private:

	inline static fs::path homeDir;
    inline static fs::path exeDir;
    inline static fs::path configDir;
    inline static fs::path cacheDir;
    inline static fs::path appDir;

    inline static std::string appName;
    inline static std::mutex m;
};
