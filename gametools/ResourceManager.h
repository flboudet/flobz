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
        : m_ownerResHolder(ownerResHolder){ m_ownerResHolder->m_numRefs++; }
    ~ResourceReference() { m_ownerResHolder->m_numRefs--; }
    operator T *() {
        return m_ownerResHolder->m_res;
    }
private:
    ResourceHolder<T> *m_ownerResHolder;
};

template <typename T>
class ResourceFactory
{
public:
    virtual T *create(const char *resourcePath) = 0;
};

template <typename T>
class ResourceManager
{
public:
    ResourceManager(ResourceFactory<T> &factory)
        : m_factory(factory) {}
    virtual void cacheResource(const char *resourcePath) = 0;
    virtual ResourceReference<T> getResource(const char *resourcePath) = 0;
protected:
    ResourceFactory<T> &m_factory;
};

template <typename T>
class SimpleResourceManager : public ResourceManager<T>
{
private:
    typedef typename std::map<std::string, ResourceHolder<T> * > ResourceMap;
    ResourceMap m_resources;
protected:
    using ResourceManager<T>::m_factory;
public:
    SimpleResourceManager(ResourceFactory<T> &factory)
        : ResourceManager<T>(factory) {}
    virtual void cacheResource(const char *resourcePath) {}
    virtual ResourceReference<T> getResource(const char *resourcePath)
    {
        typename ResourceMap::iterator resIter = m_resources.find(resourcePath);
        if (resIter == m_resources.end()) {
            T *res = m_factory.create(resourcePath);
            ResourceHolder<T> *resHolder = new ResourceHolder<T>(res);
            m_resources[resourcePath] = resHolder;
            return ResourceReference<T>(resHolder);
        }
        else {
            return ResourceReference<T>(resIter->second);
        }
    }
};

#endif // _RESOURCE_MANAGER_