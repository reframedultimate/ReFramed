#pragma once

#include "rfcommon/PluginType.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/SharedLibrary.hpp"
#include "rfcommon/Vector.hpp"
#include <QString>
#include <QVector>
#include <QSet>

struct rfcommon_dynlib;
struct RFPluginFactory;
struct RFPluginInterface;
struct RFPluginFactoryInfo;

class QWidget;

namespace rfcommon {
    class Hash40Strings;
    class Plugin;
    class AnalyzerPlugin;
    class VisualizerPlugin;
    class RealtimePlugin;
    class StandalonePlugin;
    class UserMotionLabels;
}

namespace rfapp {

class Protocol;

/*!
 * \brief Responsible for loading .so/.dll files and registering factories
 * therein. Models and Views exposed through those factories can be instantiated
 * through this class as well.
 */
class PluginManager
{
public:
    PluginManager(rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings);
    ~PluginManager();

    /*!
     * \brief Scans for DLLs and updates the internal list of plugins.
     */
    void scanForPlugins();

    /*!
     * \brief Attempts to load a shared library and registers all factories
     * in it if successful.
     * \param fileName The full path to the file to load.
     * \return True if it was successful, false if otherwise.
     */
    bool loadInterface(const QString& fileName);

    /*!
     * \brief Returns a list of all factory names available, filtered by type.
     * These can be used to instantiate models/views using one of the create*()
     * methods.
     */
    QVector<QString> availableFactoryNames(RFPluginType type) const;

    /*!
     * \brief Returns a pointer to the info structure in the loaded shared
     * library, giving the name, author, description, etc. of the plugin.
     */
    const RFPluginFactoryInfo* getFactoryInfo(const QString& name) const;

    rfcommon::Plugin* create(const QString& name);
    void destroy(rfcommon::Plugin* plugin);

private:
    rfcommon::Reference<rfcommon::UserMotionLabels> userLabels_;
    rfcommon::Reference<rfcommon::Hash40Strings> hash40Strings_;

    struct LoadedPlugin
    {
        LoadedPlugin(const char* fileName);
        ~LoadedPlugin();
        LoadedPlugin(const LoadedPlugin& other) = delete;  // We need to move-construct SharedLibrary or it will unload libraries every time vector reallocates
        LoadedPlugin(LoadedPlugin&& other);

        rfcommon::SharedLibrary library;
        RFPluginInterface* iface = nullptr;
        bool started = false;
    };
    rfcommon::Vector<LoadedPlugin> plugins_;
    QSet<QString> factoryNames_;
};

}
