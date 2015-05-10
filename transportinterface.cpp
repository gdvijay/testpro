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

#include "transportinterface.h"


#include "socketlistener.h"
#include "socketsender.h"

using namespace std;

void* socket_sender_thread ( void* arg )
{  TransportInterface* transportinstance = ( TransportInterface* ) arg;
   SocketSender *ptr = new SocketSender ( transportinstance );
   transportinstance->m_sender = ptr;
   ptr->start_thread();
   delete ptr;
}

bool TransportInterface::send_message ( const char* out_message, const ssize_t& length )
{

   ssize_t n_left = length;
   int err = 0;
   ssize_t ret = 0;
   // The send will come out only on sending the expected number of bytes of data or
   // on timeout of the select system call.


   cout << "Sending out message to : " << out_message << " Size:=" << length << endl;


   struct pollfd poll_socket_descriptor[1];
   poll_socket_descriptor[0].fd = m_socket;
   poll_socket_descriptor[0].events = POLLOUT;
   while ( n_left > 0 )
   {  ret = send_to_socket ( out_message, n_left, err );
      if ( ret <= 0 )
      {  if ( EINTR == err )
         {  continue;
         }
         else if ( EAGAIN == err )
         {  cout << "EAGAIN error on Send Socket!" << endl;
            ret = ::poll ( poll_socket_descriptor, 1, 1000 );
            if ( ret < 0 ) // Handles both timeout and errors in select call
            {  err = errno;
               if ( EINTR == err )
               {  continue;
               }
               char error_string_buffer[ERR_STR_LENGTH] = { 0 };
               const char* errorstring = get_error_string ( err, error_string_buffer );

               cout << "Error (" << err << ")" << errorstring << " while poll on socket for sending message:"
                    << m_socket <<endl ;
               return false;
            }
            else if ( 0 == ret )
            {  cout << "Poll timed out for sending message to peer: " << endl;
               return false;
            }
            else
            {  if ( poll_socket_descriptor[0].revents & POLLOUT ) // To check whether socket is really ready to send.
               {  continue;
               }
               else if ( poll_socket_descriptor[0].revents & POLLNVAL )
               {  cout << "Received POLLNVAL on socket while sending to " << endl;
                  return false;
               }
               else if ( poll_socket_descriptor[0].revents & POLLERR )
               {  cout << "Received POLLERR Error on socket while sending to " << endl;
                  return false;
               }
               else if ( poll_socket_descriptor[0].revents & POLLHUP )
               {  cout << "Received POLLHUP Error on socket while sending to " << endl;
                  return false;
               }
            }
         }
         else
         {  char error_string_buffer[ERR_STR_LENGTH] = { 0 };
            const char* errorstring = get_error_string ( err, error_string_buffer );
            cout << "Message send failed for socket descriptor:" << m_socket << "and the error is " << err << "("
                 << errorstring << ")" << endl;
            return false;
         }
      }
      else
      {  cout << "Successfully sent " << ret << " bytes "<< endl;
         n_left -= ret;
         out_message += ret;
         ret = length - n_left;
      }
   }
   return true;
}


