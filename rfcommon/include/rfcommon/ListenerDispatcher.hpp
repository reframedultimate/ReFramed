#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Vector.hpp"
#include <exception>

namespace rfcommon {

/*!
 * @brief Generic class for handling dispatching messages to listeners.
 * A listener (or better known as an **observer**) is an object which can
 * register itself to a dispatcher in order to receive notifications of
 * specific events by using callback functions.
 *
 * For this to work, three classes need to exist. The **Listener Interface**,
 * the **Listener** - which must inherit the listener interface and implement
 * its abstract methods - and a **dispatcher**.
 *
 * The dispatcher holds a list of objects inheriting the listener interface,
 * and provides functions through which the programmer can dispatch an event
 * to all registered objects.
 *
 * The following code example demonstrates this behaviour.
 * ```
 * struct ListenerInterface {
 *     virtual void printMessage(std::string message) = 0;
 * }
 *
 * struct Listener : public ListenerInterface {
 *     virtual void printMessage(std::string message)?{
 *         std::cout << "received: " << message << std::endl;
 *     }
 * }
 *
 * int main()?{
 *
 *     // instantiate the dispatcher object. The template argument specifies
 *     // the type of listener objects it should manage.
 *     ListenerDispatcher<ListenerInterface> dispatcher;
 *
 *     // instantiate some objects derived from the "ListenerInterface" class
 *     // and register them to the dispatcher object so they can receive
 *     // notifications
 *     Listener a, b;
 *     dispatcher.addListener(&a);
 *     dispatcher.addListener(&b);
 *
 *     // dispatch a message using the dispatcher. All registered objects
 *     // will receive this message
 *     dispatcher.dispatch(&ListenerInterface::printMessage, "hello world!");
 *
 *     return 0;
 * }
 * ```
 */
template <typename Listener>
class ListenerDispatcher
{
public:
    using ContainerType = SmallVector<Listener*, 4>;
    using Iterator = typename ContainerType::Iterator;
    using ConstIterator = typename ContainerType::ConstIterator;

    /*!
     * @brief Default constructor
     */
    ListenerDispatcher();

    /*!
     * @brief Default destructor
     */
    ~ListenerDispatcher();

    /*!
     * @brief Registers a listener to be notified on events.
     * The listener object must derive from the listener interface.
     * @note If the listener is already registered, this method will silently
     * fail.
     * @param listener A pointer to the listener object to register.
     * @param listenerName A globally unique identifier string for this
     * listener.
     */
    void addListener(Listener* listener);

    /*!
     * @brief Unregisters a listener by pointer
     * @note If the listener is not registered to begin with, this method will
     * silently fail.
     * @param listener A pointer to a listener to unregister
     */
    void removeListener(Listener* listener);

    /*!
     * @brief Dispatches a message to all listeners
     * @param func A pointer to a member function of the listener interface class.
     * For example:
     * @code &ListenerInterface::doThing @endcode
     * @param params Parameter list of values to be dispatched to the
     * listeners. These must map the function signature of the function declared
     * in the listener interface.
     */
    template <typename L, typename Ret, typename... Args, typename... Params>
    void dispatch(Ret (L::*func)(Args...), Params&&... params) const;

    template <typename L, typename Ret, typename... Args, typename... Params>
    void dispatchIgnore(Listener* ignore, Ret (L::*func)(Args...), Params&&... params) const;

    /*!
     * @brief Dispatches a message to all listeners
     * If any of the listeners return false, then this method will return false.
     * If all listeners return true, then this method will return true.
     * @note As soon as a listener returns false, this method will return. All
     * listeners that would have been notified afterwards are skipped.
     * @param func A pointer to a member function of the listener class.
     * For example:
     * @code &ListenerInterface::doThing @endcode
     * @param params Parameter list of values to be dispatched to the
     * listeners. These must map the function signature of the function declared
     * in the listener interface.
     */
    template <typename L, typename... Args, typename... Params>
    bool dispatchAndFindFalse(bool (L::*func)(Args...), Params&&... params) const;

private:
    ContainerType listeners_;

#ifndef NDEBUG
    mutable bool isDispatching_ = false;
#endif
};


// ----------------------------------------------------------------------------
template <class Listener>
ListenerDispatcher<Listener>::ListenerDispatcher()
{
}

// ----------------------------------------------------------------------------
template <class Listener>
ListenerDispatcher<Listener>::~ListenerDispatcher()
{
}

// ----------------------------------------------------------------------------
template <class Listener>
void ListenerDispatcher<Listener>::addListener(Listener* listener)
{
#ifndef NDEBUG
    if (isDispatching_)
        std::terminate();

    for (const auto& l : listeners_)
        if (l == listener)
            std::terminate();
#endif

    listeners_.push(listener);
}

// ----------------------------------------------------------------------------
template <typename Listener>
void ListenerDispatcher<Listener>::removeListener(Listener* listener)
{
#ifndef NDEBUG
    if (isDispatching_)
        std::terminate();
#endif

    for (auto it = listeners_.begin(); it != listeners_.end(); ++it)
        if (*it == listener)
        {
            listeners_.erase(it);
            return;
        }
}

// ----------------------------------------------------------------------------
template <typename Listener>
template <typename L, typename Ret, typename... Args, typename... Params>
void ListenerDispatcher<Listener>::dispatch(Ret(L::*func)(Args...), Params&&... params) const
{
#ifndef NDEBUG
    isDispatching_ = true;
#endif

    for(auto listener : listeners_)
        (listener->*func)(std::forward<Args>(params)...);

#ifndef NDEBUG
    isDispatching_ = false;
#endif
}

// ----------------------------------------------------------------------------
template <typename Listener>
template <typename L, typename Ret, typename... Args, typename... Params>
void ListenerDispatcher<Listener>::dispatchIgnore(Listener* ignore, Ret(L::*func)(Args...), Params&&... params) const
{
#ifndef NDEBUG
    isDispatching_ = true;
#endif

    for(auto listener : listeners_)
        if (listener != ignore)
            (listener->*func)(std::forward<Args>(params)...);

#ifndef NDEBUG
    isDispatching_ = false;
#endif
}

// ----------------------------------------------------------------------------
template <typename Listener>
template <typename L, typename... Args, typename... Params>
bool ListenerDispatcher<Listener>::dispatchAndFindFalse(bool (L::*func)(Args...), Params&&... params) const
{
#ifndef NDEBUG
    isDispatching_ = true;
#endif

    for(auto listener : listeners_)
        if ((listener->*func)(std::forward<Args>(params)...) == false)
            return false;
    return true;

#ifndef NDEBUG
    isDispatching_ = false;
#endif
}

}
