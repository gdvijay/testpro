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

#include "tlsmanager.h"
#include "sslthreadwrapper.h"
#include <openssl/bio.h>

using namespace std;

void tlsmanager::tls_accept()
{  int ret_code = 0;

   BIO *sbio = 0;

   if ( m_transport_ptr->m_protocol == 0 )
   {  sbio=BIO_new_socket ( m_transport_ptr->m_socket,BIO_NOCLOSE );

   }
   else
   {  sbio=BIO_new_dgram_sctp ( m_transport_ptr->m_socket,BIO_NOCLOSE );
   }


   if ( !sbio )
   {  exit ( 0 );

   }

   m_ssl=SSL_new ( m_ctx );
   SSL_set_bio ( m_ssl,sbio,sbio );

   if ( ( ret_code=SSL_accept ( m_ssl ) <=0 ) )
   {  cout << "error on ssl accept" << endl;
   }

}


void tlsmanager::tls_connect()
{  int err;

   struct pollfd poll_socket_descriptor[1];
   poll_socket_descriptor[0].fd = m_transport_ptr->m_socket;

   //create a new SSL connection  state object.
   m_ssl = SSL_new ( m_ctx );

   BIO *sbio = 0;

   if ( m_transport_ptr->m_protocol == 0 )
   {  sbio=BIO_new_socket ( m_transport_ptr->m_socket,BIO_NOCLOSE );

   }
   else
   {  sbio=BIO_new_dgram_sctp ( m_transport_ptr->m_socket,BIO_NOCLOSE );
   }


   // attach the ssl session to the socket descriptor.
   //sbio=BIO_new_socket ( m_transport_ptr->m_socket, BIO_NOCLOSE );
   SSL_set_bio ( m_ssl, sbio, sbio );

   int attempt = 0;
   while ( attempt < 3 )
   {


      int r_code =0;
      //now try ssl connect
      r_code = SSL_connect ( m_ssl );
      if ( r_code ==1 )
      {  cout << "tls connect sucesssful" << endl;
         //tls connect sucessful.
         return;
      }
      ++attempt;

      int ssl_error = SSL_get_error ( m_ssl, r_code );
      cout << "ssl_connect error code "<< ssl_error << tlsmanager::get_error_string() << endl;

      switch ( ssl_error )
      {  case SSL_ERROR_WANT_WRITE:
         {  //wait for socket to become writable and retry ssl_connect again
            // to avoid this thread blocking on this poll, start a RTP timer and do poll() when timer expires.
            poll_socket_descriptor[0].events = POLLOUT;

         }
         break;
         case SSL_ERROR_WANT_READ:
         {  //wait for socket to become readable and retry ssl_connect again
            poll_socket_descriptor[0].events = POLLIN;

         }
         break;
         default:
         {  cout << "unhandled error in ssl connect" << endl;
            return;
         }
         break;

      }

      int poll_ret = poll ( poll_socket_descriptor, 1, 2000 );
      if ( ( poll_ret < 0 ) || ( 0 == poll_ret ) )
      {  err = errno;
         if ( EINTR == err )
         {  continue;
         }
         cout << "Error: poll for ssl connect returned: " << poll_ret<<endl;
         return;

      }
      else
      {  //poll returned sucesssful
         if ( ( poll_socket_descriptor[0].revents & POLLOUT ) || ( poll_socket_descriptor[0].revents & POLLIN ) )
         {  //try ssl_connect again.
            continue;
         }
         else if ( ( poll_socket_descriptor[0].revents & POLLNVAL ) ||
                   ( poll_socket_descriptor[0].revents & POLLERR ) ||
                   ( poll_socket_descriptor[0].revents & POLLHUP ) )
         {  cout << "Error: poll for ssl connect returned: " << poll_ret<<endl;
            return;
         }
      }

   }
}

