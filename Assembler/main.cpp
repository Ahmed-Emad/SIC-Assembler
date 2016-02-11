//  main.cpp
//  Assembler
//
//  Created by Ahmed Barakat on 315 / 11 / 14.
//  Copyright (c) 2014 Ahmed Barakat. All rights reserved.

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include <map>
#include <vector>

using namespace std;

string intTOstring(int number);
int stringTOint(string number);
int hexTOint(string hexValue);
string intToHex(int decimalValue);
bool isValidString(string input);
bool isValidStringIndexing(string input);
bool isValidHex(string input);
bool isValidInt(string input);
string trim(string line);
string trimIndex(string line);
string trimBYTE(string line);
string trimWORD(string line);
vector<int> stringTOintegers(string number);
void printIntSixes();
void printHexSixes();
string Literal(bool isHex);
int findInLITERALS(int value);
int findInClonedLIT(string value);
int findInBlocks(string value);
void print(string element);

string START = "start";
string END = "end";
string WORD = "word";
string BYTE = "byte";
string RESW = "resw";
string RESB = "resb";
string RSUB = "rsub";
string LTORG = "ltorg";
string EQU = "equ";
string ORG = "org";
string USE = "use";

ifstream src("/Users/ahmadbarakat/Desktop/SRCFILE");
ifstream inter("/Users/ahmadbarakat/Desktop/INTFILE");
ifstream lis("/Users/ahmadbarakat/Desktop/LISFILE");

map<string, int> OPTAB;
map<string, int> EQUTAB;
vector<map<string, int>> allLITABLE;
vector<vector<int>> allLITERALS;
vector<vector<int>> allLITSize;
vector<vector<string>> allclonedlit;
vector<vector<int>> allclonedcode;
vector<vector<int>> allclonedloc;
vector<string> errors;
vector<int> errorsLines;
vector<string> blocks;
vector<map<string, int>> allSYMTAB;
vector<int> currenLOCCTR;
vector<int> nexLOCCTR;
int currentBlock;

string line, locctrString, opcode, label, mnemonic, operand, comment, blocknum;
bool errorFlag = false;
int startingAddress = 0, progLength = 0, lineNo = 0, lastLOCCTR = 0;

