#include "Viviz.h"

Viviz::Viviz() {}

Viviz::Viviz(DWORD ProcessId) : processId(ProcessId)
{
    this->Refresh();
}

Viviz::~Viviz() {}

void Viviz::SetProcessId(DWORD ProcessId)
{
    this->processId = ProcessId;
    this->Refresh();
}

DWORD Viviz::GetProcessId() const
{
    return this->processId;
}

void Viviz::Refresh()
{
    this->symbols.clear();
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_OPERATION, FALSE, this->processId);

    if (hProcess)
    {
        if (SymInitialize(hProcess, NULL, TRUE))
        {
            HMODULE hMods[0x1000];
            DWORD out;

            if (EnumProcessModulesEx(hProcess, hMods, 0x1000, &out, LIST_MODULES_ALL))
            {
                for (uint32_t i = 0; i < out / 4; ++i)
                {
                    MODULEINFO mi;

                    if (GetModuleInformation(hProcess, hMods[i], &mi, sizeof(mi)))
                    {
                        char path[MAX_PATH];

                        this->module_name = GetModuleBaseNameA(hProcess, hMods[i], path, MAX_PATH) ? path : std::string();

                        SymEnumSymbols(hProcess, reinterpret_cast<ULONG64>(mi.lpBaseOfDll), "*!*", SymEnumSymbolsProc, this);
                    }
                }
            }
            SymCleanup(hProcess);
        }
        CloseHandle(hProcess);
    }
}

BOOL CALLBACK Viviz::SymEnumSymbolsProc(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext)
{
    Viviz *c = static_cast<Viviz*>(UserContext); //'this' ptr passed as UserContext
    c->symbols.push_back(WinSymbol(pSymInfo, c->module_name));
    return TRUE;
}


std::vector<WinSymbol> Viviz::All() const
{
    return this->symbols;
}

std::vector<WinSymbol> Viviz::FromAddress(ULONG64 Address) const
{
    std::vector<WinSymbol> results;
    for (auto i = this->symbols.begin(); i != this->symbols.end(); ++i)
    {
        if (i->address == Address)
        {
            results.push_back(*i);
        }
    }
    return results;
}

std::vector<WinSymbol> Viviz::FromName(const std::string &Name, Sensitivity s) const
{
    std::vector<WinSymbol> results;
    for (auto i = this->symbols.begin(); i != this->symbols.end(); ++i)
    {
        if ((s == Sensitivity::CaseInsensitive && !_stricmp(i->name.c_str(), Name.c_str())) || (s == Sensitivity::CaseSensitive && !strcmp(i->name.c_str(), Name.c_str())))
        {
            results.push_back(*i);
        }
    }
    return results;
}

std::vector<WinSymbol> Viviz::FromModule(ULONG64 ModuleBase) const
{
    std::vector<WinSymbol> results;
    for (auto i = this->symbols.begin(); i != this->symbols.end(); ++i)
    {
        if (i->module_address == ModuleBase)
        {
            results.push_back(*i);
        }
    }
    return results;
}

std::vector<WinSymbol> Viviz::FromModule(const std::string &ModuleName, Sensitivity s) const
{
    std::vector<WinSymbol> results;
    for (auto i = this->symbols.begin(); i != this->symbols.end(); ++i)
    {
        if ((s == Sensitivity::CaseInsensitive && !_stricmp(i->module_name.c_str(), ModuleName.c_str())) || (s == Sensitivity::CaseSensitive && !strcmp(i->module_name.c_str(), ModuleName.c_str())))
        {
            results.push_back(*i);
        }
    }
    return results;
}

std::vector<WinSymbol> Viviz::RegExMatch(const std::regex &re) const
{

    std::vector<WinSymbol> results;
    std::smatch m;
    for (auto i = this->symbols.begin(); i != this->symbols.end(); ++i)
    {
        if (std::regex_match(i->name, m, re))
        {
            results.push_back(*i);
        }
    }
    return results;
}



WinSymbol::WinSymbol(PSYMBOL_INFO si, std::string mn) : name(si->Name), module_name(mn), address(si->Address), module_address(si->ModBase), flags(si->Flags) {}

WinSymbol::~WinSymbol() {}
