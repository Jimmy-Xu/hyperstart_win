#include "fmt/format.h"
#include "fmt/ostream.h"
#include <iostream>
#include <time.h>

using namespace std;
using std::cout;
using std::endl;


class Date {
  int year_, month_, day_;
public:
  Date(int year, int month, int day) : year_(year), month_(month), day_(day) {}

  friend std::ostream &operator<<(std::ostream &os, const Date &d) {
    return os << d.year_ << '-' << d.month_ << '-' << d.day_;
  }
};

void test_hello() {
  fmt::print("Hello, {}!\n", "world"); // uses Python-like format string syntax
  fmt::printf("Hello, %s!\n", "world"); // uses printf format string syntax
  std::string s = fmt::format("{0}{1}{0}", "abra", "cad");
  cout << s << endl;
  return;
}

void test_itoa() {
  fmt::MemoryWriter w;
  w << 42;           // replaces itoa(42, buffer, 10)
  cout << w.str() << endl;

  w.clear();
  w << fmt::hex(42); // replaces itoa(42, buffer, 16)
  cout << w.str() << endl;
}

void test_time() {
    time_t calendar_time = time(NULL);

    char buffer[80];
    struct tm time_info;
    localtime_s(&time_info, &calendar_time);
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", &time_info);
    printf("strftime : %s\n", buffer);
    
    char calendar_str[26];
    ctime_s(calendar_str, sizeof calendar_str, &calendar_time);
    cout << fmt::format("ctime_s  :{0} \n", calendar_str) << endl;
}

void test_ostream() {
  std::string s = fmt::format("The date is {}", Date(2012, 12, 9));
  cout << s << endl;
}

// Prints formatted error message.
void report_error(const char *format, fmt::ArgList args) {
  fmt::print("Error: ");
  fmt::print(format, args);
}
FMT_VARIADIC(void, report_error, const char *)

int main(int argc, char **argv)
{
  test_hello();
  test_itoa();
  test_ostream();

  report_error("file not found: {}", "c:\\test\n");

  test_time();

  return 0;
}