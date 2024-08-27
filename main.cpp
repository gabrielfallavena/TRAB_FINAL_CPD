#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <utility>
#include <algorithm>
#include <sys/time.h>
#include <iomanip>
#include <unordered_map>
#include "csv.h"

const int R = 256; //Constante ASCII para os filhos de cada nodo da TRIE

using namespace std;

//STRUCT DO PLAYER
struct Player{
    int Id;
    string shortName;
    string longName;
    string Positions;
    string Nation;
    string Team;
    string League;
    double sumRating = 0;
    int countRatings = 0;
    double media;
    float rating;

    double getMedia(){
        return (countRatings != 0) ? sumRating/countRatings : 0; //Função para obter a média de cada jogador
    }

    //Construtor
    Player(int id,
           const string& shortname,
           const string& longname,
           const string& positions,
           const string& nation,
           const string& team,
           const string& league)
    : Id(id), shortName(shortname),longName(longname), Positions(positions), Nation(nation), Team(team), League(league) {}
};

//STRUCT DO USER
struct User{
    int Id;
    vector<pair<int, float>> ratings;
    //Construtor
    User(int id, vector<pair<int,float>> reviews) : Id(id), ratings(reviews) {}
};

//STRUCT DO NODO DAS TRIES
struct TrieNode{
    bool isEndOfWord; //Marca se o nodo é fim de uma palavra
    vector<TrieNode*> children; //Vetor de ponteiros para outros nodos filhos
    int ID; //Armazena o ID do jogador, caso isEndOfWord == true(usado na trie dos nomes)
    vector<int> tagsID; //Guarda o ID do jogador que possui a tag, caso isEndOfWord == true(utilizado na trie das tags)
    TrieNode() : isEndOfWord(false), children(R, nullptr) {} //children começa com tamanho 256, com todos indices nulos
};

// Funcao de particionamento do vetor.
int partition_float(vector<pair<int,float>>&arr, int low, int high) {
    float pivot = arr[high].second;  // Escolhe o elemento mais a direita do vetor
    int i = (low - 1);  // Indice do menor elemento

    for (int j = low; j <= high - 1; j++) {
        // se o elemento atual eh maior ou igual o pivot.
        if (arr[j].second >= pivot) {
            i++;  // Incrementa o indice do menor elemento.
            swap(arr[i], arr[j]);
        }
    }
    swap(arr[i + 1], arr[high]);
    return (i + 1);
}

// Quicksort 
void quicksort_float(vector<pair<int,float>>&arr, int low, int high) {
    if (low < high) {
        //  indice do particionador
        int pivot = partition_float(arr, low, high);

        quicksort_float(arr, low, pivot - 1);
        quicksort_float(arr, pivot + 1, high);
    }
}

// Wrapper function to call quicksort
void descendingQuicksort_float(vector<pair<int,float>>&arr) {
    quicksort_float(arr, 0, arr.size() - 1);
}

// Funcao de particionamento do vetor.
int partition_secondary(vector<Player*>&arr, int low, int high) {
    double pivot = arr[high]->media;  // Escolhe o elemento mais a direita do vetor
    int i = (low - 1);  // Indice do menor elemento

    for (int j = low; j <= high - 1; j++) {
        // se o elemento atual eh maior ou igual o pivot.
        if (arr[j]->media >= pivot) {
            i++;  // Incrementa o indice do menor elemento.
            swap(arr[i], arr[j]);
        }
    }
    swap(arr[i + 1], arr[high]);
    return (i + 1);
}

// Quicksort 
void quicksort_secondary(vector<Player*>&arr, int low, int high) {
    if (low < high) {
        //  indice do particionador
        int pivot = partition_secondary(arr, low, high);

        quicksort_secondary(arr, low, pivot - 1);
        quicksort_secondary(arr, pivot + 1, high);
    }
}

// Wrapper function to call quicksort
void descendingQuicksort_secondary(vector<Player*>&arr,int inicio, int fim) {
    quicksort_secondary(arr, inicio, fim);
}

vector<int> findIntersection(const vector<vector<int>>& ids) {
    unordered_map<int, int> idCount;
    int totalVetores = ids.size();

    // Conta a frequência de cada ID
    for (const auto& vetor : ids) {
        for (int id : vetor) {
            idCount[id]++;
        }
    }

    // Encontra os IDs que aparecem em todos os vetores
    vector<int> intersecao;
    for (const auto& pair : idCount) {
        if (pair.second == totalVetores) {
            intersecao.push_back(pair.first);
        }
    }

    return intersecao;
}

