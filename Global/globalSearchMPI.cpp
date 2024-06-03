#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <map>
#include <algorithm>
#include <climits>
#include <set>
#include <chrono>
#include <stack>
#include <mpi.h>
#include <unordered_map>

using namespace std;

class Grafo {
    unordered_map<int, vector<pair<int, int>>> adjacencias;

public:
    void adicionarAresta(int origem, int destino, int peso) {
        adjacencias[origem].push_back(make_pair(destino, peso));
    }

    int calcularCustoRota(const vector<int>& rota) {
        vector<int> rota_copia = rota;
        rota_copia.push_back(0);
        rota_copia.insert(rota_copia.begin(), 0);
        int custo = 0;
        for (int i = 0; i < rota_copia.size() - 1; i++) {
            int origem = rota_copia[i];
            int destino = rota_copia[i + 1];

            for (const auto& aresta : adjacencias[origem]) {
                if (aresta.first == destino) {
                    custo += aresta.second;
                    break;
                }
            }
        }
        return custo;
    }

    bool verificarRotaValida(const vector<int>& rota) {
        for (int i = 0; i < rota.size() - 1; i++) {
            int origem = rota[i];
            int destino = rota[i + 1];

            if (adjacencias.find(origem) == adjacencias.end()) {
                return false;
            }

            bool arestaEncontrada = false;
            for (const auto& aresta : adjacencias[origem]) {
                if (aresta.first == destino) {
                    arestaEncontrada = true;
                    break;
                }
            }
            if (!arestaEncontrada) {
                return false;
            }
        }
        return true;
    }
};

void LerGrafo(string file, map<int, int>& demanda, vector<tuple<int, int, int>>& arestas, vector<int>& locais, Grafo& grafo);
vector<vector<int>> GerarTodasAsCombinacoes(const vector<int> locais, map<int, int> demanda, int capacidade, Grafo& grafo);
bool VerificarCapacidade(vector<int> rota, map<int, int> demanda, int capacidade);
void ResolverVRPComDemanda(vector<int> locais, map<int, int> demanda, int capacidade);
int CalcularCusto(vector<int> rota);
void encontrarMelhorCombinacao(const vector<vector<int>>& rotas, vector<vector<int>>& combinacaoAtual, int index, const vector<int>& locais, int& melhorCusto, vector<vector<int>>& melhorCombinacao, Grafo& grafo, int rank, int size);

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    auto start = std::chrono::high_resolution_clock::now();
    if (argc < 2) {
        if (rank == 0) {
            cout << "Usage: " << argv[0] << " <file>" << endl;
        }
        MPI_Finalize();
        return 1;
    }
    string file = argv[1];
    int capacidade = 10;
    Grafo grafo;
    map<int, int> demanda;
    vector<tuple<int, int, int>> arestas;
    vector<int> locais;

    LerGrafo(file, demanda, arestas, locais, grafo);

    if (rank == 0) {
        cout << "Local: " << locais.size() << endl;
    }

    vector<vector<int>> rotas = GerarTodasAsCombinacoes(locais, demanda, capacidade, grafo);

    if (rank == 0) {
        cout << "Rotas: " << rotas.size() << endl;
    }

    int melhorCustoGlobal = INT_MAX;
    vector<vector<int>> melhorCombinacaoGlobal;

    vector<vector<int>> combinacaoAtual;
    vector<vector<int>> melhorCombinacaoLocal;
    int melhorCustoLocal = INT_MAX;

    encontrarMelhorCombinacao(rotas, combinacaoAtual, 0, locais, melhorCustoLocal, melhorCombinacaoLocal, grafo, rank, size);

    MPI_Reduce(&melhorCustoLocal, &melhorCustoGlobal, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "Melhor combinação de rotas:" << endl;
        for (const auto& rota : melhorCombinacaoGlobal) {
            cout << "{ ";
            for (int cidade : rota) {
                cout << cidade << " ";
            }
            cout << "} com custo: " << grafo.calcularCustoRota(rota) << endl;
        }
        cout << "Menor custo: " << melhorCustoGlobal << endl;

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Tempo de execução: " << duration.count() << " segundos" << std::endl;
    }

    MPI_Finalize();
    return 0;
}

