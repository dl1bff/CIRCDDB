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


#include <wx/init.h>
#include <wx/string.h>
#include <wx/socket.h>
#include <wx/log.h>
#include <wx/utils.h>

#include "IRCDDB.h"



int main (int argc, char *argv[])
{

  wxInitializer init;

  wxLogStderr log;
  log.SetVerbose();


  if (init.IsOk())
  {
    wxLogVerbose(wxT("main: init ok"));
  }
  else
  {
    wxLogError(wxT("main: wxWidgets could not be initialized"));
    return 1;
  }

  CIRCDDB ii( wxT("group1-irc.ircddb.net"), 9007, wxT("u-test"), wxT("secret"));

  wxLogVerbose(wxT("main: before open"));

  ii.open();

  wxLogVerbose(wxT("main program running"));
  wxSleep(10);
  wxLogVerbose(wxT("main program running"));

  ii.close();

  wxLogVerbose(wxT("main: after close"));

  return 0;
}

