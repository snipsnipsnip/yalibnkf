#!/usr/bin/perl

use strict;
use warnings;
use File::Slurp;
use IPC::Run3;

main() if __FILE__ eq $0;

sub main {
  my $yankf = $ARGV[0];
  my $test_cases = $ARGV[1] or die "usage: $0 yankf.exe test_cases.inc\n\nRuns nkf test, generated from convert_nkf_test.pl, using pipelines.\n";

  my @cases = load_test_cases($test_cases);
  my $error_count;

  for (@cases) {
    if (run_test($yankf, $_)) {
      print "Ok\n";
    } else {
      print "Fail\n";
      $error_count++;
    }
  }

  if ($error_count > 1) {
    print $error_count, " errors were found.\n";
  } elsif ($error_count == 1) {
    print "1 error was found.\n";
  } else {
    print "All tests are succeeded.\n";
  }

  exit 1 if $error_count > 0;
}

sub load_test_cases {
  my $src = read_file($_[0]);

  $src =~ s/.*\*\///s;
  $src =~ s/[$@%]/\\\0/g;
  $src =~ s/^([a-z_]+)\(/\$$1->(/gm;

  my @cases;
  my @descs;

  my $description = sub {
    push @descs, @_;
  };

  my $test_input = sub {
    if (@descs) {
      push @cases, {description => [@descs], tests => []};
      @descs = ();
    }
    my ($options, undef, $input) = @_;
    push @{$cases[$#cases]->{tests}}, {options => $options, input => $input, answers => []};
  };

  my $test_output = sub {
    my (undef, $answer) = @_;
    push @{$cases[$#cases]->{tests}->[-1]->{answers}}, $answer;
  };

  eval $src;
  die "Failed loading test cases: $@" if $@;
  die "Test case format error: dangling descriptions" if @descs;

  @cases;
}

sub run_test {
  my ($yankf, $bundle) = @_;

  printf "%-40s", $_ for @{$bundle->{description}};

  for my $case (@{$bundle->{tests}}) {
    my $tolerant = $case->{options} =~ /-\w+m[NS]/;
    my ($out, $err) = run_nkf($yankf, $case->{options}, $case->{input});

    warn "nkf warned: $err\n" if $err;

    unless (equal($tolerant, $case->{answers}, $out)) {
      return 0;
    }
  }

  return 1;
}

sub equal {
  my ($tolerant, $answers_ref, $actual) = @_;
  my @answers = @$answers_ref;

  if ($tolerant) {
    $actual =~ s/ //g;
    s/ //g for @answers;
  }

  for (@answers) {
    return 1 if $actual eq $_;
  }

  0;
}

sub run_nkf {
  my ($nkf, $nkf_opt, $input) = @_;
  my ($out, $err);
  run3([$nkf, $nkf_opt], \$input, \$out, \$err, {
    binmode_stdin => ':raw',
    binmode_stdout => ':raw',
    binmode_stderr => ':raw',
  });
  ($out, $err);
}
