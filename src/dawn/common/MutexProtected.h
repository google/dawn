// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_COMMON_MUTEXPROTECTED_H_
#define SRC_DAWN_COMMON_MUTEXPROTECTED_H_

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <type_traits>
#include <utility>

#include "dawn/common/Compiler.h"
#include "dawn/common/Defer.h"
#include "dawn/common/Mutex.h"
#include "dawn/common/NonMovable.h"
#include "dawn/common/Ref.h"
#include "dawn/common/StackAllocated.h"
#include "dawn/common/Time.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "partition_alloc/pointers/raw_ptr_exclusion.h"

namespace dawn {

template <typename T, template <typename, typename> class Guard>
class MutexProtected;

template <typename T, template <typename, typename> class Guard>
class MutexCondVarProtected;

template <typename T, template <typename, typename> class Guard>
class MutexProtectedSupport;

namespace detail {

template <typename T>
struct MutexProtectedTraits {
    using MutexType = std::mutex;
    using LockType = std::unique_lock<std::mutex>;
    using ObjectType = T;

    static constexpr bool kSupportsTryLock = true;

    static MutexType CreateMutex() { return std::mutex(); }
    static std::mutex& GetMutex(MutexType& m) { return m; }
    static ObjectType* GetObj(T* const obj) { return obj; }
    static const ObjectType* GetObj(const T* const obj) { return obj; }

    static std::optional<LockType> TryLock(MutexType& mutex) {
        LockType lock(GetMutex(mutex), std::try_to_lock);
        if (!lock.owns_lock()) {
            return std::nullopt;
        }
        return lock;
    }
};

template <typename T>
struct MutexProtectedTraits<Ref<T>> {
    using MutexType = Ref<Mutex>;
    using LockType = Mutex::AutoLock;
    using ObjectType = T;

    static constexpr bool kSupportsTryLock = false;

    static MutexType CreateMutex() { return AcquireRef(new Mutex()); }
    static Mutex* GetMutex(MutexType& m) { return m.Get(); }
    static ObjectType* GetObj(Ref<T>* const obj) { return obj->Get(); }
    static const ObjectType* GetObj(const Ref<T>* const obj) { return obj->Get(); }
};

template <typename T>
struct MutexProtectedSupportTraits {
    using MutexType = std::mutex;
    using LockType = std::unique_lock<std::mutex>;

    static constexpr bool kSupportsTryLock = true;

    static MutexType CreateMutex() { return std::mutex(); }
    static std::mutex& GetMutex(MutexType& m) { return m; }
    static auto* GetObj(T* const obj) { return &obj->mImpl; }
    static const auto* GetObj(const T* const obj) { return &obj->mImpl; }

    static std::optional<LockType> TryLock(MutexType& mutex) {
        LockType lock(GetMutex(mutex), std::try_to_lock);
        if (!lock.owns_lock()) {
            return std::nullopt;
        }
        return lock;
    }
};

template <typename T, typename Traits>
class Guard;
template <typename T, typename Traits>
class CondVarGuard;

// Guard class is a wrapping class that gives access to a protected resource after acquiring the
// lock related to it. For the lifetime of this class, the lock is held.
template <typename T, typename Traits>
class DAWN_SCOPED_LOCKABLE Guard : public NonMovable, StackAllocated {
  public:
    // It's the programmer's burden to not save the pointer/reference and reuse it without the lock.
    auto* operator->() const { return Get(); }
    auto& operator*() const { return *Get(); }

    void Defer(std::function<void()> f) {
        DAWN_ASSERT(mDefer);
        mDefer->Append(std::move(f));
    }

  protected:
    Guard() : mLock() {}
    Guard(T* obj, typename Traits::MutexType& mutex, class Defer* defer = nullptr)
        : mLock(Traits::GetMutex(mutex)), mObj(obj), mDefer(defer) {}
    Guard(T* obj, Traits::LockType&& lock, class Defer* defer = nullptr)
        : mLock(std::move(lock)), mObj(obj), mDefer(defer) {}
    Guard(Guard&& other)
        : mLock(std::move(other.mLock)),
          mObj(std::move(other.mObj)),
          mDefer(std::move(other.mDefer)) {
        other.mObj = nullptr;
    }

    Guard& operator=(Guard&& other) {
        if (this != &other) {
            mLock = std::move(other.mLock);
            mObj = std::move(other.mObj);
            mDefer = std::move(other.mDefer);
            other.mObj = nullptr;
            other.mDefer = nullptr;
        }
        return *this;
    }

    Guard(const Guard& other) = delete;
    Guard& operator=(const Guard& other) = delete;

    auto* Get() const { return Traits::GetObj(mObj); }

  private:
    using NonConstT = typename std::remove_const<T>::type;
    friend class CondVarGuard<T, Traits>;
    friend class MutexProtectedSupport<NonConstT, Guard>;
    friend class MutexProtected<NonConstT, Guard>;

