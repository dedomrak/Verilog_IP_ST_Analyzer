
#include "./database/DataBase.h"
#include "./database/Instance.h"

#include "./verilog/veri_file.h"
#include "./verilog/VeriModule.h"
#include "./verilog/VeriMisc.h"
#include "./verilog/VeriExpression.h"
#include "./verilog_nl/veri_nl_file.h"

#include "./util/Strings.h"
#include "./containers/Array.h" // a dynamic array
#include "./containers/Set.h"   // a simple hash table
#include "./containers/Map.h"
#include <vector>
#include <map>
#include <list>
#include <queue>
#include <algorithm>
#include "./SDC_parser/SDC_parser.h"
#include "./LIB_parser/LIB_parser.h"

#ifdef VERIFIC_NAMESPACE
using namespace Verific ;
#endif
VeriModule *topModule;


struct libCellStruct {
    QString cellType;
    QVector<Net *> fanInList;
    QVector<Net *> fanOutList;
    double arrivalTime = 0;
    double inputSlew = 2;
    double slack = 0;
    double requiredArrivalTime = static_cast<double>(INT_MAX);
    bool isPrimaryOutput = false;
    bool isPrimaryInput = false;
    bool isPrimaryInout = false;
    Cell * libCell;
    QString cellName;
};

struct standartCell
{

    QVector<QVector<double>> delayTable;
    QVector<QVector<double>> slewValues= {{0.1,0.1,0.1,0.1,0.1,0.1,0.1},
                                          {0.1,0.1,0.1,0.1,0.1,0.1,0.1},
                                          {0.1,0.1,0.1,0.1,0.1,0.1,0.1},
                                          {0.1,0.1,0.1,0.1,0.1,0.1,0.1},
                                          {0.1,0.1,0.1,0.1,0.1,0.1,0.1},
                                          {0.1,0.1,0.1,0.1,0.1,0.1,0.1},
                                          {0.1,0.1,0.1,0.1,0.1,0.1,0.1}};
    double capacitance = 0.1;
    QVector<double>inputSlew;
    QVector<double>outputLoad;
};

QMap< Net *, libCellStruct> netlistMap;

QVector<Net *> topologicalSort() {
    QVector<Net *> sortedList;
    QMap< Net *, int> inDegree;
    foreach(auto netID,netlistMap.keys()) {
        inDegree[netID] = netlistMap.value(netID).fanInList.size();
    }

    std::queue<Net *> q;
    foreach(auto netID,netlistMap.keys()) {
        if(inDegree[netID] == 0) {
            q.push(netID);
        }
    }

    while(!q.empty()) {
        Net * netI = q.front();
        q.pop();
        sortedList.push_back(netI);
        foreach(auto netValue,netlistMap.value(netI).fanOutList) {
            if(--inDegree[netValue] == 0) {
                q.push(netValue);
            }
        }
    }
    return  sortedList;
}

