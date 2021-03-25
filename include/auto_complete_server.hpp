// Copyright 2020 LabradorWithShades
// 70892548+LabradorWithShades@users.noreply.github.com

#ifndef INCLUDE_AUTO_COMPLETE_SERVER_HPP_
#define INCLUDE_AUTO_COMPLETE_SERVER_HPP_

#include <boost/beast/core.hpp>
#include <string>
#include <vector>

class AutoCompleteServer {
 public:
  AutoCompleteServer(std::string_view rootDirectory,
                     boost::asio::ip::address address,
                     unsigned short port);
  ~AutoCompleteServer();

  [[noreturn]] void startServer();

 private:
  const std::string m_rootDirectory;
  boost::asio::ip::address m_address;
  unsigned short m_port;
};

#endif  // INCLUDE_AUTO_COMPLETE_SERVER_HPP_
