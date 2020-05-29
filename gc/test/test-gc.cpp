#include "Object.h"
#include <catch2/catch.hpp>
#include <iostream>
#include <omtalk/Allocate.h>
#include <omtalk/Handle.h>
#include <omtalk/Heap.h>
#include <omtalk/MemoryManager.h>
#include <omtalk/Ref.h>
#include <omtalk/Tracing.h>
#include <omtalk/Util/BitArray.h>

//===----------------------------------------------------------------------===//
// TestValueProxy
//===----------------------------------------------------------------------===//

class TestObjectProxy;

class TestValueProxy {
public:
  TestValueProxy(TestValue *target) : target(target) {}

  TestValueProxy(const TestValueProxy &) = default;

  gc::Ref<TestObject> loadRef() const noexcept { return target->asRef; }

  void storeRef(gc::Ref<TestObject> object) const noexcept {
    target->asRef = object.get();
  }

  TestObjectProxy loadProxy() const noexcept;

private:
  TestValue *target;
};

//===----------------------------------------------------------------------===//
// TestRObjectProxy
//===----------------------------------------------------------------------===//

// template <typename S>
// struct SizeOf {
//   template <typename ObjectProxyT>
//   void operator()(Context &cx, ObjectProxyT target) const noexcept {
//     return target.size(cx);
//   }
// };

// template <typename C, typename V>
// void walk(C &cx, V &visitor) {
//   for (unsigned i = 0; i < length; i++) {
//     auto &slot = slots[i];
//     if (slot.kind == TestValue::Kind::REF) {
//       visitor.visit(cx, TestValueProxy(&slot));
//     }
//   }
// }

template <typename C, typename V>
class ValueProxyVisitor {
public:
  void visit(C &cx, TestValue *slot) {
    visitor.visit(cx, TestValueProxy(slot));
  }
  V &visitor;
};

class TestObjectProxy {
public:
  explicit TestObjectProxy(gc::Ref<TestObject> obj) : target(obj) {}

  explicit TestObjectProxy(gc::Ref<TestStructObject> obj)
      : TestObjectProxy(obj.reinterpret<TestObject>()) {}

  explicit TestObjectProxy(gc::Ref<TestMapObject> obj)
      : TestObjectProxy(obj.reinterpret<TestObject>()) {}

  std::size_t getSize() const noexcept {
    switch (target->kind) {
    case TestObjectKind::STRUCT:
      return target.reinterpret<TestStructObject>()->getSize();
    case TestObjectKind::MAP:
      return target.reinterpret<TestMapObject>()->getSize();
    default:
      return 0;
    }
  }

  template <typename ContextT, typename VisitorT>
  void walk(ContextT &cx, VisitorT &visitor) const noexcept {

    ValueProxyVisitor<ContextT, VisitorT> proxyVisitor(visitor);

    switch (target->kind) {
    case TestObjectKind::STRUCT:
      target.cast<TestStructObject>()->walk(cx, proxyVisitor);
      break;
    case TestObjectKind::MAP:
      // target.cast<TestMapObject>()->walk(cx, proxyVisitor);
      break;
    default:
      break;
    }
  }

  gc::Ref<TestObject> get() const noexcept { return target; }

private:
  gc::Ref<TestObject> target;
};

class TestRootWalker {};

//===----------------------------------------------------------------------===//
// TestValueProxy inlines
//===----------------------------------------------------------------------===//

inline TestObjectProxy TestValueProxy::loadProxy() const noexcept {
  return TestObjectProxy(target->asRef);
}

//===----------------------------------------------------------------------===//
// Test Collector
//===----------------------------------------------------------------------===//

struct TestCollectorScheme {
  using ObjectProxy = TestObjectProxy;
  using SlotProxy = TestValueProxy;
};

template <>
struct gc::GetProxy<TestCollectorScheme> {
  TestObjectProxy operator()(Ref<void> target) const noexcept {
    return TestObjectProxy(target.reinterpret<TestObject>());
  }
};

//===----------------------------------------------------------------------===//
// Test Allocator
//===----------------------------------------------------------------------===//

inline gc::Ref<TestStructObject>
allocateTestStructObject(gc::Context &cx, std::size_t nslots) noexcept {
  auto size = TestStructObject::allocSize(nslots);
  return gc::allocate<TestStructObject>(
      cx, size, [=](auto object) { object->length = nslots; });
}

//===----------------------------------------------------------------------===//
// Test Main
//===----------------------------------------------------------------------===//

TEST_CASE("allocation", "[garbage collector]") {
  gc::MemoryManager mm;
  // gc::MemoryManager<TestCollectorScheme> collector;
  // gc::MarkingScheme<TestCollectorScheme> markingScheme;

  gc::Context context(mm);
  auto ref = allocateTestStructObject(context, 10);

  for (int i = 0; i < 1000; i++) {
   

    if (ref == nullptr) {
      std::cout << "Bad Allocation\n";
      break;
    }

    std::cout << "successful allocation\n" << *ref << std::endl;
  }
  return;
  int x = 1234;
  REQUIRE(x == 1234);
}

TEST_CASE("this is dumb", "[dumb[") {

  // auto collector = gc::Scheme().marking(testMarkingScheme()).build();
}

// MarkingScheme *testMarkingScheme() { return MakeMarkingScheme<TestScheme>();
// }