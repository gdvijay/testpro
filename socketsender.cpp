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

#include "socketsender.h"
#include "socketreceiver.h"
#include <unistd.h>
#include <pthread.h>
#include "tlsmanager.h"


void* socket_receiver_thread ( void* arg )
{  TransportInterface* transportinstance = ( TransportInterface* ) arg;
   SocketReceiver *ptr = new SocketReceiver ( transportinstance );
   transportinstance-> m_reciever = ptr;
   ptr->start_thread();
   delete ptr;

}

void SocketSender::start_thread()
{  cout << "Spawned sender thread" << endl;


   if ( m_transport_ptr->m_type == CLIENT )
   {
      cout << "Initiating connect ......" << endl;
      bool ret = m_transport_ptr->connect();

      if ( !ret )
      {  cout << "connect failed" << endl;
         m_transport_ptr->close();
         return;
      }

      cout << "connect suceeded" << endl;
   }

   pthread_t receiver_tid;
   int return_code = pthread_create ( &receiver_tid, NULL, socket_receiver_thread, m_transport_ptr );


   sleep ( 10 );

   /*

      char* ptr = "client_message";

      if ( m_transport_ptr->m_type == CLIENT )
      {  m_transport_ptr->send_message ( ptr, 14 );
      }
      else
      {  ptr = "server_message";
         m_transport_ptr->send_message ( ptr, 13 );
      }

   */
   if ( m_transport_ptr->m_type == CLIENT )
   {  tlsmanager *ptr = new tlsmanager();
      ptr->init ( m_transport_ptr );
      ptr->tls_connect();
   }

}
