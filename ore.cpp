//
//  main.cpp
//  CMPE 130 Project
//
//  Copyright © 2018 Nishin Shouzab. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
using namespace std;

int IND = -1;
int N = 3; //shift amount
vector<string> WORDS;

void sortf(string fname);
int searchm(string message);

int main(int argc, const char * argv[]) {
    //Encrypt the messages
    ifstream fin_m;
    ofstream fout_e;
    fin_m.open("messages.txt");
    if (fin_m.fail()) {
        cout << "The messages file could not be opened.\n";
    }
    fout_e.open("encrypted.txt");
    if (fout_e.fail()) {
        cout << "The encrypted file could not be opened.\n";
    }
    while (!fin_m.eof()) {
        char c;
        fin_m.get(c);
        if (c > 0 && c < 127) {
            c = (c+ N)%127;
        }
        fout_e << c;
    }
    string word;
    fin_m.clear();
    fin_m.seekg(0, ios::beg);
    while (getline(fin_m, word)) {
        WORDS.push_back(word);
    }
    sort(WORDS.begin(), WORDS.end());
    
    fin_m.close();
    fout_e.close();
    
    //Sort the encrypted file alphabetically
    auto start = std::chrono::high_resolution_clock::now();
    sortf("encrypted.txt");
    auto finish = std::chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = finish - start;
    cout << elapsed.count()*1000000000 << "nanoseconds\n";
    //Search for a message and return an index of it
    string search = "Good, What about you?";
    cout << "The phrase \"" << search << "\" was found at index: " << searchm("Good, What about you?") << endl;
    return 0;
}

void sortf(string fname){
    ofstream fout_se;
    
    fout_se.open("sorted_encrypted.txt");
    if (fout_se.fail()) {
        cout << "The sorted encrypted file could not be opened.\n";
    }

    for (int i = 0; i < WORDS.size(); i++) {
        for (int j = 0; j < WORDS[i].size(); j++) {
            WORDS[i][j] = (WORDS[i][j]+N)%127;
            fout_se << WORDS[i][j];
        }
        fout_se << endl;
    }
    fout_se.close();
}

int searchm(string message){
    ifstream fin_me;
    fin_me.open("messages.txt");
    if (fin_me.fail()) {
        cout << "The messages file could not be opened.\n";
    }
    IND = -1;
    while (!fin_me.eof()) {
        ++IND;
        string m;
        getline(fin_me, m);
        if(m == message){
            return IND;
        }
    }
    return IND;
}
