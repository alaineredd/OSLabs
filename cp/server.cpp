#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cstring>
#include <random>
#include <ctime>
#include <atomic>
#include <memory>

#include "os.h"

struct Game {
    std::string name;
    std::string secretWord;
    std::set<std::string> players;
    std::map<std::string, std::string> guesses;
    bool gameOver;
    std::string winner;
    int maxPlayers;
};

struct ThreadArgs {
    std::atomic<bool>* running;
    std::map<std::string, Game>* games;
    OSInterface* os;
};

class BullsAndCowsServer {
private:
    std::map<std::string, Game> games;
    std::vector<std::string> wordList;
    std::unique_ptr<OSInterface> os;
    std::atomic<bool> running;
    void* cleanup_thread;
    char* shm_request_ptr;
    char* shm_response_ptr;
    
    static BullsAndCowsServer* server_instance;
    
public:
    BullsAndCowsServer() : running(true), cleanup_thread(nullptr), 
                          shm_request_ptr(nullptr), shm_response_ptr(nullptr) {
        loadWordList();
        os = OSFactory::createForCurrentOS();
        if (!os) {
            throw std::runtime_error("Failed to create OS interface");
        }
        server_instance = this;
    }
    
    ~BullsAndCowsServer() {
        stop();
        server_instance = nullptr;
    }
    
    void loadWordList() {
        wordList = {
            "apple", "bread", "chair", "dance", "earth",
            "flame", "glass", "heart", "image", "jolly",
            "knife", "lemon", "music", "night", "ocean",
            "piano", "queen", "river", "smile", "table",
            "umbra", "vital", "water", "xenon", "yacht",
            "zebra", "brain", "cloud", "dream", "eagle"
        };
    }
    
    std::string getRandomWord() {
        std::mt19937 rng(std::time(nullptr));
        std::uniform_int_distribution<size_t> dist(0, wordList.size() - 1);
        return wordList[dist(rng)];
    }
    
    static void* cleanupGamesThread(void* arg) {
        ThreadArgs* args = static_cast<ThreadArgs*>(arg);
        
        while (*(args->running)) {
            args->os->sleepMilliseconds(60000);

            auto it = args->games->begin();
            while (it != args->games->end()) {
                if (it->second.players.empty()) {
                    std::cout << "[SERVER] Cleaning up empty game: " << it->first << std::endl;
                    it = args->games->erase(it);
                } else {
                    ++it;
                }
            }
        }
        
        delete args;
        return nullptr;
    }
    
    bool initialize() {
        if (!os->createSharedMemory(SharedResources::SHARED_MEMORY_REQUEST, 
                                   SharedResources::DEFAULT_SHM_SIZE)) {
            return false;
        }
        
        shm_request_ptr = os->mapSharedMemory(SharedResources::DEFAULT_SHM_SIZE);
        if (!shm_request_ptr) {
            return false;
        }

        if (!os->createSharedMemory(SharedResources::SHARED_MEMORY_RESPONSE, 
                                   SharedResources::DEFAULT_SHM_SIZE)) {
            return false;
        }

        shm_response_ptr = os->mapSharedMemory(SharedResources::DEFAULT_SHM_SIZE);
        if (!shm_response_ptr) {
            return false;
        }

        memset(shm_request_ptr, 0, SharedResources::DEFAULT_SHM_SIZE);
        memset(shm_response_ptr, 0, SharedResources::DEFAULT_SHM_SIZE);

        if (!os->createSemaphore(SharedResources::SEMAPHORE_NAME)) {
            return false;
        }

        ThreadArgs* args = new ThreadArgs{&running, &games, os.get()};
        cleanup_thread = os->createThread(cleanupGamesThread, args);
        if (!cleanup_thread) {
            delete args;
            return false;
        }

        os->detachThread(cleanup_thread);
        
        std::cout << "[SERVER] Shared memory initialized with 2 buffers" << std::endl;
        return true;
    }
    
    std::pair<int, int> calculateBullsAndCows(const std::string& secret, const std::string& guess) {
        int bulls = 0;
        int cows = 0;
        
        for (size_t i = 0; i < secret.length(); ++i) {
            if (secret[i] == guess[i]) {
                bulls++;
            } else if (secret.find(guess[i]) != std::string::npos) {
                cows++;
            }
        }
        
        return {bulls, cows};
    }
    
