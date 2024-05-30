#include <iostream>
#include <vector>
#include <set>
#include <climits>

using namespace std;

// Definindo uma estrutura para armazenar as rotas e seus custos
struct Rota {
    vector<int> cidades;
    int custo;
};

// Função para verificar se uma combinação de rotas cobre todas as cidades
bool cobreTodasCidades(const vector<Rota>& combinacao, const set<int>& cidadesTotais) {
    set<int> cidadesCobertas;
    for (const auto& rota : combinacao) {
        for (int cidade : rota.cidades) {
            cidadesCobertas.insert(cidade);
        }
    }
    return cidadesCobertas == cidadesTotais;
}

// Função recursiva para gerar todas as combinações possíveis de rotas
void encontrarMelhorCombinacao(const vector<Rota>& rotas, vector<Rota>& combinacaoAtual, int index, 
                               const set<int>& cidadesTotais, int& melhorCusto, vector<Rota>& melhorCombinacao) {
    if (cobreTodasCidades(combinacaoAtual, cidadesTotais)) {
        int custoTotal = 0;
        for (const auto& rota : combinacaoAtual) {
            custoTotal += rota.custo;
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
    encontrarMelhorCombinacao(rotas, combinacaoAtual, index + 1, cidadesTotais, melhorCusto, melhorCombinacao);

    // Excluir a rota atual da combinação
    combinacaoAtual.pop_back();
    encontrarMelhorCombinacao(rotas, combinacaoAtual, index + 1, cidadesTotais, melhorCusto, melhorCombinacao);
}

int main() {
    // Definindo as rotas e os custos
    vector<Rota> rotas = {
        {{1}, 184},
        {{2}, 58},
        {{3}, 28},
        {{4}, 52},
        {{5}, 124},
        {{1, 5}, 235},
        {{6}, 4},
        {{7}, 120},
        {{4, 7}, 175},
        {{8}, 142},
        {{1, 8}, 180},
        {{9}, 78},
        {{5, 9}, 105},
        {{1, 5, 9}, 216},
        {{7, 9}, 121},
    };

    // Encontrar todas as cidades únicas
    set<int> cidadesTotais;
    for (const auto& rota : rotas) {
        for (int cidade : rota.cidades) {
            cidadesTotais.insert(cidade);
        }
    }

    // Variáveis para armazenar o melhor resultado
    int melhorCusto = INT_MAX;
    vector<Rota> melhorCombinacao;

    // Vetor temporário para armazenar a combinação atual
    vector<Rota> combinacaoAtual;

    // Encontrar a melhor combinação de rotas
    encontrarMelhorCombinacao(rotas, combinacaoAtual, 0, cidadesTotais, melhorCusto, melhorCombinacao);

    // Imprimir o resultado
    cout << "Melhor combinação de rotas:" << endl;
    for (const auto& rota : melhorCombinacao) {
        cout << "{ ";
        for (int cidade : rota.cidades) {
            cout << cidade << " ";
        }
        cout << "} com custo: " << rota.custo << endl;
    }
    cout << "Menor custo: " << melhorCusto << endl;

    return 0;
}
