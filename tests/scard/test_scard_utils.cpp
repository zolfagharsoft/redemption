/*
   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
   Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2020
   Author(s): 
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "scard/scard_utils.hpp"


///////////////////////////////////////////////////////////////////////////////


RED_AUTO_TEST_CASE(test_scard_utils_static_handle_table)
{
   enum {
      max_handle_count = 32,
      reserved_value = 12
   };

   static_handle_table<
      int, max_handle_count, reserved_value
   > table;

   for (int i = 0; i < max_handle_count; ++i)
   {
      auto handle = table.allocate();

      if (i < reserved_value)
      {
         RED_CHECK_EQUAL(i, handle);
      }
      else
      {
         RED_CHECK_EQUAL(i + 1, handle);
      }
   }
   
   for (int i = 0; i < max_handle_count; ++i)
   {
      if (i < reserved_value)
      {
         RED_CHECK(table.deallocate(i));
      }
      else
      if (i == reserved_value)
      {
         RED_CHECK(!table.deallocate(i));
      }
      else
      {
         RED_CHECK(table.deallocate(i));
      }
   }
}
