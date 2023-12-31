#include <google/protobuf/endian.h>
#include <iostream>
// #include <utils/tid.h>
#include <pqxx/pqxx>
using namespace std;

int test_psql() {
  try {
    pqxx::connection C(
        "dbname=postgres hostaddr=127.0.0.1 user=postgres password=123");
    std::cout << "Connected to " << C.dbname() << std::endl;
    pqxx::work W(C);

    pqxx::result R = W.exec("SELECT name FROM employee");

    std::cout << "Found " << R.size() << "employees:" << std::endl;
    for (auto row : R)
      std::cout << row[0].c_str() << std::endl;

    std::cout << "Doubling all employees' salaries..." << std::endl;
    W.exec("UPDATE employee SET salary = salary*2");

    std::cout << "Making changes definite: ";
    W.commit();
    std::cout << "OK." << std::endl;
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}

int main(int argc, char **argv) {
  //   test_psql();
  // cout << ors::utils::tid::tid() << endl;
  return 0;
}
