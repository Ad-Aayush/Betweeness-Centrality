#include <bits/stdc++.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
using namespace std;

const int MAX_PHASE_SIZE = 1024;
const int MAX_SUCC_SIZE = 1024;
const int K = 8;  // Number of worker threads

struct Graph {
  int n;
  vector<vector<int>> adj;

  Graph(int vertices) : n(vertices), adj(vertices) {}

  void add_edge(int u, int v) {
    adj[u].push_back(v);
    adj[v].push_back(u);
  }
};

// Shared synchronization variables
mutex mtx;
condition_variable cv_start;
condition_variable cv_done;
bool ready = false;
bool terminate_flag = false;
int completed_threads = 0;

// Variables to hold current phase work
struct PhaseWork {
  Graph* G;
  vector<vector<int>>* S;
  vector<atomic<int>>* S_size;
  vector<atomic<int>>* d;
  vector<atomic<int>>* sigma;
  vector<vector<int>>* Succ;
  vector<atomic<int>>* Succ_size;
  int phase;
  int source;
} current_work;

// Worker thread function
void worker_thread(int thread_id) {
  while (true) {
    unique_lock<mutex> lock(mtx);
    cv_start.wait(lock, [] { return ready || terminate_flag; });
    lock.unlock();

    if (terminate_flag) {
      break;
    }

    // Capture the current work
    PhaseWork work = current_work;

    int total_nodes = work.S_size->at(work.phase).load();
    int chunk_size = (total_nodes + K - 1) / K;
    int start = thread_id * chunk_size;
    int end = min(start + chunk_size, total_nodes);

    for (int i = start; i < end; ++i) {
      int v = work.S->at(work.phase)[i];
      for (int w : work.G->adj[v]) {
        int dw = -1;
        if (work.d->at(w).compare_exchange_strong(dw,
                                                  work.d->at(v).load() + 1)) {
          int pos = work.S_size->at(work.phase + 1).fetch_add(1);
          if (pos < MAX_PHASE_SIZE) {
            work.S->at(work.phase + 1)[pos] = w;
          }
        }
        if (work.d->at(w).load() == work.d->at(v).load() + 1) {
          work.sigma->at(w).fetch_add(work.sigma->at(v).load());
          int pos = work.Succ_size->at(v).fetch_add(1);
          if (pos < MAX_SUCC_SIZE) {
            work.Succ->at(v)[pos] = w;
          }
        }
      }
    }

    lock.lock();
    completed_threads++;
    if (completed_threads == K) {
      ready = false;
      cv_done.notify_all();
    }
    lock.unlock();
  }
}

vector<double> DynamicMedium(Graph& G) {
  const int n = G.n;
  vector<double> BC(n, 0.0);

  // Initialize worker threads
  vector<thread> workers;
  for (int t = 0; t < K; ++t) {
    workers.emplace_back(worker_thread, t);
  }

  for (int s = 0; s < n; ++s) {
    vector<vector<int>> Succ(n, vector<int>(MAX_SUCC_SIZE, -1));
    vector<atomic<int>> Succ_size(n);
    vector<atomic<int>> sigma(n);
    vector<atomic<int>> d(n);
    vector<vector<int>> S(n + 1, vector<int>(MAX_PHASE_SIZE));
    vector<atomic<int>> S_size(n + 1);
    vector<double> delta(n, 0.0);

    for (int i = 0; i < n; ++i) {
      sigma[i].store(0);
      d[i].store(-1);
      Succ_size[i].store(0);
      delta[i] = 0.0;
      S_size[i].store(0);
    }
    sigma[s].store(1);
    d[s].store(0);
    S[0][0] = s;
    S_size[0].store(1);

    int phase = 0;

    // Forward BFS phase
    while (S_size[phase].load() > 0) {
      // Reset the size for the next phase
      S_size[phase + 1].store(0);

      // Set up the current work for this phase
      current_work =
          PhaseWork{&G, &S, &S_size, &d, &sigma, &Succ, &Succ_size, phase, s};

      // Lock and prepare to start the phase
      {
        unique_lock<mutex> lock(mtx);
        ready = true;
        completed_threads = 0;
      }

      // Notify all worker threads to start processing
      cv_start.notify_all();

      // Wait until all worker threads have completed processing
      {
        unique_lock<mutex> lock(mtx);
        cv_done.wait(lock, [] { return !ready; });
      }

      phase++;
    }

    for (int i = 0; i < n; ++i) {
      delta[i] = 0.0;
    }

    while (phase > 0) {
      phase--;
#pragma omp parallel for
      for (int i = 0; i < S_size[phase].load(); ++i) {
        int w = S[phase][i];
        double dsw = 0.0;
        double sw = sigma[w].load();

        for (int j = 0; j < Succ_size[w].load(); ++j) {
          int v = Succ[w][j];
          dsw += (sw / sigma[v].load()) * (1.0 + delta[v]);
        }
        delta[w] = dsw;

        if (w != s) {
          BC[w] += dsw;
        }
      }
    }
  }

  // Signal worker threads to terminate
  {
    unique_lock<mutex> lock(mtx);
    terminate_flag = true;
  }
  cv_start.notify_all();

  // Join all worker threads
  for (auto& th : workers) {
    if (th.joinable()) {
      th.join();
    }
  }

  return BC;
}

int main() {
  // Read graph from file
  ifstream file("graph.txt");
  if (!file.is_open()) {
    cerr << "Error opening graph.txt" << endl;
    return 1;
  }

  int n, m;
  file >> n >> m;

  Graph G(n);
  for (int i = 0; i < m; ++i) {
    int u, v;
    file >> u >> v;
    G.add_edge(u, v);
  }
  file.close();

  auto start = chrono::high_resolution_clock::now();
  vector<double> BC = DynamicMedium(G);
  auto end = chrono::high_resolution_clock::now();

  ofstream out("output.txt");
  if (!out.is_open()) {
    cerr << "Error opening output.txt" << endl;
    return 1;
  }

  out << "Betweenness Centrality Scores:\n";
  for (int v = 0; v < BC.size(); ++v) {
    out << "Vertex " << v << ": " << BC[v] << "\n";
  }
  out.close();

  cout << "Time: "
       << chrono::duration_cast<chrono::milliseconds>(end - start).count()
       << "ms\n";

  return 0;
}
