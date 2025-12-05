#include "LIB_parser.h"

#include "qfile.h"
#include <QStringList>
#include <QString>
#include <QHash>
#include <qmath.h>
#include <QDebug>
#include <QRegExp>
#include <QRegularExpression>
#include <QStack>

#define MAX_ALLOWABLE_JITTER 1.0
#define SCOPE_JOIN_STR " : "

LIBParser::LIBParser()
{
    m_currentPosition = 0;
    extracted_pins_finish =0;

    lutType = -1;
}

doubleNumber getCompleteDouble(QString number)
{
    doubleNumber result;
    QStringList splitNumber = number.split(".",QString::SplitBehavior::SkipEmptyParts);
    if(2==splitNumber.size()) {
        QString div ="1";
        for(int i=0;i<splitNumber[1].size();++i) {
            div +="0";
        }
        result.divisor = div.toInt();
        result.compNumber = QString(splitNumber[0]+splitNumber[1]).toInt();
        result.fullPart = splitNumber[0].toInt();
        unsigned divPart = splitNumber[1].toInt();
        result.leadZeros =   splitNumber[1].size() - QString::number(divPart).size();
    }
    else {
        result.divisor = 1;
        result.compNumber = number.toInt();
        result.fullPart = number.toInt();
        result.leadZeros = 0;
    }
    return  result;
}

LIBParser::~LIBParser() {

}