int main(int argc, const char * argv[]) {
    
    OPTAB["add"] = 24;
    OPTAB["and"] = 64;
    OPTAB["comp"] = 40;
    OPTAB["div"] = 36;
    OPTAB["j"] = 60;
    OPTAB["jeq"] = 48;
    OPTAB["jgt"] = 52;
    OPTAB["jlt"] = 56;
    OPTAB["jsub"] = 72;
    OPTAB["lda"] = 0;
    OPTAB["ldch"] = 80;
    OPTAB["ldl"] = 8;
    OPTAB["ldx"] = 4;
    OPTAB["mul"] = 32;
    OPTAB["or"] = 68;
    OPTAB["rd"] = 216;
    OPTAB["rsub"] = 76;
    OPTAB["sta"] = 12;
    OPTAB["stch"] = 84;
    OPTAB["stl"] = 20;
    OPTAB["stx"] = 16;
    OPTAB["sub"] = 28;
    OPTAB["td"] = 224;
    OPTAB["tix"] = 44;
    OPTAB["wd"] = 220;
    
    currentBlock = 0;
    blocks.push_back("");
    map<string, int> b0;
    allSYMTAB.push_back(b0);
    currenLOCCTR.push_back(0);
    nexLOCCTR.push_back(0);
    
    map<string, int> temp1;
    allLITABLE.push_back(temp1);
    vector<int> temp2;
    allLITERALS.push_back(temp2);
    vector<int> temp3;
    allLITSize.push_back(temp3);
    vector<string> temp4;
    allclonedlit.push_back(temp4);
    vector<int> temp5;
    allclonedcode.push_back(temp5);
    vector<int> temp6;
    allclonedloc.push_back(temp6);
    //..............................................................
    //..............................................................
    //....................... PASS 1 ...............................
    
    freopen("/Users/ahmadbarakat/Desktop/INTFILE", "w", stdout);
    printf("SIC Assembler By: Ahmed Emad\n\n");
    
    while (getline(src, line) && (mnemonic != END)) {
        if (line[0] != '.' && line.size()>9) {
            lineNo++;
            errorFlag = false;
            errors.clear();
            if (currenLOCCTR[currentBlock] > 32768) {
                errorFlag = true;
                errorsLines.push_back(lineNo);
                errors.push_back("**** memory out of range\n");
            }
            label = line.substr(0, 8);
            mnemonic = line.substr(9, 6);
            transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(), ::tolower);
            
            if (line.size() > 17) {
                operand = line.substr(17, 17);
            }
            else if (trim(mnemonic) != RSUB && trim(mnemonic) != LTORG
                     && trim(mnemonic) != ORG && trim(mnemonic) != USE){
                errorFlag = true;
                errorsLines.push_back(lineNo);
                errors.push_back("**** missing operand\n");
            }
            else
                operand = "";
            if (line.size() > 36)
                comment = line.substr(35);
            else
                comment = "";
            
            if (!isValidString(label)) {
                errorFlag = true;
                errorsLines.push_back(lineNo);
                errors.push_back("**** illegal format in label field\n");
            }
            if (!isValidString(mnemonic)) {
                errorFlag = true;
                errorsLines.push_back(lineNo);
                errors.push_back("**** illegal format in mnemonic field\n");
            }
            if (!isValidStringIndexing(operand) && trim(mnemonic)!=BYTE && trim(mnemonic)!=WORD && trim(mnemonic)!=RSUB
                && trim(mnemonic)!=LTORG && trim(mnemonic)!=ORG && trim(mnemonic)!=USE && operand[0] != '=') {
                errorFlag = true;
                errorsLines.push_back(lineNo);
                errors.push_back("**** illegal format in operand field\n");
            }
            label = trim(label);
            mnemonic = trim(mnemonic);
            if (mnemonic == BYTE || operand[0] == '=')
                operand = trimBYTE(operand);
            else if (operand.find(",x") != string::npos)
                operand = trimIndex(operand);
            else if ( operand.find(",") != string::npos ) {
                errorFlag = true;
                errorsLines.push_back(lineNo);
                errors.push_back("**** illegal format in operand field\n");
            }
            else if (mnemonic == WORD)
                operand = trimWORD(operand);
            else
                operand = trim(operand);
            if (!errorFlag){
                if (mnemonic == START && lineNo == 1) {
                    currenLOCCTR[currentBlock] = startingAddress = nexLOCCTR[currentBlock] = hexTOint(operand);
                    if (startingAddress > 65535) {
                        errorFlag = true;
                        errorsLines.push_back(lineNo);
                        errors.push_back("**** memory out of range\n");
                    }
                }
                else if (mnemonic == START && lineNo != 1) {
                    errorFlag = true;
                    errorsLines.push_back(lineNo);
                    errors.push_back("**** duplicate or misplaced start statement\n");
                }
                else {
                    if (mnemonic == USE) {
                        if (operand.size() == 0) {
                            currentBlock = 0;
                        }
                        else if (findInBlocks(operand) < 0) {
                            blocks.push_back(operand);
                            currentBlock = findInBlocks(operand);
                            map<string, int> b;
                            allSYMTAB.push_back(b);
                            currenLOCCTR.push_back(0);
                            nexLOCCTR.push_back(0);
                            
                            map<string, int> temp1;
                            allLITABLE.push_back(temp1);
                            vector<int> temp2;
                            allLITERALS.push_back(temp2);
                            vector<int> temp3;
                            allLITSize.push_back(temp3);
                            vector<string> temp4;
                            allclonedlit.push_back(temp4);
                            vector<int> temp5;
                            allclonedcode.push_back(temp5);
                            vector<int> temp6;
                            allclonedloc.push_back(temp6);
                        }
                        else {
                            currentBlock = findInBlocks(operand);
                        }
                    }
                    else if (mnemonic == ORG) {
                        if (label.size()==0) {
                        if (operand.size() == 0) {
                            if (lastLOCCTR != 0) {
                                currenLOCCTR[currentBlock] = nexLOCCTR[currentBlock] = lastLOCCTR;
                                lastLOCCTR = 0;
                            }
                            else {
                                errorFlag = true;
                                errorsLines.push_back(lineNo);
                                errors.push_back("**** missing prev org statement\n");
                            }
                        }
                        else {
                            if (isValidInt(operand)) {
                                lastLOCCTR = currenLOCCTR[currentBlock];
                                currenLOCCTR[currentBlock] = nexLOCCTR[currentBlock] = stringTOint(operand);
                            }
                            else if (allSYMTAB[currentBlock].find(operand) != allSYMTAB[currentBlock].end()){
                                lastLOCCTR = currenLOCCTR[currentBlock];
                                currenLOCCTR[currentBlock] = nexLOCCTR[currentBlock] = allSYMTAB[currentBlock].find(operand)->second;
                            }
                            else {
                                errorFlag = true;
                                errorsLines.push_back(lineNo);
                                errors.push_back("**** missing operand in org statement\n");
                            }
                            
                        }
                        }
                        else {
                            errorFlag = true;
                            errorsLines.push_back(lineNo);
                            errors.push_back("**** found label in org statement\n");
                        }
                        
                    }
                    if (label.size() != 0) {
                        if ( (allSYMTAB[currentBlock].find(label) != allSYMTAB[currentBlock].end()) || (EQUTAB.find(label) != EQUTAB.end())) {
                            errorFlag = true;
                            errorsLines.push_back(lineNo);
                            errors.push_back("**** duplicate symbol\n");
                        }
                        else if(mnemonic != EQU)
                            (allSYMTAB[currentBlock])[label] = currenLOCCTR[currentBlock];
                    }
                    if (mnemonic == RSUB) {
                        if (operand.size() > 0) {
                            errorFlag = true;
                            errorsLines.push_back(lineNo);
                            errors.push_back("**** illegal operand in rsub statement\n");
                        }
                        else
                            nexLOCCTR[currentBlock] += 3;
                    }
                    else if (mnemonic == EQU) {
                        if (!errorFlag) {
                        if (label.size()!=0) {
                        if (isValidInt(operand)) {
                            if (stringTOint(operand) < 65536) {
                                EQUTAB[label] = stringTOint(operand);
                                printf("%04X %d %-8s %-6s  %s%s\n", stringTOint(operand), currentBlock, label.c_str(), mnemonic.c_str(),
                                       operand.c_str(), comment.c_str());
                            }
                            else {
                                errorFlag = true;
                                errorsLines.push_back(lineNo);
                                errors.push_back("**** so big number in equ statement\n");
                            }
                        }
                        else {
                            if (allSYMTAB[currentBlock].find(operand) != allSYMTAB[currentBlock].end()) {
                                EQUTAB[label] = allSYMTAB[currentBlock].find(operand)->second;
                                printf("%04X %d %-8s %-6s  %s%s\n", allSYMTAB[currentBlock].find(operand)->second,
                                       currentBlock, label.c_str(), mnemonic.c_str(), operand.c_str(),
                                       comment.c_str());
                            }
                            else {
                                errorFlag = true;
                                errorsLines.push_back(lineNo);
                                errors.push_back("**** missing operand in equ statement\n");
                            }
                        }
                        }
                        else {
                            errorFlag = true;
                            errorsLines.push_back(lineNo);
                            errors.push_back("**** missing label in equ statement\n");
                        }
                        }
                    }
                    else if (operand[0] == '=') {
                        if (((operand[1] == 'x') || (operand[1] == 'X')) && (operand[2] == '\'')
                            && (operand[operand.length() - 1] == '\'')) {
                            
                            if (findInLITERALS(hexTOint(Literal(true))) < 0) {
                                if ((operand.size() - 4) % 2 == 1) {
                                    errorFlag = true;
                                    errorsLines.push_back(lineNo);
                                    errors.push_back("**** odd length hex string in literal statement\n");
                                }
                                else {
                                    (allLITABLE[currentBlock])[operand] = hexTOint(Literal(true));
                                    (allLITERALS[currentBlock]).push_back(hexTOint(Literal(true)));
                                    (allLITSize[currentBlock]).push_back((operand.size() - 4) * 0.5);
                                }
                            }
                            else {
                                (allclonedlit[currentBlock]).push_back(operand);
                                (allclonedcode[currentBlock]).push_back(hexTOint(Literal(true)));
                                (allclonedloc[currentBlock]).push_back(0);
                            }
                        }
                        else if (((operand[1] == 'c') || (operand[1] == 'C')) && (operand[2] == '\'')
                                 && (operand[operand.length() - 1] == '\'')) {
                            
                            if (findInLITERALS(hexTOint(Literal(false))) < 0) {
                                (allLITABLE[currentBlock])[operand] = hexTOint(Literal(false));
                                (allLITERALS[currentBlock]).push_back(hexTOint(Literal(false)));
                                (allLITSize[currentBlock]).push_back(operand.size() - 4);
                            }
                            else {
                                (allclonedlit[currentBlock]).push_back(operand);
                                (allclonedcode[currentBlock]).push_back(hexTOint(Literal(false)));
                                (allclonedloc[currentBlock]).push_back(0);
                            }
                        }
                        else {
                            string test = operand;
                            test.erase(test.begin());
                            if (isValidInt(test)) {
                                if (findInLITERALS(stringTOint(test)) < 0) {
                                    if (stringTOint(test) > 16777216) {
                                        errorFlag = true;
                                        errorsLines.push_back(lineNo);
                                        errors.push_back("**** operand in literal statement lead to memory out of range\n");
                                    }
                                    else {
                                        (allLITABLE[currentBlock])[operand] = stringTOint(test);
                                        (allLITERALS[currentBlock]).push_back(stringTOint(test));
                                        (allLITSize[currentBlock]).push_back(3);
                                    }
                                }
                                else {
                                    (allclonedlit[currentBlock]).push_back(operand);
                                    (allclonedcode[currentBlock]).push_back(stringTOint(test));
                                    (allclonedloc[currentBlock]).push_back(0);
                                }
                            }
                            else if (operand[1] == '*'){
                            }
                            else {
                                errorFlag = true;
                                errorsLines.push_back(lineNo);
                                errors.push_back("**** illegal operand in literal statement\n");
                            }
                        }
                        nexLOCCTR[currentBlock] += 3;
                    }
                    else if (mnemonic == LTORG) {
                        if (!errorFlag) {
                        printf("     %d %-8s %-6s  %s%s\n", currentBlock, label.c_str(), mnemonic.c_str(), operand.c_str(),
                               comment.c_str());
                        map<string, int>::iterator it;
                        string labeltemp = label;
                        string mnemonictemp = mnemonic;
                        string operandtemp = operand;
                        bool found = false;
                        for (it = (allLITABLE[currentBlock]).begin(); it != (allLITABLE[currentBlock]).end(); it++) {
                            if (allSYMTAB[currentBlock].find(it->first) == allSYMTAB[currentBlock].end()) {
                                allSYMTAB[currentBlock][it->first] = currenLOCCTR[currentBlock];
                                for (int j = 0; j<(allclonedcode[currentBlock]).size(); j++) {
                                    if (it->second == (allclonedcode[currentBlock])[j]) {
                                        (allclonedloc[currentBlock])[j] = currenLOCCTR[currentBlock];
                                    }
                                }
                                label = "*";
                                mnemonic = "";
                                operand = it->first;
                                printf("%04X %d %-8s %-6s  %s%s\n", currenLOCCTR[currentBlock], currentBlock, label.c_str(),
                                       mnemonic.c_str(), operand.c_str(), comment.c_str());
                                currenLOCCTR[currentBlock] += (allLITSize[currentBlock])[findInLITERALS(it->second)];
                                found = true;
                            }
                        }
                        if (!found) {
                                errorFlag = true;
                                errorsLines.push_back(lineNo);
                                errors.push_back("**** no literals found\n");
                        }
                        nexLOCCTR[currentBlock] = currenLOCCTR[currentBlock];
                        label = labeltemp;
                        mnemonic = mnemonictemp;
                        operand = operandtemp;
                        }
                    }
                    else if (OPTAB.find(mnemonic) != OPTAB.end())
                        nexLOCCTR[currentBlock] += 3;
                    else if (mnemonic == WORD) {
                        if (isValidInt(operand)) {
                            if (stringTOint(operand) > 8388607) {
                                errorFlag = true;
                                errorsLines.push_back(lineNo);
                                errors.push_back("**** illegal operand in word statement\n");
                            }
                            else
                                nexLOCCTR[currentBlock] += 3;
                        }
                        else {
                            errorFlag = true;
                            errorsLines.push_back(lineNo);
                            errors.push_back("**** illegal operand in word statement\n");
                        }
                    }
                    else if (mnemonic == RESW) {
                        if (isValidInt(operand)) {
                            if (stringTOint(operand) > 0) {
                                nexLOCCTR[currentBlock] += (3 * (stringTOint(operand)));
                                if (nexLOCCTR[currentBlock] > 32768) {
                                    errorFlag = true;
                                    errorsLines.push_back(lineNo);
                                    errors.push_back("**** reserve lead to memory out of range\n");
                                }
                            }
                            else {
                                errorFlag = true;
                                errorsLines.push_back(lineNo);
                                errors.push_back("**** illegal operand in resw statement\n");
                            }
                        }
                        else {
                            errorFlag = true;
                            errorsLines.push_back(lineNo);
                            errors.push_back("**** illegal operand in resw statement\n");
                        }
                    }
                    else if (mnemonic == RESB) {
                        if (isValidInt(operand)) {
                            if (stringTOint(operand) > 0) {
                                nexLOCCTR[currentBlock] += (stringTOint(operand));
                                if (nexLOCCTR[currentBlock] > 32768) {
                                    errorFlag = true;
                                    errorsLines.push_back(lineNo);
                                    errors.push_back("**** reserve lead to memory out of range\n");
                                }
                            }
                            else {
                                errorFlag = true;
                                errorsLines.push_back(lineNo);
                                errors.push_back("**** illegal operand in resb statement\n");
                            }
                        }
                        else {
                            errorFlag = true;
                            errorsLines.push_back(lineNo);
                            errors.push_back("**** illegal operand in resb statement\n");
                        }
                    }
                    else if (mnemonic == BYTE) {
                        unsigned long length = 0;
                        if (((operand[0] == 'c') || (operand[0] == 'C')) && (operand[1] == '\'')
                            && (operand[operand.length() - 1] == '\'')) {
                            length = operand.length() - 3;
                            nexLOCCTR[currentBlock] += length;
                        }
                        else if (((operand[0] == 'x') || (operand[0] == 'X')) && (operand[1] == '\'')
                                 && (operand[operand.length() - 1] == '\'')) {
                            length = operand.length() - 3;
                            if (length % 2 == 0)
                                nexLOCCTR[currentBlock] += (length*0.5);
                            else {
                                errorFlag = true;
                                errorsLines.push_back(lineNo);
                                errors.push_back("**** odd length hex string in byte statement\n");
                            }
                            string operandCopy = operand;
                            operandCopy.erase(operandCopy.begin());
                            operandCopy.erase(operandCopy.begin());
                            operandCopy.erase(operandCopy.end()-1);
                            if (!isValidHex(operandCopy)) {
                                errorFlag = true;
                                errorsLines.push_back(lineNo);
                                errors.push_back("**** illegal operand in HEX statement\n");
                            }
                        }
                        else
                        {
                            errorFlag = true;
                            errorsLines.push_back(lineNo);
                            errors.push_back("**** illegal operand in byte statement\n");
                        }
                    }
                    else if (mnemonic == END) {
                        map<string, int>::iterator it;
                        string labeltemp = label;
                        string mnemonictemp = mnemonic;
                        string operandtemp = operand;
                        int currentblock2 = currentBlock;
                        for (int k = 0; k<blocks.size(); k++) {
                            currentBlock = k;
                        for (it = (allLITABLE[currentBlock]).begin(); it != (allLITABLE[currentBlock]).end(); it++) {
                            if (allSYMTAB[currentBlock].find(it->first) == allSYMTAB[currentBlock].end()) {
                                allSYMTAB[currentBlock][it->first] = currenLOCCTR[currentBlock];
                                for (int j = 0; j<(allclonedcode[currentBlock]).size(); j++) {
                                    if (it->second == (allclonedcode[currentBlock])[j]) {
                                        (allclonedloc[currentBlock])[j] = currenLOCCTR[currentBlock];
                                    }
                                }
                                label = "*";
                                mnemonic = "";
                                operand = it->first;
                                printf("%04X %d %-8s %-6s  %s%s\n", currenLOCCTR[currentBlock], currentBlock, label.c_str(), mnemonic.c_str(),
                                       operand.c_str(), comment.c_str());
                                currenLOCCTR[currentBlock] += (allLITSize[currentBlock])[findInLITERALS(it->second)];
                            }
                        }
                        }
                        label = labeltemp;
                        mnemonic = mnemonictemp;
                        operand = operandtemp;
                        currentBlock = currentblock2;
                        nexLOCCTR[currentBlock] += 3;
                    }
                    else if (mnemonic != ORG && mnemonic != USE){
                        errorFlag = true;
                        errorsLines.push_back(lineNo);
                        errors.push_back("**** unrecognized mnemonic\n");
                    }
                }
            }
            if (errorFlag && mnemonic != LTORG) {
                line.insert(0, "       ");
                cout << line << endl;
                for_each(errors.begin(), errors.end(), print);
            }
            else if (errorFlag && mnemonic == LTORG)
                for_each(errors.begin(), errors.end(), print);
            else if(mnemonic != LTORG && mnemonic != EQU)
                printf("%04X %d %-8s %-6s  %s%s\n", currenLOCCTR[currentBlock], currentBlock, label.c_str(), mnemonic.c_str(), operand.c_str(),
                       comment.c_str());
            currenLOCCTR[currentBlock] = nexLOCCTR[currentBlock];
            if (currentBlock == 0)
                progLength = currenLOCCTR[currentBlock] - startingAddress;
        }
        else{
            if (line[0] != '.' && line.size()>1 && line.size()<9) {
                line.insert(0, "       ");
                cout << line << endl;
                cout << "**** missing mnemonic" << endl;
            }
            else if (line[0]!='\n')
                cout <<  "       " << line << endl;
        }
        line = "";
    }
    
    if (line[0] != '.' && line.size()>1) {
        cout <<  "       " << line << endl;
        cout << "**** statement should not follow end statement" << endl;
    }
    else if (line.size()>1)
        cout <<  "       " << line << endl;
    while (getline(src, line)) {
        if (line[0] != '.') {
            cout <<  "       " << line << endl;
            cout << "**** statement should not follow end statement" << endl;
        }
        else
            cout <<  "       " << line << endl;
    }
    
    //..............................................................
    //..............................................................
    //....................... LISTFILE .................................
    
    freopen("/Users/ahmadbarakat/Desktop/LISFILE", "w", stdout);
    
    printf("SIC Assembler By: Ahmed Emad\n\n");
    
    lineNo = 0;
    locctrString = "";
    label = "";
    mnemonic = "";
    operand = "";
    int op = 0;
    bool indexing = false;
    
    getline(inter, line);
    getline(inter, line);
    while (getline(inter, line) && (mnemonic != END)) {
        if (line[7] != '.' && line[0] != '*' && line.size()>14) {
            lineNo++;
            indexing = false;
            if ( find(errorsLines.begin(), errorsLines.end(), lineNo) == errorsLines.end() ) {
                locctrString = line.substr(0, 4);
                blocknum = line.substr(5,1);
                label = line.substr(7, 8);
                mnemonic = line.substr(16, 6);
                if (isValidInt(blocknum))
                    currentBlock = stringTOint(blocknum);
                if (line.size() > 24)
                    operand = line.substr(24, 17);
                else
                    operand = "";
                if (line.size() > 40)
                    comment = line.substr(40);
                else
                    comment = "";
                locctrString = trim(locctrString);
                if (operand[0] == '=')
                    label = trimBYTE(label);
                else
                    label = trim(label);
                mnemonic = trim(mnemonic);
                if (mnemonic == BYTE || operand[0] == '=')
                    operand = trimBYTE(operand);
                else if (operand.find(",x") != string::npos) {
                    indexing = true;
                    operand = trimIndex(operand);
                }
                else if (mnemonic != WORD)
                    operand = trim(operand);
                if ((mnemonic == START) || (mnemonic == END)) {
                    printf("%s %s        %-8s %-6s  %s%s\n", locctrString.c_str(), blocknum.c_str(), label.c_str(),
                           mnemonic.c_str(), operand.c_str(), comment.c_str());
                }
                else if (mnemonic == WORD) {
                    if (stringTOint(operand) < 0) {
                        int operand2 = 16777215 + stringTOint(operand) + 1;
                        printf("%s %s %06X %-8s %-6s  %s%s\n", locctrString.c_str(), blocknum.c_str(), operand2, label.c_str(),
                               mnemonic.c_str(), operand.c_str(), comment.c_str());
                    }
                    else
                        printf("%s %s %06X %-8s %-6s  %s%s\n", locctrString.c_str(), blocknum.c_str(), stringTOint(operand), label.c_str(),
                                mnemonic.c_str(), operand.c_str(), comment.c_str());
                }
                else if (mnemonic == LTORG) {
                    line.insert(7, "       ");
                    cout << line << endl;
                }
                else if (mnemonic == EQU) {
                    line.insert(7, "       ");
                    cout << line << endl;
                }
                else if (mnemonic == ORG) {
                    line.insert(7, "       ");
                    cout << line << endl;
                }
                else if (mnemonic == USE) {
                    line.insert(7, "       ");
                    cout << line << endl;
                }
                else if ((OPTAB.find(mnemonic) != OPTAB.end()) || ((trim(operand))[0] == '0')) {
                    if ((trim(operand))[0] == '0') {
                        op = hexTOint(trim(operand));
                        if (indexing)
                            op += 32768;
                        printf("%s %s %02X%04X %-8s %-6s  %s%s\n", locctrString.c_str(), blocknum.c_str(), OPTAB.find(mnemonic)->second,
                               op, label.c_str(), mnemonic.c_str(),
                               operand.c_str(), comment.c_str());
                    }
                    else if (operand[0] == '=' && operand[1] == '*') {
                        op = hexTOint(locctrString);
                        printf("%s %s %02X%04X %-8s %-6s  %s%s\n", locctrString.c_str(), blocknum.c_str(), OPTAB.find(mnemonic)->second,
                               op, label.c_str(), mnemonic.c_str(),
                               operand.c_str(), comment.c_str());
                    }
                    else if (allSYMTAB[currentBlock].find(trim(operand)) != allSYMTAB[currentBlock].end()) {
                        if (indexing) {
                            op = allSYMTAB[currentBlock].find(trim(operand))->second;
                            op += 32768;
                        }
                        else
                            op = allSYMTAB[currentBlock].find(trim(operand))->second;
                        printf("%s %s %02X%04X %-8s %-6s  %s%s\n", locctrString.c_str(), blocknum.c_str(), OPTAB.find(mnemonic)->second,
                               op, label.c_str(), mnemonic.c_str(),
                               operand.c_str(), comment.c_str());
                    }
                    else if (allSYMTAB[currentBlock].find(operand) != allSYMTAB[currentBlock].end()) {
                        op = allSYMTAB[currentBlock].find(operand)->second;
                        printf("%s %s %02X%04X %-8s %-6s  %s%s\n", locctrString.c_str(), blocknum.c_str(), OPTAB.find(mnemonic)->second,
                               op, label.c_str(), mnemonic.c_str(),
                               operand.c_str(), comment.c_str());
                    }
                    else if (EQUTAB.find(trim(operand)) != EQUTAB.end()) {
                        printf("%s %s %02X%04X %-8s %-6s  %s%s\n", locctrString.c_str(), blocknum.c_str(), OPTAB.find(mnemonic)->second,
                               EQUTAB.find(trim(operand))->second, label.c_str(), mnemonic.c_str(),
                               operand.c_str(), comment.c_str());
                    }
                    else if (operand[0] == '=') {
                        op = (allclonedloc[currentBlock])[findInClonedLIT(operand)];
                        printf("%s %s %02X%04X %-8s %-6s  %s%s\n", locctrString.c_str(), blocknum.c_str(), OPTAB.find(mnemonic)->second,
                               op, label.c_str(), mnemonic.c_str(),
                               operand.c_str(), comment.c_str());
                    }
                    else if (mnemonic == RSUB) {
                        printf("%s %s %02X0000 %-8s %-6s  %s%s\n", locctrString.c_str(), blocknum.c_str(), OPTAB.find(mnemonic)->second,
                               label.c_str(), mnemonic.c_str(),
                               operand.c_str(), comment.c_str());
                    }
                    else {
                        if ((trim(operand))[0] != '0') {
                            line.insert(7, "       ");
                            cout << line << endl;
                            cout << "**** undefined symbol in operand" << endl;
                        }
                    }
                }
                else if (operand[0] == '=' && label[0]!=' ') {
                    string labeltemp = label;
                    string mnemonictemp = mnemonic;
                    string operandtemp = operand;
                    label = "*";
                    mnemonic = "";
                    if (operand[1] == 'x' || operand[1] == 'X') {
                        printHexSixes();
                    }
                    else if (operand[1] == 'c' || operand[1] == 'C') {
                        printIntSixes();
                    }
                    else {
                        operand.erase(operand.begin());
                        printf("%s %s %06X %-8s %-6s  %s%s\n", locctrString.c_str(), blocknum.c_str(), stringTOint(operand), label.c_str(),
                               mnemonic.c_str(), operand.c_str(), comment.c_str());
                    }
                    label = labeltemp;
                    mnemonic = mnemonictemp;
                    operand = operandtemp;
                }
                else if ((mnemonic == RESW) || (mnemonic == RESB))
                    printf("%s %s        %-8s %-6s  %s%s\n", locctrString.c_str(), blocknum.c_str(), label.c_str(), mnemonic.c_str(),
                           operand.c_str(), comment.c_str());
                else if (mnemonic == BYTE) {
                    if (((operand[0] == 'x') || (operand[0] == 'X')))
                        printHexSixes();
                    else if (((operand[0] == 'c') || (operand[0] == 'C')))
                        printIntSixes();
                }
            }
            else {
                line.insert(7, "       ");
                cout << line << endl;
            }
        }
        else if (line[7] == '.') {
            cout << "       " << line << endl;
        }
        else
            cout << line << endl;
    }
    
    if (line[0] != '*' && line.size()>1)
        cout <<  "       " << line << endl;
    else if (line.size()>1)
        cout << line << endl;
    while (getline(inter, line)) {
        if (line[0] != '*')
            cout <<  "       " << line << endl;
        else
            cout << line << endl;
    }
    
    //..............................................................
    //..............................................................
    //....................... PASS 2 ...............................
    
    freopen("/Users/ahmadbarakat/Desktop/OBJFILE", "w", stdout);
    
    mnemonic = "";
    opcode = "";
    string record = "";
    int counter = 0, LOCCTR = 0, startRecord = 0;
    lineNo = 0;
    getline(lis, line);
    getline(lis, line);
    
    if (errorsLines.size() == 0) {
        while (getline(lis, line) && (mnemonic != END)) {
            if (line[0] != '.' && line[0] != '*' && line.size() > 5) {
                lineNo++;
                locctrString = line.substr(0, 4);
                blocknum = line.substr(5,1);
                opcode = line.substr(7, 6);
                if (line.size() > 14)
                    label = line.substr(14, 8);
                if (line.size() > 23)
                    mnemonic = line.substr(23, 6);
                locctrString = trim(locctrString);
                opcode = trim(opcode);
                label = trim(label);
                mnemonic = trim(mnemonic);
                if (mnemonic == BYTE)
                    operand = trimBYTE(operand);
                else if (operand.find(",x") != string::npos)
                    operand = trimIndex(operand);
                else
                    operand = trim(operand);
                if (((counter > 29) || (counter + (opcode.size() / 2) > 30) || (mnemonic == RESW) || (mnemonic == RESB)
                     || (mnemonic == ORG) || (mnemonic == USE)) && counter!=0) {
                    printf("T00%04X%02X%s\n", startRecord, counter, record.c_str());
                    record = "";
                    counter = 0;
                }
                
                if (mnemonic == START || lineNo == 0)
                    printf("H%-6s%06X%06X\n", label.c_str(), startingAddress, progLength);
                else if (mnemonic == END) {
                    if (counter > 0)
                        printf("T00%04X%02X%s\n", startRecord, counter, record.c_str());
                    printf("E%06X\n", startingAddress);
                }
                else if ((opcode.size() != 0) && (counter + (opcode.size() / 2) <= 30)) {
                    if (counter == 0) {
                        if (locctrString.size() != 0) {
                            stringstream ss;
                            ss << locctrString;
                            ss >> hex >> LOCCTR;
                        }
                        startRecord = LOCCTR;
                    }
                    record += opcode;
                    counter += (opcode.size() / 2);
                    LOCCTR += (opcode.size() / 2);
                }
            }
        }
    }
    else
        cout << "SomeThing Went Wrong :(" << endl;
    //system("pause");
    
    return 0;
}

