#!/usr/bin/env python
# -*- coding: utf-8 -*-
# MIT License. @snipsnipsnip

"""
混汞器/アマルガメータ (Konkohki/amalgamator): #include-only dumb C Preprocessor

Sqliteのamalgamationみたいなことをするツール。
(本家を見るなら tool/mksqlite3c.tcl 参照)
つまり、#includeだけを部分的に実行するCプリプロセッサ。
ファイルの見つからない#includeはエラーにせずそのままにしておくのが普通のプリプロセッサと逸脱しているところ。
(今のところ)static関数の衝突や#ifdefや#pragma onceなどは特に処理しない。
一応ブラックリストとホワイトリストで処理するinclude対象を限定できるようにしたがあまり役に立っている感じがしない。

概念的にはこんな具合。
 '...#include "hoge.c"...'
                     ↓ scan includes
 ('...', '#include "hoge.c"', '...')
                     ↓ load hoge.c
 ('...', ',,,#include "hoge.impl",,,', '...')
                     ↓ scan includes
 ('...', (',,,', '#include "hoge.impl"', ',,,'), '...')
                     ↓ load hoge.impl
 ('...', (',,,', <hoge.implの内容>, ',,,'), '...')
                     ↓ traverse and join
    '...,,,<hoge.implの内容>,,,...'

>>> try: Amalgamator.from_argv(("amalgamator.py", "-h")).run()
... except SystemExit: pass
usage: amalgamator.py [-h] [-o FILE] [-v] [-I DIR] [-w GLOB] [-b GLOB]
                      [-e NAME] [-E NAME] [-c] [-V]
                      FILE [FILE ...]
<BLANKLINE>
Recursively and partially preprocess some of the #include directives in a C
source file.
<BLANKLINE>
positional arguments:
  FILE                  input file(s) to preprocess
<BLANKLINE>
optional arguments:
  -h, --help            show this help message and exit
  -o FILE, --out FILE   Output file
  -v, --verbose         if set, report verbosely. Repeat like -vv for
                        increased verboseness.
  -I DIR, --include DIR
                        search path for #include
  -w GLOB, --whitelist GLOB
                        allowed #include files which will be expanded.
  -b GLOB, --blacklist GLOB
                        denied #include files which won't be expanded. Note
                        that you need to specify the real file path. Specify
                        like -b '**/stdio.h' to deny all <stdio.h>
  -e NAME, --input-encoding NAME
                        possible source file encoding(s). Defaults to utf-8
  -E NAME, --output-encoding NAME
                        possible source file encoding(s). Defaults to utf-8
  -c, --comment         if set, inserts comment before and after #include
                        processed
  -V, --version         show program's version number and exit
"""

import argparse
import codecs
import fnmatch
import logging
import os
import os.path
import re
import sys

VERSION = '0.1.0'
DEFAULT_ENCODING = 'utf-8'
LOG = logging.getLogger('konkohki')
LOG.setLevel(logging.CRITICAL)
LOG.addHandler(logging.StreamHandler(codecs.getwriter(DEFAULT_ENCODING)(sys.stderr)))

def set_logger_verbosity(verbosity):
    """
    Sets log level of amalgamator.LOG.

    >>> set_logger_verbosity(0); LOG.getEffectiveLevel() == logging.ERROR
    True
    >>> set_logger_verbosity(1); LOG.getEffectiveLevel() == logging.WARNING
    True
    >>> set_logger_verbosity(2); LOG.getEffectiveLevel() == logging.INFO
    True
    >>> set_logger_verbosity(3); LOG.getEffectiveLevel() == logging.DEBUG
    True
    >>> set_logger_verbosity(4); LOG.getEffectiveLevel() == logging.DEBUG
    True
    """

    if verbosity == 0:
        LOG.setLevel(logging.ERROR)
    elif verbosity == 1:
        LOG.setLevel(logging.WARNING)
    elif verbosity == 2:
        LOG.setLevel(logging.INFO)
    else:
        LOG.setLevel(logging.DEBUG)


class IndentFilter(logging.Filter):
    """
    Adds indentation to log messages.

    >>> log = logging.getLogger("test")
    >>> log.addHandler(logging.StreamHandler(sys.stdout))
    >>> log.warn("foo")
    foo
    >>> with IndentFilter(log): log.warn("foo")
      foo
    >>> log.warn("foo")
    foo
    >>> with IndentFilter(log):
    ...   with IndentFilter(log): log.warn("foo")
        foo
    >>> log.warn("foo")
    foo
    """

    def __init__(self, logger):
        """
        >>> log = logging.getLogger("test")
        >>> filter = IndentFilter(log)
        >>> filter.indent
        0
        >>> filter.logger == log
        True
        """

        self.indent = 0
        self.logger = logger

    def filter(self, record):
        record.msg = "  " + record.msg
        return True

    def __enter__(self):
        self.logger.addFilter(self)

    def __exit__(self, exc_type, exc_value, traceback):
        self.logger.removeFilter(self)