void tlsmanager::check_key_cert_consistency()
{  //check certificate consistency with key.
   if ( SSL_CTX_check_private_key ( m_ctx ) != 1 )     // check cert <-> key consistency
   {  //cert <-> key pair is inconsistent
      cout <<"function SSL_CTX_check_private_key(\"RSA certificate <-> key\") failed,"<< endl;
      cout <<"RSA certificate <==> key mismatch."<<endl;
      exit ( 0 );
   }


   /* Load the CAs we trust*/

   /*
   if(!(SSL_CTX_load_verify_locations(ctx,CA_LIST,0)))
     berr_exit("Couldn't read CA list");
   SSL_CTX_set_verify_depth(ctx,1);
   */

   /* Load randomness */
   /*
   if(!(RAND_load_file(RANDOM,1024*1024)))
     berr_exit("Couldn't load randomness");
   */


}


void tlsmanager::feed_key()
{  std::stringstream file_content;
   string key_file_path = "domain.key";
   std::ifstream in ( key_file_path.c_str(), std::ios_base::in );
   file_content << in.rdbuf();
   std::string key_file_content = file_content.str();

   //convert key
   EVP_PKEY *evp_key = 0;
   BIO *bio = BIO_new_mem_buf ( ( void* ) key_file_content.c_str(), static_cast<unsigned> ( key_file_content.size() ) );
   if ( !bio )
   {  cout << "Error in creating BIO for key" << endl;
      exit ( 0 );
   }
   evp_key = PEM_read_bio_PrivateKey ( bio, NULL, NULL, NULL );

   if ( 0 == evp_key )
   {  cout << "PEM_read_bio_PrivateKey() failed, OpenSSL error queue: \""<< endl;

   }

   BIO_free ( bio );
   if ( SSL_CTX_use_PrivateKey ( m_ctx, evp_key ) != 1 ) //feed OpenSSL
   {  //feeding failed
      cout <<  "function SSL_CTX_use_PrivateKey failed, OpenSSL error queue: \""<< endl;
      EVP_PKEY_free ( evp_key );                                  //free converted key
      exit ( 0 );
   }

}

void tlsmanager::feed_certificate()
{

   std::stringstream file_content;
   string cert_file_path = "domain.crt";
   std::ifstream in ( cert_file_path.c_str(), std::ios_base::in );
   file_content << in.rdbuf();
   std::string cert_file_content = file_content.str();

   X509* x509_cert = NULL;

   //convert certificate
   BIO* bio  = BIO_new_mem_buf ( ( void* ) cert_file_content.c_str(), static_cast<unsigned> ( cert_file_content.size() ) );
   if ( 0 == bio )
   {  cout << "Conversion from PEM format for " << " certificate failed." << endl;
      exit ( 0 );
   }

   x509_cert = PEM_read_bio_X509 ( bio, NULL, NULL, NULL );

   if ( 0 == x509_cert )
   {  cout << "PEM_read_bio_X509() failed, OpenSSL error queue: \<<"<<endl;
      exit ( 0 );
   }

   BIO_free ( bio );

   char subject[256] = "";
   char issuer [256] = "";

   X509_NAME_oneline ( X509_get_issuer_name ( x509_cert ), issuer,  sizeof ( issuer ) ); //extract issuer
   X509_NAME_oneline ( X509_get_subject_name ( x509_cert ), subject, sizeof ( subject ) ); //extract subject
   cout << " certificate exists, issuer: "<<issuer<<" subject: "<<subject << endl; //trace it

   if ( SSL_CTX_use_certificate ( m_ctx, x509_cert ) != 1 )            //feed OpenSSL
   {  //feeding certificates failed
      cout << "function SSL_CTX_use_certificate failed, OpenSSL error queue:";
      X509_free ( x509_cert );                                                    //free certificate
      cout <<  "Cannot use certificate." << endl;
      exit ( 0 );
   }

}

static int verify_certificate ( X509_STORE_CTX *x509_store_ctx, void* arg )
{  return 1;

}

