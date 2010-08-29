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

  private:

  static unsigned int counter;
};



WX_DECLARE_STRING_HASH_MAP( IRCDDBAppUserObject, IRCDDBAppUserMap );


struct IRCDDBAppPrivate
{
  IRCMessageQueue * sendQ;

  IRCDDBAppUserMap user;

  wxString currentServer;
  wxString myNick;

  int state;
  int timer;

  wxString updateChannel;
  wxString channelTopic;

  bool acceptPublicUpdates;

};

	
IRCDDBApp::IRCDDBApp( const wxString& u_chan ) : d(new IRCDDBAppPrivate)
{

  d->sendQ = NULL;
  d->acceptPublicUpdates = false;

  userListReset();
		
  d->state = 0;
  d->timer = 0;
  d->myNick = wxT("none");
		
  d->updateChannel = u_chan;
		
}

IRCDDBApp::~IRCDDBApp()
{
  if (d->sendQ != NULL)
  {
    delete d->sendQ;
  }
  delete d;
}

	
void IRCDDBApp::userJoin (const wxString& nick, const wxString& name, const wxString& host)
{
  IRCDDBAppUserObject u( nick, name, host );
		
  d->user[nick] = u;
}

void IRCDDBApp::userLeave (const wxString& nick)
{
  d->user.erase(nick);

  if (d->currentServer.Len() > 0)
  {
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

	public void userListReset()
	{
		user = Collections.synchronizedMap( new HashMap<String,UserObject>() );

		if (extApp != null)
		{
			extApp.userListReset();
		}
	}

	public void setCurrentNick(String nick)
	{
		myNick = nick;

		if (extApp != null)
		{
			extApp.setCurrentNick(nick);
		}
	}

	public void setTopic(String topic)
	{
		channelTopic = topic;

		if (extApp != null)
		{
			extApp.setTopic(topic);
		}
	}
	
	
	boolean findServerUser()
	{
		boolean found = false;
		
		Collection<UserObject> v = user.values();
		
		Iterator<UserObject> i = v.iterator();
		
		while (i.hasNext())
		{
			UserObject u = i.next();
			
			// System.out.println("LIST: " + u.nick + " " + u.op);
			
			if (u.nick.startsWith("s-") && u.op && !myNick.equals(u.nick))
			{
				currentServer = u.nick;
				found = true;
				if (extApp != null) 
				{
					extApp.setCurrentServerNick(currentServer);
				}
				break;
			}
		}
		
		return found;
	}
	
	
	public void userChanOp (String nick, boolean op)
	{
		UserObject u = user.get( nick );
		
		if (u != null)
		{
			// System.out.println("APP: op " + nick + " " + op);
			if ((extApp != null) && (u.op != op))
			{
				extApp.userChanOp(nick, op);
			}

			u.op = op;
		}
	}
	

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
	 
	void enablePublicUpdates()
	{
	  acceptPublicUpdates = true;

	  for (int i = (numberOfTables-1); i >= 0; i--)
	  {
		while (publicUpdates[i].messageAvailable())
		{
			IRCMessage m = publicUpdates[i].getMessage();

			String msg = m.params[1];

                        Scanner s = new Scanner(msg);

			processUpdate(i, s, null);
		}
	  }
	}
	
	public void msgChannel (IRCMessage m)
	{
		// System.out.println("APP: chan");

		
		if (m.getPrefixNick().startsWith("s-"))  // server msg
		{
			int tableID = 0;

			String msg = m.params[1];
			
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
		}
	}

	private String getTableIDString( int tableID, boolean spaceBeforeNumber )
	{
	  if (tableID == 0)
	  {
	    return "";
	  }
	  else if ((tableID > 0) && (tableID < numberOfTables))
	  {
	    if (spaceBeforeNumber)
	    {
	      return " " + tableID;
	    }
	    else
	    {	
	      return tableID + " ";
	    }
	  }
	  else
	  {
	    return " TABLE_ID_OUT_OF_RANGE ";
	  }
	}
	
	public void msgQuery (IRCMessage m)
	{
	
		String msg = m.params[1];
		
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
	}
	
	
	public synchronized void setSendQ( IRCMessageQueue s )
	{
		// System.out.println("APP: setQ " + s);
		
		sendQ = s;
		
		if (extApp != null)
		{
			extApp.setSendQ(s);
		}
	}
	
	public synchronized IRCMessageQueue getSendQ ()
	{
		return sendQ;
	}
	
	
	String getLastEntryTime(int tableID)
	{

		if (extApp != null)
		{

			Date d = extApp.getLastEntryDate(tableID);

			if (d != null)
			{
				return parseDateFormat.format( d );
			}
		}
		
		return "DBERROR";
	}
	
	
	public void run()
	{

		int dumpUserDBTimer = 60;
		int sendlistTableID = 0;
		
		while (true)
		{
			
			if (timer > 0)
			{
				timer --;
			}
			
			// System.out.println("state " + state);
			
			
			switch(state)
			{
			case 0:  // wait for network to start
					
				if (getSendQ() != null)
				{
					state = 1;
				}
				break;
				
			case 1:
			  // connect to db
			  state = 2;
			  timer = 200;
			  break;
			
			case 2:   // choose server
			  Dbg.println(Dbg.DBG1, "IRCDDBApp: state=2 choose new 's-'-user");
				if (getSendQ() == null)
				{
					state = 10;
				}
				else
				{	
					if (findServerUser())
					{
						sendlistTableID = numberOfTables;

						if (extApp != null)
						{
						  state = 3; // next: send "SENDLIST"
						}	
						else
						{
						  state = 6; // next: enablePublicUpdates
						}
					}
					else if (timer == 0)
					{
						state = 10;
						
						IRCMessage m = new IRCMessage();
						m.command = "QUIT";
						m.numParams = 1;
						m.params[0] = "no op user with 's-' found.";
						
						IRCMessageQueue q = getSendQ();
						if (q != null)
						{
							q.putMessage(m);
						}
					}
				}
				break;
				
			case 3:
				if (getSendQ() == null)
				{
				  state = 10; // disconnect DB
				}
				else
				{
				  sendlistTableID --;
				  if (sendlistTableID < 0)
				  {
				    state = 6; // end of sendlist
				  }
				  else
				  {
				    Dbg.println(Dbg.DBG1, "IRCDDBApp: state=3 tableID="+sendlistTableID);
				    state = 4; // send "SENDLIST"
				    timer = 900; // 15 minutes max for update
				  }
				}
				break;

			case 4:
				if (getSendQ() == null)
				{
				  state = 10; // disconnect DB
				}
				else
				{
				    if (extApp.needsDatabaseUpdate(sendlistTableID))
				    {
				      IRCMessage m = new IRCMessage();
				      m.command = "PRIVMSG";
				      m.numParams = 2;
				      m.params[0] = currentServer;
				      m.params[1] = "SENDLIST" + getTableIDString(sendlistTableID, true) 
				       + " " + getLastEntryTime(sendlistTableID);
				      
				      IRCMessageQueue q = getSendQ();
				      if (q != null)
				      {
					      q.putMessage(m);
				      }

				      state = 5; // wait for answers
				    }
				    else
				    {
				      state = 3; // don't send SENDLIST for this table, go to next table
				    }
				}
				break;

			case 5: // sendlist processing
				if (getSendQ() == null)
				{
					state = 10; // disconnect DB
				}
				else if (timer == 0)
				{
					state = 10;
					
					IRCMessage m = new IRCMessage();
					m.command = "QUIT";
					m.numParams = 1;
					m.params[0] = "timeout SENDLIST";
					
					IRCMessageQueue q = getSendQ();
					if (q != null)
					{
					q.putMessage(m);
					}
				}
				break;

			case 6:
				if (getSendQ() == null)
				{
					state = 10; // disconnect DB
				}
				else
				{
				  UserObject me = user.get(myNick);
				  UserObject other = user.get(currentServer);

				  if ((me != null) && (currentServer != null) && !me.op && other.op
					  && other.nick.startsWith("s-") && me.nick.startsWith("s-") )
				  {
					  IRCMessage m2 = new IRCMessage();
					  m2.command = "PRIVMSG";
					  m2.numParams = 2;
					  m2.params[0] = other.nick;
					  m2.params[1] = "OP_BEG";
					  
					  IRCMessageQueue q = getSendQ();
					  if (q != null)
					  {
						  q.putMessage(m2);
					  }
				  }

				  Dbg.println(Dbg.DBG1, "IRCDDBApp: state=6 enablePublcUpdates");
				  enablePublicUpdates();
				  state = 7;
				}
				break;
				
			
			case 7: // standby state after initialization
				if (getSendQ() == null)
				{
					state = 10; // disconnect DB
				}
				break;
				
			case 10:
				// disconnect db
				state = 0;
				timer = 0;
				acceptPublicUpdates = false;
				break;
			
			case 11:
				if (timer == 0)
				{
					System.exit(0);
				}
				break;
			}

			
			
			try
			{
				Thread.sleep(1000);
			}
			catch ( InterruptedException e )
			{
				Dbg.println(Dbg.WARN, "sleep interrupted " + e);
			}


			if (!dumpUserDBFileName.equals("none"))
			{
			if (dumpUserDBTimer <= 0)
			{
				dumpUserDBTimer = 300;

				try
				{
					PrintWriter p = new PrintWriter(
						new FileOutputStream(dumpUserDBFileName));

					Collection<UserObject> c = user.values();

					for (UserObject o : c)
					{
						p.println(o.nick + " " + o.name +
							" " + o.host + " " + o.op);
					}

					p.close();


				}
				catch (IOException e)
				{
					Dbg.println(Dbg.WARN, "dumpDb failed " + e);
				}

			}
			else
			{
				dumpUserDBTimer --;
			}
			} // if (!dumpUserDBFileName.equals("none"))
		}
	}
	





