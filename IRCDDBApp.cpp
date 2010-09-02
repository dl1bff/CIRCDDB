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


struct IRCDDBAppPrivate
{
  IRCMessageQueue * sendQ;

  IRCDDBAppUserMap user;
  wxMutex userMapMutex;

  wxString currentServer;
  wxString myNick;

  int state;
  int timer;

  wxString updateChannel;
  wxString channelTopic;

  bool acceptPublicUpdates;

  bool terminateThread;
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

  wxLogVerbose(wxT("user %d"), u.usn );
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
	
/*
	IRCDDBExtApp.UpdateResult processUpdate ( int tableID, Scanner s, String ircUser )
	{
		if (s.hasNext(datePattern))
		{
			String d = s.next(datePattern);
			
			if (s.hasNext(timePattern))
			{
				String t = s.next(timePattern);
				
				
				Date dbDate = null;

				try
				{
					dbDate = parseDateFormat.parse(d + " " + t);
				}
				catch (java.text.ParseException e)
				{
					dbDate = null;
				}
					
				if ((dbDate != null) && s.hasNext(keyPattern[tableID]))
				{
					String key = s.next(keyPattern[tableID]);
					
					
					if (s.hasNext(valuePattern[tableID]))
					{
						String value = s.next(valuePattern[tableID]);

						if (extApp != null)
						{
							return extApp.dbUpdate( tableID, dbDate, key, value, ircUser );
						}
					}
				}
			}
		}
		
		return null;
	}
	 
 */

void IRCDDBApp::enablePublicUpdates()
{
  d->acceptPublicUpdates = true;
}

void IRCDDBApp::msgChannel (IRCMessage * m)
{
		
  if (m->getPrefixNick().StartsWith(wxT("s-")) && (m->numParams >= 2))  // server msg
  {
    // int tableID = 0;

    wxString msg = m->params[1];
			
			/*
			Scanner s = new Scanner(msg);

			if (s.hasNext(tablePattern))
			{
			  tableID = s.nextInt();
			  if ((tableID < 0) || (tableID >= numberOfTables))
			  {
			    Dbg.println(Dbg.INFO, "invalid table ID " + tableID);
			    return;
			  }
			}
			
			if (s.hasNext(datePattern))
			{
				if (acceptPublicUpdates)
				{
					processUpdate(tableID, s, null); 
				}
				else
				{
					publicUpdates[tableID].putMessage(m);
				}
			}
			else
			{
				if (extApp != null)
				{
					extApp.msgChannel( m );
				}
			}
			*/
  }
}

static int numberOfTables = 2;

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
  wxString msg = m->params[1];
		
