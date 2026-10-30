//  Copyright 2026 Ewogijk
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef RUNEOS_EVENTBROKER_H
#define RUNEOS_EVENTBROKER_H

#include <KRE/Collections/Array.h>
#include <KRE/Collections/HashMap.h>
#include <KRE/Memory.h>
#include <KRE/String.h>
#include <KRE/Threading/CriticalSection.h>
#include <KRE/Threading/Spinlock.h>
#include <KRE/TypeTraits.h>

namespace Rune {

    /// @brief A type of event is identified by an unique static address.
    using TypeID = const void*;

    /// @brief The tag for a type, it will be uniquely allocated for each type T.
    /// @tparam T Type that should be tagged.
    template <class T>
    inline constexpr char TYPE_ID_TAG = 0;

    /// @brief Get the type ID of T.
    /// @tparam T Type whose tag address will be returned.
    /// @return The unique address of the tag for T.
    template <typename T>
    constexpr auto type_id() -> TypeID {
        return &TYPE_ID_TAG<T>;
    }

    /// @brief An event to be published by an event broker.
    ///
    ///
    /// Events used with post() must be trivially copyable and pointers should be avoided, because
    /// their behavior might not be defined in case the event is dispatched asynchronously.
    /// Imagine a UniquePointer: A raw memcpy would create a second instance of it, which provides
    /// two issues:
    ///     1. UniquePointer is no longer unique.
    ///     2. The original UniquePointer would free the memory when it goes out of scope and the
    ///         copy accesses illegal memory (or not if the original outlives the copy...)
    /// If necessary, pointers should point to static memory only.
    class Event {
        TypeID      m_type;
        const char* m_name;

      public:
        explicit Event(TypeID id, const char* name) : m_type(id), m_name(name) {}

        [[nodiscard]] auto type() const -> TypeID { return m_type; }

        [[nodiscard]] auto name() const -> const char* { return m_name; }
    };

    /// @brief Base class for events.
    /// @tparam Derived Type of the derived class.
    ///
    /// The base class uses CRTP to derive type id and name of the derived class. The derived class
    /// must declare its name as `static constexpr const char* NAME`.
    template <class Derived>
    class EventBase : public Event {
      public:
        EventBase() : Event(type_id<Derived>(), Derived::NAME) {}
    };

    /// @brief Statistics of a subscriber.
    struct SubscriberStats {
        String m_name     = "";
        U64    m_notified = 0;
    };

    /// @brief Statistics of an event and all subscribers of the event.
    struct EventStats {
        String                      m_event_name = "";
        LinkedList<SubscriberStats> m_subscribers;
    };

    /// @brief Statistics of the event pipe.
    struct EventPipeStats {
        size_t m_pending  = 0;
        size_t m_capacity = 0;
        U64    m_dropped  = 0;
    };

    /// @brief Called after an event was queued by post().
    ///
    /// The callback is called in the context of the publisher, which may be an interrupt context.
    /// It must be IRQ-safe, e.g. wake up a thread that calls dispatch_pending().
    using OnPending = void (*)();

    /// @brief A thread-safe event broker that allows subscribers to subscribe on specific events
    ///         and receive updates whenever the event gets published.
    ///
    /// A subscriber shall not publish or subscribe to events while it is running, this would lead
    /// to a deadlock or possibly undefined behavior.
    ///
    /// Events may be published synchronously:
    ///
    /// @code
    ///     broker.publish<MyEvent>({});
    /// @endcode
    ///
    /// This publishes the event immediately to all subscribers but it is not safe to use in
    /// interrupt context, because subscribers could use blocking calls.
    ///
    /// Events can also be published asynchronously, if running in interrupt context this is a must:
    ///
    /// @code
    ///     broker.post<MyEvent>({});
    /// @endcode
    ///
    /// This adds the event to an event pipe and where it stays until it is published by a
    /// background thread running outside of the interrupt context.
    class EventBroker {
      public:
        /// @brief Maximum number of events in the event pipe, must be a power of two.
        static constexpr size_t EVENT_PIPE_SIZE = 64;

        /// @brief Maximum size of an event in the pipe in bytes.
        static constexpr size_t EVENT_PIPE_MAX_EVENT_SIZE = 64;

