#include <iostream>

int main() {
    int *tr = (int *) 0x233e2591aa0;
    // *tr = 12;
    std::cout << tr << ' ' << *tr;
    return 0;
}
