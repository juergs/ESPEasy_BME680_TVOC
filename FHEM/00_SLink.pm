###############################################################################
# $Id: 00_SLink.pm  $
#
# this module is part of fhem under the same license
# copyright 2018, joerg herrmann
# 
# history
# initial checkin
#
###############################################################################
package main;

use strict;
use warnings;
use utf8;
use Socket qw( inet_pton inet_ntop sockaddr_in AF_INET INADDR_ANY IP_ADD_MEMBERSHIP IP_DROP_MEMBERSHIP );
use Time::HiRes qw(time);

our $deb = 1;

sub SLink_Initialize {
  my ($hash) = @_;

  $hash->{Clients}        = "SLink:SLinkIAQC:SLinkS0:";

  $hash->{MatchList}      = { "0:SLink"       => '^T:[^;]*;FW:[^;]*;ID:[^;]*;IP:[^;]*;R:[^;]*;F:SERVICE[^;]*.*$', 
                              "10:SLinkIAQC"  => '^T:IAQC;FW:[^;]*;ID:[^;]*;.*$',
                              "20:SLinkS0"    => '^T:[^;]*;FW:[^;]*;ID:[^;]*;IP:[^;]*;R:[^;]*;F:S0[^;]*.*$',
                              #"21:SLinkTH"    => '^T:[^;]*;FW:[^;]*;ID:[^;]*;IP:[^;]*;R:[^;]*;F:TH[^;]*.*$',
                              #"21:SLinkIAQ"   => '^T:[^;]*;FW:[^;]*;ID:[^;]*;IP:[^;]*;R:[^;]*;F:IAQ[^;]*.*$',
                            };

  # we receive our own service msg
  $hash->{Match}          = '^T:[^;]*;FW:[^;]*;ID:[^;]*;IP:[^;]*;R:[^;]*;F:SERVICE[^;]*.*$';

  $hash->{DefFn}          = "SLink_Define";
  $hash->{UndefFn}        = "SLink_Shutdown";
  $hash->{SetFn}          = "SLink_Set";
  $hash->{GetFn}          = "SLink_Get";
  $hash->{AttrFn}         = "SLink_Attr";
  $hash->{NotifyFn}       = "SLink_Notify";
  $hash->{DeleteFn}       = "SLink_Delete";
  $hash->{ShutdownFn}     = "SLink_Shutdown";
  $hash->{ParseFn}        = "SLink_Parse";
  $hash->{AttrList}   		= "".$readingFnAttributes;
  return undef;
};

sub SLink_Define {
  my ($hash, $def) = @_;
  return "only one SLink instance is allowed" if (SLink_GetService($hash));
  my ($name, $module, $ip) = split / /, $def;
  eval {
    if ($ip) {
      $hash->{'helper'}->{'mcaddr'} = inet_pton(AF_INET, $ip);
    } else {
      $hash->{'helper'}->{'mcaddr'} = INADDR_ANY;
    };
    1;
  } or do {
    return "invalid ip: $ip";
  };
  #$hash->{'.clientArray'} = [qw( SLink SLinkIAQC SLinkS0 SLinkTH )];
  notifyRegexpChanged($hash, 'global');
	SLink_Run($hash) if ($init_done);
  return undef;
};

sub SLink_Set {
  my ($hash, $name, $cmd, @args) = @_;
  return undef;
};

sub SLink_Get {
  my ($hash) = @_;
  return undef;
};

sub SLink_Attr {
  my ($hash) = @_;
  return undef;
};

sub SLink_Notify {
  my ($hash, $ntfyDev) = @_;
  my $events = deviceEvents($ntfyDev,1);
  return undef if(!$events);
  foreach my $event (@{$events}) {
    next if (!defined($event));
    SLink_Run($hash) if ($event eq 'INITIALIZED');
  };
  return undef;
};

sub SLink_Run {
  my ($hash) = @_;
  SLink_CreateService($hash);
  return undef;
};

sub SLink_Delete(@) {
  my ($hash) = @_;
  return undef;
};

sub SLink_Shutdown {
  my ($hash) = @_;
  my ($shash, $socket);
  return unless ($shash = SLink_GetService($hash));
  delete $selectlist{$shash->{'NAME'}};
  return unless ($socket = $shash->{'FH'});
  my $ip_level = getprotobyname('IP');
  my $mcaddr = $hash->{'helper'}->{'mcaddr'};
  my $ip_mreq = inet_pton(AF_INET, '239.255.255.250') . $mcaddr;
  setsockopt ($socket, $ip_level, IP_DROP_MEMBERSHIP, $ip_mreq);
  close($socket) if $socket;
  return undef;
};


sub SLink_GetService { 
  my ($hash, $create) = @_;
  if (exists($main::selectlist{'global-SLink-Service'})) {
    return $main::selectlist{'global-Slink-Service'};
  } elsif ($create) {
    return SLink_CreateService($hash);
  } else {
    return undef;
  };
};

