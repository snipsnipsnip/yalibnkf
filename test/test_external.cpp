#include <stdio.h>
#include <sstream>
#include <vector>
#include "yalibnkf.h"
#include "nkf.h"

static std::string make_options(int argc, char **argv)
{
  std::ostringstream options;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      options << ' ' << argv[i];
    }
  }
  
  return options.str();
}

static std::vector<char> slurp()
{
  std::vector<char> buf;
  int c;
  
  while ((c = getchar()) != EOF) {
    buf.push_back(char(c));
  }
  
  return buf;
}

int main(int argc, char **argv)
{
  setbinmode(stdin);
  setbinmode(stdout);
  
  std::string options = make_options(argc, argv);
  std::vector<char> input = slurp();

  yalibnkf_str result = yalibnkf_convert(options.c_str(), &input[0], input.size());
  
  if (result.str == NULL) {
    fprintf(stderr, "<yalibnkf/test_main.c: failed to convert>\n");
    return 1;
  }
  
  for (size_t i = 0; i < result.len; i++) {
    putchar(result.str[i]);
  }
  
  yalibnkf_free(result);
  yalibnkf_quit();
  
  return 0;
}
