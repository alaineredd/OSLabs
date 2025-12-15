#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <atomic>
#include <memory>
#include <algorithm>
#include <cctype>

#include "os.h"

class BullsAndCowsClient;

struct ClientThreadArgs {
    std::atomic<bool>* running;
    BullsAndCowsClient* client;
};

class BullsAndCowsClient {
private:
    std::unique_ptr<OSInterface> os;
    std::string playerName;
    std::string currentGame;
    std::atomic<bool> running;
    void* input_thread;
    char* shm_request_ptr;
    char* shm_response_ptr;
    
public:
    BullsAndCowsClient(const std::string& name) : 
        playerName(name), running(true), input_thread(nullptr), 
        shm_request_ptr(nullptr), shm_response_ptr(nullptr) {
        os = OSFactory::createForCurrentOS();
        if (!os) {
            throw std::runtime_error("Failed to create OS interface");
        }
    }
    
    ~BullsAndCowsClient() {
        disconnect();
    }
    
    static void* inputHandlerThread(void* arg) {
        ClientThreadArgs* args = static_cast<ClientThreadArgs*>(arg);
        args->client->showMenuLoop();
        delete args;
        return nullptr;
    }
    
    void showMenuLoop() {
        int choice;
        
        while (running) {
            showMenu();
            
            if (!(std::cin >> choice)) {
                std::cin.clear();
                std::cin.ignore(10000, '\n');
                continue;
            }
            
            processChoice(choice);
        }
    }
    
    void showMenu() {
        std::cout << "\n=== Bulls and Cows Game ===" << std::endl;
        std::cout << "Player: " << playerName << std::endl;
        if (!currentGame.empty()) {
            std::cout << "Current game: " << currentGame << std::endl;
        }
        std::cout << "1. Create new game" << std::endl;
        std::cout << "2. Join existing game" << std::endl;
        std::cout << "3. Make a guess" << std::endl;
        std::cout << "4. List all games" << std::endl;
        std::cout << "5. Leave current game" << std::endl;
        std::cout << "6. Check game status" << std::endl;
        std::cout << "7. Exit" << std::endl;
        std::cout << "Choose option: ";
    }
    
    void processChoice(int choice) {
        switch (choice) {
            case 1:
                createGame();
                break;
            case 2:
                joinGame();
                break;
            case 3:
                makeGuess();
                break;
            case 4:
                listGames();
                break;
            case 5:
                leaveGame();
                break;
            case 6:
                gameStatus();
                break;
            case 7:
                quit();
                running = false;
                break;
            default:
                std::cout << "Invalid option!" << std::endl;
        }
    }
    
    bool connect() {
            if (!os->openSharedMemory(SharedResources::SHARED_MEMORY_REQUEST)) {
            std::cerr << "[CLIENT] Failed to open request shared memory" << std::endl;
            return false;
        }
        
        shm_request_ptr = os->mapSharedMemory(SharedResources::DEFAULT_SHM_SIZE);
        if (!shm_request_ptr) {
            std::cerr << "[CLIENT] Failed to map request shared memory" << std::endl;
            return false;
        }
        
        if (!os->openSharedMemory(SharedResources::SHARED_MEMORY_RESPONSE)) {
            std::cerr << "[CLIENT] Failed to open response shared memory" << std::endl;
            return false;
        }
        
        shm_response_ptr = os->mapSharedMemory(SharedResources::DEFAULT_SHM_SIZE);
        if (!shm_response_ptr) {
            std::cerr << "[CLIENT] Failed to map response shared memory" << std::endl;
            return false;
        }
        
        if (!os->openSemaphore(SharedResources::SEMAPHORE_NAME)) {
            std::cerr << "[CLIENT] Failed to open semaphore" << std::endl;
            return false;
        }
        
        memset(shm_request_ptr, 0, SharedResources::DEFAULT_SHM_SIZE);
        memset(shm_response_ptr, 0, SharedResources::DEFAULT_SHM_SIZE);
        
        std::cout << "[CLIENT] Connected to shared memory" << std::endl;
        return true;
    }
    
    std::string sendAndReceive(const std::string& message) {
        os->waitSemaphore();
        
        memset(shm_response_ptr, 0, SharedResources::DEFAULT_SHM_SIZE);
        
        memset(shm_request_ptr, 0, SharedResources::DEFAULT_SHM_SIZE);
        strncpy(shm_request_ptr, message.c_str(), SharedResources::DEFAULT_SHM_SIZE - 1);
        std::cout << "[CLIENT] Sent request: " << message << std::endl;
        
        os->postSemaphore();
        
        int attempts = 0;
        const int max_attempts = 100;
        std::string response;
        
        while (attempts < max_attempts && running) {
            os->sleepMilliseconds(100);
            
            os->waitSemaphore();
            
            if (strlen(shm_response_ptr) > 0) {
                response = std::string(shm_response_ptr);
                std::cout << "[CLIENT] Received response: " << response << std::endl;
                
                memset(shm_request_ptr, 0, SharedResources::DEFAULT_SHM_SIZE);
                
                os->postSemaphore();
                break;
            }
            
            os->postSemaphore();
            attempts++;
        }
        
        if (attempts >= max_attempts) {
            std::cerr << "[CLIENT] Timeout waiting for server response" << std::endl;
            return "ERROR|Server timeout";
        }
        
        if (response.empty()) {
            return "ERROR|Empty response from server";
        }
        
        return response;
    }
    
