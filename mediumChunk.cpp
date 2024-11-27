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

int MAX_PHASE_SIZE = 1024;
int MAX_SUCC_SIZE = 128;
int K = 8;

struct Graph {
  int n;
  vector<vector<int>> adj;

  Graph(int vertices) : n(vertices), adj(vertices) {}

  void add_edge(int u, int v) {
    adj[u].push_back(v);
    adj[v].push_back(u);
  }
};

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

vector<double> DynamicMediumOptimized(Graph& G) {
  const int n = G.n;
  vector<double> BC(n, 0.0);


  for (int s = 0; s < n; ++s) {
    if (s % 100 == 0) {
      cout << "Processing source " << s << "\n";
    }
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
      vector<thread> threads;

      int tasks_per_phase = K;
      int chunk_size =
          (current_phase_size + tasks_per_phase - 1) / tasks_per_phase;

      for (int t = 0; t < tasks_per_phase; ++t) {
        int start = t * chunk_size;
        int end = min(start + chunk_size, current_phase_size);
        if (start >= end) continue;

        threads.emplace_back([&, start, end]() {
          for (int i = start; i < end; ++i) {
            ProcessOne(G, S, S_size, d, sigma, Succ, Succ_size, phase, i);
          }
        });
      }

      for (auto& thread : threads) {
        thread.join();
      }

      phase++;
    }

    while (phase > 0) {
      phase--;

      int current_phase_size = S_size[phase].load();

      int tasks_per_phase = K;
      int chunk_size =
          (current_phase_size + tasks_per_phase - 1) / tasks_per_phase;

      vector<thread> threads;

      for (int t = 0; t < tasks_per_phase; ++t) {
        int start = t * chunk_size;
        int end = min(start + chunk_size, current_phase_size);
        if (start >= end) continue;

        threads.emplace_back([&, start, end, s, t]() {
          for (int i = start; i < end; ++i) {
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
        });
      }

      for (auto& thread : threads) {
        thread.join();
      }
    }
  }

  return BC;
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    cerr << "Usage: " << argv[0] << " <graph file> <number of threads>\n";
    return 1;
  }
  ifstream file(argv[1]);
  K = atoi(argv[2]);
  if (!file.is_open()) {
    cerr << "Error opening graph.txt\n";
    return 1;
  }

  int n, m;
  file >> n >> m;
  MAX_PHASE_SIZE = max(n, MAX_PHASE_SIZE);

  Graph G(n);
  for (int i = 0; i < m; ++i) {
    int u, v;
    file >> u >> v;
    G.add_edge(u, v);
  }
  file.close();

  auto start = chrono::high_resolution_clock::now();

  vector<double> BC = DynamicMediumOptimized(G);

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
