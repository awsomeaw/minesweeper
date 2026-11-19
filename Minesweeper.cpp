#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <stdexcept>
using namespace std;

const int MAX_ROWS = 16;
const int MAX_COLS = 30;
const int INVALID_COORDINATE = -1;

enum Difficulty { 
    BEGINNER = 1, 
    INTERMEDIATE = 2, 
    EXPERT = 3, 
    CUSTOM = 4 
};

enum GameState { PLAYING, WON, LOST, QUIT };

class Cell {
private:
    bool mine, revealed, flagged;
    int adjacentMines;

public:
    Cell() { reset(); }
    void setMine(bool value) { mine = value; }
    bool hasMine() const { return mine; }
    void setRevealed(bool value) { revealed = value; }
    bool isRevealed() const { return revealed; }
    void setFlagged(bool value) { flagged = value; }
    bool isFlagged() const { return flagged; }
    void setAdjacentMines(int value) { adjacentMines = value; }
    int getAdjacentMines() const { return adjacentMines; }
    void reset() {
        mine = false;
        revealed = false;
        flagged = false;
        adjacentMines = 0;
    }
};

class PlayerStats {
private:
    string playerName;
    int moves, flagsPlaced, cellsRevealed, gamesWon, gamesLost;

public:
    PlayerStats() {
        playerName = "Player";
        moves = flagsPlaced = cellsRevealed = gamesWon = gamesLost = 0;
    }

    void setName(const string& name) { playerName = name.empty() ? "Player" : name; }
    string getName() const { return playerName; }
    void addMove() { moves++; }
    void addRevealedCell() { cellsRevealed++; }
    void addFlag() { flagsPlaced++; }
    void removeFlag() { if (flagsPlaced > 0) flagsPlaced--; }
    void recordWin() { gamesWon++; }
    void recordLoss() { gamesLost++; }
    void resetRound() { moves = flagsPlaced = cellsRevealed = 0; }

    void displayRoundStats() const {
        cout << "\n----------- Round statistics -----------\n";
        cout << "Player: " << playerName << '\n';
        cout << "Moves: " << moves << '\n';
        cout << "Cells revealed: " << cellsRevealed << '\n';
        cout << "Flags placed: " << flagsPlaced << '\n';
        cout << "----------------------------------------\n";
    }

    void displayOverallStats() const {
        cout << "\n----------- PLAYER RECORD -----------\n";
        cout << "Player: " << playerName << '\n';
        cout << "Games won: " << gamesWon << '\n';
        cout << "Games lost: " << gamesLost << '\n';
        cout << "--------------------------------------\n";
    }
};

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int readInteger(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            clearInput();
            return value;
        }
        cout << "Please enter a whole number.\n";
        clearInput();
    }
}

string readText(const string& prompt) {
    string value;
    cout << prompt;
    getline(cin, value);
    return value;
}

class Minesweeper {
private:
    Cell board[MAX_ROWS][MAX_COLS];
    Cell* boardPointer[MAX_ROWS];
    int rows, columns, mineCount, safeCellsRemaining;
    GameState state;
    Difficulty difficulty;
    PlayerStats stats;
    bool firstMove;

    void initializeBoard();
    void connectBoardPointers();
    void placeMines(int safeRow, int safeColumn);
    void calculateNumbers();
    bool isInsideBoard(int row, int column) const;
    bool isValidSelection(int row, int column) const;
    bool isSafeCell(int row, int column) const;
    void revealCell(int row, int column);
    void revealEmptyNeighbors(int row, int column);
    void toggleFlag(int row, int column);
    void revealAllMines();
    void checkForWin();
    void processSelection(int row, int column);
    void displayHeader() const;
    void displayBoard(bool showAll = false) const;
    void displayCell(int row, int column, bool showAll) const;
    void displayLegend() const;
    void displayCommands() const;
    void displayEndMessage() const;

public:
    Minesweeper();
    void setPlayerName(const string& name) { stats.setName(name); }
    void startNewGame();
    void play();
    void showInstructions() const;
    void chooseDifficulty();
    void setDifficulty(Difficulty selected);
    void setCustomBoard(int newRows, int newColumns, int newMines);
    GameState getGameState() const;
    void showStatistics() const;
};

// Constructor and pointer setup
Minesweeper::Minesweeper() {
    rows = 9;
    columns = 9;
    mineCount = 10;
    safeCellsRemaining = 0;
    state = PLAYING;
    difficulty = BEGINNER;
    firstMove = true;
    connectBoardPointers();
    initializeBoard();
}