class SourceDir(object):
    """
    Represents source directories.
    Handles stuff around #include search path, whitelist, and blacklist.

    >>> path = os.path.abspath(__file__)
    >>> name = os.path.basename(path)
    >>> dir = os.path.dirname(path)
    >>> srcdir = SourceDir([dir], ['utf_8'], [], [])
    >>> srcdir.find_file(name, '.') is not None
    True
    >>> srcdir.find_file('some_nonexistent_file', '.') is None
    True
    >>> srcdir = SourceDir([dir], ['utf_8'], [], [])
    >>> srcdir.find_file('some_nonexistent_file', '.') is None
    True
    >>> isinstance(srcdir.find_file(name, '.'), SourceFile)
    True
    >>> srcdir.find_file(name, '.').path == name
    True
    """

    def __init__(self, dirs, source_encodings, whitelist, blacklist):
        self.dirs = dirs
        self.source_encodings = source_encodings
        self.whitelist = whitelist
        self.blacklist = blacklist

    def find_file(self, key, included_from):
        """Search a file for an #include directive.
        The source file where the #included from matters, since its parent directory is sought first.
        Return the content of the file wrapped in a SourceFile if found.
        Return None otherwise.
        """

        path = os.path.normcase(os.path.normpath(key))

        resolved = self._resolve_source_path(path, included_from)
        if resolved:
            content = self._read_content(resolved)
            return SourceFile(resolved, content)
        else:
            return None

    def _resolve_source_path(self, path, included_from):
        """Search file that matches path.
        The file path where the directive is from does matter.
        Return None if it's not found, not in the whitelist, or in the blacklist.
        Return the content of the file otherwise.
        """

        existing = self._find_from_dirs(path, included_from)

        if not existing:
            LOG.debug('ignored "%s" for "%s"', path, included_from)
            return None
        elif len(existing) > 1:
            LOG.info('conflicting files for "%s": %s, resolving to "%s"', path, existing, existing[0])

        return existing[0]

    def _find_from_dirs(self, path, included_from):
        """List candidates for #included file path."""

        dirs = [os.path.dirname(included_from)] + self.dirs
        candidates = [os.path.join(d, path) for d in dirs]
        existing = [c for c in candidates if os.path.exists(c) and self._is_allowed_in_list(c)]

        return existing

    def _is_allowed_in_list(self, path):
        """Return True if path is allowed in both whitelist and blacklist."""

        if self.whitelist and not self._is_in_list(path, self.whitelist):
            LOG.info('"%s" is not in the whitelist', path)
            return False
        elif self.blacklist and self._is_in_list(path, self.blacklist):
            LOG.info('"%s" is in the blacklist', path)
            return False
        else:
            return True

    def _is_in_list(self, path, list):
        """Return True if path matches in any glob in list."""

        return any(fnmatch.fnmatch(path, glob) for glob in list)

    def _read_content(self, path):
        """Try to read content of path with each encoding in source_encodings."""

        for encoding in self.source_encodings:
            try:
                with codecs.open(path, 'rb', encoding) as f:
                    return f.read()
            except ValueError:
                LOG.debug('reading "%s" failed with the encoding %s', path, encoding)

        raise IOError('Failed to determine the encoding of file "%s"' % path)


