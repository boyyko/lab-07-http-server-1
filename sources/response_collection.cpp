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

struct Entry {
  std::string suggestion;
  size_t weight;
};

bool operator<(const Entry& lhs, const Entry& rhs) {
  return lhs.weight < rhs.weight;
}

std::vector<Entry> findMatches(const std::string& req_str,
                               const nlohmann::json& json) {
  std::vector<Entry> matches;

  for (auto& x : json) {
    if (x["id"] == req_str)
      matches.push_back({x["name"], x["cost"]});
  }

  std::sort(matches.begin(), matches.end());
  return matches;
}

std::string ResponseCollection::getResponse(std::string_view req_json_str) {
  nlohmann::json request{};
  nlohmann::json response{};
  std::string req_str{};
  try {
    request = nlohmann::json::parse(req_json_str);
    req_str = request["input"];
  } catch (...) {
    response = nlohmann::json::parse(R"({ "status": "ill-formated request" })");
    return response.dump();
  }

  m_mutex.lock_shared();
  std::vector<Entry> suggestions = findMatches(req_str, m_json);
  m_mutex.unlock_shared();

  response = nlohmann::json::parse(R"({ "suggestions": [] })");
  for (size_t i = 0; i < suggestions.size(); ++i) {
    nlohmann::json entry =
        nlohmann::json::parse(R"({ "text": "hello", "position": 0 })");
    entry["text"] = std::move(suggestions[i].suggestion);
    entry["position"] = i;
    response["suggestions"].push_back(entry);
  }

  return response.dump();
}

void ResponseCollection::setSourceFile(std::string newFileName) {
  m_mutex.lock_shared();
  m_filename = std::move(newFileName);
  m_mutex.unlock_shared();
}
