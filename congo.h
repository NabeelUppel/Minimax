#include <fstream>
#include <random>
#include <regex>
#include <limits>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <array>
#include <utility>
#include <vector>
#include <map>
#include <iterator>
#include <unordered_map>
#include <set>

#ifndef CONGO_CONGO_H
#define CONGO_CONGO_H

#define STANDARD 0
#define ALPHA_BETA 1
#define TRANSPOSITION_TABLES 2
#define RANDOM 3
#define ORDERED 4
#define ITERATIVE_DEEPENING 5

template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator &g) {
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}

std::chrono::duration<double> seconds_to_duration(double seconds) {
    return std::chrono::duration<double, std::ratio<1>>(seconds);
}

char flipColour(char c) {
    if (c == 'w') {
        return 'b';
    } else {
        return 'w';
    }

}

std::string contractDigits(const std::string &str) {
    std::string final;
    int running_total = 0;
    for (auto ch: str) {
        if (ch == '1') {
            running_total++;
            continue;
        }

        if (running_total > 0) { final += std::to_string(running_total); }
        final += ch;
        running_total = 0;
    }
    if (running_total > 0) { final += std::to_string(running_total); }
    return final;
}

std::vector<std::string> splitFen(std::string fen) {
    std::vector<std::string> split;
    std::regex re("[/\\s]");
    std::sregex_token_iterator first{fen.begin(), fen.end(), re, -1}, last;
    split = {first, last};
    return split;
}

std::array<char, 49> buildBoardFromFen(std::string fen) {
    std::array<char, 49> board{};
    board.fill('_');
    auto split = splitFen(std::move(fen));
    //int turnNum = std::stoi(split.back());
    split.pop_back();
    //char toPlay = split.back()[0];
    split.pop_back();

    int total = 0;
    //Fills Board Up
    for (const auto &i: split) {
        for (auto j: i) {
            if (isdigit(j)) {
                int num = j - '0';
                total += num;
            } else {

                board[total] = j;
                total++;
            }
        }
    }

    return board;
}

bool validCoord(std::string coord) {
    int letter = int((unsigned char) coord[0]);
    int number = coord[1] - '0';

    if (letter < 97 || letter > 103 || number < 1 || number > 7) {
        return false;
    }
    return true;
}

char findToPlay(std::string fen) {
    size_t foundW = fen.find('w');
    size_t foundB = fen.find('b');
    char toPlay;
    if (foundW != std::string::npos) {
        toPlay = fen[foundW];
    }
    if (foundB != std::string::npos) {
        toPlay = fen[foundB];
    }
    return toPlay;
}

int getSinglePiecePos(std::array<char, 49> b, char p) {
    for (int i = 0; i < b.size(); ++i) {
        if (b[i] == p) {
            return i;
        }
    }
    return -1;
}

std::vector<int> getMultiplePiecePos(std::array<char, 49> b, char p) {
    std::vector<int> Pos;
    for (int i = 0; i < b.size(); ++i) {
        if (b[i] == p) {
            Pos.push_back(i);
        }
    }
    return Pos;
}

std::string getCoordFromIndex(int index) {
    std::vector<char> letters = {'a', 'b', 'c', 'd', 'e', 'f', 'g'};
    std::vector<char> numbers = {'7', '6', '5', '4', '3', '2', '1'};
    char column = letters[index % 7];
    char row = numbers[index / 7];
    std::string tile;
    tile += column;
    tile += row;
    return tile;
}

char setPieceCase(char toPlay, char piece) {
    if (toPlay == 'w') {
        return (char) toupper(piece);
    } else {
        return (char) tolower(piece);
    }
}

char getPieceColour(char piece) {
    if (isupper(piece))
        return 'w';
    else if (islower(piece))
        return 'b';
    else
        return 'n';
}

int getIndexFromCoord(std::string coord) {
    int letter = int((unsigned char) coord[0]);
    int number = coord[1] - '0';

    if (!validCoord(coord)) {
        throw std::invalid_argument("Please Enter Valid Coordinate");
    }

    int row = (7 - number) * 7;
    int column = letter - 97;

    return row + column;
}