ssize_t TransportInterface::receive ( char* data, const size_t& length )
{

   size_t n_left = length;
   ssize_t nread = 0;
   // The receive will come out only on receiving the expected number of bytes of data or
   // on timeout of the select system call.
   int ret = 0, err = 0;
   struct pollfd poll_socket_descriptor[1];
   poll_socket_descriptor[0].fd = m_socket;
   poll_socket_descriptor[0].events = POLLIN;
   while ( n_left > 0 )
   {  nread = receive_from_socket ( data, n_left, err );
      if ( nread <= 0 )
      {  if ( 0 == nread )
         {  cout << "Peer has closed connection!" << endl;
            exit ( 0 );
         }
         else if ( EINTR == err )
         {  cout << "Interrupt signal received on recv. Continue to wait for data." << endl;
            continue;
         }
         else if ( EAGAIN == err )
         {

            ret = ::poll ( poll_socket_descriptor, 1, 1000 );

            if ( ret < 0 ) // Handles errors of poll()
            {  err = errno;
               if ( EINTR == err )
               {  cout << "Interrupt signal received on select. Continue to wait for data." << endl;
                  continue;
               }
               char error_string_buffer[ERR_STR_LENGTH] = { 0 };
               const char* errorstring = get_error_string ( err, error_string_buffer );

               cout << "Error (" << err << ")" << errorstring
                    << " while poll on socket for receiving messages:" << m_socket << endl;
               nread = -1;
               break;
            }
            else if ( 0 == ret ) // Handles timeout of poll()
            {  cout << "poll for receive timed out!" << endl;
               nread = -1;
               break;
            }
            else
            {  if ( poll_socket_descriptor[0].revents & POLLIN ) // To check whether socket is really ready to read.
               {  continue;
               }
               else if ( poll_socket_descriptor[0].revents & POLLNVAL )
               {  cout << "Received POLLNVAL on socket while receiving from " << endl;
                  nread = -1;
                  break;
               }
               else if ( poll_socket_descriptor[0].revents & POLLERR )
               {  cout << "Received POLLERR Error on socket while receiving from " << endl;
                  nread = -1;
                  break;
               }
               else if ( poll_socket_descriptor[0].revents & POLLHUP )
               {  cout << "Received POLLHUP Error on socket while receiving from " << endl;
                  nread = -1;
                  break;
               }
            }
         }
         else
         {  char error_string_buffer[ERR_STR_LENGTH] = { 0 };
            const char* errorstring = get_error_string ( err, error_string_buffer );

            cout << "Error while receiving messages : (" << err << ")" << errorstring <<
                 "Current socket descriptor is : " << m_socket << endl;
            nread = -1;
            exit ( 0 );
         }
      }
      n_left -= nread;
      data += nread;
   }
   return nread;
}


bool TransportInterface::receive_message ( char* receive_buffer, const ssize_t& length )
{

   ssize_t ret = receive ( receive_buffer, length );
   if ( ret > 0 )
   {  return true;
   }
   return false;

}

ssize_t TransportInterface::receive_from_socket ( char* data, const size_t& length, int& err )
{  ssize_t nread = recv ( m_socket, data, length, 0 );
   if ( nread <= 0 )
   {  err = errno;
      char error_string_buffer[ERR_STR_LENGTH] = { 0 };
      const char* errorstring = get_error_string ( err, error_string_buffer );

      cout << "Error : " << errorstring << " (" << err << ") - while receiving message on "
           << m_socket << endl;
   }
   return nread;
}


ssize_t TransportInterface::send_to_socket ( const char* out_message, const int& length, int& err )
{

   cout << "sending message out" << out_message<< "on fd: "<< m_socket << endl;

   int ret_value = ::send ( m_socket, out_message, length, 0 );
   if ( ret_value <= 0 )
   {  err = errno;
      char error_string_buffer[ERR_STR_LENGTH] = { 0 };
      const char* errorstring = get_error_string ( err, error_string_buffer );

      cout << "Error : " << errorstring << " (" << err << ") - while sending message on "
           << m_socket << endl;

   }
}

void TransportInterface::close()
{


   cout << "Closing connection - " << m_socket<< endl;

   if ( -1 != m_socket )
   {  struct linger sock_linger_option;
      sock_linger_option.l_onoff = 1; // Switch on the LINGER feature

      sock_linger_option.l_linger = 0; // Waiting time after CLOSE call in sec

      cout << "Setting SO_LINGER option on the socket : " << m_socket << endl;
      if ( setsockopt ( m_socket, SOL_SOCKET, SO_LINGER, &sock_linger_option, sizeof ( sock_linger_option ) ) < 0 )
      {  int err = errno;
         char error_string_buffer[ERR_STR_LENGTH] = { 0 };
         const char* errorstring = get_error_string ( err, error_string_buffer );

         cout << "Error : " << errorstring << " (" << err << ") - while setting linger option on socket "
              << m_socket << endl;
      }


      int ret = ::shutdown ( m_socket, SHUT_RDWR );
      if ( ret != 0 )
      {  int err = errno;
         char error_string_buffer[ERR_STR_LENGTH] = { 0 };
         const char* errorstring = get_error_string ( err, error_string_buffer );
         cout << "Error : " << errorstring << " (" << err << ") - while shutting down the socket " << endl;


         ret = ::close ( m_socket );
         if ( ret != 0 )
         {  // log the error if socket close returns an error
            int err = errno;
            char error_string_buffer[ERR_STR_LENGTH] = { 0 };
            const char* errorstring = get_error_string ( err, error_string_buffer );
            cout << "Error : " << errorstring << " (" << err << ") - while closing the socket "
                 << m_socket << endl;
         }
         m_socket = -1;
      }

   }
}

