#ifndef OMTALK_GC_TEST_OBJECT_H_
#define OMTALK_GC_TEST_OBJECT_H_

#include <cstddef>
#include <iostream>
#include <omtalk/Ref.h>
#include <ostream>

struct TestObject;
namespace gc = omtalk::gc;

//===----------------------------------------------------------------------===//
// TestValue
//===----------------------------------------------------------------------===//

struct RefTag {};
struct IntTag {};
constexpr RefTag REF;
constexpr IntTag INT;

struct TestValue {
  enum class Kind { REF, INT };

  TestValue() {}
  TestValue(IntTag, int x) : asInt(x), kind(Kind::INT) {}
  TestValue(RefTag, TestObject *x) : asRef(x), kind(Kind::REF) {}
  TestValue(RefTag, gc::Ref<TestObject> x) : asRef(x.get()), kind(Kind::REF) {}

  union {
    TestObject *asRef;
    int asInt;
  };
  Kind kind;
};

std::ostream &operator<<(std::ostream &out, const TestValue::Kind &kind) {
  switch (kind) {
  case TestValue::Kind::REF:
    out << "REF";
    break;
  case TestValue::Kind::INT:
    out << "INT";
    break;
  }
  return out;
}

std::ostream &operator<<(std::ostream &out, const TestValue &obj) {
  out << "(TestValue kind: " << obj.kind << ", value: ";
  if (obj.kind == TestValue::Kind::REF) {
    out << obj.asRef;
  } else {
    out << obj.asInt;
  }
  out << ")";
  return out;
}

//===----------------------------------------------------------------------===//
// TestObjectKind
//===----------------------------------------------------------------------===//

enum class TestObjectKind { INVALID, STRUCT, MAP };

std::ostream &operator<<(std::ostream &out, const TestObjectKind &obj) {
  switch (obj) {
  case TestObjectKind::INVALID:
    out << "INVALID";
    break;
  case TestObjectKind::STRUCT:
    out << "STRUCT";
    break;
  case TestObjectKind::MAP:
    out << "MAP";
    break;
  }
  return out;
}

//===----------------------------------------------------------------------===//
// TestObject
//===----------------------------------------------------------------------===//

struct TestObject {
  TestObjectKind kind;
};

std::ostream &operator<<(std::ostream &out, const TestObject &obj) {
  out << "(TestObject";
  out << " kind: " << obj.kind;
  out << ")";
  return out;
}

struct TestForwardedRecord : TestObject {
  void *to;
};

//===----------------------------------------------------------------------===//
// TestStructObject
//===----------------------------------------------------------------------===//

struct TestStructObject : public TestObject {
  static constexpr std::size_t allocSize(std::size_t nslots) {
    return sizeof(TestStructObject) + (sizeof(TestValue) * nslots);
  }

  std::size_t getSize() const noexcept { return allocSize(length); }

  std::size_t getLength() const noexcept { return length; }

  void setSlot(unsigned slot, TestValue value) noexcept { slots[slot] = value; }

  template <typename C, typename V>
  void walk(C &cx, V &visitor) {
    std::cout << "!!! TestStructObject::walk: " << this << std::endl;
    for (unsigned i = 0; i < length; i++) {
      auto &slot = slots[i];
      if (slot.kind == TestValue::Kind::REF && slot.asRef != nullptr) {
        std::cout << "!!!   slot:" << slot << std::endl;
        visitor.visit(cx, &slot);
      }
    }
  }

  std::size_t length;
  TestValue slots[];
};

std::ostream &operator<<(std::ostream &out, const TestStructObject &obj) {
  out << "(TestStructObject";
  out << " kind: " << obj.kind;
  out << ", length: " << obj.length;
  out << ",\n";
  for (unsigned i = 0; i < obj.getLength(); i++)
    out << "  slot[" << i << "]: " << obj.slots[i] << std::endl;
  out << ")";
  return out;
}

//===----------------------------------------------------------------------===//
// TestMapObject
//===----------------------------------------------------------------------===//

///
/// TODO
///

struct TestMapObject : public TestObject {

  struct Bucket {
    int key;
    TestValue value;
  };

  static constexpr std::size_t allocSize(std::size_t nslots) {
    return sizeof(TestMapObject) + (sizeof(Bucket) * nslots);
  }

  static constexpr int TOMBSTONE = std::numeric_limits<int>::max();

  std::size_t getSize() const noexcept { return allocSize(length); }

  std::size_t getLength() const noexcept { return length; }

  bool insert(int key, TestValue value) noexcept { return false; }

  bool remove(int key) noexcept { return false; }

  TestValue get(int key) const noexcept { return {INT, 0}; }

  template <typename C, typename V>
  void walk(C &cx, V &visitor) {
    for (unsigned i = 0; i < length; i++) {
      auto &slot = buckets[i].value;
      if (slot.kind == TestValue::Kind::REF) {
        visitor.visit(cx, &slot);
      }
    }
  }

  std::size_t length;
  Bucket buckets[];
};

std::ostream &operator<<(std::ostream &out,
                         const TestMapObject::Bucket &bucket) {
  out << "key: " << bucket.key;
  out << " value: " << bucket.value;
  return out;
}

std::ostream &operator<<(std::ostream &out, const TestMapObject &obj) {
  out << "(TestMapObject";
  out << " kind: " << obj.kind;
  out << " length: " << obj.length;
  for (unsigned i = 0; i < obj.getLength(); i++)
    out << " bucket[" << i << "]: " << obj.buckets[i];
  out << ")";
  return out;
}

#endif