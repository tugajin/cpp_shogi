#ifndef VAR_HPP
#define VAR_HPP
namespace var {
    
extern bool Ponder;
extern int32 Hash;
extern int32 Threads;

void init();
void update();

std::string get(const std::string &name);
void set(const std::string &name, const std::string &value);
bool get_bool(const std::string &name);
int get_int(const std::string &name);

}
#endif