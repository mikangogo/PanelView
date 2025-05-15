#pragma once

#include "PvPf_Windows.hpp"

typedef void* PvWindowDataPointer;

class PvWindowBase : public PvPfWindowEventOperator
{
public:
    PvWindowBase();
    virtual ~PvWindowBase();

    void Create(const char8_t* windowName);
    void Delete();
    
protected:
    bool OnReceiveMessage() override;

private:
    PvWindowDataPointer _windowData;
};