    typename Traits::LockType mLock;
    // RAW_PTR_EXCLUSION: This pointer is created/destroyed on each access to a MutexProtected.
    // The pointer is always transiently used while the MutexProtected is in scope so it is
    // unlikely to be used after it is freed.
    RAW_PTR_EXCLUSION T* mObj = nullptr;
    raw_ptr<class Defer> mDefer = nullptr;
};

// CondVarGuard is a different guard class that internally holds a Guard, but provides additional
// functionality w.r.t condition variables. Specifically, the non-const version of this Guard will
// automatically call notify_all() on the underlying condition variable so that calls to |Wait*()|
// will unblock when |Pred| is true.
template <typename T, typename Traits>
class CondVarGuard : public NonMovable, StackAllocated {
  public:
    // It's the programmer's burden to not save the pointer/reference and reuse it without the lock.
    auto* operator->() const { return mGuard.Get(); }
    auto& operator*() const { return *mGuard.Get(); }

    template <typename Predicate>
    void Wait(Predicate pred) {
        DAWN_ASSERT(mNotifyScope.cv);
        mNotifyScope.cv->wait(mGuard.mLock, [&] { return pred((*Get())); });
    }
    template <typename Predicate>
    bool WaitFor(Nanoseconds timeout, Predicate pred) {
        DAWN_ASSERT(mNotifyScope.cv);
        if (timeout < kMaxDurationNanos) {
            return mNotifyScope.cv->wait_for(
                mGuard.mLock, std::chrono::nanoseconds(static_cast<uint64_t>(timeout)),
                [&] { return pred(*Get()); });
        } else {
            Wait(pred);
            return true;
        }
    }

  protected:
    CondVarGuard(T* obj,
                 typename Traits::MutexType& mutex,
                 class Defer* defer = nullptr,
                 std::condition_variable* cv = nullptr)
        : mNotifyScope(cv), mGuard(obj, mutex, defer) {}
    CondVarGuard(T* obj,
                 Traits::LockType&& lock,
                 class Defer* defer = nullptr,
                 std::condition_variable* cv = nullptr)
        : mNotifyScope(cv), mGuard(obj, std::move(lock), defer) {}

    auto* Get() const { return mGuard.Get(); }

  private:
    using NonConstT = typename std::remove_const<T>::type;
    friend class MutexProtected<NonConstT, CondVarGuard>;
    friend class MutexCondVarProtected<NonConstT, CondVarGuard>;

    struct NotifyScope {
        explicit NotifyScope(std::condition_variable* cv) : cv(cv) { DAWN_ASSERT(cv); }
        ~NotifyScope() {
            if constexpr (!std::is_const_v<T>) {
                cv->notify_all();
            }
        }

        raw_ptr<std::condition_variable> cv = nullptr;
    };
    NotifyScope mNotifyScope;
    Guard<T, Traits> mGuard;
};

template <typename T, typename Traits, template <typename, typename> class Guard = detail::Guard>
class MutexProtectedBase {
  public:
    using Usage = Guard<T, Traits>;
    using ConstUsage = Guard<const T, Traits>;

    MutexProtectedBase() : mMutex(Traits::CreateMutex()) {}
    virtual ~MutexProtectedBase() = default;

    Usage operator->() { return Use(); }
    ConstUsage operator->() const { return Use(); }

    template <typename Fn>
    auto Use(Fn&& fn) {
        return fn(Use());
    }
    template <typename Fn>
    auto Use(Fn&& fn) const {
        return fn(ConstUse());
    }
    template <typename Fn>
    auto ConstUse(Fn&& fn) const {
        return fn(ConstUse());
    }

    template <typename Fn>
    auto UseWithDefer(Fn&& fn) {
        Defer defer;
        return fn(UseWithDefer(defer));
    }

    std::optional<Usage> TryUse()
        requires Traits::kSupportsTryLock
    {
        auto maybeLock = Traits::TryLock(this->mMutex);
        if (!maybeLock.has_value()) {
            return std::nullopt;
        }
        return Use(std::move(*maybeLock));
    }

  protected:
    virtual Usage Use() = 0;
    virtual Usage Use(Traits::LockType&& lock) = 0;
    virtual Usage UseWithDefer(Defer& defer) = 0;
    virtual ConstUsage ConstUse() const = 0;

