
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

#ifndef RUNEOS_EVENTBROKERTEST_H
#define RUNEOS_EVENTBROKERTEST_H

#include <KRE/EventBroker.h>

#include <Test/Heimdall/Heimdall.h>

using namespace Rune;

namespace EventBrokerTest {
    class EventA : public EventBase<EventA> {
      public:
        static constexpr const char* NAME = "EventA";

        int m_value;

        explicit EventA(int value) : m_value(value) {}
    };

    class EventB : public EventBase<EventB> {
      public:
        static constexpr const char* NAME = "EventB";

        int m_value;

        explicit EventB(int value) : m_value(value) {}
    };

    static_assert(is_trivially_copyable<Event>::value);
    static_assert(is_trivially_copyable<EventA>::value);
} // namespace EventBrokerTest

using namespace EventBrokerTest;

TEST("type_id - Same type has the same ID",
     "EventBroker"){REQUIRE((type_id<EventA>() == type_id<EventA>()))}

TEST("type_id - Different types have different IDs",
     "EventBroker"){REQUIRE((type_id<EventA>() != type_id<EventB>()))}

TEST("EventBase - Event type is the type ID of the derived class", "EventBroker") {
    EventA event(1);
    REQUIRE((event.type() == type_id<EventA>()))
    REQUIRE((event.type() != type_id<EventB>()))
}

TEST("EventBase - Event name is the name of the derived class", "EventBroker") {
    EventA event(1);
    REQUIRE((String(event.name()) == "EventA"))
}

TEST("publish - No subscribers", "EventBroker") {
    EventBroker broker;
    broker.publish(EventA(1));
    REQUIRE(true)
}

TEST("publish - Subscriber receives the event data", "EventBroker") {
    EventBroker broker;
    int         received = 0;
    broker.subscribe<EventA>("Receiver", [&received](const EventA& event) -> void {
        received = event.m_value;
    });

    broker.publish(EventA(42)); // NOLINT
    REQUIRE(received == 42)
}

TEST("publish - Subscriber is notified on every publish", "EventBroker") {
    EventBroker broker;
    int         notified = 0;
    broker.subscribe<EventA>("Counter", [&notified](const EventA& event) -> void {
        SILENCE_UNUSED(event)
        notified++;
    });

    broker.publish(EventA(1));
    broker.publish(EventA(2));
    broker.publish(EventA(3));
    REQUIRE(notified == 3)
}

TEST("publish - All subscribers are notified in subscription order", "EventBroker") {
    EventBroker   broker;
    Array<int, 3> order = {0, 0, 0};
    int           calls = 0;
    broker.subscribe<EventA>("First", [&order, &calls](const EventA& event) -> void {
        SILENCE_UNUSED(event)
        order[calls++] = 1;
    });
    broker.subscribe<EventA>("Second", [&order, &calls](const EventA& event) -> void {
        SILENCE_UNUSED(event)
        order[calls++] = 2;
    });
    broker.subscribe<EventA>("Third", [&order, &calls](const EventA& event) -> void {
        SILENCE_UNUSED(event)
        order[calls++] = 3;
    });

    broker.publish(EventA(1));
    REQUIRE(calls == 3)
    REQUIRE(order[0] == 1)
    REQUIRE(order[1] == 2)
    REQUIRE(order[2] == 3)
}

TEST("publish - Subscribers of other event types are not notified", "EventBroker") {
    EventBroker broker;
    int         received_a = 0;
    int         received_b = 0;
    broker.subscribe<EventA>("ReceiverA", [&received_a](const EventA& event) -> void {
        received_a = event.m_value;
    });
    broker.subscribe<EventB>("ReceiverB", [&received_b](const EventB& event) -> void {
        received_b = event.m_value;
    });

    broker.publish(EventA(1));
    REQUIRE(received_a == 1)
    REQUIRE(received_b == 0)

    broker.publish(EventB(2));
    REQUIRE(received_a == 1)
    REQUIRE(received_b == 2)
}

TEST("publish - Event published as base class reaches the derived subscriber", "EventBroker") {
    EventBroker broker;
    int         received = 0;
    broker.subscribe<EventA>("Receiver", [&received](const EventA& event) -> void {
        received = event.m_value;
    });

    EventA       event(7); // NOLINT
    const Event& base = event;
    broker.publish(base);
    REQUIRE(received == 7)
}

namespace EventBrokerTest {
    /// @brief Find the statistics of an event by name, the order of event statistics is undefined.
    auto find_event_stats(const LinkedList<EventStats>& stats, const String& event_name)
        -> const EventStats* {
        for (auto& event_stats : stats) {
            if (event_stats.m_event_name == event_name) return &event_stats;
        }
        return nullptr;
    }
} // namespace EventBrokerTest

TEST("statistics - No subscribers", "EventBroker") {
    EventBroker broker;
    REQUIRE(broker.statistics().size() == 0U)
}

