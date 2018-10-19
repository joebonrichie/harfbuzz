/*
 * Copyright © 2018  Ebrahim Byagowi
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include "hb-test.h"

/* Unit tests for hb-map.h */


static void
test_map_basic (void)
{
  hb_map_t *empty = hb_map_get_empty ();
  g_assert (!hb_map_is_empty (empty)); /* this feels wrong */
  g_assert (!hb_map_allocation_successful (empty));
  hb_map_destroy (empty);

  hb_map_t *m = hb_map_create ();
  g_assert (hb_map_allocation_successful (m));
  g_assert (!hb_map_is_empty (m)); /* this as well */

  hb_map_set (m, 213, 223);
  hb_map_set (m, 643, 675);
  g_assert_cmpint (hb_map_get_population (m), ==, 2);

  g_assert_cmpint (hb_map_get (m, 213), ==, 223);
  g_assert (hb_map_get (m, 123) == -1);
  g_assert (hb_map_has (m, 213));

  hb_map_del (m, 213);
  g_assert (!hb_map_has (m, 213));

  g_assert_cmpint (hb_map_get (m, 643), ==, 675);
  hb_map_set (m, 237, 673);
  g_assert (hb_map_has (m, 237));
  hb_map_clear (m);
  g_assert (!hb_map_has (m, 237));
  g_assert (!hb_map_has (m, 643));
  g_assert_cmpint (hb_map_get_population (m), ==, 0);

  hb_map_destroy (m);
}

#define true 1
#define false 0

static void
test_map_userdata ()
{
  hb_map_t *m = hb_map_create ();

  hb_user_data_key_t key[2];
  int *data = (int *) malloc (sizeof (int));
  *data = 3123;
  hb_map_set_user_data (m, &key[0], data, free, true);
  g_assert_cmpint (*((int *) hb_map_get_user_data (m, &key[0])), ==, 3123);

  int *data2 = (int *) malloc (sizeof (int));
  *data2 = 6343;
  hb_map_set_user_data (m, &key[0], data2, free, false);
  g_assert_cmpint (*((int *) hb_map_get_user_data (m, &key[0])), ==, 3123);
  hb_map_set_user_data (m, &key[0], data2, free, true);
  g_assert_cmpint (*((int *) hb_map_get_user_data (m, &key[0])), ==, 6343);

  hb_map_destroy (m);
}

static void
test_map_refcount ()
{
  hb_map_t *m = hb_map_create ();
  hb_map_set (m, 213, 223);
  g_assert_cmpint (hb_map_get (m, 213), ==, 223);

  hb_map_t *m2 = hb_map_reference (m);
  hb_map_destroy (m);
  g_assert (hb_map_has (m, 213));
  g_assert (hb_map_has (m2, 213));

  hb_map_destroy (m2);
  g_assert (hb_map_has (m, 213)); /* shouldn't these return false? */
  g_assert (hb_map_has (m2, 213));
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_map_basic);
  hb_test_add (test_map_userdata);

  return hb_test_run();
}