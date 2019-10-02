#include <iostream>
#include <locale>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <time.h>
#include <thread>
#include <mutex>
#include <ctime>
#include <chrono>
#include <queue>
#include <windows.h>
#include <algorithm>
#include <cmath>

// 6 SECONDS PER HOUR
#define SPEED_TIME 600

void SetConsolePos(SHORT x, SHORT y);

enum class ETypeProprietary {Teacher, Coordinator, Director};

class CClock
{
    private:
        std::chrono::time_point<std::chrono::system_clock> start, end;

    public:
        void Start();

        void Reset();

        unsigned int Elapsed();
};


class CTime
{
    public:
        int hour;
        int minute;
        int second;

        CTime(int hora, int minuto);
        
        CTime();

        void AddSeconds(int seconds);

        unsigned int GetAllInSeconds();

        void SetTime(int hour, int minute);

        int GetHour();

        int GetMinute();

        void PrintTime();
};


class Proprietary
{
    public:
        ETypeProprietary m_type;
        char m_name[64];
        int m_priority;

        Proprietary(char *name, char type);
};


class CPrintRequest
{
    public:
        Proprietary *m_proprietary;
        unsigned int m_pages;
        bool m_printed;
        bool m_printing;
        CTime c_deadline;
        CTime m_startTime;
        CTime diffTime;


        CPrintRequest(Proprietary *p, unsigned int pages, CTime deadline, CTime startTime);

};



class CPrinter
{

public:
    static const int PAGES_PER_MINUTE = 40; // 1.5 seconds per page; 2~ ms
    static const int REFILL_TIME = 8; // 13 miliseconds

    int m_id;
    int m_refillCooldown;
    int m_pageTimer;
    bool m_printing;
    bool m_cooldown;
    bool m_printed;


    CPrintRequest *m_current;

    CPrinter(int id);

    bool IsPrinting();

    void Print(CPrintRequest* request);

    void update(int delta);
};


class LogRecorder
{
    public:
        struct Log
        {
            CPrintRequest m_request;
            CTime m_timeStart;
            CTime m_timeEnd;
        };


    std::vector<Log> logs;
};



class Engine
{

public:
    std::vector<Proprietary> m_proprietaries;
    std::vector<CPrinter> m_printers;

    // caixa de e-mail
    std::vector<CPrintRequest> m_requests;
    std::vector<CPrintRequest> m_requests_future;

    void AddProprietary(Proprietary proprietary);

    bool ExistsProprietary(char *name);

    Proprietary* GetProprietaryByName(char *name);

    void AddRequest(CPrintRequest request);

    void AddPrinter(CPrinter printer);
    void RemoveRequest(CPrintRequest* request);
 


    bool ParseFile(const char* path);


    void Sort();

    void Initialize(int printersCount);

    void Run();
    
    void Update(int delta);

};

int PrintBanner();
