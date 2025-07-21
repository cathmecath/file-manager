#include <iostream>
#include "os_file.h"

using namespace std;

int main() {
    file_manager_t mm;

    setup_file_manager(&mm);
    int TOTAL_SIZE = 1000;

    cout << mm.create(TOTAL_SIZE) << endl;
    cout << mm.create_dir("dir1") << endl;

    return 0;
}