//TRIE PARA BUSCA POR PREFIXO DOS NOMES
class TrieName{
private:
    TrieNode* root;
    void getNames(TrieNode* node, string& prefix){
        if(node->isEndOfWord)
            cout << prefix << endl;
        for(int i = 0; i < R; ++i){
            if(node->children[i] != nullptr){ //Percorre os nomes que satisfazem o prefixo
                prefix.push_back(static_cast<char>(i));
                getNames(node->children[i], prefix);
                prefix.pop_back();
            }
        }
    }

    void getNamesAndIDs(TrieNode* node, string& prefix, vector<int>& ids) {
        if (node->isEndOfWord) {
            ids.push_back(node->ID);
    }

    for (int i = 0; i < R; ++i) {
        if (node->children[i] != nullptr) {
            prefix.push_back('a' + i);
            getNamesAndIDs(node->children[i], prefix, ids);
            prefix.pop_back();
        }
    }
    }
    

public:
    TrieName() : root(new TrieNode()) {} //Construtor que inicializa root com a raiz da TRIE

    //Insere uma palavra na TRIE
    void insertName(const string& word, int id){
        TrieNode* node = root;
        for(char ch : word){ //Percorre a TRIE no caminho das letras passadas
            int index = static_cast<int>(ch);
            if(node->children[index] == nullptr)
                node->children[index] = new TrieNode();
            node = node->children[index];
        }
        node->isEndOfWord = true;
        node->ID = id; //Marca como fim de palavra e vincula ela ao ID passado
    }

    //Printa os nomes da trie que satisfazem o prefixo passado
    void searchPrefix(const string& prefix){
        TrieNode* node = root;
        for(char ch : prefix){
            int index = static_cast<int>(ch);
            if(node->children[index] == nullptr){
                cout << "Nenhum nome encontrado com " << prefix << endl;
                return;
            }
            node = node->children[index];
        }
        //Chama função privada para obter os nomes que satisfazem a tag
        string currentPrefix = prefix;
        getNames(node, currentPrefix);
    }
    vector<int> searchByPrefix(const string& prefix) {
        vector<int> ids;
        TrieNode* node = root;
        for (char ch : prefix) {
            int index = static_cast<int>(ch);
            if (node->children[index] == nullptr) {
                return ids; // n encontrou 
            }
            node = node->children[index];
        }
        string currentPrefix = prefix;
        getNamesAndIDs(node, currentPrefix, ids);
        return ids;
    }
};

//TRIE PARA ARMAZENAR TAGS E VINCULA-LAS AOS JOGADORES
class TrieTags{
private:
    TrieNode* root;

public:
    TrieTags() : root(new TrieNode()) {} //Construtor que inicializa root com a raiz da TRIE

    //Insere uma palavra na TRIE e a vicula ao jogador
    void insertTag(const string& word, int id){
        TrieNode* node = root;
        for(char ch : word){
            int index = static_cast<int>(ch);
            if(node->children[index] == nullptr)
                node->children[index] = new TrieNode();
            node = node->children[index];
        }
        node->isEndOfWord = true;
        for(auto& item : node->tagsID){
            if(item == id)
                return; //Se o jogador ja teve o ID vinculado a essa tag, não adiciona de novo
        }
        node->tagsID.emplace_back(id);
    }

    //Retorna os ids
    vector<int> searchWord(string& word){
        TrieNode* node = root;
        for(char ch : word){
            int index = static_cast<int>(ch);
            if(node->children[index] == nullptr)
                return {};
            node = node->children[index];
        }
        if(node->isEndOfWord == true){
            return node->tagsID;
            
        }
        else
            return {};
    }

};

//HASH PARA ARMAZENAR JOGADORES
class HashPlayers{

    private:
        //Função Hash
        int hashgroups; //Tamanho da tabela
        //vector<vector<Player>> table; //Vector de list de players
        int HashFunction(int id){
            return int(id*1.21) % hashgroups;
        }
    public:
        //É preciso detalhar o tamanho da tabela ao cria-la
        HashPlayers(int groups) : hashgroups(groups), table(groups){}
        vector<vector<Player>> table; //Vector de list de players
        //Insere os dados do jogador na tabela
        void insertPlayer(int id, double rating = 0.0, const string& shortname = "", const string& longname = "", const string& positions= "", const string& nation = "", const string& team = "", const string& league = ""){
            int hashValue = HashFunction(id);
            auto& cell = table[hashValue]; //Abre a tabela na posição hash, e cell vira referência para a lista de players
            for(auto& item : cell){
                if(item.Id == id){ //Se o jogador ja existir, atualiza seus dados
                    item.sumRating += rating;
                    item.countRatings++;
                    return;
                }
            }
            cell.emplace_back(id, shortname, longname, positions, nation, team, league); //Caso o jogador não exista, insere ele e seus dados no final da lista
        }

