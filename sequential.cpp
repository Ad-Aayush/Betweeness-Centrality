#include <bits/stdc++.h>
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

std::vector<double> SequentialBrandes(Graph &G) {
  int n = G.n;
  std::vector<double> BC(n, 0.0);

  for (int s = 0; s < n; ++s) {
    std::vector<std::vector<int>> P(n);
    std::vector<int> sigma(n, 0);
    std::vector<int> d(n, -1);

    sigma[s] = 1;
    d[s] = 0;

    int phase = 0;
    std::stack<int> S;
    S.push(s);

    std::queue<int> Q;
    Q.push(s);

    while (Q.size()) {
      int v = Q.front();
      Q.pop();
      S.push(v);

      for (int w : G.adj[v]) {
        if (d[w] < 0) {
          Q.push(w);
          d[w] = d[v] + 1;
        }
        if (d[w] == d[v] + 1) {
          sigma[w] += sigma[v];
          P[w].push_back(v);
        }
      }
    }

    std::vector<double> delta(n, 0.0);

    while (!S.empty()) {
      int w = S.top();
      S.pop();
      for (int v : P[w]) {
        double c = (static_cast<double>(sigma[v]) / sigma[w]) * (1 + delta[w]);
        delta[v] += c;
      }
      if (w != s) {
        BC[w] += delta[w];
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
  std::vector<double> BC = SequentialBrandes(G);
  auto end = chrono::high_resolution_clock::now();

  // Print the betweenness centrality scores
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