sub SLink_CreateService {
	my ($hash) = @_;
  my $socket;
  my $mcaddr = $hash->{'helper'}->{'mcaddr'};
  socket($socket, PF_INET, SOCK_DGRAM, 0);
  setsockopt($socket, SOL_SOCKET, SO_REUSEADDR, 1);
  bind($socket, sockaddr_in(2085, $mcaddr));
  #bind($socket, sockaddr_in(2085, INADDR_ANY));
  #bind($socket, sockaddr_in(2085, inet_pton(AF_INET, '192.168.178.56') ));
  my $ip_mreq = inet_pton(AF_INET, '239.255.255.250') . $mcaddr;
  #my $ip_mreq = inet_pton(AF_INET, '239.255.255.250') . INADDR_ANY;
  #my $ip_mreq = inet_pton(AF_INET, '239.255.255.250') . inet_pton(AF_INET, '192.168.178.56');
  my $ip_level = getprotobyname('IP');
  setsockopt ($socket, $ip_level, IP_ADD_MEMBERSHIP, $ip_mreq);
  my $shash = {};
  $shash->{'NAME'} = "global-SLink-Service";
  $shash->{'PARENT'} = $hash->{'NAME'};
  $shash->{'FH'} = $socket;
  $shash->{'FD'} = $socket->fileno();
  $shash->{'directReadFn'} = \&SLink_Receive;
  my $portaddr = sockaddr_in(2085, inet_pton (AF_INET, '239.255.255.250'));
  my $uniqueID = getUniqueId();
  my $query = "T:SLink;FW:1.0;ID:$uniqueID;IP:0;R:0;F:SERVICE;START:Startup;\n";
  my $result = send($socket, $query, 0, $portaddr);
  return $selectlist{$shash->{'NAME'}} = $shash;
};

sub SLink_Parse {
  my ($iohash, $msg) = @_;
  my $name = $iohash->{'NAME'};
  if (my ($type, $fw, $id, $ip, $rssi, $reason) = $msg =~ m/^T:([^;]*);FW:([^;]*);ID:([^;]*);IP:([^;]*);R:([^;]*);F:SERVICE;START:(.*);/gm) {
    Log3 (undef, 3, sprintf('SLink: sensor %s (%s) start, reason %s', $id, $ip, $reason));
  }; 
  return $name;
};

sub SLink_Receive {
  my ($shash) = @_;
  my $socket = $shash->{'FH'};
  my $msg;
  my $hispaddr = recv ($shash->{'FH'}, $msg, POSIX::BUFSIZ, 0); # POSIX::BUFSIZ, 0;

  my ($port, $iaddr) = sockaddr_in($hispaddr);
  my $herstraddr     = inet_ntop(AF_INET, $iaddr);

  Log3 ($shash->{PARENT}, 5 , sprintf("SLink Service received from %s:%s packet %s", $herstraddr, $port, $msg));
  if (my ($type, $fw, $id, $ip, $rssi, $fn) = $msg =~ m/^T:([^;]*);FW:([^;]*);ID:([^;]*);IP:([^;]*);R:([^;]*);F:([^;]*).*\n$/gm) {  
    $msg =~ s/[\r\n]+$//gm;
    #$modules{'SLinkS0'}{'sensor'}{$id} = $ip;
    my $parent = $shash->{PARENT};
    my $hash = $defs{$parent};
    my $addvals = {
      'RAWMSG'    =>  $msg,
      'RSSI'      =>  $rssi,
      'REMOTE'    =>  $herstraddr,
    };
    # we need to find a better solution for sensors under construction. 
    my $noUnknown = 0; 
    Dispatch($hash, $msg, $addvals, $noUnknown);
  } else {
    $msg =~ s/[^\w;:,.-]/\?/;
    #Log3 ($shash->{PARENT}, 1 , sprintf("SLink Service received %s %s invalid packet %s", $port, $herstraddr, $msg));
    return undef;
  };
};

sub SLinkS0_SendMCAST {
  my ($hash, $msg) = @_;
  eval {
    my $socket;
    #my $SensorIP = $hash->{'SENSOR'};
    my $id = $hash->{'ID'};
    my $SensorIP = $modules{'SLinkS0'}{'sensor'}{$id};
    socket($socket, PF_INET, SOCK_DGRAM, 0);
    setsockopt($socket, SOL_SOCKET, SO_REUSEADDR, 1);
    my $portaddr = sockaddr_in(2085, inet_pton (AF_INET, $SensorIP));
    # inet_pton(AF_INET, '239.255.255.250')
    my $result = send ($socket, $msg, 0, $portaddr);
  };
  if ($@) {
    return sprintf ('error while sending %s', $@);
  } else {
    return undef;
  }
};

# take care and leave multicast group in case of unexpexted close. wont work on signals
END {
  SLink_Shutdown({});
};

1;