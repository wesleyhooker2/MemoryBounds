#include <cstdio> // For printf, no cout in multithreaded
#include <thread>
#include <mutex>
#include <inttypes.h>
#include <iostream>
#include <functional>
#include <chrono>
using std::mutex;
using std::thread;
using namespace std;

//Globals
const uint64_t NUM_ROWS = (1024 * 1024);
const uint64_t NUM_COLS = (1024 * 10);
// const uint64_t NUM_ROWS = (10);
// const uint64_t NUM_COLS = (10);
const uint64_t SIZE = NUM_ROWS * NUM_COLS; // 10 GB!
int totalWorkUnits = 20;
int currentWorkUnit = 0;
int workUnitSize = SIZE / totalWorkUnits;
int result;
int numThreads = 2;
mutex myMutex;

void singleThreadedRowMajor(uint8_t *arr, int bla)
{
    int localResult = 0;
    for (uint64_t i = 0; i < NUM_ROWS; i++)
    {
        for (uint64_t j = 0; j < NUM_COLS; j++)
        {
            localResult += arr[i * NUM_COLS + j];
            // printf("arr[%d][%d] = %d\n", j, i, i * NUM_COLS + j);
        }
    }
    result = localResult;
}

void singleThreadedColumnMajor(uint8_t *arr, int bla)
{
    int localResult = 0;
    for (uint64_t i = 0; i < NUM_COLS; i++)
    {
        for (uint64_t j = 0; j < NUM_ROWS; j++)
        {
            localResult += arr[j * NUM_COLS + i];
            // printf("arr[%d][%d] = %d\n", j, i, j * NUM_COLS + i);
        }
    }
    result = localResult;
}

void multiThreadedColumnMajor(uint8_t *arr, uint8_t threadID)
{
    int localResult = 0;

    uint64_t startCol = (threadID * NUM_COLS) / numThreads;
    uint64_t endCol = ((threadID + 1) * NUM_COLS) / numThreads;
    for (uint64_t j = startCol; j < endCol; j++)
    {
        for (uint64_t i = 0; i < NUM_ROWS; i++)
        {
            localResult += arr[i * NUM_COLS + j];
        }
    }

    // MUTEX
    myMutex.lock();
    // Run critical region of code
    result += localResult;
    myMutex.unlock();
}

void multiThreadedRowMajor(uint8_t *arr, uint8_t threadID)
{
    int localResult = 0;

    uint64_t startIndex = threadID * NUM_ROWS / numThreads;
    uint64_t endIndex = (threadID + 1) * NUM_ROWS / numThreads;
    for (uint64_t i = startIndex; i < endIndex; i++)
    {
        for (uint64_t j = 0; j < NUM_COLS; j++)
        {
            localResult += arr[i * NUM_COLS + j];
        }
    }

    // MUTEX
    myMutex.lock();
    // Run critical region of code
    result += localResult;
    myMutex.unlock();
}

//WorkPool onward all work like row major, continuing on with arr[i * NUM_COLS + j] to calculate the row at this point is unecessary.
//All i would be doing is casting it to a 2d array like that and when im calculating the size of the workpool id be negating it to see what row
//and column i am on. Then putting it back to arr[i * NUM_COLS + j] form.
void workPool(uint8_t *arr, uint8_t threadID)
{
    int localResult = 0;
    while (currentWorkUnit < totalWorkUnits)
    {
        //Get workpool
        myMutex.lock();
        int localWorkUnitStart = currentWorkUnit * workUnitSize;
        int localWorkUnitEnd = (currentWorkUnit + 1) * workUnitSize;
        currentWorkUnit++;
        myMutex.unlock();

        //Do work
        for (int i = localWorkUnitStart; i < localWorkUnitEnd; i++)
        {
            if (i >= SIZE)
            {
                return;
            }
            // printf("i = %d\n", i);
            localResult += arr[i];
        }

        //Save Result
        myMutex.lock();
        // Run critical region of code
        result += localResult;
        myMutex.unlock();
    }
}