void TransportInterface::accept()
{


   sockaddr peer_address;
   socklen_t length = sizeof ( peer_address );
   memset ( &peer_address, '\0', length );

   int conn_sock_fd = ::accept ( m_passive_socket, ( sockaddr* ) ( &peer_address ), &length );

   if ( conn_sock_fd < 0 )
   {  int err = errno;
      char error_string_buffer[ERR_STR_LENGTH] = { 0 };
      const char* errorstring = get_error_string ( err, error_string_buffer );

      // If the socket is marked non-blocking and no pending connections are present on the queue,
      // accept() fails with the error EAGAIN or EWOULDBLOCK but still we return ERROR.
      // we should't block here in this case with select() or poll() as the error from here will be
      // handled as an re-try option and Listener thread will handle using poll().
      if ( EAGAIN == err || EINTR == err || ECONNABORTED == err || EPROTO == err )
      {  cout << "Received :(" << err << ") " << errorstring << " while accepting a connection will be ignored" << endl;
      }
      else
      {  cout << "Error :(" << err << ") " << errorstring << " while accepting a connection" << endl;
      }

      exit ( 0 );
   }
   else
   {

      char ip_str[INET6_ADDRSTRLEN];
      if ( inet_ntop ( AF_INET, peer_address.sa_data, ip_str, sizeof ( ip_str ) ) == 0 )
      {  cout << "Error in peer address conversion" << endl;
      }
      else
      {  //cout << "Connection accepted from " << ip_str << ". Resulting socket descriptor is :" << conn_sock_fd << endl;
      }


      socklen_t len = sizeof ( struct sockaddr_storage );
      const sockaddr_storage& addr = m_socket_address_src;
      if ( ::getsockname ( conn_sock_fd, ( struct sockaddr* ) &addr, &len ) != 0 )
      {  int err = errno;
         char error_string_buffer[ERR_STR_LENGTH] = { 0 };
         const char* errorstring = get_error_string ( err, error_string_buffer );

         cout << "Error (" << err << ")" << errorstring << " while getting getsockname() :" << endl;

         ::close ( conn_sock_fd );
         conn_sock_fd = -1;
         exit ( 0 );
      }



      const sockaddr_storage& peer_addr = m_socket_address_dest;
      if ( ::getpeername ( conn_sock_fd, ( struct sockaddr* ) &peer_addr, &len ) != 0 )
      {  int err = errno;
         char error_string_buffer[ERR_STR_LENGTH] = { 0 };
         const char* errorstring = get_error_string ( err, error_string_buffer );

         cout << "Error (" << err << ")" << errorstring << " while getting getpeername() :"<< endl;

         ::close ( conn_sock_fd );
         conn_sock_fd = -1;
         exit ( 0 );

      }

      char host_ip_address[INET6_ADDRSTRLEN] = {0};

      inet_ntop ( AF_INET,
                  ( void* ) & ( ( ( sockaddr_in* ) ( &m_socket_address_dest ) )->sin_addr.s_addr ),
                  host_ip_address,
                  INET6_ADDRSTRLEN );

      cout << "connection accepted from addr:=" << host_ip_address << " ";
      cout << "port:=" << ntohs ( ( * ( ( sockaddr_in* ) ( &m_socket_address_dest ) ) ).sin_port ) << " ";
      cout << "sin_family:=" << ( * ( ( sockaddr_in* ) ( &m_socket_address_dest ) ) ).sin_family << endl;

      m_socket = conn_sock_fd;
      cout << "socket connected: " << m_socket << endl;

      pthread_t sender_tid;
      int return_code = pthread_create ( &sender_tid, NULL, socket_sender_thread, this );

   }

}