        //Procura um player pelo ID, retorna um ponteiro para ele
        Player* searchPlayer(int id){
            int hashValue = HashFunction(id);
            auto& cell = table[hashValue];
            for(auto& item : cell){ //Procura a ID na lista
                if(item.Id == id) return &item; //Se encontrar a ID, retorna o ponteiro
            }
            cout << "Player nao encontrado" << endl; //Caso contrario, avisa e retorna NULL
            return nullptr;
        }

};

//HASH PARA ARAMAZENAR USUARIOS E SUAS REVIEWS
class HashUsers{
    private:
        int hashgroups; //Tamanho da tabela
        vector<vector<User>> table; //Vector de list de players
        //Função Hash
        int HashFunction(int id){
            return int(id*1.21) % hashgroups;
        }
    public:
        //É preciso detalhar o tamanho da tabela ao cria-la
        HashUsers(int groups) : hashgroups(groups), table(groups){}

        //Insere um usuário e sua review do jogador
        void insertUser(int id, const int& playerId, const float& rating){
            int hashValue = HashFunction(id);
            auto& cell = table[hashValue]; //Abre a tabela na posição hash, e cell vira referência para a lista de users

            for(auto& item : cell){
                if(item.Id == id){ //Se o usuario ja existir, atualiza seus dados
                    item.ratings.emplace_back(make_pair(playerId, rating));
                    return;
                }
            }
            vector<pair<int, float>> vec;
            vec.emplace_back(make_pair(playerId, rating));
            cell.emplace_back(id, vec); //Caso o usuario não exista, insere ele e seus dados no final da lista
        }

        User* sendUserData(int id){
            int hashValue = HashFunction(id);
            auto& cell = table[hashValue];
            for(auto& item : cell){ //Procura a ID na lista
                if(item.Id == id){
                    //for(auto& i : item.ratings)
                      //  cout << "User: "<< item.Id << " Player "<< i.first << " Rating: " << i.second << endl;
                      return &item;
                }
            }
            cout << "Usuario nao encontrado" << endl; //Caso contrario, avisa e retorna NULL
            return nullptr;

        }


};

//Copia o players.csv para o hashPlayers, e seus nomes na arvore TRIE
void copyPlayers(const string& file_name, HashPlayers& hashp, TrieName& trie){
    io::LineReader in(file_name);
    in.next_line();
    char c;
        
    string line, idStr, shortname, longname, positionsStr, nation, team, league;
    while(char* line = in.next_line()){
        stringstream ss(line);
        getline(ss, idStr, ',');
        getline(ss, shortname, ',');
        getline(ss, longname, ',');
        ss.get(c);
        if(c == '"'){
            getline(ss, positionsStr, '"'); // Lê o conteúdo entre aspas
            ss.get(c);
        }else{
            ss.seekg(-1, ios::ios_base::cur);
            getline(ss,positionsStr, ',');
        }

        getline(ss, nation, ','); 
        getline(ss, team, ','); 
        getline(ss, league); 

        int id = stoi(idStr);
        //Passa as posições para a lista de posições
        //vector<string> positions;
        //stringstream posStream(positionsStr);
        //string position;
        //while(getline(posStream, position, ',')){
            //positions.emplace_back(position);
        //}
        //Insere o jogador na tabela
        hashp.insertPlayer(id, 0.0, shortname, longname, positionsStr, nation, team, league);
        //Insere o nome do jogador na trie, junto com seu ID
        trie.insertName(longname, id);
    }

}

//Guarda as reviews dos usuarios e adiciona as médias dos jogadores no seu hashPlayers
void copyRatings(const string& file_name, HashUsers& hashU, HashPlayers& hashP){
    io::CSVReader<3> in(file_name);
    in.read_header(io::ignore_extra_column, "user_id" , "sofifa_id", "rating");
    int user; 
    int player; 
    float rating;
    while(in.read_row(user, player, rating)){ //Passa o conteúdo da linha para as variaveis
        hashU.insertUser(user, player, rating);
        hashP.insertPlayer(player, rating);
    }
}

//Guarda as tags na TRIE as vincula aos jogadores que as receberam
void copyTags(const string& file_name, TrieTags& trieT){
    io::CSVReader<3> in(file_name);  
    in.read_header(io::ignore_extra_column, "user_id", "sofifa_id", "tag");
    
    int user; 
    int player;  
    string tag;

    while(in.read_row(user, player, tag)){ //Passa o conteúdo da linha para as variaveis
        trieT.insertTag(tag, player);
    }
}

