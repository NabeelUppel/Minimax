#include "congo.h"


namespace Congo {
    class Board {
    public:
        explicit Board(const std::string &fen) {
            board = buildBoard(fen);
            toPlay = findToPlay(fen);
            turnNum = getTurnNumber(fen);
        }

        Board(const Board &b) : board(b.board), toPlay(b.toPlay), turnNum(b.turnNum), PieceLists(b.PieceLists) {}

        void updatePiecePositions(int des) {
            std::map<int, char> changePairs;
            for (const auto &i: PieceLists) {
                for (auto j: i.second) {
                    if (board[j] != i.first) {
                        changePairs[j] = i.first;
                    }
                }
            }

            for (auto i: changePairs) {
                PieceLists[i.second].erase(i.first);
            }
            if (board[des] != '_') {
                PieceLists[board[des]].insert(des);
            }
        }

        void printPiecePositions() {
            for (const auto &i: PieceLists) {
                std::cout << i.first << " ";
                for (auto j: i.second) {
                    std::cout << j << " ";
                }
                std::cout << std::endl;
            }
        }

        std::array<char, 49> buildBoard(std::string fen) {
            board.fill('_');
            auto split = splitFen(std::move(fen));
            turnNum = std::stoi(split.back());
            split.pop_back();
            toPlay = split.back()[0];
            split.pop_back();

            int total = 0;
            //Fills Board Up
            for (const auto &i: split) {
                for (auto j: i) {
                    if (isdigit(j)) {
                        int num = j - '0';
                        total += num;
                    } else {
                        if (PieceLists.find(j) == PieceLists.end()) {
                            std::set<int> pos;
                            pos.insert(total);
                            PieceLists[j] = pos;
                        } else {
                            PieceLists[j].insert(total);
                        }
                        board[total] = j;
                        total++;
                    }
                }
            }

            return board;
        }

        std::array<char, 49> getBoard() const {
            return board;
        }

        std::map<char, std::set<int>> getPiecePositions() const {
            return PieceLists;
        }

        void makeMove(const std::string &move) {
            std::string loc = move.substr(0, move.size() / 2);
            std::string des = move.substr(move.size() / 2);
            int locIndex = getIndexFromCoord(loc);
            int desIndex = getIndexFromCoord(des);
            int locNum = (int) (loc[1] - '0') - 1;
            int desNum = (int) (des[1] - '0') - 1;

            if (!validCoord(loc) || !validCoord(des)) {
                throw std::invalid_argument("Invalid Move");
            }

            if (islower(board[locIndex]) && toPlay == 'w' || std::isupper(board[locIndex]) && toPlay == 'b') {
                throw std::invalid_argument("Can't Move Opponent's Piece");
            }
            if (board[locIndex] == '_') {
                std::cout << move << std::endl;
                throw std::invalid_argument("No Piece To Move");
            }

            std::vector<char> beforeRiver(board.begin() + 21, board.begin() + 28);

            if ((isupper(board[locIndex]) && isupper(board[desIndex])) ||
                (std::islower(board[locIndex]) && std::islower(board[desIndex]))) {
                throw std::invalid_argument("Can't Take Your Own Piece");
            }

            if (locNum == desNum && locNum == 3) {
                board[desIndex] = '_';
                board[locIndex] = '_';
            } else {
                board[desIndex] = board[locIndex];
                board[locIndex] = '_';
            }

            std::vector<char> afterRiver(board.begin() + 21, board.begin() + 28);

            vectorRiverRemoval(beforeRiver, afterRiver);
            if (toPlay == 'b') {
                turnNum++;
            }
            toPlay = flipColour(toPlay);
            updatePiecePositions(desIndex);
        }

        std::vector<std::string> GenerateAllLegalMoves() const {
            std::vector<std::string> AllMoves;
            DumpVectorContents(AllMoves, LionMoves(board, toPlay));
            DumpVectorContents(AllMoves, ZebraMoves(board, toPlay));
            DumpVectorContents(AllMoves, ElephantMoves(board, toPlay));
            DumpVectorContents(AllMoves, PawnMoves(board, toPlay));
            //std::sort(AllMoves.begin(), AllMoves.end());
            return AllMoves;
        }

        char isGameOver() const {
            int whiteLion = -1;
            int blackLion = -1;
            int blackPieceCount = 0;
            int whitePieceCount = 0;
            char winner = 'n';
            for (int i = 0; i < board.size(); ++i) {
                if (isupper(board[i])) {
                    whitePieceCount++;
                    if (board[i] == 'L') {
                        whiteLion = i;
                    }
                } else if (islower(board[i])) {
                    blackPieceCount++;
                    if (board[i] == 'l') {
                        blackLion = i;
                    }
                }

                if (blackLion != -1 && whiteLion != -1 && blackPieceCount >= 2 && whitePieceCount >= 2) {
                    break;
                }
            }

            if (whiteLion == -1 && blackLion != -1) {
                winner = 'b';
                return winner;
            } else if (whiteLion != -1 && blackLion == -1) {
                winner = 'w';
                return winner;
            }

            if (blackPieceCount == 1 && whitePieceCount == 1) {
                winner = 'd';
                auto moves = LionMoves(board, toPlay);
                for (const auto &m: moves) {
                    int val = isEndGameMove(m);
                    if (val == 10)
                        winner = 'n';
                }
            }

            return winner;
        }

        int getPieceCount(char colour) const {
            int count = 0;
            for (auto i: board) {
                if (isupper(i) && colour == 'w') {
                    count++;
                }
                if (colour == 'b' && islower(i)) {
                    count++;
                }
            }
            return count;
        }

        int getRiverPieceCount(char colour) const {
            int count = 0;
            for (int i = 21; i < 28; ++i) {
                if (isupper(board[i]) && colour == 'w') {
                    count++;
                }
                if (colour == 'b' && islower(board[i])) {
                    count++;
                }
            }
            return count;
        }

