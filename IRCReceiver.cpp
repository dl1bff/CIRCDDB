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


#include <wx/thread.h>
#include <wx/log.h>

#include "IRCReceiver.h"


IRCReceiver::IRCReceiver( )
{
}

IRCReceiver::~IRCReceiver()
{
}

bool IRCReceiver::startWork()
{

  if (Create() != wxTHREAD_NO_ERROR)
  {
    wxLogError(wxT("IRCReceiver::startWork: Could not create the worker thread!"));
    return false;
  }

  terminateThread = false;

  if (GetThread()->Run() != wxTHREAD_NO_ERROR)
  {
    wxLogError(wxT("IRCReceiver::startWork: Could not run the worker thread!"));
    return false;
  }

  return true;
}

void IRCReceiver::stopWork()
{
  terminateThread = true;

  if (GetThread() &&  GetThread()->IsRunning())
  {
    GetThread()->Wait();
  }

}

wxThread::ExitCode IRCReceiver::Entry ()
{
  while ((!GetThread()->TestDestroy()) && (!terminateThread))
  {

    wxLogVerbose(wxT("IRCReceiver: tick"));


    wxThread::Sleep(500);

  }

  return 0;
}