bool LIBParser::Parse(QTextStream  *stream)
{

    QString line;
    unsigned lineNum = 1;

    while ( true )
    {
        line = stream->readLine();

        if ( line.isNull() )
            break;

        line = line.trimmed();

        if ( line.isEmpty() )
            continue;

        //  printf(" >>>>>>>>>>>>>>> LINE = %d\n",lineNum);

        if(line.startsWith("library ",Qt::CaseSensitive) )
        {
            //printf(" <library> found!\n");
            QRegularExpressionMatch matchPer = getRegExMatch("[(]([^)]*)", line);
            if (matchPer.hasMatch()) {
                QString matched = matchPer.captured(0);
                QString name = matchPer.captured(1);
                // printf("matched =%s  period = %s\n",matched.toStdString().c_str(),name.toStdString().c_str());
                stdLib.name = name;
                continue;
            }
        }

        if(line.startsWith("delay_model",Qt::CaseSensitive) )
        {
            // printf(" <delay_model> found!\n");
            QRegularExpressionMatch matchName = getRegExMatch(":\\s*([^;]+);", line);
            if (matchName.hasMatch()) {
                QString matched = matchName.captured(0);
                QString name = matchName.captured(1);
                // printf("matched =%s  name = %s\n",matched.toStdString().c_str(),name.toStdString().c_str());
                stdLib.delay_model = name;
            }
            continue;
        }

        if(line.startsWith("time_unit",Qt::CaseSensitive) )
        {
            //printf(" <time_unit> found!\n");
            QRegularExpressionMatch matchName = getRegExMatch("\"([^\"]+)\"", line);
            if (matchName.hasMatch()) {
                QString matched = matchName.captured(0);
                QString name = matchName.captured(1);
                // printf("matched =%s  name = %s\n",matched.toStdString().c_str(),name.toStdString().c_str());
                stdLib.time_unit = name;
            }
            continue;
        }

        if(line.startsWith("leakage_power_unit",Qt::CaseSensitive) )
        {
            // printf(" <power_unit> found!\n");
            QRegularExpressionMatch matchName = getRegExMatch(":\\s*([^;]+);", line);
            if (matchName.hasMatch()) {
                QString matched = matchName.captured(0);
                QString name = matchName.captured(1);
                //  printf("matched =%s  name = %s\n",matched.toStdString().c_str(),name.toStdString().c_str());
                stdLib.power_unit = name;
            }
            continue;
        }


        if(line.startsWith("pulling_resistance_unit",Qt::CaseSensitive) )
        {
            // printf(" <resistance_unit> found!\n");
            QRegularExpressionMatch matchName = getRegExMatch("\"([^\"]+)\"", line);
            if (matchName.hasMatch()) {
                QString matched = matchName.captured(0);
                QString name = matchName.captured(1);
                // printf("matched =%s  name = %s\n",matched.toStdString().c_str(),name.toStdString().c_str());
                stdLib.resistance_unit = name;
            }
            continue;
        }

        if(line.startsWith("capacitive_load_unit",Qt::CaseSensitive) )
        {
            // printf(" <capacitance_unit> found!\n");
            QRegularExpressionMatch matchName = getRegExMatch("[(]([^)]*)", line);
            if (matchName.hasMatch()) {
                QString matched = matchName.captured(0);
                QString name = matchName.captured(1);
                //  printf("matched =%s  name = %s\n",matched.toStdString().c_str(),name.toStdString().c_str());
                stdLib.capacitance_unit = name;
            }
            continue;
        }


        if(line.startsWith("current_unit",Qt::CaseSensitive) )
        {
            // printf(" <current_unit> found!\n");
            QRegularExpressionMatch matchName = getRegExMatch("\"([^\"]+)\"", line);
            if (matchName.hasMatch()) {
                QString matched = matchName.captured(0);
                QString name = matchName.captured(1);
                //    printf("matched =%s  name = %s\n",matched.toStdString().c_str(),name.toStdString().c_str());
                stdLib.current_unit = name;
            }
            continue;
        }

        if(line.startsWith("voltage_unit",Qt::CaseSensitive) )
        {
            //  printf(" <voltage_unit> found!\n");
            QRegularExpressionMatch matchName = getRegExMatch("\"([^\"]+)\"", line);
            if (matchName.hasMatch()) {
                QString matched = matchName.captured(0);
                QString name = matchName.captured(1);
                //   printf("matched =%s  name = %s\n",matched.toStdString().c_str(),name.toStdString().c_str());
                stdLib.voltage_unit = name;
            }
            continue;
        }

        if(line.startsWith("nom_voltage",Qt::CaseSensitive))
        {
            // printf(" <voltage> found!\n");
            QRegularExpressionMatch matchPer = getRegExMatch(":\\s*([^;]+);", line);
            if (matchPer.hasMatch()) {
                QString matched = matchPer.captured(0);
                QString period = matchPer.captured(1);
                //   printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                doubleNumber numD = getCompleteDouble(period);
                stdLib.voltage = numD;
            }
            continue;
        }


        if(line.startsWith("default_cell_leakage_power",Qt::CaseSensitive))
        {
            //   printf(" <default_cell_leakage_power> found!\n");
            QRegularExpressionMatch matchPer = getRegExMatch(":\\s*([^;]+);", line);
            if (matchPer.hasMatch()) {
                QString matched = matchPer.captured(0);
                QString period = matchPer.captured(1);
                //    printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                //stdLib.default_cell_leakage_power = period.toFloat();
                doubleNumber numD = getCompleteDouble(period);
                stdLib.default_cell_leakage_power = numD;
            }
            continue;
        }


        if(line.startsWith("default_inout_pin_cap",Qt::CaseSensitive))
        {
            //  printf(" <default_inout_pin_cap> found!\n");
            QRegularExpressionMatch matchPer = getRegExMatch(":\\s*(.*)$", line);
            if (matchPer.hasMatch()) {
                QString matched = matchPer.captured(0);
                QString period = matchPer.captured(1);
                //    printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                doubleNumber numD = getCompleteDouble(period);
                stdLib.default_inout_pin_cap = numD;
            }
            continue;
        }


        if(line.startsWith("default_input_pin_cap",Qt::CaseSensitive))
        {
            // printf(" <default_input_pin_cap> found!\n");
            QRegularExpressionMatch matchPer = getRegExMatch(":\\s*(.*)$", line);
            if (matchPer.hasMatch()) {
                QString matched = matchPer.captured(0);
                QString period = matchPer.captured(1);
                //  printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                doubleNumber numD = getCompleteDouble(period);
                stdLib.default_input_pin_cap = numD;
            }
            continue;
        }


        if(line.startsWith("default_output_pin_cap",Qt::CaseSensitive))
        {
            // printf(" <default_output_pin_cap> found!\n");
            QRegularExpressionMatch matchPer = getRegExMatch(":\\s*(.*)$", line);
            if (matchPer.hasMatch()) {
                QString matched = matchPer.captured(0);
                QString period = matchPer.captured(1);
                //  printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                doubleNumber numD = getCompleteDouble(period);
                stdLib.default_output_pin_cap = numD;
            }
            continue;
        }


        if(line.startsWith("default_fanout_load",Qt::CaseSensitive))
        {
            //  printf(" <default_fanout_load> found!\n");
            QRegularExpressionMatch matchPer = getRegExMatch(":\\s*([^;]+);", line);
            if (matchPer.hasMatch()) {
                QString matched = matchPer.captured(0);
                QString period = matchPer.captured(1);
                //  printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                doubleNumber numD = getCompleteDouble(period);
                stdLib.default_fanout_load = numD;
            }
            continue;
        }


        if(line.startsWith("default_max_fanout",Qt::CaseSensitive))
        {
            //printf(" <default_max_fanout> found!\n");
            QRegularExpressionMatch matchPer = getRegExMatch(":\\s*([^;]+);", line);
            if (matchPer.hasMatch()) {
                QString matched = matchPer.captured(0);
                QString period = matchPer.captured(1);
                // printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                doubleNumber numD = getCompleteDouble(period);
                stdLib.default_max_fanout = numD;
            }
            continue;
        }


        if(line.startsWith("default_max_transition",Qt::CaseSensitive))
        {
            // printf(" <default_max_transition> found!\n");
            QRegularExpressionMatch matchPer = getRegExMatch(":\\s*(.*)$", line);
            if (matchPer.hasMatch()) {
                QString matched = matchPer.captured(0);
                QString period = matchPer.captured(1);
                //  printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                doubleNumber numD = getCompleteDouble(period);
                stdLib.default_max_transition = numD;
            }
            continue;
        }


        // PARSE CELL!!!

        if(line.startsWith("cell ",Qt::CaseSensitive))
        {
            //printf(" <cell> found!\n");
            CellStd foundCell;
            QRegularExpressionMatch matchPer = getRegExMatch("[(]([^)]*)", line);
            if (matchPer.hasMatch()) {
                QString matched = matchPer.captured(0);
                QString period = matchPer.captured(1);
                //printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                foundCell.name = period;
            }
            QStack<int> openBracket;
            QStack<int> closeBracket;
            int numOpenBr = 0;
            int numCloseBr =0;
            if(!line.contains('{'))
                continue;
            else {
                openBracket.push(numOpenBr+1);
                while ( true )
                {
                    line = stream->readLine();
                    if ( line.isNull() )
                        break;

                    line = line.trimmed();
                    if ( line.isEmpty() )
                        continue;
                    if(line.contains('{')) {
                        numOpenBr++;
                        openBracket.push(numOpenBr);
                    }
                    if(line.contains('}')) {
                        numCloseBr++;
                        closeBracket.push(numCloseBr);
                    }
                    if(openBracket.size()==closeBracket.size() && openBracket.size() && closeBracket.size())
                        break;
                    lineNum++;
                    if(line.startsWith("area ",Qt::CaseSensitive))
                    {
                        //printf(" <area> found!\n");
                        QRegularExpressionMatch matchPer = getRegExMatch(":\\s*(.*)$", line);
                        if (matchPer.hasMatch()) {
                            QString matched = matchPer.captured(0);
                            QString period = matchPer.captured(1);
                            // printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                            foundCell.area =  getCompleteDouble(period);
                            continue;
                        }
                    }

                    if(line.startsWith("cell_footprint ",Qt::CaseSensitive))
                    {
                        // printf(" <cell_footprint> found!\n");
                        QRegularExpressionMatch matchPer = getRegExMatch(":\\s*(.*)$", line);
                        if (matchPer.hasMatch()) {
                            QString matched = matchPer.captured(0);
                            QString period = matchPer.captured(1);
                            //  printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                            foundCell.cell_footprint =  period;
                            continue;
                        }
                    }

                    if(line.startsWith("leakage_power ",Qt::CaseSensitive))
                    {
                        //printf(" <leakage_power> found!\n");
                        continue;
                    }
                    if(line.startsWith("value ",Qt::CaseSensitive))
                    {
                        // printf(" <leakage_power:value> found!\n");
                        QRegularExpressionMatch matchPer = getRegExMatch(":\\s*([^;]+);", line);
                        if (matchPer.hasMatch()) {
                            QString matched = matchPer.captured(0);
                            QString period = matchPer.captured(1);
                            // printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                            foundCell.leakage_power<getCompleteDouble(period)? getCompleteDouble(period):foundCell.leakage_power ;
                            continue;
                        }
                    }

                    if(line.startsWith("pin",Qt::CaseSensitive))
                    {
                        // printf(" <pin> found!\n");
                        CellPin foundPin;
                        QRegularExpressionMatch matchPer = getRegExMatch("[(]([^)]*)", line);
                        if (matchPer.hasMatch()) {
                            QString matched = matchPer.captured(0);
                            QString period = matchPer.captured(1);
                            // printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                            foundPin.name = period;
                        }
                        QStack<int> openBracket;
                        QStack<int> closeBracket;
                        int numOpenBr = 0;
                        int numCloseBr =0;
                        if(!line.contains('{'))
                            continue;
                        else {
                            openBracket.push(numOpenBr+1);
                            while ( true )
                            {
                                line = stream->readLine();
                                if ( line.isNull() )
                                    break;

                                line = line.trimmed();
                                if ( line.isEmpty() )
                                    continue;
                                if(line.contains('{')) {
                                    numOpenBr++;
                                    openBracket.push(numOpenBr);
                                }
                                if(line.contains('}')) {
                                    numCloseBr++;
                                    closeBracket.push(numCloseBr);
                                }
                                if(openBracket.size()==closeBracket.size()&& openBracket.size() && closeBracket.size())
                                    break;
                                lineNum++;

                                if(line.startsWith("timing ",Qt::CaseSensitive))
                                {
                                    //  printf(" <timing> found!\n");
                                    Timing foundTiming;
                                    Lut    foundLut;
                                    QStack<int> openBracket;
                                    QStack<int> closeBracket;
                                    int numOpenBr = 0;
                                    int numCloseBr =0;
                                    if(!line.contains('{'))
                                        continue;
                                    else {
                                        openBracket.push(numOpenBr+1);
                                        while ( true )
                                        {
                                            line = stream->readLine();
                                            if ( line.isNull() )
                                                break;

                                            line = line.trimmed();
                                            if ( line.isEmpty() )
                                                continue;
                                            if(line.contains('{')) {
                                                numOpenBr++;
                                                openBracket.push(numOpenBr);
                                            }
                                            if(line.contains('}')) {
                                                numCloseBr++;
                                                closeBracket.push(numCloseBr);
                                            }
                                            if(openBracket.size()==closeBracket.size()&& openBracket.size() && closeBracket.size())
                                                break;
                                            lineNum++;




                                            if(line.startsWith("related_pin",Qt::CaseSensitive))
                                            {
                                                //printf(" <related_pin> found!\n");
                                                QRegularExpressionMatch matchPer = getRegExMatch("\"([^\"]+)\"", line);
                                                if (matchPer.hasMatch()) {
                                                    QString matched = matchPer.captured(0);
                                                    QString period = matchPer.captured(1);
                                                    // printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                                                    QString relatedPin = period;
                                                    foundTiming.related_pin = relatedPin;
                                                }
                                            }

                                            if(line.startsWith("sense",Qt::CaseSensitive))
                                            {
                                                // printf(" <sense> found!\n");
                                                QRegularExpressionMatch matchPer = getRegExMatch(":\\s*([^;]+);", line);
                                                if (matchPer.hasMatch()) {
                                                    QString matched = matchPer.captured(0);
                                                    QString period = matchPer.captured(1);
                                                    //  printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                                                    QString relatedPin = period;
                                                    foundTiming.sense =  getTimingSense(period);
                                                }
                                            }
                                            if(line.startsWith("timing_type",Qt::CaseSensitive))
                                            {
                                                //printf(" <timing_type> found!\n");
                                                QRegularExpressionMatch matchPer = getRegExMatch(":\\s*([^;]+);", line);
                                                if (matchPer.hasMatch()) {
                                                    QString matched = matchPer.captured(0);
                                                    QString period = matchPer.captured(1);
                                                    // printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                                                    QString relatedPin = period;
                                                    foundTiming.type =  getTimingType(period);
                                                }
                                            }

                                            if(line.startsWith("index_1",Qt::CaseSensitive))
                                            {
                                                //printf(" <index_1> found!\n");
                                                QRegularExpressionMatch matchPer = getRegExMatch("\"([^\"]+)", line);
                                                if (matchPer.hasMatch()) {
                                                    QString matched = matchPer.captured(0);
                                                    QString period = matchPer.captured(1);
                                                    QVector<doubleNumber> valuesVec = extractValues(period);
                                                    foundLut.indices1 = valuesVec;
                                                }
                                            }
                                            if(line.startsWith("index_2",Qt::CaseSensitive))
                                            {
                                                // printf(" <index_2> found!\n");
                                                QRegularExpressionMatch matchPer = getRegExMatch("\"([^\"]+)", line);
                                                if (matchPer.hasMatch()) {
                                                    QString matched = matchPer.captured(0);
                                                    QString period = matchPer.captured(1);
                                                    QVector<doubleNumber> valuesVec = extractValues(period);
                                                    foundLut.indices2 = valuesVec;
                                                }
                                            }


                                            if(line.startsWith("cell_rise",Qt::CaseSensitive))
                                            {
                                                // printf(" <cell_rise> found!\n");
                                                setLutType("cell_rise");
                                                continue;
                                            }

                                            if(line.startsWith("rise_transition",Qt::CaseSensitive))
                                            {
                                                // printf(" <rise_transition> found!\n");
                                                setLutType("rise_transition");
                                                continue;
                                            }

                                            if(line.startsWith("cell_fall",Qt::CaseSensitive))
                                            {
                                                // printf(" <cell_fall> found!\n");
                                                setLutType("cell_fall");
                                                continue;
                                            }
                                            if(line.startsWith("fall_transition",Qt::CaseSensitive))
                                            {
                                                // printf(" <fall_transition> found!\n");
                                                setLutType("fall_transition");
                                                continue;
                                            }

                                            if(line.startsWith("values ",Qt::CaseSensitive))
                                            {
                                                // printf(" <values:cell_rise> found!\n");
                                                bool openBracket = false;
                                                bool closeBracket = false;
                                                if(line.contains('('))
                                                    openBracket = true;
                                                if(line.contains('('))
                                                    while ( !closeBracket )
                                                    {
                                                        line = stream->readLine();
                                                        if ( line.isNull() )
                                                            break;

                                                        line = line.trimmed();
                                                        if ( line.isEmpty() )
                                                            continue;
                                                        lineNum++;
                                                        if(line.contains(')')) {
                                                            QRegularExpressionMatch matchPer = getRegExMatch("\"([^\"]+)\"", line);
                                                            if (matchPer.hasMatch()) {
                                                                QString matched = matchPer.captured(0);
                                                                QString period = matchPer.captured(1);
                                                                QVector<doubleNumber> valuesVec = extractValues(matched);
                                                                foundLut.table = valuesVec;
                                                            }
                                                            closeBracket = true;
                                                        }  else  {
                                                            QRegularExpressionMatch matchPer = getRegExMatch("([^\"]+)", line);
                                                            if (matchPer.hasMatch()) {
                                                                QString matched = matchPer.captured(0);
                                                                QString period = matchPer.captured(1);
                                                                QVector<doubleNumber> valuesVec = extractValues(matched);
                                                                foundLut.table = valuesVec;
                                                            }
                                                        }
                                                    } }

                                        }  }
                                    int lutType = getLutType();
                                    switch (lutType) {
                                    case 0: {
                                        foundTiming.cell_rise = foundLut;
                                        break;
                                    }
                                    case 1: {
                                        foundTiming.cell_fall = foundLut;
                                        break;
                                    }
                                    case 2: {
                                        foundTiming.rise_transition = foundLut;
                                        break;
                                    }
                                    case 3: {
                                        foundTiming.fall_transition = foundLut;
                                        break;
                                    }
                                    case 4: {
                                        foundTiming.rise_constraint = foundLut;
                                        break;
                                    }
                                    case 5: {
                                        foundTiming.fall_constraint = foundLut;
                                        break;
                                    }
                                    default:
                                        printf("Unknown values!\n");
                                    }
                                    foundPin.timings.push_back(foundTiming);
                                }



                                if(line.startsWith("internal_power ",Qt::CaseSensitive))
                                {
                                    // printf(" <internal_power> found!\n");
                                    InternalPower foundPower;
                                    Lut    foundLut;
                                    bool isRisePowerLut = false;
                                    QStack<int> openBracket;
                                    QStack<int> closeBracket;
                                    int numOpenBr = 0;
                                    int numCloseBr =0;
                                    if(!line.contains('{'))
                                        continue;
                                    else {
                                        openBracket.push(numOpenBr+1);
                                        while ( true )
                                        {
                                            line = stream->readLine();
                                            if ( line.isNull() )
                                                break;

                                            line = line.trimmed();
                                            if ( line.isEmpty() )
                                                continue;
                                            if(line.contains('{')) {
                                                numOpenBr++;
                                                openBracket.push(numOpenBr);
                                            }
                                            if(line.contains('}')) {
                                                numCloseBr++;
                                                closeBracket.push(numCloseBr);
                                            }
                                            if(openBracket.size()==closeBracket.size()&& openBracket.size() && closeBracket.size())
                                                break;
                                            lineNum++;




                                            if(line.startsWith("related_pin",Qt::CaseSensitive))
                                            {
                                                // printf(" <related_pin> found!\n");
                                                QRegularExpressionMatch matchPer = getRegExMatch("\"([^\"]+)\"", line);
                                                if (matchPer.hasMatch()) {
                                                    QString matched = matchPer.captured(0);
                                                    QString period = matchPer.captured(1);
                                                    // printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                                                    QString relatedPin = period;
                                                    foundPower.related_pin = relatedPin;
                                                }
                                            }


                                            if(line.startsWith("index_1",Qt::CaseSensitive))
                                            {
                                                // printf(" <index_1> found!\n");
                                                QRegularExpressionMatch matchPer = getRegExMatch("\"([^\"]+)", line);
                                                if (matchPer.hasMatch()) {
                                                    QString matched = matchPer.captured(0);
                                                    QString period = matchPer.captured(1);
                                                    QVector<doubleNumber> valuesVec = extractValues(period);
                                                    foundLut.indices1 = valuesVec;
                                                }
                                            }
                                            if(line.startsWith("index_2",Qt::CaseSensitive))
                                            {
                                                //printf(" <index_2> found!\n");
                                                QRegularExpressionMatch matchPer = getRegExMatch("\"([^\"]+)", line);
                                                if (matchPer.hasMatch()) {
                                                    QString matched = matchPer.captured(0);
                                                    QString period = matchPer.captured(1);
                                                    QVector<doubleNumber> valuesVec = extractValues(period);
                                                    foundLut.indices2 = valuesVec;
                                                }
                                            }


                                            if(line.startsWith("rise_power",Qt::CaseSensitive))
                                            {
                                                // printf(" <rise_power> found!\n");
                                                isRisePowerLut = true;
                                                continue;
                                            }

                                            if(line.startsWith("fall_power",Qt::CaseSensitive))
                                            {
                                                //printf(" <fall_power> found!\n");
                                                isRisePowerLut = false;
                                                continue;
                                            }

                                            if(line.startsWith("values ",Qt::CaseSensitive))
                                            {
                                                //printf(" <values:cell_rise> found!\n");
                                                bool openBracket = false;
                                                bool closeBracket = false;
                                                if(line.contains('('))
                                                    openBracket = true;
                                                if(line.contains('('))
                                                    while ( !closeBracket )
                                                    {
                                                        line = stream->readLine();
                                                        if ( line.isNull() )
                                                            break;

                                                        line = line.trimmed();
                                                        if ( line.isEmpty() )
                                                            continue;
                                                        lineNum++;
                                                        if(line.contains(')')) {
                                                            QRegularExpressionMatch matchPer = getRegExMatch("\"([^\"]+)\"", line);
                                                            if (matchPer.hasMatch()) {
                                                                QString matched = matchPer.captured(0);
                                                                QString period = matchPer.captured(1);
                                                                QVector<doubleNumber> valuesVec = extractValues(matched);
                                                                foundLut.table = valuesVec;
                                                            }
                                                            closeBracket = true;
                                                        }  else  {
                                                            QRegularExpressionMatch matchPer = getRegExMatch("([^\"]+)", line);
                                                            if (matchPer.hasMatch()) {
                                                                QString matched = matchPer.captured(0);
                                                                QString period = matchPer.captured(1);
                                                                QVector<doubleNumber> valuesVec = extractValues(matched);
                                                                foundLut.table = valuesVec;
                                                            }
                                                        }
                                                    } }

                                        }  }
                                    if(isRisePowerLut)
                                        foundPower.rise_power = foundLut;
                                    else
                                        foundPower.fall_power = foundLut;
                                    foundPin.internalPower.push_back(foundPower);
                                }






                            }
                        }


                        foundCell.cellPins.insert(foundPin.name,foundPin);
                    }





                }
            }

            libCellsVector.push_back(foundCell);

        } //*/
        lineNum++;



    }

    return true;
}