        /// @brief Maximum alignment of an event in the pipe in bytes.
        static constexpr size_t EVENT_PIPE_MAX_EVENT_ALIGNMENT = 16;

      private:
        static_assert((EVENT_PIPE_SIZE & (EVENT_PIPE_SIZE - 1)) == 0,
                      "EVENT_PIPE_SIZE must be a power of two");

        struct Subscriber {
            String                       m_name;
            U64                          m_notified;
            Function<void(const Event&)> m_handler;
        };

        struct Topic {
            String                 m_event_name = "";
            LinkedList<Subscriber> m_subscribers;
        };

        HashMap<TypeID, Topic> m_topics;

        // A Spinlock instead of an IRQSpinlock because subscribers are run while holding the lock,
        // an IRQSpinlock would keep interrupts disabled for the whole duration of all subscribers.
        mutable Spinlock m_lock;

        /// @brief An event is stored as raw bytes, the event offset is the offset of the Event base
        ///         class within the payload.
        struct EventSlot {
            alignas(EVENT_PIPE_MAX_EVENT_ALIGNMENT) Array<U8, EVENT_PIPE_MAX_EVENT_SIZE> m_payload;
            size_t m_event_offset;
        };

        using EventPipe = Array<EventSlot, EVENT_PIPE_SIZE>;

        // The event pipe is accessed from interrupt context, it must only be accessed with the
        // IRQSpinlock held. The read and write cursors only grow, the slot index is the cursor
        // modulo EVENT_PIPE_SIZE.
        UniquePointer<EventPipe> m_event_pipe;
        size_t                   m_read    = 0;
        size_t                   m_write   = 0;
        U64                      m_dropped = 0;
        OnPending                m_on_pending;
        mutable IRQSpinlock      m_event_pipe_lock;

        static auto on_pending_none() -> void {}

        /// @brief Remove the oldest event from the event pipe.
        /// @param slot Receives a copy of the event slot.
        /// @return True: An event was removed, False: The pipe is empty.
        auto pop_event_pipe(EventSlot& slot) -> bool {
            CriticalSection _(m_event_pipe_lock);
            if (m_read == m_write) return false;

            slot = (*m_event_pipe)[m_read++ & (EVENT_PIPE_SIZE - 1)];
            return true;
        }

      public:
        EventBroker() : m_event_pipe(make_unique<EventPipe>()), m_on_pending(&on_pending_none) {}

        EventBroker(const EventBroker&)                    = delete;
        EventBroker(EventBroker&&)                         = delete;
        auto operator=(const EventBroker&) -> EventBroker& = delete;
        auto operator=(EventBroker&&) -> EventBroker&      = delete;

        /// @brief Get the statistics of all events that have at least one subscriber.
        /// @return A list of event statistics.
        [[nodiscard]] auto statistics() const -> LinkedList<EventStats> {
            CriticalSection        _(m_lock);
            LinkedList<EventStats> stats;
            for (const auto& entry : m_topics) {
                EventStats event_stats{
                    .m_event_name  = entry.value->m_event_name,
                    .m_subscribers = LinkedList<SubscriberStats>(),
                };
                for (auto& subscriber : entry.value->m_subscribers)
                    event_stats.m_subscribers.add_back(
                        {.m_name = subscriber.m_name, .m_notified = subscriber.m_notified});
                stats.add_back(event_stats);
            }
            return stats;
        }

        /// @brief Get the statistics of the event pipe.
        /// @return The event pipe statistics.
        [[nodiscard]] auto event_pipe_statistics() const -> EventPipeStats {
            CriticalSection _(m_event_pipe_lock);
            return {
                .m_pending  = m_write - m_read,
                .m_capacity = EVENT_PIPE_SIZE,
                .m_dropped  = m_dropped,
            };
        }

        /// @brief Set the function that is called whenever an event was queued by post().
        /// @param on_pending IRQ-safe callback function, nullptr removes the callback.
        auto set_on_pending(OnPending on_pending) -> void {
            CriticalSection _(m_event_pipe_lock);
            m_on_pending = on_pending != nullptr ? on_pending : &on_pending_none;
        }

