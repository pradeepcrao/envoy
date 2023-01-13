#include "envoy/config/route/v3/route_components.pb.h"

#include "source/common/common/packed_struct.h"

#include "test/common/stats/stat_test_utility.h"
#include "test/test_common/utility.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace Envoy {
namespace {

TEST(PackedStruct, StringStruct) {
  enum class RedirectStringElement { scheme_redirect, host_redirect, path_redirect };
  using RedirectStringsPackedStruct = PackedStruct<std::string, 3, RedirectStringElement>;

  Stats::TestUtil::MemoryTest memory_test;
  // Initialize capacity to 2.
  RedirectStringsPackedStruct redirect_strings(2);
  redirect_strings.set(RedirectStringElement::scheme_redirect, "abc");
  redirect_strings.set(RedirectStringElement::path_redirect, "def");
  EXPECT_MEMORY_LE(memory_test.consumedBytes(), 2 * sizeof(std::string) + 16);

  EXPECT_EQ(redirect_strings.size(), 2);
  EXPECT_EQ(redirect_strings.capacity(), 2);
  EXPECT_EQ(redirect_strings.get(RedirectStringElement::scheme_redirect), "abc");
  EXPECT_EQ(redirect_strings.get(RedirectStringElement::path_redirect), "def");
  EXPECT_FALSE(redirect_strings.has(RedirectStringElement::host_redirect));

  // Add a third element.
  redirect_strings.set(RedirectStringElement::host_redirect, "abcd");
  EXPECT_EQ(redirect_strings.get(RedirectStringElement::host_redirect), "abcd");
  EXPECT_MEMORY_LE(memory_test.consumedBytes(), 3 * sizeof(std::string) + 16);
  EXPECT_EQ(redirect_strings.size(), 3);
  EXPECT_EQ(redirect_strings.capacity(), 3);
}

// Test the move constructor and move assignment operators. Verify that no
// memory is allocated on the heap because of these operations.
TEST(PackedStruct, StringStructMove) {
  enum class RedirectStringElement { scheme_redirect, host_redirect, path_redirect };
  using RedirectStringsPackedStruct = PackedStruct<std::string, 3, RedirectStringElement>;

  Stats::TestUtil::MemoryTest memory_test;
  // Initialize capacity to 2.
  RedirectStringsPackedStruct redirect_strings(2);
  redirect_strings.set(RedirectStringElement::scheme_redirect, "abc");
  redirect_strings.set(RedirectStringElement::path_redirect, "def");
  EXPECT_MEMORY_LE(memory_test.consumedBytes(), 2 * sizeof(std::string) + 16);

  EXPECT_EQ(redirect_strings.size(), 2);
  EXPECT_EQ(redirect_strings.capacity(), 2);
  EXPECT_EQ(redirect_strings.get(RedirectStringElement::scheme_redirect), "abc");
  EXPECT_EQ(redirect_strings.get(RedirectStringElement::path_redirect), "def");
  EXPECT_FALSE(redirect_strings.has(RedirectStringElement::host_redirect));

  // Invoke move constructor.
  RedirectStringsPackedStruct redirect_strings2(move(redirect_strings));
  // Ensure no memory was allocated on the heap by the move constructor.
  EXPECT_MEMORY_LE(memory_test.consumedBytes(), 2 * sizeof(std::string) + 16);

  EXPECT_EQ(redirect_strings2.size(), 2);
  EXPECT_EQ(redirect_strings2.capacity(), 2);
  EXPECT_EQ(redirect_strings2.get(RedirectStringElement::scheme_redirect), "abc");
  EXPECT_EQ(redirect_strings2.get(RedirectStringElement::path_redirect), "def");
  EXPECT_FALSE(redirect_strings2.has(RedirectStringElement::host_redirect));

  // Verify that redirect_strings is now empty.
  EXPECT_EQ(redirect_strings.size(), 0);
  EXPECT_EQ(redirect_strings.capacity(), 0);
  EXPECT_FALSE(redirect_strings.has(RedirectStringElement::scheme_redirect));
  EXPECT_FALSE(redirect_strings.has(RedirectStringElement::path_redirect));
  EXPECT_FALSE(redirect_strings.has(RedirectStringElement::host_redirect));

  // Invoke move assignment.
  RedirectStringsPackedStruct redirect_strings3(0);
  redirect_strings3 = move(redirect_strings2);
  // Ensure no memory was allocated on the heap by the move assignment.
  EXPECT_MEMORY_LE(memory_test.consumedBytes(), 2 * sizeof(std::string) + 16);

  EXPECT_EQ(redirect_strings3.size(), 2);
  EXPECT_EQ(redirect_strings3.capacity(), 2);
  EXPECT_EQ(redirect_strings3.get(RedirectStringElement::scheme_redirect), "abc");
  EXPECT_EQ(redirect_strings3.get(RedirectStringElement::path_redirect), "def");
  EXPECT_FALSE(redirect_strings3.has(RedirectStringElement::host_redirect));

  // Verify that redirect_strings2 is now empty.
  EXPECT_EQ(redirect_strings2.size(), 0);
  EXPECT_EQ(redirect_strings2.capacity(), 0);
  EXPECT_FALSE(redirect_strings2.has(RedirectStringElement::scheme_redirect));
  EXPECT_FALSE(redirect_strings2.has(RedirectStringElement::path_redirect));
  EXPECT_FALSE(redirect_strings2.has(RedirectStringElement::host_redirect));
}

} // namespace
} // namespace Envoy
