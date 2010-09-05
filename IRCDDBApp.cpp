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

#include "IRCDDBApp.h"

#include <wx/datetime.h>
#include <wx/regex.h>
#include <wx/tokenzr.h>

class IRCDDBAppUserObject
{
  public:

  wxString nick;
  wxString name;
  wxString host;
  bool op;
  unsigned int usn;

  IRCDDBAppUserObject ()
  {
    IRCDDBAppUserObject (wxT(""), wxT(""), wxT(""));
  }
  
  IRCDDBAppUserObject ( const wxString& n, const wxString& nm, const wxString& h )
  {
    nick = n;
    name = nm;
    host = h;
    op = false;
    usn = counter;
    counter ++;
  }

  static unsigned int counter;
};

unsigned int IRCDDBAppUserObject::counter = 0;


WX_DECLARE_STRING_HASH_MAP( IRCDDBAppUserObject, IRCDDBAppUserMap );


class IRCDDBAppRptrObject
{
  public:

  wxString arearp_cs;
  wxDateTime lastChanged;
  wxString zonerp_cs;

  IRCDDBAppRptrObject ()
  {
  }

  IRCDDBAppRptrObject (wxDateTime& dt, wxString& repeaterCallsign, wxString& gatewayCallsign)
  {
    arearp_cs = repeaterCallsign;
    lastChanged = dt;
    zonerp_cs = gatewayCallsign;

    if (dt.IsLaterThan(maxTime))
    {
      maxTime = dt;
    }
  }

  static wxDateTime maxTime;
}; 

wxDateTime IRCDDBAppRptrObject::maxTime((time_t) 950000000);  // February 2000


WX_DECLARE_STRING_HASH_MAP( IRCDDBAppRptrObject, IRCDDBAppRptrMap );


class IRCDDBAppPrivate
{
  public:

  IRCDDBAppPrivate()
  : tablePattern(wxT("^[0-9]$")),
    datePattern(wxT("^20[0-9][0-9]-((1[0-2])|(0[1-9]))-((3[01])|([12][0-9])|(0[1-9]))$")),
    timePattern(wxT("^((2[0-3])|([01][0-9])):[0-5][0-9]:[0-5][0-9]$")),
    dbPattern(wxT("^[0-9A-Z_]{8}$"))
  {}

  IRCMessageQueue * sendQ;

  IRCDDBAppUserMap user;
  wxMutex userMapMutex;

  wxString currentServer;
  wxString myNick;

  wxRegEx tablePattern;
  wxRegEx datePattern;
  wxRegEx timePattern;
  wxRegEx dbPattern;

  int state;
  int timer;

  wxString updateChannel;
  wxString channelTopic;

  bool acceptPublicUpdates;

  bool terminateThread;

  IRCDDBAppRptrMap rptrMap;
  wxMutex rptrMapMutex;

  IRCMessageQueue replyQ;
};

	
IRCDDBApp::IRCDDBApp( const wxString& u_chan )
  : wxThread(wxTHREAD_JOINABLE),
    d(new IRCDDBAppPrivate)
{

  d->sendQ = NULL;
  d->acceptPublicUpdates = false;

  userListReset();
		
  d->state = 0;
  d->timer = 0;
  d->myNick = wxT("none");
		
  d->updateChannel = u_chan;

  d->terminateThread = false;
		
}

IRCDDBApp::~IRCDDBApp()
{
  if (d->sendQ != NULL)
  {
    delete d->sendQ;
  }
  delete d;
}

IRCDDB_RESPONSE_TYPE IRCDDBApp::getReplyMessageType()
{
  IRCMessage * m = d->replyQ.peekFirst();
  if (m == NULL)
  {
    return IDRT_NONE;
  }

  wxString msgType = m->getCommand();

  if (msgType.IsSameAs(wxT("IDRT_USER")))
  {
    return IDRT_USER;
  }
  else if (msgType.IsSameAs(wxT("IDRT_REPEATER")))
  {
    return IDRT_REPEATER;
  }
  else if (msgType.IsSameAs(wxT("IDRT_GATEWAY")))
  {
    return IDRT_GATEWAY;
  }

  wxLogError(wxT("IRCDDBApp::getMessageType: unknown msg type"));

  return IDRT_NONE;
}


IRCMessage *  IRCDDBApp::getReplyMessage()
{
  return d->replyQ.getMessage();
}



bool IRCDDBApp::startWork()
{

  if (Create() != wxTHREAD_NO_ERROR)
  {
    wxLogError(wxT("IRCClient::startWork: Could not create the worker thread!"));
    return false;
  }

  d->terminateThread = false;

  if (Run() != wxTHREAD_NO_ERROR)
  {
    wxLogError(wxT("IRCClient::startWork: Could not run the worker thread!"));
    return false;
  }

  return true;
}