double interpolate(double inputSlew, double loadCap, const standartCell &gateInfo, bool is_delay) {

    const QVector<double> &slewIndices = gateInfo.inputSlew;
    const QVector<double> &capIndices = gateInfo.outputLoad;

    inputSlew /= 1000.0; // Convert to nanoseconds

    // Find bounding indices for input slew
    auto slewIt = std::upper_bound(slewIndices.begin(), slewIndices.end(), inputSlew);
    int slewIndex2 = std::distance(slewIndices.begin(), slewIt);
    int slewIndex1 = std::max(0, slewIndex2 - 1);
    slewIndex2 = std::min(static_cast<int>(slewIndices.size()) - 1, slewIndex2);

    // Find bounding indices for load capacitance
    auto capIt = std::upper_bound(capIndices.begin(), capIndices.end(), loadCap);
    int capIndex2 = std::distance(capIndices.begin(), capIt);
    int capIndex1 = std::max(0, capIndex2 - 1);
    capIndex2 = std::min(static_cast<int>(capIndices.size()) - 1, capIndex2);

    // Retrieve values for bilinear interpolation
    double v11, v12, v21, v22;
    if (is_delay) {
        v11 = gateInfo.delayTable[slewIndex1][capIndex1];
        v12 = gateInfo.delayTable[slewIndex1][capIndex2];
        v21 = gateInfo.delayTable[slewIndex2][capIndex1];
        v22 = gateInfo.delayTable[slewIndex2][capIndex2];
    } else {
        v11 = gateInfo.slewValues[slewIndex1][capIndex1];
        v12 = gateInfo.slewValues[slewIndex1][capIndex2];
        v21 = gateInfo.slewValues[slewIndex2][capIndex1];
        v22 = gateInfo.slewValues[slewIndex2][capIndex2];
    }

    // Perform bilinear interpolation
    double t1 = slewIndices[slewIndex1], t2 = slewIndices[slewIndex2];
    double c1 = capIndices[capIndex1], c2 = capIndices[capIndex2];
    double interpolatedValue =
            (v11 * (c2 - loadCap) * (t2 - inputSlew) +
             v12 * (loadCap - c1) * (t2 - inputSlew) +
             v21 * (c2 - loadCap) * (inputSlew - t1) +
             v22 * (loadCap - c1) * (inputSlew - t1)) /
            ((c2 - c1) * (t2 - t1));
    return interpolatedValue * 1000.0; // Convert back to picoseconds
}

double forwardTraversal(QVector<Net *> &sortedList, QMap<QString, standartCell> &gates) {
    double circuitDelay = 0;
    for (Net * node : sortedList) {
        libCellStruct& currentNode = netlistMap[node];

        if (currentNode.isPrimaryInput || (currentNode.cellType == "OUTPUT" && !currentNode.isPrimaryOutput)) {
            currentNode.arrivalTime = 0;
            currentNode.inputSlew = 2;
            continue;
        }

        if (currentNode.cellType == "INPUT" && !currentNode.isPrimaryInput) {
            circuitDelay = std::max(circuitDelay, currentNode.arrivalTime);
            continue;
        }

        const standartCell &gateInfo = gates.value(currentNode.cellType);
        double maxArrivalTime = 0;
        double maxInputSlew = 0;
        double loadCap = currentNode.isPrimaryOutput ? 4 * gates.value("INV").capacitance : 0;

        for (Net * fanOutNode : currentNode.fanOutList) {
            loadCap += gates.value(netlistMap[fanOutNode].cellType).capacitance;
        }

        for (Net * fanInNode : currentNode.fanInList) {
            double delay = interpolate(netlistMap[fanInNode].inputSlew, loadCap, gateInfo, true);
            double outputSlew = interpolate(netlistMap[fanInNode].inputSlew, loadCap, gateInfo, false);

            if (currentNode.fanInList.size() > 2) {
                delay *= (currentNode.fanInList.size() / 2.0);
            }

            double arrivalTime = netlistMap[fanInNode].arrivalTime + delay;
            maxArrivalTime = std::max(maxArrivalTime, arrivalTime);
            maxInputSlew = std::max(maxInputSlew, outputSlew);
        }

        currentNode.arrivalTime = maxArrivalTime;
        currentNode.inputSlew = maxInputSlew;

        if (currentNode.isPrimaryOutput) {
            circuitDelay = std::max(circuitDelay, currentNode.arrivalTime);
        }
    }
    return circuitDelay;
}