void Minesweeper::connectBoardPointers() {
    for (int row = 0; row < MAX_ROWS; row++) boardPointer[row] = board[row];
}

void Minesweeper::initializeBoard() {
    for (int row = 0; row < MAX_ROWS; row++)
        for (int column = 0; column < MAX_COLS; column++)
            board[row][column].reset();

    safeCellsRemaining = (rows * columns) - mineCount;

    state = PLAYING;
}

// Difficulty selection
void Minesweeper::chooseDifficulty() {
    cout << "\nSelect difficulty:\n";
    cout << "1. Beginner      (9 x 9, 10 mines)\n";
    cout << "2. Intermediate  (12 x 12, 25 mines)\n";
    cout << "3. Expert        (16 x 30, 99 mines)\n";
    cout << "4. Custom board\n";
    int choice = readInteger("\nEnter your choice: ");

    switch (choice) {
        case 1: setDifficulty(BEGINNER); break;
        case 2: setDifficulty(INTERMEDIATE); break;
        case 3: setDifficulty(EXPERT); break;
        case 4: {
            int customRows = readInteger("Enter rows (5-16): ");
            int customColumns = readInteger("Enter columns (5-30): ");
            int customMines = readInteger("Enter mine count: ");
            try {
                setCustomBoard(customRows, customColumns, customMines);
            }
            catch (const exception& error) {
                cout << "Custom board error: " << error.what() << '\n';
                cout << "Beginner settings will be used instead.\n";
                setDifficulty(BEGINNER);
            }
            break;
        }
        default:
            cout << "Invalid choice. Beginner selected.\n";
            setDifficulty(BEGINNER);
            break;
    }
}

void Minesweeper::setDifficulty(Difficulty selected) {
    difficulty = selected;

    switch (selected) {
        case BEGINNER:     rows = 9;  columns = 9;  mineCount = 10; break;
        case INTERMEDIATE: rows = 12; columns = 12; mineCount = 25; break;
        case EXPERT:       rows = 16; columns = 30; mineCount = 99; break;
        default:
            rows = 9;
            columns = 9;
            mineCount = 10;
            difficulty = BEGINNER;
            break;
    }

    initializeBoard();
}

// Custom board validation
void Minesweeper::setCustomBoard(int newRows, int newColumns, int newMines) {
    if (newRows < 5 || newRows > MAX_ROWS)
        throw invalid_argument("Rows must be between 5 and 16.");
    if (newColumns < 5 || newColumns > MAX_COLS)
        throw invalid_argument("Columns must be between 5 and 30.");
    if (newMines < 1 || newMines > newRows * newColumns - 9)
        throw invalid_argument("Mine count must leave at least 9 safe cells.");

    rows = newRows;
    columns = newColumns;
    mineCount = newMines;
    difficulty = CUSTOM;
    initializeBoard();
}

void Minesweeper::placeMines(int safeRow, int safeColumn) {
    // Never places a mine on the first selected cell or any cell next to it
    int placed = 0;

    while (placed < mineCount) {
        int randomRow = rand() % rows;
        int randomColumn = rand() % columns;

        // skip the cell and its 8 neighbors
        if (abs(randomRow - safeRow) <= 1 && abs(randomColumn - safeColumn) <= 1) continue;

        Cell* currentCell = boardPointer[randomRow] + randomColumn;

        if (!currentCell->hasMine()) {
            currentCell->setMine(true);
            placed++;
        }
    }
}

bool Minesweeper::isInsideBoard(int row, int column) const {
    return row >= 0 && row < rows && column >= 0 && column < columns;
}

bool Minesweeper::isValidSelection(int row, int column) const {
    return isInsideBoard(row, column) && !board[row][column].isRevealed() && !board[row][column].isFlagged();
}

bool Minesweeper::isSafeCell(int row, int column) const {
    return isInsideBoard(row, column) && !board[row][column].hasMine();
}

// Adjacent mines
void Minesweeper::calculateNumbers() {
    for (int row = 0; row < rows; row++) {
        for (int column = 0; column < columns; column++) {
            if (board[row][column].hasMine()) continue;

            int count = 0;
            for (int rowOffset = -1; rowOffset <= 1; rowOffset++) {
                for (int columnOffset = -1; columnOffset <= 1; columnOffset++) {
                    int neighborRow = row + rowOffset;
                    int neighborColumn = column + columnOffset;
                    if (isInsideBoard(neighborRow, neighborColumn) &&
                        board[neighborRow][neighborColumn].hasMine()) {
                        count++;
                    }
                }
            }
            board[row][column].setAdjacentMines(count);
        }
    }
}
    
