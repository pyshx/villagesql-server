// Copyright (c) 2026 VillageSQL Contributors
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 2.0,
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License, version 2.0, for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

// Exercises the vsql::preview::session capability. Each VDF returns one
// session attribute, or NULL when the attribute is unavailable.

#include <villagesql/preview/session.h>
#include <villagesql/vsql.h>

using namespace vsql;
using SessionCapability = vsql::preview_session::SessionCapability;

static SessionCapability g_session;

static void emit(const SessionCapability::StringResult &r, StringResult out) {
  if (r.status == SessionCapability::Status::OK) {
    out.set(r.value);
  } else {
    out.set_null();
  }
}

void sc_query_text(StringResult out) { emit(g_session.query_text(), out); }
void sc_host_or_ip(StringResult out) { emit(g_session.host_or_ip(), out); }
void sc_login_user(StringResult out) { emit(g_session.login_user(), out); }
void sc_active_roles(StringResult out) { emit(g_session.active_roles(), out); }
void sc_sql_command(StringResult out) { emit(g_session.sql_command(), out); }
void sc_proxy_user(StringResult out) { emit(g_session.proxy_user(), out); }

void sc_da_status(IntResult out) {
  const auto r = g_session.da_status();
  if (r.status == SessionCapability::Status::OK) {
    out.set(static_cast<long long>(r.value));
  } else {
    out.set_null();
  }
}

VEF_GENERATE_ENTRY_POINTS(make_extension()
                              .func(make_func<&sc_query_text>("query_text")
                                        .returns(STRING)
                                        .no_params()
                                        .build())
                              .func(make_func<&sc_host_or_ip>("host_or_ip")
                                        .returns(STRING)
                                        .no_params()
                                        .build())
                              .func(make_func<&sc_login_user>("login_user")
                                        .returns(STRING)
                                        .no_params()
                                        .build())
                              .func(make_func<&sc_active_roles>("active_roles")
                                        .returns(STRING)
                                        .no_params()
                                        .build())
                              .func(make_func<&sc_da_status>("da_status")
                                        .returns(INT)
                                        .no_params()
                                        .build())
                              .func(make_func<&sc_sql_command>("sql_command")
                                        .returns(STRING)
                                        .no_params()
                                        .build())
                              .func(make_func<&sc_proxy_user>("proxy_user")
                                        .returns(STRING)
                                        .no_params()
                                        .build())
                              .with(g_session))
