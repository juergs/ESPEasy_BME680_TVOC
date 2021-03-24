###############################################################################
# $Id: 00_SLinkS0.pm  $
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
use POSIX qw( log10 fmod );
use Socket qw( inet_pton sockaddr_in AF_INET );

sub SLinkS0_Initialize {
  my ($hash) = @_;

  $hash->{Match}          = '^T:[^;]*;FW:[^;]*;ID:[^;]*;IP:[^;]*;R:[^;]*;F:S0[^;]*.*$';

  $hash->{DefFn}          = "SLinkS0_Define";
  $hash->{UndefFn}        = "SLinkS0_Undef";
  $hash->{Delete}         = "SLinkS0_Undef";
  $hash->{ShutdownFn}     = "SLinkS0_Undef";
  $hash->{SetFn}          = "SLinkS0_Set";
  $hash->{GetFn}          = "SLinkS0_Get";
  $hash->{AttrFn}         = "SLinkS0_Attr";
  $hash->{RenameFn}       = "SLinkS0_Rename";
  $hash->{NotifyFn}       = "SLinkS0_Notify";
  $hash->{ParseFn}        = "SLinkS0_Parse";
  
  $hash->{AttrList}   		= "factor ".$readingFnAttributes;
  return undef;
};

sub SLinkS0_Define {
  my ($hash, $def) = @_;
  return "usage: define name SLinkS0 id function" 
    unless (my ($name, $module, $id, $fn) = $def =~ m/^(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s*/gm);
  return sprintf ('SLinkS0 %s %s already defined as %s', $id, $fn, $modules{'SLinkS0'}{'defptr'}{$id}{$fn}) 
    if ($modules{'SLinkS0'}{'defptr'}{$id}{$fn});
	$modules{'SLinkS0'}{'defptr'}{$id}{$fn} = $name;
  $hash->{'ID'} = $id;
  $hash->{'FUNCTION'} = $fn;
  $hash->{'DROP'} = 0;
  $hash->{'STATE'} = "pending";
  $attr{$name}{'stateFormat'} = 'meter';
  notifyRegexpChanged($hash, 'global');
  SLinkS0_Run($hash) if ($init_done);
  return undef;
};

sub SLinkS0_Undef(@) {
  my ($hash) = @_;
  my $id = $hash->{'ID'};
  delete $modules{'SLinkS0'}{'defptr'}{$id};
  return undef;
};

sub SLinkS0_Set {
  my ($hash, $name, $cmd, @args) = @_;
  return "no set argument specified" if(!$cmd);
  if ($cmd eq 'meter') {
    return "usage set meter <value> <factor>" if (
      (@args != 2) or 
      ($args[0] !~ m/^\d+(?:\.\d+)?$/) or
      ($args[1] !~ m/^\d+$/) or
      ($args[1] == 0)
    );
    $args[0] *= $args[1];
    my $fn = $hash->{'FUNCTION'};
    my $msg = sprintf('%s_Meter:%s,%s;', $fn, $args[0], $args[1]);
    return (SLinkS0_Send($hash, $msg), 1);
  };
  return "Unknown argument $cmd, choose one of meter";
};

sub SLinkS0_Get {
  my ($hash) = @_;
  return undef;
};

sub SLinkS0_Attr {
  my ($hash, $name, $attr, @args) = @_;
  # return "no attr name specified" if(!$attr);
  return undef;
};

sub SLinkS0_Rename {
  my ($newName, $oldName) = @_;
  my $hash = $defs{$newName};
  my $id = $hash->{'ID'};
  my $fn = $hash->{'FUNCTION'};
  $modules{'SLinkS0'}{'defptr'}{$id}{$fn} = $newName;
  return undef;
};

sub SLinkS0_Notify {
  my ($hash, $ntfyDev) = @_;
  my $events = deviceEvents($ntfyDev,1);
  return undef if(!$events);
  foreach my $event (@{$events}) {
    next if (!defined($event));
    SLinkS0_Run($hash) if ($event eq 'INITIALIZED');
  };
  return undef;
};

sub SLinkS0_Run {
  my ($hash) = @_;
  return undef;
};

sub SLinkS0_Message {
  my ($hash, $msg) = @_;
  if (my ($type, $fw, $id, $ip, $rssi, $fn) = $msg =~ m/^T:([^;]*);FW:([^;]*);ID:([^;]*);IP:([^;]*);R:([^;]*);F:([^;]*);/gm) {
    $msg =~ s/^T:([^;]*);FW:([^;]*);ID:([^;]*);IP:([^;]*);R:([^;]*);F:([^;]*);//gm;
    if (my ($idx, $c, $isSynched) = $msg =~ m/^IX:([^;]*);C:([^;]*);S:([^;]*);$/gm) {
      $hash->{'SENSOR'} = $ip;
      $hash->{'DROP'}++ if ($hash->{'LAST_IDX'} and (($hash->{'LAST_IDX'} +1) != $idx));
      $hash->{'LAST_IDX'} = $idx;
      my ($m, $factor) = split /,/, $c;
      return undef if (!$isSynched);
      $factor = 1 if (!$factor); # should not happen but would cause a div by zero
      my $dec = log10($factor);
      my $fmt = (fmod($dec, 1))?'%s':'%.'.$dec.'f';
      my $meter = sprintf($fmt, $m / $factor);
      readingsBeginUpdate($hash);
      #readingsBulkUpdate($hash, 'state', $meter);
      readingsBulkUpdate($hash, 'meter', $meter);
      readingsEndUpdate($hash, 1);
    };
  };
};

sub SLinkS0_Parse {
  my ($iohash, $msg) = @_;
  if (my ($type, $fw, $id, $ip, $rssi, $fn) = $msg =~ m/^T:([^;]*);FW:([^;]*);ID:([^;]*);IP:([^;]*);R:([^;]*);F:([^;]*).*$/gm) {
    if (my $device = $modules{'SLinkS0'}{'defptr'}{$id}{$fn}) {
      if (exists($defs{$device}) and (my $hash = $defs{$device})) {
        SLinkS0_Message($hash, $msg);
        return $device;
      };
    } else {
      return sprintf("UNDEFINED SLinkS0_%s_%s SLinkS0 %s %s", $id, $fn, $id, $fn);
    };
  }; 
  return undef;
};

sub SLinkS0_Send {
  my ($hash, $msg) = @_;
  eval {
    my $socket;
    #my $SensorIP = $hash->{'SENSOR'};
    my $id = $hash->{'ID'};
    my $SensorIP = $modules{'SLinkS0'}{'sensor'}{$id};
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
=item summary SLink SO Counter 
=item summary_DE SLink SO Counter 

=begin html
 Englische Commandref in HTML
=end html

=begin html_DE
 Deutsche Commandref in HTML
=end html

# Ende der Commandref
=cut