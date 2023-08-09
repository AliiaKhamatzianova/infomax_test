#include <thread>
#include <iostream>
#include <list>
#include <chrono>
#include <vector>
#include <algorithm>
#include <memory>
#include <mutex>
#include <condition_variable>

std::condition_variable cv;

class Request;
Request* GetRequest() throw(); 
void ProcessRequest(Request* request) throw();
const int NumberOfThreads = 2;
std::list<Request*> requests;
std::atomic<bool> isWorking = true;
std::mutex mutex;
void thread_func();

constexpr int sleepGetRequest = 1000;
constexpr int sleepProcessRequest = 100;
constexpr int countOffRequest = 100;

int main(int arg, char** argv)
{
    std::vector<std::thread> threads;
    for(auto i = 0; i < NumberOfThreads; ++i)
    {
        threads.push_back(std::thread(thread_func));
    }

    Request* request = nullptr;
    
    do
    {
        try
        {
            request = GetRequest();
            {
                std::unique_lock<std::mutex> m(mutex);
                requests.push_back(request);
                std::cout << "notify one" << std::endl;
                cv.notify_one();
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }while (request != nullptr);

    std::cout << __FUNCTION__ << " end" <<std::endl;
    isWorking = false;
    std::for_each( std::begin(threads), std::end(threads),  [](auto& thread){ thread.join();});

    std::cout << __FUNCTION__ << " finish" <<std::endl;

    for (auto request : requests)
    {
        delete request;
    }

    return 0;
}

class Request
{

};

Request* GetRequest() throw()
{
    static int requestCount = 0;
    Request *request = nullptr;
    if(requestCount < countOffRequest)
    {
        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(sleepGetRequest));
        request = new Request;
        ++requestCount;
        std::cout << __FUNCTION__ <<std::endl;
    }
    return request;
};

void thread_func()
{
    while(isWorking)
    {
        try
        {
            std::unique_ptr<Request> request;
            {
                std::unique_lock<std::mutex> m(mutex);
                
                if(!requests.empty()) 
                {
                    request =  std::unique_ptr<Request>(requests.front());
                    requests.pop_front();
                }
                else
                {
                    std::cout << "waiting" << std::endl;
                    cv.wait(m);
                }
            }
            ProcessRequest(request.get());
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }
}



void ProcessRequest(Request* request) throw()
{
    std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(sleepProcessRequest));
    std::cout << __FUNCTION__ << " threadId =  " << std::this_thread::get_id() << std::endl;

};
