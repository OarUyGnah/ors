#include <iostream>
#include <string>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
using namespace std;

int main(int argc, char const *argv[])
{
    string str = "/tmp/ors_configtest/test.txt";
    std::cerr << "path == " << str << std::endl;
    
    int fd = open(str.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (fd < 0) {
      std::cerr << "Couldn't open temp file " << errno << std::endl;
    }
    const char *file = "# this is a sample config file \n"
                       "        # here's a comment in a random place \n"
                       " double = 3.14  \n"
                       " int = 314  #  comment  \n"
                       " string = a = b = c = d  #  newline comes next  \n"
                       "\n"
                       " multiline = \n"
                       "                strange\n"
                       "# just a comment\n"
                       " empty =  \n";
    printf("%s\n",file);
    printf("=====%d=====\n",uint32_t(strlen(file)));
    if (write(fd, file, uint32_t(strlen(file))) == -1)
      std::cerr << "Couldn't write to temp file" << std::endl;
    close(fd);
    return 0;
}
