#ifndef LIBPARSER_H
#define LIBPARSER_H

#include <QTextStream>
#include <QStack>
#include <QStringList>
#include <vector>
#include <string>


struct doubleNumber {
    unsigned int divisor=1;
    long long int compNumber = 1;
    int leadZeros = 0;
    int fullPart = 0;
    friend bool operator<(const doubleNumber& left, const doubleNumber& right)
    {
        if(left.fullPart<right.fullPart)
            return true;
        else if(left.fullPart==right.fullPart) {
            if(left.leadZeros > right.leadZeros)
                return true;
            if(left.leadZeros < right.leadZeros)
                return false;
            if(left.leadZeros == right.leadZeros) {
                if(left.divisor == right.divisor) {
                    return left.compNumber < right.compNumber;
                } else if(left.leadZeros < right.leadZeros)
                    return false;
                else
                    return  true;
            }
        }
        else
            return false;
        return  false;
    }
    friend bool operator>(const doubleNumber& left, const doubleNumber& right)
    {
        if(left.fullPart<right.fullPart)
            return false;
        else if(left.fullPart==right.fullPart) {
            if(left.leadZeros > right.leadZeros)
                return false;
            if(left.leadZeros < right.leadZeros)
                return true;
            if(left.leadZeros == right.leadZeros) {
                if(left.divisor == right.divisor) {
                    return left.compNumber > right.compNumber;
                } else if(left.leadZeros < right.leadZeros)
                    return false;
                else
                    return  true;
            }
        }
        else
            return true;
        return  false;
    }
    friend bool operator==(const doubleNumber& left, const doubleNumber& right)
    {
        if ((left.fullPart==right.fullPart) && (left.divisor==right.divisor) && (left.leadZeros == right.leadZeros) &&
                left.compNumber==right.compNumber)
            return true;
        else
            return false;
        return  false;
    }

};

QRegularExpressionMatch getRegExMatch(QString pattern,QString &line);
void removeComments(const std::string& inputFilePath, const std::string& outputFilePath);
doubleNumber getCompleteDouble(QString number);
QVector<doubleNumber> extractValues(QString &series);


struct CellLib {
    QString name;
    QString delay_model;

    QString time_unit;
    QString power_unit;
    QString resistance_unit;
    QString capacitance_unit;
    QString current_unit;
    QString voltage_unit;

    doubleNumber voltage;

    doubleNumber default_cell_leakage_power;
    doubleNumber default_inout_pin_cap;
    doubleNumber default_input_pin_cap;
    doubleNumber default_output_pin_cap;
    doubleNumber default_fanout_load;
    doubleNumber default_max_fanout;
    doubleNumber default_max_transition;

};

struct Lut {

    QString name;

    QVector<doubleNumber> indices1;
    QVector<doubleNumber> indices2;
    QVector<doubleNumber> table;
};

struct InternalPower {

    QString related_pin;

    Lut rise_power;
    Lut fall_power;

};


enum TimingSense {
    NON_UNATE = 0,
    POSITIVE_UNATE,
    NEGATIVE_UNATE
};

TimingSense getTimingSense(QString sense);

enum TimingType {
    COMBINATIONAL = 0,
    COMBINATIONAL_RISE,
    COMBINATIONAL_FALL,
    THREE_STATE_DISABLE,
    THREE_STATE_DISABLE_RISE,
    THREE_STATE_DISABLE_FALL,
    THREE_STATE_ENABLE,
    THREE_STATE_ENABLE_RISE,
    THREE_STATE_ENABLE_FALL,
    RISING_EDGE,
    FALLING_EDGE,
    PRESET,
    CLEAR,
    HOLD_RISING,
    HOLD_FALLING,
    SETUP_RISING,
    SETUP_FALLING,
    RECOVERY_RISING,
    RECOVERY_FALLING,
    SKEW_RISING,
    SKEW_FALLING,
    REMOVAL_RISING,
    REMOVAL_FALLING,
    MIN_PULSE_WIDTH,
    MINIMUM_PERIOD,
    MAX_CLOCK_TREE_PATH,
    MIN_CLOCK_TREE_PATH,
    NON_SEQ_SETUP_RISING,
    NON_SEQ_SETUP_FALLING,
    NON_SEQ_HOLD_RISING,
    NON_SEQ_HOLD_FALLING,
    NOCHANGE_HIGH_HIGH,
    NOCHANGE_HIGH_LOW,
    NOCHANGE_LOW_HIGH,
    NOCHANGE_LOW_LOW,
};

