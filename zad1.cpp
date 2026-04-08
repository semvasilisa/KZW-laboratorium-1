#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

struct Task {
    int id;
    int r, p, q;
};

void readFile(string fileName, int size, Task* tasks){
   ifstream f(fileName);

    if (!f.is_open()) {
        cerr << "Error opening the file!" << endl;
        return;
    }
    int id, r, p, q;
    
    //ignoring the first line
    string line;
    if (!getline(f, line)) {
      cerr << "Error reading the first line or the file is empty" << endl;
      return;
    }

    for (int i = 0; i < size; i++) {
      if (f >> r >> p >> q) {  
            tasks[i].id = i+1;
            tasks[i].r = r;
            tasks[i].p = p;
            tasks[i].q = q;
        } else {
            cerr << "Error reading data for task " << i + 1 << endl;
            break; 
        }
    }
    f.close();
};

int calcCmax(Task* tasks, int size){
  // m - moment, w którym maszyna kończy przetwarzanie zadania i
  // Cmax - maksymalny czas zakończenia wszystkich zadań
  int m = 0;
  int Cmax = 0;
  for(int i = 0; i < size; i++){
    m = max(m,tasks[i].r) + tasks[i].p;
    Cmax = max(m + tasks[i].q, Cmax);
  }
  return Cmax;
};

bool compare(Task a, Task b){ return (a.r<b.r); };

bool compareRQ(Task a, Task b){
  return (a.r + a.q > b.r + b.q);
};

void sortR(Task* tasks, int size){
   sort(tasks,tasks+size,compare);
};

void sortRQ(Task* tasks, int size){
   sort(tasks,tasks+size,compareRQ);
};

void printAllTasks(Task* tasks, int size){
  for (int i = 0; i < size; ++i) {
    cout << "Task " << i + 1 << ": "
          << "id = " << tasks[i].id << ", "
          << "r = " << tasks[i].r << ", "
          << "p = " << tasks[i].p << ", "
          << "q = " << tasks[i].q << endl;
  }
}

int main() {
  const string files[] = {"data1.txt", "data2.txt", "data3.txt", "data4.txt"};
  int totalCmax = 0;
   
  //cout << "-------------sortowanie R-----------------" << endl;
  cout << "-------------sortowanie RQ-----------------" << endl;
  for (int i = 0; i < 4; ++i) {
      ifstream infile(files[i]);
      int rozmiar = 0;

      if (infile.good()) {
          string sLine;
          getline(infile, sLine);
          rozmiar = stoi(sLine); 
      }
      infile.close();

      
      Task* tasks = new Task[rozmiar];

      readFile(files[i], rozmiar, tasks);

      
      cout << "---" << (i + 1) << " zbiór danych---" << endl;
      // sortR(tasks,rozmiar);
      sortRQ(tasks,rozmiar);
      int Cmax = calcCmax(tasks, rozmiar); 
      cout << "Cmax = " << Cmax << endl;

      totalCmax += Cmax;

      delete[] tasks;
  }

  cout << "Suma wszystkich Cmax = " << totalCmax << endl;

  return 0;
}