        std::string getFenFromBoard() const {
            std::vector<std::string> rows;
            std::string A;
            std::string fen;
            std::string sep;
            for (int i = 0; i < board.size(); ++i) {
                A += board[i];
                if ((i + 1) % 7 == 0) {
                    std::replace(A.begin(), A.end(), '_', '1');
                    A = contractDigits(A);
                    fen += sep;
                    fen += A;
                    sep = "/";
                    A = "";
                }
            }

            fen += ' ';
            fen += toPlay;
            fen += ' ';
            fen += std::to_string(turnNum);
            return fen;
        }

        std::string getFen() const {

            std::string fen(49, '1');
            for (const auto &i: PieceLists) {
                for (auto j: i.second) {
                    fen[j] = i.first;
                }
            }
            int j = 0;
            for (int i = 7; i < 49; i += 7) {
                fen.insert(i + j, "/");
                j++;
            }

            fen = contractDigits(fen);
            fen += " ";
            fen += toPlay;
            fen += " ";
            fen += std::to_string(turnNum);
            return fen;
        }

        std::string getPieceString(char colour) const {
            std::string pieces = getFen();
            pieces.erase(remove_if(pieces.begin(), pieces.end(), [](char c) { return !isalpha(c); }), pieces.end());
            pieces.pop_back();
            if (colour == 'w') {
                pieces.erase(
                        remove_if(pieces.begin(), pieces.end(), [](char c) { return !std::isupper(c); }),
                        pieces.end());
            } else {
                pieces.erase(
                        remove_if(pieces.begin(), pieces.end(), [](char c) { return !std::islower(c); }),
                        pieces.end());
            }
            return pieces;
        }

        void printBoard() const {
            int size = (int) board.size();
            for (int i = 0; i < size; ++i) {
                if ((i + 1) % 7 == 0) {
                    std::cout << board[i] << std::endl;
                } else {
                    std::cout << board[i] << ' ';
                }
            }
            std::cout << std::endl;
        }

        char getToPlay() const {
            return toPlay;
        }

        int getTurnNum() const {
            return turnNum;
        }

        void flipToPlay() {
            if (toPlay == 'w') {
                toPlay = 'b';
            } else {
                toPlay = 'w';
            }
        }

    private:
        std::map<char, std::set<int>> PieceLists;
        std::array<char, 49> board{};
        char toPlay;
        int turnNum;
        std::map<char, int> pieceCapture = {{'P', 100},
                                            {'p', 100},
                                            {'Z', 300},
                                            {'z', 300},
                                            {'E', 200},
                                            {'e', 200},
                                            {'l', 10000},
                                            {'L', 10000}};

        void vectorRiverRemoval(std::vector<char> before, std::vector<char> after) {
            std::vector<char> newRiver = after;
            for (int i = 0; i < before.size(); ++i) {
                if (toPlay == 'w') {
                    if (isupper(before[i]) && before[i] == after[i]) {
                        newRiver[i] = '_';
                    }
                } else {
                    if (islower(before[i]) && before[i] == after[i]) {
                        newRiver[i] = '_';
                    }
                }
            }
            int j = 0;
            for (int i = 21; i < 28; ++i) {
                board[i] = newRiver[j];
                j++;
            }
        }

        int isEndGameMove(const std::string &move) const {
            std::string des = move.substr(move.size() / 2);
            int desIndex = getIndexFromCoord(des);
            char desPiece = board[desIndex];
            if (desPiece == 'L' || desPiece == 'l') {
                return 10;
            }
            return 0;
        }


        static int isAttackedTileHelper(const std::vector<std::string> &AllMoves, const std::string &move) {
            std::string des = move.substr(move.size() / 2);
            int total = 0;
            for (auto m: AllMoves) {
                std::string desM = move.substr(move.size() / 2);
                if (des == desM) {
                    total += -5;
                }
            }
            return total;
        }

    };

    enum FLAG {
        EXACT,
        LOWERBOUND,
        UPPERBOUND
    };

    struct HashEntry {
        HashEntry(int d, int s, FLAG f, std::string m, char tP) {
            depth = d;
            score = s;
            flag = f;
            move = std::move(m);
            toPlay = tP;
        }

        int depth;
        int score;
        std::string move;
        char toPlay;
        FLAG flag;
    };

    class MiniMax {
    private:
        std::mt19937 mt;
        std::string bestMove;
        std::chrono::duration<double> startTime;
        int LastCompletedDepth = 1;
        std::unordered_map<unsigned long long int, HashEntry *> TranspositionTable;

        unsigned long long int randomInt() {
            std::uniform_int_distribution<unsigned long long int>
                    dist(0, UINT64_MAX);
            return dist(mt);
        }

        void initZobristTable() {
            for (auto &i: ZobristTable) {
                for (unsigned long long &j: i) {
                    j = randomInt();
                }
            }
        }

        unsigned long long int computeHash(std::array<char, 49> board) {
            unsigned long long int h = 0;
            for (int i = 0; i < board.size(); ++i) {
                if (board[i] != '_') {
                    int j = ZobristPieceIndex[board[i]];
                    h ^= ZobristTable[i][j];
                }
            }
            return h;
        }

        void storeEntry(const Board &b, HashEntry *entry) {
            int transposition_index = (int) computeHash(b.getBoard()) & 0xFFFF;
            TranspositionTable[transposition_index] = entry;
        }

        HashEntry *GetTranspositionEntry(const Board &board) {
            int transposition_index = (int) computeHash(board.getBoard()) & 0xFFFF;
            if (TranspositionTable.find(transposition_index) != TranspositionTable.end()) {
                return TranspositionTable[transposition_index];
            }
            return nullptr;
        }

