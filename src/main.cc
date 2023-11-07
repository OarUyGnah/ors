#include <iostream>
#include <utils/buffer.h>
using namespace std;
// using namespace o
int main(int argc, char** argv)
{
    cout << 1 << endl;
    ors::utils::buffer buf;
    int *a = new int;
    *a = 8;
    buf.set_data(a,sizeof(int),nullptr);
    return 0;
}
