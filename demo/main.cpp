#include <auto_complete_server.hpp>
#include <response_collection.hpp>

#include <thread>

void collectionUpdater() {
  ResponseCollection::getInstance().updateCollection();
  std::this_thread::sleep_for(std::chrono::minutes(15));
}

int main() {
  AutoCompleteServer server(
      "/v1/api/suggest", boost::asio::ip::make_address("0.0.0.0"), 3000);
  std::thread{collectionUpdater}.detach();
  server.startServer();
}