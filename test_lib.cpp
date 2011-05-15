/*

CIRCDDB - ircDDB client library in C++

Copyright (C) 2010-2011   Michael Dirska, DL1BFF (dl1bff@mdx.de)

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

#include <wx/wx.h>


wxArrayString userList;
wxArrayString rptrList;
wxArrayString gwList;


int main (int argc, char *argv[])
{

  userList.Add(wxT("DL1BFF  "));
  userList.Add(wxT("DG8NGN  "));
  userList.Add(wxT("DL5DI   "));
  userList.Add(wxT("Y35O    "));

  rptrList.Add(wxT("XRF023 A"));
  rptrList.Add(wxT("DB0DF  B"));
  rptrList.Add(wxT("DB0MYK B"));
  rptrList.Add(wxT("DB0VOX B"));

  gwList.Add(wxT("DB0VOX G"));
  gwList.Add(wxT("DB0DF  G"));
  gwList.Add(wxT("Y21O   G"));
  gwList.Add(wxT("DB0TVM G"));

  srand(time(0));

  wxInitializer init;

  wxLogStderr log;
  log.SetVerbose();

#if defined(__WINDOWS__)
     // Initialize Winsock
  WSADATA wsaData;
  int iResult = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0)
  {
    wxLogError(wxT("WSAStartup failed: %d"), iResult);
    return 1;
  }
#endif

  if (init.IsOk())
  {
    wxLogVerbose(wxT("main: init ok"));
  }
  else
  {
    wxLogError(wxT("main: wxWidgets could not be initialized"));
    return 1;
  }

  if (argc < 3)
  {
    wxLogError(wxT("Usage: test_lib <username> <password> [<local bind address>]"));
    return 1;
  }

  wxString localAddr = wxEmptyString;

  if (argc >= 4)
  {
    localAddr =  wxString(argv[3], wxConvUTF8);
  }

  CIRCDDB ii( wxT("group1-irc.ircddb.net"), 9007,
      wxString(argv[1], wxConvUTF8),
      wxString(argv[2], wxConvUTF8),
      wxT("test_lib:20110515"),
      localAddr); 

  ii.rptrQTH( 52, 13, wxT("line1"), wxT("line2"), wxT("http://example.com/"));

  ii.rptrQRG( wxT("A"), 1298, -28, 100, 5 );
  ii.rptrQRG( wxT("AD"), 1270, 0, 100, 5 );
  ii.rptrQRG( wxT("B"), 439, -7.6, 100, 5 );

  wxLogVerbose(wxT("main: before open"));

  ii.open();

  wxLogVerbose(wxT("main program running"));

  bool keep_running = true;

  for (int i=0; (i < 7200) && keep_running; i++)
  {

    wxLogVerbose(wxT("CONNECTION: %d"), ii.getConnectionState());

    if ( (i & 0x03) == 0 )
    {
      wxString s;
      switch ( i & 0x3c )
      {
	case 0:
	  s = userList.Item( (i >> 4) & 0x03 );
	  ii.findUser(s);
	  wxLogVerbose(wxT("REQUSER: (") + s + wxT(")"));
	  break;

	case 0x04:
	  s = rptrList.Item( (i >> 4) & 0x03 );
	  ii.findRepeater(s);
	  wxLogVerbose(wxT("REQRPTR: (") + s + wxT(")"));
	  break;

	case 0x08:
	  s = gwList.Item( (i >> 4) & 0x03 );
	  ii.findGateway(s);
	  wxLogVerbose(wxT("REQGWAY: (") + s + wxT(")"));
	  break;

	case 0x10: 
	  ii.sendHeard(
	      wxT("DL1BFF  "),
	      wxT("TEST"),
	      wxT("CQCQCQ  "),
	      wxT("DB0DF  B"),
	      wxT("DB0DF  G"),
	      0, (i & 255), ((i >> 8) & 255));

	  ii.sendHeardWithTXMsg(
	      wxT("DL1BFF  "),
	      wxT("TEST"),
	      wxT("CQCQCQ  "),
	      wxT("DB0DF  B"),
	      wxT("DB0DF  G"),
	      0, 0, 0,
	      wxT(""),
	      wxT("\thttp://ircddb.net\n\n"));

	  ii.sendHeardWithTXStats(
	      wxT("DL1BFF  "),
	      wxT("TEST"),
	      wxT("CQCQCQ  "),
	      wxT("DB0DF  B"),
	      wxT("DB0DF  G"),
	      0, 0, 0,
	      500, 250, 120 );
	  break;

	case 0x20:
	  ii.rptrQRG( wxT("A"), 10000.0 + i, -28, 100, 5 );
	  break;
	    
      }
    }


    IRCDDB_RESPONSE_TYPE res;

    while ((res = ii.getMessageType()) != IDRT_NONE)
    {
      switch (res)
      {
	case IDRT_USER:
	  {
	    wxString user;
	    wxString rptr;
	    wxString gateway;
	    wxString ipaddr;

	    ii.receiveUser(user, rptr, gateway, ipaddr);
	    wxLogVerbose(wxT("USER: (") + user + wxT(") (") + rptr +
		wxT(") (") + gateway + wxT(") (") + ipaddr + wxT(")"));
	  }
	  break;

	case IDRT_REPEATER:
	  {
	    wxString rptr;
	    wxString gateway;
	    wxString ipaddr;
	    DSTAR_PROTOCOL proto;

	    ii.receiveRepeater(rptr, gateway, ipaddr, proto);
	    wxLogVerbose(wxT("RPTR: (") + rptr +
		wxT(") (") + gateway + wxT(") (") + ipaddr + wxT(")"));
	  }
	  break;

	case IDRT_GATEWAY:
	  {
	    wxString gateway;
	    wxString ipaddr;
	    DSTAR_PROTOCOL proto;

	    ii.receiveGateway(gateway, ipaddr, proto);
	    wxLogVerbose(wxT("GWAY: (") +
		 gateway + wxT(") (") + ipaddr + wxT(")"));
	  }
	  break;

	default:
	  wxLogError(wxT("unknown message type!!"));
	  keep_running = false;
      }

      if (!keep_running) break;
    }

    wxSleep(1);

    ii.kickWatchdog(wxT("test_lib"));
  }
  wxLogVerbose(wxT("main program running"));

  ii.close();

  wxLogVerbose(wxT("main: after close"));

#if defined(__WINDOWS__)
  ::WSACleanup();
#endif

  return 0;
}