        /// @brief Subscribe the call back function for events of the requested type.
        /// @tparam E Type of the event that will be subscribed.
        /// @param name       Name of the subscriber.
        /// @param subscriber Function that will be called when an event is published.
        template <class E>
        auto subscribe(const String& name, const Function<void(const E&)>& subscriber) -> void {
            CriticalSection _(m_lock);
            auto&           topic = m_topics[type_id<E>()];
            if (topic.m_event_name.is_empty()) topic.m_event_name = E::NAME;

            topic.m_subscribers.add_back({
                .m_name     = name,
                .m_notified = 0,
                .m_handler  = [subscriber](const Event& event) -> void {
                    // The static_cast is safe because the hashmap stores
                    // subscribers by the event type ID and only events of
                    // that ID will ever reach the subscriber.
                    subscriber(static_cast<const E&>(event));
                },
            });
        }

        /// @brief Notify all subscribers about the given event.
        /// @tparam E Type of the event that will be published.
        /// @param event Event about to be published.
        template <class E>
        auto publish(const E& event) -> void {
            CriticalSection _(m_lock);
            auto            maybe_topic = m_topics.find(event.type());
            if (maybe_topic == m_topics.end()) return;

            for (auto& subscriber : maybe_topic->value->m_subscribers) {
                subscriber.m_notified++;
                subscriber.m_handler(event);
            }
        }

        /// @brief Queue the event for later delivery by dispatch_pending().
        /// @tparam E Type of the event that will be published.
        /// @param event Event about to be published.
        /// @return True: The event was queued, False: The event pipe is full and the event was
        ///             dropped.
        ///
        /// This function is IRQ-safe: It does not allocate memory, does not run subscribers and
        /// only holds an IRQSpinlock while the event is copied into the event pipe.
        template <class E>
            requires TriviallyCopyable<E>
        auto post(const E& event) -> bool {
            static_assert(sizeof(E) <= EVENT_PIPE_MAX_EVENT_SIZE,
                          "Event exceeds EVENT_PIPE_MAX_EVENT_SIZE");
            static_assert(alignof(E) <= EVENT_PIPE_MAX_EVENT_ALIGNMENT,
                          "Event exceeds EVENT_PIPE_MAX_EVENT_ALIGNMENT");

            OnPending on_pending = nullptr;
            {
                CriticalSection _(m_event_pipe_lock);
                if (m_write - m_read == EVENT_PIPE_SIZE) {
                    m_dropped++;
                    return false;
                }

                auto& slot = (*m_event_pipe)[m_write++ & (EVENT_PIPE_SIZE - 1)];
                memcpy(slot.m_payload.data(), &event, sizeof(E));
                slot.m_event_offset = reinterpret_cast<const U8*>(static_cast<const Event*>(&event))
                                      - reinterpret_cast<const U8*>(&event);
                on_pending          = m_on_pending;
            }
            // Notify outside the event pipe lock, so on_pending does not run with the lock held
            on_pending();
            return true;
        }

        /// @brief Publish all events that were in the event pipe when this function was called.
        /// @return The number of published events.
        ///
        /// Events queued while dispatching are left for the next call.
        ///
        /// This function must not be called from interrupt context or by a subscriber.
        auto dispatch_pending() -> size_t {
            size_t pending = 0;
            {
                CriticalSection _(m_event_pipe_lock);
                pending = m_write - m_read;
            }

            size_t    dispatched = 0;
            EventSlot slot{};
            while (dispatched < pending && pop_event_pipe(slot)) {
                const auto* event =
                    reinterpret_cast<const Event*>(slot.m_payload.data() + slot.m_event_offset);
                publish(*event);
                dispatched++;
            }
            return dispatched;
        }
    };

    /// @brief The kernel-wide event broker. Events by kernel systems should be globally published
    ///         by this broker and if any other kernel system is interested in subscribing to events
    ///         it should do so in this broker.
    ///
    /// A note on global constructors: C++ does not define any order in which global objects are
    /// initialized, this means subscription should never be done in a global constructor because
    /// g_event_broker may not exist yet.
    extern EventBroker g_event_broker;
} // namespace Rune

#endif // RUNEOS_EVENTBROKER_H