// Reveal and flood-fill logic
void Minesweeper::revealCell(int row, int column) {
    if (!isValidSelection(row, column)) return;

    board[row][column].setRevealed(true);
    stats.addRevealedCell();

    if (board[row][column].hasMine()) {
        state = LOST;
        return;
    }

    safeCellsRemaining--;

    if (board[row][column].getAdjacentMines() == 0) {
        revealEmptyNeighbors(row, column);
    }

    checkForWin();
}

void Minesweeper::revealEmptyNeighbors(int row, int column) {
    for (int rowOffset = -1; rowOffset <= 1; rowOffset++) {
        for (int columnOffset = -1; columnOffset <= 1; columnOffset++) {
            if (rowOffset == 0 && columnOffset == 0) continue;
            revealCell(row + rowOffset, column + columnOffset);
        }
    }
}

void Minesweeper::toggleFlag(int row, int column) {
    if (!isInsideBoard(row, column)) {
        cout << "That position is outside the board.\n";
        return;
    }
    if (board[row][column].isRevealed()) {
        cout << "You cannot flag a revealed cell.\n";
        return;
    }

    if (board[row][column].isFlagged()) {
        board[row][column].setFlagged(false);
        stats.removeFlag();
        cout << "Flag removed.\n";
    }
    else {
        board[row][column].setFlagged(true);
        stats.addFlag();
        cout << "Flag placed.\n";
    }
}

void Minesweeper::checkForWin() {
    if (safeCellsRemaining == 0 && state == PLAYING) {
        state = WON;
        stats.recordWin();
    }
}

void Minesweeper::processSelection(int row, int column) {
    if (!isInsideBoard(row, column)) {
        cout << "Invalid position. Choose a square on the board.\n";
        return;
    }
    if (!isValidSelection(row, column)) {
        cout << "That square is already revealed or flagged.\n";
        return;
    }

    if (firstMove) {
        placeMines(row, column);
        calculateNumbers();
        firstMove = false;
    }

    stats.addMove();
    revealCell(row, column);

    if (state == LOST) stats.recordLoss();
}

void Minesweeper::revealAllMines() {
    for (int row = 0; row < rows; row++)
        for (int column = 0; column < columns; column++)
            if (board[row][column].hasMine()) board[row][column].setRevealed(true);
}

// Board display
void Minesweeper::displayHeader() const {
    cout << "\n========================================\n";
    cout << "Player: " << stats.getName() << '\n';
    cout << "Board: " << rows << " x " << columns << "\n";
    cout << "Mines: " << mineCount << '\n';
    cout << "Safe cells remaining: " << safeCellsRemaining << '\n';
    cout << "========================================\n";
}

void Minesweeper::displayCell(int row, int column, bool showAll) const {
    const Cell& current = board[row][column];

    if (showAll && current.hasMine()) cout << " * ";
    else if (current.isFlagged() && !current.isRevealed()) cout << " F ";
    else if (!current.isRevealed()) cout << " # ";
    else if (current.hasMine()) cout << " * ";
    else if (current.getAdjacentMines() == 0) cout << "   ";
    else cout << " " << current.getAdjacentMines() << " ";
}

void Minesweeper::displayBoard(bool showAll) const {
    cout << "\n    ";                               // column numbers
    for (int column = 0; column < columns; column++) cout << setw(3) << column + 1;

    cout << "\n    +";                              // top border
    for (int column = 0; column < columns; column++) cout << "---";
    cout << "+\n";

    // board rows
    for (int row = 0; row < rows; row++) {
        cout << setw(3) << row + 1 << " |";         // row numbers + left border
        for (int column = 0; column < columns; column++) displayCell(row, column, showAll);
        cout << "|\n";                              // right border
    }

    // bottom border
    cout << "    +";
    for (int column = 0; column < columns; column++) cout << "---";
    cout << "+\n";
}

void Minesweeper::displayLegend() const {
    cout << "\nLegend:\n";
    cout << " #        Hidden square\n";
    cout << " F        Flagged square\n";
    cout << " *        Mine\n";
    cout << " 1-8      Number of adjacent mines\n";
    cout << " (blank)  Safe square\n";
}

// Command list
void Minesweeper::displayCommands() const {
    cout << "\nCommands:\n";
    cout << "  reveal x,y   (or 'r')\n";
    cout << "  flag x,y     (or 'f')\n";
    cout << "  help         (or 'h')\n";
    cout << "  quit         (or 'q')\n";
}

