#include <iostream>
#include <thread>

void thread_function()
{
    std::cout << "thread function\n";
}

int main()
{
    std::thread t(&thread_function);   // t starts running
    std::cout << "main thread\n";
    t.detach();
    return 0;
}
