#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <map>
#include <algorithm>


using namespace std;

void LerGrafo(string grafo, map<int,int> &demanda, vector<tuple<int, int , int>> &arestas, vector<int> &locais);
vector<vector<int>> GerarTodasAsCombinacoes(const vector<int> locais);
void ResolverVRPComDemanda(vector<int> locais, map<int,int> demanda, int capacidade);
bool VerificarCapacidade(vector<int> rota, map<int,int> demanda, int capacidade);
int CalcularCusto(vector<int> rota);

int main(){
    string grafo = "grafo.txt";
    map<int,int> demanda;
    vector<tuple<int, int , int>> arestas;
    // Realiza a leitura do grafo
    vector<int> locais;
    LerGrafo(grafo, demanda, arestas,locais);

    // for (auto it = demanda.begin(); it != demanda.end(); it++){
    //     cout << "Local: " << it->first << ", Demanda: " << it->second << endl;
    // }

    // for (auto& aresta : arestas){
    //     cout << "Nó 1: " << get<0>(aresta) << ", Nó 2: " << get<1>(aresta) << ", Custo: " << get<2>(aresta) << endl;
    // }
    cout << "Local: "  << locais.size() << endl;
    vector<vector<int>> rotas = GerarTodasAsCombinacoes(locais);
    cout << "Rotas: " << rotas.size() << endl;
    // for (auto& rota : rotas){
    //     for (auto& local : rota){
    //         cout << local << " ";
    //     }
    //     cout << endl;
    // }
    return 0;

}











void LerGrafo(string grafo, map<int,int> &demanda, vector<tuple<int, int , int>> &arestas, vector<int> &locais){
    ifstream arquivo;
    arquivo.open(grafo);
    if(arquivo.is_open()){
        int N; // número de locais a serem visitados
        arquivo >> N;
        N -= 1;
        for (int i = 1; i < N; i++){
            locais.push_back(i);
        }
        // Populando a lista de demandas dos locais
        for (int i = 0; i < N; i++){
            int id_no, demanda_no;
            arquivo >> id_no;
            arquivo >> demanda_no;
            demanda[id_no] = demanda_no;
        }
        // número de aresyas
        int K; // Declare a variável K antes de utilizá-la

        arquivo >> K; // Leia o número de arestas do arquivo

        for (int i = 0; i < K; i++){
            int id_no1, id_no2, custo;
            arquivo >> id_no1;
            arquivo >> id_no2;
            arquivo >> custo;
            arestas.push_back(make_tuple(id_no1, id_no2, custo));
        }
    }
    arquivo.close();
}

// Função GerarTodasAsCombinações(Locais):
//     // Gera todas as permutações possíveis de locais e agrupa em rotas válidas conforme a capacidade
//     // Esta função é bastante complexa,
//     // pois precisa considerar todas as subdivisões possíveis dos locais em rotas que atendam à capacidade do veículo
//     // Retorna uma lista de combinações válidas
//     Retornar combinações




// Função para gerar todas as combinações possíveis
vector<vector<int>> GerarTodasAsCombinacoes(const vector<int> locais) {
    vector<vector<int>> rotas;
    int n = locais.size();
    for (int i = 1; i < (1 << n); i++) {
        vector<int> rota;
        for (int j = 0; j < n; j++) {
            if (i & (1 << j)) {
                rota.push_back(locais[j]);
            }
        }
        rotas.push_back(rota);
    }
    return rotas;
}

