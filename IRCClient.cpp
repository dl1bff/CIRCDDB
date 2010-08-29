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



#include "IRCClient.h"

#include <wx/wx.h>


#if defined(__WINDOWS__)
#include <winsock.h>

#include "getaddrinfo.h"

#else
#include <netdb.h>
#endif


#include <fcntl.h>
#include <errno.h>



static int getAllIPV4Addresses ( const char * name, unsigned short port,
    unsigned int * num, struct sockaddr_in * addr, unsigned int max_addr )
{

#if defined(__WINDOWS__)
    // Initialize Winsock
  WSADATA wsaData;
  int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        wxLogError(wxT("WSAStartup failed: %d"), iResult);
        return 1;
    }
#endif


  struct addrinfo hints;
  struct addrinfo * res;

  memset(&hints, 0x00, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  int r = getaddrinfo( name, NULL, &hints, &res );

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

      for (rp = res, i=0 ; (rp != NULL) && (i < numAddr); rp = rp->ai_next)
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




IRCClient::IRCClient( IRCApplication * app, const wxString& update_channel,
    const wxString& hostName, unsigned int port, const wxString& callsign, const wxString& password )
{
  strncpy(host_name, hostName.mb_str(), sizeof host_name);
  host_name[(sizeof host_name) - 1] = 0;

  this -> callsign = callsign;
  this -> port = port;
  this -> password = password;

  this -> app = app;

  proto = new IRCProtocol ( app, callsign, password, update_channel );

  recvQ = NULL;
  sendQ = NULL;

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

  unsigned int numAddr;

#define MAXIPV4ADDR 10
  struct sockaddr_in addr[MAXIPV4ADDR];

  int state = 0;
  int timer = 0;
  int sock;
  unsigned int currentAddr;

  while (!GetThread()->TestDestroy())
  {

    if (timer > 0)
    {
      timer --;
    }

    switch (state)
    {
    case 0:
      if (terminateThread)
      {
	wxLogVerbose(wxT("IRCClient::Entry: thread terminated at state=%d"), state);
	return 0;
      }
      
      if (timer == 0)
      {
	timer = 30;

	if (getAllIPV4Addresses(host_name, port, &numAddr, addr, MAXIPV4ADDR) == 0)
	{
	  wxLogVerbose(wxT("IRCClient::Entry: number of DNS entries %d"), numAddr);
	  if (numAddr > 0)
	  {
	    currentAddr = 0;
	    state = 1;
	    timer = 0;
	  }
	}
      }
      break;

    case 1:
      if (terminateThread)
      {
	wxLogVerbose(wxT("IRCClient::Entry: thread terminated at state=%d"), state);
	return 0;
      }
      
      if (timer == 0)
      {
	sock = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sock < 0)
	{
	  wxLogSysError(wxT("IRCClient::Entry: socket"));
	  timer = 30;
	  state = 0;
	}
	else
	{
#if defined(__WINDOWS__)
	  u_long nonBlock = 1UL;
	  if (ioctlsocket( sock, FIONBIO, &nonBlock ) != 0)
	  {
	    wxLogSysError(wxT("IRCClient::Entry: ioctlsocket"));
	    closesocket(sock);
	    timer = 30;
	    state = 0;
	  }
#else
	  if (fcntl( sock, F_SETFL, O_NONBLOCK ) < 0)
	  {
	    wxLogSysError(wxT("IRCClient::Entry: fcntl"));
	    close(sock);
	    timer = 30;
	    state = 0;
	  }
#endif
	  else
	  {
	    unsigned char * h = (unsigned char *) &(addr[currentAddr].sin_addr);
	    wxLogVerbose(wxT("IRCClient::Entry: trying to connect to %d.%d.%d.%d"), 
		   h[0], h[1], h[2], h[3]);
		
	    int res = connect(sock, (struct sockaddr *) (addr + currentAddr), sizeof (struct sockaddr_in));

	    if (res == 0)
	    {
	      wxLogVerbose(wxT("IRCClient::Entry: connected"));
	      state = 4;
	    }
	    else 
	    { 
#if defined(__WINDOWS__)
	      if (WSAGetLastError() == WSAEWOULDBLOCK)
#else
	      if (errno == EINPROGRESS)
#endif
	      {
		wxLogVerbose(wxT("IRCClient::Entry: connect in progress"));
		state = 3;
		timer = 10;  // 5 second timeout
	      }
	      else
	      {
		wxLogSysError(wxT("IRCClient::Entry: connect"));
#if defined(__WINDOWS__)
		closesocket(sock);
#else
		close(sock);
#endif
		currentAddr ++;
		if (currentAddr >= numAddr)
		{
		  state = 0;
		  timer = 30;
		}
		else
		{
		  state = 1;
		  timer = 4;
		}
	      }
	    }
	  } // connect
	}
      }
      break;
    
    case 3:
      {
	struct timeval tv;
	tv.tv_sec = 0; 
        tv.tv_usec = 0; 
	fd_set myset;
	FD_ZERO(&myset); 
	FD_SET(sock, &myset); 
	int res;
	res = select(sock+1, NULL, &myset, NULL, &tv); 

	if (res < 0)
	{
	  wxLogSysError(wxT("IRCClient::Entry: select"));
#if defined(__WINDOWS__)
	  closesocket(sock);
#else
	  close(sock);
#endif
	  state = 0;
	  timer = 30;
	}
	else if (res > 0) // connect is finished
	{
#if defined(__WINDOWS__)
	  int val_len;
#else
	  socklen_t val_len;
#endif
	  int value;

	  val_len = sizeof value;

	  if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *) &value, &val_len) < 0)
	  {
	     wxLogSysError(wxT("IRCClient::Entry: getsockopt"));
#if defined(__WINDOWS__)
	     closesocket(sock);
#else
	     close(sock);
#endif
	     state = 0;
	     timer = 30;
	  }
	  else
	  {
	    if (value != 0)
	    {
	      wxLogWarning(wxT("IRCClient::Entry: SO_ERROR=%d"), value);
#if defined(__WINDOWS__)
	      closesocket(sock);
#else
	      close(sock);
#endif
	      currentAddr ++;
	      if (currentAddr >= numAddr)
	      {
		state = 0;
		timer = 30;
	      }
	      else
	      {
		state = 1;
		timer = 2;
	      }
	    }
	    else
	    {
	      wxLogVerbose(wxT("IRCClient::Entry: connected2"));
	      state = 4;
	    }
	  }

	}
	else if (timer == 0)
	{  // select timeout and timer timeout
	  wxLogVerbose(wxT("IRCClient::Entry: connect timeout"));
#if defined(__WINDOWS__)
	  closesocket(sock);
#else
	  close(sock);
#endif
	  currentAddr ++;
	  if (currentAddr >= numAddr)
	  {
	    state = 0;
	    timer = 30;
	  }
	  else
	  {
	    state = 1; // open new socket
	    timer = 2;
	  }
	}

      }
      break;

    case 4:
      {
	recvQ = new IRCMessageQueue();
	sendQ = new IRCMessageQueue();

	recv = new IRCReceiver(sock, recvQ);
	recv->startWork();

	proto->setNetworkReady(true);
	state = 5;
	timer = 0;

      }
      break;


    case 5:
      if (terminateThread)
      {
	state = 6;
      }
      else
      {

	if (recvQ -> isEOF())
	{
	  timer = 0;
	  state = 6;
	}
	else if (proto -> processQueues(recvQ, sendQ) == false)
	{
	  timer = 0;
	  state = 6;
	}

	while ((state == 5) && sendQ->messageAvailable())
	{
	  IRCMessage * m = sendQ -> getMessage();

	  wxString out;

	  m -> composeMessage ( out );

	  char buf[200];
	  strncpy(buf, out.mb_str(wxConvUTF8), sizeof buf);
	  buf[(sizeof buf) - 1] = 0;
	  int len = strlen(buf);

	  if (buf[len - 1] == 10)  // is there a NL char at the end?
	  {
	    int r = write(sock, buf, len);

	    if (r != len)
	    {
	      wxLogVerbose(wxT("IRCClient::Entry: short write %d < %d"), r, len);

	      timer = 0;
	      state = 6;
	    }
/*	    else
	    {
	      wxLogVerbose(wxT("write %d bytes (") + out + wxT(")"), len );
	    } */
	  }
	  else
	  {
	      wxLogVerbose(wxT("IRCClient::Entry: no NL at end, len=%d"), len);

	      timer = 0;
	      state = 6;
	  }

	  delete m;
	}
      }
      break;

    case 6:
      {
	if (app != NULL)
	{
	  app->setSendQ(NULL);
	  app->userListReset();
	}

	proto->setNetworkReady(false);
	recv->stopWork();

	wxThread::Sleep(2000);

	delete recv;
	delete recvQ;
	delete sendQ;

#if defined(__WINDOWS__)
	closesocket(sock);
#else
	close(sock);
#endif

	if (terminateThread) // request to end the thread
	{
	  wxLogVerbose(wxT("IRCClient::Entry: thread terminated at state=%d"), state);
	  return 0;
	}

	timer = 30;
	state = 0;  // reconnect to IRC server
      }
      break;

    }

    wxThread::Sleep(500);

  }

  return 0;
}





