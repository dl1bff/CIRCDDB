/*

CIRCDDB - ircDDB client library in C++

Copyright (C) 2010   Michael Dirska, DL1BFF (dl1bff@mdx.de)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


#include "IRCDDB.h"

#include "IRCClient.h"
#include "IRCDDBApp.h"

#include <wx/wx.h>


struct CIRCDDBPrivate
{
    IRCClient * client;
    IRCDDBApp * app;
};


CIRCDDB::CIRCDDB(const wxString& hostName, unsigned int port,
    const wxString& callsign, const wxString& password,
    const wxString& versionInfo ) : d( new CIRCDDBPrivate )

{
  wxString update_channel = wxT("#dstar");

  d->app = new IRCDDBApp(update_channel);

  d->client = new IRCClient( d->app, update_channel, hostName, port, callsign,
    password, versionInfo );
}

CIRCDDB::~CIRCDDB()
{
  delete d->client;
  delete d->app;
  delete d;
}


	// A false return implies a network error, or unable to log in
bool CIRCDDB::open()
{
  wxLogVerbose(wxT("start"));
  return d->client -> startWork()   &&   d->app->startWork();
}


int CIRCDDB::getConnectionState()
{
  return d->app->getConnectionState();
}



// Send heard data, a false return implies a network error
bool CIRCDDB::sendHeard( const wxString& myCall, const wxString& myCallExt,
          const wxString& yourCall, const wxString& rpt1,
	  const wxString& rpt2, unsigned char flag1,
	  unsigned char flag2, unsigned char flag3 )
{
  if (myCall.Len() != 8)
  {
    wxLogVerbose(wxT("CIRCDDB::sendHeard:myCall: len != 8"));
    return false;
  }

  if (myCallExt.Len() != 4)
  {
    wxLogVerbose(wxT("CIRCDDB::sendHeard:myCallExt: len != 4"));
    return false;
  }

  if (yourCall.Len() != 8)
  {
    wxLogVerbose(wxT("CIRCDDB::sendHeard:yourCall: len != 8"));
    return false;
  }

  if (rpt1.Len() != 8)
  {
    wxLogVerbose(wxT("CIRCDDB::sendHeard:rpt1: len != 8"));
    return false;
  }

  if (rpt2.Len() != 8)
  {
    wxLogVerbose(wxT("CIRCDDB::sendHeard:rpt2: len != 8"));
    return false;
  }

  return d->app->sendHeard( myCall, myCallExt, yourCall, rpt1, rpt2, flag1, flag2, flag3,
    wxT("        "), wxT(""), wxT(""));
}


// Send heard data, a false return implies a network error
bool CIRCDDB::sendHeardWithTXMsg( const wxString& myCall, const wxString& myCallExt,
          const wxString& yourCall, const wxString& rpt1,
	  const wxString& rpt2, unsigned char flag1,
	  unsigned char flag2, unsigned char flag3,
	  const wxString& network_destination,
	  const wxString& tx_message )
{
  if (myCall.Len() != 8)
  {
    wxLogVerbose(wxT("CIRCDDB::sendHeard:myCall: len != 8"));
    return false;
  }

  if (myCallExt.Len() != 4)
  {
    wxLogVerbose(wxT("CIRCDDB::sendHeard:myCallExt: len != 4"));
    return false;
  }

  if (yourCall.Len() != 8)
  {
    wxLogVerbose(wxT("CIRCDDB::sendHeard:yourCall: len != 8"));
    return false;
  }

  if (rpt1.Len() != 8)
  {
    wxLogVerbose(wxT("CIRCDDB::sendHeard:rpt1: len != 8"));
    return false;
  }

  if (rpt2.Len() != 8)
  {
    wxLogVerbose(wxT("CIRCDDB::sendHeard:rpt2: len != 8"));
    return false;
  }

  wxString dest = network_destination;

  if (dest.Len() == 0)
  {
    dest = wxT("        ");
  }

  if (dest.Len() != 8)
  {
    wxLogVerbose(wxT("CIRCDDB::sendHeard:network_destination: len != 8"));
    return false;
  }

  wxString msg;

  if (tx_message.Len() == 20)
  {
    unsigned int i;
    for (i=0; i < tx_message.Len(); i++)
    {
      wxChar ch = tx_message.GetChar(i);

      if ((ch > 32) && (ch < 127))
      {
	msg.Append(ch);
      }
      else
      {
	msg.Append(wxT('_'));
      }
    }
  }

  return d->app->sendHeard( myCall, myCallExt, yourCall, rpt1, rpt2, flag1, flag2, flag3,
    dest, msg, wxT(""));
}



bool CIRCDDB::sendHeardWithTXStats( const wxString& myCall, const wxString& myCallExt,
          const wxString& yourCall, const wxString& rpt1,
	  const wxString& rpt2, unsigned char flag1,
	  unsigned char flag2, unsigned char flag3,
	  int num_dv_frames,
          int num_dv_silent_frames,
          int num_bit_errors )
{
  if ((num_dv_frames <= 0) || (num_dv_frames > 65535))
  {
    wxLogVerbose(wxT("CIRCDDB::sendHeard:num_dv_frames not in range 1-65535"));
    return false;
  }

  if (num_dv_silent_frames > num_dv_frames)
  {
    wxLogVerbose(wxT("CIRCDDB::sendHeard:num_dv_silent_frames > num_dv_frames"));
    return false;
  }

  if (num_bit_errors > (4*num_dv_frames)) // max 4 bit errors per frame
  {
    wxLogVerbose(wxT("CIRCDDB::sendHeard:num_bit_errors > (4*num_dv_frames)"));
    return false;
  }

  if (myCall.Len() != 8)
  {
    wxLogVerbose(wxT("CIRCDDB::sendHeard:myCall: len != 8"));
    return false;
  }

  if (myCallExt.Len() != 4)
  {
    wxLogVerbose(wxT("CIRCDDB::sendHeard:myCallExt: len != 4"));
    return false;
  }

  if (yourCall.Len() != 8)
  {
    wxLogVerbose(wxT("CIRCDDB::sendHeard:yourCall: len != 8"));
    return false;
  }

  if (rpt1.Len() != 8)
  {
    wxLogVerbose(wxT("CIRCDDB::sendHeard:rpt1: len != 8"));
    return false;
  }

  if (rpt2.Len() != 8)
  {
    wxLogVerbose(wxT("CIRCDDB::sendHeard:rpt2: len != 8"));
    return false;
  }

  wxString stats = wxString::Format(wxT("%04x"), num_dv_frames);

  if (num_dv_silent_frames >= 0)
  {
    wxString s = wxString::Format(wxT("%02x"), (num_dv_silent_frames * 100) / num_dv_frames);
    stats.Append(s);

    if (num_bit_errors >= 0)
    {
      s = wxString::Format(wxT("%02x"), (num_bit_errors * 125) / (num_dv_frames * 3));
      stats.Append(s);
    }
    else
    {
      stats.Append(wxT("__"));
    }
  }
  else
  {
    stats.Append(wxT("____"));
  }

  stats.Append(wxT("____________"));  // stats string should have 20 chars

  return d->app->sendHeard( myCall, myCallExt, yourCall, rpt1, rpt2, flag1, flag2, flag3,
    wxT("        "), wxT(""), stats);
}



// Send query for a gateway/reflector, a false return implies a network error
bool CIRCDDB::findGateway(const wxString& gatewayCallsign)
{
  if (gatewayCallsign.Len() != 8)
  {
    wxLogVerbose(wxT("CIRCDDB::findGateway: len != 8"));
    return false;
  }

  return d->app->findGateway( gatewayCallsign.Upper());
}


bool CIRCDDB::findRepeater(const wxString& repeaterCallsign)
{
  if (repeaterCallsign.Len() != 8)
  {
    wxLogVerbose(wxT("CIRCDDB::findRepeater: len != 8"));
    return false;
  }

  return d->app->findRepeater( repeaterCallsign.Upper());
}

// Send query for a user, a false return implies a network error
bool CIRCDDB::findUser(const wxString& userCallsign)
{
  if (userCallsign.Len() != 8)
  {
    wxLogVerbose(wxT("CIRCDDB::findUser: len != 8"));
    return false;
  }

  return d->app->findUser( userCallsign.Upper());
}

// The following functions are for processing received messages

// Get the waiting message type
IRCDDB_RESPONSE_TYPE CIRCDDB::getMessageType()
{
  return d->app->getReplyMessageType();
}

// Get a gateway message, as a result of IDRT_REPEATER returned from getMessageType()
// A false return implies a network error
bool CIRCDDB::receiveRepeater(wxString& repeaterCallsign, wxString& gatewayCallsign, wxString& address, DSTAR_PROTOCOL& protocol)
{
  IRCDDB_RESPONSE_TYPE rt = d->app->getReplyMessageType();

  if (rt != IDRT_REPEATER)
  {
    wxLogError(wxT("CIRCDDB::receiveRepeater: unexpected response type"));
    return false;
  }

  IRCMessage * m = d->app->getReplyMessage();

  if (m == NULL)
  {
    wxLogError(wxT("CIRCDDB::receiveRepeater: no message"));
    return false;
  }

  if (!m->getCommand().IsSameAs(wxT("IDRT_REPEATER")))
  {
    wxLogError(wxT("CIRCDDB::receiveRepeater: wrong message type"));
    return false;
  }

  if (m->getParamCount() != 3)
  {
    wxLogError(wxT("CIRCDDB::receiveRepeater: unexpected number of message parameters"));
    return false;
  }

  repeaterCallsign = m->getParam(0);
  gatewayCallsign = m->getParam(1);
  address = m->getParam(2);

  delete m;

  return true;
}

// Get a gateway message, as a result of IDRT_GATEWAY returned from getMessageType()
// A false return implies a network error
bool CIRCDDB::receiveGateway(wxString& gatewayCallsign, wxString& address, DSTAR_PROTOCOL& protocol)
{
  IRCDDB_RESPONSE_TYPE rt = d->app->getReplyMessageType();

  if (rt != IDRT_GATEWAY)
  {
    wxLogError(wxT("CIRCDDB::receiveGateway: unexpected response type"));
    return false;
  }

  IRCMessage * m = d->app->getReplyMessage();

  if (m == NULL)
  {
    wxLogError(wxT("CIRCDDB::receiveGateway: no message"));
    return false;
  }

  if (!m->getCommand().IsSameAs(wxT("IDRT_GATEWAY")))
  {
    wxLogError(wxT("CIRCDDB::receiveGateway: wrong message type"));
    return false;
  }

  if (m->getParamCount() != 2)
  {
    wxLogError(wxT("CIRCDDB::receiveGateway: unexpected number of message parameters"));
    return false;
  }

  gatewayCallsign = m->getParam(0);
  address = m->getParam(1);

  delete m;

  return true;
}

// Get a user message, as a result of IDRT_USER returned from getMessageType()
// A false return implies a network error
bool CIRCDDB::receiveUser(wxString& userCallsign, wxString& repeaterCallsign, wxString& gatewayCallsign, wxString& address)
{
  IRCDDB_RESPONSE_TYPE rt = d->app->getReplyMessageType();

  if (rt != IDRT_USER)
  {
    wxLogError(wxT("CIRCDDB::receiveUser: unexpected response type"));
    return false;
  }

  IRCMessage * m = d->app->getReplyMessage();

  if (m == NULL)
  {
    wxLogError(wxT("CIRCDDB::receiveUser: no message"));
    return false;
  }

  if (!m->getCommand().IsSameAs(wxT("IDRT_USER")))
  {
    wxLogError(wxT("CIRCDDB::receiveUser: wrong message type"));
    return false;
  }

  if (m->getParamCount() != 4)
  {
    wxLogError(wxT("CIRCDDB::receiveUser: unexpected number of message parameters"));
    return false;
  }

  userCallsign = m->getParam(0);
  repeaterCallsign = m->getParam(1);
  gatewayCallsign = m->getParam(2);
  address = m->getParam(3);

  delete m;

  return true;
}

void CIRCDDB::close()		// Implictely kills any threads in the IRC code
{
  d->client -> stopWork();
  d->app -> stopWork();
}

