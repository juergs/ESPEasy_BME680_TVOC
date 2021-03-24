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

sub SLinkIAQC_Initialize {
  my ($hash) = @_;

  $hash->{Match}          = '^T:IAQC;FW:[^;]*;ID:[^;]*;IP:[^;]*;R:[^;]*;.*$';

  $hash->{DefFn}          = "SLinkIAQC_Define";
  $hash->{UndefFn}        = "SLinkIAQC_Undef";
  $hash->{Delete}         = "SLinkIAQC_Undef";
  $hash->{ShutdownFn}     = "SLinkIAQC_Undef";
  $hash->{SetFn}          = "SLinkIAQC_Set";
  $hash->{GetFn}          = "SLinkIAQC_Get";
  $hash->{AttrFn}         = "SLinkIAQC_Attr";
  $hash->{RenameFn}       = "SLinkIAQC_Rename";
  $hash->{NotifyFn}       = "SLinkIAQC_Notify";
  $hash->{ParseFn}        = "SLinkIAQC_Parse";
  
  $hash->{AttrList}   		= $readingFnAttributes;
  return undef;
};

sub SLinkIAQC_Define {
  my ($hash, $def) = @_;
  return "usage: define name SLinkIAQC id" 
    unless (my ($name, $module, $id) = $def =~ m/^(\S+)\s+(\S+)\s+(\S+)\s*/gm);
  return sprintf ('SLinkIAQC %s already defined as %s', $id, $modules{'SLinkIAQC'}{'defptr'}{$id}) 
    if ($modules{'SLinkIAQC'}{'defptr'}{$id});
	$modules{'SLinkIAQC'}{'defptr'}{$id} = $name;
  $hash->{'ID'} = $id;
  notifyRegexpChanged($hash, 'global');
  SLinkIAQC_Run($hash) if ($init_done);
  return undef;
};

sub SLinkIAQC_Undef(@) {
  my ($hash) = @_;
  my $id = $hash->{'ID'};
  delete $modules{'SLinkIAQC'}{'defptr'}{$id};
  return undef;
};

sub SLinkIAQC_Set {
  my ($hash, $name, $cmd, @args) = @_;
  return undef;
};

sub SLinkIAQC_Get {
  my ($hash) = @_;
  return undef;
};

sub SLinkIAQC_Attr {
  my ($hash, $name, $attr, @args) = @_;
  return undef;
};

sub SLinkIAQC_Rename {
  my ($newName, $oldName) = @_;
  my $hash = $defs{$newName};
  my $id = $hash->{'ID'};
  $modules{'SLinkIAQC'}{'defptr'}{$id} = $newName;
  return undef;
};

sub SLinkIAQC_Notify {
  my ($hash, $ntfyDev) = @_;
  my $events = deviceEvents($ntfyDev,1);
  return undef if(!$events);
  foreach my $event (@{$events}) {
    next if (!defined($event));
    SLinkIAQC_Run($hash) if ($event eq 'INITIALIZED');
  };
  return undef;
};

sub SLinkIAQC_Run {
  my ($hash) = @_;
  return undef;
};