        void initPieceSquareTables() {
            std::array<int, 49> zebraPieceSquare =
                    {-50, -40, -30, -30, -30, -40, -50,
                     -40, -20, 0, 0, 0, -20, -40,
                     -30, 0, 10, 15, 10, 00, -30,
                     -30, -5, -15, -20, -15, -5, -30,
                     -30, 0, 10, 15, 10, 0, -30,
                     -40, -20, 0, 0, 0, -2, -40,
                     -50, -40, -30, -30, -30, -40, -50};

            std::array<int, 49> lionPieceSquare =
                    {0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, -0, 0, 0,
                     0, 0, 0, 0, 0, 0, 0,
                     0, 0, -20, -10, -20, 0, 0,
                     0, 0, -5, 2, -5, 0, 0,
                     0, 0, 10, 0, 10, 0, 0};

            //Pawns
            std::array<int, 49> PawnPieceSquare =
                    {0, 0, 0, 0, 0, 0, 0,
                     5, 5, 5, 5, 5, 5, 5,
                     7, 8, 9, 10, 9, 8, 7,
                     -7, -5, -4, -3, -4, -5, -7,
                     -3, 2, 7, 8, 7, 2, -3,
                     3, 3, 9, 9, 9, 3, 3,
                     0, 0, 0, 0, 0, 0, 0};

            std::array<int, 49> ElephantPieceSquare =
                    {-50, -30, -30, -30, -30, -30, -50,
                     -20, 10, 10, 15, 10, 10, -20,
                     -20, 20, 25, 30, 25, 20, -20,
                     -25, -5, -5, -5, -5, -5, -25,
                     -20, 20, 25, 30, 25, 20, -20,
                     -20, 10, 10, 15, 10, 10, -20,
                     -50, -30, -30, -30, -30, -30, -50};

            PieceSquareTables['L'] = lionPieceSquare;
            PieceSquareTables['P'] = PawnPieceSquare;
            PieceSquareTables['Z'] = zebraPieceSquare;
            PieceSquareTables['E'] = ElephantPieceSquare;
            std::reverse(lionPieceSquare.begin(), lionPieceSquare.end());
            std::reverse(PawnPieceSquare.begin(), PawnPieceSquare.end());
            PieceSquareTables['l'] = lionPieceSquare;
            PieceSquareTables['p'] = PawnPieceSquare;
            PieceSquareTables['z'] = zebraPieceSquare;
            PieceSquareTables['e'] = ElephantPieceSquare;
        }


    public:
        MiniMax() : mt((std::random_device()) ()) {
            initZobristTable();
            initPieceSquareTables();
        }

        int negINF = -10000000;
        int INF = 10000000;
        std::map<char, std::array<int, 49>> PieceSquareTables;
        std::map<int, std::pair<int, std::string>> DepthMoves;
        std::map<char, int> pieceValues = {{'P', 100},
                                           {'p', 100},
                                           {'Z', 300},
                                           {'z', 300},
                                           {'E', 200},
                                           {'e', 200}};


        std::map<char, int> ZobristPieceIndex = {{'P', 0},
                                                 {'E', 1},
                                                 {'Z', 2},
                                                 {'L', 3},
                                                 {'p', 4},
                                                 {'e', 5},
                                                 {'z', 6},
                                                 {'l', 7}};
        std::array<std::array<unsigned long long int, 8>, 49> ZobristTable{};


        int BasicBoardEvaluation(const Board &board) {
            int eval;
            int factor = 1;
            char toPlay = board.getToPlay();
            if (toPlay == 'b') {
                factor = -1;
            }

            char winner = board.isGameOver();
            if (winner == 'w') {
                return factor * 10000;
            }
            if (winner == 'b') {
                return factor * -10000;
            }


            std::string whitePieces = board.getPieceString('w');
            std::string blackPieces = board.getPieceString('b');

            if (whitePieces == "L" && blackPieces == "l") {
                return 0;
            }

            //Piece Value
            int wTotal = 0;
            int bTotal = 0;
            int total = 0;
            for (auto i: whitePieces) {
                wTotal += pieceValues[i];
            }
            for (auto i: blackPieces) {
                bTotal += pieceValues[i];
            }
            eval = (wTotal - bTotal);




            /* auto x = board.getPiecePositions();
             for (const auto &i: x) {
                 auto table = PieceSquareTables[i.first];
                 for (auto j: i.second) {
                     eval += table[j];
                 }
             }*/
            return factor * eval;
        }


        int boardEvaluation(const Board &board) {
            int eval;
            int factor = 1;
            char toPlay = board.getToPlay();
            if (toPlay == 'b') {
                factor = -1;
            }

            char winner = board.isGameOver();
            if (winner == 'w') {
                return factor * 10000;
            }
            if (winner == 'b') {
                return factor * -10000;
            }


            std::string whitePieces = board.getPieceString('w');
            std::string blackPieces = board.getPieceString('b');

            if (whitePieces == "L" && blackPieces == "l") {
                return 0;
            }

            //Piece Value
            int wTotal = 0;
            int bTotal = 0;
            int total = 0;
            for (auto i: whitePieces) {
                wTotal += pieceValues[i];
            }
            for (auto i: blackPieces) {
                bTotal += pieceValues[i];
            }
            eval = (wTotal - bTotal);


            Board oppBoard = board;
            oppBoard.flipToPlay();

            //Mobility Score
            auto oppMoves = oppBoard.GenerateAllLegalMoves();
            auto myMoves = board.GenerateAllLegalMoves();
            if (toPlay == 'w') {
                eval += (int) (myMoves.size() - oppMoves.size());
            } else {
                eval += (int) (oppMoves.size() - myMoves.size());
            }

            //Attacking Score
            int wAttackTotal = 0;
            int bAttackTotal = 0;
            auto myBoardArr = board.getBoard();
            for (const auto &m: myMoves) {
                int desIndex = getIndexFromCoord(m.substr(m.size() / 2));
                char desPieces = myBoardArr[desIndex];
                if (toPlay == 'w') {
                    if (islower(desPieces)) {
                        wAttackTotal += 1;
                        if (desPieces == 'l') {
                            wAttackTotal += 10;
                        }
                    }
                } else {
                    if (isupper(desPieces)) {
                        bAttackTotal += 1;
                        if (desPieces == 'L') {
                            bAttackTotal += 10;
                        }
                    }
                }
            }
            auto oppBoardArr = board.getBoard();
            for (const auto &m: oppMoves) {
                int desIndex = getIndexFromCoord(m.substr(m.size() / 2));
                char desPieces = myBoardArr[desIndex];
                if (toPlay == 'b') {
                    if (islower(desPieces)) {
                        wAttackTotal += 1;
                        if (desPieces == 'l') {
                            wAttackTotal += 10;
                        }
                    }
                } else {
                    if (isupper(desPieces)) {
                        bAttackTotal += 1;
                        if (desPieces == 'L') {
                            bAttackTotal += 10;
                        }
                    }
                }
            }
            eval += wAttackTotal - bAttackTotal;


            return factor * eval;
        }

