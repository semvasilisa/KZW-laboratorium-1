#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <climits>
#include <chrono>

using namespace std;

int calls = 0;
const int MAX_DEPTH = 500;
const int TIME_LIMIT_SEC = 5;
chrono::steady_clock::time_point startTime;

struct Task {
    int id;
    int r, p, q;
};

struct SchrageResult {
    int Cmax;
    vector<Task> order;
    vector<int> completionTimes;
};


void readFile(string fileName, int size, Task* tasks){
    ifstream f(fileName);
    string line;
    getline(f, line);

    for (int i = 0; i < size; i++) {
        f >> tasks[i].r >> tasks[i].p >> tasks[i].q;
        tasks[i].id = i + 1;
    }
    f.close();
}


bool compareR(Task a, Task b){ return a.r < b.r; }
bool compareQ(Task a, Task b){ return a.q < b.q; }

// algorytm Shrage z interruptions, szukamy LB
int schragePmtn(Task* tasks, int n) {
    int t = 0, Cmax = 0; // t - aktualny czas 
    vector<Task> ready;
    vector<Task> N(tasks, tasks + n); //copy

    sort(N.begin(), N.end(), compareR);

    Task currentTask = {-1, 0, 0, INT_MAX};
    int i = 0;

    while(i < n || !ready.empty()) {

        while(i < n && N[i].r <= t) {
            ready.push_back(N[i]);

            // jeśli nowe zadanie ma większy priorytet - interruption  
            if(currentTask.id != -1 && N[i].q > currentTask.q) {
                int executed = t - N[i].r;
                currentTask.p -= executed;
                t = N[i].r;

                if(currentTask.p > 0)
                    ready.push_back(currentTask);
            }
            i++;
        }

        if(ready.empty()) {
            t = N[i].r;
        } else {
            auto it = max_element(ready.begin(), ready.end(),
                                  [](Task a, Task b){ return a.q < b.q; });

            currentTask = *it;
            ready.erase(it);

            t += currentTask.p;
            Cmax = max(Cmax, t + currentTask.q);
        }
    }

    return Cmax;
}

// zwykły algorytm Shrage
SchrageResult schrage(Task* tasks, int n){
    int t = 0, Cmax = 0, i = 0;
    vector<Task> ready, order;
    vector<int> completion;

    while(i < n || !ready.empty()) {

        while(i < n && tasks[i].r <= t) {
            ready.push_back(tasks[i]);
            i++;
        }

        if(ready.empty()) {
            t = tasks[i].r;
        } else {
            auto it = max_element(ready.begin(), ready.end(),
                                  [](Task a, Task b){ return a.q < b.q; });

            Task e = *it;
            ready.erase(it);

            t = max(t, e.r) + e.p;
            Cmax = max(Cmax, t + e.q);

            order.push_back(e);
            completion.push_back(t);
        }
    }

    return {Cmax, order, completion};
}

// metoda do poszukiwania zadania według id
int findTaskIndex(Task* tasks, int n, int id){
    for(int i = 0; i < n; i++){
        if(tasks[i].id == id) return i;
    }
    return -1;
}

// sprawdzamy czy minęło więcej niż 5 sekund, sprawdzamy aby algorytm nie działał w nieskończoność
bool timeExceeded() {
    auto now = chrono::steady_clock::now();
    return chrono::duration_cast<chrono::seconds>(now - startTime).count() > TIME_LIMIT_SEC;
}


void carlier(Task* tasks, int n, int& UB, int depth = 0){

    calls++; //ile razy wykonała się metoda(ile scenariusze zostało sprawdzone)

    if(calls % 10000 == 0)
        cout << "Calls: " << calls << endl;

    if(depth > MAX_DEPTH) return; // takie ograniczenia pozwolają zapobiegać bardzo głębokiej rekurencji

    // kopiujemy i sortujemy
    vector<Task> tasksCopy(tasks, tasks + n);
    sort(tasksCopy.begin(), tasksCopy.end(), compareR);

    // UB - wynik zwykłego Shrage 
    SchrageResult result = schrage(tasksCopy.data(), n);

    if(result.Cmax < UB) UB = result.Cmax;

    // zadanie B - zadanie zamykające blok krytyczny
    int B = -1;
    for(int i = 0; i < result.order.size(); i++){
        if(result.completionTimes[i] + result.order[i].q == result.Cmax){
            B = i;
            break;
        }
    }

    // zadanie A - początek bloku krytycznego
    int A = B;
    while(A > 0 &&
          result.completionTimes[A] - result.order[A].p ==
          result.completionTimes[A-1]){
        A--;
    }

    // zadanie C - ma q[C] < q[B]
    int C = -1;
    for(int i = B-1; i >= A; i--){
        if(result.order[i].q < result.order[B].q){
            C = i;
            break;
        }
    }

    if(C == -1) return;

    // obliczenie K 
    int rK = INT_MAX, pK = 0, qK = INT_MAX;

    for(int i = C+1; i <= B; i++){
        rK = min(rK, result.order[i].r);
        pK += result.order[i].p;
        qK = min(qK, result.order[i].q);
    }

    int taskC_id = result.order[C].id;
    int idx = findTaskIndex(tasks, n, taskC_id);

    int r_old = tasks[idx].r;
    int q_old = tasks[idx].q;

    // gałąż 1 - zwiększamy r[C]
    tasks[idx].r = max(tasks[idx].r, rK + pK);
    int LB = schragePmtn(tasks, n);
    if(LB < UB) carlier(tasks, n, UB, depth + 1);
    tasks[idx].r = r_old;

    // gałąż 2 - zwiększamy q[C]
    tasks[idx].q = max(tasks[idx].q, qK + pK);
    LB = schragePmtn(tasks, n);
    if(LB < UB) carlier(tasks, n, UB, depth + 1);
    tasks[idx].q = q_old;
}


int main() {
    ifstream infile("data4.txt");
    int n;
    infile >> n;
    infile.close();

    Task* tasks = new Task[n];
    readFile("data4.txt", n, tasks);

    vector<Task> tmp(tasks, tasks + n);
    sort(tmp.begin(), tmp.end(), compareR);
    int UB = schrage(tmp.data(), n).Cmax;

    startTime = chrono::steady_clock::now();

    carlier(tasks, n, UB);

    cout << "4 zestaw danych " << UB << endl;
    cout << "Optimal Cmax: " << UB << endl;
    cout << "Total calls: " << calls << endl;

    delete[] tasks;
    return 0;
}