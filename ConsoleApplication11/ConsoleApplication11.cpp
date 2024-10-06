
/* код сервера1, тут отправлюятсю сообщения только активных потоков,остальные клиенты могут только получать*/
//#undef UNICODE
//#define WIN32_LEAN_AND_MEAN
//
//#include <windows.h>
//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <thread>
//#include <vector>
//#include <mutex>
//#include <iostream>
//#include <condition_variable>
//
//#pragma comment(lib, "Ws2_32.lib")
//
//#define DEFAULT_BUFLEN 512
//#define DEFAULT_PORT "27015"
//#define MAX_THREADS 3
//
//std::vector<SOCKET> clients;
//std::mutex mtx;
//std::condition_variable cv;
//int numThreads = 0;
//
//void HandleClient(SOCKET ClientSocket) {
//   char recvbuf[DEFAULT_BUFLEN];
//   int iResult;
//
//   {
//      std::lock_guard<std::mutex> lock(mtx);
//      clients.push_back(ClientSocket);
//   }
//
//   do {
//      iResult = recv(ClientSocket, recvbuf, DEFAULT_BUFLEN, 0);
//      // std::cout << "Received message: " << recvbuf << std::endl;
//      if (iResult > 0) {
//         {
//            std::lock_guard<std::mutex> lock(mtx);
//            for (auto& client : clients) {
//               if (client != ClientSocket) {
//                  send(client, recvbuf, iResult, 0);
//               }
//            }
//         }
//      }
//      else if (iResult == 0) {
//         closesocket(ClientSocket);
//      }
//      else {
//         std::cout << "recv failed with error: " << WSAGetLastError() << std::endl;
//         closesocket(ClientSocket);
//      }
//   } while (iResult > 0);
//
//   {
//      std::lock_guard<std::mutex> lock(mtx);
//      clients.erase(std::remove(clients.begin(), clients.end(), ClientSocket), clients.end());
//   }
//
//   {
//      std::unique_lock<std::mutex> lock(mtx);
//      numThreads--;
//      cv.notify_one();
//   }
//}
//
//int main() {
//   WSADATA wsaData;
//   int iResult;
//
//   SOCKET ListenSocket = INVALID_SOCKET;
//
//   struct addrinfo* result = NULL;
//   struct addrinfo hints;
//
//   // Initialize Winsock
//   iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//
//   ZeroMemory(&hints, sizeof(hints));
//   hints.ai_family = AF_INET;
//   hints.ai_socktype = SOCK_STREAM;
//   hints.ai_protocol = IPPROTO_TCP;
//   hints.ai_flags = AI_PASSIVE;
//
//   // Resolve the server address and port
//   iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
//
//   // Create a SOCKET for the server to listen for client connections
//   ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
//
//   iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
//
//   freeaddrinfo(result);
//
//   iResult = listen(ListenSocket, SOMAXCONN);
//
//   while (true) {
//      SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
//
//      std::unique_lock<std::mutex> lock(mtx);
//      cv.wait(lock, [] { return numThreads < MAX_THREADS; });
//
//      numThreads++;
//      std::thread clientThread(HandleClient, ClientSocket);
//      clientThread.detach();
//   }
//
//   closesocket(ListenSocket);
//   WSACleanup();
//
//   return 0;
//}










/*код сервера 2 с использованием пулла потоков*/

//#undef UNICODE
//#define WIN32_LEAN_AND_MEAN
//
//#include <windows.h>
//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <thread>
//#include <vector>
//#include <mutex>
//#include <condition_variable>
//#include <queue>
//#include <iostream>
//
//#pragma comment(lib, "Ws2_32.lib")
//
//#define DEFAULT_BUFLEN 512
//#define DEFAULT_PORT "27015"
//#define MAX_THREADS 10
//
//struct ClientMessage {
//   SOCKET clientSocket;
//   char message[DEFAULT_BUFLEN];
//   int messageLength;
//};
//
//std::vector<SOCKET> clients;
//std::queue<ClientMessage> messageQueue;
//std::mutex mtx;
//std::condition_variable cv;
//bool running = true;
//
//void WorkerThread() {
//   while (running) {
//      ClientMessage msg;
//
//      {
//         std::unique_lock<std::mutex> lock(mtx);
//         cv.wait(lock, [] { return !messageQueue.empty() || !running; });
//
//         if (!running) break;
//
//         msg = messageQueue.front();
//         messageQueue.pop();
//      }
//
//      // Отправка сообщения всем остальным клиентам
//      for (auto& client : clients) {
//         if (client != msg.clientSocket) {
//            send(client, msg.message, msg.messageLength, 0);
//         }
//      }
//   }
//}
//
//void HandleClient(SOCKET ClientSocket) {
//   char recvbuf[DEFAULT_BUFLEN];
//   int iResult;
//
//   do {
//      // Получение сообщения от клиента
//      iResult = recv(ClientSocket, recvbuf, DEFAULT_BUFLEN, 0);
//      if (iResult > 0) {
//         std::cout << "Received message: " << recvbuf << std::endl;
//
//         // Добавление сообщения в очередь
//         {
//            std::lock_guard<std::mutex> lock(mtx);
//            messageQueue.push({ ClientSocket, {}, iResult });
//            memcpy(messageQueue.back().message, recvbuf, iResult);
//         }
//         cv.notify_one(); // Уведомление потока-работника
//      }
//      else if (iResult == 0) {
//         std::cout << "Connection closed." << std::endl;
//         closesocket(ClientSocket);
//      }
//      else {
//         std::cout << "recv failed with error: " << WSAGetLastError() << std::endl;
//         closesocket(ClientSocket);
//      }
//   } while (iResult > 0);
//
//   // Удаление клиента из списка
//   {
//      std::lock_guard<std::mutex> lock(mtx);
//      clients.erase(std::remove(clients.begin(), clients.end(), ClientSocket), clients.end());
//   }
//}
//
//int main() {
//   WSADATA wsaData;
//   int iResult;
//
//   SOCKET ListenSocket = INVALID_SOCKET;
//
//   struct addrinfo* result = NULL;
//   struct addrinfo hints;
//
//   // Инициализация Winsock
//   iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//
//   ZeroMemory(&hints, sizeof(hints));
//   hints.ai_family = AF_INET;
//   hints.ai_socktype = SOCK_STREAM;
//   hints.ai_protocol = IPPROTO_TCP;
//   hints.ai_flags = AI_PASSIVE;
//
//   // Разрешение адреса сервера и порта
//   iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
//
//   // Создание сокета для сервера
//   ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
//
//   iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
//   freeaddrinfo(result);
//
//   iResult = listen(ListenSocket, SOMAXCONN);
//
//   // Запуск пула потоков
//   std::vector<std::thread> threadPool;
//   for (int i = 0; i < MAX_THREADS; ++i) {
//      threadPool.emplace_back(WorkerThread);
//   }
//
//   while (running) {
//      SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
//
//      if (ClientSocket != INVALID_SOCKET) {
//         std::lock_guard<std::mutex> lock(mtx);
//         clients.push_back(ClientSocket);
//
//         // Создание потока для обработки клиента
//         std::thread(HandleClient, ClientSocket).detach();
//      }
//
//      // Задержка для предотвращения высоких нагрузок на процессор при отсутствии клиентов        
//      std::this_thread::sleep_for(std::chrono::milliseconds(100));
//   }
//
//   // Завершение работы потоков
//   {
//      std::lock_guard<std::mutex> lock(mtx);
//      running = false;
//   }
//   cv.notify_all(); // Уведомление всех потоков о завершении
//
//   for (auto& thread : threadPool) {
//      thread.join();
//   }
//
//   // Очистка ресурсов
//   closesocket(ListenSocket);
//   WSACleanup();
//
//   return 0;
//}





