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

#include "IRCClient.h"

#include <netdb.h>



static int getAllIPV4Addresses ( const char * name, unsigned short port,
    unsigned int * num, struct sockaddr_in * addr, unsigned int max_addr )
{
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
    unsigned int numAddr = 0;

    for (rp = res; rp != NULL; rp = rp->ai_next)
    {
      if (rp->ai_family == AF_INET)
      {
	numAddr ++;
      }
    }

    if (numAddr > 0)
    {
      if (numAddr > max_addr)
      {
	numAddr = max_addr;
      }

      int * shuffle = new int[numAddr];

      unsigned int i;

      for (i=0; i < numAddr; i++)
      {
	shuffle[i] = i;
      }
      
      for (i=0; i < (numAddr - 1); i++)
      {
	if (rand() & 1)
	{
	  int tmp;
	  tmp = shuffle[i];
	  shuffle[i] = shuffle[i+1];
	  shuffle[i+1] = tmp;
	}
      }

      for (i=(numAddr - 1); i > 0; i--)
      {
	if (rand() & 1)
	{
	  int tmp;
	  tmp = shuffle[i];
	  shuffle[i] = shuffle[i-1];
	  shuffle[i-1] = tmp;
	}
      }

      for (rp = res, i=0 ; rp != NULL; rp = rp->ai_next)
      {
	if (rp->ai_family == AF_INET)
	{
	  memcpy( addr+shuffle[i], rp->ai_addr, sizeof (struct sockaddr_in) );

	  addr[shuffle[i]].sin_port = htons(port);

	  i++;
	}
      }

      delete shuffle;
    }

    *num = numAddr;

    freeaddrinfo(res);

    return 0;

  }
  else
  {
    wxString e( gai_strerror(r), wxConvUTF8);

    wxLogWarning(wxT("getaddrinfo: ") + e );

    return 1;
  }


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

  unsigned int num;

#define MAXIPV4ADDR 10
  struct sockaddr_in addr[MAXIPV4ADDR];

  if (getAllIPV4Addresses(host_name, port, &num, addr, MAXIPV4ADDR) == 0)
  {
    wxLogVerbose(wxT("NumIP: %d"), num);

    if (num > 0)
    {
      for (unsigned int i=0; i < num; i++)
      {
	unsigned char * h = (unsigned char *) (addr + i);

	for (int j=0; j < 8; j++)
	{

	  wxLogVerbose(wxString::Format(wxT(" %d") , h[j]));
	}
      }
    }
  }

  while ((!GetThread()->TestDestroy()) && (!terminateThread))
  {

    wxLogVerbose(wxT("IRCClient: tick"));


    wxThread::Sleep(600);

  }

  recv->stopWork();

  return 0;
}




