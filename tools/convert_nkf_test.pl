#!/usr/bin/perl

use strict;
use warnings;

my $DESCRIPTION = <<'EOS';
/*
  This file is generated from nkf_test.pl.

  Implement following functions:

    void description(const char *message);
    void test_input(const char *nkf_options, int input_length, const char *input);
    void test_output(int answer_length, const char *answer);

  test_input() is followed by one or more calls of test_output().
  String lengths are important since string data, except nkf_options, often
  contain one or more non-terminating '\0's.

  Example:

    description("`nkf -foo` must convert 'bar' into either 'hoge' or 'fuga'.");
    test_input("-foo", 3, "bar");
    test_output(4, "hoge");
    test_output(4, "fuga");
    description("`nkf -baz` must convert 'moga' into 'foge'.");
    test_input("-baz", 3, "moga");
    test_output(4, "foge");

  Note: In original test.pl test runner, there are special treatments
  if $nkf_options matches regexp /-\w+m[NS]/, in which case
  space letters (' ') in output and answers are ignored.
*/
EOS

my $LOG = '';
my @CASES;

main() if __FILE__ eq $0;

sub main {
  my $test_pl = $ARGV[0] or die "usage: $0 nkf_test.pl\n\nUnpacks test cases in nkf_test.pl into C-like expressions.\n";

  {
    local *STDOUT;
    open STDOUT, '>', \$LOG;
    do $test_pl;
  }

  die "couldn't parse '$test_pl': $@\n" if $@;
  die "couldn't do '$test_pl': $!\n" if $!;

  print $DESCRIPTION;
  for (@CASES) {
    print translate_case(@$_), "\n";
  }
}

sub translate_case {
  my ($name, $head, @args) = @_;
  my $arg = quote(escape(strip($head)));

  if ($name =~ /\Atestcase\z/) {
    my ($input, @answers) = map { length($_) . ', ' . quote(escape($_)); } @args;

    join "\n", map { "$_->[0]($_->[1]);" } [test_input => "$arg, $input"], map { [test_output => $_] } @answers;
  } else {
    "$name($arg);";
  }
}

sub quote {
  qq("$_[0]");
}

sub escape {
  local ($_) = @_;
  s/[^- \(\)\/_\[\]:=\n\ra-zA-Z]/sprintf '\%03o', ord $&/gse;
  s/\n/\\n/g;
  s/\r/\\r/g;
  $_;
}

sub strip {
  local ($_) = @_;
  s/\A\s+|\s+\z//;
  $_;
}

sub test {
  @_ >= 3 or die "Expected more than 3 arguments, but got: " . scalar @_;
  my ($nkf_options, $input, @answers) = @_;
  my $desc = flush_log();

  for my $line (split /\n/, $desc) {
    push @CASES, ["description", $line];
  }

  push @CASES, ["testcase", $nkf_options, $input, @answers];
}

sub flush_log {
  my $part = strip($LOG);
  $LOG = '';
  seek STDOUT, 0, 0;
  $part;
}

sub command_tests {
  @_ % 3 == 0 or die "Expected arguments of a multiple of 3, but got: " . scalar @_;
  for (0..(@_ / 3 - 1)) {
    test(@_[$_ * 3..$_ * 3 + 2]);
  }
  1;
}
