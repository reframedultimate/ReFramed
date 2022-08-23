#include "rfcommon/Log.hpp"
#include "rfcommon/BuildInfo.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/String.hpp"

#include <cstdarg>
#include <ctime>
#include <cstring>
#include <cstdlib>

#if defined(RFCOMMON_LOGGING)

namespace rfcommon {

static Log* rootLog = nullptr;
static const char* head = "<html>\n"
"    <head>\n"
"        <style type=\"text/css\">\n"
"\n"
"            body{\n"
"            font-family: Trebuchet MS, Lucida Sans Unicode, Arial, sans-serif;    /* Font to use */\n"
"            margin:0px;\n"
"\n"
"            }\n"
"\n"
"            .dhtmlgoodies_question{    /* Styling question */\n"
"            /* Start layout CSS */\n"
"            color:#FFF;\n"
"            font-size:0.9em;\n"
"            background-color:#317082;\n"
"            width:1230px;\n"
"            margin-bottom:2px;\n"
"            margin-top:2px;\n"
"            padding-left:2px;\n"
"            height:20px;\n"
"\n"
"            /* End layout CSS */\n"
"\n"
"            overflow:hidden;\n"
"            cursor:pointer;\n"
"            }\n"
"            .dhtmlgoodies_answer{    /* Parent box of slide down content */\n"
"            /* Start layout CSS */\n"
"            border:1px solid #317082;\n"
"            background-color:#E2EBED;\n"
"            width:1200px;\n"
"            background-image:url('data/rd.png');\n"
"            background-repeat:repeat;\n"
"            background-position:top right;\n"
"\n"
"            /* End layout CSS */\n"
"\n"
"            visibility:hidden;\n"
"            height:0px;\n"
"            overflow:hidden;\n"
"            position:relative;\n"
"\n"
"            }\n"
"            .dhtmlgoodies_answer_content{    /* Content that is slided down */\n"
"            padding:1px;\n"
"            font-size:0.9em;\n"
"            position:relative;\n"
"            }\n"
"\n"
"        </style>\n"
"\n"
"        <script type=\"text/javascript\">\n"
"            /************************************************************************************************************\n"
"            Show hide content with slide effect\n"
"            Copyright (C) August 2010  DTHMLGoodies.com, Alf Magne Kalleland\n"
"\n"
"            This library is free software; you can redistribute it and/or\n"
"            modify it under the terms of the GNU Lesser General Public\n"
"            License as published by the Free Software Foundation; either\n"
"            version 2.1 of the License, or (at your option) any later version.\n"
"\n"
"            This library is distributed in the hope that it will be useful,\n"
"            but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"            MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
"            Lesser General Public License for more details.\n"
"\n"
"            You should have received a copy of the GNU Lesser General Public\n"
"            License along with this library; if not, write to the Free Software\n"
"            Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA\n"
"\n"
"            Dhtmlgoodies.com., hereby disclaims all copyright interest in this script\n"
"            written by Alf Magne Kalleland.\n"
"\n"
"            Alf Magne Kalleland, 2010\n"
"            Owner of DHTMLgoodies.com\n"
"\n"
"            ************************************************************************************************************/\n"
"\n"
"            var dhtmlgoodies_slideSpeed = 600;    // Higher value = faster\n"
"            var dhtmlgoodies_timer = 10;    // Lower value = faster\n"
"\n"
"            var objectIdToSlideDown = false;\n"
"            var dhtmlgoodies_activeId = false;\n"
"            var dhtmlgoodies_slideInProgress = false;\n"
"            var dhtmlgoodies_slideInProgress = false;\n"
"            var dhtmlgoodies_expandMultiple = true; // true if you want to be able to have multiple items expanded at the same time.\n"
"\n"
"            function showHideContent(e,inputId)\n"
"            {\n"
"            if(dhtmlgoodies_slideInProgress)return;\n"
"            dhtmlgoodies_slideInProgress = true;\n"
"            if(!inputId)inputId = this.id;\n"
"            inputId = inputId + '';\n"
"            var numericId = inputId.replace(/[^0-9]/g,'');\n"
"            var answerDiv = document.getElementById('dhtmlgoodies_a' + numericId);\n"
"\n"
"            objectIdToSlideDown = false;\n"
"\n"
"            if(!answerDiv.style.display || answerDiv.style.display=='none'){\n"
"                if(dhtmlgoodies_activeId &&  dhtmlgoodies_activeId!=numericId && !dhtmlgoodies_expandMultiple){\n"
"                    objectIdToSlideDown = numericId;\n"
"                    slideContent(dhtmlgoodies_activeId,(dhtmlgoodies_slideSpeed*-1));\n"
"                }else{\n"
"\n"
"                    answerDiv.style.display='block';\n"
"                    answerDiv.style.visibility = 'visible';\n"
"\n"
"                    slideContent(numericId,dhtmlgoodies_slideSpeed);\n"
"                }\n"
"            }else{\n"
"                slideContent(numericId,(dhtmlgoodies_slideSpeed*-1));\n"
"                dhtmlgoodies_activeId = false;\n"
"            }\n"
"            }\n"
"\n"
"            function slideContent(inputId,direction)\n"
"            {\n"
"\n"
"            var obj =document.getElementById('dhtmlgoodies_a' + inputId);\n"
"            var contentObj = document.getElementById('dhtmlgoodies_ac' + inputId);\n"
"            height = obj.clientHeight;\n"
"            if(height==0)height = obj.offsetHeight;\n"
"            height = height + direction;\n"
"            rerunFunction = true;\n"
"            if(height>contentObj.offsetHeight){\n"
"                height = contentObj.offsetHeight;\n"
"                rerunFunction = false;\n"
"            }\n"
"            if(height<=1){\n"
"                height = 1;\n"
"                rerunFunction = false;\n"
"            }\n"
"\n"
"            obj.style.height = height + 'px';\n"
"            var topPos = height - contentObj.offsetHeight;\n"
"            if(topPos>0)topPos=0;\n"
"            contentObj.style.top = topPos + 'px';\n"
"            if(rerunFunction){\n"
"                setTimeout('slideContent(' + inputId + ',' + direction + ')',dhtmlgoodies_timer);\n"
"            }else{\n"
"                if(height<=1){\n"
"                    obj.style.display='none';\n"
"                    if(objectIdToSlideDown && objectIdToSlideDown!=inputId){\n"
"                        document.getElementById('dhtmlgoodies_a' + objectIdToSlideDown).style.display='block';\n"
"                        document.getElementById('dhtmlgoodies_a' + objectIdToSlideDown).style.visibility='visible';\n"
"                        slideContent(objectIdToSlideDown,dhtmlgoodies_slideSpeed);\n"
"                    }else{\n"
"                        dhtmlgoodies_slideInProgress = false;\n"
"                    }\n"
"                }else{\n"
"                    dhtmlgoodies_activeId = inputId;\n"
"                    dhtmlgoodies_slideInProgress = false;\n"
"                }\n"
"            }\n"
"            }\n"
"\n"
"            function initShowHideDivs()\n"
"            {\n"
"            var divs = document.getElementsByTagName('DIV');\n"
"            var divCounter = 1;\n"
"            for(var no=0;no<divs.length;no++){\n"
"                if(divs[no].className=='dhtmlgoodies_question'){\n"
"                    divs[no].onclick = showHideContent;\n"
"                    divs[no].id = 'dhtmlgoodies_q'+divCounter;\n"
"                    var answer = divs[no].nextSibling;\n"
"                    while(answer && answer.tagName!='DIV'){\n"
"                        answer = answer.nextSibling;\n"
"                    }\n"
"                    answer.id = 'dhtmlgoodies_a'+divCounter;\n"
"                    contentDiv = answer.getElementsByTagName('DIV')[0];\n"
"                    contentDiv.style.top = 0 - contentDiv.offsetHeight + 'px';\n"
"                    contentDiv.className='dhtmlgoodies_answer_content';\n"
"                    contentDiv.id = 'dhtmlgoodies_ac' + divCounter;\n"
"                    answer.style.display='none';\n"
"                    answer.style.height='1px';\n"
"                    divCounter++;\n"
"                }\n"
"            }\n"
"            }\n"
"            window.onload = initShowHideDivs;\n"
"        </script>\n"
"    </head>\n"
"    <body>\n";

struct LogPrivate
{
    Log* parent;
    rfcommon::Vector<Log*> children;

