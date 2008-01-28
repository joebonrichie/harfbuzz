/*
 * Copyright (C) 2007,2008  Red Hat, Inc.
 *
 *  This is part of HarfBuzz, an OpenType Layout engine library.
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
 *
 * Red Hat Author(s): Behdad Esfahbod
 */

#define HB_OT_LAYOUT_CC
#include "hb-ot-layout-open-private.h"
#include "hb-ot-layout-gdef-private.h"
#include "hb-ot-layout-gsub-private.h"

#include <stdlib.h>
#include <stdio.h>

int
main (int argc, char **argv)
{
  if (argc != 2) {
    fprintf (stderr, "usage: %s font-file.ttf\n", argv[0]);
    exit (1);
  }

  GMappedFile *mf = g_mapped_file_new (argv[1], FALSE, NULL);
  const char *font_data = g_mapped_file_get_contents (mf);
  int len = g_mapped_file_get_length (mf);
  
  printf ("Opened font file %s: %d bytes long\n", argv[1], len);
  
  const OpenTypeFontFile &ot = OpenTypeFontFile::get_for_data (font_data);

  switch (ot.get_tag()) {
  case OpenTypeFontFile::TrueTypeTag:
    printf ("OpenType font with TrueType outlines\n");
    break;
  case OpenTypeFontFile::CFFTag:
    printf ("OpenType font with CFF (Type1) outlines\n");
    break;
  case OpenTypeFontFile::TTCTag:
    printf ("TrueType Collection of OpenType fonts\n");
    break;
  default:
    printf ("Unknown font format\n");
    break;
  }

  int num_fonts = ot.get_face_count ();
  printf ("%d font(s) found in file\n", num_fonts);
  for (int n_font = 0; n_font < num_fonts; n_font++) {
    const OpenTypeFontFace &font = ot.get_face (n_font);
    printf ("Font %d of %d:\n", n_font+1, num_fonts);

    int num_tables = font.get_table_count ();
    printf ("  %d table(s) found in font\n", num_tables);
    for (int n_table = 0; n_table < num_tables; n_table++) {
      const OpenTypeTable &table = font.get_table (n_table);
      printf ("  Table %2d of %2d: %.4s (0x%08lx+0x%08lx)\n", n_table+1, num_tables,
	      (const char *)table.get_tag(), table.get_offset(), table.get_length());

      if (table.get_tag() == GSUBGPOS::GSUBTag || table.get_tag() == GSUBGPOS::GPOSTag) {
        const GSUBGPOS &g = GSUBGPOS::get_for_data (ot.get_table_data (table));

	const ScriptList &scripts = g.get_script_list();
	int num_scripts = scripts.get_len ();
	printf ("    %d script(s) found in table\n", num_scripts);
	for (int n_script = 0; n_script < num_scripts; n_script++) {
	  const Script &script = scripts[n_script];
	  printf ("    Script %2d of %2d: %.4s\n", n_script+1, num_scripts,
	          (const char *)scripts.get_tag(n_script));

	  if (!script.has_default_language_system())
	    printf ("      No default language system\n");
	  int num_langsys = script.get_len ();
	  printf ("      %d language system(s) found in script\n", num_langsys);
	  for (int n_langsys = 0; n_langsys < num_langsys; n_langsys++) {
	    const LangSys &langsys = script[n_langsys];
	    printf ("      Language System %2d of %2d: %.4s; %d features\n", n_langsys+1, num_langsys,
	            (const char *)script.get_tag(n_langsys),
		    langsys.get_len ());
	    if (!langsys.get_required_feature_index ())
	      printf ("        No required feature\n");
	  }
	}
        
	const FeatureList &features = g.get_feature_list();
	int num_features = features.get_len ();
	printf ("    %d feature(s) found in table\n", num_features);
	for (int n_feature = 0; n_feature < num_features; n_feature++) {
	  const Feature &feature = features[n_feature];
	  printf ("    Feature %2d of %2d: %.4s; %d lookup(s)\n", n_feature+1, num_features,
	          (const char *)features.get_tag(n_feature),
		  feature.get_len());
	}
        
	const LookupList &lookups = g.get_lookup_list();
	int num_lookups = lookups.get_len ();
	printf ("    %d lookup(s) found in table\n", num_lookups);
	for (int n_lookup = 0; n_lookup < num_lookups; n_lookup++) {
	  const Lookup &lookup = lookups[n_lookup];
	  printf ("    Lookup %2d of %2d: type %d, flags %04x\n", n_lookup+1, num_lookups,
	          lookup.get_type(), lookup.get_flag());
	}
      } else if (table.get_tag() == "GDEF") {
        const GDEF &gdef = GDEF::get_for_data (ot[table]);

	for (int glyph = 0; glyph < 1; glyph++)
	  printf ("    glyph %d has class %d and mark attachment type %d\n",
		  glyph,
		  gdef.get_glyph_class (glyph),
		  gdef.get_mark_attachment_type (glyph));
      }
    }
  }

  return 0;
}
