#pragma once

#include "rfcommon/config.hpp"
#include <cstdio>

#if defined(__GNUC__) || defined(__clang__)
#   define PRINTF_FORMAT(fmt, params) __attribute__((format(printf, fmt, params)))
#else
#   define PRINTF_FORMAT(fmt, params)
#endif

namespace rfcommon {

struct LogPrivate;

/*!
 * @brief A simple HTML logger.
 *
 * Messages can be logged via the global variable *defaultLogger*, which is an
 * instance of this class.
 *
 * All messages are converted to HTML so it looks nice in a browser.
 */
#if defined(RFCOMMON_LOGGING)
class RFCOMMON_PUBLIC_API Log
{
    Log();
    ~Log();
public:
    static void init(const char* logPath);
    static void deinit();

    static Log* root();

    Log* child(const char* logName);

    /*!
     * @brief Begins a dropdown box. All proceeding logged messages are written
     * to the dropdown box.
     *
     * Dropdowns are useful for keeping the overall log clean from clutter.
     * One might have to dump large amounts of debug data that is not
     * necessarily directly relevant, but still useful. Putting this kind of
     * data in a dropdown hides the data initially from the viewer.
     *
     * @param[in] title This is the title of the dropdown box. When clicking
     */
    void beginDropdown(const char* title, ...);

    /*!
     * @brief Ends a dropdown box. All proceeding logged messages are written
     * normally again.
     */
    void endDropdown();

    /// Debug, noisy stuff
    PRINTF_FORMAT(2, 3)
#if RFCOMMON_LOG_LEVEL <= RFCOMMON_LOG_DEBUG
    void debug(const char* fmt, ...);
#else
    void debug(const char* fmt, ...) {}
#endif

    /// Non-critical information. Uses the printf-style format string.
    PRINTF_FORMAT(2, 3)
#if RFCOMMON_LOG_LEVEL <= RFCOMMON_LOG_INFO
    void info(const char* fmt, ...);
#else
    void info(const char* fmt, ...) {}
#endif

    /// More interesting information, but not critical. Uses the printf-style format string.
    PRINTF_FORMAT(2, 3)
#if RFCOMMON_LOG_LEVEL <= RFCOMMON_LOG_NOTICE
    void notice(const char* fmt, ...);
#else
    void notice(const char* fmt, ...) {}
#endif

    /// Warning message. Uses the printf-style format string.
    PRINTF_FORMAT(2, 3)
#if RFCOMMON_LOG_LEVEL <= RFCOMMON_LOG_WARNING
    void warning(const char* fmt, ...);
#else
    void warning(const char* fmt, ...) {}
#endif

    /// An error has occurred. Uses the printf-style format string.
    PRINTF_FORMAT(2, 3)
#if RFCOMMON_LOG_LEVEL <= RFCOMMON_LOG_ERROR
    void error(const char* fmt, ...);
#else
    void error(const char* fmt, ...) {}
#endif

    /// A fatal error has occurred. Uses the printf-style format string.
    PRINTF_FORMAT(2, 3)
#if RFCOMMON_LOG_LEVEL <= RFCOMMON_LOG_FATAL
    void fatal(const char* fmt, ...);
#else
    void fatal(const char* fmt, ...) {}
#endif

    //! Returns the open file stream of the log file.
    FILE* fileStream() const;
    //! Returns the file name of the open log file. This includes its path.
    const char* fileName() const;
    //! Returns the name of the logger.
    const char* name() const;

private:
    LogPrivate* d_;
};
#else
class RFCOMMON_PUBLIC_API Log
{
    Log() {}
    ~Log() {}
public:
    static void init(const char* logPath) {}
    static void deinit() {}
    static Log* root() { static Log log; return &log; }
    Log* child(const char* logName) { return this; }
    void beginDropdown(const char* title, ...) {}
    void endDropdown() {}
    void debug(const char* fmt, ...) {}
    void info(const char* fmt, ...) {}
    void notice(const char* fmt, ...) {}
    void warning(const char* fmt, ...) {}
    void error(const char* fmt, ...) {}
    void fatal(const char* fmt, ...) {}

    FILE* fileStream() const { return nullptr; }
    const char* fileName() const { return ""; }
    const char* name() const { return ""; }
};
#endif

class RFCOMMON_PUBLIC_API LogDropdown
{
public:
    LogDropdown(Log* log, const char* name) : log_(log) { log_->beginDropdown(name); }
    ~LogDropdown() { log_->endDropdown(); }

private:
    Log* log_;
};

} // namespace rfcommon
