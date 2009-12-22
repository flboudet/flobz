#ifndef _RESOURCE_MANAGER_
#define _RESOURCE_MANAGER_

#include <string>
#include <map>

template <typename T>
class ResourceReference;

template <typename T>
class ResourceHolder
{
public:
    ResourceHolder(T *res) : m_res(res), m_numRefs(0) {}
private:
    T *m_res;
    int m_numRefs;
    friend class ResourceReference<T>;
};

template <typename T>
class ResourceReference
{
public:
    ResourceReference()
        : m_ownerResHolder(NULL) {}
    ResourceReference(ResourceHolder<T> *ownerResHolder)
        : m_ownerResHolder(ownerResHolder){
        if (m_ownerResHolder != NULL)
            m_ownerResHolder->m_numRefs++;
    }
    ~ResourceReference() {
        if (m_ownerResHolder != NULL)
            m_ownerResHolder->m_numRefs--;
    }
    operator T *() {
        if (m_ownerResHolder == NULL)
            return NULL;
        return m_ownerResHolder->m_res;
    }
    T *get() const {
        if (m_ownerResHolder == NULL)
            return NULL;
        return m_ownerResHolder->m_res;
    }
    bool empty() const { return m_ownerResHolder == NULL; }
private:
    ResourceHolder<T> *m_ownerResHolder;
};

template <typename T, typename K=std::string>
class ResourceFactory
{
public:
    virtual T *create(const K &resourceKey) = 0;
    virtual void destroy(T *res) = 0;
};

template <typename T, typename K=std::string>
class ResourceManager
{
public:
    ResourceManager(ResourceFactory<T> &factory)
        : m_factory(factory) {}
    virtual void cacheResource(const K &resourceKey) = 0;
    virtual ResourceReference<T> getResource(const K &resourceKey) = 0;
protected:
    ResourceFactory<T> &m_factory;
};

template <typename T, typename K=std::string>
class SimpleResourceManager : public ResourceManager<T, K>
{
private:
    typedef typename std::map<K, ResourceHolder<T> * > ResourceMap;
    ResourceMap m_resources;
protected:
    using ResourceManager<T>::m_factory;
public:
    SimpleResourceManager(ResourceFactory<T> &factory)
        : ResourceManager<T>(factory) {}
    virtual void cacheResource(const K &resourceKey) {}
    virtual ResourceReference<T> getResource(const K &resourceKey)
    {
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
};

#endif // _RESOURCE_MANAGER_