void asUint64(uint8_t *arr, uint8_t threadID)
{
    int localResult = 0;
    while (currentWorkUnit < totalWorkUnits)
    {
        //Get workpool
        myMutex.lock();
        int localWorkUnitStart = currentWorkUnit * workUnitSize;
        int localWorkUnitEnd = (currentWorkUnit + 1) * workUnitSize;
        currentWorkUnit++;
        myMutex.unlock();

        //Do work
        for (int i = localWorkUnitStart; i < localWorkUnitEnd; i += 8)
        {
            if (i >= SIZE)
            {
                return;
            }
            // Make a copy of 8 bytes.
            uint64_t buffer = *(reinterpret_cast<uint64_t *>(&arr[i]));
            uint8_t *bufferArray = reinterpret_cast<uint8_t *>(&buffer);
            for (int j = 0; j < 8; j++)
            {
                if (i + j < localWorkUnitEnd)
                {
                    // printf("i = %d\n", i + j);
                    localResult += bufferArray[j];
                }
            }
        }

        //Save Result
        myMutex.lock();
        // Run critical region of code
        result += localResult;
        myMutex.unlock();
    }
}

void loopUnroll4(uint8_t *arr, uint8_t threadID)
{
    int localResult = 0;
    while (currentWorkUnit < totalWorkUnits)
    {
        //Get workpool
        myMutex.lock();
        int localWorkUnitStart = currentWorkUnit * workUnitSize;
        int localWorkUnitEnd = (currentWorkUnit + 1) * workUnitSize;
        currentWorkUnit++;
        myMutex.unlock();

        //Do work
        for (int i = localWorkUnitStart; i < localWorkUnitEnd; i += 4)
        {
            if (i >= SIZE)
            {
                return;
            }
            localResult += arr[i + 0];
            localResult += arr[i + 1];
            localResult += arr[i + 2];
            localResult += arr[i + 3];
        }

        //Save Result
        myMutex.lock();
        // Run critical region of code
        result += localResult;
        myMutex.unlock();
    }
}

void loopUnroll20(uint8_t *arr, uint8_t threadID)
{
    int localResult = 0;
    while (currentWorkUnit < totalWorkUnits)
    {
        //Get workpool
        myMutex.lock();
        int localWorkUnitStart = currentWorkUnit * workUnitSize;
        int localWorkUnitEnd = (currentWorkUnit + 1) * workUnitSize;
        currentWorkUnit++;
        myMutex.unlock();

        //Do work
        for (int i = localWorkUnitStart; i < localWorkUnitEnd; i += 20)
        {
            if (i >= SIZE)
            {
                return;
            }
            localResult += arr[i + 0];
            localResult += arr[i + 1];
            localResult += arr[i + 2];
            localResult += arr[i + 3];
            localResult += arr[i + 4];
            localResult += arr[i + 5];
            localResult += arr[i + 6];
            localResult += arr[i + 7];
            localResult += arr[i + 8];
            localResult += arr[i + 9];
            localResult += arr[i + 10];
            localResult += arr[i + 11];
            localResult += arr[i + 12];
            localResult += arr[i + 13];
            localResult += arr[i + 14];
            localResult += arr[i + 15];
            localResult += arr[i + 16];
            localResult += arr[i + 17];
            localResult += arr[i + 18];
            localResult += arr[i + 19];
        }

        //Save Result
        myMutex.lock();
        // Run critical region of code
        result += localResult;
        myMutex.unlock();
    }
}

void runThreads(int numThreads, uint8_t *arr, function<void(uint8_t *, int)> func)
{
    int localResult = 0;
    if (numThreads == 1)
    {
        func(arr, 0);
    }
    else
    {
        // Create thread tracking objets, these are NOT threads themselves
        thread *threads = new thread[numThreads];
        // FORK-JOIN MODEL
        for (int i = 0; i < numThreads; i++)
        {
            threads[i] = thread(func, arr, i);
        }
        for (int i = 0; i < numThreads; i++)
        {
            threads[i].join();
        }
        delete[] threads;
    }

    result = localResult;
}

int main()
{
    uint8_t *arr = new uint8_t[SIZE];

    auto start = std::chrono::high_resolution_clock::now();
    //Run the Read function
    //-----------------------------------------
    runThreads(numThreads, arr, loopUnroll20);
    //-----------------------------------------
    auto end = std::chrono::high_resolution_clock::now();
    auto timeInMilli = std::chrono::duration<double, std::milli>(end - start).count();
    printf("Total time: %1.3lf ms.\n", timeInMilli);

    delete[] arr;
    return 0;
}