void Minesweeper::displayEndMessage() const {
    cout << "\n========================================\n";

    if (state == WON) {
        cout << "              YOU WIN!                  \n";
        cout << "Congratulations! You've cleared the board.\n";
    }
    else if (state == LOST) {
        cout << "             GAME OVER                  \n";
        cout << "You selected a mine :(\n";
    }
    else if (state == QUIT) {
        cout << "          GAME ENDED EARLY              \n";
    }

    cout << "========================================\n";
}

void Minesweeper::showInstructions() const {
    cout << "\n------------- HOW TO PLAY -------------\n";
    cout << "The board contains hidden squares.\n";
    cout << "Some squares contain hidden mines.\n";
    cout << "Reveal a safe square to continue.\n";
    cout << "A number shows nearby mines.\n";
    cout << "Use flags to mark squares you suspect.\n";
    cout << "Reveal every safe square to win.\n";
    cout << "Reveal a mine and the game ends.\n";
    cout << "---------------------------------------\n";
    displayCommands();
    displayLegend();
}

GameState Minesweeper::getGameState() const { return state; }

void Minesweeper::showStatistics() const { stats.displayOverallStats(); }

void Minesweeper::startNewGame() {
    initializeBoard();
    stats.resetRound();
    connectBoardPointers();
    firstMove = true;
    cout << "\nA new board has been created.\n";
}

// Main game loop
void Minesweeper::play() {
    while (state == PLAYING) {
        displayHeader();
        displayBoard();
        displayCommands();

        string line = readText("\nEnter command: ");

        if (!cin) {        // input stream closed
            state = QUIT;
            break;
        }

        // uppercase -> lowercase, tabs and commas -> spaces
        for (size_t i = 0; i < line.size(); i++) {
            if (line[i] >= 'A' && line[i] <= 'Z') line[i] = static_cast<char>(line[i] - 'A' + 'a');
            else if (line[i] == '\t' || line[i] == ',') line[i] = ' ';
        }

        // Read the command word
        size_t pos = 0;
        while (pos < line.size() && line[pos] == ' ') pos++;
        size_t start = pos;
        while (pos < line.size() && line[pos] >= 'a' && line[pos] <= 'z') pos++;
        string command = line.substr(start, pos - start);

        if (command == "help" || command == "h") {
            showInstructions();
        }
        else if (command == "quit" || command == "q") {
            state = QUIT;
        }
        else if (command == "reveal" || command == "r" || command == "flag" || command == "f") {
            // row, then column
            int numbers[2] = { 0, 0 };
            bool valid = true;

            for (int i = 0; i < 2 && valid; i++) {
                while (pos < line.size() && line[pos] == ' ') pos++;

                if (pos >= line.size() || line[pos] < '0' || line[pos] > '9') {
                    valid = false;
                    break;
                }

                while (pos < line.size() && line[pos] >= '0' && line[pos] <= '9') {
                    if (numbers[i] < 100000) numbers[i] = numbers[i] * 10 + (line[pos] - '0');
                    pos++;
                }
            }

            if (!valid) cout << "Use the format: reveal x,y or flag x,y\n";
            else if (command == "reveal" || command == "r") processSelection(numbers[0] - 1, numbers[1] - 1);
            else toggleFlag(numbers[0] - 1, numbers[1] - 1);
        }
        else {
            cout << "Unknown command. Try: reveal x,y | flag x,y | help | quit\n";
        }
    }

    if (state == LOST || state == WON) revealAllMines();

    displayBoard(true);
    displayEndMessage();
    stats.displayRoundStats();
}

void displayMainMenu() {
    cout << "\n========================================\n";
    cout << "1. Start game\n";
    cout << "2. Instructions\n";
    cout << "3. Exit\n";
    cout << "========================================\n";
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    Minesweeper game;

    string name = readText("Enter your name: ");
    game.setPlayerName(name);

    cout << "\nHello, " << (name.empty() ? "Player" : name) << "!\n";

    bool running = true;

    while (running) {
        displayMainMenu();
        int choice = readInteger("\nEnter your choice: ");

        switch (choice) {
            case 1:
                game.chooseDifficulty();
                game.startNewGame();
                game.play();
                break;
            case 2:
                game.showInstructions();
                break;
            case 3:
                running = false;
                cout << "\nThank you for playing!\n";
                break;
            default:
                cout << "Invalid menu choice. Try again.\n";
                break;
        }
    }

    return 0;
}
