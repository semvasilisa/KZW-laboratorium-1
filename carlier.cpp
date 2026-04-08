#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

using namespace std;

struct Task {
    int id;
    int r, p, q;
};

struct SchrageResult {
    int Cmax;
    vector<Task> order; // kolejność wykonania zadań
    vector<int> completionTimes; // momenty kończenia wykonania zadań
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

int schragePmtn(Task* tasks, int size){
  int m = 0;
  int Cmax = 0;
  vector<Task> ready;
  int i = 0;
  Task* current = nullptr; // задача которую сейчас выполняем

  while(i < size || !ready.empty() || current != nullptr){
        
      // добавляем все задачи которые стали доступны
      while(i < size && tasks[i].r <= m){
          ready.push_back(tasks[i]);
          i++;
      }

      // если очередь пуста и нет текущей задачи - прыгаем к следующему r
      if(ready.empty() && current == nullptr){
          m = tasks[i].r;
          continue;
      }

      // если есть задачи в очереди - проверяем нужно ли прервать текущую
      if(!ready.empty()){
          auto it = max_element(ready.begin(), ready.end(), compareQ);
            
          // прерываем текущую если новая задача имеет больший q
          if(current != nullptr && it->q > current->q){
              ready.push_back(*current);  // возвращаем текущую в очередь
              delete current;
              current = nullptr;
          }

          // если нет текущей задачи - берём лучшую из очереди
          if(current == nullptr){
              current = new Task(*it);
              ready.erase(it);
          }
      }

      // определяем до какого момента выполняем текущую задачу
      int nextR = (i < size) ? tasks[i].r : INT_MAX;
        
      // выполняем до следующего r или до конца задачи
      int executeUntil = min(m + current->p, nextR);
      current->p -= (executeUntil - m);  // уменьшаем оставшееся p
      m = executeUntil;

      // если задача полностью выполнена
      if(current->p == 0){
          Cmax = max(Cmax, m + current->q);
          delete current;
          current = nullptr;
      }
  }
  return Cmax;
}

SchrageResult schrage(Task* tasks, int rozmiar){
  int m = 0; // aktualny moment w który urządzenie jest wolne
  int Cmax = 0; // moment kiedy ostatnie zadanie wystygnie 
  vector<Task> ready; // wektor dostępnych zadań
  int i = 0; // interator
  vector<Task> order;
  vector<int> completionTimes;

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
      order.push_back(taskToComplete);
      ready.erase(it);


      m = max(m, taskToComplete.r) + taskToComplete.p;
      completionTimes.push_back(m);
      Cmax = max(m + taskToComplete.q, Cmax);

      // cout << "Wykonano zadanie id=" << taskToComplete.id 
      //    << " m=" << m 
      //    << " p=" << taskToComplete.p
      //    << " Cmax=" << Cmax << endl;
    }
  }

  return {Cmax, order, completionTimes};
}

void carlier(Task* tasks, int n, int& UB){
  SchrageResult result = schrage(tasks, n);

  // jeśli Cmax jest lepszy od UB, aktualizujemy UB
  if(result.Cmax < UB) UB = result.Cmax;

  int B = -1;
  for(int i = 0; i< result.order.size(); i++){
    if(result.completionTimes[i] + result.order[i].q == result.Cmax){
      B = i; // task position in order of execution 
      break;
    }
  }

  // szukamy A - początek bloku krytycznego 
  int A = -1;
  int j = B;
  while(j>0 && result.completionTimes[j] - result.order[j].p == result.completionTimes[j-1]){
    j--;
  }
  A = j;

  // szukamy C - zadanie które musze poprawić wynik po zmianie
  int C = -1;
  int minQ = result.order[B].q;
  for(int i = A; i < B; i++){
      if(result.order[i].q < minQ){
          minQ = result.order[i].q;
          C = i;
      }
  }

  if(C == -1) return; // jeśli C == -1, nie ma nic do poprawy, wracamy

  // wyznaczamy K
  vector<Task> K;
  for(int i = C+1; i <= B; i++){
    K.push_back(result.order[i]);
  }
  
  // wyznaczmy rK, qK, pK
  int rK = INT_MAX;
  int pK = 0;
  int qK = INT_MAX;

  for(int i = 0; i< K.size(); i++){
    rK = min(rK, K[i].r);   
    pK += K[i].p;           
    qK = min(qK, K[i].q);
  }

  /////////////////////////////////
  // ========== ВЕТКА 1: меняем r_c ==========

// находим саму задачу C в массиве tasks
// (важно! result.order[C] это копия, нам нужен оригинал в tasks)
int taskC_id = result.order[C].id;

// находим индекс C в оригинальном массиве tasks
int originalC = -1;
for(int i = 0; i < n; i++){
    if(tasks[i].id == taskC_id){
        originalC = i;
        break;
    }
}

// сохраняем старое значение r_c
int old_r = tasks[originalC].r;

// новое r_c = max(старый r_c, rK + pK)
tasks[originalC].r = max(tasks[originalC].r, rK + pK);

// считаем LB через SchragePmtn
int LB1 = schragePmtn(tasks, n);

// если LB обещающий - углубляемся
if(LB1 < UB){
    carlier(tasks, n, UB);
}

// восстанавливаем r_c
tasks[originalC].r = old_r;

// ========== ВЕТКА 2: меняем q_c ==========

// сохраняем старое значение q_c
int old_q = tasks[originalC].q;

// новое q_c = max(старый q_c, qK + pK)
tasks[originalC].q = max(tasks[originalC].q, qK + pK);

// считаем LB через SchragePmtn
int LB2 = schragePmtn(tasks, n);

// если LB обещающий - углубляемся
if(LB2 < UB){
    carlier(tasks, n, UB);
}

// восстанавливаем q_c
tasks[originalC].q = old_q;

}

int main() {
  ifstream infile("data2.txt");
  int rozmiar = 0;

  if (infile.good())
  {
    string sLine;
    getline(infile, sLine);
    rozmiar = stoi(sLine);
  }
  infile.close();

  Task tasks[rozmiar]; 
  readFile("data2.txt", rozmiar, tasks);

  Task tasksCopy[rozmiar];
  copy(tasks, tasks + rozmiar, tasksCopy);
  sortR(tasksCopy, rozmiar);
  // printAllTasks(tasks,rozmiar);

  // -------------- algorytm Schrage --------------
  SchrageResult result = schrage(tasksCopy, rozmiar);

  cout << "Cmax: " << result.Cmax << endl;

  cout << "Order: ";
  for (const auto& t : result.order) {
      cout << t.id << " ";
  }
  cout << endl;

  cout << "Completion times: ";
  for (int t : result.completionTimes) {
      cout << t << " ";
  }
  cout << endl;

  // -------------- algorytm Carlier --------------
  int UB = INT_MAX;          // początkowy Upper Bound
  carlier(tasksCopy, rozmiar, UB);
  cout << "Optymalny Cmax (Carlier): " << UB << endl; // 13862
  return 0;
}

 