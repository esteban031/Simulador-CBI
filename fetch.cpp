#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

vector<string> Fetch(string &line){
    vector<string> delimitador;
    istringstream iss(line);   // flujo de entrada desde la cadena
    string palabra;

    while (iss >> palabra){
        delimitador.push_back(palabra);


    }





}
