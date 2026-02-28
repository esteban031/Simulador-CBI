#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

void Fetch(string &line){
    vector<string> delimitador;
    istringstream iss(line);   // flujo de entrada desde la cadena
    string auxPalabra;
    int cnt = 0;

    while (iss >> auxPalabra && cnt < 4){
        delimitador.push_back(auxPalabra);
        cnt++;

    } 
    
    // para determinar que instruccion realizar

    string instruction = delimitador[0];

    if (instruction == "SET"){
        instructionSET(delimitador);
    }
    else if (instruction == "LDR"){
        instructionLDR(delimitador);
    }
    else if (instruction == "ADD"){
        instructionADD(delimitador);
    }
    else if (instruction == "INC"){
        instructionINC(delimitador);
    }
    else if (instruction == "DEC"){
        instructionDEC(delimitador);
    }
    else if (instruction == "STR"){
        instructionSTR(delimitador);
    }
    else if (instruction == "SHW"){
        instructionSHW(delimitador);
    }
    else if (instruction == "PAUSE"){
        instructionPAUSE(delimitador);
    }
    else if (instruction == "END"){
        instructionEND(delimitador);
    }



}