    void createGame() {
        std::string gameName;
        std::cout << "Enter game name: ";
        std::cin >> gameName;
        
        if (gameName.empty()) {
            std::cout << "Game name cannot be empty!" << std::endl;
            return;
        }
        
        std::string message = playerName + "|CREATE|" + gameName;
        std::string response = sendAndReceive(message);
        
        std::cout << "Server response:\n" << parseResponse(response) << std::endl;
        
        if (response.find("OK") == 0) {
            currentGame = gameName;
            std::cout << "✓ Game '" << gameName << "' created successfully!" << std::endl;
        }
    }
    
    void joinGame() {
        std::string gameName;
        std::cout << "Enter game name to join: ";
        std::cin >> gameName;
        
        if (gameName.empty()) {
            std::cout << "Game name cannot be empty!" << std::endl;
            return;
        }
        
        std::string message = playerName + "|JOIN|" + gameName;
        std::string response = sendAndReceive(message);
        
        std::cout << "Server response:\n" << parseResponse(response) << std::endl;
        
        if (response.find("OK") == 0) {
            currentGame = gameName;
            std::cout << "✓ Joined game '" << gameName << "' successfully!" << std::endl;
        }
    }
    
    void makeGuess() {
        if (currentGame.empty()) {
            std::cout << "You are not in any game!" << std::endl;
            return;
        }
        
        std::string guess;
        std::cout << "Enter your 5-letter guess: ";
        std::cin >> guess;
        
        if (guess.length() != 5) {
            std::cout << "Word must be 5 letters long!" << std::endl;
            return;
        }
        
        for (char& c : guess) {
            c = std::tolower(c);
        }
        
        std::string message = playerName + "|GUESS|" + currentGame + "|" + guess;
        std::string response = sendAndReceive(message);
        
        std::string result = parseResponse(response);
        std::cout << "Result:\n" << result << std::endl;
        
        if (response.find("WINNER") == 0) {
            currentGame.clear();
            std::cout << "🎉 Congratulations! You won the game!" << std::endl;
        }
    }
    
    void listGames() {
        std::string message = playerName + "|LIST|";
        std::string response = sendAndReceive(message);
        
        std::cout << "Available games:\n" << parseResponse(response) << std::endl;
    }
    
    void leaveGame() {
        if (currentGame.empty()) {
            std::cout << "You are not in any game!" << std::endl;
            return;
        }
        
        std::cout << "Leaving game '" << currentGame << "'..." << std::endl;
        std::string message = playerName + "|LEAVE|" + currentGame;
        
        std::string response = sendAndReceive(message);
        
        std::cout << parseResponse(response) << std::endl;
        currentGame.clear();
    }
    
    void gameStatus() {
        if (currentGame.empty()) {
            std::cout << "Enter game name: ";
            std::string gameName;
            std::cin >> gameName;
            currentGame = gameName;
        }
        
        std::string message = playerName + "|STATUS|" + currentGame;
        std::cout << "Checking status of game '" << currentGame << "'..." << std::endl;
        
        std::string response = sendAndReceive(message);
        
        std::cout << "Game status:\n" << parseResponse(response) << std::endl;
    }
    
    void quit() {
        std::cout << "Disconnecting from server..." << std::endl;
        
        if (!currentGame.empty()) {
            std::string message = playerName + "|LEAVE|" + currentGame;
            sendAndReceive(message);
        }
        
        std::string message = playerName + "|QUIT|";
        sendAndReceive(message);
    }
    
    std::string parseResponse(const std::string& response) {
        std::string result;
        size_t start = 0;
        size_t end = response.find('|');
        
        while (end != std::string::npos) {
            if (start != 0) result += "\n";
            result += response.substr(start, end - start);
            start = end + 1;
            end = response.find('|', start);
        }
        
        if (start < response.length()) {
            if (start != 0) result += "\n";
            result += response.substr(start);
        }
        
        return result;
    }
    
    void run() {
        std::cout << "==================================" << std::endl;
        std::cout << "  Bulls and Cows Client" << std::endl;
        std::cout << "  Player: " << playerName << std::endl;
        std::cout << "==================================" << std::endl;
        
        if (!connect()) {
            std::cerr << "Failed to connect to server" << std::endl;
            std::cerr << "Make sure server is running!" << std::endl;
            return;
        }
        
        std::cout << "Connected to server on " << os->getOSName() << "!" << std::endl;
        
        ClientThreadArgs* args = new ClientThreadArgs();
        args->running = &running;
        args->client = this;
        input_thread = os->createThread(inputHandlerThread, args);
        if (!input_thread) {
            delete args;
            std::cerr << "Failed to create input thread" << std::endl;
            return;
        }

        os->joinThread(input_thread);
        input_thread = nullptr;
        
        std::cout << "\nGoodbye!" << std::endl;
    }
    
    void disconnect() {
        running = false;
        
        if (input_thread) {
            os->joinThread(input_thread);
            input_thread = nullptr;
        }
        
        os->cleanupResources();
    }
};

int main() {
    try {
        std::string playerName;
        std::cout << "Enter your name: ";
        std::getline(std::cin, playerName);
        
        if (playerName.empty()) {
            std::cout << "Name cannot be empty. Using default name 'Player'" << std::endl;
            playerName = "Player";
        }
        
        BullsAndCowsClient client(playerName);
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}