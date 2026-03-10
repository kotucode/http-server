#include "html_filler.h"

#include <chrono>

#include "inja.hpp"
#include "nlohmann/json.hpp"

namespace fs = std::filesystem;
namespace krono = std::chrono;

std::string HtmlFiller::GenerateDirList(const std::string& url_path,
                                        const std::filesystem::path& dir_path,
                                        const std::string& local_ip,
                                        const int& local_port) {
  // 模板
  static constexpr auto kDirListTemplate = R"(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Index of {{ title }}</title>
  <style>
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
      background: #fff;
      color: #333;
      padding: 20px;
      max-width: 800px;
      margin: 0 auto;
    }
    h1 { font-size: 1.5rem; margin-bottom: 1rem; }
    hr { border: none; border-top: 1px solid #ddd; margin: 1rem 0; }
    table { width: 100%; border-collapse: collapse; }
    th, td { padding: 8px 12px; text-align: left; }
    th { font-weight: 500; color: #666; border-bottom: 1px solid #ddd; }
    td { border-bottom: 1px solid #eee; }
    a { color: #0066cc; text-decoration: none; }
    a:hover { text-decoration: underline; }
    address {
      margin-top: 2rem;
      font-style: normal;
      color: #888;
      font-size: 0.875rem;
    }
  </style>
</head>
<body>
  <h1>Index of {{ title }}</h1><hr>
  <table>
    <thead>
      <tr>
        <th>Name</th>
        <th>Size</th>
        <th>Modified</th>
      </tr>
    </thead>
    <tbody>
      {% for entry in entries %}
        <tr>
          <td><a href="{{ entry.href }}">{{ entry.name }}</a></td>
          <td>{{ entry.size }}</td>
          <td>{{ entry.ftime }}</td>
        </tr>
      {% endfor %}
    </tbody>
  </table> 
  <address>
    Made with C++20/ 
    <a href="https://github.com/kotucode/http-server">http-server</a> 
    server running @ {{ ip }}:{{ port }}
  </address>
</body>
</html>)";

  // 准备 json 数据
  nlohmann::json data;
  data["ip"] = local_ip;
  data["port"] = local_port;
  data["title"] = url_path;
  data["entries"] = nlohmann::json::array();

  // 收集目录项信息
  for (auto&& entry : fs::directory_iterator(dir_path)) {
    auto name = entry.path().filename().string();

    auto file_time = entry.last_write_time();
    auto sys_time = krono::clock_cast<krono::system_clock>(file_time);
    auto zoned_time = krono::zoned_time(krono::current_zone(), sys_time);

    std::string size = "-";
    if (entry.is_directory()) {
      name += '/';
    } else if (entry.is_regular_file()) {
      static constexpr double GiB_lim = 1024L * 1024L * 1024L * 8L;
      static constexpr double MiB_lim = 1024L * 1024L * 8L;
      static constexpr double KiB_lim = 1024L * 8L;
      auto filesize = static_cast<double>(entry.file_size());
      auto suffix = "B";
      if (filesize > GiB_lim) {
        suffix = "GiB";
        filesize /= GiB_lim;
      } else if (filesize > MiB_lim) {
        suffix = "MiB";
        filesize /= MiB_lim;
      } else if (filesize > KiB_lim) {
        suffix = "KiB";
        filesize /= KiB_lim;
      }
      size = std::format("{:.2}{}", filesize, suffix);
    }

    data["entries"].push_back({{"href", url_path + name},
                               {"name", name},
                               {"size", size},
                               {"ftime", std::format("{:%F %R}", zoned_time)}});
  }

  std::ostringstream html;
  html << inja::render(kDirListTemplate, data);
  return html.str();
}
