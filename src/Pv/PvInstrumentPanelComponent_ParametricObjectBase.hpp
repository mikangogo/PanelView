#pragma once

#include "PvInstrumentPanelDefinitions.hpp"

class PvInstrumentPanelComponent_ParametricObjectBase
{
public:
    PvInstrumentPanelComponent_ParametricObjectBase() = default;

    void SetSubject(const PvInstrumentPanelSubjectKeyTitle parameterType) { _subjectType = parameterType; }
    void SetSubjectIndex(const unsigned int index) { _subjectIndex = index; }
    void SetParameter(const double value) { _value = value; }


    PvInstrumentPanelSubjectKeyTitle GetSubjectType() const { return _subjectType; }
    unsigned int GetSubjectIndex() const { return _subjectIndex; }
    double GetParameter() const { return _value; }

protected:
    static int GetDigitValue(const int sourceValue, const int digit)
    {
        auto value = 0;

        switch (digit)
        {
            case 0:
                value = sourceValue % 10;
                break;
            case 1:
                value = (sourceValue / 10) % 10;
                break;
            case 2:
                value = (sourceValue / 100) % 10;
                break;
            default:
                break;
        }

        return value;
    }

    static int GetAtsValue(const int* const atsArray, const int index)
    {
        return atsArray[index];
    }

    void SetParameterFromDoorState(const bool isClosed)
    {
        switch (GetSubjectType())
        {
            case PvInstrumentPanelSubjectKeyTitle_Door:
                {
                    SetParameter(isClosed ? 1.0 : 0.0);
                }
                break;
            default:
                break;
        }
    }

    void SetParameterFromTractionControllerPosition(const int position)
    {
        switch (GetSubjectType())
        {
            case PvInstrumentPanelSubjectKeyTitle_Power:
                {
                    SetParameter(position);
                }
                break;
            default:
                break;
        }
    }

    void SetParameterFromBrakeControllerPosition(const int position)
    {
        switch (GetSubjectType())
        {
            case PvInstrumentPanelSubjectKeyTitle_Brake:
                {
                    SetParameter(position);
                }
                break;
            default:
                break;
        }
    }

    void SetParameterFromUpdateInfo(const PvBridgeUpdateInfo& updateInfo)
    {
        switch (GetSubjectType())
        {
            case PvInstrumentPanelSubjectKeyTitle_Nop:
                {
                    SetParameter(0.0);
                }
                break;
            case PvInstrumentPanelSubjectKeyTitle_True:
                {
                    SetParameter(1.0);
                }
                break;
            case PvInstrumentPanelSubjectKeyTitle_Kmph:
                {
                    SetParameter(updateInfo.Speed);
                }
                break;
            case PvInstrumentPanelSubjectKeyTitle_Kmphd:
                {
                    const auto value = GetDigitValue(static_cast<int>(updateInfo.Speed), GetSubjectIndex());
                    SetParameter(value);
                }
                break;
            case PvInstrumentPanelSubjectKeyTitle_Bc:
                {
                    SetParameter(updateInfo.BcPressure);
                }
                break;
            case PvInstrumentPanelSubjectKeyTitle_Bcd:
                {
                    const auto value = GetDigitValue(static_cast<int>(updateInfo.BcPressure), GetSubjectIndex());
                    SetParameter(value);
                }
                break;
            case PvInstrumentPanelSubjectKeyTitle_Mr:
                {
                    SetParameter(updateInfo.MrPressure);
                }
                break;
            case PvInstrumentPanelSubjectKeyTitle_Mrd:
                {
                    const auto value = GetDigitValue(static_cast<int>(updateInfo.BcPressure), GetSubjectIndex());
                    SetParameter(value);
                }
                break;
            case PvInstrumentPanelSubjectKeyTitle_Sap:
                {
                    SetParameter(updateInfo.SapPressure);
                }
                break;
            case PvInstrumentPanelSubjectKeyTitle_Bp:
                {
                    SetParameter(updateInfo.BpPressure);
                }
                break;
            case PvInstrumentPanelSubjectKeyTitle_Er:
                {
                    SetParameter(updateInfo.ErPressure);
                }
                break;
            case PvInstrumentPanelSubjectKeyTitle_Am:
                {
                    SetParameter(updateInfo.Current);
                }
                break;
            case PvInstrumentPanelSubjectKeyTitle_Amabs:
                {
                    SetParameter(std::fabs(updateInfo.Current));
                }
                break;

            case PvInstrumentPanelSubjectKeyTitle_Doorl:
                break;
            case PvInstrumentPanelSubjectKeyTitle_Doorr:
                break;

            case PvInstrumentPanelSubjectKeyTitle_Csc:
                break;

            case PvInstrumentPanelSubjectKeyTitle_Ats:
                {
                    const auto value = GetAtsValue(updateInfo.PanelArray, GetSubjectIndex());
                    SetParameter(value);
                }
                break;

            case PvInstrumentPanelSubjectKeyTitle_Power:
                break;
            case PvInstrumentPanelSubjectKeyTitle_Brake:
                break;

            case PvInstrumentPanelSubjectKeyTitle_Hour:
                {
                    constexpr auto div = 1000 * 3600;
                    const auto value = updateInfo.Time / div;
                    SetParameter(value);
                }
                break;
            case PvInstrumentPanelSubjectKeyTitle_Min:
                {
                    constexpr auto div = 1000 * 60;
                    const auto value = updateInfo.Time / div;
                    SetParameter(value);
                }
                break;
            case PvInstrumentPanelSubjectKeyTitle_Sec:
                {
                    constexpr auto div = 1000;
                    const auto value = updateInfo.Time / div;
                    SetParameter(value);
                }
                break;
        }
    }

private:
    PvInstrumentPanelSubjectKeyTitle _subjectType = PvInstrumentPanelSubjectKeyTitle_Nop;
    unsigned int _subjectIndex = 0;
    double _value = 0.0;
};
