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

#if !defined(_IRCDDBAPP_H)
#define _IRCDDBAPP_H


#include "IRCApplication.h"

#include <wx/wx.h>

struct IRCDDBAppPrivate;

class IRCDDBApp : public IRCApplication, wxThread
{
  public:
    IRCDDBApp(const wxString& update_channel);

    virtual ~IRCDDBApp();

    virtual void userJoin (const wxString& nick, const wxString& name, const wxString& host);

    virtual void userLeave (const wxString& nick);

    virtual void userChanOp (const wxString& nick, bool op);
    virtual void userListReset();

    virtual void msgChannel (IRCMessage * m);
    virtual void msgQuery (IRCMessage * m);

    virtual void setCurrentNick(const wxString& nick);
    virtual void setTopic(const wxString& topic);

    virtual void setSendQ( IRCMessageQueue * s );
    virtual IRCMessageQueue * getSendQ ();

    bool startWork();
    void stopWork();

  protected:
    virtual wxThread::ExitCode Entry();

  private:
    bool findServerUser();
    void enablePublicUpdates();
    IRCDDBAppPrivate * d;
};


#endif