string intTOstring(int number)
{
    stringstream ss;
    ss << number;
    return ss.str();
}

int stringTOint(string number)
{
    return atoi(number.c_str());
}

int hexTOint(string hexValue)
{
    int decimalValue;
    stringstream ss;
    ss << hexValue;
    ss >> hex >> decimalValue;
    return decimalValue;
}

string intToHex(int decimalValue)
{
    stringstream ss;
    ss << hex << decimalValue;
    string result = ss.str();
    return result;
}

string trim(string line)
{
    char * x = new char[line.size() + 1];
    copy(line.begin(), line.end(), x);
    x[line.size()] = '\0';
    delete[] x;
    for (int i = 0; i<line.size(); i++) {
        if (!((x[i]>64 && x[i]<91) || (x[i]>96 && x[i]<123) || (x[i]>47 && x[i]<58)))
            x[i] = 0;
    }
    string lineTrimmed(x);
    return lineTrimmed;
}

string trimIndex(string line)
{
    char * x = new char[line.size() + 1];
    copy(line.begin(), line.end(), x);
    x[line.size()] = '\0';
    delete[] x;
    for (int i = 0; i<line.size(); i++) {
        if (!((x[i]>64 && x[i]<91) || (x[i]>96 && x[i]<123) || (x[i]>47 && x[i]<58) || x[i] == 44))
            x[i] = 0;
    }
    string lineTrimmed(x);
    return lineTrimmed;
}