const QMap<QString,TimingType> timing_types = {
    {"combinational",            TimingType::COMBINATIONAL},
    {"combinational_rise",       TimingType::COMBINATIONAL_RISE},
    {"combinational_fall",       TimingType::COMBINATIONAL_FALL},
    {"three_state_disable",      TimingType::THREE_STATE_DISABLE},
    {"three_state_disable_rise", TimingType::THREE_STATE_DISABLE_RISE},
    {"three_state_disable_fall", TimingType::THREE_STATE_DISABLE_FALL},
    {"three_state_enable",       TimingType::THREE_STATE_ENABLE},
    {"three_state_enable_rise",  TimingType::THREE_STATE_ENABLE_RISE},
    {"three_state_enable_fall",  TimingType::THREE_STATE_ENABLE_FALL},
    {"rising_edge",              TimingType::RISING_EDGE},
    {"falling_edge",             TimingType::FALLING_EDGE},
    {"preset",                   TimingType::PRESET},
    {"clear",                    TimingType::CLEAR},
    {"hold_rising",              TimingType::HOLD_RISING},
    {"hold_falling",             TimingType::HOLD_FALLING},
    {"setup_rising",             TimingType::SETUP_RISING},
    {"setup_falling",            TimingType::SETUP_FALLING},
    {"recovery_rising",          TimingType::RECOVERY_RISING},
    {"recovery_falling",         TimingType::RECOVERY_FALLING},
    {"skew_rising",              TimingType::SKEW_RISING},
    {"skew_falling",             TimingType::SKEW_FALLING},
    {"removal_rising",           TimingType::REMOVAL_RISING},
    {"removal_falling",          TimingType::REMOVAL_FALLING},
    {"min_pulse_width",          TimingType::MIN_PULSE_WIDTH},
    {"minimum_period",           TimingType::MINIMUM_PERIOD},
    {"max_clock_tree_path",      TimingType::MAX_CLOCK_TREE_PATH},
    {"min_clock_tree_path",      TimingType::MIN_CLOCK_TREE_PATH},
    {"non_seq_setup_rising",     TimingType::NON_SEQ_SETUP_RISING},
    {"non_seq_setup_falling",    TimingType::NON_SEQ_SETUP_FALLING},
    {"non_seq_hold_rising",      TimingType::NON_SEQ_HOLD_RISING},
    {"non_seq_hold_falling",     TimingType::NON_SEQ_HOLD_FALLING},
    {"nochange_high_high",       TimingType::NOCHANGE_HIGH_HIGH},
    {"nochange_high_low",        TimingType::NOCHANGE_HIGH_LOW},
    {"nochange_low_high",        TimingType::NOCHANGE_LOW_HIGH},
    {"nochange_low_low",         TimingType::NOCHANGE_LOW_LOW}
};

QString getTimingTypeStr(int number);
TimingType getTimingType(QString type);


struct Timing {

    QString related_pin;

    TimingSense sense;
    TimingType type;
    Lut cell_rise;
    Lut cell_fall;
    Lut rise_transition;
    Lut fall_transition;
    Lut rise_constraint;
    Lut fall_constraint;

};

enum class Direction {
    INPUT =0,
    OUTPUT,
    INOUT,
    INTERNAL
};

struct CellPin {

    QString name;
    QString original_pin;

    Direction direction;
    doubleNumber capacitance;      // Pin capacitance
    doubleNumber max_capacitance;  // Max pin capacitance (output pin).
    doubleNumber min_capacitance;  // Min pin capacitance (output pin).
    doubleNumber max_transition;   // Max transition.
    doubleNumber min_transition;   // Min transition.
    doubleNumber fall_capacitance;
    doubleNumber rise_capacitance;
    doubleNumber fanout_load;
    doubleNumber max_fanout;
    doubleNumber min_fanout;
    doubleNumber is_clock;          // Is clock pin.

    QVector<Timing> timings;
    QVector<InternalPower> internalPower;

};


struct CellStd {

    QString name;
    QString cell_footprint;

    doubleNumber leakage_power;
    doubleNumber area;
    QMap <QString,CellPin> cellPins;

};


class LIBParser
{
public:
    LIBParser();
    ~LIBParser();

    bool Parse ( QTextStream  *stream);

    void setLutType(QString type);
    int  getLutType();
    CellLib getStdLib() {return stdLib;};
    QVector<CellStd>getStdCellsVec() {return libCellsVector;};


    QString Unit() const{ return m_unit; }
    int Time() const{ return m_time; }

    void exportToFile(QStringList &list);
    void setOutputFileName(QString fName);

private:

    QString m_unit;
    int m_time;
    int lutType;

    bool extracted_pins_finish;
    QString exportFileName;

    CellLib stdLib;
    QVector <CellStd> libCellsVector;
    QStack<QString> m_stack;

    qulonglong m_currentPosition;
    QList<QString> tempStrList;

    void setUnit(QString val) { m_unit = val ; }
    void setTime(int val) { m_time = val ; }

private:

};

#endif
