__cnvnatorWrapperl.pl__

cnvnatorWrapper.pl is a perl script designed to run cnvnator automatically and in parallel on one or more bam files. It has no affiliation with CNVnator or ROOT which come bundled with this program.

Note: you must be using bash as your shell when running cnvnatorWrapper.pl to allow proper sourcing of environment variables. It should be easily hackable to allow for other shells if needed.

__INSTALL__

This version should run as is on 64-bit Debian 7 after decompressing the chromosome fasta files in the 'fasta' folder (e.g. "gzip -d fasta/\*.gz"). For other systems you will have to recompile CNVnator and ROOT (or download the appropriate binary for your system). 

To set this up on a different system, recompile CNVnator as per the instructions in CNVnator\_v0.3/README. You then need to delete the 'root' directory and its contents, download or compile version 5.34 of ROOT (https://root.cern.ch/drupal/content/production-version-534) ensuring the resulting files are in a directory named 'root'.

Gzipped per chromosome fasta files are provided in the fasta directory for the human b37 genome (decompress before using). If you are using a different genome you will need to either point the script to a different directory containing the relevant per chromosome fasta files using the --fasta option or replace those in the enclosed fasta directory.

You will also need to install the Shell::Source and Pod::Usage perl modules via CPAN or your distributions package manager. 

Detailed usage information for the cnvnatorWrapperl.pl script is available by invoking the program with --help or --manual flags (assuming you have Pod::Usage perl module installed).

__TO DO__

-Maybe support other shells. 

-Make thread allocation more sophisticated (e.g. allow user to specify max threads).

Really this depends on whether anyone other than me ends up using this and wants any of these features. These should both be easy to acheive.

__CREDIT__

Please see the documentation for CNVnator and ROOT for copyright and authorship information. 

cnvnatorWrapperl.pl is Copyright 2015  David A. Parry

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.