void backwardTraversal(const QVector<Net *>& sortedList , const QMap<QString, standartCell>& gates, double circuitDelay) {

    double requiredTimeVal = 1.1 * circuitDelay;
    foreach(auto netID,netlistMap.keys()) {
        libCellStruct netValue = netlistMap.value(netID);
        if (netValue.isPrimaryOutput) {
            netValue.requiredArrivalTime = requiredTimeVal;
        } else {
            netValue.requiredArrivalTime = static_cast<double>(INT_MAX);
        }
        netlistMap[netID] = netValue;
    }

    // Step 2: Traverse the sorted list in reverse to propagate required times
    for (auto it = sortedList.rbegin(); it != sortedList.rend(); ++it) {
        Net * netI = *it;
        libCellStruct& currentNode = netlistMap[netI];

        // Skip primary output nodes as they're already initialized
        if (currentNode.isPrimaryOutput) {
            currentNode.slack = currentNode.requiredArrivalTime - currentNode.arrivalTime;
            continue;
        }

        double minRequiredTime = static_cast<double>(INT_MAX);

        // Step 3: Calculate required time based on fan-out nodes
        for (Net * fanOutNode : currentNode.fanOutList) {
            const libCellStruct& fanOutGate = netlistMap[fanOutNode];
            const standartCell& fanOutGateInfo = gates.value(fanOutGate.cellType);

            // Calculate load capacitance
            double loadCap = fanOutGate.isPrimaryOutput ? 4 * gates.value("INV").capacitance : 0;
            for (Net * nextFanOutNode : fanOutGate.fanOutList) {
                loadCap += gates.value(netlistMap[nextFanOutNode].cellType).capacitance;
            }

            // Calculate delay and required time for the current fan-out connection
            double delay = interpolate(currentNode.inputSlew, loadCap, fanOutGateInfo, true);
            double requiredTime = fanOutGate.requiredArrivalTime - delay;

            minRequiredTime = std::min(minRequiredTime, requiredTime);
        }

        // Step 4: Set required time and calculate slack
        if (minRequiredTime < static_cast<double>(INT_MAX)) {
            currentNode.requiredArrivalTime = minRequiredTime;
        } else {
            currentNode.requiredArrivalTime = requiredTimeVal;
        }

        // Special handling for primary inputs:
        if (currentNode.isPrimaryInput) {
            double slackForInput = static_cast<double>(INT_MAX);

            // Calculate slack as the required time - delay from fan-out nodes back to input
            for (Net * fanOutNode : currentNode.fanOutList) {
                const libCellStruct& fanOutGate = netlistMap[fanOutNode];
                const standartCell& fanOutGateInfo = gates.value(fanOutGate.cellType);

                double loadCap = fanOutGate.isPrimaryOutput ? 4 * gates.value("INV").capacitance : 0;
                for (Net * nextFanOutNode : fanOutGate.fanOutList) {
                    loadCap += gates.value(netlistMap[nextFanOutNode].cellType).capacitance;
                }

                double delay = interpolate(currentNode.inputSlew, loadCap, fanOutGateInfo, true);
                double requiredTimeForInput = fanOutGate.requiredArrivalTime - delay;
                slackForInput = std::min(slackForInput, requiredTimeForInput - currentNode.arrivalTime);
            }
            currentNode.slack = slackForInput;
        } else {
            currentNode.slack = currentNode.requiredArrivalTime - currentNode.arrivalTime;
        }
    }
}

QVector<Net *> CriticalPath() {
    QVector<Net *> criticalPath;
    Net * currentNet;
    double maxDelay = 0;

    foreach(auto netID,netlistMap.keys()) {
        libCellStruct netValue = netlistMap.value(netID);
        if (netValue.isPrimaryOutput && netValue.arrivalTime > maxDelay) {
            maxDelay = netValue.arrivalTime;
            currentNet = netID;
        }
    }

    while (currentNet) {
        criticalPath.push_back(currentNet);
        if (netlistMap.value(currentNet).isPrimaryInput) break;

        Net * prevNet ;
        double maxPrevArrivalTime = -1;
        for (Net * fanInNode : netlistMap.value(currentNet).fanInList) {
            if (netlistMap.value(fanInNode).arrivalTime > maxPrevArrivalTime) {
                maxPrevArrivalTime = netlistMap.value(fanInNode).arrivalTime;
                prevNet = fanInNode;
            }
        }
        currentNet = prevNet;
    }

    std::reverse(criticalPath.begin(), criticalPath.end());
    return criticalPath;
}

#include <fstream>
#include <sstream>

