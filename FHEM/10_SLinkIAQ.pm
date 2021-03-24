###############################################################################
# $Id: 10_SLinkIAQC.pm  $
#
# this module is part of fhem under the same license
# copyright 2019, joerg herrmann
# 
# history
# initial checkin
#
###############################################################################
package main;

use strict;
use warnings;
use utf8;
use Socket qw( inet_pton sockaddr_in AF_INET );

sub SLinkIAQ_Initialize {
  my ($hash) = @_;

  $hash->{Match}          = '^T:[^;]*;FW:[^;]*;ID:[^;]*;IP:[^;]*;R:[^;]*;F:IAQ[^;]*.*$';

  $hash->{DefFn}          = "SLinkIAQ_Define";
  $hash->{UndefFn}        = "SLinkIAQ_Undef";
  $hash->{Delete}         = "SLinkIAQ_Undef";
  $hash->{ShutdownFn}     = "SLinkIAQ_Undef";
  $hash->{SetFn}          = "SLinkIAQ_Set";
  $hash->{GetFn}          = "SLinkIAQ_Get";
  $hash->{AttrFn}         = "SLinkIAQ_Attr";
  $hash->{RenameFn}       = "SLinkIAQ_Rename";
  $hash->{NotifyFn}       = "SLinkIAQ_Notify";
  $hash->{ParseFn}        = "SLinkIAQ_Parse";
  
  $hash->{AttrList}   		= $readingFnAttributes;
  return undef;
};

sub SLinkIAQ_Define {
  my ($hash, $def) = @_;
  return "usage: define name SLinkIAQ id function" 
    unless (my ($name, $module, $id, $fn) = $def =~ m/^(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s*/gm);
  return sprintf ('SLinkIAQ %s %s already defined as %s', $id, $fn, $modules{'SLinkIAQ'}{'defptr'}{$id}{$fn}) 
    if ($modules{'SLinkIAQ'}{'defptr'}{$id}{$fn});
	$modules{'SLinkIAQ'}{'defptr'}{$id}{$fn} = $name;
  $hash->{'ID'} = $id;
  $hash->{'FUNCTION'} = $fn;
  notifyRegexpChanged($hash, 'global');
  SLinkIAQ_Run($hash) if ($init_done);
  return undef;
};

sub SLinkIAQ_Undef(@) {
  my ($hash) = @_;
  my $id = $hash->{'ID'};
  delete $modules{'SLinkIAQ'}{'defptr'}{$id};
  return undef;
};

sub SLinkIAQ_Set {
  my ($hash, $name, $cmd, @args) = @_;
  return undef;
};

sub SLinkIAQ_Get {
  my ($hash) = @_;
  return undef;
};

sub SLinkIAQ_Attr {
  my ($hash, $name, $attr, @args) = @_;
  return undef;
};

sub SLinkIAQ_Rename {
  my ($newName, $oldName) = @_;
  my $hash = $defs{$newName};
  my $id = $hash->{'ID'};
  my $fn = $hash->{'FUNCTION'};
  $modules{'SLinkIAQ'}{'defptr'}{$id}{$fn} = $newName;
  return undef;
};

sub SLinkIAQ_Notify {
  my ($hash, $ntfyDev) = @_;
  my $events = deviceEvents($ntfyDev,1);
  return undef if(!$events);
  foreach my $event (@{$events}) {
    next if (!defined($event));
    SLinkIAQ_Run($hash) if ($event eq 'INITIALIZED');
  };
  return undef;
};

sub SLinkIAQ_Run {
  my ($hash) = @_;
  return undef;
};

sub SLinkIAQ_Message {
  my ($hash, $msg) = @_;
  if (my ($type, $fw, $id, $ip, $rssi, $fn) = $msg =~ m/^T:([^;]*);FW:([^;]*);ID:([^;]*);IP:([^;]*);R:([^;]*);F:([^;]*);/gm) {
    $msg =~ s/^T:([^;]*);FW:([^;]*);ID:([^;]*);IP:([^;]*);R:([^;]*);F:([^;]*);//gm;
    if (my ($co2, $tvoc, $res) = $msg =~ m/^C:([^;]*);V:([^;]*);R:([^;]*);$/gm) {
      $hash->{'SENSOR'} = $ip;
      readingsBeginUpdate($hash);
      readingsBulkUpdate($hash, 'state', "c: $co2 t: $tvoc r: $res");
      readingsBulkUpdate($hash, 'co2', $co2);
      readingsBulkUpdate($hash, 'tvoc', $tvoc);
      readingsBulkUpdate($hash, 'res', $res);
      readingsEndUpdate($hash, 1);
    };
  };
};

sub SLinkIAQ_Parse {
  my ($iohash, $msg) = @_;
  if (my ($type, $fw, $id, $ip, $rssi, $fn) = $msg =~ m/^T:([^;]*);FW:([^;]*);ID:([^;]*);IP:([^;]*);R:([^;]*);F:([^;]*).*$/gm) {
    if (my $device = $modules{'SLinkIAQ'}{'defptr'}{$id}{$fn}) {
      if (exists($defs{$device}) and (my $hash = $defs{$device})) {
        SLinkIAQ_Message($hash, $msg);
        return $device;
      };
    } else {
      return sprintf("UNDEFINED SLinkIAQ_%s_%s SLinkIAQ %s %s", $id, $fn, $id, $fn);
    };
  }; 
  return undef;
};

sub SLinkIAQ_Send {
  my ($hash, $msg) = @_;
  eval {
    my $socket;
    #my $SensorIP = $hash->{'SENSOR'};
    my $id = $hash->{'ID'};
    my $SensorIP = $modules{'SLinkIAQ'}{'sensor'}{$id};
    socket($socket, PF_INET, SOCK_DGRAM, 0);
    setsockopt($socket, SOL_SOCKET, SO_REUSEADDR, 1);
    my $portaddr = sockaddr_in(2085, inet_pton (AF_INET, $SensorIP));
    my $result = send ($socket, $msg, 0, $portaddr);
  };
  if ($@) {
    return sprintf ('error while sending %s', $@);
  } else {
    return undef;
  }
};

1;

# Beginn der Commandref

=pod
=item [device]
=item summary SLink Temp / Hum Sensor 
=item summary_DE SLink Temp / Hum Sensor

=begin html
 Englische Commandref in HTML
=end html

=begin html_DE
 Deutsche Commandref in HTML
=end html

# Ende der Commandref
=cut