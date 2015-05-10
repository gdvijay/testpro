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

#include <sys/epoll.h>
#include "socketreceiver.h"

const unsigned int MAX_POLL_ENTRIES = 100;  

void SocketReceiver::start_thread()
{
   cout << "spawned receiver thread" << endl;
   m_epoll_fd = epoll_create1 ( 0 );
   if ( m_epoll_fd == -1 )
   {  char error_string_buffer[ERR_STR_LENGTH] = { 0 };
      const char* errorstring = m_transport_ptr->get_error_string ( errno, error_string_buffer );

      cout << "epoll_create1 failed with error : " << errno << "(" << errorstring << ")" << endl;
      exit ( 0 );
   }

   struct epoll_event event;
   memset ( &event, 0, sizeof ( event ) );
   event.events = EPOLLIN;
      
   epoll_ctl ( m_epoll_fd, EPOLL_CTL_ADD, m_transport_ptr->m_socket, &event );

   
   cout << "Adding socket: " << m_transport_ptr->m_socket << " to the epoll set"<< endl;
   
   struct epoll_event epoll_returned_events[128] = {0};

   //connect
   while ( 0 )
   {

      int events_count = epoll_wait ( m_epoll_fd, epoll_returned_events, 128, 1000 );
      if ( events_count > 0 ) //poll() returned an event
      {  cout << "Socket is ready to read or some event occurred" << endl;

         for ( int i = 0; i < events_count; i++ )
         {

            if ( epoll_returned_events[i].events & EPOLLIN )
            {  char message [30];
               bool received_bytes = m_transport_ptr->receive_message ( message, 13 );

               if (received_bytes)
               {  cout << "received bytes ... " << message << endl;
               }
               else
               {  cout << "socket read failed" << endl;
               }

            }

         }
      }
      else if ( -1 == events_count )      //poll() encountered an error
      {  char error_string_buffer[ERR_STR_LENGTH] = { 0 };
         const char* errorstring = m_transport_ptr->get_error_string ( errno, error_string_buffer );

         cout << "Error while epolling on socket : " << events_count << "(" << errorstring << ")" << endl;
      }
      else if ( 0 == events_count )
      {
	//cout << "epoll timed out" << endl;
      }


   }

}
