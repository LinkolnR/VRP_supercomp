
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
#include <iomanip>
#include <mpi.h>
#include <unordered_map>

using namespace std;

class Grafo {
    unordered_map<int, vector<pair<int, int>>> grafo_classe;

public:
    void adicionarAresta(int origem, int destino, int peso) {
        grafo_classe[origem].push_back(make_pair(destino, peso));
    }

    int calcularCustoRota(const vector<int>& rota) {
        vector<int> rota_copia = rota;
        rota_copia.push_back(0);
        rota_copia.insert(rota_copia.begin(), 0);
        int custo = 0;
        for (int i = 0; i < rota_copia.size() - 1; i++) {
            int origem = rota_copia[i];
            int destino = rota_copia[i + 1];
            for (const auto& aresta : grafo_classe[origem]) {
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
            if (grafo_classe.find(origem) == grafo_classe.end()) {
                return false;
            }
            bool arestaEncontrada = false;
            for (const auto& aresta : grafo_classe[origem]) {
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

void LerGrafo(string file, map<int,int> &demanda, vector<tuple<int, int , int>> &arestas, vector<int> &locais, Grafo &grafo);
vector<vector<int>> GerarTodasAsCombinacoes(const vector<int> locais, map<int,int> demanda, int capacidade, Grafo &grafo);
bool VerificarCapacidade(vector<int> rota, map<int,int> demanda, int capacidade);
void encontrarMelhorCombinacao(const vector<vector<int>>& rotas, vector<vector<int>>& combinacaoAtual, int index, const vector<int>& locais, int& melhorCusto, vector<vector<int>>& melhorCombinacao, Grafo& grafo);
bool cobreTodasCidades(const vector<vector<int>>& combinacao, const vector<int>& locais);

int main(int argc, char* argv[]){
    // Agora utilizando MPI temos que fazer as devidas preparações para o seu uso
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    auto start = std::chrono::high_resolution_clock::now();

    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <file>" << endl;
        MPI_Finalize();
        return 1;
    }
    string file = argv[1];
    int capacidade = 10;
    Grafo grafo;    
    map<int,int> demanda;
    vector<tuple<int, int , int>> arestas;
    vector<int> locais;

    LerGrafo(file, demanda, arestas, locais, grafo);
    if (rank == 0) {
        cout << "Local: "  << locais.size() << endl;
    }
    vector<vector<int>> rotas = GerarTodasAsCombinacoes(locais, demanda, capacidade, grafo);
    if (rank == 0) {
        cout << "Rotas: " << rotas.size() << endl;
    }
    int melhorCustoGlobal = INT_MAX;
    vector<vector<int>> melhorCombinacaoGlobal;

    // como agora estamos utilizando MPI, precisamos dividir o trabalho entre os processos, lembrando que o rank 0 é o processo principal e size é o número total de processos
    int chunkSize = rotas.size() / size;
    int startIdx = rank * chunkSize;
    int endIdx;
    if (rank == size - 1) {
        endIdx = rotas.size();
    } else {
        endIdx = startIdx + chunkSize;
    }

    int melhorCustoLocal = INT_MAX;
    vector<vector<int>> melhorCombinacaoLocal;
    vector<vector<int>> combinacaoAtual;
    // então aqui estamos rodando a função de encontrar a melhor combinação para cada processo. Ou seja cada um dos proceoss está pegando um trecho do vetor de rotas
    encontrarMelhorCombinacao(rotas, combinacaoAtual, startIdx, locais, melhorCustoLocal, melhorCombinacaoLocal, grafo);

    if (rank != 0) {
        // se o processo for diferente do processo principal, então o processo deve enviar o seu melhor custo e a sua melhor combinação para o processo principal
        MPI_Send(&melhorCustoLocal, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        int localSize = melhorCombinacaoLocal.size();
        MPI_Send(&localSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        // e aqui estamos enviando a combinação de rotas

        for (const auto& rota : melhorCombinacaoLocal) {
            // para cada rota, enviamos o tamanho da rota e a rota em si
            int rotaSize = rota.size();
            MPI_Send(&rotaSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            MPI_Send(rota.data(), rotaSize, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    } else {
        // se o processo for o processo principal, então ele deve receber os resultados dos outros processos e comparar com o seu resultado
        melhorCustoGlobal = melhorCustoLocal;
        melhorCombinacaoGlobal = melhorCombinacaoLocal;
        for (int i = 1; i < size; i++) {
            // recebemos o melhor custo de cada processo
            int melhorCustoRecv;
            MPI_Recv(&melhorCustoRecv, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (melhorCustoRecv < melhorCustoGlobal) {
                melhorCustoGlobal = melhorCustoRecv;
                int localSize;
                // recebemos o tamanho da combinação de rotas
                MPI_Recv(&localSize, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                melhorCombinacaoGlobal.clear();
                for (int j = 0; j < localSize; j++) {
                    int rotaSize;
                    // recebemos o tamanho da rota
                    MPI_Recv(&rotaSize, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    // recebemos a rota
                    vector<int> rota(rotaSize);
                    MPI_Recv(rota.data(), rotaSize, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    melhorCombinacaoGlobal.push_back(rota);
                }
            }
        }
    }

    if (rank == 0) {
        // para finalizar utilizamos o processo principal para imprimir o resultado final e o tempo de execução
        cout << "Melhor combinação de rotas:" << endl;
        for (const auto& rota : melhorCombinacaoGlobal) {
            cout << "{ ";
            for (int cidade : rota) {
                cout << cidade << " ";
            }
            cout << "} com custo: " << grafo.calcularCustoRota(rota) << endl;
        }
        cout << "Custo total: " << melhorCustoGlobal << endl;

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Tempo de execução: " << duration.count() << " segundos" << std::endl;

        std::ofstream outputFile("tempo_execucao_" + file + ".txt");
        if (outputFile.is_open()) {
            outputFile << "Tempo de execução: " << std::fixed << setprecision(3) << duration.count() << " segundos" << std::endl;
            outputFile.close();
        } else {
            std::cout << "Erro ao abrir o arquivo de saída" << std::endl;
        }
    }

    MPI_Finalize();
    return 0;
}

void LerGrafo(string file, map<int,int> &demanda, vector<tuple<int, int , int>> &arestas, vector<int> &locais, Grafo &grafo) {
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

bool VerificarCapacidade(vector<int> rota, map<int,int> demanda, int capacidade){
    int demanda_total = 0;
    for (auto& local : rota){
        demanda_total += demanda[local];
    }
    return demanda_total <= capacidade;
}

vector<vector<int>> GerarTodasAsCombinacoes(const vector<int> locais, map<int,int> demanda, int capacidade,Grafo &grafo) {
    vector<vector<int>> rotas;
    int n = locais.size();
    for (int i = 1; i < (1 << n); i++) {
        vector<int> rota;
        for (int j = 0; j < n; j++) {
            if (i & (1 << j)) {
                rota.push_back(locais[j]);
            }
        }
        if (grafo.verificarRotaValida(rota)){
            if (VerificarCapacidade(rota, demanda, capacidade)){
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
                               const vector<int>& locais, int& melhorCusto, vector<vector<int>>& melhorCombinacao, Grafo& grafo) {
    if (cobreTodasCidades(combinacaoAtual, locais)) {
        int custoTotal = 0;
        for (const auto& rota : combinacaoAtual) {
            custoTotal += grafo.calcularCustoRota(rota);
        }
        if (custoTotal < melhorCusto) {
            melhorCusto = custoTotal;
            melhorCombinacao = combinacaoAtual;
        }
        return;
    }

    if (index >= rotas.size()) {
        return;
    }

    combinacaoAtual.push_back(rotas[index]);
    encontrarMelhorCombinacao(rotas, combinacaoAtual, index + 1, locais, melhorCusto, melhorCombinacao, grafo);

    combinacaoAtual.pop_back();
    encontrarMelhorCombinacao(rotas, combinacaoAtual, index + 1, locais, melhorCusto, melhorCombinacao, grafo);
}
