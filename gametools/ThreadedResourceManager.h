#ifndef _THREADED_RESOURCE_MANAGER_H_
#define _THREADED_RESOURCE_MANAGER_H_

#include <vector>
#include "ResourceManager.h"
#include "ios_thread.h"
#include "ios_mutex.h"

template <typename T>
class ThreadedResourceManager : public ResourceManager<T>, ios_fc::Runnable
{
private:
    typedef typename std::map<std::string, ResourceHolder<T> * > ResourceMap;
    ResourceMap m_resources;
    std::vector<std::string> m_loadList;
    ios_fc::Thread m_backgroundThread;
    ios_fc::Mutex  m_loadListMutex, m_loadLockMutex, m_resourcesMutex;
protected:
    using ResourceManager<T>::m_factory;
public:
    ThreadedResourceManager(ResourceFactory<T> &factory)
        : ResourceManager<T>(factory), m_backgroundThread(this) {
        m_loadLockMutex.lock();
        m_backgroundThread.run();
    }
    virtual void cacheResource(const char *resourcePath) {
        ios_fc::Lock lock(m_loadListMutex);
        m_loadList.push_back(resourcePath);
        m_loadLockMutex.unlock();
    }
    virtual ResourceReference<T> getResource(const char *resourcePath)
    {
        typename ResourceMap::iterator resIter = m_resources.find(resourcePath);
        if (resIter == m_resources.end()) {
            T *res = m_factory.create(resourcePath);
            if (res == NULL)
                return ResourceReference<T>();
            ResourceHolder<T> *resHolder = new ResourceHolder<T>(res);
            m_resources[resourcePath] = resHolder;
            return ResourceReference<T>(resHolder);
        }
        else {
            return ResourceReference<T>(resIter->second);
        }
    }
    virtual void run() {
        m_loadLockMutex.lock();
        {
            ios_fc::Lock lock(m_loadListMutex);
            for (std::vector<std::string>::iterator iter = m_loadList.begin() ;
                 iter != m_loadList.end() ; iter++) {
                std::string &resourcePath = *iter;
                ios_fc::Lock lockResources(m_resourcesMutex);
                if (m_resources.find(resourcePath) != m_resources.end())
                    continue;
                T *res = m_factory.create(resourcePath.c_str());
                if (res == NULL)
                    continue;
                ResourceHolder<T> *resHolder = new ResourceHolder<T>(res);
                m_resources[resourcePath] = resHolder;
            }
            m_loadList.clear();
        }
    }
};

#endif // _THREADED_RESOURCE_MANAGER_H_

