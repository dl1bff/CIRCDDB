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

#include <wx/log.h>
#include <wx/thread.h>

#include "IRCDDB.h"

#include "IRCClient.h"

struct CIRCDDBPrivate
{
    IRCClient * client;
};


CIRCDDB::CIRCDDB(const wxString& hostName, unsigned int port,
    const wxString& callsign, const wxString& password) : d( new CIRCDDBPrivate )

{
  d->client = new IRCClient( hostName, port, callsign, password );

}

CIRCDDB::~CIRCDDB()
{
  delete d->client;
  delete d;
}


	// A false return implies a network error, or unable to log in
bool CIRCDDB::open()
{
  wxLogVerbose(wxT("start"));
  return d->client -> startWork();
}

// The following three functions don't block waiting for a reply, they just send the data

// Send heard data, a false return implies a network error
bool CIRCDDB::sendHeard(const wxString& userCallsign, const wxString& repeaterCallsign)
{
  return true;
}

// Send query for a gateway/reflector, a false return implies a network error
bool CIRCDDB::findGateway(const wxString& gatewayCallsign)
{
  return true;
}

// Send query for a user, a false return implies a network error
bool CIRCDDB::findUser(const wxString& userCallsign)
{
  return true;
}

// The following functions are for processing received messages

// Get the waiting message type
IRCDDB_RESPONSE_TYPE CIRCDDB::getMessageType()
{
  return IDRT_NONE;
}

// Get a gateway message, as a result of IDRT_GATEWAY returned from getMessageType()
// A false return implies a network error
bool CIRCDDB::receiveGateway(wxString& gatewayCallsign, wxString& address, DSTAR_PROTOCOL& protocol)
{
  return true;
}

// Get a user message, as a result of IDRT_USER returned from getMessageType()
// A false return implies a network error
bool CIRCDDB::receiveUser(wxString& userCallsign, wxString& repeaterCallsign, wxString& address)
{
  return true;
}

void CIRCDDB::close()		// Implictely kills any threads in the IRC code
{
  d->client -> stopWork();
}

