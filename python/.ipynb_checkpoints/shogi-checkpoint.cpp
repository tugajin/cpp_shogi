#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char* argv[]) {
  vector<string> vec;
  for(auto i = 0; i < argc; i++) {
    const auto v = string(argv[i]);
    vec.push_back(v);
  }
  if(vec[1] == "think") {
    cout<<"G*5b:0.273 7f6f:0.872 5d2d:0.254";
  } else if(vec[1] == "init") {
    cout<<"lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1";
  }

  return 0;
}