void LerGrafo(string file, map<int, int>& demanda, vector<tuple<int, int, int>>& arestas, vector<int>& locais, Grafo& grafo) {
    ifstream arquivo;
    arquivo.open(file);
    if (arquivo.is_open()) {
        int N;
        arquivo >> N;
        N -= 1;
        for (int i = 1; i <= N; i++) {
            locais.push_back(i);
        }
        for (int i = 0; i < N; i++) {
            int id_no, demanda_no;
            arquivo >> id_no;
            arquivo >> demanda_no;
            demanda[id_no] = demanda_no;
        }
        int K;
        arquivo >> K;
        for (int i = 0; i < K; i++) {
            int id_no1, id_no2, custo;
            arquivo >> id_no1;
            arquivo >> id_no2;
            arquivo >> custo;
            arestas.push_back(make_tuple(id_no1, id_no2, custo));
            grafo.adicionarAresta(id_no1, id_no2, custo);
        }
    }
    arquivo.close();
}

bool VerificarCapacidade(vector<int> rota, map<int, int> demanda, int capacidade) {
    int demanda_total = 0;
    for (auto& local : rota) {
        demanda_total += demanda[local];
    }
    return demanda_total <= capacidade;
}

vector<vector<int>> GerarTodasAsCombinacoes(const vector<int> locais, map<int, int> demanda, int capacidade, Grafo& grafo) {
    vector<vector<int>> rotas;
    int n = locais.size();
    for (int i = 1; i < (1 << n); i++) {
        vector<int> rota;
        for (int j = 0; j < n; j++) {
            if (i & (1 << j)) {
                rota.push_back(locais[j]);
            }
        }
        if (VerificarCapacidade(rota, demanda, capacidade)) {
            if (grafo.verificarRotaValida(rota)) {
                rotas.push_back(rota);
            }
        }
    }
    return rotas;
}

bool cobreTodasCidades(const vector<vector<int>>& combinacao, const vector<int>& locais) {
    set<int> cidadesCobertas;
    for (const auto& rota : combinacao) {
        for (int cidade : rota) {
            cidadesCobertas.insert(cidade);
        }
    }
    return cidadesCobertas.size() == locais.size();
}

void encontrarMelhorCombinacao(const vector<vector<int>>& rotas, vector<vector<int>>& combinacaoAtual, int index, 
                               const vector<int>& locais, int& melhorCusto, vector<vector<int>>& melhorCombinacao, 
                               Grafo& grafo, int rank, int size) {
    int n = rotas.size();
    int chunkSize = n / size;
    int start = rank * chunkSize;
    int end = (rank == size - 1) ? n : start + chunkSize;

    stack<pair<int, int>> pilha;
    pilha.push(make_pair(index, 0));

    while (!pilha.empty()) {
        pair<int, int> topo = pilha.top();
        pilha.pop();

        int i = topo.first;
        int opcao = topo.second;

        if (opcao == 0) {
            if (cobreTodasCidades(combinacaoAtual, locais)) {
                int custoTotal = 0;
                for (const auto& rota : combinacaoAtual) {
                    custoTotal += grafo.calcularCustoRota(rota);
                }
                if (custoTotal < melhorCusto) {
                    melhorCusto = custoTotal;
                    melhorCombinacao = combinacaoAtual;
                }
                continue;
            }
        }

        if (opcao == 0 && i < end) {
            combinacaoAtual.push_back(rotas[i]);
            pilha.push(make_pair(i, 1));
            pilha.push(make_pair(i + 1, 0));
        } else if (opcao == 1) {
            combinacaoAtual.pop_back();
            pilha.push(make_pair(i + 1, 0));
        }
    }
}
