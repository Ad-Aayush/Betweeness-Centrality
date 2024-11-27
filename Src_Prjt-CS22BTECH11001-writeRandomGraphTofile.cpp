#include <bits/stdc++.h>
using namespace std;

bool CheckConnected(int n, vector<pair<int, int>> &edges) {
  vector<vector<int>> adj(n);
  for (auto [u, v] : edges) {
    adj[u].push_back(v);
    adj[v].push_back(u);
  }

  vector<bool> visited(n, false);
  queue<int> q;
  q.push(0);
  visited[0] = true;

  while (!q.empty()) {
    int u = q.front();
    q.pop();

    for (int v : adj[u]) {
      if (!visited[v]) {
        visited[v] = true;
        q.push(v);
      }
    }
  }

  for (int i = 0; i < n; ++i) {
    if (!visited[i]) {
      return false;
    }
  }

  return true;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " <number of vertices>\n";
    return 1;
  }
  int n = atoi(argv[1]);
  int m = 8 * n;

  // Generate random graph
  srand(time(0));

  bool isConnected = false;
  vector<pair<int, int>> edges;
  while (!isConnected) {
    edges.clear();
    for (int i = 0; i < m; ++i) {
      int u = rand() % n;
      int v = rand() % n;
      edges.push_back({u, v});
    }

    isConnected = CheckConnected(n, edges);
  }

  // Write graph to file
  string filename = "Graphs/graph-" + to_string(n) + ".txt";
  ofstream file(filename);
  file << n << " " << m << endl;
  for (auto [u, v] : edges) {
    file << u << " " << v << endl;
  }
  file.close();
}