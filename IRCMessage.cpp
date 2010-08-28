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

#include "IRCMessage.h"



IRCMessage::IRCMessage ()
{
  numParams = 0;
  prefixParsed = false;
}

IRCMessage::IRCMessage ( const wxString& toNick, const wxString& msg )
{
  command = wxT("PRIVMSG");
  numParams = 2;
  params.Add( toNick );
  params.Add( msg );
  prefixParsed = false;
}

IRCMessage::~IRCMessage()
{
}

	
void IRCMessage::parsePrefix()
{
  unsigned int i;

  for (i=0; i < 3; i++)
  {
    prefixComponents.Add(wxT(""));
  }

  int state = 0;
  
  for (i=0; i < prefix.Len(); i++)
  {
    wxChar c = prefix.GetChar(i);
			
    switch (c)
    {
    case wxT('!'): 
	    state = 1; // next is name
	    break;
	    
    case wxT('@'):
	    state = 2; // next is host
	    break;
	    
    default:
	    prefixComponents[state].Append(c);
	    break;
    }
  }

  prefixParsed = true;
}

wxString& IRCMessage::getPrefixNick()
{
  if (!prefixParsed)
  {
    parsePrefix();
  }
  
  return prefixComponents[0];
}

wxString& IRCMessage::getPrefixName()
{
  if (!prefixParsed)
  {
    parsePrefix();
  }
  
  return prefixComponents[1];
}

wxString& IRCMessage::getPrefixHost()
{
  if (!prefixParsed)
  {
    parsePrefix();
  }
  
  return prefixComponents[2];
}

void IRCMessage::composeMessage ( wxString& output )
{

}

/*
	void writeMessage ( java.io.OutputStream os, boolean debug ) throws java.io.IOException
	{

		if (debug)
		{
			System.out.print("T [" + prefix + "]" );

                        System.out.print(" [" + command +"]" );

                        for (int i=0; i < numParams; i++)
                        {
                        	System.out.print(" [" + params[i] + "]" );
                        }
                        System.out.println();
		}

		java.io.PrintWriter p = new java.io.PrintWriter(os);

		if (prefix.length() > 0)
		{
			p.format(":%s ", prefix);
		}

		p.print(command);

		for (int i=0; i < numParams; i++)
		{
			if (i == (numParams - 1))
			{
				p.format(" :%s", params[i]);
			}
			else
			{
				p.format(" %s", params[i]);
			}
		}

		p.print("\r\n");
		p.flush();

	}
*/
