#ifndef SDCPARSER_H
#define SDCPARSER_H

#include <QTextStream>
#include <QStack>
#include <QStringList>
#include <QFile>
#include <vector>
#include <string>

struct createClkStruct {
    int  period =0;
    QString  name;
    QString  get_ports;
};

struct setInpDelayStruct {
    int  value = 0;
    bool  min;
    bool  max;
    bool  rise;
    bool  fall;
    QString  get_ports;
    QString clock;
};

struct setOutDelayStruct {
    int  value =0;
    bool  min = false;
    bool  max = false;
    bool  rise = false;
    bool  fall = false;
    QString  get_ports;
    QString clock;
};

struct setInpTransStruct {
    int  value =0;
    bool  min = false;
    bool  max = false;
    bool  rise = false;
    bool  fall = false;
    QString  get_ports;
    QString clock;
};

struct setLoadStruct {
    int  value =0;
    QString  pin_load;
    QString  get_ports;
};

QRegularExpressionMatch getRegExMatchSDC(QString pattern,QString &line);

class SDCParser
{
public:
    SDCParser();
    ~SDCParser();

    bool Parse ( QTextStream  *stream);

    QString Unit() const{ return m_unit; }
    int Time() const{ return m_time; }

    void exportToFile(QStringList &list);
    void setOutputFileName(QString fName);

private:

	QString m_unit;
    int m_time;
    QString exportFileName;

    QList<createClkStruct> m_createClkList;
    QList<setInpDelayStruct> m_inDelayList;
    QList<setOutDelayStruct> m_outDelayList;
    QList<setInpTransStruct> m_inpTransList;
    QList<setLoadStruct> m_setLoadList;

    qulonglong m_currentPosition; // Time given by last #nnnn

     void setUnit(QString val) { m_unit = val ; }
     void setTime(int val) { m_time = val ; }

};

#endif