std::vector<std::string> LionMoves(std::array<char, 49> board, char toPlay) {
    std::vector<std::string> LegalMoves;
    char Piece = setPieceCase(toPlay, 'l');
    std::vector<int> confinement;
    std::vector<int> offset = {1, 6, 7, 8, -1, -6, -7, -8};

    int MyLionIndex = getSinglePiecePos(board, Piece);
    std::string MyLionCoord = getCoordFromIndex(MyLionIndex);
    char oppLion;
    std::string OppLionCoord;
    int OppLionIndex;
    int diag;
    confinement = {2, 3, 4, 9, 10, 11, 16, 17, 18,30, 31, 32, 37, 38, 39, 44, 45, 46};
    if (toPlay == 'w') {
        oppLion = 'l';
    } else {
        oppLion = 'L';
    }
    OppLionIndex = getSinglePiecePos(board, oppLion);
    OppLionCoord = getCoordFromIndex(OppLionIndex);
    diag = MyLionIndex - OppLionIndex;

    for (auto i: offset) {
        int newPos = MyLionIndex + i;
        if (newPos < board.size() && newPos >= 0) {
            if (std::find(confinement.begin(), confinement.end(), newPos) != confinement.end()) {
                char tile = board[newPos];
                if (getPieceColour(tile) != toPlay) {
                    std::string move;
                    move += MyLionCoord;
                    move += getCoordFromIndex(newPos);
                    LegalMoves.push_back(move);
                }
            }
        }

    }

    // InLine with OppLion
    if (MyLionCoord[0] == OppLionCoord[0]) {
        bool freeCol = true;
        int it = MyLionIndex;
        while (it != OppLionIndex) {
            if (MyLionIndex > OppLionIndex) {
                it -= 7;
            } else {
                it += 7;
            }
            if (board[it] != '_' && it != OppLionIndex) {
                freeCol = false;
                break;
            }
        }
        if (freeCol) {
            std::string move;
            move += MyLionCoord;
            move += OppLionCoord;
            LegalMoves.push_back(move);
        }
    }

    //Diagonal With OppLion
    if (diag == 12 || diag == 16 || diag == -12 || diag == -16) {
        if (board[24] == '_') {
            std::string move;
            move += MyLionCoord;
            move += OppLionCoord;
            LegalMoves.push_back(move);
        }
    }

    sort(LegalMoves.begin(), LegalMoves.end());
    return LegalMoves;
}

std::vector<std::string> ZebraMoves(std::array<char, 49> board, char toPlay) {
    std::vector<std::string> LegalMoves;
    char Piece = setPieceCase(toPlay, 'z');
    int MyZebraIndex = getSinglePiecePos(board, Piece);

    std::string MyZebraCoord = getCoordFromIndex(MyZebraIndex);
    int letter = (int) (unsigned char) MyZebraCoord[0];
    int num = (int) (unsigned char) MyZebraCoord[1];

    std::vector<int> letterOffset = {2, 1, -1, -2, -2, -1, 1, 2};
    std::vector<int> NumOffset = {1, 2, 2, 1, -1, -2, -2, -1};
    for (int i = 0; i < letterOffset.size(); ++i) {
        int newNum = (num + NumOffset[i]);
        int newLetter = (letter + letterOffset[i]);
        if (newLetter > 96 && newLetter <= 103 && newNum > 48 && newNum <= 55) {
            std::string newCoord;
            newCoord += (char) newLetter;
            newCoord += (char) newNum;
            int newIndex = getIndexFromCoord(newCoord);
            char tile = board[newIndex];

            if (getPieceColour(tile) != toPlay) {
                std::string move;
                move += MyZebraCoord;
                move += newCoord;
                LegalMoves.push_back(move);
            }
        }
    }


    std::sort(LegalMoves.begin(), LegalMoves.end());
    return LegalMoves;
}

std::vector<std::string> ElephantHelper(std::array<char, 49> b, int startPos, char toPlay) {
    //char Piece = b[startPos];
    std::vector<std::string> LegalMovesList;

    std::string startCoord = getCoordFromIndex(startPos);
    int letter = (int) (unsigned char) startCoord[0];
    int num = (int) (unsigned char) startCoord[1];

    std::vector<int> letterOffset = {0, 0, 0, 0, 1, -1, -2, 2};
    std::vector<int> NumOffset = {1, -1, 2, -2, 0, 0, 0, 0};
    for (int i = 0; i < letterOffset.size(); ++i) {
        int newNum = (num + NumOffset[i]);
        int newLetter = (letter + letterOffset[i]);
        if (newLetter > 96 && newLetter <= 103 && newNum > 48 && newNum <= 55) {
            std::string newCoord;
            newCoord += (char) newLetter;
            newCoord += (char) newNum;
            int newIndex = getIndexFromCoord(newCoord);
            char tile = b[newIndex];
            if (getPieceColour(tile) != toPlay) {
                std::string move;
                move += startCoord;
                move += newCoord;
                LegalMovesList.push_back(move);
            }
        }
    }

    return LegalMovesList;
}

