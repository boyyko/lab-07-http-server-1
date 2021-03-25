// Copyright 2020 LabradorWithShades
// 70892548+LabradorWithShades@users.noreply.github.com

#ifndef INCLUDE_RESPONSE_COLLECTION_HPP_
#define INCLUDE_RESPONSE_COLLECTION_HPP_

#include <nlohmann/json.hpp>

#include <shared_mutex>

class ResponseCollection {
 public:
  ResponseCollection(const ResponseCollection&) = delete;
  ResponseCollection(ResponseCollection&&) = delete;
  ResponseCollection& operator=(const ResponseCollection&) = delete;
  ResponseCollection& operator=(ResponseCollection&&) = delete;

  static ResponseCollection& getInstance();

  void setSourceFile(std::string newFileName);
  void updateCollection();
  std::string getResponse(std::string_view entry);
 private:
  explicit ResponseCollection(std::string filename);

  nlohmann::json m_json;
  std::string m_filename;
  mutable std::shared_mutex m_mutex;
};

#endif  // INCLUDE_RESPONSE_COLLECTION_HPP_