/* код клиента */
//#define WIN32_LEAN_AND_MEAN
//
//#include <windows.h>
//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <iostream>
//#include <string>
//#include <thread>
//#include <mutex>
//
//// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
//#pragma comment (lib, "Ws2_32.lib")
//#pragma comment (lib, "Mswsock.lib")
//#pragma comment (lib, "AdvApi32.lib")
//
//#define DEFAULT_BUFLEN 512
//#define DEFAULT_PORT "27015"
//
//std::mutex mtx;
//
//void sendMessage(SOCKET client) {
//   int iResult;
//
//   while (true) {
//      std::string str;
//      std::getline(std::cin, str); // Use getline to allow spaces in messages
//      iResult = send(client, str.c_str(), (int)str.length(), 0);
//      if (iResult == SOCKET_ERROR) {
//         std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
//         break;
//      }
//   }
//}
//
//void receiveMessage(SOCKET client) {
//   char recvbuf[DEFAULT_BUFLEN];
//   int iResult;
//
//   while (true) {
//      iResult = recv(client, recvbuf, DEFAULT_BUFLEN - 1, 0);
//      if (iResult > 0) {
//         recvbuf[iResult] = '\0'; // Null-terminate the received string
//         std::lock_guard<std::mutex> lock(mtx);
//         std::cout << "Received: " << recvbuf << std::endl;
//      }
//      else if (iResult == 0) {
//         std::cout << "Connection closed" << std::endl;
//         break;
//      }
//      else {
//         std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
//         break;
//      }
//   }
//}
//
//int __cdecl main(int argc, char** argv) {
//   WSADATA wsaData;
//   SOCKET ConnectSocket = INVALID_SOCKET;
//   struct addrinfo* result = NULL,
//      * ptr = NULL,
//      hints;
//   const char* sendbuf = "this is a test";
//   int iResult;
//
//   // Initialize Winsock
//   iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//   if (iResult != 0) {
//      printf("WSAStartup failed with error: %d\n", iResult);
//      return 1;
//   }
//
//   ZeroMemory(&hints, sizeof(hints));
//   hints.ai_family = AF_UNSPEC;
//   hints.ai_socktype = SOCK_STREAM;
//   hints.ai_protocol = IPPROTO_TCP;
//
//   
//
//   iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
//   if (iResult != 0) {
//      printf("getaddrinfo failed with error: %d\n", iResult);
//      WSACleanup();
//      return 1;
//   }
//
//   // Attempt to connect to an address until one succeeds
//   for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
//      // Create a SOCKET for connecting to server
//      ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
//         ptr->ai_protocol);
//      if (ConnectSocket == INVALID_SOCKET) {
//         printf("socket failed with error: %ld\n", WSAGetLastError());
//         WSACleanup();
//         return 1;
//      }
//
//      // Connect to server.
//      iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
//      if (iResult == SOCKET_ERROR) {
//         closesocket(ConnectSocket);
//         ConnectSocket = INVALID_SOCKET;
//         continue;
//      }
//      break;
//   }
//
//   freeaddrinfo(result);
//
//   if (ConnectSocket == INVALID_SOCKET) {
//      printf("Unable to connect to server!\n");
//      WSACleanup();
//      return 1;
//   }
//
//   // Start the send and receive threads
//   std::thread send_thr(sendMessage, ConnectSocket);
//   std::thread recv_thr(receiveMessage, ConnectSocket);
//
//   // Join threads to the main thread
//   send_thr.join();
//   recv_thr.join();
//
//   // Cleanup
//   closesocket(ConnectSocket);
//   WSACleanup();
//
//   return 0;
//}
