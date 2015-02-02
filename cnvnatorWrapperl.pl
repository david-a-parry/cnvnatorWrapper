#!/usr/bin/perl
use strict;
use warnings;
use threads;
use Shell::Source;
use Getopt::Long;
use POSIX qw/strftime/;
use Pod::Usage;
use File::Basename;

my %opts = ();
my @input; 
GetOptions(\%opts,
        "outdir=s",
        "input=s{,}" => \@input,
        "window=i",
        "fasta=s",
        "root_dir=s",
        "cnvnator=s",
        "vcf_converter=s",
        "dry_run",
        "help",
        "manual",
        ) or pod2usage(-exitval => 2, -message => "Syntax error") ;
pod2usage (-verbose => 2) if $opts{manual};
pod2usage (-verbose => 1) if $opts{help};
pod2usage(-exitval => 2, -message => "Syntax error") if not @input; 


my ($script, $path) = fileparse($0);
my $cnvnator = "$path/CNVnator_v0.3/src/cnvnator";
if ($opts{cnvnator}){
    $cnvnator = $opts{cnvnator};
}

my $vcf_converter = "$path/CNVnator_v0.3/cnvnator2VCF.pl";
if ($opts{vcf_converter}){
    $vcf_converter = $opts{vcf_converter};
}

my $window = 100;
if ($opts{window}){
    $window = $opts{window};
}
my $root_dir = "$path/root";
if ($opts{root_dir}){
    $root_dir = $opts{root_dir};
}
my $fasta_dir = "$path/fasta";
if ($opts{fasta}){
    $fasta_dir = $opts{fasta};
}
foreach my $bam (@input){
    die "$bam does not exist!\n" if not -e $bam;
}
print STDERR "Sourcing root env variables from $root_dir.\n";
my $bsh = Shell::Source->new(shell => "bash", file => "$root_dir/bin/thisroot.sh");
$bsh->inherit();
print STDERR $bsh->output;
print STDERR $bsh->shell;
my $c = 0;
my %commands = ();
my @exts = qw (.bam .sam .cram .BAM .SAM .CRAM);
foreach my $bam (@input){
    my $n = 0;
    my ($name, $path, $suffix) = fileparse($bam, @exts);
    my $output_prefix = $name;
    my $vcf_out = "var.cnvnator.$name.root.calls.vcf";
    if ($opts{outdir}){
        $opts{outdir} =~ s/\/$//;
        $output_prefix = "$opts{outdir}/$name";
        $vcf_out = "$opts{outdir}/var.cnvnator.$name.root.calls.vcf";
    }
    $commands{$bam}->{$n++} = "$cnvnator -root $output_prefix.root -tree $bam";
    $commands{$bam}->{$n++} = "$cnvnator -root $output_prefix.root -his $window -d $fasta_dir";
    $commands{$bam}->{$n++} = "$cnvnator -root $output_prefix.root -stat $window";
    $commands{$bam}->{$n++} = "$cnvnator -root $output_prefix.root -partition $window";
    $commands{$bam}->{$n++} = "$cnvnator -root $output_prefix.root -call $window > $output_prefix.root.calls";
    $commands{$bam}->{$n++} = "$vcf_converter $output_prefix.root.calls $fasta_dir > $vcf_out";
    $c = $c < $n ? $n : $c;
}

for (my $i = 0; $i < $c ; $i++){
    foreach my $k (keys %commands){
        my $thr = threads->create(\&runCommand, \%commands, $k, $i);
    }
    while (threads->list(threads::all)){
        foreach my $joinable (threads->list(threads::joinable)){
            $joinable->join();
        }
        sleep 2 unless $opts{dry_run};
    }
}


#################################
sub runCommand{
	my ($commands, $file, $i) = @_;
    my $time = strftime "%a %b %e %Y: %H:%M:%S", localtime;
    my $com = $commands->{$file}->{$i};
	print STDERR "\n>[$time] Starting command " . (1 + $i) .  " for file $file.\n";
	print STDERR ">>$com\n";
    my $e;
	my $exit_status = 0 ;
    if (not  $opts{dry_run}){
        system $com;
        $exit_status = $?;
    }
    my $end_time = strftime "%a %b %e %Y: %H:%M:%S", localtime;
    if ($exit_status == 0){
        print STDERR "\n>Command " . (1 + $i) ." for file $file finished (started $time, finished $end_time)\n";
    }else{
        print STDERR "\n>Command " . (1 + $i) . " for file $file exited with error $exit_status (started $time, finished $end_time)\n";
	}
	return $exit_status;
}

=head1 NAME

cnvnatorWrapperl.pl - run CNVnator on one or more bam files automagically 

=head1 SYNOPSIS

    cnvnatorWrapperl.pl -i input.bam [options]

    cnvnatorWrapperl.pl --help for help message

    cnvnatorWrapperl.pl --manual for manual page

=head1 ARGUMENTS

=over 8

=item B<-i    --input>

One or more bam files to process. Commands for bam files will be executed in parallel (one thread per bam file) so make sure you only specify a sensible number of bam files here!

=item B<-o    --outdir>

Directory for output files. Defaults to current working directory.

=item B<-w    --window>

Window size in bp to use with CNVnator commands.

=item B<-f    --fasta>

Custom location of per chromosome fasta files. Default is the 'fasta' directory in the same directory as this script.

=item B<-r    --root_dir>

Custom location for your Drupal Root installation. Default is the 'root' directory in the same directory as this script.

=item B<-c    --cnvnator>

Custom location of cnvnator binary. Default is in 'CNVnator_v0.3/src/cnvnator' relative to the location of this script. 

=item B<-v    --vcf_converter>

Custom location of cnvnator2VCF.pl script. Default is in 'CNVnator_v0.3/cnvnator2VCF.pl' relative to the location of this script.

=item B<-d    --dry_run>

Print commands to STDERR but do not execute commands.

=item B<-h    --help>

Display help message.

=item B<-m    --manual>

Show manual page.

=back
=cut

=head1 DESCRIPTION

Helper script to execute CNVnator commands on one or more bam files. Each bam file will be processed using a separate thread. Note, this is only written for use with bash shells. Will require some very light hacking to work with other shells (i.e. change "bash" in the line 'my $bsh = Shell::Source->new(shell => "bash", file => "$root_dir/bin/thisroot.sh");' or add another option to the GetOpts function.).

I wrote this helper script but have nothing to do with CNVnator. 

 CNVnator is developed by the Gerstein Lab (http://sv.gersteinlab.org/). 

 Abyzov A, Urban AE, Snyder M, Gerstein M.

 CNVnator: an approach to discover, genotype, and characterize typical and atypical CNVs from family and population genome sequencing.

 Genome Res. 2011 Jun;21(6):974-84. doi: 10.1101/gr.114876.110.

=head1 AUTHOR

cnvnatorWrapperl.pl was written by David A. Parry

University of Leeds

=head1 COPYRIGHT AND LICENSE

Copyright 2015  David A. Parry

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

=cut


