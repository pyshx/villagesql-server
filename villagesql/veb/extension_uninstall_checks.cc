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

#include "villagesql/veb/extension_uninstall_checks.h"

namespace villagesql {

bool check_for_indexes_of_extension(
    const ExtensionEntry & /*ext_entry*/,
    const std::vector<const IndexEntry *> & /*all_indexes*/) {
  // Task 1 stub: returns false unconditionally so the failing unit test
  // pins the predicate. Replaced with real enumeration in Task 2.
  return false;
}

}  // namespace villagesql
