#include "yahttp.hpp"
#include "router.hpp"

namespace YaHTTP {
  typedef funcptr::tuple<int,int> TDelim;

  // router is defined here.
  Router Router::router;

  void Router::map(const std::string& method, const std::string& url, THandlerFunction handler, const std::string& name) {
    std::string method2 = method;
    bool isopen=false;
    // add into vector
    for(std::string::const_iterator i = url.begin(); i != url.end(); i++) {
       if (*i == '<' && isopen) throw std::runtime_error("Invalid URL mask, cannot have < after <");
       if (*i == '<') isopen = true;
       if (*i == '>' && !isopen) throw std::runtime_error("Invalid URL mask, cannot have > without < first");
       if (*i == '>') isopen = false;
    }
    std::transform(method2.begin(), method2.end(), method2.begin(), ::toupper); 
    routes.push_back(funcptr::make_tuple(method2, url, handler, name));
  };

  void Router::route(Request *req, Response *resp) {
    std::map<std::string, TDelim> params;
    int pos1,pos2;
    std::string pname;
    bool matched = false;
    THandlerFunction handler;

    // iterate routes
    for(TRouteList::iterator i = routes.begin(); !matched && i != routes.end(); i++) {
      int pos1,pos2,k1,k2,k3;
      std::string pname;
      std::string method, url;
      std::tie(method, url, handler, std::ignore) = *i;
    
      if (method.empty() == false && req->method != method) continue; // no match on method
      // see if we can't match the url
      params.clear();
      // simple matcher func
      for(k1=0, k2=0; k1 < url.size() && k2 < req->url.path.size(); ) {
        if (url[k1] == '<') {
          pos1 = k2;
          k3 = k1+1;
          // start of parameter
          while(k1 < url.size() && url[k1] != '>') k1++;
          pname = std::string(url.begin()+k3, url.begin()+k1);
          // then we also look it on the url
          if (pname[0]=='*') {
            pname = pname.substr(1);
            // this matches whatever comes after it, basically end of string
            pos2 = req->url.path.size();
            matched = true;
            params[pname] = std::tie(pos1,pos2);
            k1 = url.size();
            k2 = req->url.path.size();
            break;
          } else { 
            // match until url[k1]
            while(k2 < req->url.path.size() && req->url.path[k2] != url[k1+1]) k2++;
            pos2 = k2;
            params[pname] = std::tie(pos1,pos2);
          }
          k2--;
        }
        else if (url[k1] == '*') {
          matched = true;
          break;
        }
        else if (url[k1] != req->url.path[k2]) {
          break;
        }

        k1++; k2++;
      }

      // ensure.
      if (url[k1] != req->url.path[k2]) 
        matched = false;
      else
        matched = true;
    }

    if (!matched) { return; } // no route
    req->params.clear();    

    for(std::map<std::string, TDelim>::iterator i = params.begin(); i != params.end(); i++) {
      int p1,p2;
      std::tie(p1,p2) = i->second;
      req->params[i->first] = std::string(req->url.path.begin() + p1, req->url.path.begin() + p2);
    }

    handler(req,resp);
  };

  void Router::printRoutes(std::ostream &os) {
    for(TRouteList::iterator i = routes.begin(); i != routes.end(); i++) {
      os << std::get<0>(*i) << "    " << std::get<1>(*i) << "    " << std::get<3>(*i) << std::endl;
    } 
  };

  std::pair<std::string,std::string> Router::urlFor(const std::string &name, const strstr_map_t& arguments) {
    std::ostringstream path;
    std::string mask,method,result;
    int k1,k2,k3;

    bool found = false;
    for(TRouteList::iterator i = routes.begin(); !found && i != routes.end(); i++) {
      if (std::get<3>(*i) == name) { mask = std::get<1>(*i); method = std::get<0>(*i); found = true; }
    }

    if (!found)
      throw std::runtime_error("Route not found");

    for(k1=0,k3=0;k1<mask.size();k1++) {
      if (mask[k1] == '<') {
        std::string pname;
        strstr_map_t::const_iterator pptr;
        k2=k1;
        while(k1<mask.size() && mask[k1]!='>') k1++;
        path << mask.substr(k3,k2-k3);
        if (mask[k2+1] == '*')
          pname = std::string(mask.begin() + k2 + 2, mask.begin() + k1);
        else 
          pname = std::string(mask.begin() + k2 + 1, mask.begin() + k1);
        if ((pptr = arguments.find(pname)) != arguments.end()) 
          path << pptr->second;
        k3 = k1+1;
      }
      else if (mask[k1] == '*') {
        // ready 
        k3++;
        continue;
      }
    }
    std::cout << mask.substr(k3) << std::endl;
    path << mask.substr(k3);
    result = path.str();
    return std::make_pair(method, result);
  }
};