void IRCDDBApp::stopWork()
{
    d->terminateThread = true;

    Wait();
}

	
void IRCDDBApp::userJoin (const wxString& nick, const wxString& name, const wxString& host)
{
  wxMutexLocker lock(d->userMapMutex);

  IRCDDBAppUserObject u( nick, name, host );
		
  d->user[nick] = u;

  if (d->acceptPublicUpdates)
  {
    int hyphenPos = nick.Find(wxT('-'));

    if ((hyphenPos >= 4) && (hyphenPos <= 6))
    {
      wxString gatewayCallsign = nick.Mid(0, hyphenPos).Upper();

      while (gatewayCallsign.Length() < 7)
      {
	gatewayCallsign.Append(wxT(' '));
      }

      gatewayCallsign.Append(wxT('G'));

      IRCMessage * m2 = new IRCMessage(wxT( "IDRT_GATEWAY"));
      m2->addParam(gatewayCallsign);
      m2->addParam(host);
      d->replyQ.putMessage(m2);
    }
  }

  // wxLogVerbose(wxT("user %d"), u.usn );
}

void IRCDDBApp::userLeave (const wxString& nick)
{
  wxMutexLocker lock(d->userMapMutex);

  d->user.erase(nick);

  if (d->currentServer.Len() > 0)
  {
    IRCDDBAppUserMap::iterator i = d->user.find( nick );

    if (i == d->user.end())
    {
      return;
    }

    IRCDDBAppUserObject me = d->user[d->myNick];

    if (me.op == false)  
    {
	    // if I am not op, then look for new server

      if (d->currentServer.IsSameAs(nick))
      {
	      // currentServer = null;
	      d->state = 2;  // choose new server
	      d->timer = 200;
	      d->acceptPublicUpdates = false;
      }
    }
  }
}

void IRCDDBApp::userListReset()
{
  wxMutexLocker lock(d->userMapMutex);

  d->user.clear();
}

void IRCDDBApp::setCurrentNick(const wxString& nick)
{
  d->myNick = nick;
}

void IRCDDBApp::setTopic(const wxString& topic)
{
  d->channelTopic = topic;
}

bool IRCDDBApp::findServerUser()
{
  wxMutexLocker lock(d->userMapMutex);

  bool found = false;

  IRCDDBAppUserMap::iterator it;

  for( it = d->user.begin(); it != d->user.end(); ++it )
  {
    wxString key = it->first;
    IRCDDBAppUserObject u = it->second;

    if (u.nick.StartsWith(wxT("s-")) && u.op && !d->myNick.IsSameAs(u.nick))
    {
      d->currentServer = u.nick;
      found = true;
      break;
    }
  }

  return found;
}
	
void IRCDDBApp::userChanOp (const wxString& nick, bool op)
{
  wxMutexLocker lock(d->userMapMutex);

  IRCDDBAppUserMap::iterator i = d->user.find( nick );

  if (i != d->user.end())
  {
    d->user[nick].op = op;
  }
}
	

void IRCDDBApp::enablePublicUpdates()
{
  d->acceptPublicUpdates = true;
}

static const int numberOfTables = 2;





wxString IRCDDBApp::getIPAddress(wxString& zonerp_cs)
{
  wxString gw = zonerp_cs;

  gw.Replace(wxT("_"), wxT(" "));
  gw.LowerCase();

  unsigned int max_usn = 0;
  wxString ipAddr;

  int j;

  for (j=1; j <= 4; j++)
  {
    wxString ircUser = gw.Strip() + wxString::Format(wxT("-%d"), j);

    // wxLogVerbose(ircUser);

    IRCDDBAppUserMap::iterator i = d->user.find( ircUser );

    if (i != d->user.end())
    {
      IRCDDBAppUserObject o = d->user[ ircUser ];

      if (o.usn > max_usn)
      {
	max_usn = o.usn;
	ipAddr = o.host;
      }
    }
  }

  return ipAddr;
}



void IRCDDBApp::msgChannel (IRCMessage * m)
{
  if (m->getPrefixNick().StartsWith(wxT("s-")) && (m->numParams >= 2))  // server msg
  {
    doUpdate(m->params[1]);
  }
}


