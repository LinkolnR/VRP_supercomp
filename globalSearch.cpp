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

// utilizando o namespace std apenas para facilitar a leitura do código
using namespace std;
// classe grafo criada para facilitar as operações
class Grafo {
    // grafo_classe é um mapa que mapeia um vértice para uma lista de pares (vértice, peso)
    unordered_map<int, vector<pair<int, int>>> grafo_classe;

public:
    // Função para adicionar uma aresta ao grafo
    void adicionarAresta(int origem, int destino, int peso) {
        grafo_classe[origem].push_back(make_pair(destino, peso));
    }

    // função para calcular custo de uma rota
    int calcularCustoRota(const vector<int>& rota) {
        // copia da rota para adicionar o vértice 0 no início e no final
        vector<int> rota_copia = rota;
        rota_copia.push_back(0);
        rota_copia.insert(rota_copia.begin(), 0);
        int custo = 0;
        // percorre a rota e soma o custo de cada aresta
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

    // Função para verificar se uma rota é válida
    bool verificarRotaValida(const vector<int>& rota) {
        for (int i = 0; i < rota.size() - 1; i++) {
            int origem = rota[i];
            int destino = rota[i + 1];

            // Verifica se há uma aresta entre os vértices da rota
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

// declarando os construtores das funções
void LerGrafo(string file, map<int,int> &demanda, vector<tuple<int, int , int>> &arestas, vector<int> &locais, Grafo &grafo);
vector<vector<int>> GerarTodasAsCombinacoes(const vector<int> locais, map<int,int> demanda, int capacidade, Grafo &grafo);
bool VerificarCapacidade(vector<int> rota, map<int,int> demanda, int capacidade);
void encontrarMelhorCombinacao(const vector<vector<int>>& rotas, vector<vector<int>>& combinacaoAtual, int index, const vector<int>& locais, int& melhorCusto, vector<vector<int>>& melhorCombinacao, Grafo& grafo);


int main(int argc, char* argv[]){
    /*
    Recebe o nome do arquivo como argumento e chama a função LerGrafo para ler o arquivo e armazenar as informações 
    */
    // Inicia a contagem do tempo de execução utilizando a biblioteca chrono
    auto start = std::chrono::high_resolution_clock::now();
    // verifica se o nome do arquivo foi passado como argumento
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <file>" << endl;
        return 1;
    }
    string file = argv[1];
    // capacidade do veículo
    int capacidade = 10;
    // declaração das variáveis
    Grafo grafo;    
    map<int,int> demanda;
    vector<tuple<int, int , int>> arestas;
    vector<int> locais;
    // Realiza a leitura do grafo e armazena as informações nas variáveis declaras
    LerGrafo(file, demanda, arestas, locais, grafo);
    // Imprime o número de locais e o número de rotas possíveis
    cout << "Local: "  << locais.size() << endl;
    vector<vector<int>> rotas = GerarTodasAsCombinacoes(locais, demanda, capacidade, grafo);
    cout << "Rotas: " << rotas.size() << endl;
    // Inicializa o melhor custo com o maior valor possível
    int melhorCusto = INT_MAX;
    // Inicializa a melhor combinação de rotas
    vector<vector<int>> combinacaoAtual;
    vector<vector<int>> melhorCombinacao;
    // Encontra a melhor combinação de rotas
    encontrarMelhorCombinacao(rotas, combinacaoAtual, 0, locais, melhorCusto, melhorCombinacao, grafo);
    // Imprimir o resultado
    cout << "Melhor combinação de rotas:" << endl;
    for (const auto& rota : melhorCombinacao) {
        cout << "{ ";
        for (int cidade : rota) {
            cout << cidade << " ";
        }
        cout << "} com custo: " << grafo.calcularCustoRota(rota) << endl;
    }
    cout << "Custo total: " << melhorCusto << endl;


    // Calcula o tempo de execução
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Tempo de execução: " << duration.count() << " segundos" << std::endl;
    // Escrever o tempo de execução em um arquivo
    std::ofstream outputFile("tempo_execucao_" + file + ".txt");
    if (outputFile.is_open()) {
        outputFile << "Tempo de execução: " << std::fixed << setprecision(3) << duration.count() << " segundos" << std::endl;
        outputFile.close();
    } else {
        std::cout << "Erro ao abrir o arquivo de saída" << std::endl;
    }
    return 0;
}


// Função para ler o grafo a partir de um arquivo
void LerGrafo(string file, map<int,int> &demanda, vector<tuple<int, int , int>> &arestas, vector<int> &locais, Grafo &grafo) {
    ifstream arquivo;
    arquivo.open(file);
    if (arquivo.is_open()) {
        int N; // número de locais a serem visitados
        arquivo >> N;
        N -= 1;
        for (int i = 1; i <= N; i++) {
            locais.push_back(i);
        }
        // Populando a lista de demandas dos locais
        for (int i = 0; i < N; i++) {
            int id_no, demanda_no;
            arquivo >> id_no;
            arquivo >> demanda_no;
            demanda[id_no] = demanda_no;
        }
        // número de arestas
        int K; // Declare a variável K antes de utilizá-la

        arquivo >> K; // Leia o número de arestas do arquivo

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

// Função para verificar se uma rota respeita a capacidade do veículo
bool VerificarCapacidade(vector<int> rota, map<int,int> demanda, int capacidade){
    int demanda_total = 0;
    for (auto& local : rota){
        demanda_total += demanda[local];
    }
    return demanda_total <= capacidade;
}

// Função para gerar todas as combinações possíveis
vector<vector<int>> GerarTodasAsCombinacoes(const vector<int> locais, map<int,int> demanda, int capacidade,Grafo &grafo) {
    vector<vector<int>> rotas;
    int n = locais.size();
    // Gera todas as combinações possíveis de rotas
    for (int i = 1; i < (1 << n); i++) {
        vector<int> rota;
        for (int j = 0; j < n; j++) {
            if (i & (1 << j)) {
                rota.push_back(locais[j]);
            }
        }
        // Para cada rota fazemos a verificação se é uma rota válida
        if (grafo.verificarRotaValida(rota)){
            // caso a rota seja válida, verificamos se a rota respeita a capacidade do veículo
            if (VerificarCapacidade(rota, demanda, capacidade)){
                rotas.push_back(rota);
            }
        }
    }
    return rotas;
}


// Função para verificar se uma combinação de rotas cobre todas as cidades
bool cobreTodasCidades(const vector<vector<int>>& combinacao, const vector<int>& locais) {
    set<int> cidadesCobertas;
    for (const auto& rota : combinacao) {
        for (int cidade : rota) {
            cidadesCobertas.insert(cidade);
        }
    }
    return cidadesCobertas.size() == locais.size();
}

// Função para encontrar a melhor combinação de rotas, essa é a função principal do algoritmo, que devolve a solução do problema
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

    // Incluir a rota atual na combinação
    combinacaoAtual.push_back(rotas[index]);
    encontrarMelhorCombinacao(rotas, combinacaoAtual, index + 1, locais, melhorCusto, melhorCombinacao, grafo);

    // Excluir a rota atual da combinação
    combinacaoAtual.pop_back();
    encontrarMelhorCombinacao(rotas, combinacaoAtual, index + 1, locais, melhorCusto, melhorCombinacao, grafo);
}
