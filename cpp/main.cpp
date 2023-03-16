#include <time.h>
#include "subprocess.h"

int main()
{
    const char* path = "/** absolute path **/python.exe";
    const char* py = "../py/main.py";
    int w = 1920;
    int h = 1080;
    SubProcess p(path, vector<string>{ py, to_string(w), to_string(h) });

    vector<char> buf(w * h * 3);
    while (true)
    {
        clock_t start = clock();
        auto h = p.try_write(buf.data(), buf.size());
        // p.try_read(buf.data(), buf.size());
        clock_t end = clock();
        cout << (double)(end - start) * 1000 / CLOCKS_PER_SEC << " msec." << endl;
    }
}