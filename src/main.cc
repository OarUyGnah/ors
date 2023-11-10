#include <iostream>
#include <google/protobuf/endian.h>
#include <utils/tid.h>
using namespace std;

int main(int argc, char** argv)
{
    
    cout << "123" << endl;
    cout << ors::utils::tid::tid() << endl;
    return 0;
}
