/* Copyright (c) 2026 VillageSQL Contributors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "villagesql/services/preview/session_string.h"

namespace villagesql_unittest {

using villagesql::services::copy_utf8;

TEST(SessionCopyUtf8, WholeStringFits) {
  char buf[16];
  size_t out = 0;
  EXPECT_EQ(VEF_SESSION_OK, copy_utf8(buf, sizeof(buf), "hello", 5, &out));
  EXPECT_STREQ("hello", buf);
  EXPECT_EQ(5u, out);
}

TEST(SessionCopyUtf8, ExactFitIncludingNul) {
  char buf[6];  // 5 chars + NUL
  size_t out = 0;
  EXPECT_EQ(VEF_SESSION_OK, copy_utf8(buf, sizeof(buf), "hello", 5, &out));
  EXPECT_STREQ("hello", buf);
  EXPECT_EQ(5u, out);
}

TEST(SessionCopyUtf8, TruncatesAsciiAndReportsNeeded) {
  char buf[4];  // room for 3 chars + NUL
  size_t out = 0;
  EXPECT_EQ(VEF_SESSION_TRUNCATED,
            copy_utf8(buf, sizeof(buf), "hello", 5, &out));
  EXPECT_STREQ("hel", buf);
  EXPECT_EQ(5u, out);  // needed = full length
}

TEST(SessionCopyUtf8, DoesNotSplitMultibyteCodePoint) {
  // "héllo": 'é' = 0xC3 0xA9 at bytes [1,2]; total 6 bytes.
  const char *s = "h\xC3\xA9llo";
  char buf[3];  // room for 2 bytes + NUL; a naive cut lands mid-'é'
  size_t out = 0;
  EXPECT_EQ(VEF_SESSION_TRUNCATED, copy_utf8(buf, sizeof(buf), s, 6, &out));
  EXPECT_STREQ("h", buf);  // drops the partial 'é'
  EXPECT_EQ(6u, out);
}

TEST(SessionCopyUtf8, ZeroBufferReportsTruncatedAndNeeded) {
  size_t out = 0;
  EXPECT_EQ(VEF_SESSION_TRUNCATED, copy_utf8(nullptr, 0, "x", 1, &out));
  EXPECT_EQ(1u, out);
}

}  // namespace villagesql_unittest
