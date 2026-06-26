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

#include <villagesql/abi/types.h>

namespace villagesql_unittest {

class SessionContextAbiTest : public ::testing::Test {};

TEST_F(SessionContextAbiTest, KillStatusEnumValues) {
  EXPECT_EQ(0u, VEF_KILL_NOT_KILLED);
  EXPECT_EQ(1u, VEF_KILL_CONNECTION);
  EXPECT_EQ(2u, VEF_KILL_QUERY);
  EXPECT_EQ(3u, VEF_KILL_TIMEOUT);
  EXPECT_EQ(255u, VEF_KILL_UNKNOWN);
}

TEST_F(SessionContextAbiTest, ContextHoldsTier1Fields) {
  vef_context_t ctx{};
  ctx.protocol = VEF_PROTOCOL_4;
  ctx.schema = "mydb";
  ctx.connection_id = 42;
  ctx.priv_user = "root";
  ctx.priv_host = "localhost";
  ctx.kill_status = VEF_KILL_QUERY;

  EXPECT_STREQ("mydb", ctx.schema);
  EXPECT_EQ(42u, ctx.connection_id);
  EXPECT_STREQ("root", ctx.priv_user);
  EXPECT_STREQ("localhost", ctx.priv_host);
  EXPECT_EQ(VEF_KILL_QUERY, ctx.kill_status);
}

}  // namespace villagesql_unittest
