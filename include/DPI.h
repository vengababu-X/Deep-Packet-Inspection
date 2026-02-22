#ifndef DPI_H
#define DPI_H

#include <string>

struct DPIContext {
    std::string app_name;
    std::string protocol;
};

DPIContext analyze_flow(const std::string& domain, int sport, int dport);
std::string extract_tls_sni(const unsigned char* data, int len);
std::string detect_app(const std::string& domain);

#endif