    void processClientMessage(const std::string& message, std::string& response) {
        size_t pos1 = message.find('|');
        size_t pos2 = message.find('|', pos1 + 1);
        
        if (pos1 == std::string::npos || pos2 == std::string::npos) {
            response = "ERROR|Invalid message format";
            return;
        }
        
        std::string playerName = message.substr(0, pos1);
        std::string command = message.substr(pos1 + 1, pos2 - pos1 - 1);
        std::string data_msg = message.substr(pos2 + 1);
        
        std::cout << "[SERVER] Processing command: " << command << " from " << playerName << std::endl;
        
        if (command == "CREATE") {
            createGame(playerName, data_msg, response);
        } else if (command == "JOIN") {
            joinGame(playerName, data_msg, response);
        } else if (command == "GUESS") {
            processGuess(playerName, data_msg, response);
        } else if (command == "LIST") {
            listGames(response);
        } else if (command == "LEAVE") {
            leaveGame(playerName, data_msg, response);
        } else if (command == "STATUS") {
            getGameStatus(data_msg, response);
        } else if (command == "QUIT") {
            removePlayerFromAllGames(playerName);
            response = "OK|Goodbye";
        } else {
            response = "ERROR|Unknown command: " + command;
        }
    }
    
    void createGame(const std::string& playerName, const std::string& data_msg, std::string& response) {
        size_t pos = data_msg.find('|');
        if (pos == std::string::npos) {
            response = "ERROR|Invalid create game format";
            return;
        }
        
        std::string gameName = data_msg.substr(0, pos);
        int maxPlayers;
        try {
            maxPlayers = std::stoi(data_msg.substr(pos + 1));
        } catch (const std::exception& e) {
            response = "ERROR|Invalid max players value";
            return;
        }
        
        if (maxPlayers < 1 || maxPlayers > 10) {
            response = "ERROR|Max players must be between 1 and 10";
            return;
        }
        
        if (games.find(gameName) != games.end()) {
            response = "ERROR|Game already exists";
            return;
        }
        
        Game newGame;
        newGame.name = gameName;
        newGame.secretWord = getRandomWord();
        newGame.players.insert(playerName);
        newGame.gameOver = false;
        newGame.maxPlayers = maxPlayers;
        
        games[gameName] = newGame;
        std::cout << "[SERVER] Game '" << gameName << "' created with secret word: " 
                  << newGame.secretWord << ", max players: " << maxPlayers << std::endl;
        response = "OK|Game created|Secret word set|Max players: " + std::to_string(maxPlayers);
    }
    
    void joinGame(const std::string& playerName, const std::string& gameName, std::string& response) {
        auto it = games.find(gameName);
        if (it == games.end()) {
            response = "ERROR|Game not found";
            return;
        }
        
        if (it->second.gameOver) {
            response = "ERROR|Game is over";
            return;
        }
        
        if (it->second.players.size() >= static_cast<size_t>(it->second.maxPlayers)) {
            response = "ERROR|Game is full|Current players: " + std::to_string(it->second.players.size()) +
                      "|Max players: " + std::to_string(it->second.maxPlayers);
            return;
        }
        
        it->second.players.insert(playerName);
        response = "OK|Joined game|Current players: " + std::to_string(it->second.players.size()) +
                  "|Max players: " + std::to_string(it->second.maxPlayers);
    }
    
    void processGuess(const std::string& playerName, const std::string& data_msg, std::string& response) {
        size_t pos = data_msg.find('|');
        if (pos == std::string::npos) {
            response = "ERROR|Invalid guess format";
            return;
        }
        
        std::string gameName = data_msg.substr(0, pos);
        std::string guess = data_msg.substr(pos + 1);
        
        auto it = games.find(gameName);
        if (it == games.end()) {
            response = "ERROR|Game not found";
            return;
        }
        
        if (it->second.gameOver) {
            response = "ERROR|Game is over";
            return;
        }
        
        if (it->second.players.find(playerName) == it->second.players.end()) {
            response = "ERROR|You are not in this game";
            return;
        }
        
        if (guess.length() != 5) {
            response = "ERROR|Word must be 5 letters";
            return;
        }
        
        for (char& c : guess) {
            c = std::tolower(c);
        }
        
        it->second.guesses[playerName] = guess;
        
        if (guess == it->second.secretWord) {
            it->second.gameOver = true;
            it->second.winner = playerName;
            response = "WINNER|" + playerName + "|You guessed the word!";
        } else {
            auto [bulls, cows] = calculateBullsAndCows(it->second.secretWord, guess);
            response = "RESULT|Bulls: " + std::to_string(bulls) + "|Cows: " + std::to_string(cows);
        }
    }
    