bool TransportInterface::connect()
{  const sockaddr_storage& remote_addr = m_socket_address_dest;
   ::connect ( m_socket, ( sockaddr* ) &remote_addr, sizeof ( sockaddr_in ) );

   // spin until socket connects
   int ret = true;
   for ( int  i =0 ; i<= 2 ; i++ )
   {  ret = poll();

      if ( ret == true )
      {  return true;
      }

   }
   return false;
}


bool TransportInterface::poll()
{

   int ret = -1, err = EINPROGRESS;

   struct pollfd poll_socket_descriptor[1];
   poll_socket_descriptor[0].fd = m_socket;
   poll_socket_descriptor[0].events = POLLIN | POLLOUT;
   if ( ret < 0 )
   {  if ( EINPROGRESS != err )
      {  char error_string_buffer[ERR_STR_LENGTH] = { 0 };
         const char* errorstring = get_error_string ( err, error_string_buffer );
         cout<< "Error (" << err << ")" << errorstring << " while connecting to the peer :" << endl;
         return false;
      }
      else
      {

         while ( 1 )
         {

            cout << "polling ...." << endl;

            ret = ::poll ( poll_socket_descriptor, 1, 1000 );
            ++m_connection_attempt;
            err = errno;
            m_errornumber = err;
            char error_string_buffer[ERR_STR_LENGTH] = { 0 };
            const char* errorstring = get_error_string ( err, error_string_buffer );
            cout << "Error (" << err << ")" << errorstring << " while select on socket  :" << m_socket << " error code: " << ret << endl;

            if ( ret < 0 ) // Handles errors in poll()
            {

               if ( EINTR == err || EINPROGRESS == err || EALREADY == err )
               {  continue;
               }



               return false;

            }
            else if ( ret == 0 )
            {  return false;
            }
            else
            {

               if ( poll_socket_descriptor[0].revents & POLLOUT ||
                     poll_socket_descriptor[0].revents & POLLIN ) // To check whether socket is really ready to connect.
               {  int get_sock_opt_error = 0;
                  socklen_t length = sizeof ( get_sock_opt_error );
                  err = 0;
                  ret = getsockopt ( m_socket, SOL_SOCKET, SO_ERROR, &get_sock_opt_error, &length );
                  if ( ret < 0 )
                  {  err = errno; // Solaris case where the getsockopt return -1 and errno is set to pending error.
                  }
                  else if ( get_sock_opt_error )
                  {  // Linux case where the getsockopt return 0 and pending error returned in error argument get_sock_opt_error
                     err = get_sock_opt_error;
                  }
                  if ( err != 0 )
                  {  char error_string_buffer[ERR_STR_LENGTH] = { 0 };
                     const char* errorstring = get_error_string ( err,
                                               error_string_buffer );

                     cout << "Error (" << err << ")" << errorstring << " while connecting to the peer :"
                          << endl;
                     return false;
                  }
               }
               else if ( poll_socket_descriptor[0].revents & POLLNVAL )
               {  cout << "Received POLLNVAL on socket while connecting to " << endl;
                  return false;
               }
               else if ( poll_socket_descriptor[0].revents & POLLERR )
               {  cout << "Received POLLERR Error on socket while connecting to " << endl;
                  return false;
               }
               else if ( poll_socket_descriptor[0].revents & POLLHUP )
               {  cout<< "Received POLLHUP Error on socket while connecting to " << endl;
                  return false;
               }
            }
            break;
         }

      }
   }

   return true ;

}

void  TransportInterface::create_sock_in_struct ( string address, sockaddr_storage& m_socket_address, uint32_t port )
{

   cout << "creating socket address for ip: " << address << "  and port: " << port << endl;

   bzero ( &m_socket_address, sizeof ( m_socket_address ) );
   struct sockaddr_in* ip_addr = ( sockaddr_in* ) &m_socket_address;

   ( *ip_addr ).sin_family = AF_INET;
   ( *ip_addr ).sin_port =htons ( port );

   if ( 1 != inet_pton ( AF_INET, address.c_str(), & ( ip_addr->sin_addr ) ) )
   {  cout <<  "error while conversion" << endl;
      exit ( 0 );
   }

}

