#include "SDC_parser.h"

#include "qfile.h"
#include <QStringList>
#include <QString>
#include <QHash>
#include <qmath.h>
#include <QDebug>
#include <QRegExp>
#include <QRegularExpression>


SDCParser::SDCParser()
{
    m_currentPosition = 0;
}


SDCParser::~SDCParser() {
}


bool SDCParser::Parse(QTextStream  *stream)
{
    QString line;

    while ( true )
    {
        line = stream->readLine();

        if ( line.isNull() )
            break;

        line = line.trimmed();

        if ( line.isEmpty() )
            continue;

        // Process this line

        if(line.startsWith("create_clock",Qt::CaseSensitive) )
        {
            //printf(" <create_clock> found!\n");
            createClkStruct foundClk;
            QRegularExpressionMatch matchPer = getRegExMatchSDC("\\-period\\s+(\\d+)", line);
            if (matchPer.hasMatch()) {
                QString matched = matchPer.captured(0);
                QString period = matchPer.captured(1);
                // printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                foundClk.period = period.toInt();
            }
            QRegularExpressionMatch matchName = getRegExMatchSDC("\\-name\\s+(\\S+)", line);
            if (matchName.hasMatch()) {
                QString matched = matchName.captured(0);
                QString name = matchName.captured(1);
                // printf("matched =%s  name = %s\n",matched.toStdString().c_str(),name.toStdString().c_str());
                foundClk.name = name;
            }
            QRegularExpressionMatch matchGetPort = getRegExMatchSDC("get_ports\\s+(\\S+)", line);
            if (matchGetPort.hasMatch()) {
                QString matched = matchGetPort.captured(0);
                QString getPort = matchGetPort.captured(1);
                if(getPort.endsWith(']'))
                    getPort.chop(1);
                // printf("matched =%s  getPort = %s\n",matched.toStdString().c_str(),getPort.toStdString().c_str());
                foundClk.get_ports = getPort;
            }
            m_createClkList.push_back(foundClk);
        }

        if(line.startsWith("set_input_delay",Qt::CaseSensitive))
        {
            // printf(" <set_input_delay> found!\n");
            setInpDelayStruct foundIDelay;
            QRegularExpressionMatch matchPer = getRegExMatchSDC("set_input_delay\\s+(\\d+)", line);
            if (matchPer.hasMatch()) {
                QString matched = matchPer.captured(0);
                QString period = matchPer.captured(1);
                //  printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                foundIDelay.value = period.toInt();
            }
            if (line.contains("-max"))
                foundIDelay.max = true;
            if (line.contains("-min"))
                foundIDelay.min = true;
            if (line.contains("-rise"))
                foundIDelay.rise = true;
            if (line.contains("-fall"))
                foundIDelay.fall = true;
            QRegularExpressionMatch matchGetPort = getRegExMatchSDC("get_ports\\s+(\\S+)", line);
            if (matchGetPort.hasMatch()) {
                QString matched = matchGetPort.captured(0);
                QString getPort = matchGetPort.captured(1);
                if(getPort.endsWith(']'))
                    getPort.chop(1);
                // printf("matched =%s  getPort = %s\n",matched.toStdString().c_str(),getPort.toStdString().c_str());
                foundIDelay.get_ports = getPort;
            }
            QRegularExpressionMatch matchName = getRegExMatchSDC("\\-clock\\s+(\\S+)", line);
            if (matchName.hasMatch()) {
                QString matched = matchName.captured(0);
                QString name = matchName.captured(1);
                //  printf("matched =%s  name = %s\n",matched.toStdString().c_str(),name.toStdString().c_str());
                foundIDelay.clock = name;
            }
            m_inDelayList.push_back(foundIDelay);

        }


        if(line.startsWith("set_input_transition",Qt::CaseSensitive))
        {
            //printf(" <set_input_transition> found!\n");
            setInpTransStruct foundITransition;
            QRegularExpressionMatch matchPer = getRegExMatchSDC("set_input_transition\\s+(\\d+)", line);
            if (matchPer.hasMatch()) {
                QString matched = matchPer.captured(0);
                QString period = matchPer.captured(1);
                // printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                foundITransition.value = period.toInt();
            }
            if (line.contains("-max"))
                foundITransition.max = true;
            if (line.contains("-min"))
                foundITransition.min = true;
            if (line.contains("-rise"))
                foundITransition.rise = true;
            if (line.contains("-fall"))
                foundITransition.fall = true;
            QRegularExpressionMatch matchGetPort = getRegExMatchSDC("get_ports\\s+(\\S+)", line);
            if (matchGetPort.hasMatch()) {
                QString matched = matchGetPort.captured(0);
                QString getPort = matchGetPort.captured(1);
                if(getPort.endsWith(']'))
                    getPort.chop(1);
                // printf("matched =%s  getPort = %s\n",matched.toStdString().c_str(),getPort.toStdString().c_str());
                foundITransition.get_ports = getPort;
            }
            QRegularExpressionMatch matchName = getRegExMatchSDC("\\-clock\\s+(\\S+)", line);
            if (matchName.hasMatch()) {
                QString matched = matchName.captured(0);
                QString name = matchName.captured(1);
                // printf("matched =%s  name = %s\n",matched.toStdString().c_str(),name.toStdString().c_str());
                foundITransition.clock = name;
            }
            m_inpTransList.push_back(foundITransition);
        }

        if(line.startsWith("set_load",Qt::CaseSensitive) )
        {
            // printf(" <set_load> found!\n");
            setLoadStruct foundSetLoad;
            QRegularExpressionMatch matchPer = getRegExMatchSDC("\\-pin_load\\s+(\\d+)", line);
            if (matchPer.hasMatch()) {
                QString matched = matchPer.captured(0);
                QString period = matchPer.captured(1);
                // printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                foundSetLoad.value = period.toInt();
                QRegularExpressionMatch matchGetPort = getRegExMatchSDC("get_ports\\s+(\\S+)", line);
                if (matchGetPort.hasMatch()) {
                    QString matched = matchGetPort.captured(0);
                    QString getPort = matchGetPort.captured(1);
                    if(getPort.endsWith(']'))
                        getPort.chop(1);
                    // printf("matched =%s  getPort = %s\n",matched.toStdString().c_str(),getPort.toStdString().c_str());
                    foundSetLoad.get_ports = getPort;
                }

                m_setLoadList.push_back(foundSetLoad);
            }
        }
        if(line.startsWith("set_output_delay",Qt::CaseSensitive) )
        {
            // printf(" <set_output_delay> found!\n");
            setOutDelayStruct foundODelay;
            QRegularExpressionMatch matchPer = getRegExMatchSDC("set_output_delay\\s+(\\d+)", line);
            if (matchPer.hasMatch()) {
                QString matched = matchPer.captured(0);
                QString period = matchPer.captured(1);
                // printf("matched =%s  period = %s\n",matched.toStdString().c_str(),period.toStdString().c_str());
                foundODelay.value = period.toInt();
            }
            if (line.contains("-max"))
                foundODelay.max = true;
            if (line.contains("-min"))
                foundODelay.min = true;
            if (line.contains("-rise"))
                foundODelay.rise = true;
            if (line.contains("-max"))
                foundODelay.fall = true;
            QRegularExpressionMatch matchGetPort = getRegExMatchSDC("get_ports\\s+(\\S+)", line);
            if (matchGetPort.hasMatch()) {
                QString matched = matchGetPort.captured(0);
                QString getPort = matchGetPort.captured(1);
                if(getPort.endsWith(']'))
                    getPort.chop(1);
                // printf("matched =%s  getPort = %s\n",matched.toStdString().c_str(),getPort.toStdString().c_str());
                foundODelay.get_ports = getPort;
            }
            QRegularExpressionMatch matchName = getRegExMatchSDC("\\-clock\\s+(\\S+)", line);
            if (matchName.hasMatch()) {
                QString matched = matchName.captured(0);
                QString name = matchName.captured(1);
                //  printf("matched =%s  name = %s\n",matched.toStdString().c_str(),name.toStdString().c_str());
                foundODelay.clock = name;
            }
            m_outDelayList.push_back(foundODelay);
        }
    }


    return true;
}



void SDCParser::setOutputFileName(QString fName){
    exportFileName= fName;
}

void SDCParser::exportToFile(QStringList &list) {

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

QRegularExpressionMatch getRegExMatchSDC(QString pattern,QString &line)
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
