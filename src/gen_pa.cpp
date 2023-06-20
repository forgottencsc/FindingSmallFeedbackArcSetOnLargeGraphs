#include <bits/stdc++.h>
using namespace std;


struct segt {
    int n, s;
    vector<int> sum;

    segt(int n_) : n(n_), s(0), sum(2 << (int)ceil(log2(n_))) {};

    void add(int x, int v, int p, int l, int r) {
        if (l + 1 == r);
        else {
            sum[p]++;
            int m = (l + r) >> 1;
            if (x < m)
                add(x, v, p << 1, l, m);
            else 
                add(x, v, p << 1 | 1, m, r);
        }
    }

    void add(int x, int v) {
        add(x, v, 1, 0, n);
        s++;
    }

    int get(int k, int p, int l, int r) {
        if (l + 1 == r)
            return l;
        else {
            int m = (l + r) >> 1;
            if (k < sum[p << 1])
                return get(k, p << 1, l, m);
            else
                return get(k - sum[p << 1], p << 1 | 1, m, r);
        }
    }

    int get(int k) {
        return get(k, 1, 0, n);
    }

};

struct pa_sampler {
    int n;
    segt s, t;
    mt19937_64 mt;
    template<class T>
    T rnd(T l, T r) { return uniform_int_distribution<T>(l, r)(mt); }

    pa_sampler(int n_) : n(n_), s(n), mt(2022) {}

    int get_s() {
        return s.get(rnd(0, s.s - 1));
    }

    int get_t() {
        return t.get(rnd(0, t.s - 1));
    }

    void add(int u, int v) {
        s.add(u, 1);
        t.add(v, 1);
    }
};

typedef unsigned ui;
typedef unsigned long long ull;
typedef pair<ui, ui> pii;
template<>
struct std::hash<pii> {
    ull operator()(pii p) const { return (ull)p.first << 32 | p.second; }
};

int main(void) {
    const ui N[] = { 1 << 20, 1 << 23, 1 << 26 };
    const ui D[] = { 4, 8, 16 };
    for (ui n : N) {
        for (ui d : D) {
            unordered_set<pii> es;
            ui m = n * d / 2;
            pa_sampler s(2 * n);
            
            int vc = 2;
            s.add(0, 1);
            es.emplace(0, 1);
            while (vc < n) {
                if (s.rnd(0, n + m - 1) < n) {
                    int u = vc++, v = s.get_t();
                    es.emplace(u, v);
                    s.add(u, v);
                }
                else {
                    int u = s.get_s(), v = s.get_t();
                    es.emplace(u, v);
                    s.add(u, v);
                }
            }

            vector<pii> es2;
            es2.reserve(es.size());
            es2.assign(es.begin(), es.end());
            shuffle(es2.begin(), es2.end(), s.mt);
            vector<int> pv(n);
            iota(pv.begin(), pv.end(), 0);
            for (int i = 0; i < n; ++i)
                swap(pv[i], pv[s.rnd((ui)i, n - 1)]);
            ofstream ofs("syn2-" + to_string(__builtin_ctz(m)));
            ofs << n << ' ' << m << endl;
            for (pii p : es)
                ofs << pv[p.first] << ' ' << pv[p.second] << '\n';
            es.clear();
        }
    }
    return 0;
}