        int negamax(const Board &board, int depth, int initDepth) {
            if (depth <= 0 || board.isGameOver() != 'n') {
                return BasicBoardEvaluation(board);
            }
            int value = negINF;
            auto moves = board.GenerateAllLegalMoves();
            for (const auto &m: moves) {
                Board nextState = board;
                nextState.makeMove(m);
                int childVal = -negamax(nextState, depth - 1, initDepth);
                if (childVal > value) {
                    value = childVal;
                    if (depth == initDepth)
                        bestMove = m;
                }
            }
            return value;
        }

        int AlphaBetaNegamax(const Board &board, int depth, int initDepth, int alpha, int beta) {
            if (depth == 0 || board.isGameOver() != 'n') {
                return boardEvaluation(board);
            }
            int value = negINF;

            auto moves = board.GenerateAllLegalMoves();
            for (const auto &m: moves) {
                Board nextState = board;
                nextState.makeMove(m);
                int childVal = -AlphaBetaNegamax(nextState, depth - 1, initDepth, -beta, -alpha);
                if (childVal > value) {
                    value = childVal;
                    if (depth == initDepth) {
                        bestMove = m;
                    }
                }
                alpha = std::max(alpha, value);
                if (alpha >= beta) {
                    break;
                }
            }
            return value;
        }

        int AlphaBetaNegamaxWithTT(const Board &board, int depth, int initDepth, int alpha, int beta) {
            int origAlpha = alpha;
            HashEntry *entry = GetTranspositionEntry(board);
            int ttDepth = -1;
            std::string ttMove, localBestMove = bestMove;
            FLAG ttFlag;
            int ttScore;
            char toPlay = board.getToPlay();
            if (entry != nullptr) {
                ttDepth = entry->depth;
                ttMove = entry->move;
                ttFlag = entry->flag;
                ttScore = entry->score;
                if (entry->toPlay != toPlay) {
                    ttScore *= -1;
                }
                if (ttDepth >= depth && ttDepth != initDepth) {

                    if (ttFlag == EXACT) {
                        return ttScore;
                    } else if (ttFlag == LOWERBOUND) {
                        alpha = std::max(alpha, ttScore);
                    } else if (ttFlag == UPPERBOUND) {
                        beta = std::min(beta, ttScore);
                    }
                    if (alpha >= beta) {
                        return ttScore;
                    }
                }
            }

            if (depth == 0 || board.isGameOver() != 'n') {
                return boardEvaluation(board);
            }
            int value;

            value = negINF;
            auto moves = board.GenerateAllLegalMoves();
            for (const auto &m: moves) {
                alpha = std::max(value, alpha);
                auto nextState = board;
                nextState.makeMove(m);
                int childVal = -AlphaBetaNegamaxWithTT(nextState, depth - 1, initDepth, -beta, -alpha);
                localBestMove = m;
                if (childVal > value) {
                    value = childVal;
                    if (depth == initDepth) {
                        bestMove = m;
                    }
                }
                alpha = std::max(alpha, value);
                if (alpha >= beta) {
                    break;
                }
            }

            FLAG flag = EXACT;
            if (value <= origAlpha)
                flag = UPPERBOUND;
            else if (value >= beta)
                flag = LOWERBOUND;

            auto newEntry = new HashEntry(depth, value, flag, localBestMove, toPlay);
            storeEntry(board, newEntry);

            return value;
        }

        int PVS(const Board &board, int depth, int initDepth, int alpha, int beta) {
            if (depth == 0 || board.isGameOver() != 'n') {
                return boardEvaluation(board);
            }
            int value;
            auto moves = board.GenerateAllLegalMoves();
            for (int i = 0; i < moves.size(); ++i) {
                std::string m = moves[i];
                Board nextState = board;
                nextState.makeMove(m);
                if (i == 0) {
                    value = -PVS(nextState, depth - 1, initDepth, -beta, -alpha);

                } else {
                    value = -PVS(nextState, depth - 1, initDepth, -alpha - 1, -alpha);
                    if (alpha < value < beta) {
                        value = -PVS(nextState, depth - 1, initDepth, -beta, -value);
                    }

                }
                if (depth == initDepth)
                    bestMove = m;
                alpha = std::max(alpha, value);
                if (alpha >= beta)
                    break;
            }
            return value;
        }

        std::string getBestMoveNegamax(const std::string &fen, int depth, int Mode) {
            auto board = Board(fen);
            int score = 0;
            if (Mode == STANDARD)
                score = negamax(board, depth, depth);
            else if (Mode == ALPHA_BETA)
                score = AlphaBetaNegamax(board, depth, depth, negINF, INF);
            else if (Mode == TRANSPOSITION_TABLES)
                score = AlphaBetaNegamaxWithTT(board, depth, depth, negINF, INF);
            if (Mode == TRANSPOSITION_TABLES)
                TranspositionTable.clear();
            return bestMove;
        }