string tlsmanager::get_error_string()
{

   unsigned long   error_code = 0;
   char            error_code_string[256];
   const char      *file = 0, *data = 0;
   int             line = 0, flags = 0;
   std::string     error_string;
   unsigned long   tid = pthread_self();

   //each thread has its own error queue => also the 'data'/'file' is thread priv
   while ( ( error_code=ERR_get_error_line_data ( &file, &line, &data, &flags ) ) != 0 )
   {  error_code_string[0] = '\0'; //should not be necessary
      ERR_error_string_n ( error_code, error_code_string, sizeof ( error_code_string ) ); //converts error_code into human-readable string
      error_code_string[sizeof ( error_code_string )-1] = '\0'; //should not be necessary, manpage states that it is \0-terminated

      std::stringstream strm;
      strm<< "tid=="<<tid<<":"<<error_code_string<<":"<<file<<":"<<line<<":additional info...\""<< ( ( flags & ERR_TXT_STRING ) ? data : "" ) <<"\"\n";
      error_string += strm.str();
   }
   return error_string;


}


void tlsmanager::initialise_context()
{

   //TLS_RSA_WITH_RC4_128_MD5
   //TLS_RSA_WITH_RC4_128_SHA
   //TLS_RSA_WITH_3DES_EDE_CBC_SHA
   //TLS_RSA_WITH_AES_128_CBC_SHA

   ssl_thread_setup();

   string cipher_list = "RC4-MD5:RC4-SHA:DES-CBC3-SHA:AES128-SHA";

   //SSL_library_init() registers the available SSL/TLS ciphers and digests.
   SSL_library_init();


   //ERR_load_crypto_strings() registers the error strings for all libcrypto functions. SSL_load_error_strings() does the same,
   //but also registers the libssl error strings.
   SSL_load_error_strings();


   //create SSLv2 SSLv3 method and allow only TLS using set options.
   //use the same SSL context for all server connections.

   if ( m_transport_ptr->m_type == CLIENT )
   {

      if ( m_transport_ptr->m_protocol ==0 )
         m_ctx = SSL_CTX_new ( SSLv23_client_method() );
      else
         m_ctx = SSL_CTX_new ( DTLSv1_client_method() );

      SSL_CTX_set_verify ( m_ctx, SSL_VERIFY_PEER, NULL );
   }
   else
   {

      if ( m_transport_ptr->m_protocol ==0 )
         m_ctx = SSL_CTX_new ( SSLv23_server_method() );
      else
         m_ctx = SSL_CTX_new ( DTLSv1_server_method() );

      SSL_CTX_set_verify ( m_ctx, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL );
   }

   //No callbacks set. verify_callback set specifically for this ssl remains.
   //SSL_CTX_set_verify ( m_ctx, SSL_VERIFY_NONE, NULL );


   //SSL_OP_NO_SSLv2 -- Do not use the SSLv2 protocol.
   //SSL_OP_NO_SSLv3 -- Do not use the SSLv3 protocol.

   int opts =  SSL_OP_ALL                          |
               ( SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 ) ;

   SSL_CTX_set_options ( m_ctx, opts );
   SSL_CTX_set_mode ( m_ctx, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER );

   if ( SSL_CTX_set_cipher_list ( m_ctx, cipher_list.c_str() ) != 1 )
   {  cout << "function SSL_CTX_set_cipher_list() failed, OpenSSL error queue: " << endl;
   }

   //switch off session cache.
   SSL_CTX_set_session_cache_mode ( m_ctx, SSL_SESS_CACHE_OFF );
   SSL_CTX_set_cert_verify_callback ( m_ctx,verify_certificate,0 );

}

void tlsmanager::init ( TransportInterface* transport_ptr )
{  m_transport_ptr = transport_ptr;
   initialise_context();
   feed_certificate();
   feed_key();
   check_key_cert_consistency();
}
