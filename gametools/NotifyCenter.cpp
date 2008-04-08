#include "NotifyCenter.h"
#include <stdio.h>

namespace gameui {
 
    NotifyCenter GlobalNotificationCenter;
    
    //
    // NotifyCenter
    //
    
    void NotifyCenter::addListener(String identifier, NotificationResponder * listener)
    {
        std::vector<NotificationResponder *> * listeners;
        
        // Get the listener list for this notification
        HashValue * listenersHash = get((const char *)identifier);
        
        // If no list available, create one
        if (listenersHash == NULL) 
        {
            listeners = new std::vector<NotificationResponder *>;
            if (listeners == NULL) 
            {
                fprintf(stderr, "No more memory, ignoring addListener to notification %s\n",(const char *)identifier);
                return;
            }
            put((const char *)identifier, (void *)listeners);
        }
        else listeners = (std::vector<NotificationResponder *> *)listenersHash->ptr;
        
        // add the listener (even if already present)
        listeners->push_back(listener);
    }
    
    void NotifyCenter::removeListener(String identifier, NotificationResponder * listener)
    {
        std::vector<NotificationResponder *> * listeners;
        
       // Get the listener list for this notification
        HashValue * listenersHash = get((const char *)identifier);
        
        // If no list available, warn and ignore
        if (listenersHash == NULL) 
        {
            fprintf(stderr, "Ooops, asked to remove notification listener for notification %s\n",(const char *)identifier);
            fprintf(stderr, "But the notification wasn't registered for anyone, ignoring...\n");
            return;
        }
        else listeners = (std::vector<NotificationResponder *> *)listenersHash->ptr;
        
        // Look for the listener and remove it, or warn and ignore if not found
        std::vector<NotificationResponder *>::iterator listenerIterator;
        listenerIterator = find(listeners->begin(),listeners->end(),listener);
        if (listenerIterator==listeners->end())
        {
            fprintf(stderr, "Ooops, asked to remove notification listener for notification %s\n",(const char *)identifier);
            fprintf(stderr, "But the listener wasn't registered for this notification, ignoring...\n");
            return;
        }
        listeners->erase(listenerIterator);
        
    }
    
    void NotifyCenter::notify(String identifier, void * context)
    {
        // Get the listener list for this notification
        HashValue * listenersHash = get((const char *)identifier);
        
        // If no list available, do nothing
        if (listenersHash == NULL) return;

        // else, iterate for every listener
        std::vector<NotificationResponder *> * listeners = (std::vector<NotificationResponder *> *)listenersHash->ptr;
        std::vector<NotificationResponder *>::iterator listenerIterator;
        for (listenerIterator = listeners->begin(); listenerIterator != listeners->end(); listenerIterator++)
        {
            (*listenerIterator)->notificationOccured(identifier, context);
        }
    }
    
}