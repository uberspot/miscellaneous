#!/usr/bin/perl
#Original piratebay scrapper found on github
#I added some more info to gather for each torrent and made it start from the last torrent recorded if any.

use warnings;
use strict;
use Parallel::ForkManager;
use 5.010;
my $pm=new Parallel::ForkManager(7);

use Fcntl qw(:flock SEEK_END);

$pm->run_on_finish(sub{
        my (undef, undef, undef, undef, undef, $res_ref) = @_;
        my ($res, $line) = @$res_ref;
        if ($res == 1) {
                        open my $outf, ">>", "outfile";
                        flock($outf, LOCK_EX) or next;
                        seek($outf, 0, SEEK_END);
                        say $outf $line;
                        say $line;
                        flock($outf, LOCK_UN);
       }
});

my $i = 0;
my $filename = "outfile";
if(-e $filename) {
    `tail -n 1 $filename` =~ m/(\d+)/;
    $i = $1;
}
say $i;

while (1) {
        $i++;
        $pm->start and next;
        my $res;
        say "started ".$i;
        my $page="";
        $page = `curl -s http://thepiratebay.se/torrent/$i -m 120` 
        while ($page !~ /<!DOCTYPE html/);
        my $line = "";
        if ($page =~ m{<title>Not Found}) {
                 $res = 0;
        } else {
                 $res = 1;
                 my ($title) = $page =~ /<div id="title">\s*(.*?)\s*<\/div>/s;
                 my ($size) = $page =~ /<dt>Size:<\/dt>\s*<dd>.*?\((\d*)&nbsp;Bytes\)<\/dd>/s;
                 my ($seeders) = $page =~ /<dt>Seeders:<\/dt>\s*<dd>(\d*)<\/dd>/;
                 my ($leechers) = $page =~ /<dt>Leechers:<\/dt>\s*<dd>(\d*)<\/dd>/;
                 my ($magnet) = $page =~ /magnet:\?xt=urn:btih:(.*?)(&|")/;
                 my ($date) = $page =~ /<dt>Uploaded:<\/dt>\s*<dd>\s*(.*?)\s*<\/dd>/;
                 my ($uploader) = $page =~ /<dt>By:<\/dt>\s*<dd>\s*<a href="\/user\/(.*?)\/"/;
                 my ($category) = $page =~ /title="More from this category">\s*(.*?)\s*<\/a><\/dd>/;
                 $category =~ s/&gt;/>/g;
                 $uploader = "Anonymous" if (not defined $uploader);
                 $line = $i."|".$date."|".$category."|".$uploader."|".$title."|".$size."|".$seeders."|".$leechers."|magnet:?xt=urn:btih:".$magnet;
        }
        $pm->finish(0,[$res, $line]);
}