void IRCDDBApp::doUpdate ( wxString& msg )
{
    int tableID = 0;

    wxStringTokenizer tkz(msg);

    if (!tkz.HasMoreTokens()) 
    {
      return;  // no text in message
    }

    wxString tk = tkz.GetNextToken();


    if (d->tablePattern.Matches(tk))
    {
      long i;

      if (tk.ToLong(&i))
      {
	tableID = i;
	if ((tableID < 0) || (tableID >= numberOfTables))
	{
	  wxLogVerbose(wxT("invalid table ID %d"), tableID);
	  return;
	}
      }
      else
      {
	return; // not a valid number
      }

      if (!tkz.HasMoreTokens()) 
      {
	return;  // received nothing but the tableID
      }

      tk = tkz.GetNextToken();
    }

    if (d->datePattern.Matches(tk))
    {
      if (!tkz.HasMoreTokens()) 
      {
	return;  // nothing after date string
      }

      wxString timeToken = tkz.GetNextToken();

      if (! d->timePattern.Matches(timeToken))
      {
	return; // no time string after date string
      }

      wxDateTime dt;

      if (dt.ParseFormat(tk + wxT(" ") + timeToken, wxT("%Y-%m-%d %H:%M:%S")) == NULL)
      {
	return; // date+time parsing failed
      }

      if ((tableID == 0) || (tableID == 1))
      {
	if (!tkz.HasMoreTokens())
	{
	  return;  // nothing after time string
	}

	wxString key = tkz.GetNextToken();

	if (! d->dbPattern.Matches(key))
	{
	  return; // no valid key
	}

	if (!tkz.HasMoreTokens())
	{
	  return;  // nothing after time string
	}

	wxString value = tkz.GetNextToken();

	if (! d->dbPattern.Matches(value))
	{
	  return; // no valid key
	}

	//wxLogVerbose(wxT("TABLE %d ") + key + wxT(" ") + value, tableID );


	if (tableID == 1)
	{
	  wxMutexLocker lock(d->rptrMapMutex);

	  IRCDDBAppRptrObject newRptr(dt, key, value);

	  d->rptrMap[key] = newRptr;

	  if (d->acceptPublicUpdates)
	  {
	    wxString arearp_cs = key;
	    wxString zonerp_cs = value;

	    arearp_cs.Replace(wxT("_"), wxT(" "));
	    zonerp_cs.Replace(wxT("_"), wxT(" "));
	    zonerp_cs.SetChar(7, wxT('G'));

	    IRCMessage * m2 = new IRCMessage(wxT("IDRT_REPEATER"));
	    m2->addParam(arearp_cs);
	    m2->addParam(zonerp_cs);
	    m2->addParam(getIPAddress(value));
	    d->replyQ.putMessage(m2);
	  }
	}
	else if ((tableID == 0) && d->acceptPublicUpdates)
	{
	  wxMutexLocker lock(d->rptrMapMutex);

	  wxString userCallsign = key;
	  wxString arearp_cs = value;
	  wxString zonerp_cs;
	  wxString ip_addr;

	  userCallsign.Replace(wxT("_"), wxT(" "));
	  arearp_cs.Replace(wxT("_"), wxT(" "));

	  IRCDDBAppRptrMap::iterator i = d->rptrMap.find( value );

	  if (i != d->rptrMap.end())
	  {
	    IRCDDBAppRptrObject o = d->rptrMap[value];
	    zonerp_cs = o.zonerp_cs;
	    zonerp_cs.Replace(wxT("_"), wxT(" "));
	    zonerp_cs.SetChar(7, wxT('G'));

	    ip_addr = getIPAddress(o.zonerp_cs);
	  }

	  IRCMessage * m2 = new IRCMessage(wxT("IDRT_USER"));
	  m2->addParam(userCallsign);
	  m2->addParam(arearp_cs);
	  m2->addParam(zonerp_cs);
	  m2->addParam(ip_addr);
	  d->replyQ.putMessage(m2);

	}


      }
    }

}


static wxString getTableIDString( int tableID, bool spaceBeforeNumber )
{
  if (tableID == 0)
  {
    return wxT("");
  }
  else if ((tableID > 0) && (tableID < numberOfTables))
  {
    if (spaceBeforeNumber)
    {
      return wxString::Format(wxT(" %d"),tableID);
    }
    else
    {	
      return wxString::Format(wxT("%d "),tableID);
    }
  }
  else
  {
    return wxT(" TABLE_ID_OUT_OF_RANGE ");
  }
}


void IRCDDBApp::msgQuery (IRCMessage * m)
{

  if (m->getPrefixNick().StartsWith(wxT("s-")) && (m->numParams >= 2))  // server msg
  {
    wxString msg = m->params[1];
    wxStringTokenizer tkz(msg);

    if (!tkz.HasMoreTokens()) 
    {
      return;  // no text in message
    }

    wxString cmd = tkz.GetNextToken();

    if (cmd.IsSameAs(wxT("UPDATE")))
    {
      wxString restOfLine = tkz.GetString();
      doUpdate(restOfLine);
    }
    else if (cmd.IsSameAs(wxT("LIST_END")))
    {
      if (d->state == 5) // if in sendlist processing state
      {
	d->state = 3;  // get next table
      }
    }
    else if (cmd.IsSameAs(wxT("LIST_MORE")))
    {
      if (d->state == 5) // if in sendlist processing state
      {
	d->state = 4;  // send next SENDLIST
      }
    }
    else if (cmd.IsSameAs(wxT("NOT_FOUND")))
    {
    }
  }
}
	
	
void IRCDDBApp::setSendQ( IRCMessageQueue * s )
{
  d->sendQ = s;
}
	
