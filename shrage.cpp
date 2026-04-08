#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

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
  // return (a.r+a.p+a.q>b.r+b.p+b.q);
  return (a.q > b.q);
  // return a.r + a.q > b.r + b.q;
  // return (a.r > b.r);
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

bool compareQ(Task a, Task b){ return (a.q<b.q);}

int main() {
  ifstream infile("data4.txt");
  int rozmiar = 0;

  if (infile.good())
  {
    string sLine;
    getline(infile, sLine);
    rozmiar = stoi(sLine);
  }
  infile.close();

  Task tasks[rozmiar]; // posortowane według r
  readFile("data4.txt", rozmiar, tasks);
  sortR(tasks, rozmiar);
  //printAllTasks(tasks,rozmiar);

  //cout << "________________________________________________________________________________________" << endl;

  int m = 0; // aktualny moment w który urządzenie jest wolne
  int Cmax = 0; // moment kiedy ostatnie zadanie wystygnie 
  vector<Task> ready; // wektor dostępnych zadań
  int i = 0; // interator

  while(i < rozmiar || !ready.empty()){
    // if current element r is less than m, look for all elements with r<currentM
    while(i < rozmiar && tasks[i].r <= m){
      ready.push_back(tasks[i]);
      i++;
    }

    // if current element r is more than m, set it to currentR
    if(ready.empty()){
      m = tasks[i].r; 
    }
    // if we have tasks whcih has r<=m -> choose one with the highest q
    else{
      auto it = max_element(ready.begin(), ready.end(), compareQ);
      Task taskToComplete = *it; // turn max to task
      ready.erase(it);


      m = max(m, taskToComplete.r) + taskToComplete.p;
      Cmax = max(m + taskToComplete.q, Cmax);


      // cout << "Wykonano zadanie id=" << taskToComplete.id 
      //    << " m=" << m 
      //    << " Cmax=" << Cmax << endl;
    }
  }

  cout <<"4 zbiór danych, Cmax = "<< Cmax << endl;

  return 0;
}