###############################################################################
# $Id: 00_SLinkTH.pm  $
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

sub SLinkTH_Initialize {
  my ($hash) = @_;

  $hash->{Match}          = '^T:[^;]*;FW:[^;]*;ID:[^;]*;IP:[^;]*;R:[^;]*;F:TH[^;]*.*$';

  $hash->{DefFn}          = "SLinkTH_Define";
  $hash->{UndefFn}        = "SLinkTH_Undef";
  $hash->{Delete}         = "SLinkTH_Undef";
  $hash->{ShutdownFn}     = "SLinkTH_Undef";
  $hash->{SetFn}          = "SLinkTH_Set";
  $hash->{GetFn}          = "SLinkTH_Get";
  $hash->{AttrFn}         = "SLinkTH_Attr";
  $hash->{RenameFn}       = "SLinkTH_Rename";
  $hash->{NotifyFn}       = "SLinkTH_Notify";
  $hash->{ParseFn}        = "SLinkTH_Parse";
  
  $hash->{AttrList}   		= $readingFnAttributes;
  return undef;
};

sub SLinkTH_Define {
  my ($hash, $def) = @_;
  return "usage: define name SLinkTH id function" 
    unless (my ($name, $module, $id, $fn) = $def =~ m/^(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s*/gm);
  return sprintf ('SLinkTH %s %s already defined as %s', $id, $fn, $modules{'SLinkTH'}{'defptr'}{$id}{$fn}) 
    if ($modules{'SLinkTH'}{'defptr'}{$id}{$fn});
	$modules{'SLinkTH'}{'defptr'}{$id}{$fn} = $name;
  $hash->{'ID'} = $id;
  $hash->{'FUNCTION'} = $fn;
  notifyRegexpChanged($hash, 'global');
  SLinkTH_Run($hash) if ($init_done);
  return undef;
};

sub SLinkTH_Undef(@) {
  my ($hash) = @_;
  my $id = $hash->{'ID'};
  delete $modules{'SLinkTH'}{'defptr'}{$id};
  return undef;
};

sub SLinkTH_Set {
  my ($hash, $name, $cmd, @args) = @_;
  return undef;
};

sub SLinkTH_Get {
  my ($hash) = @_;
  return undef;
};

sub SLinkTH_Attr {
  my ($hash, $name, $attr, @args) = @_;
  return undef;
};

sub SLinkTH_Rename {
  my ($newName, $oldName) = @_;
  my $hash = $defs{$newName};
  my $id = $hash->{'ID'};
  my $fn = $hash->{'FUNCTION'};
  $modules{'SLinkTH'}{'defptr'}{$id}{$fn} = $newName;
  return undef;
};

sub SLinkTH_Notify {
  my ($hash, $ntfyDev) = @_;
  my $events = deviceEvents($ntfyDev,1);
  return undef if(!$events);
  foreach my $event (@{$events}) {
    next if (!defined($event));
    SLinkTH_Run($hash) if ($event eq 'INITIALIZED');
  };
  return undef;
};

sub SLinkTH_Run {
  my ($hash) = @_;
  return undef;
};

sub SLinkTH_Message {
  my ($hash, $msg) = @_;
  if (my ($type, $fw, $id, $ip, $rssi, $fn) = $msg =~ m/^T:([^;]*);FW:([^;]*);ID:([^;]*);IP:([^;]*);R:([^;]*);F:([^;]*);/gm) {
    $msg =~ s/^T:([^;]*);FW:([^;]*);ID:([^;]*);IP:([^;]*);R:([^;]*);F:([^;]*);//gm;
    if (my ($t, $h) = $msg =~ m/^T:([^;]*);H:([^;]*);$/gm) {
      $hash->{'SENSOR'} = $ip;
      readingsBeginUpdate($hash);
      readingsBulkUpdate($hash, 'state', "T: $t H: $h");
      readingsBulkUpdate($hash, 'temperature', $t);
      readingsBulkUpdate($hash, 'humidity', $h);
      readingsEndUpdate($hash, 1);
    };
  };
};

sub SLinkTH_Parse {
  my ($iohash, $msg) = @_;
  if (my ($type, $fw, $id, $ip, $rssi, $fn) = $msg =~ m/^T:([^;]*);FW:([^;]*);ID:([^;]*);IP:([^;]*);R:([^;]*);F:([^;]*).*$/gm) {
    if (my $device = $modules{'SLinkTH'}{'defptr'}{$id}{$fn}) {
      if (exists($defs{$device}) and (my $hash = $defs{$device})) {
        SLinkTH_Message($hash, $msg);
        return $device;
      };
    } else {
      return sprintf("UNDEFINED SLinkTH_%s_%s SLinkTH %s %s", $id, $fn, $id, $fn);
    };
  }; 
  return undef;
};

sub SLinkTH_Send {
  my ($hash, $msg) = @_;
  eval {
    my $socket;
    #my $SensorIP = $hash->{'SENSOR'};
    my $id = $hash->{'ID'};
    my $SensorIP = $modules{'SLinkTH'}{'sensor'}{$id};
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