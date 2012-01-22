#!/usr/bin/python

import sys, os, re, difflib, unicodedata, errno

class Colors:
	class Null:
		red = ''
		green = ''
		end = ''
	class ANSI:
		red = '\033[41;37;1m'
		green = '\033[42;37;1m'
		end = '\033[m'
	class HTML:
		red = '<span style="color:red">'
		green = '<span style="color:green">'
		end = '</span>'

	@staticmethod
	def Auto (argv = [], out = sys.stdout):
		if os.isatty (out.fileno ()):
			color = Colors.ANSI
		else:
			color = Colors.Null
		if "--color" in argv:
			argv.remove ("--color")
			color = Colors.ANSI
		if "--color=ansi" in argv:
			argv.remove ("--color=ansi")
			color = Colors.ANSI
		if "--color=html" in argv:
			argv.remove ("--color=html")
			color = Colors.HTML
		if "--no-color" in argv:
			argv.remove ("--no-color")
			color = Colors.Null
		return color


	@staticmethod
	def Default (argv = []):
		return Colors.ANSI


class FancyDiffer:

	diff_regex = re.compile ('([a-za-z0-9_]*)([^a-za-z0-9_]?)')

	@staticmethod
	def diff_lines (l1, l2, colors=Colors.Null):

		# Easy without colors
		if colors == Colors.Null:
			if l1 == l2:
				return [' ', l1]
			return ['-', l1, '+', l2]

		ss = [FancyDiffer.diff_regex.sub (r'\1\n\2\n', l).splitlines (True) for l in (l1, l2)]
		oo = ["",""]
		st = [False, False]
		for l in difflib.Differ().compare (*ss):
			if l[0] == '?':
				continue
			if l[0] == ' ':
				for i in range(2):
					if st[i]:
						oo[i] += colors.end
						st[i] = False
				oo = [o + l[2:] for o in oo]
				continue
			if l[0] == '-':
				if not st[0]:
					oo[0] += colors.red
					st[0] = True
				oo[0] += l[2:]
				continue
			if l[0] == '+':
				if not st[1]:
					oo[1] += colors.green
					st[1] = True
				oo[1] += l[2:]
		for i in range(2):
			if st[i]:
				oo[i] += colors.end
				st[i] = 0
		oo = [o.replace ('\n', '') for o in oo]
		if oo[0] == oo[1]:
			return [' ', oo[0], '\n']
		return ['-', oo[0], '\n', '+', oo[1], '\n']

	@staticmethod
	def diff_files (f1, f2, colors=Colors.Null):
		try:
			for (l1,l2) in zip (f1, f2):
				if l1 == l2:
					sys.stdout.writelines ([" ", l1])
					continue

				sys.stdout.writelines (FancyDiffer.diff_lines (l1, l2, colors))
			# print out residues
			for l in f1:
				sys.stdout.writelines (["-", colors.red, l1, colors.end])
			for l in f1:
				sys.stdout.writelines (["-", colors.green, l1, colors.end])
		except IOError as e:
			if e.errno != errno.EPIPE:
				print >> sys.stderr, "%s: %s: %s" % (sys.argv[0], e.filename, e.strerror)
				sys.exit (1)


class DiffFilters:

	@staticmethod
	def filter_failures (f):
		for l in f:
			if l[0] in '-+':
				yield l


class ShapeFilters:

	pass


class FilterHelpers:

	@staticmethod
	def filter_printer_function (filter_callback):
		def printer (f):
			for line in filter_callback (f):
				print line
		return printer

	@staticmethod
	def filter_printer_function_no_newline (filter_callback):
		def printer (f):
			for line in filter_callback (f):
				sys.stdout.writelines ([line])
		return printer


class UtilMains:

	@staticmethod
	def process_multiple_files (callback, mnemonic = "FILE"):

		if len (sys.argv) == 1:
			print "Usage: %s %s..." % (sys.argv[0], mnemonic)
			sys.exit (1)

		try:
			for s in sys.argv[1:]:
				callback (FileHelpers.open_file_or_stdin (s))
		except IOError as e:
			if e.errno != errno.EPIPE:
				print >> sys.stderr, "%s: %s: %s" % (sys.argv[0], e.filename, e.strerror)
				sys.exit (1)

	@staticmethod
	def process_multiple_args (callback, mnemonic):

		if len (sys.argv) == 1:
			print "Usage: %s %s..." % (sys.argv[0], mnemonic)
			sys.exit (1)

		try:
			for s in sys.argv[1:]:
				callback (s)
		except IOError as e:
			if e.errno != errno.EPIPE:
				print >> sys.stderr, "%s: %s: %s" % (sys.argv[0], e.filename, e.strerror)
				sys.exit (1)

	@staticmethod
	def filter_multiple_strings_or_stdin (callback, mnemonic, \
					      separator = " ", \
					      concat_separator = False):

		if len (sys.argv) == 1 or ('--stdin' in sys.argv and len (sys.argv) != 2):
			print "Usage:\n  %s %s...\nor:\n  %s --stdin" \
			      % (sys.argv[0], mnemonic, sys.argv[0])
			sys.exit (1)

		try:
			if '--stdin' in sys.argv:
				sys.argv.remove ('--stdin')
				while (1):
					line = sys.stdin.readline ()
					if not len (line):
						break
					print callback (line)
			else:
				args = sys.argv[1:]
				if concat_separator != False:
					args = [concat_separator.join (args)]
				print separator.join (callback (x) for x in (args))
		except IOError as e:
			if e.errno != errno.EPIPE:
				print >> sys.stderr, "%s: %s: %s" % (sys.argv[0], e.filename, e.strerror)
				sys.exit (1)


