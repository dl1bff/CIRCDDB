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


CPPFLAGS=-Wall ` wx-config --cppflags base net`

LDFLAGS=` wx-config --libs base net` 


L=libircddb.a



$L: $L(IRCDDB.o IRCClient.o IRCReceiver.o)

test_lib: test_lib.o $L

$L(IRCDDB.o): IRCDDB.h $L(IRCClient.o)

$L(IRCClient.o): IRCClient.h $L(IRCReceiver.o)

$L(IRCReceiver.o): IRCReceiver.h