      /*
		Scanner s = new Scanner(msg);
		
		String command;
		
		if (s.hasNext())
		{
			command = s.next();
		}
		else
		{
			return; // no command
		}

		int tableID = 0;

		if (s.hasNext(tablePattern))
		{
		  tableID = s.nextInt();
		  if ((tableID < 0) || (tableID >= numberOfTables))
		  {
		    Dbg.println(Dbg.WARN, "invalid table ID " + tableID);
		    return;
		  }
		}
		
		if (command.equals("UPDATE"))
		{	
			UserObject other = user.get(m.getPrefixNick()); // nick of other user
			
			if (s.hasNext(datePattern)  &&
				(other != null))
			{
				IRCDDBExtApp.UpdateResult result = processUpdate(tableID, s, other.nick);
				
				if (result != null)
				{
					boolean sendUpdate = false;
					
					if (result.keyWasNew)
					{
						sendUpdate = true;
					}
					else
					{

						if (result.newObj.value.equals(result.oldObj.value)) // value is the same
						{
							long newMillis = result.newObj.modTime.getTime();
							long oldMillis = result.oldObj.modTime.getTime();
							
							if (newMillis > (oldMillis + 2400000))  // update max. every 40 min
							{
								sendUpdate = true;
							}
						}
						else
						{
							sendUpdate = true;  // value has changed, send update via channel
						}
				
					}

					UserObject me = user.get(myNick); 
					
					if ((me != null) && me.op && sendUpdate)  // send only if i am operator
					{
				
						IRCMessage m2 = new IRCMessage();
						m2.command = "PRIVMSG";
						m2.numParams = 2;
						m2.params[0] = updateChannel;
						m2.params[1] = getTableIDString(tableID, false) + 
						      parseDateFormat.format(result.newObj.modTime) + " " +
							result.newObj.key + " " + result.newObj.value + "  (from: " + m.getPrefixNick() + ")";
						
						IRCMessageQueue q = getSendQ();
						if (q != null)
						{
							q.putMessage(m2);
						}
					}

				     if (debugChannel != null)
				     {
				       IRCMessage m2 = new IRCMessage();
				       m2.command = "PRIVMSG";
				       m2.numParams = 2;
				       m2.params[0] = debugChannel;
				       m2.params[1] = m.getPrefixNick() + ": UPDATE OK: " + msg;

				       IRCMessageQueue q = getSendQ();
				       if (q != null)
				       {
					  q.putMessage(m2);
				       }
				     }
				}
				else
				{
				   if (debugChannel != null)
				   {
				     IRCMessage m2 = new IRCMessage();
				     m2.command = "PRIVMSG";
				     m2.numParams = 2;
				     m2.params[0] = debugChannel;
				     m2.params[1] = m.getPrefixNick() + ": UPDATE ERROR: " + msg;

				     IRCMessageQueue q = getSendQ();
				     if (q != null)
				     {
					q.putMessage(m2);
				     }
				   }

				}
			}
			
			
	
		}
		else if (command.equals("SENDLIST"))
		{
		
			String answer = "LIST_END";
			
			if (s.hasNext(datePattern))
			{
				String d = s.next(datePattern);
				
				if (s.hasNext(timePattern))
				{
					String t = s.next(timePattern);
					
					
					Date dbDate = null;

					try
					{
						dbDate = parseDateFormat.parse(d + " " + t);
					}
					catch (java.text.ParseException e)
					{
						dbDate = null;
					}
						
					if ((dbDate != null) && (extApp != null))
					{
						final int NUM_ENTRIES = 30;

						LinkedList<IRCDDBExtApp.DatabaseObject> l = 
							extApp.getDatabaseObjects( tableID, dbDate, NUM_ENTRIES );

						int count = 0;
				
						if (l != null)
						{
						  for (IRCDDBExtApp.DatabaseObject o : l)
						  {
						    IRCMessage m3 = new IRCMessage(
							  m.getPrefixNick(),
							  "UPDATE" + getTableIDString(tableID, true) +
							    " " + parseDateFormat.format(o.modTime) + " "
							     + o.key + " " + o.value	);
					  
						    IRCMessageQueue q = getSendQ();
						    if (q != null)
						    {
						      q.putMessage(m3);
						    }
								  
						    count ++;
						  }
						}

						if (count > NUM_ENTRIES)
						{
							answer = "LIST_MORE";
						}
					}
				}
			}
			
			IRCMessage m2 = new IRCMessage();
			m2.command = "PRIVMSG";
			m2.numParams = 2;
			m2.params[0] = m.getPrefixNick();
			m2.params[1] = answer;
			
			IRCMessageQueue q = getSendQ();
			if (q != null)
			{
				q.putMessage(m2);
			}
		}
		else if (command.equals("LIST_END"))
		{
		    if (state == 5) // if in sendlist processing state
		    {
		      state = 3;  // get next table
		    }
		}
		else if (command.equals("LIST_MORE"))
		{
		    if (state == 5) // if in sendlist processing state
		    {
		      state = 4;  // send next SENDLIST
		    }
		}
		else if (command.equals("OP_BEG"))
		{
			UserObject me = user.get(myNick); 
			UserObject other = user.get(m.getPrefixNick()); // nick of other user

			if ((me != null) && (other != null) && me.op && !other.op
				&& other.nick.startsWith("s-") && me.nick.startsWith("s-") )
			{
				IRCMessage m2 = new IRCMessage();
				m2.command = "MODE";
				m2.numParams = 3;
				m2.params[0] = updateChannel;
				m2.params[1] = "+o";
				m2.params[2] = other.nick;
				
				IRCMessageQueue q = getSendQ();
				if (q != null)
				{
					q.putMessage(m2);
				}
			}
		}
		else if (command.equals("QUIT_NOW"))
		{
			UserObject other = user.get(m.getPrefixNick()); // nick of other user

			if ((other != null) && other.op
                                && other.nick.startsWith("u-"))
                        {

				IRCMessage m2 = new IRCMessage();
				m2.command = "QUIT";
				m2.numParams = 1;
				m2.params[0] = "QUIT_NOW sent by "+other.nick;

				IRCMessageQueue q = getSendQ();
				if (q != null)
				{
					q.putMessage(m2);
				}

				timer = 3;
				state = 11;  // exit
			}
		}
		else if (command.equals("SHOW_PROPERTIES"))
		{
		  UserObject other = user.get(m.getPrefixNick()); // nick of other user

		  if ((other != null) && other.op
			  && other.nick.startsWith("u-"))
		  {
		    int num = properties.size();

		    for (Enumeration e = properties.keys(); e.hasMoreElements(); )
		    {
		      String k = (String) e.nextElement();
		      String v = properties.getProperty(k);

		      if (k.equals("irc_password"))
		      {
			v = "*****";
		      }
		      else
		      {
			v = "(" + v + ")";
		      }

			IRCMessage m2 = new IRCMessage();
			m2.command = "PRIVMSG";
			m2.numParams = 2;
			m2.params[0] = m.getPrefixNick();
			m2.params[1] = num + ": (" + k + ") " + v;
			
			IRCMessageQueue q = getSendQ();
			if (q != null)
			{
				q.putMessage(m2);
			}
		      num --;
		    }
		  }
		}
		else
		{
			if (extApp != null)
			{
				extApp.msgQuery(m);
			}
		}

		*/
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
    return wxT("2000-01-01 12:00:00");
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
} // Entry()
	