        std::vector<std::string>
        iterativeDeepeningOutput(const std::string &fen, int maxDepth, int Mode, int timeLimitSeconds) {
            int searchDepth = 0;
            startTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch());
            auto timeLimit = seconds_to_duration(timeLimitSeconds);
            std::chrono::duration<double> totalTime = seconds_to_duration(0);
            std::vector<std::string> stringArray;

            for (int i = 1; i <= maxDepth; ++i) {

                Board board = Board(fen);
                int score = 0;
                auto start = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch());

                if (Mode == STANDARD)
                    score = itNegamax(board, i, i, timeLimit);
                else if (Mode == ALPHA_BETA)
                    score = itAlphaBetaNegamax(board, i, i, negINF, INF, timeLimit);
                else if (Mode == TRANSPOSITION_TABLES)
                    score = itAlphaBetaNegamaxWithTT(board, i, i, negINF, INF, timeLimit);
                auto end = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch());
                auto timeTaken = end - start;

                totalTime += timeTaken;

                if (LastCompletedDepth != i) {
                    break;
                }

                stringArray.push_back("Depth: " + std::to_string(i) + "\n");
                stringArray.push_back("Time: " + std::to_string(timeTaken.count()) + "\n");
                stringArray.push_back("Best Move: " + bestMove + "\n");
                stringArray.push_back("Score: " + std::to_string(score) + "\n");
                stringArray.push_back("Last Completed Depth: " + std::to_string(LastCompletedDepth) + "\n\n");

                searchDepth = i;
                DepthMoves[i] = std::make_pair(score, bestMove);
            }


            for (const auto &i: DepthMoves) {
                stringArray.push_back(
                        "Depth: " + std::to_string(i.first) + " Score: " + std::to_string(i.second.first) + " Move: " +
                        i.second.second + "\n");
            }
            stringArray.push_back("Total Time: " + std::to_string(totalTime.count()) + "\n");
            if (Mode == TRANSPOSITION_TABLES)
                TranspositionTable.clear();
            return stringArray;
        }

        std::string
        iterativeDeepening(const std::string &fen, int maxDepth, int Mode, int timeLimitSeconds) {
            int searchDepth = 0;
            startTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch());
            auto timeLimit = seconds_to_duration(timeLimitSeconds);
            std::chrono::duration<double> totalTime = seconds_to_duration(0);
            for (int i = 1; i <= maxDepth; ++i) {
                Board board = Board(fen);
                int score = 0;
                auto start = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch());

                if (Mode == STANDARD)
                    score = itNegamax(board, i, i, timeLimit);
                else if (Mode == ALPHA_BETA)
                    score = itAlphaBetaNegamax(board, i, i, negINF, INF, timeLimit);
                else if (Mode == TRANSPOSITION_TABLES)
                    score = itAlphaBetaNegamaxWithTT(board, i, i, negINF, INF, timeLimit);
                auto end = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch());
                auto timeTaken = end - start;

                totalTime += timeTaken;

                if (LastCompletedDepth != i) {
                    break;
                }

                DepthMoves[i] = std::make_pair(score, bestMove);
            }
            if (Mode == TRANSPOSITION_TABLES)
                TranspositionTable.clear();
            return DepthMoves[LastCompletedDepth].second;
        }


        int itNegamax(const Board &board, int depth, int initDepth, std::chrono::duration<double> timeLimit) {
            auto currTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch());
            if (currTime - startTime >= timeLimit) {
                LastCompletedDepth = initDepth - 1;
                return -INF;
            }
            if (depth <= 0 || board.isGameOver() != 'n') {
                LastCompletedDepth = initDepth;
                return BasicBoardEvaluation(board);
            }
            int value = negINF;
            auto moves = board.GenerateAllLegalMoves();

            for (const auto &m: moves) {
                Board nextState = board;
                nextState.makeMove(m);
                int childVal = -itNegamax(nextState, depth - 1, initDepth, timeLimit);
                if (childVal > value) {
                    value = childVal;
                    if (depth == initDepth)
                        bestMove = m;
                }
            }
            return value;
        }

        int itAlphaBetaNegamax(const Board &board, int depth, int initDepth, int alpha, int beta,
                               std::chrono::duration<double> timeLimit) {
            auto currTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch());
            if (currTime - startTime >= timeLimit) {
                LastCompletedDepth = initDepth - 1;
                return -INF;
            }
            if (depth == 0 || board.isGameOver() != 'n') {
                LastCompletedDepth = initDepth;
                return boardEvaluation(board);
            }

            int value = negINF;
            auto moves = board.GenerateAllLegalMoves();
            for (const auto &m: moves) {
                Board nextState = board;
                nextState.makeMove(m);
                int childVal = -itAlphaBetaNegamax(nextState, depth - 1, initDepth, -beta, -alpha, timeLimit);
                if (childVal > value) {
                    value = childVal;
                    if (depth == initDepth) {
                        bestMove = m;
                    }
                }
                alpha = std::max(alpha, value);
                if (alpha >= beta) {
                    break;
                }
            }

            return value;
        }

        int itAlphaBetaNegamaxWithTT(const Board &board, int depth, int initDepth, int alpha, int beta,
                                     std::chrono::duration<double> timeLimit) {
            auto currTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch());
            if (currTime - startTime >= timeLimit) {
                LastCompletedDepth = initDepth - 1;
                return -INF;
            }

            int origAlpha = alpha;
            HashEntry *entry = GetTranspositionEntry(board);
            int ttDepth = -1;
            std::string ttMove, localBestMove = bestMove;
            FLAG ttFlag;
            int ttScore;
            char toPlay = board.getToPlay();
            if (entry != nullptr) {
                ttDepth = entry->depth;
                ttMove = entry->move;
                ttFlag = entry->flag;
                ttScore = entry->score;
                if (entry->toPlay != toPlay) {
                    ttScore *= -1;
                }
                if (ttDepth >= depth && ttDepth != initDepth) {

                    if (ttFlag == EXACT) {
                        return ttScore;
                    } else if (ttFlag == LOWERBOUND) {
                        alpha = std::max(alpha, ttScore);
                    } else if (ttFlag == UPPERBOUND) {
                        beta = std::min(beta, ttScore);
                    }
                    if (alpha >= beta) {
                        return ttScore;
                    }
                }

            }

            if (depth == 0 || board.isGameOver() != 'n') {
                LastCompletedDepth = initDepth;
                return boardEvaluation(board);
            }
            int value;

            value = negINF;
            auto moves = board.GenerateAllLegalMoves();

            for (const auto &m: moves) {
                alpha = std::max(value, alpha);
                auto nextState = board;
                nextState.makeMove(m);
                int childVal = -itAlphaBetaNegamaxWithTT(nextState, depth - 1, initDepth, -beta, -alpha, timeLimit);
                localBestMove = m;
                if (childVal > value) {
                    value = childVal;
                    if (depth == initDepth) {
                        bestMove = m;
                    }
                }
                alpha = std::max(alpha, value);
                if (alpha >= beta) {
                    break;
                }
            }

            FLAG flag = EXACT;
            if (value <= origAlpha)
                flag = UPPERBOUND;
            else if (value >= beta)
                flag = LOWERBOUND;

            auto newEntry = new HashEntry(depth, value, flag, localBestMove, toPlay);
            storeEntry(board, newEntry);


            return value;
        }

        std::string getBestMove() {
            return bestMove;
        }
    };


    Board
    playBoardNegamaxGame(std::string fen, int depth, int mode, char toPlayAs, int oppMode, int oppDepth = 2,
                         bool ID = false, bool oppID = false,
                         int myTime = 10, int oppTime = 10) {
        int moveCount = 0;
        auto board = Board(fen);
        auto myMinimax = new MiniMax;
        auto oppMinimax = new MiniMax;
        std::vector<std::string> LegalMoves;

        char gameOver = board.isGameOver();
        std::cout << fen << std::endl;
        int moveIndex = 0;
        while (gameOver == 'n') {
            std::string move;
            if (findToPlay(fen) == toPlayAs) {
                if (ID) {
                    move = myMinimax->iterativeDeepening(fen, 50, mode, myTime);
                } else {
                    move = myMinimax->getBestMoveNegamax(fen, depth, mode);
                }
            } else {
                if (oppMode == ORDERED) {
                    auto x = board.GenerateAllLegalMoves();
                    if (moveIndex >= x.size()) {
                        moveIndex = 0;
                    }
                    move = x[moveIndex];
                    moveIndex++;
                } else if (oppMode == RANDOM) {
                    auto x = board.GenerateAllLegalMoves();
                    move = getRandomMove(x);
                } else {
                    if (ID)
                        move = oppMinimax->iterativeDeepening(fen, 50, oppMode, oppTime);
                    else
                        move = oppMinimax->getBestMoveNegamax(fen, oppDepth, oppMode);
                }
            }
            if (findToPlay(fen) == 'b') {
                moveCount++;
            }
            board.makeMove(move);
            fen = board.getFen();
            std::cout << move << " -> " << fen << std::endl;
            gameOver = board.isGameOver();
            if (moveCount == 100) {
                gameOver = 'd';
            }
        }
        std::cout << "Winner is: " << gameOver << std::endl;
        return board;
    }

}