string trimBYTE(string line)
{
    char * x = new char[line.size() + 1];
    copy(line.begin(), line.end(), x);
    x[line.size()] = '\0';
    delete[] x;
    for (int i = 0; i<line.size(); i++) {
        if ( (x[i] < 33 || x[i]>126) && x[i]!=32)
            x[i] = 0;
    }
    string lineTrimmed(x);
    return lineTrimmed;
}

string trimWORD(string line)
{
    char * x = new char[line.size() + 1];
    copy(line.begin(), line.end(), x);
    x[line.size()] = '\0';
    delete[] x;
    for (int i = 0; i<line.size(); i++) {
        if ( !((x[i]>64 && x[i]<91) || (x[i]>96 && x[i]<123) || (x[i]>47 && x[i]<58) || x[i]=='-') )
            x[i] = 0;
    }
    string lineTrimmed(x);
    return lineTrimmed;
}

bool isValidString(string input)
{
    char * x = new char[input.size() + 1];
    copy(input.begin(), input.end(), x);
    x[input.size()] = '\0';
    delete[] x;
    for (int i = 0; i<input.size() - 1; i++) {
        if ( !((x[i]>64 && x[i]<91) || (x[i]>96 && x[i]<123) || (x[i]>47 && x[i]<58) ||
               x[i] == 32 ) ) {
            return false;
        }
    }
    return true;
}

