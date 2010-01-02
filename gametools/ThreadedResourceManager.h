#ifndef _THREADED_RESOURCE_MANAGER_H_
#define _THREADED_RESOURCE_MANAGER_H_

#include <vector>
#include "ResourceManager.h"
#include "ios_thread.h"
#include "ios_mutex.h"

template <typename T>
class ThreadedResourceHolder
    : public ResourceHolder<T>
{
public:
    ThreadedResourceHolder(T *res) : ResourceHolder<T>(res) {}
    ios_fc::Mutex m_resourceHolderMutex;
    T *getRes() const { return m_res; }
    void reset(T *res) { m_res = res; }
protected:
    using ResourceHolder<T>::m_res;
};

template <typename T, typename K=std::string>
class ThreadedResourceManager
    : public ResourceManager<T, K>, ios_fc::Runnable
{
private:
    typedef typename std::map<K, ThreadedResourceHolder<T> * > ResourceMap;
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
        m_backgroundThread.launch();
    }
    virtual void cacheResource(const K &resourceKey) {
        ios_fc::Lock lock(m_loadListMutex);
        m_loadList.push_back(resourceKey);
        m_loadLockMutex.unlock();
    }
    virtual ResourceReference<T> getResource(const K &resourceKey)
    {
        ThreadedResourceHolder<T> *resHolder = getResourceHolder(resourceKey);
        // Exclusive work on the ResourceHolder
        {
            ios_fc::Lock locCurResHolder(resHolder->m_resourceHolderMutex);
            if (resHolder->getRes() == NULL) {
                // Loading resource
                T *res = m_factory.create(resourceKey);
                if (res == NULL) {
                    return ResourceReference<T>();
                }
                resHolder->reset(res);
                return ResourceReference<T>(resHolder);
            }
            else {
                // Resource already loaded, returning
                return ResourceReference<T>(resHolder);
            }
        }
    }
    // Background resource loading thread
    virtual void run() {
        while (true) {
            m_loadLockMutex.lock();
            // Copying the loadList
            std::vector<K> loadListCopy;
            {
                ios_fc::Lock lock(m_loadListMutex);
                loadListCopy = m_loadList;
                m_loadList.clear();
            }
            // Loading all the resources from the loadList
            for (typename std::vector<K>::iterator iter = loadListCopy.begin() ;
                 iter != loadListCopy.end() ; iter++) {
                K &resourceKey = *iter;
                // Getting the ResourceHolder
                ThreadedResourceHolder<T> *resHolder = getResourceHolder(resourceKey);
                // Exclusive work on the ResourceHolder
                {
                    ios_fc::Lock locCurResHolder(resHolder->m_resourceHolderMutex);
                    if (resHolder->getRes() == NULL) {
                        // Loading resource
                        T *res = m_factory.create(resourceKey);
                        resHolder->reset(res);
                    }
                }
            }
        }
    }
private:
    // Checking wether there is an entry for resourceKey in m_resources.
    // If not, create one.
    ThreadedResourceHolder<T> *getResourceHolder(const K &resourceKey)
    {
        ThreadedResourceHolder<T> *resHolder;
        ios_fc::Lock lockResources(m_resourcesMutex);
        typename ResourceMap::iterator resIter = m_resources.find(resourceKey);
        if (resIter == m_resources.end()) {
            resHolder = new ThreadedResourceHolder<T>(NULL);
            m_resources[resourceKey] = resHolder;
        }
        else {
            resHolder = resIter->second;
        }
        return resHolder;
    }
};

#endif // _THREADED_RESOURCE_MANAGER_H_