using namespace Congo;

void FullGameTime(int depth) {
    std::ofstream output("FullGameResults.txt", std::ios::app);

    output << "Depth: " << depth << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 0";

    if (depth < 5) {
        auto start1 = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch());
        output << "Standard" << std::endl;
        Board STD = playBoardNegamaxGame(startFen, depth, STANDARD, 'b', ORDERED);
        output << "Winner: " << STD.isGameOver() << std::endl;
        output << "Moves: " << STD.getTurnNum() << std::endl;
        auto end1 = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch());
        auto diff1 = end1 - start1;
        output << "Time: " << diff1.count() << std::endl << std::endl;
    }

    auto start2 = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch());
    output << "Alpha Beta" << std::endl;
    Board AB = playBoardNegamaxGame(startFen, depth, ALPHA_BETA, 'b', ORDERED);
    output << "Winner: " << AB.isGameOver() << std::endl;
    output << "Moves: " << AB.getTurnNum() << std::endl;
    auto end2 = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch());
    auto diff2 = end2 - start2;
    output << "Time: " << diff2.count() << std::endl << std::endl;

    auto start3 = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch());
    output << "Transposition Table" << std::endl;
    Board TT = playBoardNegamaxGame(startFen, depth, TRANSPOSITION_TABLES, 'b', ORDERED);
    output << "Winner: " << TT.isGameOver() << std::endl;
    output << "Moves: " << TT.getTurnNum() << std::endl;
    auto end3 = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch());
    auto diff3 = end3 - start3;
    output << "Time: " << diff3.count() << std::endl;

    output << std::endl << std::endl;
    output.close();
}

void ID_Output(int time, int Mode, const std::string &filename, const std::string &fen) {
    std::ofstream output(filename);
    auto mm = MiniMax();
    int maxDepth = 20;
    auto Arr = mm.iterativeDeepeningOutput(fen, maxDepth, Mode, time);
    for (const auto &i: Arr) {
        output << i;
    }
    output.close();
}

void saveIDResults(int time, const std::string &fen) {
    ID_Output(time, STANDARD, "ID_Standard.txt", fen);
    ID_Output(time, ALPHA_BETA, "ID_AlphaBeta.txt", fen);
    ID_Output(time, TRANSPOSITION_TABLES, "ID_Transposition.txt", fen);
}

