#ifndef VIVIZ_H
#define VIVIZ_H

#include <Windows.h>
#include <DbgHelp.h>
#include <Psapi.h>

#include <string>
#include <vector>
#include <regex>

class WinSymbol;

class Viviz
{
public:
    Viviz();
    Viviz(DWORD ProcessId);
    ~Viviz();

    enum Sensitivity { CaseSensitive, CaseInsensitive };

    //call this when more symbols are loaded by the target (note- it is automatically called when setting a processid or initializing w/ one)
    void Refresh();

    void SetProcessId(DWORD ProcessId);
    DWORD GetProcessId() const;

    std::vector<WinSymbol> All() const;

    //gets symbols from a desired address
    std::vector<WinSymbol> FromAddress(ULONG64 Address) const;

    //gets symbols with a matching name
    std::vector<WinSymbol> FromName(const std::string &Name, Sensitivity s = Sensitivity::CaseSensitive) const;

    //gets all symbols from a specified module
    std::vector<WinSymbol> FromModule(ULONG64 ModuleBase) const;
    std::vector<WinSymbol> FromModule(const std::string &ModuleName, Sensitivity s = Sensitivity::CaseSensitive) const;

    //gets all symbols that match a regular expression (slow)
    std::vector<WinSymbol> RegExMatch(const std::regex &re) const;

private:
    static BOOL CALLBACK SymEnumSymbolsProc(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext);

    DWORD processId;

    std::string module_name;
    std::vector<WinSymbol> symbols;
};

class WinSymbol
{
    friend Viviz;
private:
    WinSymbol(PSYMBOL_INFO pi, std::string mn);

public:
    ~WinSymbol();

    std::string name, module_name;
    ULONG64 address, module_address;
    ULONG flags;
};
#endif // VIVIZ_H
