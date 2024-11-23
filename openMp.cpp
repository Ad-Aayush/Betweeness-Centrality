#include <bits/stdc++.h>
#include <omp.h>
using namespace std;

struct Graph {
  int n;
  std::vector<std::vector<int>> adj;

  Graph(int vertices) : n(vertices), adj(vertices, std::vector<int>()) {}

  void add_edge(int u, int v) {
    adj[u].push_back(v);
    adj[v].push_back(u);
  }
};

std::vector<double> OpenMpBrandes(Graph &G) {
  int n = G.n;
  std::vector<double> BC(n, 0.0);

  for (int s = 0; s < n; ++s) {
    std::vector<std::vector<int>> P(n);
    std::vector<int> sigma(n, 0);
    std::vector<int> d(n, -1);

    sigma[s] = 1;
    d[s] = 0;

    int phase = 0;
    std::array<std::vector<int>, 100> S;  // We need some high number of phases
    S[phase].push_back(s);

    int cnt = 1;

    while (cnt > 0) {
      cnt = 0;
#pragma omp parallel for
      for (auto v : S[phase]) {
#pragma omp for
        for (int w : G.adj[v]) {
          if (d[w] < 0) {
            S[phase + 1].push_back(w);
            cnt++;
            d[w] = d[v] + 1;
          }
          if (d[w] == d[v] + 1) {
            sigma[w] += sigma[v];
            P[w].push_back(v);
          }
        }
        phase = phase + 1;
      }
    }
    phase = phase - 1;

    std::vector<double> delta(n, 0.0);

    while (phase > 0) {
      #pragma omp parallel for
      for (int i = 0; i < S[phase].size(); ++i) {
        int w = S[phase][i];
        for (auto &v : P[w]) {
          double c = ((double)sigma[v] / sigma[w]) * (1.0 + delta[w]);
          #pragma omp atomic
          delta[v] += c;
        }
        if (w != s) {
          #pragma omp atomic
          BC[w] += delta[w];
        }
      }
      phase = phase - 1;
    }
  }
  return BC;
}

int main() {
  Graph G(6);
  G.add_edge(0, 1);
  G.add_edge(0, 2);
  G.add_edge(1, 3);
  G.add_edge(2, 3);
  G.add_edge(3, 4);
  G.add_edge(4, 5);

  std::vector<double> BC = OpenMpBrandes(G);

  // Print the betweenness centrality scores
  std::cout << "Betweenness Centrality Scores:\n";
  for (int v = 0; v < BC.size(); ++v) {
    std::cout << "Vertex " << v << ": " << BC[v] << "\n";
  }

  return 0;
}