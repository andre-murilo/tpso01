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
#include "main.h"


void SetConsolePos(SHORT x, SHORT y)
{
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {x, y};
    SetConsoleCursorPosition(output, pos);
}



void CClock::Start()
{
    start = std::chrono::system_clock::now();
}

void CClock::Reset()
{
    start = std::chrono::system_clock::now();
}

unsigned int CClock::Elapsed()
{
    end = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}


CTime::CTime(int hora, int minuto)
{
    this->hour = hora;
    this->minute = minuto;
    this->second = 0;
}

CTime::CTime()
{
    this->hour = 0;
    this->minute = 0;
    this->second = 0;
}


void CTime::AddSeconds(int seconds)
{
    this->second += seconds;

    while(second >= 60)
    {
        minute++;
        second -= 60;

        while(minute >= 60)
        {
            hour++;
            minute -= 60;

            while(hour >= 24)
            {
                hour = 0;
                minute = 0;
                second = 0;
            }
        }
    }
}


unsigned int CTime::GetAllInSeconds()
{
    return second + (minute * 60) + (hour * 3600);
}

void CTime::SetTime(int hour, int minute)
{
    this->hour = hour;
    this->minute = minute;
}

int CTime::GetHour()
{
    return this->hour;
}

int CTime::GetMinute()
{
    return this->minute;
}

void CTime::PrintTime()
{
    printf("Now -> %d:%d:%d\n", hour, minute, second);
}


CTime g_Time(0,0);



Proprietary::Proprietary(char *name, char type)
{
    memcpy(this->m_name, name, 64);

    switch (type)
    {
    case 'P':
        myDefault:
            m_type = ETypeProprietary::Teacher;
            m_priority = 3;
        break;
    case 'C':
        m_type = ETypeProprietary::Coordinator;
        m_priority = 2;
        break;
    case 'D':
        m_type = ETypeProprietary::Director;
        m_priority = 1;
        break;

    default:
        goto myDefault;
        break;
    }
}

CPrintRequest::CPrintRequest(Proprietary *p, unsigned int pages, CTime deadline, CTime startTime)
{
    this->m_proprietary = p;
    this->m_pages = pages;
    this->c_deadline = deadline;
    this->m_startTime = startTime;
    this->m_printed = false;
    this->m_printing = false;
}


CPrinter::CPrinter(int id)
{
    this->m_id = id;
    this->m_printing = false;
    this->m_cooldown = false;
    this->m_refillCooldown = 0;
    this->m_pageTimer = 0;
    this->m_printed = false;
}

bool CPrinter::IsPrinting()
{
    return this->m_printing;
}

void CPrinter::Print(CPrintRequest* request)
{
    this->m_current = request;
    this->m_printing = true;
    this->m_current->m_printing = true;

    printf("Imprimindo %u paginas...\n", request->m_pages);
}

void CPrinter::update(int delta)
{
    if(m_cooldown)
    {
        m_refillCooldown += delta;

        if(m_refillCooldown >= 13)
        {
            m_cooldown = false;
        }
    }

    if(m_printing)
    {
        m_pageTimer += delta;

        while(m_pageTimer >= 2)
        {
            m_pageTimer -= 2;

            if(m_current->m_pages >= 1)
                this->m_current->m_pages--;
            else
            {
                m_printing = false;
                m_current->m_printed = true;
                this->m_printed = true;
                this->m_cooldown = true;

                // impressao completa
                printf("Impressao completa: %d:%d:%d\n\n", g_Time.hour, g_Time.minute, g_Time.second);
            }
        }
    }
}






bool sortByPriority(const CPrintRequest& req1, const CPrintRequest& req2)
{
    CTime time1 = req1.c_deadline;
    CTime time2 = req2.c_deadline;

    int totalSecondsToPrint1 = req1.m_pages * 2;
    int totalSecondsToPrint2 = req2.m_pages * 2;

    bool time1Priority = false;
    bool time2Priority = false;

    // verifica se o prazo esta proximo; 2 horas
    if(g_Time.GetAllInSeconds() + totalSecondsToPrint1 >  time1.GetAllInSeconds() + 7200)
    {
        // prioridade!!!
        time1Priority = true;
    }

    if(g_Time.GetAllInSeconds() + totalSecondsToPrint2 >  time2.GetAllInSeconds() + 7200)
    {
        // prioridade!!!
        time2Priority = true;
    }

    // se ambos nao estiverem na situacao critica
    if(!time1Priority && !time2Priority )
    {
        // ordenar pela prioridade
        return req1.m_proprietary > req2.m_proprietary;
    }
    else
    {
        // ordenar pelo tempo
        return time1Priority > time2Priority;
    }
}


