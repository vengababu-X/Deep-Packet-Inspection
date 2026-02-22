#include "DPI.h"
#include <algorithm>

DPIContext analyze_flow(const std::string& domain, int sport, int dport) {
    DPIContext ctx;
    ctx.app_name = detect_app(domain);
    
    // Determine protocol based on port and domain
    if (dport == 443 || dport == 80 || !domain.empty()) {
        ctx.protocol = "HTTP/HTTPS";
    } else if (dport == 53 || sport == 53) {
        ctx.protocol = "DNS";
    } else if (dport == 22 || sport == 22) {
        ctx.protocol = "SSH";
    } else {
        ctx.protocol = "Unknown";
    }
    
    return ctx;
}

std::string detect_app(const std::string& domain) {
    std::string lower_domain = domain;
    std::transform(lower_domain.begin(), lower_domain.end(), 
                   lower_domain.begin(), ::tolower);

    // Simple application detection based on SNI domain
    if (lower_domain.find("google") != std::string::npos) return "Google";
    if (lower_domain.find("youtube") != std::string::npos) return "YouTube";
    if (lower_domain.find("facebook") != std::string::npos) return "Facebook";
    if (lower_domain.find("instagram") != std::string::npos) return "Instagram";
    if (lower_domain.find("twitter") != std::string::npos) return "Twitter";
    if (lower_domain.find("github") != std::string::npos) return "GitHub";
    if (lower_domain.find("github.com") != std::string::npos) return "GitHub";
    if (lower_domain.find("cloudflare") != std::string::npos) return "Cloudflare";
    if (lower_domain.find("amazon") != std::string::npos) return "Amazon";
    if (lower_domain.find("netflix") != std::string::npos) return "Netflix";
    if (lower_domain.find("discord") != std::string::npos) return "Discord";

    return "";
}

std::string extract_tls_sni(const unsigned char* data, int len) {
    if (!data || len < 5) return "";

    // Look for TLS handshake with SNI extension
    // This is a simplified extraction - real TLS parsing is more complex
    for (int i = 0; i < len - 5; i++) {
        // Look for common SNI patterns (simplified)
        if (data[i] == 0x00 && data[i+1] == 0x00) {
            int length = data[i+2];
            if (length > 0 && length < 256 && i + 3 + length <= len) {
                std::string potential_sni((const char*)&data[i+3], length);
                // Basic validation: contains only printable ASCII
                bool valid = true;
                for (char c : potential_sni) {
                    if (c < 32 || c > 126) {
                        valid = false;
                        break;
                    }
                }
                if (valid && potential_sni.find('.') != std::string::npos) {
                    return potential_sni;
                }
            }
        }
    }
    return "";
}
