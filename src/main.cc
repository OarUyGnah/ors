#include <iostream>
#include <google/protobuf/endian.h>
using namespace std;

int main(int argc, char** argv)
{
    cout << __bswap_16(100) << endl;
    return 0;
}