sub SLinkIAQC_Message {
  my ($hash, $msg) = @_;
  if (my ($fw, $id, $ip, $rssi, $fn) = $msg =~ m/^T:IAQC;FW:([^;]*);ID:([^;]*);IP:([^;]*);R:([^;]*);F:([^;]*);/gm) {
    $hash->{'SENSOR'} = $ip;
    $msg =~ s/^T:IAQC;FW:([^;]*);ID:([^;]*);IP:([^;]*);R:([^;]*);//gm;
    if (my ($brightness) = $msg =~ m/^F:BR;B:([^;]*);$/gm) {
      readingsBeginUpdate($hash);
      readingsBulkUpdate($hash, 'brightness', $brightness);
      readingsEndUpdate($hash, 1);
    };
    if (my ($eco2, $tvoc, $res) = $msg =~ m/^F:IAQ;C:([^;]*);V:([^;]*);R:([^;]*);$/gm) {
      readingsBeginUpdate($hash);
      #readingsBulkUpdate($hash, 'state', "c: $co2 t: $tvoc r: $res");
      readingsBulkUpdate($hash, 'IAQ_tvoc', $tvoc);
      #readingsBulkUpdate($hash, 'res', $res);
      readingsEndUpdate($hash, 1);
    };
    if (my ($co2) = $msg =~ m/^F:CO2;C:([^;]*);$/gm) {
      readingsBeginUpdate($hash);
      #readingsBulkUpdate($hash, 'state', "c: $co2 t: $tvoc r: $res");
      readingsBulkUpdate($hash, 'MHZ_co2', $co2);
      #readingsBulkUpdate($hash, 'res', $res);
      readingsEndUpdate($hash, 1);
    };
    if (my ($t, $rh, $ah, $dp, $p) = $msg =~ m/^F:THP;T:([^;]*);H:([^;]*);AH:([^;]*);D:([^;]*);P:([^;]*);$/gm) { #BME280
      readingsBeginUpdate($hash);
      #readingsBulkUpdate($hash, 'state', "c: $co2 t: $tvoc r: $res");
      readingsBulkUpdate($hash, 'BME280_temperature', $t);
      readingsBulkUpdate($hash, 'BME280_rH', $rh);
      readingsBulkUpdate($hash, 'BME280_aH', $ah);
      readingsBulkUpdate($hash, 'BME280_dewpoint', $dp);
      readingsBulkUpdate($hash, 'BME280_pressure', $p);
      readingsEndUpdate($hash, 1);
    };
    if (my ($t, $rh, $ah, $dp, $p, $tvoc, $r, $dbg_baseC, $dbg_filtered, $dbg_ratio) = $msg =~ m/^F:THPV;T:([^;]*);H:([^;]*);AH:([^;]*);D:([^;]*);P:([^;]*);V:([^;]*);R:([^;]*);DB:([^;]*);DF:([^;]*);DR:([^;]*);$/gm) { #BME680
      readingsBeginUpdate($hash);
      #readingsBulkUpdate($hash, 'state', "c: $co2 t: $tvoc r: $res");
      readingsBulkUpdate($hash, 'BME680_temperature', $t);
      readingsBulkUpdate($hash, 'BME680_rH', $rh);
      readingsBulkUpdate($hash, 'BME680_aH', $ah);
      readingsBulkUpdate($hash, 'BME680_dewpoint', $dp);
      readingsBulkUpdate($hash, 'BME680_pressure', $p);
      readingsBulkUpdate($hash, 'BME680_tvoc', $tvoc);
      readingsBulkUpdate($hash, 'BME680_DBG_R', $r);
      readingsBulkUpdate($hash, 'BME680_DBG_BASE_C', $dbg_baseC);
      readingsBulkUpdate($hash, 'BME680_DBG_FILTERED', $dbg_filtered);
      readingsBulkUpdate($hash, 'BME680_DBG_RATIO', $dbg_ratio);
      readingsEndUpdate($hash, 1);
    };
  };
};

sub SLinkIAQC_Parse {
  my ($iohash, $msg) = @_;
  if (my ($fw, $id, $ip, $rssi, $fn) = $msg =~ m/^T:IAQC;FW:([^;]*);ID:([^;]*);IP:([^;]*);R:([^;]*);F:([^;]*).*$/gm) {
    if (my $device = $modules{'SLinkIAQC'}{'defptr'}{$id}) {
      if (exists($defs{$device}) and (my $hash = $defs{$device})) {
        SLinkIAQC_Message($hash, $msg);
        return $device;
      };
    } else {
      return sprintf("UNDEFINED SLinkIAQC_%s SLinkIAQC %s", $id, $id);
    };
  }; 
  return undef;
};

sub SLinkIAQC_Send {
  my ($hash, $msg) = @_;
  eval {
    my $socket;
    #my $SensorIP = $hash->{'SENSOR'};
    my $id = $hash->{'ID'};
    my $SensorIP = $modules{'SLinkIAQC'}{'sensor'}{$id};
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