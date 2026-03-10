#ifndef HTTP_SERVER_STATIC_SERVER_H_
#define HTTP_SERVER_STATIC_SERVER_H_

#include <string>

#include "httplib.h"

class StaticServer {
 public:
  using HandlerResponse = httplib::Server::HandlerResponse;
  using Request = httplib::Request;
  using Response = httplib::Response;

 public:
  StaticServer() {}
  StaticServer(std::string ip, int port, std::string dir)
      : ip_(ip), port_(port), base_dir_(dir) {}

  void Run() {
    svr_.set_mount_point("/", base_dir_);
    svr_.set_pre_routing_handler(std::bind(&StaticServer::pre_handle, this,
                                           std::placeholders::_1,
                                           std::placeholders::_2));
    svr_.listen(ip_, port_);
  }

 private:
  HandlerResponse pre_handle(const Request&, Response&);

 private:
  std::string ip_{"0.0.0.0"};
  int port_{35565};
  std::string base_dir_{"."};
  httplib::Server svr_;
};

#endif  // !HTTP_SERVER_STATIC_SERVER_H_