int readLibrary(char *fName, QMap<QString, standartCell> &gates)
{

    std::ifstream ifs(fName);
    bool cap = false;
    bool cellDelay = false;
    bool index1 = false;
    bool index2 = false;
    bool insideValues = false;
    bool insideSlewValues = false;

    std::string line;
    standartCell currGate;
    QString gateName;
    int row = 0;

    while (getline(ifs, line))
    {

        if (line.empty()) {
            continue;
        }

        if (line.find("cell ") != std::string::npos)
        {
            if (!gateName.isEmpty()) {
                gates[gateName] = currGate;
            }
            gateName.clear();
            row = 0;

            currGate.inputSlew.clear();
            currGate.outputLoad.clear();

            char start;
            std::string cellName;
            std::istringstream iss(line);
            iss >> cellName >> start >> cellName;
            gateName = QString::fromStdString(cellName.substr(0, cellName.find(')')));
            cap = true;
        }

        if(line.find("capacitance") != std::string::npos && cap)
        {
            int pos = line.find(":");
            if(pos != std::string::npos)
            {
                std::string valString = line.substr(pos+1);
                valString.erase(remove(valString.begin(), valString.end(), ','), valString.end());
                currGate.capacitance = stod(valString);
                cap = false;
            }
        }

        if(line.find("cell_delay") != std::string::npos)
        {
            cellDelay = true;
            index1 = true;
        }

        if(line.find("index_1 (") != std::string::npos && index1)
        {
            line = line.substr(line.find("index_1 (" + 8));
            line.erase(remove(line.begin(), line.end(), '('), line.end());
            line.erase(remove(line.begin(), line.end(), '\"'), line.end());
            line.erase(remove(line.begin(), line.end(), ')'), line.end());
            std::istringstream valuestream(line);
            std::string value;
            while(getline(valuestream, value, ','))
            {
                if(value.size() > 0)
                {
                    currGate.inputSlew.push_back(stod(value));
                }
            }
            index1 = false;
            index2 = true;
        }

        if(line.find("index_2 (") != std::string::npos && index2)
        {
            line = line.substr(line.find("index_2 (" + 8));
            line.erase(remove(line.begin(), line.end(), '('), line.end());
            line.erase(remove(line.begin(), line.end(), '\"'), line.end());
            line.erase(remove(line.begin(), line.end(), ')'), line.end());
            std::istringstream valuestream(line);
            std::string value;
            while(getline(valuestream, value, ','))
            {
                if(value.size() > 0)
                {
                    currGate.outputLoad.push_back(stod(value));
                }
            }
            index2 = false;
        }

        if (line.find("values (") != std::string::npos && cellDelay)
        {
            line = line.substr(line.find("values (") + 8);
            insideValues = true;
        }

        if (insideValues) {
            line.erase(remove(line.begin(), line.end(), '\"'), line.end());
            line.erase(remove(line.begin(), line.end(), '\\'), line.end());
            std::istringstream valuestream(line);
            std::string value;
            int col = 0;
            QVector<double> tmpVec;
            while (getline(valuestream, value, ',')) {
                value.erase(remove(value.begin(), value.end(), ' '), value.end());
                if (value.size() > 0) {
                    if (row < 7 && col < 7) {
                        tmpVec.push_back(stod(value));
                        col++;
                    }
                }
            }
            currGate.delayTable.push_back(tmpVec);

            if (line.find(");") != std::string::npos)
            {
                insideValues = false;
                cellDelay = false;
            }
            row++;
        }

        if (line.find("values (") != std::string::npos && !cellDelay && !insideValues) {
            line = line.substr(line.find("values (") + 8);
            insideSlewValues = true;
            row = 0;
        }

        if (insideSlewValues) {
            line.erase(remove(line.begin(), line.end(), '\"'), line.end());
            line.erase(remove(line.begin(), line.end(), '\\'), line.end());
            std::istringstream valuestream(line);
            std::string value;
            int col = 0;
            QVector<double> tmpVec;
            while (getline(valuestream, value, ',')) {
                value.erase(remove(value.begin(), value.end(), ' '), value.end());
                if (value.size() > 0) {
                    if (row < 7 && col < 7) {
                        tmpVec.push_back(stod(value));
                        col++;
                    }
                }
            }
            currGate.delayTable.push_back(tmpVec);

            if (line.find(");") != std::string::npos) {
                insideSlewValues = false;
            }
            row++;
        }
    }

    if (!gateName.isEmpty()) {
        gates[gateName] = currGate;
    }

    ifs.close();
    return 0;
}


