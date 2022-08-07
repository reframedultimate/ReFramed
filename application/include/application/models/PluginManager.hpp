#pragma once

#include "rfcommon/PluginType.hpp"
#include <QString>
#include <QVector>
#include <QHash>

struct rfcommon_dynlib;
struct RFPluginFactory;
struct RFPluginInterface;
struct RFPluginFactoryInfo;

class QWidget;

namespace rfcommon {
    class Plugin;
    class AnalyzerPlugin;
    class VisualizerPlugin;
    class RealtimePlugin;
    class StandalonePlugin;
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
    PluginManager();
    ~PluginManager();

    /*!
     * \brief Attempts to load a shared library and registers all factories
     * in it if successful.
     * \param fileName The full path to the file to load.
     * \return True if it was successful, false if otherwise.
     */
    bool loadPlugin(const QString& fileName);

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

    rfcommon::AnalyzerPlugin* createAnalyzerModel(const QString& name);
    rfcommon::VisualizerPlugin* createVisualizerModel(const QString& name);
    rfcommon::RealtimePlugin* createRealtimeModel(const QString& name);
    rfcommon::StandalonePlugin* createStandaloneModel(const QString& name);

    rfcommon::Plugin* createModel(const QString& name, RFPluginType type);
    void destroyModel(rfcommon::Plugin* plugin);

private:
    QHash<QString, RFPluginFactory*> factories_;
    QVector<rfcommon_dynlib*> libraries_;
};

}