void Engine::AddProprietary(Proprietary proprietary)
{
    this->m_proprietaries.push_back(proprietary);
}

bool Engine::ExistsProprietary(char *name)
{
    for(Proprietary p : m_proprietaries)
    {
        if(strcmp((const char*)p.m_name, (const char*)name) == 0)
        {
            return true;
        }
    }

    return false;
}

Proprietary* Engine::GetProprietaryByName(char *name)
{
    for(int i = 0; i < m_proprietaries.size(); i++)
    {
        Proprietary p = m_proprietaries[i];
        if(strcmp((const char*)p.m_name, (const char*)name) == 0)
        {
            return &m_proprietaries[i];
        }

    }

    return nullptr;
}

CPrintRequest * Engine::GetNextRequest()
{
    for(int i = 0; this->m_requests.size(); i++)
    {
        if(!m_requests[i].m_printing)
            return &m_requests[i];
    }

    return NULL;
}


void Engine::AddRequest(CPrintRequest request)
{
    this->m_requests.push_back(request);
}

void Engine::AddPrinter(CPrinter printer)
{
    this->m_printers.push_back(printer);
}

void Engine::RemoveRequest(CPrintRequest *request)
{
    for(int i = 0; i < m_requests.size(); i++)
    {
        if(&m_requests[i] == request)
        {
            m_requests.erase(m_requests.begin() + i);
        }
    }
}


bool Engine::ParseFile(const char* path)
{
    std::ifstream file(path);
    if(!file.is_open())
        return false;
        
    // parse count
    char line[256];

    if(!file.getline(line, 256))
        return false;

    
    // parse pedidos
    while(file.getline(line, 256))
    {
        char name[64];
        char type;
        unsigned int total_pages;
        char time[16];
        char time2[16];
        
        CTime prazo;
        CTime chegada;

        // pedido
        sscanf(line, "%[^;];%c;%u;%[^;];%s", name, &type, &total_pages, time, time2);

        sscanf(time, "%d:%d", &prazo.hour, &prazo.minute);
        sscanf(time2, "%d:%d", &chegada.hour, &chegada.minute);


        // printf("Nome: %s - Tipo: %c - Paginas: %u - Prazo: %d:%d - Chegada: %d:%d\n",
        //         name, type, total_pages, prazo.GetHour(), prazo.GetMinute(), 
        //         chegada.GetHour(), chegada.GetMinute());

        // add
        if(!ExistsProprietary(name))
        {
            Proprietary proprietary(name, type);
            AddProprietary(proprietary);
        }

        Proprietary* proprietary = GetProprietaryByName(name);
        if(proprietary == nullptr)
            continue;

        CPrintRequest req(proprietary, total_pages, prazo, chegada);

        if(prazo.hour == 0 && prazo.minute == 0)
            this->AddRequest(req);
        else
            this->m_requests_future.push_back(req);
    }

    return true;
}


void Engine::Sort()
{
    // sort per priority or time
    std::sort(m_requests.begin(), m_requests.end(), sortByPriority);
}


void Engine::Initialize(int printersCount)
{
    printf("[*] Lendo pedidos do arquivos: dados.txt\n");
    ParseFile("dados.txt");
    printf("[+] Pedidos inseridos: %d\n", this->m_requests.size() + this->m_requests_future.size());

    this->Sort();
    
    
    // add printers
    for(int i = 0; i < printersCount; i++)
    {
        CPrinter printer(i);
        m_printers.push_back(printer);
    }
    
}

void Engine::Run()
{
    CClock clock;

    int delta = 0;

    clock.Start();
    while(true)
    {
        delta = clock.Elapsed();
        clock.Reset();

        delta = (int)ceil(delta);

        while(delta >= 1000000)
        {
            delta -= 1000000;
            Update(1);
        }

        Sleep(1);
    }
}
void Engine::Update(int delta)
{
    // update system time
    g_Time.AddSeconds(delta);
    //g_Time.PrintTime();

    // verificar se tem pedido a ser inserido
    for(int i = 0; i < m_requests_future.size(); i++)
    {
        CTime chegada = m_requests_future[i].m_startTime;

        
        if(g_Time.GetAllInSeconds() > chegada.GetAllInSeconds())
        {
            // opa, esta na hora de inserir esse cara!
            this->m_requests.push_back(m_requests_future[i]);
            printf("Entrou um pedido: %d:%d:%d\n", chegada.hour, chegada.minute, chegada.second);
            this->Sort();
            
            m_requests_future.erase(m_requests_future.begin() + i);
        }
    }

    // atualizar impressoras
    for(int i = 0; i < m_printers.size(); i++)
    {
        // verifica se ja imprimiu
        if(m_printers[i].m_printed)
        {
            CPrintRequest *req = m_printers[i].m_current;
            RemoveRequest(req);
            m_printers[i].m_printed = false;
            m_printers[i].m_cooldown = true;
            m_printers[i].m_current = nullptr;
        }

        // verifica se a impressora esta disponivel
        if(!m_printers[i].IsPrinting() && !m_printers[i].m_cooldown)
        {
            CPrintRequest *toPrint = GetNextRequest();
            if(toPrint != NULL)
                m_printers[i].Print(toPrint);
        }

        m_printers[i].update(delta);
    }


}