    void listGames(std::string& response) {
        if (games.empty()) {
            response = "INFO|No active games";
            return;
        }
        
        response = "LIST";
        for (const auto& [name, game] : games) {
            response += "|" + name + " (" + std::to_string(game.players.size()) + 
                       "/" + std::to_string(game.maxPlayers) + " players)" + 
                       (game.gameOver ? " [Finished]" : "");
        }
    }
    
    void leaveGame(const std::string& playerName, const std::string& gameName, std::string& response) {
        auto it = games.find(gameName);
        if (it == games.end()) {
            response = "ERROR|Game not found";
            return;
        }
        
        it->second.players.erase(playerName);
        it->second.guesses.erase(playerName);
        
        if (it->second.players.empty()) {
            games.erase(it);
            response = "OK|Game removed";
        } else {
            response = "OK|Left game|Remaining players: " + std::to_string(it->second.players.size()) +
                      "|Max players: " + std::to_string(it->second.maxPlayers);
        }
    }
    
    void getGameStatus(const std::string& gameName, std::string& response) {
        auto it = games.find(gameName);
        if (it == games.end()) {
            response = "ERROR|Game not found";
            return;
        }
        
        response = "STATUS|Players: " + std::to_string(it->second.players.size()) +
                  "/" + std::to_string(it->second.maxPlayers) +
                  "|Game over: " + std::string(it->second.gameOver ? "Yes" : "No");
        
        if (it->second.gameOver) {
            response += "|Winner: " + it->second.winner;
        }
    }
    
    void removePlayerFromAllGames(const std::string& playerName) {
        std::vector<std::string> gamesToRemove;
        
        for (auto& [gameName, game] : games) {
            game.players.erase(playerName);
            game.guesses.erase(playerName);
            
            if (game.players.empty()) {
                gamesToRemove.push_back(gameName);
            }
        }
        
        for (const auto& gameName : gamesToRemove) {
            games.erase(gameName);
        }
    }
    
    static void signalHandler(int sig) {
        if (server_instance) {
            server_instance->handleSignal(sig);
        }
    }
    
    void handleSignal(int sig) {
        std::cout << "\n[SERVER] Received signal " << sig << ", shutting down..." << std::endl;
        running = false;
    }
    
    void run() {
        if (!initialize()) {
            std::cerr << "Failed to initialize server" << std::endl;
            return;
        }
        
        std::cout << "[SERVER] Server started on " << os->getOSName() << std::endl;
        std::cout << "[SERVER] Waiting for clients..." << std::endl;
        std::cout << "[SERVER] Press Ctrl+C to stop the server." << std::endl;
        
        os->setupInterruptHandler(signalHandler);
        
        while (running) {
            os->waitSemaphore();

            if (shm_request_ptr && strlen(shm_request_ptr) > 0) {
                std::string request(shm_request_ptr);
                std::cout << "\n[SERVER] Received request: " << request << std::endl;

                std::string response;
                processClientMessage(request, response);

                memset(shm_response_ptr, 0, SharedResources::DEFAULT_SHM_SIZE);
                strncpy(shm_response_ptr, response.c_str(), SharedResources::DEFAULT_SHM_SIZE - 1);

                memset(shm_request_ptr, 0, SharedResources::DEFAULT_SHM_SIZE);
                
                std::cout << "[SERVER] Sent response: " << response << std::endl;
            }
            
            os->postSemaphore();

            os->sleepMilliseconds(50);
        }
        
        std::cout << "\n[SERVER] Server stopped." << std::endl;
    }
    
    void stop() {
        running = false;
        
        cleanup_thread = nullptr;
        
        if (shm_request_ptr) {
            os->unmapSharedMemory();
            shm_request_ptr = nullptr;
        }
        if (shm_response_ptr) {
            os->unmapSharedMemory();
            shm_response_ptr = nullptr;
        }
        os->destroySharedMemory(SharedResources::SHARED_MEMORY_REQUEST);
        os->destroySharedMemory(SharedResources::SHARED_MEMORY_RESPONSE);
        os->destroySemaphore(SharedResources::SEMAPHORE_NAME);
        os->cleanupResources();
    }
};

BullsAndCowsServer* BullsAndCowsServer::server_instance = nullptr;

int main() {
    try {
        BullsAndCowsServer server;
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}