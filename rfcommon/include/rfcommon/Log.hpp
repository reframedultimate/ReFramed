#pragma once

#include "rfcommon/config.hpp"
#include <cstdio>

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

    /// Non-critical information. Uses the printf-style format string.
    void info(const char* fmt, ...);
    /// More interesting information, but not critical. Uses the printf-style format string.
    void notice(const char* fmt, ...);
    /// Warning message. Uses the printf-style format string.
    void warning(const char* fmt, ...);
    /// An error has occurred. Uses the printf-style format string.
    void error(const char* fmt, ...);
    /// A fatal error has occurred. Uses the printf-style format string.
    void fatal(const char* fmt, ...);

    //! Returns the open file stream of the log file.
    FILE* fileStream() const;
    //! Returns the file name of the open log file. This includes its path.
    const char* fileName() const;
    //! Returns the name of the logger.
    const char* name() const;

private:
    LogPrivate* d_;
};

} // namespace common