bool isValidStringIndexing(string input)
{
    char * x = new char[input.size() + 1];
    copy(input.begin(), input.end(), x);
    x[input.size()] = '\0';
    delete[] x;
    for (int i = 0; i<input.size() - 1; i++) {
        if ( !((x[i]>64 && x[i]<91) || (x[i]>96 && x[i]<123) || (x[i]>47 && x[i]<58) ||
              x[i] == 32 || (x[i]==44 && x[i+1]==88) || (x[i]==44 && x[i+1]==120) ) ) {
            return false;
        }
    }
    return true;
}

bool isValidHex(string input)
{
    char * x = new char[input.size() + 1];
    copy(input.begin(), input.end(), x);
    x[input.size()] = '\0';
    delete[] x;
    for (int i = 0; i<input.size(); i++) {
        if (!((x[i]>47 && x[i]<58) || (x[i]>64 && x[i]<71) || (x[i]>96 && x[i]<103))) {
            return false;
        }
    }
    return true;
}

bool isValidInt(string input)
{
    char * x = new char[input.size() + 1];
    copy(input.begin(), input.end(), x);
    x[input.size()] = '\0';
    delete[] x;
    if ( !((x[0]>47 && x[0]<58)) && x[0]!='-' ) {
        return false;
    }
    for (int i = 1; i<input.size(); i++) {
        if ( !((x[i]>47 && x[i]<58)) ) {
            return false;
        }
    }
    return true;
}

