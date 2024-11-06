#include "vcan_if.hpp"


int main() {
    NetlinkInterface netlinkInterface("vcan0");

    if (netlinkInterface.createInterface() < 0) {
        return -1;
    }

    // Simulate some operation before deleting
    std::cout << "Press Enter to delete the vcan0 interface...";
    std::cin.get();

    if (netlinkInterface.deleteInterface() < 0) {
        return -1;
    }

    return 0;
}