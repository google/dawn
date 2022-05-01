// Copyright 2021 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <utility>
#include <vector>

#include "dawn/common/RefBase.h"
#include "gmock/gmock.h"

namespace {
using Id = uint32_t;

enum class Action {
    kReference,
    kRelease,
    kAssign,
    kMarker,
};

struct Event {
    Action action;
    Id thisId = 0;
    Id otherId = 0;
};

std::ostream& operator<<(std::ostream& os, const Event& event) {
    switch (event.action) {
        case Action::kReference:
            os << "Reference " << event.thisId;
            break;
        case Action::kRelease:
            os << "Release " << event.thisId;
            break;
        case Action::kAssign:
            os << "Assign " << event.thisId << " <- " << event.otherId;
            break;
        case Action::kMarker:
            os << "Marker " << event.thisId;
            break;
    }
    return os;
}

bool operator==(const Event& a, const Event& b) {
    return a.action == b.action && a.thisId == b.thisId && a.otherId == b.otherId;
}

using Events = std::vector<Event>;

struct RefTracker {
    explicit constexpr RefTracker(nullptr_t) : mId(0), mEvents(nullptr) {}

    constexpr RefTracker(const RefTracker& other) = default;

    RefTracker(Id id, Events* events) : mId(id), mEvents(events) {}

    void Reference() const { mEvents->emplace_back(Event{Action::kReference, mId}); }

    void Release() const { mEvents->emplace_back(Event{Action::kRelease, mId}); }

    RefTracker& operator=(const RefTracker& other) {
        if (mEvents || other.mEvents) {
            Events* events = mEvents ? mEvents : other.mEvents;
            events->emplace_back(Event{Action::kAssign, mId, other.mId});
        }
        mId = other.mId;
        mEvents = other.mEvents;
        return *this;
    }

    bool operator==(const RefTracker& other) const { return mId == other.mId; }

    bool operator!=(const RefTracker& other) const { return mId != other.mId; }

    Id mId;
    Events* mEvents;
};

struct RefTrackerTraits {
    static constexpr RefTracker kNullValue{nullptr};

    static void Reference(const RefTracker& handle) { handle.Reference(); }

    static void Release(const RefTracker& handle) { handle.Release(); }
};

constexpr RefTracker RefTrackerTraits::kNullValue;

using Ref = RefBase<RefTracker, RefTrackerTraits>;
}  // namespace

TEST(RefBase, Acquire) {
    Events events;
    RefTracker tracker1(1, &events);
    RefTracker tracker2(2, &events);
    Ref ref(tracker1);

    events.clear();
    { ref.Acquire(tracker2); }
    EXPECT_THAT(events, testing::ElementsAre(Event{Action::kRelease, 1},   // release ref
                                             Event{Action::kAssign, 1, 2}  // acquire tracker2
                                             ));
}

TEST(RefBase, Detach) {
    Events events;
    RefTracker tracker(1, &events);
    Ref ref(tracker);

    events.clear();
    { DAWN_UNUSED(ref.Detach()); }
    EXPECT_THAT(events, testing::ElementsAre(Event{Action::kAssign, 1, 0}  // nullify ref
                                             ));
}

TEST(RefBase, Constructor) {
    Ref ref;
    EXPECT_EQ(ref.Get(), RefTrackerTraits::kNullValue);
}

TEST(RefBase, ConstructDestruct) {
    Events events;
    RefTracker tracker(1, &events);

    events.clear();
    {
        Ref ref(tracker);
        events.emplace_back(Event{Action::kMarker, 10});
    }
    EXPECT_THAT(events, testing::ElementsAre(Event{Action::kReference, 1},  // reference tracker
                                             Event{Action::kMarker, 10},    //
                                             Event{Action::kRelease, 1}     // destruct ref
                                             ));
}

TEST(RefBase, CopyConstruct) {
    Events events;
    RefTracker tracker(1, &events);
    Ref refA(tracker);

    events.clear();
    {
        Ref refB(refA);
        events.emplace_back(Event{Action::kMarker, 10});
    }
    EXPECT_THAT(events, testing::ElementsAre(Event{Action::kReference, 1},  // reference tracker
                                             Event{Action::kMarker, 10},    //
                                             Event{Action::kRelease, 1}     // destruct ref
                                             ));
}

