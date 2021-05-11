// Copyright 2020 Your Name <your_email>

#include <server.hpp>

server::server(){}

std::string server::update_js_vec(std::string str)
{
  auto result = nlohmann::json::object();
  nlohmann::json mass_suggestion;
  int n = 0;
  if (mut.try_lock())
  {
    mass_suggestion.clear();
    for (unsigned int i = 0; i < number_js; i++) {
      if (vec_sug[i].id == str)
      {
        result["text"] = vec_sug[i].name;
        result["position"] = n;
        mass_suggestion["suggestion"].push_back(result);
        n += 1;
      }
    }
    mut.unlock();
  }
  if (n == 0)
    return "{\"suggestion\": []}";
  else
    return mass_suggestion.dump();
}

void server::create_suggestion()
{
  for (;;)
  {
    if (mut.try_lock())
    {
      number_js = 0;
      vec_sug.clear();
      std::fstream fsin;
      fsin.open("suggestions.json");
      if (!fsin.is_open()) throw std::runtime_error("File error");
      std::stringstream str_file;
      str_file << fsin.rdbuf();
      std::string str = str_file.str();
      fsin.close();
      JS = json::parse(str);
      JSON j_str;
      for (json::const_iterator it = JS.at("items").cbegin();
           it != JS.at("items").cend(); ++it) {
        j_str.id = it->at("id").get<std::string>();
        j_str.name = it->at("name").get<std::string>();
        j_str.cost = it->at("cost").get<int>();
        vec_sug.push_back(j_str);
        number_js += 1;
      }
      for (unsigned int i = 0; i < number_js - 2; i++) {
        for (unsigned int j = i + 1; j < number_js - 1; j++) {
          if (vec_sug[i].cost > vec_sug[j].cost) {
            j_str = vec_sug[i];
            vec_sug[i] = vec_sug[j];
            vec_sug[j] = j_str;
          }
        }
      }
      mut.unlock();
      sleep(time_for_sleep);
    }
  }
}

void server::start()
{
  std::thread{&server::create_suggestion, this}.detach();
  const char adr[] = "127.0.0.1";
  auto const address = boost::asio::ip::make_address(adr);
  auto const port = static_cast<u_int16_t>(std::atoi("8080"));
  boost::asio::io_context ioc{1};
  tcp::acceptor acceptor{ioc, {address, port}};
  for (;;)
  {
    tcp::socket socket{ioc};
    acceptor.accept(socket);
    std::thread{std::bind(&server::create_session, this,
                          std::move(socket))}.detach();
  }
}

void server::create_session(tcp::socket& socket)
{
  bool close = false;
  beast::error_code ec;
  beast::flat_buffer buffer;
  send_lambda<tcp::socket> lambda{socket, close, ec};
  for (;;)
  {
    // Read a request
    http::request<http::string_body> req;
    http::read(socket, buffer, req, ec);
    if (ec == http::error::end_of_stream)
      break;
    if (ec)
      return fail(ec, "read");

    // Send the response
    handle_request(std::move(req), lambda);
    if (ec)
      return fail(ec, "write");
  }
  socket.shutdown(tcp::socket::shutdown_send, ec);
}

template<class Body, class Allocator, class Send>
void server::handle_request(
    http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send)
{
  std::string request_post = req.body();
  int k = request_post.find(":");
  k += 1;
  request_post = request_post.substr(k);
  auto const bad_request =
      [&req](beast::string_view why)
      {
        http::response<http::string_body> res{http::status::bad_request,
                                              req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
      };

//  // Make sure we can handle the method
  if (req.method() != http::verb::post && req.method() != http::verb::head &&
      req.target() == rootDirectory)
    return send(bad_request("Unknown HTTP-method"));

  http::string_body::value_type body;
  body.append(update_js_vec(request_post)+"\n");

  auto const size = body.size();
  http::response<http::string_body> res
      {
          std::piecewise_construct,
          std::make_tuple(std::move(body)),
          std::make_tuple(http::status::ok, req.version())
      };
  res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  res.content_length(size);
  res.keep_alive(req.keep_alive());
  return send(std::move(res));
}

void server::fail(beast::error_code ec, char const* what)
{
  std::cerr << what << ": " << ec.message() << std::endl;
}
server::~server(){}
