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

#include "socketlistener.h"
#include "tlsmanager.h"



bool socketlistener::process_listening_socket()
{

   if ( m_poll_socket_descriptors[LISTEN_SOCKET_INDEX].revents & POLLIN )
   {  cout << "Connection request has come on listening port" << endl;
      m_transport_ptr->accept();

      tlsmanager *ptr = new tlsmanager();
      ptr->init ( m_transport_ptr );
      ptr->tls_accept();

   }
   else if ( m_poll_socket_descriptors[LISTEN_SOCKET_INDEX].revents & POLLNVAL )
   {

      cout << "Listening socket was closed." << endl;
      exit ( 0 );
   }
   else if ( m_poll_socket_descriptors[LISTEN_SOCKET_INDEX].revents & POLLERR )
   {  cout << "POLLERR Error on Listening socket." << endl;
      exit ( 0 );
   }
   else if ( m_poll_socket_descriptors[LISTEN_SOCKET_INDEX].revents & POLLHUP )
   {  cout << "POLLHUP received. Stopped listening on socket."<< endl;
      exit ( 0 );
   }
   else
   {  cout << "Listening socket, No events occurred : revents = "
           << m_poll_socket_descriptors[LISTEN_SOCKET_INDEX].revents << endl;
   }

}

void socketlistener::start_thread()
{

   cout << "started listener thread" << endl;
   m_no_of_poll_entries = 1;
   m_poll_socket_descriptors[LISTEN_SOCKET_INDEX].fd = m_transport_ptr->m_passive_socket;
   m_poll_socket_descriptors[LISTEN_SOCKET_INDEX].events = POLLIN;
   while ( 1 )
   {

      int poll_return_value = poll ( m_poll_socket_descriptors, m_no_of_poll_entries, 2000 );

      if ( -1 == poll_return_value )  //poll() error encountered
      {  int err = errno;
         char error_string_buffer[ERR_STR_LENGTH] = { 0 };
         const char* errorstring = TransportInterface::get_error_string ( err, error_string_buffer );

         if ( EINTR == err || EAGAIN == err )
         {  cout << "EINTR / EAGAIN received. Continuing to process."<< endl;
            continue;
         }
         else
         {  cout << "The nfds argument is greater than {OPEN_MAX}, or one of the fd members refers to a STREAM or"
                 " multiplexer that is linked (directly or indirectly) downstream from a multiplexer. ERROR : "
                 << errorstring << endl;
            break;
         }
      }
      else if ( 0 == poll_return_value )  //poll() timed out
      {  /*
         cout << "Timeout occurred! No data on listening and pending sockets after : "
                    << 2 << " milliseconds" << endl;
               */

      }
      else
      {  // Check for events on listening socket.
         process_listening_socket();
      }

      //cout << "Finished accepting new connections & processing pending connections. Going back to poll()" << endl;

   }

}