int PrintBanner()
{
    std::vector<std::string> banner;


    banner.push_back("             ________________________________________________\n");
    banner.push_back("            /                                                \\\n");
    banner.push_back("           |    _________________________________________     |\n");
    banner.push_back("           |   |                                         |    |\n");
    banner.push_back("           |   |                                         |    |\n");
    banner.push_back("           |   |                                         |    |\n");
    banner.push_back("           |   |                                         |    |\n");
    banner.push_back("           |   |                                         |    |\n");
    banner.push_back("           |   |                                         |    |\n");
    banner.push_back("           |   |                                         |    |\n");
    banner.push_back("           |   |                                         |    |\n");
    banner.push_back("           |   |                                         |    |\n");
    banner.push_back("           |   |                                         |    |\n");
    banner.push_back("           |   |                                         |    |\n");
    banner.push_back("           |   |                                         |    |\n");
    banner.push_back("           |   |                                         |    |\n");
    banner.push_back("           |   |_________________________________________|    |\n");
    banner.push_back("           |                                                  |\n");
    banner.push_back("            \\_________________________________________________/\n");
    banner.push_back("                   \\___________________________________/\n");
    banner.push_back("                ___________________________________________\n");
    banner.push_back("             _-'    .-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.  --- `-_\n");
    banner.push_back("          _-'.-.-. .---.-.-.-.-.-.-.-.-.-.-.-.-.-.-.--.  .-.-.`-_\n");
    banner.push_back("       _-'.-.-.-. .---.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-`__`. .-.-.-.`-_\n");
    banner.push_back("    _-'.-.-.-.-. .-----.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-----. .-.-.-.-.`-_\n");
    banner.push_back(" _-'.-.-.-.-.-. .---.-. .-------------------------. .-.---. .---.-.-.-.`-_\n");
    banner.push_back(":-------------------------------------------------------------------------:\n");
    banner.push_back("`---._.-------------------------------------------------------------._.---'\n");

    std::vector<std::string> content;
    content.push_back("root@local:~$ ./tpso01");
    content.push_back("[+] Iniciando programa...");
    content.push_back("[!] Simulacao de impressora.");
    content.push_back("");
    content.push_back("[*] Autores:");
    content.push_back("    [1] Andre Murilo.");
    content.push_back("    [2] Lucas Pereira.");
    content.push_back("    [3] Geovane de Freitas.");
    content.push_back("");
    content.push_back("Numero de impressoras: ");

    // print window
    for(std::string s : banner)
    {
        for(int i = 0; i < s.size(); i++)
        {
            printf("%c", s.at(i));
            
        }
        Sleep(40);
    }

    // print content
    SHORT posY = 4;
    SHORT posX = 18;

    int i = 0;
    for(std::string s : content)
    {
        SetConsolePos(posX, posY);

        if(i == 1)
            Sleep(500);

        for(int i = 0; i < s.size(); i++)
        {
            printf("%c", s.at(i));
            Sleep(35);
        }

        posY++;
        posX = 18;
        i++;
    }


    int printersCount = 0;
    scanf("%d", &printersCount);

    system("cls");
    SetConsolePos(0, 0);
    printf("[+] Numero de impressoras: %d\n", printersCount);

    return printersCount;
}


int main(int argc, char const *argv[])
{
    HWND console = GetConsoleWindow();
    RECT r;
    GetWindowRect(console, &r); //stores the console's current dimensions
    MoveWindow(console, r.left, r.top, 700, 550, TRUE); // 800 width, 100 height

    Engine engine;
    SetConsoleTitleA("Simulacao de Impressora");

    setlocale(LC_ALL, "portuguese-brazilian");
    //int printers = PrintBanner();
    engine.Initialize(2);
    engine.Run();


    return 0;
}
