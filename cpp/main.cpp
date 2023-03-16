#include <time.h>
#include "subprocess.h"

int main()
{
    const char* path = "/usr/bin/python3";
    const char* py = "../py/main.py";
    int w = 1920;
    int h = 1080;
    SubProcess p(path, vector<string>{ py, to_string(w), to_string(h) });

    vector<char> buf(w * h * 3);
    while (true)
    {
        clock_t start = clock();
        p.write_(buf.data(), buf.size());
        //int size = p.read_(buf.data(), buf.size());
        //if (size > 0) cout << size << endl;
        clock_t end = clock();
        cout << (double)(end - start) * 1000 / CLOCKS_PER_SEC << " msec." << endl;
    }
}