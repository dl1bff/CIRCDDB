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

#if !defined(_IRC_MESSAGE_H)
#define _IRC_MESSAGE_H

#include <wx/wx.h>


class IRCMessage
{
  public:

    IRCMessage();

    IRCMessage( const wxString& toNick, const wxString& msg );

    IRCMessage( const wxString& command );

    ~IRCMessage();



    wxString prefix;
    wxString command;
    wxArrayString params;

    int numParams;

    wxString& getPrefixNick();
    wxString& getPrefixName();
    wxString& getPrefixHost();

    void composeMessage ( wxString& output );

    void addParam( const wxString& p );

    wxString getCommand();

    wxString getParam( int pos );

    int getParamCount();

  private:

    void parsePrefix();

    wxArrayString prefixComponents;
    bool prefixParsed;

};

#endif