TEST("statistics - Event and subscriber names", "EventBroker") {
    EventBroker broker;
    broker.subscribe<EventA>("First", [](const EventA& event) -> void { SILENCE_UNUSED(event) });
    broker.subscribe<EventA>("Second", [](const EventA& event) -> void { SILENCE_UNUSED(event) });

    auto stats = broker.statistics();
    REQUIRE(stats.size() == 1U)
    REQUIRE((stats[0].m_event_name == "EventA"))
    REQUIRE(stats[0].m_subscribers.size() == 2U)
    REQUIRE((stats[0].m_subscribers[0].m_name == "First"))
    REQUIRE((stats[0].m_subscribers[1].m_name == "Second"))
}

TEST("statistics - Subscribers are not notified before publish", "EventBroker") {
    EventBroker broker;
    broker.subscribe<EventA>("Receiver", [](const EventA& event) -> void { SILENCE_UNUSED(event) });

    auto stats = broker.statistics();
    REQUIRE(stats[0].m_subscribers[0].m_notified == 0U)
}

TEST("statistics - Notified counter is incremented on every publish", "EventBroker") {
    EventBroker broker;
    broker.subscribe<EventA>("First", [](const EventA& event) -> void { SILENCE_UNUSED(event) });
    broker.subscribe<EventA>("Second", [](const EventA& event) -> void { SILENCE_UNUSED(event) });

    broker.publish(EventA(1));
    broker.publish(EventA(2));
    broker.publish(EventA(3));

    auto stats = broker.statistics();
    REQUIRE(stats[0].m_subscribers[0].m_notified == 3U)
    REQUIRE(stats[0].m_subscribers[1].m_notified == 3U)
}

TEST("statistics - Notified counters are tracked per event", "EventBroker") {
    EventBroker broker;
    broker.subscribe<EventA>("ReceiverA",
                             [](const EventA& event) -> void { SILENCE_UNUSED(event) });
    broker.subscribe<EventB>("ReceiverB",
                             [](const EventB& event) -> void { SILENCE_UNUSED(event) });

    broker.publish(EventA(1));
    broker.publish(EventA(2));
    broker.publish(EventB(3));

    auto stats = broker.statistics();
    REQUIRE(stats.size() == 2U)

    const auto* stats_a = find_event_stats(stats, "EventA");
    REQUIRE((stats_a != nullptr))
    REQUIRE((stats_a->m_subscribers[0].m_name == "ReceiverA"))
    REQUIRE(stats_a->m_subscribers[0].m_notified == 2U)

    const auto* stats_b = find_event_stats(stats, "EventB");
    REQUIRE((stats_b != nullptr))
    REQUIRE((stats_b->m_subscribers[0].m_name == "ReceiverB"))
    REQUIRE(stats_b->m_subscribers[0].m_notified == 1U)
}

TEST("statistics - Publish without subscribers creates no statistics", "EventBroker") {
    EventBroker broker;
    broker.subscribe<EventA>("Receiver", [](const EventA& event) -> void { SILENCE_UNUSED(event) });

    broker.publish(EventB(1));

    auto stats = broker.statistics();
    REQUIRE(stats.size() == 1U)
    REQUIRE((find_event_stats(stats, "EventB") == nullptr))
}

TEST("statistics - Statistics are a snapshot", "EventBroker") {
    EventBroker broker;
    broker.subscribe<EventA>("Receiver", [](const EventA& event) -> void { SILENCE_UNUSED(event) });

    broker.publish(EventA(1));
    auto stats = broker.statistics();
    broker.publish(EventA(2));

    REQUIRE(stats[0].m_subscribers[0].m_notified == 1U)
    REQUIRE(broker.statistics()[0].m_subscribers[0].m_notified == 2U)
}

namespace EventBrokerTest {
    int g_on_pending_calls = 0;

    auto count_on_pending_calls() -> void { g_on_pending_calls++; }
} // namespace EventBrokerTest

TEST("dispatch_pending - Empty event pipe", "EventBroker") {
    EventBroker broker;
    REQUIRE(broker.dispatch_pending() == 0U)
}

TEST("post - Subscriber is not notified before dispatch", "EventBroker") {
    EventBroker broker;
    int         received = 0;
    broker.subscribe<EventA>("Receiver", [&received](const EventA& event) -> void {
        received = event.m_value;
    });

    REQUIRE(broker.post(EventA(42))) // NOLINT
    REQUIRE(received == 0)
    REQUIRE(broker.event_pipe_statistics().m_pending == 1U)
}

TEST("dispatch_pending - Subscriber receives the event data", "EventBroker") {
    EventBroker broker;
    int         received = 0;
    broker.subscribe<EventA>("Receiver", [&received](const EventA& event) -> void {
        received = event.m_value;
    });

    broker.post(EventA(42)); // NOLINT
    REQUIRE(broker.dispatch_pending() == 1U)
    REQUIRE(received == 42)
    REQUIRE(broker.event_pipe_statistics().m_pending == 0U)
}