    FILE* fp;
    rfcommon::String path;
    rfcommon::String absFileName;
    rfcommon::String name;
    rfcommon::String currentMsg;

    bool dropDownOpen = false;
};

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
static rfcommon::String currentDateTime()
{
    NOPROFILE();

    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[30];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "[%Y-%m-%d.%X] ", &tstruct);

    return buf;
}

static rfcommon::String toHTML(const char* colour, const char* tag, const char* message)
{
    NOPROFILE();

    using rfcommon::String;
    rfcommon::String str = currentDateTime();
    str += "<font color=#"; str += colour;
    str += ">["; str += tag; str += "] "; str += message;
    str += "</font>\n";
    return str;
}

// ----------------------------------------------------------------------------
Log::Log() :
    d_(new LogPrivate)
{
}

// ----------------------------------------------------------------------------
Log::~Log()
{
    info("Closing log");

    for (auto child : d_->children)
        delete child;

    fprintf(d_->fp, "</pre></body></html>");
    if (d_->fp != stderr)
        fclose(d_->fp);

    delete d_;
}

// ----------------------------------------------------------------------------
void Log::init(const char* logPath)
{
    NOPROFILE();

    if (rootLog)
        return;

    rootLog = new Log;
    rootLog->d_->parent = nullptr;
    rootLog->d_->path = logPath;
#if defined(RFCOMMON_PLATFORM_WINDOWS)
    rootLog->d_->absFileName = rootLog->d_->path + "\\ReFramed.html";
#else
    rootLog->d_->absFileName = rootLog->d_->path + "/ReFramed.html";
#endif

    rootLog->d_->fp = fopen(rootLog->d_->absFileName.cStr(), "w");
    if (rootLog->d_->fp == nullptr)
    {
        fprintf(stderr, "Failed to open log \"%s\"", rootLog->d_->absFileName.cStr());
        rootLog->d_->fp = stderr;  // backup
    }

    // write HTML header
    fprintf(rootLog->d_->fp, "%s", head);

    // make sure to open <pre> tag, this is where everything gets logged
    fprintf(rootLog->d_->fp, "<pre>");

    rootLog->beginDropdown("Build system info");
    fprintf(rootLog->d_->fp, "Commit: %s\n", BuildInfo::commit());
    fprintf(rootLog->d_->fp, "Build host: %s\n", BuildInfo::buildHost());
    fprintf(rootLog->d_->fp, "Compiler: %s\n", BuildInfo::compiler());
    fprintf(rootLog->d_->fp, "Platform: %s\n", BuildInfo::platform());
    fprintf(rootLog->d_->fp, "Bits: %d\n", BuildInfo::bits());
    rootLog->endDropdown();
}