TEST(RefBase, RefCopyAssignment) {
    Events events;
    RefTracker tracker1(1, &events);
    RefTracker tracker2(2, &events);
    Ref refA(tracker1);
    Ref refB(tracker2);

    events.clear();
    {
        Ref ref;
        events.emplace_back(Event{Action::kMarker, 10});
        ref = refA;
        events.emplace_back(Event{Action::kMarker, 20});
        ref = refB;
        events.emplace_back(Event{Action::kMarker, 30});
        ref = refA;
        events.emplace_back(Event{Action::kMarker, 40});
    }
    EXPECT_THAT(events, testing::ElementsAre(Event{Action::kMarker, 10},    //
                                             Event{Action::kReference, 1},  // reference tracker1
                                             Event{Action::kAssign, 0, 1},  // copy tracker1
                                             Event{Action::kMarker, 20},    //
                                             Event{Action::kReference, 2},  // reference tracker2
                                             Event{Action::kRelease, 1},    // release tracker1
                                             Event{Action::kAssign, 1, 2},  // copy tracker2
                                             Event{Action::kMarker, 30},    //
                                             Event{Action::kReference, 1},  // reference tracker1
                                             Event{Action::kRelease, 2},    // release tracker2
                                             Event{Action::kAssign, 2, 1},  // copy tracker1
                                             Event{Action::kMarker, 40},    //
                                             Event{Action::kRelease, 1}     // destruct ref
                                             ));
}

TEST(RefBase, RefMoveAssignment) {
    Events events;
    RefTracker tracker1(1, &events);
    RefTracker tracker2(2, &events);
    Ref refA(tracker1);
    Ref refB(tracker2);

    events.clear();
    {
        Ref ref;
        events.emplace_back(Event{Action::kMarker, 10});
        ref = std::move(refA);
        events.emplace_back(Event{Action::kMarker, 20});
        ref = std::move(refB);
        events.emplace_back(Event{Action::kMarker, 30});
    }
    EXPECT_THAT(events, testing::ElementsAre(Event{Action::kMarker, 10},    //
                                             Event{Action::kAssign, 1, 0},  // nullify refA
                                             Event{Action::kAssign, 0, 1},  // move into ref
                                             Event{Action::kMarker, 20},    //
                                             Event{Action::kRelease, 1},    // release tracker1
                                             Event{Action::kAssign, 2, 0},  // nullify refB
                                             Event{Action::kAssign, 1, 2},  // move into ref
                                             Event{Action::kMarker, 30},    //
                                             Event{Action::kRelease, 2}     // destruct ref
                                             ));
}

TEST(RefBase, RefCopyAssignmentSelf) {
    Events events;
    RefTracker tracker(1, &events);
    Ref ref(tracker);
    Ref& self = ref;

    events.clear();
    {
        ref = self;
        ref = self;
        ref = self;
    }
    EXPECT_THAT(events, testing::ElementsAre());
}

TEST(RefBase, RefMoveAssignmentSelf) {
    Events events;
    RefTracker tracker(1, &events);
    Ref ref(tracker);
    Ref& self = ref;

    events.clear();
    {
        ref = std::move(self);
        ref = std::move(self);
        ref = std::move(self);
    }
    EXPECT_THAT(events, testing::ElementsAre());
}

TEST(RefBase, TCopyAssignment) {
    Events events;
    RefTracker tracker(1, &events);
    Ref ref;

    events.clear();
    {
        ref = tracker;
        ref = tracker;
        ref = tracker;
    }
    EXPECT_THAT(events, testing::ElementsAre(Event{Action::kReference, 1},  //
                                             Event{Action::kAssign, 0, 1}));
}

TEST(RefBase, TMoveAssignment) {
    Events events;
    RefTracker tracker(1, &events);
    Ref ref;

    events.clear();
    { ref = std::move(tracker); }
    EXPECT_THAT(events, testing::ElementsAre(Event{Action::kReference, 1},  //
                                             Event{Action::kAssign, 0, 1}));
}

TEST(RefBase, TCopyAssignmentAlternate) {
    Events events;
    RefTracker tracker1(1, &events);
    RefTracker tracker2(2, &events);
    Ref ref;

    events.clear();
    {
        ref = tracker1;
        events.emplace_back(Event{Action::kMarker, 10});
        ref = tracker2;
        events.emplace_back(Event{Action::kMarker, 20});
        ref = tracker1;
        events.emplace_back(Event{Action::kMarker, 30});
    }
    EXPECT_THAT(events, testing::ElementsAre(Event{Action::kReference, 1},  // reference tracker1
                                             Event{Action::kAssign, 0, 1},  // copy tracker1
                                             Event{Action::kMarker, 10},    //
                                             Event{Action::kReference, 2},  // reference tracker2
                                             Event{Action::kRelease, 1},    // release tracker1
                                             Event{Action::kAssign, 1, 2},  // copy tracker2
                                             Event{Action::kMarker, 20},    //
                                             Event{Action::kReference, 1},  // reference tracker1
                                             Event{Action::kRelease, 2},    // release tracker2
                                             Event{Action::kAssign, 2, 1},  // copy tracker1
                                             Event{Action::kMarker, 30}));
}
