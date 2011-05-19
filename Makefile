# 
# CIRCDDB - ircDDB client library in C++
# 
# Copyright (C) 2010   Michael Dirska, DL1BFF (dl1bff@mdx.de)
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#


#CPPFLAGS=-g -DDEBUG_IRC -Wall ` wx-config --cppflags base `
# CPPFLAGS=-g -Wall ` wx-config --cppflags base `
# CPPFLAGS=-O2  -DDEBUG_IRC -Wall ` wx-config --cppflags base `
CPPFLAGS=-O2 -Wall ` wx-config --cppflags base `

# LDFLAGS=-g ` wx-config --libs base ` 
LDFLAGS=` wx-config --libs base ` 


L=libircddb.a



$L: $L(IRCDDB.o IRCClient.o IRCReceiver.o)

test_lib: test_lib.o $L

$L(IRCDDB.o): IRCDDB.h $L(IRCClient.o) $L(IRCDDBApp.o)

$L(IRCClient.o): IRCClient.h $L(IRCReceiver.o) $L(IRCProtocol.o) $L(IRCutils.o)

$L(IRCReceiver.o): IRCReceiver.h $L(IRCMessageQueue.o)

$L(IRCMessageQueue.o): IRCMessageQueue.h $L(IRCMessage.o)

$L(IRCProtocol.o): IRCProtocol.h IRCApplication.h $L(IRCMessageQueue.o)

$L(IRCMessage.o): IRCMessage.h

$L(IRCDDBApp.o): IRCDDBApp.h IRCApplication.h $L(IRCutils.o)

IRCApplication.h: IRCMessageQueue.h


clean:
	rm -f $L test_lib test_lib.o