// ----------------------------------------------------------------------------
void Log::deinit()
{
    NOPROFILE();

    delete rootLog;
    rootLog = nullptr;
}

// ----------------------------------------------------------------------------
Log* Log::root()
{
    NOPROFILE();

    return rootLog;
}

// ----------------------------------------------------------------------------
Log* Log::child(const char* logName)
{
    NOPROFILE();

    for (auto log : d_->children)
        if (strcmp(log->name(), logName) == 0)
            return log;

    rfcommon::Vector<const char*> parentNames;
    Log* log = d_->parent;
    while (log)
    {
        parentNames.insert(0, log->name());
        log = log->d_->parent;
    }

    Log* childLog = new Log;
    childLog->d_->parent = this;
    childLog->d_->path = d_->path;
#if defined(RFCOMMON_PLATFORM_WINDOWS)
#   define SEP "\\"
#else
#   define SEP "\\"
#endif
    childLog->d_->absFileName = childLog->d_->path + SEP + "ReFramed_";
    for (const char* name : parentNames)
    {
        childLog->d_->absFileName += name;
        childLog->d_->absFileName += "_";
    }
    childLog->d_->absFileName += logName;
    childLog->d_->absFileName += ".html";
    childLog->d_->name = logName;

    childLog->d_->fp = fopen(childLog->d_->absFileName.cStr(), "w");
    if (childLog->d_->fp == nullptr)
    {
        fprintf(stderr, "Failed to open log \"%s\"", childLog->d_->absFileName.cStr());
        childLog->d_->fp = stderr;  // backup
    }

    // write HTML header
    fprintf(childLog->d_->fp, "%s", head);

    // make sure to open <pre> tag, this is where everything gets logged
    fprintf(childLog->d_->fp, "<pre>");

    d_->children.push(childLog);
    return childLog;
}