void TransportInterface::set_sock_option()
{

   int sockfd = 0;

   if ( m_type == CLIENT )
   {  sockfd = m_socket;
   }
   else
   {  sockfd = m_passive_socket;
   }

   cout << "setting O_NONBLOCK option on the socket: " << endl;
   if ( fcntl ( sockfd, F_SETFL, O_NONBLOCK ) < 0 )
   {  cout << "Unable to set the socket as NON-Blocking socket: " << endl;
      exit ( 0 );
   }

   int sockopt = 1; // Switch on the Re-use addr socket option
   cout << "setting SO_REUSEADDR option on the socket: " << endl;

   if ( setsockopt ( sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof ( sockopt ) ) < 0 )
   {  int err = errno;
      char error_string_buffer[ERR_STR_LENGTH] = { 0 };
      const char* errorstring = get_error_string ( err, error_string_buffer );
      cout << "setsockopt( ... SO_REUSEADDR ...) failed, Error ignored. errno = " << errno << " : " << errorstring << endl;
      exit ( 0 );
   }

}


void TransportInterface::create_socket()
{

   int socketfd = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
   if ( 0 > m_socket )
   {  cout << "error while creating socket" << endl;
      exit ( 0 );
   }

   if ( m_type == CLIENT )
   {  m_socket = socketfd;
      m_passive_socket = socketfd;
   }
   else
   {  m_passive_socket = socketfd;
   }


   create_sock_in_struct ( m_source_ip_str,m_socket_address_src, m_src_port );
   set_sock_option();
   bind ( m_socket_address_src );

   if ( m_type == CLIENT )
   {  //client


      create_sock_in_struct ( m_dest_ip_str, m_socket_address_dest, m_dest_port );

      pthread_t sender_tid;
      int return_code = pthread_create ( &sender_tid, NULL, socket_sender_thread, this );
   }
   else
   {  //server
      //create_sock_in_struct ( m_source_ip_str,m_socket_address_src, m_src_port );

      listen();
   }

}

void TransportInterface::bind ( sockaddr_storage& local_address )
{

   int sockfd = 0;

   if ( m_type == CLIENT )
   {  sockfd = m_socket;
   }
   else
   {  sockfd = m_passive_socket;
   }


   int result = ::bind ( sockfd, ( sockaddr* ) &local_address, sizeof ( sockaddr_in ) );
   int err = errno;
   if ( result < 0 )
   {  char error_string_buffer[ERR_STR_LENGTH] = { 0 };
      const char* errorstring = get_error_string ( err, error_string_buffer );
      cout << "TCP:Error (" << err << ")" << errorstring << " while binding the socket:=" << sockfd << endl;
      exit ( 0 );
   }

}

void TransportInterface::init ( uint8_t transport_type, string src_address, string dest_address, uint32_t src_port, uint32_t dest_port )
{
   m_source_ip_str = src_address;
   m_dest_ip_str = dest_address;
   m_dest_port = dest_port;
   m_src_port = src_port;
   m_type = transport_type;
   create_socket ();

}

void* socket_listener_thread ( void* arg )
{  TransportInterface* transportinstance = ( TransportInterface* ) arg;
   socketlistener *ptr = new socketlistener ( transportinstance );
   transportinstance->m_listener = ptr;
   ptr->start_thread();
   delete ptr;
}



void TransportInterface::listen()
{  cout <<  "Start listening ...."<< endl;
   int ret = ::listen ( m_passive_socket, 7 );
   int err = errno;
   if ( ret < 0 )
   {  char error_string_buffer[ERR_STR_LENGTH] = { 0 };
      const char* errorstring = get_error_string ( err, error_string_buffer );

      cout << "Error :(" << err << ") " << errorstring << " while listening on " << endl;
      exit ( 0 );
   }

   pthread_t listener_tid;
   int return_code = pthread_create ( &listener_tid, NULL, socket_listener_thread, this );

}


TransportInterface* TransportInterface::get_instance()
{

   static TransportInterface instance;
   return &instance;
}