void menu(TrieTags trieT, TrieName trieN, HashPlayers& hashP,HashUsers hashU) {
    int opcao;
        do {
            cout << "\nMenu de Pesquisa:\n";
            cout << "1. Pesquisar por prefixo de nome de jogadores\n";
            cout << "2. Pesquisar por jogadores revisados por usuários\n";
            cout << "3. Pesquisar por melhores jogadores em determinada posição\n";
            cout << "4. Pesquisar por lista de tags\n";
            cout << "0. Sair\n";
            cout << "Digite a opção desejada: ";
            cin >> opcao;

            switch (opcao){
                case 1: {
                    string prefix;
                    cout << "A sintaxe dessa consulta é player <prefix >. \n";
                    cout<<"Digite o prefixo: \n";
                    cin >> prefix;
                    vector<int>PlayersFounded = trieN.searchByPrefix(prefix);
                    vector<Player*> players;
                    Player* player;
                    
                    //Procura o jogador pelo id e imprime os dados referentes a pesquisa no terminal.
                    for(const auto& id : PlayersFounded)
                    {
                        player = hashP.searchPlayer(id);
                        player->media= player->getMedia();
                        players.push_back(player);

                    }
                    descendingQuicksort_secondary(players,0,players.size()-1);
                    //Precisa tabular os dados, arrumar a leitura das posicoes do players.csv e ajustar a precisao da media das avaliacoes
                    cout << endl;
                    cout << left;
                    cout << setw(10) << "sofifa_id" 
                    << setw(20) << "short_name" 
                    << setw(45) << "long_name" 
                    << setw(20) << "player_positions" 
                    << setw(15) << "rating" 
                    << setw(10) << "count" << endl;
                    for(int i=0;i < players.size();i++){
                        cout << setw(10) << players[i]->Id
                        << setw(20) << players[i]->shortName
                        << setw(45) << players[i]->longName
                        << setw(20) << players[i]->Positions
                        << fixed << setprecision(6) << setw(15) << players[i]->media
                        << setw(10) << players[i]->countRatings <<endl;
                    }

                    break;
                }
                case 2: {

                    int id;
                    cout << "A sintaxe dessa consulta é: user <userID> \n";
                    cout<< "Digite o id de usuario a ser pesquisado: ";
                    cin >> id;
                    cout << left;
                    User* user = hashU.sendUserData(id);
                    
                    vector<Player*>players;
                    Player* player;
                    vector<pair<int,float>>vec;
                    descendingQuicksort_float(user->ratings);
                    //ordenação primária
                    for(int i=0;i< 20;i++)
                    {
                        //auto& item = user->ratings[i];
                        vec.emplace_back(make_pair(user->ratings[i].first, user->ratings[i].second));
                        player = hashP.searchPlayer(user->ratings[i].first);
                        player->rating = user->ratings[i].second;
                        player->media= player->getMedia();
                        players.push_back(player);
                    }
                    
                    //ordenação secundária (média global de avaliações)
                    int inicio=0,fim=0;
                    player = players[0];
                    
                    for(int i=1;i< players.size();i++){
                        if((player->rating) == (players[i]->rating) && (i != 19)){
                            fim++;
                            }
                        else{
                            descendingQuicksort_secondary(players,inicio,fim);
                            fim++;
                            inicio = fim;
                            player = players[inicio];
                        }
                    }
                    cout << endl;
                    cout << setw(10) << "sofifa_id" << setw(20) << "short_name" << setw(45) << "long_name" << setw(20) << "global_rating" << setw(15) << "count" << setw(10) << "rating" << endl;
                    for(int i=0;i < players.size();i++){
                        cout << setw(10) << players[i]->Id
                        << setw(20) << players[i]->shortName
                        << setw(45) << players[i]->longName
                        << fixed << setprecision(6) << setw(20) << players[i]->media
                        << setw(15) << players[i]->countRatings
                        << fixed << setprecision(1) << setw(10) << players[i]->rating <<endl;
                    }

                    break;
                }
                case 3: {
                    int n;
                    string position;
                    cout << "A sintaxe dessa consulta é top<N><position>" << endl;
                    cin >> n;
                    cin >> position;
                    vector<Player*> players;
                    //Player player;
                    for(auto& vec : hashP.table){
                        for(auto& item : vec){
                            if(item.Positions.find(position) != std::string::npos){
                                players.emplace_back(&item);
                            }
                        }
                    }

                    for(int k = 0; k < players.size(); k++){
                        players[k]->media = players[k]->getMedia();
                    }                    

                    descendingQuicksort_secondary(players, 0, players.size()-1);
                    int cont = 0;
                    int i = 0;
                    cout << endl;
                    cout << left;
                    cout << setw(10) << "sofifa_id" 
                    << setw(20) << "short_name" 
                    << setw(40) << "long_name" 
                    << setw(25) << "player_positions"
                    << setw(20) << "nationality" 
                    << setw(30) << "club_name"
                    << setw(35) << "league_name"  
                    << setw(10) << "rating" 
                    << setw(10) << "count" << endl;
                    while(cont != n){
                        if(players[i]->countRatings > 1000){
                            cout << setw(10) << players[i]->Id 
                            << setw(20) << players[i]->shortName 
                            << setw(40) << players[i]->longName  
                            << setw(25) << players[i]->Positions
                            << setw(20) << players[i]->Nation 
                            << setw(30) << players[i]->Team
                            << setw(35) << players[i]->League
                            << setw(10) << fixed << setprecision(6) << players[i]->media 
                            << setw(10) << players[i]->countRatings << endl;   
                            cont++;
                        }
                        i++;
                    }
                    break;

                }
                case 4: {
                    // Implementar a pesquisa por lista de tags
                    vector<string> tags;
                
                    string tagAtual,input;
                    cout<<"Digite uma lista de tags entre apostrofes e separadas por virgula: ";
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    getline(cin,input);
                    size_t pos=0;
    
                    cout << input<<endl;
                    while((pos = input.find('\'',pos)) != string::npos)
                    {   
                        pos++;
                        size_t nextPos = input.find('\'',pos);
                        tagAtual= input.substr(pos, nextPos - pos);
                        tags.push_back(tagAtual);
                        pos = nextPos + 1;
                    }

                   vector<vector<int>>ids;
                   vector<int>intersecao;

                   ids.resize(tags.size());

                   for(int i=0;i< tags.size();i++){
                    ids[i]=trieT.searchWord(tags[i]);
                   }
                   intersecao = findIntersection(ids);
                   vector<Player*> players;
                   Player* player;
                   for(auto& ids : intersecao){
                    player = hashP.searchPlayer(ids);
                    player->media = player->getMedia();
                    players.push_back(player);
                   }
                   descendingQuicksort_secondary(players,0,players.size()-1);
                    cout << endl;
                    cout << left;
                    cout << setw(10) << "sofifa_id" 
                    << setw(20) << "short_name" 
                    << setw(60) << "long_name" 
                    << setw(20) << "player_positions" 
                    << setw(15) << "nationality" 
                    << setw(30) << "club_name"
                    << setw(35) << "league_name"  
                    << setw(10) << "rating" 
                    << setw(10) << "count" << endl;

                    for(int i = 0; i < players.size(); i++){
                        cout << setw(10) << players[i]->Id 
                        << setw(20) << players[i]->shortName 
                        << setw(60) << players[i]->longName  
                        << setw(20) << players[i]->Positions
                        << setw(15) << players[i]->Nation 
                        << setw(30) << players[i]->Team
                        << setw(35) << players[i]->League
                        << fixed << setprecision(6) << setw(10) << players[i]->media 
                        << setw(10) << players[i]->countRatings << endl;   
                    }
                    break;
                }
                case 0:
                    std::cout << "Saindo...\n";
                    break;
                default:
                    std::cout << "Opção inválida.\n";
            }
        } while (opcao != 0);
    }

int main() {

    //Declaração das estruturas
    HashPlayers hashP(3989);
    HashUsers hashU(55587);
    TrieName trieN;
    TrieTags trieT;

    struct timeval inicio, fim;
    double tempo_decorrido;

    //Abre os arquivos e mede o tempo em segundos
    gettimeofday(&inicio, NULL);
    copyPlayers("./arquivos/players.csv", hashP, trieN);
    copyRatings("./arquivos/rating.csv",hashU, hashP);
    copyTags("./arquivos/tags.csv", trieT);
    gettimeofday(&fim, NULL);

    tempo_decorrido = (fim.tv_sec - inicio.tv_sec);
    tempo_decorrido += (fim.tv_usec - inicio.tv_usec)/1000000.0;
    cout << "TEMPO DE CONSTRUCAO DAS ESTRUTURAS " << tempo_decorrido << endl;

    menu(trieT,trieN,hashP,hashU);

    //cin.get();
    return 0;
}
