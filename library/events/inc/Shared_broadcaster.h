#ifndef GUARD_EVENTS_SHARED_BROADCASTER
#define GUARD_EVENTS_SHARED_BROADCASTER

#include <vector>
#include <queue>
#include <algorithm>
#include <memory>

#include "Listener.h"
#include "Service.h"

namespace events {
    template < typename Event_type >
    class Shared_broadcaster : public Listener<Event_type>, public Service<Event_type> {
        public:
            virtual ~Shared_broadcaster<Event_type> () = default;

            virtual void subscribe (Listener<Event_type>* listener);
            virtual void de_subscribe (const Listener<Event_type>* listener);

            virtual void handle_event (Event_type& event);

        protected:
            std::vector<std::shared_ptr<Listener<Event_type>>> subscriptions;
            std::queue<const Listener<Event_type>*> de_subscriptions;
            bool lock_subscriptions = false;
    };

    template < typename T >
    void Shared_broadcaster<T>::subscribe (Listener<T>* listener) {
        subscriptions.push_back(std::shared_ptr<Listener<T>>(listener));
    }

    template < typename T >
    void Shared_broadcaster<T>::de_subscribe (const Listener<T>* listener) {
        if (!lock_subscriptions) {
            subscriptions.erase(std::remove_if(subscriptions.begin(), subscriptions.end(),
                        [&](std::shared_ptr<Listener<T>> l) {
                        return l.get() == listener;
                        }), subscriptions.end());
        } else {
            de_subscriptions.push(listener);
        }
    }

    template < typename T >
    void Shared_broadcaster<T>::handle_event (T& event) {
        lock_subscriptions = true;
        std::for_each(subscriptions.begin(), subscriptions.end(),
                [&](std::shared_ptr<Listener<T>> listener) { listener->handle_event(event); });
        lock_subscriptions = false;
        while (!de_subscriptions.empty()) {
            de_subscribe(de_subscriptions.front());
            de_subscriptions.pop();
        }
    }

}

#endif // GUARD_EVENTS_SHARED_BROADCASTER
