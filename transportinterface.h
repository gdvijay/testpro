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

#ifndef TRANSPORTINTERFACE_H
#define TRANSPORTINTERFACE_H


#include <string>
#include <string.h>

#include <sys/socket.h>
#include <stdint.h>
#include <iostream>

#include <errno.h>
#include <stdlib.h>


#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <poll.h>




//#include "socketlistener.h"
//#include "socketreceiver.h"
//#include "socketsender.h"

#define CLIENT 0
#define SERVER 1
#define ERR_STR_LENGTH 256

using namespace std;

class SocketSender;
class SocketReceiver;
class socketlistener;


class TransportInterface
{
private:
public:
  
  int m_socket;
  int m_passive_socket;

  uint8_t m_type;
  uint16_t m_connection_attempt;
  int m_errornumber;


  SocketSender *m_sender;
  SocketReceiver *m_reciever;
  socketlistener *m_listener;

    TransportInterface ()
  {
    m_connection_attempt = 0;
    m_sender = 0;
    m_reciever = 0;
    m_listener = 0;
    m_socket = 0;
    m_passive_socket = 0;
  }
  bool connect ();


ssize_t send_to_socket ( const char* out_message, const int& length, int& err );
ssize_t receive_from_socket ( char* data, const size_t& length, int& err );
bool receive_message ( char* receive_buffer, const ssize_t& length );
ssize_t receive ( char* data, const size_t& length );
bool send_message(const char* out_message, const ssize_t& length);

  static const char *get_error_string (int error_number,
				       char *error_string_buffer)
  {
    const char *return_buffer = NULL;
    return_buffer = strerror_r (error_number, error_string_buffer, 256);
    return return_buffer;
  }

  void set_sock_option ();
  void bind (sockaddr_storage & local_address);
  void close ();
  void accept();
  
  static TransportInterface *get_instance ();
  void init (uint8_t transport_type, string src_address, string dest_address,
	     uint32_t src_port, uint32_t dest_port);
  void create_socket ();
  void create_sock_in_struct (string m_source_ip_str,
			      sockaddr_storage & m_socket_address_src,
			      uint32_t m_src_port);

  bool poll ();
  void listen ();

  string m_source_ip_str;
  string m_dest_ip_str;
  uint32_t m_dest_port;
  uint32_t m_src_port;
  struct sockaddr_storage m_socket_address_src;
  struct sockaddr_storage m_socket_address_dest;


};

#endif // TRANSPORTINTERFACE_H
