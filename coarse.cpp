#include <bits/stdc++.h>

#include <vector>
using namespace std;

struct Graph {
  int n;
  vector<vector<int>> adj;

  Graph(int vertices) : n(vertices), adj(vertices) {}

  void add_edge(int u, int v) {
    adj[u].push_back(v);
    adj[v].push_back(u);
  }
};

vector<double> SeqBrandes(Graph &G) {
  const int n = G.n;
  const int MAX_PHASE_SIZE = 1024;
  const int MAX_SUCC_SIZE = 1024;
  vector<double> BC(n, 0.0);
  
#pragma omp parallel for
  for (int s = 0; s < n; ++s) {
    vector<vector<int>> Succ(n, vector<int>(MAX_SUCC_SIZE, -1));
    vector<int> Succ_size(n);
    vector<int> sigma(n);
    vector<int> d(n);
    vector<vector<int>> S(n, vector<int>(MAX_PHASE_SIZE));
    vector<int> S_size(n + 1);
    vector<double> delta(n, 0.0);

    for (int i = 0; i < n; ++i) {
      sigma[i] = 0;
      d[i] = -1;
      Succ_size[i] = 0;
      delta[i] = 0.0;
      S_size[i] = 0;
    }
    sigma[s] = 1;
    d[s] = 0;
    S[0][0] = s;
    S_size[0] = 1;

    int phase = 0;

    while (S_size[phase] > 0) {
      S_size[phase + 1] = 0;

      for (int i = 0; i < S_size[phase]; ++i) {
        int v = S[phase][i];
        for (int w : G.adj[v]) {
          if (d[w] == -1) {
            int pos = S_size[phase + 1]++;
            S[phase + 1][pos] = w;
            d[w] = d[v] + 1;
          }
          if (d[w] == d[v] + 1) {
            sigma[w] += sigma[v];
            int pos = Succ_size[v]++;
            Succ[v][pos] = w;
          }
        }
      }
      phase++;
    }

    while (phase > 0) {
      phase--;
      for (int i = 0; i < S_size[phase]; ++i) {
        int w = S[phase][i];
        double dsw = 0.0;
        double sw = sigma[w];

        for (int j = 0; j < Succ_size[w]; ++j) {
          int v = Succ[w][j];
          dsw += (sw / sigma[v]) * (1.0 + delta[v]);
        }
        delta[w] = dsw;

        if (w != s) {
          BC[w] += dsw;
        }
      }
    }
  }

  return BC;
}

int main() {
  ifstream file("graph.txt");

  int n, m;
  file >> n >> m;

  Graph G(n);
  for (int i = 0; i < m; ++i) {
    int u, v;
    file >> u >> v;
    G.add_edge(u, v);
  }

  auto start = chrono::high_resolution_clock::now();
  vector<double> BC = SeqBrandes(G);
  auto end = chrono::high_resolution_clock::now();

  ofstream out("output.txt");

  out << "Betweenness Centrality Scores:\n";
  for (int v = 0; v < BC.size(); ++v) {
    out << "Vertex " << v << ": " << BC[v] << "\n";
  }
  cout << "Time: "
       << chrono::duration_cast<chrono::milliseconds>(end - start).count()
       << "ms\n";

  return 0;
}