#define FORWARD_TO_FPRINTF(msg)     \
        va_list va;                 \
        va_start(va, fmt);          \
        vfprintf(d_->fp, msg, va);  \
        va_end(va);                 \
        fflush(d_->fp);             \
        va_start(va, fmt);          \
        vfprintf(stderr, msg, va);  \
        va_end(va);                 \
        fflush(stdout)

// ----------------------------------------------------------------------------
#if RFCOMMON_LOG_LEVEL <= RFCOMMON_LOG_DEBUG
void Log::debug(const char* fmt, ...)
{
    NOPROFILE();
    FORWARD_TO_FPRINTF(toHTML("656565", "DEBUG", fmt).cStr());
}
#endif

// ----------------------------------------------------------------------------
#if RFCOMMON_LOG_LEVEL <= RFCOMMON_LOG_INFO
void Log::info(const char* fmt, ...)
{
    NOPROFILE();
    FORWARD_TO_FPRINTF(toHTML("353535", "INFO", fmt).cStr());
}
#endif

// ----------------------------------------------------------------------------
#if RFCOMMON_LOG_LEVEL <= RFCOMMON_LOG_NOTICE
void Log::notice(const char* fmt, ...)
{
    NOPROFILE();
    FORWARD_TO_FPRINTF(toHTML("007050", "NOTICE", fmt).cStr());
}
#endif

// ----------------------------------------------------------------------------
#if RFCOMMON_LOG_LEVEL <= RFCOMMON_LOG_WARNING
void Log::warning(const char* fmt, ...)
{
    NOPROFILE();
    FORWARD_TO_FPRINTF(toHTML("ff7000", "WARNING", fmt).cStr());
}
#endif

// ----------------------------------------------------------------------------
#if RFCOMMON_LOG_LEVEL <= RFCOMMON_LOG_ERROR
void Log::error(const char* fmt, ...)
{
    NOPROFILE();
    FORWARD_TO_FPRINTF(toHTML("ff0020", "ERROR", fmt).cStr());
}
#endif

// ----------------------------------------------------------------------------
#if RFCOMMON_LOG_LEVEL <= RFCOMMON_LOG_FATAL
void Log::fatal(const char* fmt, ...)
{
    NOPROFILE();
    FORWARD_TO_FPRINTF(toHTML("ff0020", "FATAL", fmt).cStr());
}
#endif

// ----------------------------------------------------------------------------
void Log::beginDropdown(const char* title, ...)
{
    NOPROFILE();

    if (d_->dropDownOpen)
        return;
    d_->dropDownOpen = true;

    va_list va;
    va_start(va, title);
    int len = vsnprintf(NULL, 0, title, va);
    va_end(va);
    d_->currentMsg.resize(len);
    va_start(va, title);
    vsprintf(d_->currentMsg.data(), title, va);
    va_end(va);

    d_->currentMsg = "<font color=#000000>" + d_->currentMsg + "</font>";
    fprintf(d_->fp, "%s", "</pre><div class=\"dhtmlgoodies_question\">");
    fprintf(d_->fp, "%s", d_->currentMsg.cStr());
    fprintf(d_->fp, "%s", "</div>");
    fprintf(d_->fp, "%s", "<div class=\"dhtmlgoodies_answer\"><div><ul><pre>");
}

// ----------------------------------------------------------------------------
void Log::endDropdown()
{
    NOPROFILE();

    if (d_->dropDownOpen == false)
        return;
    d_->dropDownOpen = false;

    fprintf(d_->fp, "</pre></ul></div></div><pre>");
    fflush(d_->fp);
}

// ----------------------------------------------------------------------------
FILE* Log::fileStream() const
{
    NOPROFILE();

    return d_->fp;
}

// ----------------------------------------------------------------------------
const char* Log::fileName() const
{
    NOPROFILE();

    return d_->absFileName.cStr();
}

// ----------------------------------------------------------------------------
const char* Log::name() const
{
    NOPROFILE();

    return d_->name.cStr();
}

}
#else
#endif
