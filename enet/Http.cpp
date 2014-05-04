/**
 * @author Edouard DUPIN
 * 
 * @copyright 2014, Edouard DUPIN, all right reserved
 * 
 * @license BSD 3 clauses (see license file)
 */

#include <enet/debug.h>
#include <enet/Http.h>
#include <map>
#include <etk/stdTools.h>

#ifdef __class__
	#undef  __class__
#endif
#define __class__ ("Http")

static std::map<int32_t, std::string> getErrorList(void) {
	static std::map<int32_t, std::string> g_list;
	return g_list;
}

enet::Http::Http(void) {
	m_connection.setPort(80);
	m_connection.setServer(false);
}

enet::Http::~Http(void) {
	reset();
}

bool enet::Http::connect(void) {
	if (m_connection.getConnectionStatus() == enet::Tcp::statusLink) {
		return true;
	}
	if (m_connection.link() == false) {
		ENET_ERROR("can not link to the socket...");
		return false;
	}
	return true;
}

void enet::Http::setSendHeaderProperties(const std::string& _key, const std::string& _val) {
	m_sendHeader.insert(make_pair(_key, _val));
}

std::string enet::Http::getSendHeaderProperties(const std::string& _key) {
	ENET_TODO("get header key=" << _key);
	return "";
}

std::string enet::Http::getReceiveHeaderProperties(const std::string& _key) {
	ENET_TODO("get header key=" << _key);
	return "";
}

bool enet::Http::reset(void) {
	if (m_connection.getConnectionStatus() != enet::Tcp::statusLink) {
		m_connection.unlink();
	}
	m_receiveData.clear();
	m_sendHeader.clear();
	m_receiveHeader.clear();
	setSendHeaderProperties("User-Agent", "e-net (ewol network interface)");
}

bool enet::Http::setServer(const std::string& _hostName) {
	// if change server ==> restart connection ...
	if (_hostName == m_connection.getHostName()) {
		return true;
	}
	reset();
	m_connection.setHostNane(_hostName);
	return true;
}

bool enet::Http::setPort(uint16_t _port) {
	// if change server ==> restart connection ...
	if (_port == m_connection.getPort()) {
		return true;
	}
	reset();
	m_connection.setPort(_port);
	return true;
}

bool enet::Http::get(const std::string& _address) {
	m_receiveData.clear();
	m_receiveHeader.clear();
	if (connect() == false) {
		return false;
	}
	std::string req = "GET http://" + m_connection.getHostName();
	if (_address != "") {
		req += "/";
		req += _address;
	}
	req += " HTTP/1.0\n";
	// add header properties :
	for (auto &it : m_sendHeader) {
		req += it.first + ": " + it.second + "\n";
	}
	// end of header
	req += "\n";
	// no body:
	
	int32_t len = m_connection.write(req, false);
	ENET_VERBOSE("read write=" << len << " data: " << req);
	if (len != req.size()) {
		ENET_ERROR("An error occured when sending data " << len << "!=" << req.size());
		return false;
	}
	std::string header;
	// Get data
	char data[1025];
	len = 1;
	bool headerEnded = false;
	while (    m_connection.getConnectionStatus() == enet::Tcp::statusLink
	        && len > 0) {
		len = m_connection.read(data, 1024);
		// TODO : Parse header ...
		
		if (headerEnded == false) {
			char previous = '\0';
			if (header.size()>0) {
				previous = header[header.size()-1];
			}
			for (int32_t iii=0; iii<len; ++iii) {
				if (headerEnded == false) {
					if (data[iii] != '\r') {
						header += data[iii];
						if (data[iii] == '\n') {
							//ENET_VERBOSE("parse: '\\n'");
							if (previous == '\n') {
								//ENET_VERBOSE("End header");
								// Find end of header
								headerEnded = true;
							}
							previous = data[iii];
						} else {
							previous = data[iii];
							//ENET_VERBOSE("parse: '" << data[iii] << "'");
						}
					}
				} else {
					m_receiveData.push_back(data[iii]);
				}
			}
		} else {
			for (int32_t iii=0; iii<len; ++iii) {
				m_receiveData.push_back(data[iii]);
			}
		}
	}
	if (m_connection.getConnectionStatus() != enet::Tcp::statusLink) {
		ENET_WARNING("server disconnected");
		return false;
	}
	// parse header :
	std::vector<std::string> list = std::split(header, '\n');
	headerEnded = false;
	for (auto element : list) {
		if (headerEnded == false) {
			header = element;
			headerEnded = true;
		} else {
			//ENET_INFO("header : '" << element << "'");
			size_t found = element.find(":");
			if (found == std::string::npos) {
				// nothing
				continue;
			}
			//ENET_INFO("header : key='" << std::string(element, 0, found) << "' value='" << std::string(element, found+2) << "'");
			m_receiveHeader.insert(make_pair(std::string(element, 0, found), std::string(element, found+2)));
		}
	}
	/*
	ENET_INFO("header : '" << header << "'");
	for (auto &it : m_receiveHeader) {
		ENET_INFO("header : key='" << it.first << "' value='" << it.second << "'");
	}
	*/
	// parse base answear:
	list = std::split(header, ' ');
	if (list.size() < 2) {
		ENET_ERROR("can not parse answear : " << list);
		return false;
	}
	int32_t ret = std::stoi(list[1]);
	switch (ret/100) {
		case 1:
			// information message
			return true;
			break;
		case 2:
			// OK
			return true;
			break;
		case 3:
			// Redirect
			ENET_WARNING("Rediret request");
			return false;
			break;
		case 4:
			// client Error
			ENET_WARNING("Client error");
			return false;
			break;
		case 5:
			// server error
			ENET_WARNING("Server error");
			return false;
			break;
	}
	return true;
}

bool enet::Http::post(const std::string& _address) {
	if (connect() == false) {
		return false;
	}
	
}

std::string enet::Http::dataString(void) {
	std::string data;
	for (auto element : m_receiveData) {
		if (element == '\0') {
			return data;
		}
		data += element;
	}
	return data;
}
