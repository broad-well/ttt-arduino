/*
 * =====================================================================================
 *
 *       Filename:  sockcomm.hh
 *
 *    Description:  Socket communication for Tic-Tac-Toe
 *
 *        Version:  0.1
 *        Created:  11/14/2016 09:46:09 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Michael Peng
 *   Organization:  Kent Middle School
 *
 * =====================================================================================
 */

#ifndef TTT_SOCKCOMM

#define TTT_SOCKCOMM
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// Decrypts a character received from the socket. Range: -3 to 30
short decrypt_char(char recv)
{
	return static_cast<short>(recv) - 3;
}

// Encrypts a signed short to a character suitable for socket transfer
char encrypt_char(short send)
{
	return static_cast<char>(send + 3);
}

boost::asio::io_service sock_io;
tcp::acceptor sock_accept(sock_io, tcp::endpoint(tcp::v4(), 52443));
tcp::socket sock;

void proto_init()
{
	try {
		cout << "Waiting for acceptance on TCP port 52443" << endl;
		sock_accept.accept(sock);
		cout << "Client accepted" << endl;
	} catch (exception& e) {
		cerr << e.what() << endl;
	}
}

short proto_out(short proto)
{
	try {
		boost::asio::write(sock,
				boost::asio::buffer(encrypt_char(proto)));
	} catch (exception& e) {
		cerr << e.what() << endl;
	}
}

short proto_query(short proto_out)
{
	try {
		boost::asio::write(sock,
				boost::asio::buffer(encrypt_char(proto_out)));
		char data[1];
		boost::system::error_code errcode;
		// TODO
	}
}

#endif
