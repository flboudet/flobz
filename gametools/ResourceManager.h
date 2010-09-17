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
    virtual ~ResourceHolder() {}
    int getNumRefs() const { return m_numRefs; }
    T *getResource() const { return m_res; }
protected:
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
    ResourceReference(const ResourceReference<T> &res)
        : m_ownerResHolder(res.m_ownerResHolder){
        if (m_ownerResHolder != NULL)
            m_ownerResHolder->m_numRefs++;
    }
    ~ResourceReference() {
        if (m_ownerResHolder != NULL)
            m_ownerResHolder->m_numRefs--;
    }
    ResourceReference<T> & operator = (const ResourceReference<T> &res) {
        if (m_ownerResHolder != NULL)
            m_ownerResHolder->m_numRefs--;
        m_ownerResHolder = res.m_ownerResHolder;
        if (m_ownerResHolder != NULL)
            m_ownerResHolder->m_numRefs++;
        return *this;
    }
    operator T *() const {
        if (m_ownerResHolder == NULL)
            return NULL;
        return m_ownerResHolder->m_res;
    }
    T * operator ->()  const {
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
    ResourceManager(ResourceFactory<T, K> &factory)
        : m_factory(factory) {}
    virtual ~ResourceManager() {}
    virtual void cacheResource(const K &resourceKey) = 0;
    virtual ResourceReference<T> getResource(const K &resourceKey) = 0;
    virtual void freeUnusedResources() = 0;
protected:
    ResourceFactory<T, K> &m_factory;
};

template <typename T, typename K=std::string>
class SimpleResourceManager : public ResourceManager<T, K>
{
private:
    typedef typename std::map<K, ResourceHolder<T> * > ResourceMap;
    ResourceMap m_resources;
protected:
    using ResourceManager<T, K>::m_factory;
public:
    SimpleResourceManager(ResourceFactory<T, K> &factory)
        : ResourceManager<T, K>(factory) {}
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
    virtual void freeUnusedResources()
    {
        typedef std::vector<typename ResourceMap::iterator> ResourceMapIteratorVector;
        ResourceMapIteratorVector entriesToErase;
        for (typename ResourceMap::iterator resIter = m_resources.begin() ;
             resIter != m_resources.end() ; resIter++) {
            ResourceHolder<T> *resHolder = resIter->second;
            if (resHolder->getNumRefs() == 0) {
                m_factory.destroy(resHolder->getResource());
                delete resHolder;
                entriesToErase.push_back(resIter);
            }
        }
        for (typename ResourceMapIteratorVector::iterator iter = entriesToErase.begin() ;
             iter != entriesToErase.end() ; iter++) {
            m_resources.erase(*iter);
        }
    }
};

#endif // _RESOURCE_MANAGER_