vector<int> stringTOintegers(string number)
{
    vector<int> integers;
    for (int i = 0; i<number.size(); i++) {
        int x = number[i];
        integers.push_back(x);
    }
    return integers;
}

void printIntSixes()
{
    int counter = 0;
    int i;
    string operand2;
    if (operand[0] == '=') {
        operand2 = operand.substr(3, operand.size() - 4);
    }
    else {
        operand2 = operand.substr(2, operand.size() - 3);
    }
    
    vector<int> integers = stringTOintegers(operand2);
    printf("%s %s ", locctrString.c_str(), blocknum.c_str());
    for (i = 0; i<integers.size(); i++) {
        if (i < 3)
            printf("%X", integers[i]);
        else
            break;
    }
    if (i < 3) {
        for (i = i * 2; i<6; i++)
            printf(" ");
    }
    printf(" %-8s %-6s  %s%s", label.c_str(), mnemonic.c_str(), operand.c_str(), comment.c_str());
    if (integers.size() > 3) {
        for (i = 3; i<integers.size(); i++) {
            if (!counter)
                printf("\n       ");
            printf("%X", integers[i]);
            counter++;
            if (counter == 3)
                counter = 0;
        }
        if (counter < 3) {
            printf("\n");
        }
    }
    else
        printf("\n");
}