    mutable typename Traits::MutexType mMutex;
};

}  // namespace detail

// Wrapping class used for object members to ensure usage of the resource is protected with a mutex.
// Example usage:
//     class Allocator {
//       public:
//         Allocation Allocate();
//         void Deallocate(Allocation&);
//     };
//     class AllocatorUser {
//       public:
//         void OnlyAllocate() {
//             auto allocation = mAllocator->Allocate();
//         }
//         void AtomicAllocateDeallocate() {
//             // Operations:
//             //   - acquire lock
//             //   - Allocate, Deallocate
//             //   - release lock
//             mAllocator.Use([](auto allocator) {
//                 auto allocation = allocator->Allocate();
//                 allocator->Deallocate(allocation);
//             });
//         }
//         void NonAtomicAllocateDeallocate() {
//             // Operations:
//             //   - acquire lock, Allocate, release lock
//             //   - acquire lock, Deallocate, release lock
//             auto allocation = mAllocator->Allocate();
//             mAllocator->Deallocate(allocation);
//         }
//       private:
//         MutexProtected<Allocator> mAllocator;
//     };
template <typename T, template <typename, typename> class Guard = detail::Guard>
class MutexProtected
    : public detail::MutexProtectedBase<T, detail::MutexProtectedTraits<T>, Guard> {
  public:
    using Traits = detail::MutexProtectedTraits<T>;
    using Base = detail::MutexProtectedBase<T, Traits, Guard>;
    using typename Base::ConstUsage;
    using typename Base::Usage;

    template <typename... Args>
    // NOLINTNEXTLINE(runtime/explicit) allow implicit construction
    MutexProtected(Args&&... args) : mObj(std::forward<Args>(args)...) {}

    using Base::TryUse;
    using Base::Use;
    using Base::UseWithDefer;

  protected:
    T mObj;

  private:
    Usage Use() override { return Usage(&mObj, this->mMutex); }
    Usage Use(Traits::LockType&& lock) override { return Usage(&mObj, std::move(lock), nullptr); }
    Usage UseWithDefer(Defer& defer) override { return Usage(&mObj, this->mMutex, &defer); }
    ConstUsage ConstUse() const override { return ConstUsage(&mObj, this->mMutex); }
};

// Wrapping class for object members to provide the protections with a mutex of a MutexProtected
// with some additional helpers to allow waiting with a conditional variable as well. The general
// usage should look the same as MutexProtected above, with additional usages like the following
// example:
//     class Example {
//       public:
//         void Complete() {
//             mDone.Use([](auto done) {
//                 // Do something
//                 mDone = true;
//             });
//         }
//         void WaitUntilDone() {
//             mDone.Use([](auto done) {
//                 done.Wait([](auto& done) { return done; });
//             });
//         }
//       private:
//         MutexCondVarProtected<bool> mDone = false;
//     };
template <typename T, template <typename, typename> class Guard = detail::CondVarGuard>
class MutexCondVarProtected : public MutexProtected<T, Guard> {
  public:
    using Base = MutexProtected<T, Guard>;
    using typename Base::ConstUsage;
    using typename Base::Usage;

    using Base::Base;

    // Note that unlike in MutexProtected where |Use| and |ConstUse| guarantee the lock for the
    // entire critical section, if a user calls |Wait| within |Fn|, the lock may be released and
    // reacquired in order for another thread to update the condition.
    using Base::Base::ConstUse;
    using Base::Base::TryUse;
    using Base::Base::Use;

  private:
    Usage Use() override { return Usage(&this->mObj, this->mMutex, nullptr, &mCv); }
    Usage Use(Base::Traits::LockType&& lock) override {
        return Usage(&this->mObj, std::move(lock), nullptr, &mCv);
    }
    ConstUsage ConstUse() const override {
        return ConstUsage(&this->mObj, this->mMutex, nullptr, &mCv);
    }

    mutable std::condition_variable mCv;
};

// CRTP wrapper to help create classes that are generally MutexProtected, but may wish to implement
// specific workarounds to avoid taking the lock in certain scenarios. See the example below and the
// unittests for more example usages of this wrapper. Example usage:
//     struct Counter : public MutexProtectedSupport<Counter> {
//       public:
//         // Reads the value stored in |mCounter| without acquiring the lock.
//         int UnsafeRead() {
//             return mImpl.mCounter;
//         }
//
//       private:
//         // This friend declaration MUST be included in all classes using this wrapper.
//         friend typename MutexProtectedSupport<Counter>::Traits;
//
//         // Internal struct that wraps all the actual data that we want to be protected. Note that
//         // this struct currently MUST be named |mImpl| to work.
//         struct {
//             int mCounter = 0;
//         } mImpl;
//     };
//     // Other uses of this struct look as if we are using a MutexProtected<mImpl>.
template <typename T, template <typename, typename> class Guard = detail::Guard>
class MutexProtectedSupport
    : public detail::MutexProtectedBase<T, detail::MutexProtectedSupportTraits<T>, Guard> {
  public:
    using Traits = detail::MutexProtectedSupportTraits<T>;
    using Base = detail::MutexProtectedBase<T, Traits, Guard>;
    using typename Base::ConstUsage;
    using typename Base::Usage;

    using Base::TryUse;
    using Base::Use;
    using Base::UseWithDefer;

  private:
    Usage Use() override { return Usage(static_cast<T*>(this), this->mMutex); }
    Usage Use(Traits::LockType&& lock) override {
        return Usage(static_cast<T*>(this), std::move(lock), nullptr);
    }
    Usage UseWithDefer(Defer& defer) override {
        return Usage(static_cast<T*>(this), this->mMutex, &defer);
    }
    ConstUsage ConstUse() const override {
        return ConstUsage(static_cast<const T*>(this), this->mMutex);
    }
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_MUTEXPROTECTED_H_
