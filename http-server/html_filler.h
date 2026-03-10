#ifndef HTTP_SERVER_HTML_FILLER_H_
#define HTTP_SERVER_HTML_FILLER_H_

#include <filesystem>
#include <string>

class HtmlFiller {
 public:
  static std::string GenerateDirList(const std::string& url_path,
                                     const std::filesystem::path& dir_path,
                                     const std::string& local_ip,
                                     const int& local_port);
};

#endif  // !HTTP_SERVER_HTML_FILLER_H_
