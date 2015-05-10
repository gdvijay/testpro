/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef SOCKETLISTENER_H
#define SOCKETLISTENER_H

#include "transportinterface.h"

const unsigned int MAX_POLL_ENTRIES = 100;  
const int LISTEN_SOCKET_INDEX = 0;



class TransportInterface;
class socketlistener
{

  TransportInterface *m_transport_ptr;

public:

  bool process_listening_socket();
  
  struct pollfd m_poll_socket_descriptors[MAX_POLL_ENTRIES];  ///< Array of pollfd structure for polling
  int m_no_of_poll_entries;   ///< Number of fds that are currently being polle
    
  void start_thread ();

    socketlistener (TransportInterface * ptr)
  {
    m_transport_ptr = ptr;

  }

};



#endif // SOCKETLISTENER_H
