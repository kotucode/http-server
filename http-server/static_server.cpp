#include "static_server.h"

#include <filesystem>
#include <fstream>

#include "html_filler.h"

namespace fs = std::filesystem;

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
  res.set_content(HtmlFiller::GenerateDirList(url_path, full_path,
                                              req.local_addr, req.local_port),
                  "text/html; charset=utf-8");
  return HandlerResponse::Handled;
}
