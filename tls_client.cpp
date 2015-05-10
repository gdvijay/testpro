#include "transportinterface.h"
#include "socketreceiver.h"
#include "socketsender.h"
#include <pthread.h>
#include <cstdlib>
#include <vector>

//! converts string to long
static bool convertStrToLong ( string& value, uint32_t& convertedValue )
{  char* endPtr = 0;

   unsigned long long int intvalue = strtoull ( value.c_str(), &endPtr, 10 ) ;
   convertedValue = ( uint32_t ) intvalue;
   if ( endPtr[0] != '\0' )
   {  return false;
   }
   return true;
}

void validate_argument ( vector<string>& arguments, uint8_t& transport_type, string& src_address, string& dest_address, uint32_t& src_port, uint32_t& dest_port )
{

   if ( arguments.empty() )
   {  cout << "no arguments" << endl;
      exit ( 0 );
   }

   if ( arguments[0].compare ( "-c" ) == 0 )
   {  transport_type =0 ;
   }
   else if ( arguments[0].compare ( "-s" ) == 0 )
   {  transport_type =1 ;
   }
   else
   {  cout << "insufficient arguments" << endl;
      exit ( 0 );

   }

   if ( ( transport_type == 0 ) && ( arguments.size() != 9 ) )
   {  cout << "insufficient arguments for client" << endl;
      exit ( 0 );

   }
   else if ( ( transport_type == 1 ) && ( arguments.size() != 5 ) )
   {  cout << "insufficient arguments for server" << endl;
      exit ( 0 );

   }

   src_address = arguments[2];
   convertStrToLong ( arguments[4] , src_port );

   cout << "src_address: " << src_address <<endl;
   cout << "src_port: " << src_port <<endl;


   if ( transport_type == 0 )
   {  dest_address =  arguments[6];
      convertStrToLong ( arguments[8] , dest_port );
      cout << "dest_address: " << dest_address <<endl;
      cout << "dest_port: " << dest_port <<endl;
   }

}

int main ( int argc, char* argv[] )
{

   uint8_t transport_type = 0;
   string src_address;
   string dest_address;
   uint32_t src_port;
   uint32_t dest_port;



   cout << "argc: " << argc << endl;
   //./tls_client -c -h 192.168.248.131 -p 13867 -H 192.168.248.132 -P 13868
   //./tls_client -s -h 192.168.248.131 -p 13867

   vector <std::string> arguments;
   for ( int i = 1; i < argc; ++i )
   {  arguments.push_back ( argv[i] );
   }

   validate_argument ( arguments, transport_type, src_address, dest_address, src_port, dest_port );

   TransportInterface* transportinstance = TransportInterface::get_instance();
   transportinstance->init ( transport_type, src_address,dest_address, src_port,dest_port);



   while ( 1 )
   {  sleep ( 100 );
   }

   //pthread_join ( sender_tid, NULL );
   //pthread_join ( receiver_tid, NULL );


   return 0;

}