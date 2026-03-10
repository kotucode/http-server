#include "static_server.h"

#include <filesystem>
#include <fstream>

#include "inja.hpp"
#include "nlohmann/json.hpp"

namespace fs = std::filesystem;

static std::string DIR_LIST_TEMPLATE = R"(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Index of {{ title }}</title>
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
</body>
</html>)";

// 生成目录列表的HTML页面
std::string generate_directory_listing(const std::string& url_path,
                                       const fs::path& dir_path) {
  // 准备 json 数据
  nlohmann::json data;
  data["title"] = url_path;
  data["entries"] = nlohmann::json::array();

  // 收集目录项信息
  for (auto&& entry : fs::directory_iterator(dir_path)) {
    auto name = entry.path().filename().string();
    std::string size = "-";

    std::ostringstream ftime;
    ftime << std::chrono::clock_cast<std::chrono::system_clock>( entry.last_write_time());

    if (entry.is_directory()) {
      name += '/';
    } else if (entry.is_regular_file()) {
      size = std::to_string(entry.file_size());
    }

    data["entries"].push_back({{"href", url_path + name},
                               {"name", name},
                               {"size", size},
                               {"ftime", ftime.str()}});
  }

  std::ostringstream html;
  html << inja::render(DIR_LIST_TEMPLATE, data);
  return html.str();
}

StaticServer::HandlerResponse StaticServer::pre_handle(
    const StaticServer::Request& req, StaticServer::Response& res) {
  if (req.method != "GET" && req.method != "HEAD") {
    return HandlerResponse::Unhandled;
  }

  // 获取路径参数
  std::string url_path = req.path;
  url_path = httplib::decode_uri(url_path);
  fs::path full_path = fs::path(base_dir_) / url_path.substr(1);

  // 检查路径是否有效
  std::error_code ec;
  fs::path canonical_path = fs::canonical(full_path, ec);
  if (ec) {
    return HandlerResponse::Unhandled;
  }
  fs::path canonical_base = fs::canonical(base_dir_, ec);
  if (ec) {
    return HandlerResponse::Unhandled;
  }

  // 确保请求路径属于基础目录
  std::string canonical_str = canonical_path.string();
  std::string base_str = canonical_base.string();
  if (canonical_str.find(base_str) != 0) {
    return HandlerResponse::Unhandled;
  }

  // 检查是否是目录
  if (!fs::is_directory(full_path)) {
    return HandlerResponse::Unhandled;
  }

  // 路径若不以 / 结尾，重定向到 / 结尾
  if (url_path.back() != '/') {
    res.set_redirect(url_path + "/", httplib::StatusCode::MovedPermanently_301);
    return HandlerResponse::Handled;
  }

  // 检查是否存在 index.html，存在则默认处理
  fs::path index_path = full_path / "index.html";
  if (fs::exists(index_path) && fs::is_regular_file(index_path)) {
    return HandlerResponse::Unhandled;
  }

  // 生成目录列表
  res.set_content(generate_directory_listing(url_path, full_path),
                  "text/html; charset=utf-8");
  return HandlerResponse::Handled;
}
