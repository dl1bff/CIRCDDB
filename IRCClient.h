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


#if !defined(_IRCCLIENT_H)
#define _IRCCLIENT_H

#include "IRCReceiver.h"
#include "IRCMessageQueue.h"
#include "IRCProtocol.h"
#include "IRCApplication.h"

#include <wx/wx.h>


class IRCClient : public wxThreadHelper
{
  public:

  IRCClient( IRCApplication * app, const wxString& update_channel,
      const wxString& hostName, unsigned int port, const wxString& callsign, const wxString& password );

  ~IRCClient();


  bool startWork();

  void stopWork();


  protected:

  virtual wxThread::ExitCode Entry();



  private:

  char host_name[100];
  unsigned int port;
  wxString callsign;
  wxString password;

  bool terminateThread;

  IRCReceiver * recv;
  IRCMessageQueue * recvQ;
  IRCMessageQueue * sendQ;
  IRCProtocol * proto;

  IRCApplication * app;

};


#endif 