void LIBParser::exportToFile(QStringList &list) {

    if(exportFileName.isEmpty()) {
        printf("Output file not specified!\n") ;
        return;
    }
    FILE * pFile;
    pFile = fopen (exportFileName.toStdString().c_str(),"w");
    if(!pFile)
    {
        printf("Error in export file open\n") ;
        return;
    }
    foreach (QString str,list) {
        fprintf(pFile,str.toStdString().c_str());
    }
    fclose(pFile);
}


QRegularExpressionMatch getRegExMatch(QString pattern,QString &line)
{
    QString matched;
    QRegularExpression re(pattern);
    QRegularExpressionMatch match = re.match(line);
    if (match.hasMatch()) {
        matched = match.captured(0);
        return  match;
    }
    return  match;
}

#include <fstream>
#include <string>
#include <iostream>

void removeComments(const std::string& inputFilePath, const std::string& outputFilePath) {
    std::ifstream inputFile(inputFilePath);
    if (!inputFile) {
        std::cerr << "Failed to open input file." << std::endl;
        return;
    }

    std::ofstream outputFile(outputFilePath);
    if (!outputFile) {
        std::cerr << "Failed to open output file." << std::endl;
        return;
    }

    std::string line;
    bool inBlockComment = false;
    std::string buffer;

    while (std::getline(inputFile, line)) {
        size_t i = 0;
        std::string cleanLine;

        while (i < line.size()) {
            if (inBlockComment) {
                // Look for end of block comment
                size_t endComment = line.find("*/", i);
                if (endComment != std::string::npos) {
                    inBlockComment = false;
                    i = endComment + 2;
                } else {
                    // Still in comment, skip the rest of the line
                    break;
                }
            } else {
                // Not in a comment
                size_t singleLinePos = line.find("//", i);
                size_t blockStartPos = line.find("/*", i);

                if (singleLinePos != std::string::npos &&
                        (blockStartPos == std::string::npos || singleLinePos < blockStartPos)) {
                    // Single line comment starts before block comment
                    cleanLine += line.substr(i, singleLinePos - i);
                    break; // Rest of line is comment
                } else if (blockStartPos != std::string::npos) {
                    // Block comment starts here
                    cleanLine += line.substr(i, blockStartPos - i);
                    i = blockStartPos + 2;
                    inBlockComment = true;
                } else {
                    // No comments ahead
                    cleanLine += line.substr(i);
                    break;
                }
            }
        }
        if (!cleanLine.empty() && !inBlockComment) {
            outputFile << cleanLine << "\n";
        }
        // If inBlockComment at line end, continue to next line
    }

    inputFile.close();
    outputFile.close();
}

