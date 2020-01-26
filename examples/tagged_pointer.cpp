// Copyright - 2020 - Jan Christoph Uhde <Jan@UhdeJC.com>
// Please see LICENSE.md for license or visit https://github.com/extcpp/basics
#include <ext/memory/tagged_pointer.hpp>
#include <ext/util/encode.hpp>

#include <atomic>

using namespace std;
using namespace ext;

int main() {
    uint64_t value = 8;
    auto* int_pointer = &value;

    cout << "---- pointer ----------------------------------------" << endl;
    cout << "        pointer: " << int_pointer << endl;
    cout << "        pointer: " << util::encode_binary(reinterpret_cast<uintptr_t>(int_pointer)) << endl;
    cout << "           size: " << sizeof(int_pointer) << endl;
    cout << endl;

    memory::tagged_pointer pointer(int_pointer, 5);
    cout << "---- tagged_pointer ---------------------------------" << endl;
    cout << "  pointer.get(): " << pointer.get() << endl;
    cout << "   pointer.mask: " << pointer.mask << endl;
    cout << "  pointer.tag(): " << pointer.tag() << endl;
    cout << "  pointer.get(): " << util::encode_binary(reinterpret_cast<uintptr_t>(pointer.get())) << endl;
    cout << "   pointer.mask: " << util::encode_binary(pointer.mask) << endl;
    cout << "  pointer.tag(): " << util::encode_binary(pointer.tag()) << endl;
    cout << "         is pod: " << std::is_pod_v<decltype(pointer)> << endl;
    cout << "standard layout: " << std::is_standard_layout_v<decltype(pointer)> << endl;
    cout << "           size: " << sizeof(pointer) << endl;
    cout << endl;

#ifndef _WIN32
    std::atomic<memory::tagged_pointer<uint64_t>> atomic_pointer(pointer);
    cout << "---- atomic tagged_pointer --------------------------" << endl;
    cout << "  pointer.get(): " << atomic_pointer.load().get() << endl;
    cout << "   pointer.mask: " << atomic_pointer.load().mask << endl;
    cout << "  pointer.tag(): " << atomic_pointer.load().tag() << endl;
    cout << "  pointer.get(): " << util::encode_binary(reinterpret_cast<uintptr_t>(atomic_pointer.load().get()))
         << endl;
    cout << "   pointer.mask: " << util::encode_binary(atomic_pointer.load().mask) << endl;
    cout << "  pointer.tag(): " << util::encode_binary(atomic_pointer.load().tag()) << endl;
    cout << "         is pod: " << std::is_pod_v<decltype(atomic_pointer)> << endl;
    cout << "standard layout: " << std::is_standard_layout_v<decltype(atomic_pointer)> << endl;
    cout << "           size: " << sizeof(atomic_pointer) << endl;
    cout << endl;
#endif // _WIN32

    auto current = pointer.tag();
    do {
        cout << "  pointer.tag(): " << pointer.tag() << endl;
        pointer.set(pointer.tag_next());
    } while (pointer.tag() != current);

    return 0;
}
