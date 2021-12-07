#pragma once

#include "uh/PluginType.hpp"
#include <QString>
#include <QVector>
#include <QHash>

struct uh_dynlib;
struct UHPluginFactory;
struct UHPluginInterface;
struct UHPluginFactoryInfo;

class QWidget;

namespace uh {
    class Plugin;
    class AnalyzerPlugin;
    class VisualizerPlugin;
    class RealtimePlugin;
    class StandalonePlugin;
}

namespace uhapp {

class Protocol;

/*!
 * \brief Responsible for loading .so/.dll files and registering factories
 * therein. Models and Views exposed through those factories can be instantiated
 * through this class as well.
 */
class PluginManager
{
public:
    PluginManager(Protocol* protocol);
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
    QVector<QString> availableFactoryNames(UHPluginType type) const;

    /*!
     * \brief Returns a pointer to the info structure in the loaded shared
     * library, giving the name, author, description, etc. of the plugin.
     */
    const UHPluginFactoryInfo* getFactoryInfo(const QString& name) const;

    uh::AnalyzerPlugin* createAnalyzerModel(const QString& name);
    uh::VisualizerPlugin* createVisualizerModel(const QString& name);
    uh::RealtimePlugin* createRealtimeModel(const QString& name);
    uh::StandalonePlugin* createStandaloneModel(const QString& name);

    uh::Plugin* createModel(const QString& name, UHPluginType type);
    void destroyModel(const QString& name, uh::Plugin* plugin);

private:
    Protocol* protocol_;
    QHash<QString, UHPluginFactory*> factories_;
    QVector<uh_dynlib*> libraries_;
};

}
