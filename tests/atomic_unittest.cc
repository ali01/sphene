#include <inttypes.h>
#include <ostream>

#include <ck_pr.h>
#include <gtest/gtest.h>

#include "fwk/atomic.h"


class AtomicInt32Test : public testing::Test {
 protected:
  Fwk::AtomicInt32 f;

  virtual void SetUp() {
    f = 0;
  }
};


class AtomicUInt32Test : public testing::Test {
 protected:
  Fwk::AtomicUInt32 f;

  virtual void SetUp() {
    f = 0;
  }
};


class AtomicInt64Test : public testing::Test {
 protected:
  Fwk::AtomicInt64 f;

  virtual void SetUp() {
    f = 0;
  }
};


class AtomicStruct32Test : public testing::Test {
 public:
  struct Struct32 {
    uint16_t a;
    uint16_t b;

    bool operator==(const Struct32& other) const {
      return (a == other.a && b == other.b);
    }
  } CK_CC_ALIGN;

  typedef Fwk::AtomicType32<Struct32> AtomicStruct32;

 protected:
  AtomicStruct32 s;
};


std::ostream& operator<<(std::ostream& out,
                         const AtomicStruct32Test::Struct32& v) {
  out << "{ a:" << v.a << ", " << "b:" << v.b << "}";
  return out;
}


/**************************/
/**** AtomicInt32Test *****/
/**************************/

TEST_F(AtomicInt32Test, construct) {
  Fwk::AtomicInt32 f(0);
  ASSERT_EQ(0, f.value());

  Fwk::AtomicInt32 g(42);
  ASSERT_EQ(42, g.value());
}

TEST_F(AtomicInt32Test, inc) {
  // Postfix.
  f = 0;
  ASSERT_EQ(0, f++);
  ASSERT_EQ(1, f.value());

  // Prefix.
  f = 0;
  ASSERT_EQ(1, ++f);
  ASSERT_EQ(1, f.value());
}

TEST_F(AtomicInt32Test, dec) {
  // Postfix.
  f = 5;
  ASSERT_EQ(5, f--);
  ASSERT_EQ(4, f.value());

  // Prefix.
  f = 5;
  ASSERT_EQ(4, --f);
  ASSERT_EQ(4, f.value());
}

TEST_F(AtomicInt32Test, add) {
  ASSERT_EQ(1, f += 1);
  ASSERT_EQ(6, f += 5);
  ASSERT_EQ(6, f.value());
}

TEST_F(AtomicInt32Test, sub) {
  f = 10;
  ASSERT_EQ(9, f -= 1);
  ASSERT_EQ(0, f -= 9);
  ASSERT_EQ(0, f.value());
}

TEST_F(AtomicInt32Test, copy) {
  f = 10;
  Fwk::AtomicInt32 g(f);
  ASSERT_EQ(10, g.value());

  f = 20;
  g = f;
  ASSERT_EQ(20, g.value());

  f = 30;
  ASSERT_EQ(20, g.value());
}


/**************************/
/**** AtomicUInt32Test ****/
/**************************/

TEST_F(AtomicUInt32Test, plus_equal) {
  f = 10;

  // operator+= with a negative value.
  int s = -5;
  f += s;
  ASSERT_EQ((uint32_t)5, f.value());

  // operator+= with a positive value.
  f += 20;
  ASSERT_EQ((uint32_t)25, f.value());

  // Big values.
  f = 4294967295u;  // 2^32 - 1
  f += 1;
  ASSERT_EQ((uint32_t)0, f.value());
}


TEST_F(AtomicUInt32Test, minus_equal) {
  f = 50;

  // operator-= with a negative value.
  int s = -5;
  f -= s;
  ASSERT_EQ((uint32_t)55, f.value());

  // operator-= with a positive value.
  f -= 20;
  ASSERT_EQ((uint32_t)35, f.value());

  // Big values.
  f = 0;
  f -= 1;
  ASSERT_EQ(4294967295u, f.value());
}


/**************************/
/**** AtomicInt64Test *****/
/**************************/

TEST_F(AtomicInt64Test, construct) {
  Fwk::AtomicInt64 f(0);
  ASSERT_EQ(0, f.value());

  Fwk::AtomicInt64 g(42);
  ASSERT_EQ(42, g.value());
}

TEST_F(AtomicInt64Test, inc) {
  // Postfix.
  f = 0;
  ASSERT_EQ(0, f++);
  ASSERT_EQ(1, f.value());

  // Prefix.
  f = 0;
  ASSERT_EQ(1, ++f);
  ASSERT_EQ(1, f.value());
}

TEST_F(AtomicInt64Test, dec) {
  // Postfix.
  f = 5;
  ASSERT_EQ(5, f--);
  ASSERT_EQ(4, f.value());

  // Prefix.
  f = 5;
  ASSERT_EQ(4, --f);
  ASSERT_EQ(4, f.value());
}

TEST_F(AtomicInt64Test, add) {
  ASSERT_EQ(1, f += 1);
  ASSERT_EQ(6, f += 5);
  ASSERT_EQ(6, f.value());
}

TEST_F(AtomicInt64Test, sub) {
  f = 10;
  ASSERT_EQ(9, f -= 1);
  ASSERT_EQ(0, f -= 9);
  ASSERT_EQ(0, f.value());
}

TEST_F(AtomicInt64Test, copy) {
  f = 10;
  Fwk::AtomicInt64 g(f);
  ASSERT_EQ(10, g.value());

  f = 20;
  g = f;
  ASSERT_EQ(20, g.value());

  f = 30;
  ASSERT_EQ(20, g.value());
}


/**************************/
/*** AtomicStruct32Test ***/
/**************************/

TEST_F(AtomicStruct32Test, size) {
  ASSERT_EQ((size_t)4, sizeof(Struct32));
}

TEST_F(AtomicStruct32Test, set) {
  Struct32 foo = { 42, 24 };
  s = foo;
  ASSERT_EQ(foo, s.value());
}

TEST_F(AtomicStruct32Test, memberCAS) {
  // Set initial values.
  Struct32 current = { 42, 0 };
  s = current;

  // Create the new value with one changed member.
  Struct32 new_value = { 42, 5 };  // change 'b' member

  // Simulate another thread updating the 'a' member.
  Struct32 other_thread_update = { 16, 0 };
  s = other_thread_update;

  // Setting s to new_value here would undo the other thread's change. We want
  // to make an atomic update to a single member in the struct. Therefore, CAS
  // should find something unexpected and fail.
  ASSERT_FALSE(s.cas(current, new_value));
  ASSERT_EQ(other_thread_update, s.value());

  // Now show a typical usage of the CAS operation to update one member of a
  // struct.
  for (;;) {
    // Get current value.
    current = s.value();

    // Update one member.
    Struct32 update = current;
    update.b = 5;

    if (s.cas(current, update))
      break;  // expected value was still there, update succeeded

    break;    // just avoid spinning in a loop in the test.
  }

  Struct32 expected = { 16, 5 };
  ASSERT_EQ(expected, s.value());
}


int
main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