IRCMessageQueue * IRCDDBApp::getSendQ()
{
  return d->sendQ;
}
	
	
static wxString getLastEntryTime(int tableID)
{

  if (tableID == 1)
  {
    wxString max = IRCDDBAppRptrObject::maxTime.Format( wxT("%Y-%m-%d %H:%M:%S") );
    return max;
  }

  return wxT("DBERROR");
}


static bool needsDatabaseUpdate( int tableID )
{
  return (tableID == 1);
}
	
	
wxThread::ExitCode IRCDDBApp::Entry()
{

  int sendlistTableID = 0;
		
  while (!d->terminateThread)
  {

    if (d->timer > 0)
    {
      d->timer --;
    }
			
    switch(d->state)
    {
    case 0:  // wait for network to start
					
      if (getSendQ() != NULL)
      {
	      d->state = 1;
      }
      break;
				
    case 1:
      // connect to db
      d->state = 2;
      d->timer = 200;
      break;
			
    case 2:   // choose server
      wxLogVerbose(wxT("IRCDDBApp: state=2 choose new 's-'-user"));
      if (getSendQ() == NULL)
      {
	d->state = 10;
      }
      else
      {	
	if (findServerUser())
	{
	  sendlistTableID = numberOfTables;

	  d->state = 3; // next: send "SENDLIST"
	}
	else if (d->timer == 0)
	{
	  d->state = 10;
	  IRCMessage * m = new IRCMessage(wxT("QUIT"));

	  m->addParam(wxT("no op user with 's-' found."));
						
	  IRCMessageQueue * q = getSendQ();
	  if (q != NULL)
	  {
	    q->putMessage(m);
	  }
	}
      }
      break;
				
    case 3:
      if (getSendQ() == NULL)
      {
	d->state = 10; // disconnect DB
      }
      else
      {
	sendlistTableID --;
	if (sendlistTableID < 0)
	{
	  d->state = 6; // end of sendlist
	}
	else
	{
	  wxLogVerbose(wxT("IRCDDBApp: state=3 tableID=%d"), sendlistTableID);
	  d->state = 4; // send "SENDLIST"
	  d->timer = 900; // 15 minutes max for update
	}
      }
      break;

    case 4:
      if (getSendQ() == NULL)
      {
	d->state = 10; // disconnect DB
      }
      else
      {
	if (needsDatabaseUpdate(sendlistTableID))
	{
	  IRCMessage * m = new IRCMessage(d->currentServer, 
			wxT("SENDLIST") + getTableIDString(sendlistTableID, true) 
			 + wxT(" ") + getLastEntryTime(sendlistTableID) );

	  IRCMessageQueue * q = getSendQ();
	  if (q != NULL)
	  {
	    q->putMessage(m);
	  }

	  d->state = 5; // wait for answers
	}
	else
	{
	  d->state = 3; // don't send SENDLIST for this table, go to next table
	}
      }
      break;

    case 5: // sendlist processing
      if (getSendQ() == NULL)
      {
	d->state = 10; // disconnect DB
      }
      else if (d->timer == 0)
      {
	d->state = 10; // disconnect DB
	  IRCMessage * m = new IRCMessage(wxT("QUIT"));

	  m->addParam(wxT("timeout SENDLIST"));
						
	  IRCMessageQueue * q = getSendQ();
	  if (q != NULL)
	  {
	    q->putMessage(m);
	  }
					
      }
      break;

    case 6:
      if (getSendQ() == NULL)
      {
	d->state = 10; // disconnect DB
      }
      else
      {
	wxLogVerbose(wxT( "IRCDDBApp: state=6 enablePublcUpdates"));
	enablePublicUpdates();
	d->state = 7;
      }
      break;
				
			
    case 7: // standby state after initialization
      if (getSendQ() == NULL)
      {
	d->state = 10; // disconnect DB
      }
      break;
				
    case 10:
      // disconnect db
      d->state = 0;
      d->timer = 0;
      d->acceptPublicUpdates = false;
      break;
			
    }

    wxThread::Sleep(1000);

			
			

  } // while

  return 0;
} // Entry()
	



