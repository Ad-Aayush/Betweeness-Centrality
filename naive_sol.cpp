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

std::vector<double> NaiveBetweeness(Graph& G) {
  int n = G.n;
  std::vector<double> BC(n, 0.0);

  // Initialize distance and shortest paths matrices
  std::vector<std::vector<int>> dist(n, std::vector<int>(n, INT_MAX));
  std::vector<std::vector<int>> shortest_paths(n, std::vector<int>(n, 0));

  // Compute shortest paths and distances using BFS for each source vertex
  for (int src = 0; src < n; ++src) {
    std::queue<int> Q;
    dist[src][src] = 0;
    shortest_paths[src][src] = 1;
    Q.push(src);

    while (!Q.empty()) {
      int u = Q.front();
      Q.pop();
      for (int v : G.adj[u]) {
        if (dist[src][v] == INT_MAX) {  // New node discovered
          dist[src][v] = dist[src][u] + 1;
          Q.push(v);
        }
        if (dist[src][v] == dist[src][u] + 1) {  // Found shortest path
          shortest_paths[src][v] += shortest_paths[src][u];
        }
      }
    }
  }

  // Compute betweenness centrality
  for (int v = 0; v < n; ++v) {      // For each vertex
    for (int s = 0; s < n; ++s) {    // For each source
      if (s == v) continue;          // Skip if source is the same as v
      for (int t = 0; t < n; ++t) {  // For each target
        if (t == v || t == s)
          continue;  // Skip if target is same as v or source
        if (dist[s][v] + dist[v][t] == dist[s][t] && shortest_paths[s][t] > 0) {
          // Contribution to BC of v from s -> t
          BC[v] += (1.0 * shortest_paths[s][v] * shortest_paths[v][t]) /
                   shortest_paths[s][t];
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

  std::vector<double> BC = NaiveBetweeness(G);

  // Print the betweenness centrality scores
  ofstream out("output.txt");

  out << "Betweenness Centrality Scores:\n";
  for (int v = 0; v < BC.size(); ++v) {
    out << "Vertex " << v << ": " << BC[v] << "\n";
  }

  return 0;
}