#include <bits/stdc++.h>
using namespace std;

int main(void) {
    typedef unsigned ui;
    const ui N[] = { 1 << 12, 1 << 15, 1 << 18, 1 << 21, 1 << 24, 1 << 27 };
    const ui D[] = { 4, 8, 16 };
    typedef pair<ui, ui> pii;
    vector<pii> es;
    es.reserve(1<<30);
    for (ui n : N) {
        for (ui d : D) {
            ui m = n * d / 2;
            mt19937_64 mt(2022);
            uniform_int_distribution<int> uid(0, n - 1);
            while (es.size() != m) {
                int c = es.size();
                for (int i = m - c; i >= 1; --i) {
                    int u, v; 
                    do u = uid(mt), v = uid(mt); while (u >= v);
                    es.emplace_back(u, v);
                }
                sort(es.begin() + c, es.end());
                inplace_merge(es.begin(), es.begin() + c, es.end());
                es.erase(unique(es.begin(), es.end()), es.end());
            }
            shuffle(es.begin(), es.end(), mt);
            ofstream ofs("syn-" + to_string(__builtin_ctz(m)));
            ofs << n << ' ' << m << endl;
            bernoulli_distribution bd;
            for (pii p : es) {
                if (bd(mt))
                    swap(p.first, p.second);
                ofs << p.first << ' ' << p.second << '\n';
            }
            es.clear();
        }
    }
    return 0;
}