void Negamax_Output(int depth, int Mode, const std::string &filename, const std::string &fen) {
    std::ofstream output(filename, std::ios::app);

    MiniMax mm = MiniMax();
    Board board = Board(fen);
    int score = mm.INF;
    auto start = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch());
    std::string modeName;

    if (Mode == STANDARD) {
        modeName = "Standard";
        score = mm.negamax(board, depth, depth);
    } else if (Mode == ALPHA_BETA) {
        modeName = "Alpha Beta";
        score = mm.AlphaBetaNegamax(board, depth, depth, mm.negINF, mm.INF);
    } else if (Mode == TRANSPOSITION_TABLES) {
        modeName = "Transposition Table";
        score = mm.AlphaBetaNegamaxWithTT(board, depth, depth, mm.negINF, mm.INF);
    }

    auto end = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch());
    auto timeTaken = end - start;
    output << "Mode: " << modeName << std::endl;
    output << "Depth: " << depth << std::endl;
    output << "Score: " << score << std::endl;
    output << "Move: " << mm.getBestMove() << std::endl;
    output << "Time: " << timeTaken.count() << std::endl;
    output << std::endl << std::endl;
    output.close();
}

void saveNegamaxResults(int depth, const std::string &fen) {
    std::string filename = "NegamaxResults.txt";
    //remove("NegamaxResults.txt");
    if (depth < 5)Negamax_Output(depth, STANDARD, filename, fen);
    Negamax_Output(depth, ALPHA_BETA, filename, fen);
    Negamax_Output(depth, TRANSPOSITION_TABLES, filename, fen);
}


/*
 * STD: Standard Minimax
 * AB: Alpha Beta
 * TT: Transposition Tables
 * ID: Iterative Deepening
 *
 * USE NegamaxResults.txt generated from SaveNegamaxResults() to determine Depth
 * MAX STD DEPTH CAN BE 4 else it takes way to0 long
 *
 *
 * ID Takes in Time x (in seconds), depth is disregarded here
 * IDK What Time to set it yet. It will take x seconds to complete one move.
 * Take note at the depth you set it yet, makeFile sure its consistent.
 *
 * STD_VS_AB =  toPlayAs vs Opp i.e. toPlayAs will be STD and Opp will be AB (rest follow same convention)
 *
 * use the X_Vs_Ordered to time full games.
 * */


int STD_Depth = 2;
int AB_Depth = 4;
int TT_Depth = 5;
int ID_TIME = 5;

void AB_Vs_STD(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << " Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = AB_Depth;
    int myMode = ALPHA_BETA;
    int myTime = 10;
    bool myID = false;

    int oppDepth = STD_Depth;
    int oppMode = STANDARD;
    int oppTime = 10;
    bool oppID = false;

    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode, oppDepth, myID, oppID, myTime, oppTime);
}

void TT_Vs_STD(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << " Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = TT_Depth;
    int myMode = TRANSPOSITION_TABLES;
    int myTime = 10;
    bool myID = false;

    int oppDepth = STD_Depth;
    int oppMode = STANDARD;
    int oppTime = 10;
    bool oppID = false;

    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode, oppDepth, myID, oppID, myTime, oppTime);

}

void TT_Vs_AB(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << " Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = TT_Depth;
    int myMode = TRANSPOSITION_TABLES;
    int myTime = ID_TIME;
    bool myID = false;

    int oppDepth = AB_Depth;
    int oppMode = ALPHA_BETA;
    int oppTime = ID_TIME;
    bool oppID = false;

    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode, oppDepth, myID, oppID, myTime, oppTime);

}

void ID_TT_Vs_STD(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << "Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = TT_Depth;
    int myMode = TRANSPOSITION_TABLES;
    int myTime = ID_TIME;
    bool myID = true;

    int oppDepth = STD_Depth;
    int oppMode = STANDARD;
    int oppTime = ID_TIME;
    bool oppID = false;

    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode, oppDepth, myID, oppID, myTime, oppTime);
}

void ID_TT_Vs_AB(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << "Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = TT_Depth;
    int myMode = TRANSPOSITION_TABLES;
    int myTime = ID_TIME;
    bool myID = true;

    int oppDepth = AB_Depth;
    int oppMode = ALPHA_BETA;
    int oppTime = ID_TIME;
    bool oppID = false;

    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode, oppDepth, myID, oppID, myTime, oppTime);
}

void ID_TT_Vs_TT(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << "Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = TT_Depth;
    int myMode = TRANSPOSITION_TABLES;
    int myTime = ID_TIME;
    bool myID = true;

    int oppDepth = TT_Depth;
    int oppMode = TRANSPOSITION_TABLES;
    int oppTime = ID_TIME;
    bool oppID = false;

    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode, oppDepth, myID, oppID, myTime, oppTime);
}

void ID_TT_Vs_ID_STD(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << "Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = TT_Depth;
    int myMode = TRANSPOSITION_TABLES;
    int myTime = ID_TIME;
    bool myID = true;

    int oppDepth = STD_Depth;
    int oppMode = STANDARD;
    int oppTime = ID_TIME;
    bool oppID = true;

    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode, oppDepth, myID, oppID, myTime, oppTime);
}

void ID_TT_Vs_ID_AB(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << "Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = TT_Depth;
    int myMode = TRANSPOSITION_TABLES;
    int myTime = ID_TIME;
    bool myID = true;

    int oppDepth = STD_Depth;
    int oppMode = ALPHA_BETA;
    int oppTime = ID_TIME;
    bool oppID = true;

    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode, oppDepth, myID, oppID, myTime, oppTime);
}

void ID_STD_Vs_STD(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << "Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = STD_Depth;
    int myMode = STANDARD;
    int myTime = ID_TIME;
    bool myID = true;

    int oppDepth = STD_Depth;
    int oppMode = STANDARD;
    int oppTime = ID_TIME;
    bool oppID = false;

    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode, oppDepth, myID, oppID, myTime, oppTime);
}

