#include <auto_complete_server.hpp>
#include <response_collection.hpp>

#include <iostream>
#include <thread>

void collectionUpdater() {
  ResponseCollection::getInstance().updateCollection();
  std::this_thread::sleep_for(std::chrono::minutes(15));
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "Error! Usage:" << std::endl
              << "    demo <request_path> <ip address> <port>" << std::endl
              << "Example:" << std::endl
              << "    demo /v1/api/suggest 0.0.0.0 3000" << std::endl;
    return 1;
  }

  uint16_t port = static_cast<uint16_t>(std::stoi(argv[3]));

  AutoCompleteServer server(
      argv[1], boost::asio::ip::make_address(argv[2]), port);
  std::thread{collectionUpdater}.detach();
  server.startServer();
}