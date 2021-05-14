// Copyright 2021 The Tint Authors.
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

#ifndef SRC_BLOCK_ALLOCATOR_H_
#define SRC_BLOCK_ALLOCATOR_H_

#include <memory>
#include <utility>
#include <vector>

namespace tint {

/// A container and allocator of objects of (or deriving from) the template type
/// `T`.
/// Objects are allocated by calling Create(), and are owned by the
/// BlockAllocator. When the BlockAllocator is destructed, all constructed
/// objects are automatically destructed and freed.
///
/// Objects held by the BlockAllocator can be iterated over using a
/// View or ConstView.
template <typename T>
class BlockAllocator {
  using InternalVector = std::vector<std::unique_ptr<T>>;
  using InternalIterator = typename InternalVector::const_iterator;

 public:
  class View;
  class ConstView;

  /// Constructor
  BlockAllocator() = default;
  /// Move constructor
  BlockAllocator(BlockAllocator&&) = default;
  /// Move assignment operator
  /// @return this BlockAllocator
  BlockAllocator& operator=(BlockAllocator&&) = default;

  /// An iterator for the objects owned by the BlockAllocator.
  class Iterator {
   public:
    /// Equality operator
    /// @param other the iterator to compare this iterator to
    /// @returns true if this iterator is equal to other
    bool operator==(const Iterator& other) const { return it_ == other.it_; }
    /// Inequality operator
    /// @param other the iterator to compare this iterator to
    /// @returns true if this iterator is not equal to other
    bool operator!=(const Iterator& other) const { return it_ != other.it_; }
    /// Advances the iterator
    /// @returns this iterator
    Iterator& operator++() {
      ++it_;
      return *this;
    }
    /// @returns the pointer to the object at the current iterator position
    T* operator*() const { return it_->get(); }

   private:
    friend View;  // Keep internal iterator impl private.
    explicit Iterator(InternalIterator it) : it_(it) {}
    InternalIterator it_;
  };

  /// A const iterator for the objects owned by the BlockAllocator.
  class ConstIterator {
   public:
    /// Equality operator
    /// @param other the iterator to compare this iterator to
    /// @returns true if this iterator is equal to other
    bool operator==(const ConstIterator& other) const {
      return it_ == other.it_;
    }
    /// Inequality operator
    /// @param other the iterator to compare this iterator to
    /// @returns true if this iterator is not equal to other
    bool operator!=(const ConstIterator& other) const {
      return it_ != other.it_;
    }
    /// Advances the iterator
    /// @returns this iterator
    ConstIterator& operator++() {
      ++it_;
      return *this;
    }
    /// @returns the pointer to the object at the current iterator position
    T* operator*() const { return it_->get(); }

   private:
    friend ConstView;  // Keep internal iterator impl private.
    explicit ConstIterator(InternalIterator it) : it_(it) {}
    InternalIterator it_;
  };

  /// View provides begin() and end() methods for looping over the objects owned
  /// by the BlockAllocator.
  class View {
   public:
    /// @returns an iterator to the beginning of the view
    Iterator begin() const { return Iterator(allocator_->objects_.begin()); }
    /// @returns an iterator to the end of the view
    Iterator end() const { return Iterator(allocator_->objects_.end()); }

   private:
    friend BlockAllocator;  // For BlockAllocator::operator View()
    explicit View(BlockAllocator const* allocator) : allocator_(allocator) {}
    BlockAllocator const* const allocator_;
  };

  /// ConstView provides begin() and end() methods for looping over the objects
  /// owned by the BlockAllocator.
  class ConstView {
   public:
    /// @returns an iterator to the beginning of the view
    ConstIterator begin() const {
      return ConstIterator(allocator_->objects_.begin());
    }
    /// @returns an iterator to the end of the view
    ConstIterator end() const {
      return ConstIterator(allocator_->objects_.end());
    }

   private:
    friend BlockAllocator;  // For BlockAllocator::operator ConstView()
    explicit ConstView(BlockAllocator const* allocator)
        : allocator_(allocator) {}
    BlockAllocator const* const allocator_;
  };

  /// @return a View of all objects owned by this BlockAllocator
  View Objects() { return View(this); }
  /// @return a ConstView of all objects owned by this BlockAllocator
  ConstView Objects() const { return ConstView(this); }

  /// Creates a new `TYPE` owned by the BlockAllocator.
  /// When the BlockAllocator is destructed the object will be destructed and
  /// freed.
  /// @param args the arguments to pass to the type constructor
  /// @returns the pointer to the constructed object
  template <typename TYPE = T, typename... ARGS>
  TYPE* Create(ARGS&&... args) {
    static_assert(
        std::is_same<T, TYPE>::value || std::is_base_of<T, TYPE>::value,
        "TYPE does not derive from T");
    auto uptr = std::make_unique<TYPE>(std::forward<ARGS>(args)...);
    auto* ptr = uptr.get();
    objects_.emplace_back(std::move(uptr));
    return ptr;
  }

 private:
  BlockAllocator(const BlockAllocator&) = delete;
  BlockAllocator& operator=(const BlockAllocator&) = delete;

  std::vector<std::unique_ptr<T>> objects_;
};

}  // namespace tint

#endif  // SRC_BLOCK_ALLOCATOR_H_
