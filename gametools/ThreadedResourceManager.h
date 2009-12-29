#ifndef _THREADED_RESOURCE_MANAGER_H_
#define _THREADED_RESOURCE_MANAGER_H_

#include <vector>
#include "ResourceManager.h"
#include "ios_thread.h"
#include "ios_mutex.h"

template <typename T, typename K=std::string>
class ThreadedResourceManager
    : public ResourceManager<T, K>, ios_fc::Runnable
{
private:
    typedef typename std::map<K, ResourceHolder<T> * > ResourceMap;
    ResourceMap m_resources;
    std::vector<K> m_loadList;
    ios_fc::Thread m_backgroundThread;
    ios_fc::Mutex  m_loadListMutex, m_loadLockMutex, m_resourcesMutex;
protected:
    using ResourceManager<T, K>::m_factory;
public:
    ThreadedResourceManager(ResourceFactory<T, K> &factory)
        : ResourceManager<T, K>(factory), m_backgroundThread(this) {
        m_loadLockMutex.lock();
        m_backgroundThread.run();
    }
    virtual void cacheResource(const K &resourceKey) {
        ios_fc::Lock lock(m_loadListMutex);
        m_loadList.push_back(resourceKey);
        m_loadLockMutex.unlock();
    }
    virtual ResourceReference<T> getResource(const K &resourceKey)
    {
        ios_fc::Lock lockResources(m_resourcesMutex);
        typename ResourceMap::iterator resIter = m_resources.find(resourceKey);
        if (resIter == m_resources.end()) {
            T *res = m_factory.create(resourceKey);
            if (res == NULL)
                return ResourceReference<T>();
            ResourceHolder<T> *resHolder = new ResourceHolder<T>(res);
            m_resources[resourceKey] = resHolder;
            return ResourceReference<T>(resHolder);
        }
        else {
            return ResourceReference<T>(resIter->second);
        }
    }
    virtual void run() {
        m_loadLockMutex.lock();
        {
            std::vector<K> loadListCopy;
            {
                ios_fc::Lock lock(m_loadListMutex);
                loadListCopy = m_loadList;
                m_loadList.clear();
            }
            for (typename std::vector<K>::iterator iter = loadListCopy.begin() ;
                 iter != loadListCopy.end() ; iter++) {
                K &resourceKey = *iter;
                ios_fc::Lock lockResources(m_resourcesMutex);
                if (m_resources.find(resourceKey) != m_resources.end())
                    continue;
                T *res = m_factory.create(resourceKey);
                if (res == NULL)
                    continue;
                ResourceHolder<T> *resHolder = new ResourceHolder<T>(res);
                m_resources[resourceKey] = resHolder;
            }
        }
    }
};

#endif // _THREADED_RESOURCE_MANAGER_H_