void ID_STD_Vs_AB(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << "Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = STD_Depth;
    int myMode = STANDARD;
    int myTime = ID_TIME;
    bool myID = true;

    int oppDepth = AB_Depth;
    int oppMode = ALPHA_BETA;
    int oppTime = ID_TIME;
    bool oppID = false;

    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode, oppDepth, myID, oppID, myTime, oppTime);
}

void ID_STD_Vs_TT(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << "Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = STD_Depth;
    int myMode = STANDARD;
    int myTime = ID_TIME;
    bool myID = true;

    int oppDepth = TT_Depth;
    int oppMode = TRANSPOSITION_TABLES;
    int oppTime = ID_TIME;
    bool oppID = false;

    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode, oppDepth, myID, oppID, myTime, oppTime);
}

void ID_AB_Vs_STD(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << "Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = AB_Depth;
    int myMode = ALPHA_BETA;
    int myTime = ID_TIME;
    bool myID = true;

    int oppDepth = STD_Depth;
    int oppMode = STANDARD;
    int oppTime = ID_TIME;
    bool oppID = false;

    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode, oppDepth, myID, oppID, myTime, oppTime);
}

void ID_AB_Vs_AB(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << "Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = AB_Depth;
    int myMode = ALPHA_BETA;
    int myTime = ID_TIME;
    bool myID = true;

    int oppDepth = AB_Depth;
    int oppMode = ALPHA_BETA;
    int oppTime = ID_TIME;
    bool oppID = false;

    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode, oppDepth, myID, oppID, myTime, oppTime);
}

void ID_AB_Vs_TT(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << "Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = AB_Depth;
    int myMode = ALPHA_BETA;
    int myTime = ID_TIME;
    bool myID = true;

    int oppDepth = TT_Depth;
    int oppMode = TRANSPOSITION_TABLES;
    int oppTime = ID_TIME;
    bool oppID = false;

    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode, oppDepth, myID, oppID, myTime, oppTime);
}

void ID_AB_Vs_ID_STD(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << "Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = AB_Depth;
    int myMode = ALPHA_BETA;
    int myTime = ID_TIME;
    bool myID = true;

    int oppDepth = STD_Depth;
    int oppMode = STANDARD;
    int oppTime = ID_TIME;
    bool oppID = true;

    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode, oppDepth, myID, oppID, myTime, oppTime);
}


//Control Environment
void STD_Vs_Ordered(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << "Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = STD_Depth;
    int myMode = STANDARD;

    int oppDepth = STD_Depth;
    int oppMode = ORDERED;
    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode);
}

void AB_Vs_Ordered(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << "Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = AB_Depth;
    int myMode = ALPHA_BETA;

    int oppDepth = STD_Depth;
    int oppMode = ORDERED;
    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode);
}

void TT_Vs_Ordered(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << "Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = TT_Depth;
    int myMode = TRANSPOSITION_TABLES;

    int oppDepth = STD_Depth;
    int oppMode = ORDERED;
    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode);
}

void ID_STD_Vs_Ordered(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << "Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = STD_Depth;
    int myMode = STANDARD;
    int myTime = ID_TIME;
    bool myID = true;

    int oppDepth = STD_Depth;
    int oppMode = ORDERED;
    int oppTime = ID_TIME;
    bool oppID = false;
    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode, oppDepth, myID, oppID, myTime, oppTime);
}

void ID_AB_Vs_Ordered(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << "Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = AB_Depth;
    int myMode = ALPHA_BETA;
    int myTime = ID_TIME;
    bool myID = true;

    int oppDepth = STD_Depth;
    int oppMode = ORDERED;
    int oppTime = ID_TIME;
    bool oppID = false;
    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode, oppDepth, myID, oppID, myTime, oppTime);
}

void ID_TT_Vs_Ordered(char toPlayAs) {
    std::cout << __PRETTY_FUNCTION__ << "Playing As: " << toPlayAs << std::endl;
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 4";
    int depth = TT_Depth;
    int myMode = TRANSPOSITION_TABLES;
    int myTime = ID_TIME;
    bool myID = true;

    int oppDepth = STD_Depth;
    int oppMode = ORDERED;
    int oppTime = ID_TIME;
    bool oppID = false;
    playBoardNegamaxGame(startFen, depth, myMode, toPlayAs, oppMode, oppDepth, myID, oppID, myTime, oppTime);
}

void API() {
    char gameOver = 'n';
    Board *board = nullptr;
    MiniMax MM = MiniMax();
    while (gameOver == 'n') {
        std::string input, positions;
        getline(std::cin, input);
        if (input == "newgame") {
            getline(std::cin, positions);
            std::regex target("position ");
            std::string replacement;
            positions = std::regex_replace(positions, target, replacement);
            board = new Board(positions);
        } else if (input.find("go ") != std::string::npos) {
            std::regex target("go ");
            std::string replacement;
            input = std::regex_replace(input, target, replacement);
            int time = std::stoi(input) - 3;
            std::string fen = board->getFen();
            std::string move = MM.iterativeDeepening(fen, 50, TRANSPOSITION_TABLES, time);
            board->makeMove(move);
            std::cout << move << std::endl;
        } else if (input.find("moves") != std::string::npos) {
            std::regex target("moves ");
            std::string replacement;
            std::string move = std::regex_replace(input, target, replacement);
            board->makeMove(move);
        } else if (input == "print") {
            board->printBoard();
        }
        gameOver = board->isGameOver();
    }


}


int main() {
    std::string startFen = "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 0";
    int depth = 3;
    int myMode = ALPHA_BETA;
    int myTime = ID_TIME;
    bool myID = true;

    int oppDepth = TT_Depth;
    int oppMode = RANDOM;
    int oppTime = ID_TIME;
    bool oppID = false;

    playBoardNegamaxGame(startFen, depth, myMode, 'w', oppMode, oppDepth, myID, oppID, myTime, oppTime);
}