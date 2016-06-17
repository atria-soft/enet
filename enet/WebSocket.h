/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <enet/Http.h>
#include <ememory/memory.h>
#include <vector>
#include <map>

namespace enet {
	class WebSocket {
		private:
			ememory::SharedPtr<enet::Http> m_interface;
		public:
			WebSocket(enet::Tcp _connection, bool _isServer=false);
			virtual ~WebSocket();
			void start(const std::string& _uri="");
			void stop(bool _inThread=false);
			bool isAlive() {
				return m_interface->isAlive();
			}
			void onReceiveData(std::vector<uint8_t>& _data);
			void onReceiveRequest(const enet::HttpRequest& _data);
			void onReceiveAnswer(const enet::HttpAnswer& _data);
		public:
			using Observer = std::function<void(std::vector<uint8_t>&)>; //!< Define an Observer: function pointer
		private:
			Observer m_observer;
		public:
			/**
			 * @brief Connect an function member on the signal with the shared_ptr object.
			 * @param[in] _class shared_ptr Object on whe we need to call ==> the object is get in keeped in weak_ptr.
			 * @param[in] _func Function to call.
			 * @param[in] _args Argument optinnal the user want to add.
			 */
			template<class CLASS_TYPE>
			void connect(CLASS_TYPE* _class, void (CLASS_TYPE::*_func)(std::vector<uint8_t>&)) {
				m_observer = [=](std::vector<uint8_t>& _value){
					(*_class.*_func)(_value);
				};
			}
			void connect(Observer _func) {
				m_observer = _func;
			}
		public:
			/**
			 * @brief Write a chunk of data on the socket
			 * @param[in] _data pointer on the data might be write
			 * @param[in] _len Size that must be written socket
			 * @return >0 byte size on the socket write
			 * @return -1 an error occured.
			 */
			int32_t write(const void* _data, int32_t _len);
			/**
			 * @brief Write a chunk of data on the socket
			 * @param[in] _data String to rite on the soccket
			 * @param[in] _writeBackSlashZero if false, the \0 is not write
			 * @return >0 byte size on the socket write
			 * @return -1 an error occured.
			 */
			int32_t write(const std::string& _data, bool _writeBackSlashZero = true) {
				if (_data.size() == 0) {
					return 0;
				}
				if (_writeBackSlashZero == true) {
					return write(_data.c_str(), _data.size()+1);
				}
				return write(_data.c_str(), _data.size());
			}
			/**
			 * @brief Write a chunk of data on the socket
			 * @param[in] _data String to rite on the soccket
			 * @param[in] _writeBackSlashZero if false, the \0 is not write
			 * @return >0 T element write on the socket
			 * @return -1 an error occured.
			 */
			template <class T>
			int32_t write(const std::vector<T>& _data) {
				if (_data.size() == 0) {
					return 0;
				}
				size_t ret = write(&_data[0], _data.size()*sizeof(T));
				if (ret <=0) {
					return ret;
				}
				return ret/sizeof(T);
			}
	};
}