class SourceFile(object):
    """Represents a source file."""

    INCLUDE_RE = re.compile(r'^[^\S\n]*#[^\S\n]*include[^\S\n]*["<]([^">]+)[">][^\S\n]*', re.IGNORECASE | re.MULTILINE)

    def __init__(self, path, content):
        self.path = path
        self.content = content

    def preprocess(self, out, srcdir, need_comment):
        """Preprocess this file and print to out."""

        self._log_marker(out, need_comment, '"%s" begin', self.path)

        state = self._init_state()

        with IndentFilter(LOG):
            for match in self.INCLUDE_RE.finditer(self.content):
                state = self._preprocess_include(match, out, srcdir, need_comment, state)

            self._preprocess_tail(out, srcdir, need_comment, state)

        self._log_marker(out, need_comment, '"%s" end', self.path)

    def _init_state(self):
        """Return initial parameters to pass around in _preprocess_*() subroutines."""

        return 0, 1

    def _preprocess_include(self, match, out, srcdir, need_comment, state):
        """Process an #include directive captured by SourceFile.INCLUDE_RE regexp."""

        prev_end, line = state

        included = match.group(1)
        part = self.content[prev_end:match.start()]
        spliced = srcdir.find_file(included, self.path)

        if spliced is None:
            return state

        newlines = part.count("\n")

        if part != "\n":
            self._log_marker(out, need_comment, 'line %s..%s of "%s"', line, line + newlines - 1, self.path)
            self._mark_line(out, line)
        out.write(part)

        spliced.preprocess(out, srcdir, need_comment)

        line += newlines
        prev_end = match.end()

        return prev_end, line

    def _preprocess_tail(self, out, srcdir, need_comment, state):
        """Print the rest of the file after all #includes."""

        prev_end, line = state

        part = self.content[prev_end:]
        newlines = part.count("\n")

        if part != "\n":
            self._log_marker(out, need_comment, 'line %s..%s of "%s"', line, line + newlines, self.path)
            self._mark_line(out, line)
        out.write(part)

    def _mark_line(self, out, line):
        out.write('#line %d "%s"\n' % (line, self.path.replace(os.path.sep, '/')))

    def _log_marker(self, out, need_comment, message, *args):
        """Log message to output file and/or logger."""

        msg = message % args
        LOG.info(msg)
        if need_comment:
            out.write('/* amalgamator.py: %s */\n' % msg)


class Amalgamator(object):
    """Main class."""

    @classmethod
    def from_argv(cls, argv):
        """Return an Amalgamator configured with argv."""

        parser = cls._make_parser(argv)
        self = Amalgamator()

        parser.parse_args(argv[1:], namespace=self)

        set_logger_verbosity(len(self.verbose) if self.verbose else 0)

        if not self.input_encoding:
            self.input_encoding.append(DEFAULT_ENCODING)

        self.out = codecs.getwriter(self.output_encoding)(self.out)

        LOG.debug("parsed arguments: %s", self)

        return self

    @classmethod
    def _make_parser(cls, argv):
        """Return argparse.ArgumentParser."""

        parser = argparse.ArgumentParser(prog=argv[0], description='Recursively and partially preprocess some of the #include directives in a C source file.')
        parser.add_argument('-o', '--out', metavar='FILE', type=argparse.FileType('wb'), default=sys.stdout, help='Output file')
        parser.add_argument('-v', '--verbose', action='append_const', const=None, help='if set, report verbosely. Repeat like -vv for increased verboseness.')
        parser.add_argument('-I', '--include', metavar='DIR', action='append', default=[], help='search path for #include')
        parser.add_argument('-w', '--whitelist', metavar='GLOB', action='append', default=[], help='allowed #include files which will be expanded.')
        parser.add_argument('-b', '--blacklist', metavar='GLOB', action='append', default=[], help='denied #include files which won\'t be expanded. Note that you need to specify the real file path. Specify like -b \'**/stdio.h\' to deny all <stdio.h>')
        parser.add_argument('-e', '--input-encoding', metavar='NAME', action='append', default=[], help='possible source file encoding(s). Defaults to %s' % DEFAULT_ENCODING)
        parser.add_argument('-E', '--output-encoding', metavar='NAME', default=DEFAULT_ENCODING, help='possible source file encoding(s). Defaults to %s' % DEFAULT_ENCODING)
        parser.add_argument('-c', '--comment', action='store_true', default=False, help='if set, inserts comment before and after #include processed')
        parser.add_argument('-V', '--version', action='version', version='%(prog)s ' + VERSION)
        parser.add_argument('root_files', metavar='FILE', nargs='+', help='input file(s) to preprocess')

        return parser

    def run(self):
        assert self.include is not None
        assert self.input_encoding is not None
        assert self.whitelist is not None
        assert self.blacklist is not None
        assert self.root_files is not None
        assert self.output_encoding is not None
        assert self.out is not None
        assert self.comment is not None

        srcdir = SourceDir(self.include, self.input_encoding, self.whitelist, self.blacklist)

        for root_file in self.root_files:
            root = srcdir.find_file(root_file, root_file)
            if root is None:
                raise IOError('root file "%s" not found' % root_file)
            else:
                root.preprocess(self.out, srcdir, self.comment)

if __name__ == '__main__':
    Amalgamator.from_argv(sys.argv).run()
