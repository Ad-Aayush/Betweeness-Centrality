#include <bits/stdc++.h>
using namespace std;

int main() {
  int n = 1024;
  int m = 8 * n;

  // Generate random graph
  srand(time(0));
  vector<pair<int, int>> edges;
  for (int i = 0; i < m; ++i) {
    int u = rand() % n;
    int v = rand() % n;
    edges.push_back({u, v});
  }

  // Write graph to file
  ofstream file("graph.txt");
  file << n << " " << m << endl;
  for (auto [u, v] : edges) {
    file << u << " " << v << endl;
  }
  file.close();
}