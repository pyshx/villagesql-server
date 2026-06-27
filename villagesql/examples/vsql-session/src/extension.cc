// Copyright (c) 2026 VillageSQL Contributors
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 2.0,
// as published by the Free Software Foundation.
//
// This program is designed to work with certain software (including
// but not limited to OpenSSL) that is licensed under separate terms,
// as designated in a particular file or component or in included license
// documentation.  The authors of MySQL hereby grant you an additional
// permission to link the program and your derivative works with the
// separately licensed software that they have either included with
// the program or referenced in the documentation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License, version 2.0, for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

// Demonstrates read-only session context for VillageSQL extensions:
//   - protocol gating via Session::available()
//   - fail-closed behavior when no session is bound
//   - copy-before-retain when building owned strings from server-owned views
//   - combining Tier 1 (Session parameter) with the Tier 2 session capability

#include <string>

#include <villagesql/preview/session.h>
#include <villagesql/vsql.h>

using namespace vsql;
using SessionCapability = vsql::preview_session::SessionCapability;

static SessionCapability g_session;

// who_am_i() -> "priv_user@priv_host" of the executing session.
// Fails closed (raises an error) when session context is unavailable.
void who_am_i(Session s, StringResult out) {
  if (!s.available()) {
    out.error("session context unavailable");  // fail closed
    return;
  }
  // Copy-before-retain: schema()/priv_user()/priv_host() are valid only for
  // this call, so build an owned std::string from them.
  std::string id(s.priv_user());
  id.push_back('@');
  id.append(s.priv_host());
  out.set(id);
}

// audit_line() -> "priv_user: <current query text>", combining the Tier 1
// identity (hot path) with the Tier 2 query_text getter.
void audit_line(Session s, StringResult out) {
  if (!s.available()) {
    out.error("session context unavailable");  // fail closed
    return;
  }
  std::string line(s.priv_user());
  line.append(": ");
  const auto qt = g_session.query_text();
  if (qt.status == SessionCapability::Status::OK) {
    line.append(qt.value);
  } else {
    line.append("<unknown>");
  }
  out.set(line);
}

VEF_GENERATE_ENTRY_POINTS(
    make_extension()
        .func(make_func<&who_am_i>("who_am_i").returns(STRING).build())
        .func(make_func<&audit_line>("audit_line").returns(STRING).build())
        .with(g_session))
