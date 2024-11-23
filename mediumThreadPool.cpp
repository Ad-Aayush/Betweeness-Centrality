#include <bits/stdc++.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

using namespace std;

const int MAX_PHASE_SIZE = 1024;
const int MAX_SUCC_SIZE = 1024;
const int K = 8;

struct Graph {
  int n;
  vector<vector<int>> adj;

  Graph(int vertices) : n(vertices), adj(vertices) {}

  void add_edge(int u, int v) {
    adj[u].push_back(v);
    adj[v].push_back(u);
  }
};

class ThreadPool {
 public:
  ThreadPool(size_t num_threads);
  ~ThreadPool();

  void enqueue(function<void()> task);

  void wait_for_tasks();

 private:
  vector<thread> workers;
  queue<function<void()>> tasks;

  mutex queue_mutex;
  condition_variable condition;
  bool stop;

  // For tracking unfinished tasks
  mutex tasks_left_mutex;
  condition_variable tasks_left_condition;
  int tasks_left;
};

ThreadPool::ThreadPool(size_t num_threads) : stop(false), tasks_left(0) {
  for (size_t i = 0; i < num_threads; ++i) {
    workers.emplace_back([this]() {
      while (true) {
        function<void()> task;

        // Fetch a task from the queue
        {
          unique_lock<mutex> lock(this->queue_mutex);
          this->condition.wait(
              lock, [this]() { return this->stop || !this->tasks.empty(); });
          if (this->stop && this->tasks.empty()) return;
          task = move(this->tasks.front());
          this->tasks.pop();
        }

        task();

        {
          unique_lock<mutex> lock(this->tasks_left_mutex);
          tasks_left--;
          if (tasks_left == 0) {
            tasks_left_condition.notify_all();
          }
        }
      }
    });
  }
}

void ThreadPool::enqueue(function<void()> task) {
  {
    unique_lock<mutex> lock(queue_mutex);
    tasks.emplace(task);
  }
  {
    unique_lock<mutex> lock(tasks_left_mutex);
    tasks_left++;
  }
  condition.notify_one();
}

void ThreadPool::wait_for_tasks() {
  unique_lock<mutex> lock(tasks_left_mutex);
  tasks_left_condition.wait(lock, [this]() { return tasks_left == 0; });
}

ThreadPool::~ThreadPool() {
  {
    unique_lock<mutex> lock(queue_mutex);
    stop = true;
  }
  condition.notify_all();
  for (thread& worker : workers) worker.join();
}

// ProcessOne Function: Processes a single node in the current phase
void ProcessOne(Graph& G, vector<vector<int>>& S, vector<atomic<int>>& S_size,
                vector<atomic<int>>& d, vector<atomic<int>>& sigma,
                vector<vector<int>>& Succ, vector<atomic<int>>& Succ_size,
                int phase, int ind) {
  int v = S[phase][ind];
  for (int w : G.adj[v]) {
    int dw = -1;
    if (d[w].compare_exchange_strong(dw, d[v].load() + 1)) {
      int pos = S_size[phase + 1].fetch_add(1);
      if (pos < MAX_PHASE_SIZE) {
        S[phase + 1][pos] = w;
      }
    }
    if (d[w].load() == d[v].load() + 1) {
      sigma[w].fetch_add(sigma[v].load());
      int pos = Succ_size[v].fetch_add(1);
      if (pos < MAX_SUCC_SIZE) {
        Succ[v][pos] = w;
      }
    }
  }
}

vector<double> DynamicMediumOptimized(Graph& G, ThreadPool& pool) {
  const int n = G.n;
  vector<double> BC(n, 0.0);

  vector<vector<double>> BC_accumulators(K, vector<double>(n, 0.0));

  for (int s = 0; s < n; ++s) {
    vector<vector<int>> Succ(n, vector<int>(MAX_SUCC_SIZE, -1));
    vector<atomic<int>> Succ_size(n);
    vector<atomic<int>> sigma(n);
    vector<atomic<int>> d(n);
    vector<vector<int>> S(n, vector<int>(MAX_PHASE_SIZE));
    vector<atomic<int>> S_size(n);
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

    while (S_size[phase].load() > 0) {
      S_size[phase + 1].store(0);
      int current_phase_size = S_size[phase].load();

      int tasks_per_phase = K;
      int chunk_size =
          (current_phase_size + tasks_per_phase - 1) / tasks_per_phase;

      for (int t = 0; t < tasks_per_phase; ++t) {
        int start = t * chunk_size;
        int end = min(start + chunk_size, current_phase_size);
        if (start >= end) continue;

        pool.enqueue([&, start, end]() {
          for (int i = start; i < end; ++i) {
            ProcessOne(G, S, S_size, d, sigma, Succ, Succ_size, phase, i);
          }
        });
      }

      pool.wait_for_tasks();

      phase++;
    }

    while (phase > 0) {
      phase--;

      int current_phase_size = S_size[phase].load();

      int tasks_per_phase = K;
      int chunk_size =
          (current_phase_size + tasks_per_phase - 1) / tasks_per_phase;

      // Enqueue tasks for the backward phase
      for (int t = 0; t < tasks_per_phase; ++t) {
        int start = t * chunk_size;
        int end = min(start + chunk_size, current_phase_size);
        if (start >= end) continue;

        pool.enqueue([&, start, end, s, t]() {
          vector<double>& local_BC = BC_accumulators[t];
          for (int i = start; i < end; ++i) {
            int w = S[phase][i];
            double dsw = 0.0;
            double sw = sigma[w].load();

            for (int j = 0; j < Succ_size[w].load(); ++j) {
              int v = Succ[w][j];
              if (sigma[v].load() == 0) continue;
              dsw += (sw / sigma[v].load()) * (1.0 + delta[v]);
            }
            delta[w] = dsw;

            if (w != s) {
              local_BC[w] += dsw;
            }
          }
        });
      }

      pool.wait_for_tasks();

      for (int t = 0; t < K; ++t) {
        for (int v = 0; v < n; ++v) {
          BC[v] += BC_accumulators[t][v];
          BC_accumulators[t][v] = 0.0;
        }
      }
    }
  }

  return BC;
}

int main() {
  ifstream file("graph.txt");
  if (!file.is_open()) {
    cerr << "Error opening graph.txt\n";
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

  ThreadPool pool(K);

  auto start = chrono::high_resolution_clock::now();

  vector<double> BC = DynamicMediumOptimized(G, pool);

  auto end = chrono::high_resolution_clock::now();

  ofstream out("output.txt");
  if (!out.is_open()) {
    cerr << "Error opening output.txt\n";
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
