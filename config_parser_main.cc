// Usage: ./config_parser <path to config file>

#include "config_parser.h"

int main(int argc, char* argv[]) {
  if (argc != 2) 
  {
    std::cerr << "Usage: ./config_parser <path to config file>\n";
    return 1;
  }

  NginxConfigParser config_parser;
  NginxConfig config;
  config_parser.Parse(argv[1], &config);

  return 0;
}