std::vector<std::string> ElephantMoves(std::array<char, 49> board, char toPlay) {
    std::vector<std::string> LegalMoves;
    char Piece = setPieceCase(toPlay, 'e');
    std::vector<int> Positions = getMultiplePiecePos(board, Piece);


    for (auto i: Positions) {
        std::vector<std::string> moves = ElephantHelper(board, i, toPlay);
        LegalMoves.insert(LegalMoves.end(), moves.begin(), moves.end());
    }
    std::sort(LegalMoves.begin(), LegalMoves.end());

    return LegalMoves;
}

std::vector<std::string> PawnHelper(std::array<char, 49> b, int startPos, char toPlay) {
    //char Piece = b[startPos];
    std::vector<int> letterOffset = {0, -1, 1};
    std::vector<int> NumOffset;
    std::vector<int> OverRiverNumOffset;
    std::vector<int> overRiverValues;
    if (toPlay == 'w') {
        overRiverValues = {5, 6, 7};
        NumOffset = {1, 1, 1};
        OverRiverNumOffset = {-1, -2};
    } else {
        overRiverValues = {1, 2, 3};
        NumOffset = {-1, -1, -1};
        OverRiverNumOffset = {1, 2};
    }

    std::vector<std::string> LegalMovesList;
    std::string startCoord = getCoordFromIndex(startPos);
    int letter = (int) (unsigned char) startCoord[0];
    int num = (int) (unsigned char) startCoord[1];
    for (int i = 0; i < letterOffset.size(); ++i) {
        int newNum = (num + NumOffset[i]);
        int newLetter = (letter + letterOffset[i]);
        if (newLetter > 96 && newLetter <= 103 && newNum > 48 && newNum <= 55) {
            std::string newCoord;
            newCoord += (char) newLetter;
            newCoord += (char) newNum;
            int newIndex = getIndexFromCoord(newCoord);
            char tile = b[newIndex];
            if (getPieceColour(tile) != toPlay) {
                std::string move;
                move += startCoord;
                move += newCoord;
                LegalMovesList.push_back(move);
            }
        }
    }

    int numberCoord = (int) num - '0';
    if (std::find(overRiverValues.begin(), overRiverValues.end(), numberCoord) != overRiverValues.end()) {
        //over river
        for (int i: OverRiverNumOffset) {
            int newNum = (num + i);
            int newLetter = letter;
            if (newNum > 48 && newNum <= 55) {
                std::string newCoord;
                newCoord += (char) newLetter;
                newCoord += (char) newNum;
                int newIndex = getIndexFromCoord(newCoord);
                char tile = b[newIndex];
                if (tile == '_') {
                    std::string move;
                    move += startCoord;
                    move += newCoord;
                    LegalMovesList.push_back(move);
                } else {
                    break;
                }
            }
        }
    }

    return LegalMovesList;
}

std::vector<std::string> PawnMoves(std::array<char, 49> board, char toPlay) {
    std::vector<std::string> LegalMoves;
    char Piece = setPieceCase(toPlay, 'p');
    std::vector<int> Positions = getMultiplePiecePos(board, Piece);

    for (auto i: Positions) {
        std::vector<std::string> moves = PawnHelper(board, i, toPlay);
        LegalMoves.insert(LegalMoves.end(), moves.begin(), moves.end());
    }
    std::sort(LegalMoves.begin(), LegalMoves.end());
    return LegalMoves;
}

void DumpVectorContents(std::vector<std::string> &into, std::vector<std::string> toDump) {
    into.insert(into.end(), toDump.begin(), toDump.end());
}

std::string getRandomMove(std::vector<std::string> AllMoves) {
    return *select_randomly(AllMoves.begin(), AllMoves.end());
}

int getTurnNumber(std::string fen) {
    fen = fen.substr(fen.find(' '));
    fen.erase(
            remove_if(fen.begin(), fen.end(), [](char c) { return !std::isdigit(c); }),
            fen.end());
    return std::stoi(fen);
}
void printMoves(const std::vector<std::string> &moves) {
    std::string sep;
    for (const auto &i: moves) {
        std::cout << sep << i;
        sep = ' ';
    }
    std::cout << std::endl;
}


#endif //CONGO_CONGO_H
