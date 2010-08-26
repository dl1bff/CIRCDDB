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
#include <wx/string.h>
#include <wx/thread.h>
#include <wx/socket.h>


#include "IRCClient.h"


WX_DECLARE_LIST( wxIPV4address, IPV4List );
#include <wx/listimpl.cpp>
WX_DEFINE_LIST( IPV4List );


#include <netdb.h>

static IPV4List * getAllIPV4Addresses ( const char * name, unsigned short port )
{
  IPV4List * k = new IPV4List();

  struct addrinfo hints;
  struct addrinfo * res;

  bzero(&hints, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  int r;


  r = getaddrinfo( name, NULL, &hints, &res );

  if (r == 0)
  {
    struct addrinfo * rp;

    for (rp = res; rp != NULL; rp = rp->ai_next)
    {
      if (rp->ai_family == AF_INET)
      {
	struct sockaddr_in * sin = (struct sockaddr_in *) rp->ai_addr;
	unsigned char * addr = (unsigned char *) & sin->sin_addr;

	wxString a = wxString::Format(wxT("%d.%d.%d.%d"),
	  ((unsigned int) addr[0]),
	  ((unsigned int) addr[1]),
	  ((unsigned int) addr[2]),
	  ((unsigned int) addr[3]) ) ;

	wxIPV4address * ip = new wxIPV4address();

	ip->Hostname(a);
	ip->Service(port);

	k->Append(ip);
      }
    }

    freeaddrinfo(res);
  }
  else
  {
    wxString e( gai_strerror(r), wxConvUTF8);

    wxLogWarning(wxT("getaddrinfo: ") + e );
  }


  return k;
}




IRCClient::IRCClient( const wxString& hostName, unsigned int port, const wxString& callsign, const wxString& password )
{
  strncpy(host_name, hostName.mb_str(), sizeof host_name);
  host_name[(sizeof host_name) - 1] = 0;

  this -> callsign = callsign;
  this -> port = port;
  this -> password = password;


  recv = NULL;
}

IRCClient::~IRCClient()
{
}

bool IRCClient::startWork()
{

  if (Create() != wxTHREAD_NO_ERROR)
  {
    wxLogError(wxT("IRCClient::startWork: Could not create the worker thread!"));
    return false;
  }

  terminateThread = false;

  if (GetThread()->Run() != wxTHREAD_NO_ERROR)
  {
    wxLogError(wxT("IRCClient::startWork: Could not run the worker thread!"));
    return false;
  }

  return true;
}

void IRCClient::stopWork()
{
  terminateThread = true;

  if (GetThread() &&  GetThread()->IsRunning())
  {
    GetThread()->Wait();
  }

}

wxThread::ExitCode IRCClient::Entry ()
{
  recv = new IRCReceiver();

  recv->startWork();

  IPV4List * list = getAllIPV4Addresses(host_name, port);

  wxLogVerbose(wxT("NumIP: %d"), list->GetCount());

  IPV4List::iterator iter;
  for (iter = list->begin(); iter != list->end(); ++iter)
  {
    wxIPV4address *current = *iter;

    wxLogVerbose(wxT("IP: %s") + current->IPAddress() );
  }

  delete list;

  while ((!GetThread()->TestDestroy()) && (!terminateThread))
  {

    wxLogVerbose(wxT("IRCClient: tick"));


    wxThread::Sleep(600);

  }

  recv->stopWork();

  return 0;
}




