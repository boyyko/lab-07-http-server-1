// Copyright 2020 LabradorWithShades
// 70892548+LabradorWithShades@users.noreply.github.com

#include <response_collection.hpp>

#include <fstream>
#include <iostream>

ResponseCollection& ResponseCollection::getInstance() {
  static ResponseCollection instance("database.json");
  return instance;
}

ResponseCollection::ResponseCollection(std::string filename)
  : m_json()
  , m_filename(std::move(filename))
  , m_mutex() {
  updateCollection();
}

void ResponseCollection::updateCollection() {
  std::ifstream input(m_filename);
  if (!input.is_open())
    throw std::runtime_error("Database file could not be opened");

  m_mutex.lock();
  input >> m_json;
  m_mutex.unlock();

  input.close();
}

std::string ResponseCollection::getResponse(std::string_view req_json_str) {
  nlohmann::json request{};
  nlohmann::json response = nlohmann::json::array();
  std::string req_str{};
  try {
    request = nlohmann::json::parse(req_json_str);
    req_str = request["input"];
  } catch (...) {
    response.push_back(R"({ "status": "ill-formated request" })");
    return response.dump();
  }
  std::cout << req_str << std::endl;
  m_mutex.lock_shared();
  //TODO: add actual response generation
  m_mutex.unlock_shared();
  return response.dump();
}

void ResponseCollection::setSourceFile(std::string newFileName) {
  m_mutex.lock_shared();
  m_filename = std::move(newFileName);
  m_mutex.unlock_shared();
}
