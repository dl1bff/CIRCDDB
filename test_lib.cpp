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

#include <wx/wx.h>


int main (int argc, char *argv[])
{

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
    wxLogError(wxT("Usage: test_lib <username> <password>"));
    return 1;
  }

  CIRCDDB ii( wxT("group1-irc.ircddb.net"), 9007,
      wxString(argv[1], wxConvUTF8),
      wxString(argv[2], wxConvUTF8) ); 


  wxLogVerbose(wxT("main: before open"));

  ii.open();

  wxLogVerbose(wxT("main program running"));

  bool keep_running = true;

  for (int i=0; (i < 1800) && keep_running; i++)
  {
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
  }
  wxLogVerbose(wxT("main program running"));

  ii.close();

  wxLogVerbose(wxT("main: after close"));

#if defined(__WINDOWS__)
  ::WSACleanup();
#endif

  return 0;
}

