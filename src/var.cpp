#include <map>
#include "libmy.hpp"
#include "var.hpp"
namespace var {

bool Ponder;
int32 Threads;
int32 Hash;

static std::map<std::string, std::string> GVar;

void init() {
   set("Ponder", "false");
   set("Threads", "1");
   set("Hash", "64");
}
void update() {
   Ponder    = get_bool("Ponder");
   Threads   = get_int("Threads");
   Hash      = 1 << ml::log_2(get_int("Hash"));
}

std::string get(const std::string &name) {
   if (GVar.find(name) == GVar.end()) {
      Tee << "unknown variable: \"" << name << "\"" << std::endl;
      std::exit(EXIT_FAILURE);
   }

   return GVar[name];
}

void set(const std::string &name, const std::string &value) {
    GVar[name] = value;
}

bool get_bool(const std::string & name) {

   std::string value = get(name);

   if (value == "true") {
      return true;
   } else if (value == "false") {
      return false;
   } else {
      Tee << "not a boolean: variable " << name << " = \"" << value << "\"" << std::endl;
      std::exit(EXIT_FAILURE);
      return false;
   }
}

int get_int(const std::string & name) {
   return std::stoi(get(name));
}

}