void printHexSixes()
{
    int counter = 0;
    int i;
    string operand2;
    if (operand[0] == '=') {
        operand2 = operand.substr(3, operand.size() - 4);
    }
    else {
        operand2 = operand.substr(2, operand.size() - 3);
    }
    
    
    if (isValidHex(operand2)) {
    
    char * x = new char[operand2.size() + 1];
    copy(operand2.begin(), operand2.end(), x);
    x[operand2.size()] = '\0';
    delete[] x;
    
    printf("%s %s ", locctrString.c_str(), blocknum.c_str());
    for (i = 0; i<operand2.size(); i++) {
        if (i < 6)
            printf("%c", x[i]);
        else
            break;
    }
    if (i < 6) {
        for (i; i<6; i++)
            printf(" ");
    }
    printf(" %-8s %-6s  %s%s", label.c_str(), mnemonic.c_str(), operand.c_str(), comment.c_str());
    if (operand2.size() > 6) {
        for (i = 6; i<operand2.size(); i++) {
            if (!counter)
                printf("\n       ");
            printf("%c", x[i]);
            counter++;
            if (counter == 6)
                counter = 0;
        }
        if (counter < 6) {
            printf("\n");
        }
    }
    else
        printf("\n");
        
    }
}

string Literal(bool isHex)
{
    int i;
    string operand2 = operand.substr(3, operand.size() - 4);
    char * x = new char[operand2.size() + 1];
    copy(operand2.begin(), operand2.end(), x);
    x[operand2.size()] = '\0';
    delete[] x;
    if (!isHex) {
        vector<int> integers = stringTOintegers(operand2);
        string out = "";
        for (i = 0; i<integers.size(); ++i) {
            out += intToHex(integers[i]);
        }
        return out;
    }
    else {
        return operand2;
    }
}

int findInLITERALS(int value)
{
    int found = -1;
    for (int i = 0; i<(allLITERALS[currentBlock]).size(); i++) {
        if((allLITERALS[currentBlock])[i] == value)
            found = i;
    }
    return found;
}

int findInClonedLIT(string value)
{
    int found = -1;
    for (int i = 0; i<(allclonedlit[currentBlock]).size(); i++) {
        if((allclonedlit[currentBlock])[i] == value)
            found = i;
    }
    return found;
}

int findInBlocks(string value)
{
    int found = -1;
    for (int i = 0; i<blocks.size(); i++) {
        if(blocks[i] == value)
            found = i;
    }
    return found;
}

void print(string element)
{
    cout << element;
}
