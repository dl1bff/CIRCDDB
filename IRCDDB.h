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


#if !defined(_IRCDDB_H)
#define _IRCDDB_H

#include <wx/wx.h>

enum IRCDDB_RESPONSE_TYPE {
	IDRT_NONE,
	IDRT_USER,
	IDRT_GATEWAY,
	IDRT_REPEATER
};

enum DSTAR_PROTOCOL {
	DP_UNKNOWN,
	DP_DEXTRA,
	DP_DPLUS
};

struct CIRCDDBPrivate;

class CIRCDDB {
public:
	CIRCDDB(const wxString& hostName, unsigned int port, const wxString& callsign, const wxString& password,
	    const wxString& versionInfo );
	~CIRCDDB();

	// A false return implies a network error, or unable to log in
	bool open();

	// The following three functions don't block waiting for a reply, they just send the data

	// Send heard data, a false return implies a network error
	bool sendHeard(const wxString& myCall, const wxString& myCallExt,
	          const wxString& yourCall, const wxString& rpt1,
		  const wxString& rpt2, unsigned char flag1,
		  unsigned char flag2, unsigned char flag3 );

	// Send query for a gateway/reflector, a false return implies a network error
	bool findGateway(const wxString& gatewayCallsign);

	// Send query for a repeater module, a false return implies a network error
	bool findRepeater(const wxString& repeaterCallsign);

	// Send query for a user, a false return implies a network error
	bool findUser(const wxString& userCallsign);

	// The following functions are for processing received messages
	
	// Get the waiting message type
	IRCDDB_RESPONSE_TYPE getMessageType();

	// Get a gateway message, as a result of IDRT_REPEATER returned from getMessageType()
	// A false return implies a network error
	bool receiveRepeater(wxString& repeaterCallsign, wxString& gatewayCallsign, wxString& address, DSTAR_PROTOCOL& protocol);

	// Get a gateway message, as a result of IDRT_GATEWAY returned from getMessageType()
	// A false return implies a network error
	bool receiveGateway(wxString& gatewayCallsign, wxString& address, DSTAR_PROTOCOL& protocol);

	// Get a user message, as a result of IDRT_USER returned from getMessageType()
	// A false return implies a network error
	bool receiveUser(wxString& userCallsign, wxString& repeaterCallsign, wxString& gatewayCallsign, wxString& address);

	void close();		// Implictely kills any threads in the IRC code


private:
	struct CIRCDDBPrivate * const d;

};

#endif  // _IRCDDB_H