int main(int argc, char *argv[])
{

    QString fileName;
    QString fileSDC;
    QString fileLib;

    //--------------------------------------------------------------
    // PARSE ARGUMENTS
    //--------------------------------------------------------------

    for (int i = 1; i < argc; i++) {
        if (!QString(argv[i]).compare("-lib")) {
            i++ ;
            fileLib = (i < argc) ? QString(argv[i]): QString() ;
            continue ;
        } else if (!QString(argv[i]).compare("-v")) {
            i++ ;
            fileName = (i < argc) ? QString(argv[i]): QString() ;
            continue ;
        } else if (!QString(argv[i]).compare("-sdc")) {
            i++ ;
            fileSDC = (i < argc) ? QString(argv[i]): QString() ;
            continue ;
        }
    }
    if(argc==1)
    {
        printf("Usage: VIP Static timing analyzer -v <input Verilog file> -lib <standard cells file> -sdc <SDC file>  \n");
        printf("-v file \t...\t verilog file\n");
        printf("-lib file \t...\t library file\n");
        printf("-sdc file \t...\t SDC file\n");
        return 1 ;
    }

    if(fileName.isEmpty()) {
        printf("Input file is missing!") ;
        return 1 ;
    }
    QFile file(fileSDC);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        printf("Could't open file\n");
        return 1;
    }

    QTextStream stream(&file);
    SDCParser sdc;
    sdc.Parse(&stream);
    file.close();

    QMap<QString, standartCell> gates;
    readLibrary(const_cast<char *>(fileLib.toStdString().c_str()), gates);

    if (!veri_nl_file::Read(fileName.toStdString().c_str(),"work")) return 1 ;
    Netlist *mid = Netlist::PresentDesign() ;
    if (!mid) {
        Message::PrintLine("cannot find any handle to 'mid' netlist") ;
        return 2 ;
    }
    Netlist *topNet = Netlist::PresentDesign() ;
    if (!topNet) {
        Message::PrintLine("cannot find any handle to the top-level netlist") ;
        return 4 ;
    }
    Cell * cellTop = topNet->Owner();
    Map * ports = topNet->GetPorts();
    MapIter mi ;
    Port *port ;
    FOREACH_MAP_ITEM(ports,mi,0,&port) {
        if (!port) continue ;
        Net *netP = port->GetNet();
        //printf("found net = %s\n",netP->Name());
        port_dir direction =  port->GetDir();
        if(direction==DIR_IN) {
            netlistMap[netP].isPrimaryInput = true;
            netlistMap[netP].cellType = "INPUT";
        }
        else if(direction==DIR_OUT) {
            netlistMap[netP].isPrimaryOutput=true;
            netlistMap[netP].cellType = "OUTPUT";
        }
        else if(direction==DIR_INOUT) {
            netlistMap[netP].isPrimaryInout=true;
            netlistMap[netP].cellType = "INOUT";
        }
        netlistMap[netP].cellName = cellTop->Name();
        netlistMap[netP].libCell = cellTop;
    }


    Map *allInst = topNet->GetInsts();
    MapIter mi4 ;
    Instance * inst;
    FOREACH_MAP_ITEM(allInst,mi4,0,&inst) {
        if (!inst) continue ;

        std::vector<Net *> tmpVecFanIn;
        std::vector<Net *> tmpVecFanOut;
        std::vector<Net *> tmpVecFanInOut;
        Cell * cellInst = inst->View()->Owner();
        if(!cellInst) continue;
        QString tmpCellName = cellInst->Name();
        QString libCellInstName = tmpCellName.split("_")[1];
        MapIter mi ;
        PortRef *pr ;
        FOREACH_PORTREF_OF_INST(inst, mi, pr) {
            Port *port = pr->GetPort();
            if(!port) continue;
            port_dir direction =  port->GetDir();
            if(direction==DIR_IN) {
                Net * net = pr->GetNet();
                if(!net) continue;
                tmpVecFanIn.push_back(net);
                netlistMap[net].cellType = libCellInstName;
                netlistMap[net].libCell = cellInst;
            }
            else if(direction==DIR_OUT) {
                Net * net = pr->GetNet();
                if(!net) continue;
                tmpVecFanOut.push_back(net);
                netlistMap[net].cellType = libCellInstName;
                netlistMap[net].libCell = cellInst;

            }
            else if(direction==DIR_INOUT) {
                Net * net = pr->GetNet();
                if(!net) continue;
                tmpVecFanInOut.push_back(net);
                netlistMap[net].cellType = libCellInstName;
                netlistMap[net].libCell = cellInst;
            }
        }
        for (std::vector<Net *>::iterator itI=tmpVecFanIn.begin(); itI!=tmpVecFanIn.end(); itI++) {
            if(*itI) {
                Net *netIn =(*itI);
                //printf("netIn = %s\n",netIn->Name());
                for (std::vector<Net *>::iterator itO=tmpVecFanOut.begin(); itO!=tmpVecFanOut.end(); itO++) {
                    if((*itO)) {
                        //printf("itO = %s\n",(*itO)->Name());
                        netlistMap[(*itO)].fanInList.push_back(netIn);
                        netlistMap[netIn].fanOutList.push_back(*itO);
                    }
                }
                for (std::vector<Net *>::iterator itIO=tmpVecFanInOut.begin(); itIO!=tmpVecFanInOut.end(); itIO++) {
                    if(*itIO) {
                        //printf("itIO = %s\n",(*itIO)->Name());
                        netlistMap[*itIO].fanInList.push_back(netIn);
                        netlistMap[netIn].fanOutList.push_back(*itIO);
                    }
                }
            }
        }
    }


    Map * allNets = topNet->GetNets();
    MapIter mi3 ;
    Net *net3 ;
    FOREACH_MAP_ITEM(allNets,mi3,0,&net3) {
        if (!net3) continue ;
        // printf("found net = %s\n",net3->Name());
    }
    Libset *glbl = Libset::Global() ;
    Library *lib = glbl->GetLibrary("work") ;
    Map *  allCells =       (lib) ? lib->GetCells() : 0;
    QVector<Net *> sortedList = topologicalSort();
    double circuitDelay = forwardTraversal(sortedList, gates);
    backwardTraversal(sortedList,  gates, circuitDelay);
    QVector<Net *> criticalPath = CriticalPath();

    printf(" ###############################################################\n");
    printf(" #\tCritical path:\n");
    printf(" #\t");
    for (size_t i = 0; i < criticalPath.size(); ++i) {
        Net * nodeID = criticalPath[i];
        const auto& netI = netlistMap.value(nodeID);

        QString prefix = netI.isPrimaryInput ? "INPUT ->" : (netI.isPrimaryOutput ? "-> OUTPUT" : netI.cellType + "->");
        //outFile << prefix << "n" << nodeID;
        printf("%s %s",prefix.toStdString().c_str(),nodeID->Name());
        if (i < criticalPath.size() - 1) printf(", ");
    }
    printf("\n ###############################################################\n");
    printf("\n ###############################################################\n");
    printf(" #\tGate slacks:\n");
    foreach(auto netID,netlistMap.keys()) {
        libCellStruct netValue = netlistMap.value(netID);
        QString prefix = netValue.isPrimaryInput ? "INPUT \t->" : (netValue.isPrimaryOutput ? "OUTPUT \t->" : netValue.cellType + "\t->");
        printf(" #\t%s %s:%f ps\n",prefix.toStdString().c_str(),netID->Name(),netValue.slack);
    }
    printf(" ###############################################################\n\n");
    printf(" ###############################################################\n");
    printf(" #\tEnd of Design summary!\n");
    printf(" ###############################################################\n");


    return 0 ;
}
