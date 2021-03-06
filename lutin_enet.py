#!/usr/bin/python
import realog.debug as debug
import lutin.tools as tools


def get_type():
	return "LIBRARY"

def get_desc():
	return "TCP/UDP/HTTP/FTP interface"

def get_licence():
	return "MPL-2"

def get_compagny_type():
	return "com"

def get_compagny_name():
	return "atria-soft"

def get_maintainer():
	return "authors.txt"

def get_version():
	return "version.txt"

def configure(target, my_module):
	my_module.add_depend([
	    'etk',
	    'ememory',
	    'algue',
	    'ethread'
	    ])
	my_module.add_path(".")
	my_module.add_src_file([
	    'enet/debug.cpp',
	    'enet/enet.cpp',
	    'enet/Udp.cpp',
	    'enet/Tcp.cpp',
	    'enet/TcpServer.cpp',
	    'enet/TcpClient.cpp',
	    'enet/Http.cpp',
	    'enet/Ftp.cpp',
	    'enet/WebSocket.cpp',
	    'enet/pourcentEncoding.cpp',
	    ])
	my_module.add_header_file([
	    'enet/enet.hpp',
	    'enet/debug.hpp',
	    'enet/Udp.hpp',
	    'enet/Tcp.hpp',
	    'enet/TcpServer.hpp',
	    'enet/TcpClient.hpp',
	    'enet/Http.hpp',
	    'enet/Ftp.hpp',
	    'enet/WebSocket.hpp',
	    'enet/pourcentEncoding.hpp',
	    ])
	if "Windows" in target.get_type():
		my_module.add_depend("ws2");
	return True







