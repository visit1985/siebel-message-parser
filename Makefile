#
# Makefile
# Copyright (C) 2014 Michael Goehler
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
TARGET  = smp
DESTDIR =
PREFIX  = /usr/local
LDFLAGS ?= -s

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(TARGET)

clean:
	$(RM) $(OBJECTS) $(TARGET)

install:
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 mdp $(DESTDIR)$(PREFIX)/bin/$(TARGET)

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/$(TARGET)

.PHONY: all clean install src uninstall
