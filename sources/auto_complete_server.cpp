// Copyright 2020 LabradorWithShades
// 70892548+LabradorWithShades@users.noreply.github.com

#include <auto_complete_server.hpp>

#include <response_collection.hpp>

#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <utility>
#include <thread>

template<bool isRequest, class Body, class Fields>
void sendMessage(boost::asio::ip::tcp::socket& socket,
                 boost::beast::http::message<isRequest, Body, Fields>&& msg) {
  boost::beast::http::serializer<isRequest, Body, Fields> sr{msg};
  boost::beast::http::write(socket, sr);
}

bool operator!=(const boost::string_view& lhs, const std::string_view& rhs) {
  if (lhs.size() != rhs.size())
    return true;

  for (size_t i = 0; i < rhs.size(); ++i)
    if (lhs[i] != rhs[i])
      return true;

  return false;
}

void workerFunction(boost::asio::ip::tcp::socket&& socket,
                    std::string_view rootDirectory) {
  boost::beast::error_code ec;
  boost::beast::flat_buffer buffer;

  for (;;) {
    boost::beast::http::request<boost::beast::http::string_body> req;
    boost::beast::http::read(socket, buffer, req, ec);

    if(ec == boost::beast::http::error::end_of_stream)
      break;

    if ((req.method() != boost::beast::http::verb::post) ||
        (req.target() != rootDirectory) ||
        (ec)) {
      boost::beast::http::response<boost::beast::http::string_body> res{
          boost::beast::http::status::internal_server_error, req.version()};
      res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
      res.set(boost::beast::http::field::content_type, "text/html");
      res.keep_alive(req.keep_alive());
      res.body() = "Fatal error! Incorrect request!";
      res.prepare_payload();
      sendMessage(socket, std::move(res));
      break;
    } else {
      boost::beast::http::response<boost::beast::http::string_body> res{
          boost::beast::http::status::ok, req.version()};
      res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
      res.set(boost::beast::http::field::content_type, "application/json");
      res.keep_alive(req.keep_alive());
      res.body() = ResponseCollection::getInstance().getResponse(req.body());
      res.prepare_payload();
      sendMessage(socket, std::move(res));
    }

    if (ec)
      break;
  }

  socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
}

AutoCompleteServer::AutoCompleteServer(std::string_view rootDirectory,
                                       boost::asio::ip::address address,
                                       unsigned short port)
  : m_rootDirectory(rootDirectory)
  , m_address(std::move(address))
  , m_port(port)
{}

AutoCompleteServer::~AutoCompleteServer() = default;

[[noreturn]] void AutoCompleteServer::startServer() {
  boost::asio::io_context ioc{1};
  boost::asio::ip::tcp::acceptor acceptor{ioc, {m_address, m_port}};

  for(;;)
  {
    boost::asio::ip::tcp::socket socket{ioc};
    acceptor.accept(socket);
    std::thread{workerFunction, std::move(socket), m_rootDirectory}.detach();
  }
}
