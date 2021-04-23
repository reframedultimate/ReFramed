#include "application/models/DataSetBackgroundLoader.hpp"
#include "application/models/RecordingGroup.hpp"
#include "uh/SavedRecording.hpp"
#include <QRunnable>
#include <QThreadPool>

namespace uhapp {

class DataSetBackgroundLoaderWorker : public QRunnable
{
public:
    DataSetBackgroundLoaderWorker(QMutex* mutex,
                                  QWaitCondition* cond,
                                  QQueue<DataSetBackgroundLoader::OutData>* in,
                                  QQueue<DataSetBackgroundLoader::InData>* out,
                                  int* activeWorkers)
        : mutex_(mutex)
        , cond_(cond)
        , in_(in)
        , out_(out)
        , activeWorkers_(activeWorkers)
    {
    }

signals:
    void recordingLoaded(uh::Recording* recording);

private:
    void run() override
    {
        mutex_->lock();
        while (true)
        {
            if (in_->isEmpty())
                break;

            auto info = in_->dequeue();
            mutex_->unlock();
                uh::Recording* recording = uh::SavedRecording::load(info.recordingFile.absoluteFilePath().toStdString());
            mutex_->lock();

            if (recording)
                out_->enqueue({recording, info.group});
        }

        // The reading thread (which dequeues our output queue) will want to know
        // when all recordings were loaded, which will be the case when the
        // worker count drops to 0
        (*activeWorkers_)--;
        if (*activeWorkers_ == 0)
            cond_->wakeOne();

        mutex_->unlock();
    }

private:
    QMutex* mutex_;
    QWaitCondition* cond_;
    QQueue<DataSetBackgroundLoader::OutData>* in_;
    QQueue<DataSetBackgroundLoader::InData>* out_;
    int* activeWorkers_;
};

// ----------------------------------------------------------------------------
DataSetBackgroundLoader::DataSetBackgroundLoader(QObject* parent)
    : QThread(parent)
{
    connect(this, &DataSetBackgroundLoader::_dataSetLoaded,
            this, &DataSetBackgroundLoader::onDataSetLoaded);

    start();
}

// ----------------------------------------------------------------------------
DataSetBackgroundLoader::~DataSetBackgroundLoader()
{
    cancelAll();
    mutex_.lock();
        requestShutdown_ = true;
        cond_.wakeOne();
    mutex_.unlock();

    wait();
}

// ----------------------------------------------------------------------------
void DataSetBackgroundLoader::loadGroup(RecordingGroup* group)
{
    mutex_.lock();
        dataSets_.emplace(group, new uh::DataSet);
        for (const auto& fileInfo : group->absFilePathList())
            out_.enqueue({fileInfo, group});

        while (activeWorkers_ < QThread::idealThreadCount())
        {
            DataSetBackgroundLoaderWorker* worker = new DataSetBackgroundLoaderWorker(
                &mutex_, &cond_, &out_, &in_, &activeWorkers_
            );

            QThreadPool::globalInstance()->start(worker);
            activeWorkers_++;
        }
    mutex_.unlock();
}

// ----------------------------------------------------------------------------
void DataSetBackgroundLoader::cancelGroup(RecordingGroup* group)
{
    mutex_.lock();
        // Delete all pending recording file names in the output queue that
        // originated from the specified recording group so the workers don't
        // load any more of them
        auto it = out_.begin();
        while (it != out_.end())
        {
            if (it->group == group)
                it = out_.erase(it);
            else
                ++it;
        }

        // Delete the dataset so that our reading thread won't append any more
        // recordings to it. The remaining data in the input queue will be
        // dequeued by the reading thread so don' thave to care about it here
        dataSets_.erase(group);
    mutex_.unlock();
}

// ----------------------------------------------------------------------------
void DataSetBackgroundLoader::cancelAll()
{
    mutex_.lock();
        out_.clear();
        dataSets_.clear();
    mutex_.unlock();
}

// ----------------------------------------------------------------------------
void DataSetBackgroundLoader::onDataSetLoaded(RecordingGroup* group)
{
    uh::Reference<uh::DataSet> ds;
    mutex_.lock();
        auto it = dataSets_.find(group);
        if (it != dataSets_.end())
        {
            ds = it->second;
            dataSets_.erase(it);
        }
    mutex_.unlock();

    if (ds.notNull())
        dispatcher.dispatch(&DataSetBackgroundLoaderListener::onDataSetBackgroundLoaderDataSetLoaded, group, ds);
}

// ----------------------------------------------------------------------------
void DataSetBackgroundLoader::run()
{
    mutex_.lock();
    while (true)
    {
        cond_.wait(&mutex_);

        if (requestShutdown_)
            break;

        while (!in_.isEmpty())
        {
            InData data = in_.dequeue();
            auto it = dataSets_.find(data.group);
            if (it != dataSets_.end())
                it->second->appendRecording(data.recording);
        }

        if (in_.isEmpty() && activeWorkers_ == 0)
        {
            for (const auto& [group, dataSet] : dataSets_)
                emit _dataSetLoaded(group);
        }
    }
    mutex_.unlock();
}

}
