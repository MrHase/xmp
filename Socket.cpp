// Implementation of the Socket class.


#include "Socket.h"
#include "string.h"
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
//#include <cstdint>



Socket::Socket() :
  m_sock ( -1 )
{

  memset ( &m_addr,
	   0,
	   sizeof ( m_addr ) );

}

Socket::~Socket()
{
  if ( is_valid() )
    ::close ( m_sock );
}

bool Socket::create()
{
  m_sock = socket ( AF_INET,
		    SOCK_STREAM,
		    0 );

  if ( ! is_valid() )
    return false;


  // TIME_WAIT - argh
  int on = 1;
  if ( setsockopt ( m_sock, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) == -1 )
    return false;


  return true;

}



bool Socket::bind ( const int port )
{

  if ( ! is_valid() )
    {
      return false;
    }



  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = INADDR_ANY;
  m_addr.sin_port = htons ( port );

  int bind_return = ::bind ( m_sock,
			     ( struct sockaddr * ) &m_addr,
			     sizeof ( m_addr ) );


  if ( bind_return == -1 )
    {
      return false;
    }

  return true;
}


bool Socket::listen() const
{
  if ( ! is_valid() )
    {
      return false;
    }

  int listen_return = ::listen ( m_sock, MAXCONNECTIONS );


  if ( listen_return == -1 )
    {
      return false;
    }

  return true;
}


bool Socket::accept ( Socket& new_socket ) const
{
  int addr_length = sizeof ( m_addr );
  new_socket.m_sock = ::accept ( m_sock, ( sockaddr * ) &m_addr, ( socklen_t * ) &addr_length );

  if ( new_socket.m_sock <= 0 )
    return false;
  else
    return true;
}


bool Socket::send ( const std::string& s, const bool fastmode ) const
{
	//allow_every_char = true:
	// you can also send a msg with "\0" in the msg
	// but also your message can max ((2^32)-4) byte contain
	
	//new... we add the length of the message at pos 0	

  
	
 	unsigned long length=s.length();
  	//printf("senden laenge: %x\n",length);
  	//std::cout <<"senden laenge: "<<length<<std::endl;
	std::string msg=s;
	if(fastmode==true){
		
		char* ptr;
		ptr=(char*)&length;
		msg.insert(0,1,*ptr);
		msg.insert(0,1,*(++ptr));
		msg.insert(0,1,*(++ptr));
		msg.insert(0,1,*(++ptr));

	}else{
		//this mode is very slow..
		//but easy to implement
		msg.insert(0,1,'\0');
		msg.insert(0,1,'\0');
		msg.insert(0,1,'\0');
		msg.insert(0,1,'\0');
		msg.insert(length+4,1,'\0');//4 zeichen sind ja dazugekommen!
		
	}
	
	
	//std::cout <<"laenge msg: "<<msg.length()<<std::endl;
  	//std::cout <<"msg: "<<msg<<std::endl;
  int status = ::send ( m_sock, msg.c_str(), msg.size(), MSG_NOSIGNAL );
  if ( status == -1 )
    {
      return false;
    }
  else
    {
      return true;
    }
}


int Socket::recv ( std::string& s ) const
{
	char buf [ MAXRECV + 1 ];
	unsigned char size[4];
	char recv_char[1];
	unsigned long length=0;
	s = "";
	size[0]=0;
	size[1]=0;
	size[2]=0;
	size[3]=0;
	memset ( buf, 0, MAXRECV + 1 );
	
	int status=::recv(m_sock,size,4,0);//recv the first 4 bytes to get the length
	if(status==0 or status==-1) return 0;
  	//printf("empfangen size: %x %x %x %x\n",size[0],size[1],size[2],size[3]);
  	//printf("empfangen long: %l\n",size);
	unsigned long tmp_size=0;
	tmp_size=size[0];
	length+=tmp_size<<24;
	tmp_size=size[1];
	length+=tmp_size<<16;
	tmp_size=size[2];
	length+=tmp_size<<8;
	tmp_size=size[3];
	length+=tmp_size;
	
	//printf("empfangen laenge: %x\n",length);
	
	
  //status = ::recv ( m_sock, buf, MAXRECV, 0 );
  //status = ::recv ( m_sock, buf, length, 0 );
  	if(length==0){
  		//std::cout<<"----"<<std::endl;
  		//while(recv_char[0]!='\0'){
  		do{
  				status = ::recv ( m_sock, recv_char, 1, 0 );
  				if ( status == -1 ){
				  std::cout << "status == -1   errno == " << errno << "  in Socket::recv\n";
				  return 0;
				} else if ( status == 0 ){
				  return 0;
				}  else
				{
				  s+=recv_char;
				  
				}
  		}while(recv_char[0]!='\0');
		// we must remove the last char!!! cause its a \0 now which was never send!  		
  	}else{
	//! ist still slow cause wie use a loop here
	//! we should receive the data with ::recv(m_sock,buf,lenght,?) but i get an error then
		for (int i=0;i<length;i++){
			status = ::recv ( m_sock, recv_char, 1, 0 );
			//status = ::recv ( m_sock, buf, length, 0 );
			
				
			if ( status == -1 ){
				std::cout << "status == -1   errno == " << errno << "  in Socket::recv\n";
				return 0;
			}else if ( status == 0 ){
				return 0;
			}else{
				/*for (int i=0;i<length;i++){
					s+=buf[i];
				}
				* */
				s+=recv_char;
				
			}
			//std::cout<<"status: " <<status<< "i: "<<i<<" recv_char: "<<recv_char[0]<<std::endl;
		}
	}
	//std::cout<<"status: "<<status<<std::endl;
	return status;
}



bool Socket::connect ( const std::string host, const int port )
{
  if ( ! is_valid() ) return false;

  m_addr.sin_family = AF_INET;
  m_addr.sin_port = htons ( port );

  int status = inet_pton ( AF_INET, host.c_str(), &m_addr.sin_addr );

  if ( errno == EAFNOSUPPORT ) return false;

  status = ::connect ( m_sock, ( sockaddr * ) &m_addr, sizeof ( m_addr ) );

  if ( status == 0 )
    return true;
  else
    return false;
}

void Socket::set_non_blocking ( const bool b )
{

  int opts;

  opts = fcntl ( m_sock,
		 F_GETFL );

  if ( opts < 0 )
    {
      return;
    }

  if ( b )
    opts = ( opts | O_NONBLOCK );
  else
    opts = ( opts & ~O_NONBLOCK );

  fcntl ( m_sock,
	  F_SETFL,opts );

}