QVector<doubleNumber> extractValues(QString &series) {
    QVector<doubleNumber> retVector;
    QStringList splitVector = series.split(",",QString::SkipEmptyParts);
    foreach(QString num, splitVector)
        retVector << getCompleteDouble(num);
    return  retVector;
}

void LIBParser::setLutType(QString type) {
    if(type=="cell_rise")
        lutType = 0;
    else if(type=="cell_fall")
        lutType = 1;
    else if(type=="rise_transition")
        lutType = 2;
    else if(type=="fall_transition")
        lutType = 3;
    else if(type=="rise_constraint")
        lutType = 4;
    else if(type=="fall_constraint")
        lutType = 5;
    else
        lutType = -1;
    return;

}

int LIBParser::getLutType() {
    return  lutType;
}

TimingSense getTimingSense(QString sense) {
    if(sense =="positive_unate")
        return TimingSense::POSITIVE_UNATE;
    else if(sense =="negative_unate")
        return TimingSense::NEGATIVE_UNATE;
    else if(sense =="non_unate")
        return TimingSense::NON_UNATE;
    return TimingSense::NON_UNATE;
}

QString getTimingTypeStr(int number) {
    return timing_types.key((TimingType)number);
}

TimingType getTimingType(QString type) {
    return timing_types.value(type);
}