class Unicode:

	@staticmethod
	def decode (s):
		return '<' + u','.join ("U+%04X" % ord (u) for u in unicode (s, 'utf-8')).encode ('utf-8') + '>'

	@staticmethod
	def encode (s):
		s = re.sub (r"[<+>,\\uU\n	]", " ", s)
		s = re.sub (r"0[xX]", " ", s)
		return u''.join (unichr (int (x, 16)) for x in s.split (' ') if len (x)).encode ('utf-8')

	shorthands = {
		"ZERO WIDTH NON-JOINER": "ZWNJ",
		"ZERO WIDTH JOINER": "ZWJ",
		"NARROW NO-BREAK SPACE": "NNBSP",
		"COMBINING GRAPHEME JOINER": "CGJ",
		"LEFT-TO-RIGHT MARK": "LRM",
		"RIGHT-TO-LEFT MARK": "RLM",
		"LEFT-TO-RIGHT EMBEDDING": "LRE",
		"RIGHT-TO-LEFT EMBEDDING": "RLE",
		"POP DIRECTIONAL FORMATTING": "PDF",
		"LEFT-TO-RIGHT OVERRIDE": "LRO",
		"RIGHT-TO-LEFT OVERRIDE": "RLO",
	}

	@staticmethod
	def pretty_name (u):
		try:
			s = unicodedata.name (u)
		except ValueError:
			return "XXX"
		s = re.sub (".* LETTER ", "", s)
		s = re.sub (".* VOWEL SIGN (.*)", r"\1-MATRA", s)
		s = re.sub (".* SIGN ", "", s)
		s = re.sub (".* COMBINING ", "", s)
		if re.match (".* VIRAMA", s):
			s = "HALANT"
		if s in Unicode.shorthands:
			s = Unicode.shorthands[s]
		return s

	@staticmethod
	def pretty_names (s):
		s = re.sub (r"[<+>\\uU]", " ", s)
		s = re.sub (r"0[xX]", " ", s)
		s = [unichr (int (x, 16)) for x in re.split ('[, \n]', s) if len (x)]
		return u' + '.join (Unicode.pretty_name (x) for x in s).encode ('utf-8')


class FileHelpers:

	@staticmethod
	def open_file_or_stdin (f):
		if f == '-':
			return sys.stdin
		return file (f)


class Manifest:

	@staticmethod
	def read (s, strict = True):

		if not os.path.exists (s):
			if strict:
				print >> sys.stderr, "%s: %s does not exist" % (sys.argv[0], s)
				sys.exit (1)
			return

		s = os.path.normpath (s)

		if os.path.isdir (s):

			try:
				m = file (os.path.join (s, "MANIFEST"))
				items = [x.strip () for x in m.readlines ()]
				for f in items:
					for p in Manifest.read (os.path.join (s, f)):
						yield p
			except IOError:
				if strict:
					print >> sys.stderr, "%s: %s does not exist" % (sys.argv[0], os.path.join (s, "MANIFEST"))
					sys.exit (1)
				return
		else:
			yield s

	@staticmethod
	def update_recursive (s):

		for dirpath, dirnames, filenames in os.walk (s, followlinks=True):

			for f in ["MANIFEST", "README", "LICENSE", "COPYING", "AUTHORS", "SOURCES"]:
				if f in dirnames:
					dirnames.remove (f)
				if f in filenames:
					filenames.remove (f)
			dirnames.sort ()
			filenames.sort ()
			ms = os.path.join (dirpath, "MANIFEST")
			print "  GEN    %s" % ms
			m = open (ms, "w")
			for f in filenames:
				print >> m, f
			for f in dirnames:
				print >> m, f
			for f in dirnames:
				Manifest.update_recursive (os.path.join (dirpath, f))

if __name__ == '__main__':
	pass
