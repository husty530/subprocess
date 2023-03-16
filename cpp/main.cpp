#include <time.h>
#include "subprocess.h"

int main()
{
    auto path = "C:/Users/husty/AppData/Local/Programs/Python/Python310/python.exe";
    auto py = "../py/main.py";
    int w = 1920;
    int h = 1080;

    SubProcess p(path, vector<string>{ py, to_string(w), to_string(h) });
    vector<char> buf(w * h * 3);
    fill(buf.begin(), buf.end(), 0);
    while (true)
    {
        clock_t start = clock();
        p.write(buf.data(), buf.size());
        //int size = p.read(buf.data(), buf.size());
        //if (size > 0) cout << size << endl;
        clock_t end = clock();
        cout << "duration = " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << endl;
    }
}