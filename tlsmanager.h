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

//TODO:
// alarm for wrong certificate and check_key_cert_consistency
// clearing
// roll back to previous working certificate and key.
// any extra certificate chain file ??
// certificate and key will be in PEM format. 
//certficate verification with CA. 
// pass phrase for private key ??



#ifndef TLSMANAGER_H
#define TLSMANAGER_H

#include <openssl/ssl.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/x509v3.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include "transportinterface.h"
#include <openssl/err.h>


class tlsmanager
{
  SSL_CTX*    m_ctx; 
  SSL* m_ssl;
  
  
public:
  
  TransportInterface* m_transport_ptr;
  void tls_connect();
  void tls_accept();
  
  void tls_shutdown();
  void free_ssl();
  void free_ssl_ctx();
  void write(char* buf, int len);
  void print_peer_certificates();
  
  static string get_error_string();
  
  void check_key_cert_consistency();
  void feed_key();
  void feed_certificate();
  void initialise_context();
  void init(TransportInterface* transport_ptr);
  
  
};

#endif // TLSMANAGER_H