TEST("dispatch_pending - Events are dispatched in publish order", "EventBroker") {
    EventBroker   broker;
    Array<int, 3> order = {0, 0, 0};
    int           calls = 0;
    broker.subscribe<EventA>("Receiver", [&order, &calls](const EventA& event) -> void {
        order[calls++] = event.m_value;
    });

    broker.post(EventA(1));
    broker.post(EventA(2));
    broker.post(EventA(3));
    REQUIRE(broker.dispatch_pending() == 3U)
    REQUIRE(order[0] == 1)
    REQUIRE(order[1] == 2)
    REQUIRE(order[2] == 3)
}

TEST("dispatch_pending - Events are routed by their type", "EventBroker") {
    EventBroker broker;
    int         received_a = 0;
    int         received_b = 0;
    broker.subscribe<EventA>("ReceiverA", [&received_a](const EventA& event) -> void {
        received_a = event.m_value;
    });
    broker.subscribe<EventB>("ReceiverB", [&received_b](const EventB& event) -> void {
        received_b = event.m_value;
    });

    broker.post(EventA(1));
    broker.post(EventB(2));
    REQUIRE(broker.dispatch_pending() == 2U)
    REQUIRE(received_a == 1)
    REQUIRE(received_b == 2)
}

TEST("dispatch_pending - Notified counter is incremented on dispatch", "EventBroker") {
    EventBroker broker;
    broker.subscribe<EventA>("Receiver", [](const EventA& event) -> void { SILENCE_UNUSED(event) });

    broker.post(EventA(1));
    REQUIRE(broker.statistics()[0].m_subscribers[0].m_notified == 0U)

    broker.dispatch_pending();
    REQUIRE(broker.statistics()[0].m_subscribers[0].m_notified == 1U)
}

TEST("post - Full event pipe drops the event", "EventBroker") {
    EventBroker broker;
    int         notified = 0;
    broker.subscribe<EventA>("Counter", [&notified](const EventA& event) -> void {
        SILENCE_UNUSED(event)
        notified++;
    });

    for (size_t i = 0; i < EventBroker::EVENT_PIPE_SIZE; i++) {
        REQUIRE(broker.post(EventA(static_cast<int>(i))))
    }
    REQUIRE(!broker.post(EventA(-1)))

    auto stats = broker.event_pipe_statistics();
    REQUIRE(stats.m_pending == EventBroker::EVENT_PIPE_SIZE)
    REQUIRE(stats.m_capacity == EventBroker::EVENT_PIPE_SIZE)
    REQUIRE(stats.m_dropped == 1U)

    REQUIRE(broker.dispatch_pending() == EventBroker::EVENT_PIPE_SIZE)
    REQUIRE(notified == static_cast<int>(EventBroker::EVENT_PIPE_SIZE))
    REQUIRE(broker.post(EventA(1)))
}

TEST("dispatch_pending - Event pipe wraps around", "EventBroker") {
    EventBroker broker;
    int         expected  = 0;
    bool        in_order  = true;
    int         delivered = 0;
    broker.subscribe<EventA>("Receiver",
                             [&expected, &in_order, &delivered](const EventA& event) -> void {
                                 if (event.m_value != expected) in_order = false;
                                 expected++;
                                 delivered++;
                             });

    // Three rounds of 3/4 of the event pipe size move the cursors past the end of the pipe
    constexpr int ROUND_SIZE = static_cast<int>(EventBroker::EVENT_PIPE_SIZE * 3 / 4);
    int           next       = 0;
    for (int round = 0; round < 3; round++) {
        for (int i = 0; i < ROUND_SIZE; i++) broker.post(EventA(next++));
        REQUIRE(broker.dispatch_pending() == static_cast<size_t>(ROUND_SIZE))
    }
    REQUIRE(delivered == 3 * ROUND_SIZE)
    REQUIRE(in_order)
}

TEST("set_on_pending - OnPending is called for every queued event", "EventBroker") {
    EventBroker broker;
    g_on_pending_calls = 0;
    broker.set_on_pending(&count_on_pending_calls);

    broker.post(EventA(1));
    broker.post(EventB(2));
    REQUIRE(g_on_pending_calls == 2)
}

TEST("set_on_pending - OnPending is not called for dropped events", "EventBroker") {
    EventBroker broker;
    for (size_t i = 0; i < EventBroker::EVENT_PIPE_SIZE; i++) {
        broker.post(EventA(1));
    }

    g_on_pending_calls = 0;
    broker.set_on_pending(&count_on_pending_calls);
    broker.post(EventA(1));
    REQUIRE(g_on_pending_calls == 0)
}

TEST("set_on_pending - nullptr removes OnPending", "EventBroker") {
    EventBroker broker;
    g_on_pending_calls = 0;
    broker.set_on_pending(&count_on_pending_calls);
    broker.set_on_pending(nullptr);

    broker.post(EventA(1));
    REQUIRE(g_on_pending_calls == 0)
}

#endif // RUNEOS_EVENTBROKERTEST_H
