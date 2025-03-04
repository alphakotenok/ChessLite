#include "search.hpp"
#include "transposition.hpp"
#include "types.hpp"
#include <thread>

void Search::quiescent() {
    bool foundPV = false;
    BoardData &bd = ds.getTopBD();

    if (ds.getRepetitions(bd.getZobrist()) > 2) {
        ds.up(0);
        return;
    }

    if (bd.board.isChecked()) {
        ++bd.isStart;
        calculate(1);
        ds.up(-bd.evalBuffer);
        return;
    }

    bd.endMove = bd.board.getCaptureMoves(bd.moves);
    bd.curMove = bd.moves;

    if (bd.board.isFinished()) {
        bd.evalBuffer = bd.board.isChecked() ? MATED : 0;
        ds.up(-bd.evalBuffer);
        return;
    }

    bd.evalBuffer = bd.evaluator.getEval(bd.board.getCol());

    if (bd.evalBuffer >= bd.beta) {
        ds.up(-bd.beta);
        return;
    }
    if (bd.evalBuffer > bd.alpha) {
        bd.alpha = bd.evalBuffer;
    }

    for (int ptr = 0; ptr < bd.endMove - bd.moves; ++ptr) {
        moveStrength[ptr] = bd.evaluator.getMoveStrength(bd.moves[ptr], bd.board.getCol());
        for (int j = ptr; j; --j) {
            if (moveStrength[j] > moveStrength[j - 1]) {
                std::swap(moveStrength[j], moveStrength[j - 1]);
                std::swap(bd.moves[j], bd.moves[j - 1]);
            }
        }
    }

    while (bd.curMove != bd.endMove) {
        ds.down(*bd.curMove);
        if (foundPV) {
            ds.setAB(-bd.alpha - 1, -bd.alpha);
            quiescent();
            if (bd.evalBuffer > bd.alpha && bd.evalBuffer < bd.beta) {
                ds.down(*bd.curMove);
                quiescent();
            }
        } else {
            quiescent();
        }
        if (bd.evalBuffer >= bd.beta) {
            bd.bestMove = *bd.curMove;
            ds.up(-bd.beta);
            return;
        }
        if (bd.evalBuffer > bd.alpha) {
            bd.alpha = bd.evalBuffer;
            bd.bestMove = *bd.curMove;
            foundPV = true;
            bd.pvLine[0] = *bd.curMove;
            memcpy(bd.pvLine + 1, ds.getFuturePVLine(), ds.getFuturePVLen() * sizeof(Move));
            bd.pvLen = ds.getFuturePVLen() + 1;
        }
        ++bd.curMove;
    }
    ds.up(-bd.alpha);
    return;
}

void Search::calculate(uint32_t depth) {
    bool foundPV = false;
    EvalType et = ALPHA;
    BoardData &bd = ds.getTopBD();
    if (!bd.isStart) ++vis;
    if (ds.getRepetitions(bd.getZobrist()) > 2) {
        ds.up(0);
        return;
    }
    if ((bd.evalBuffer = tt.probe(bd.getZobrist(), depth, bd.alpha, bd.beta, bd.bestMove)) != INVALID_TABLE_VALUE) {
        ds.up(-bd.evalBuffer);
        return;
    }
    if (!depth) {
        ++bd.isStart;
        quiescent();
        tt.add(bd.getZobrist(), bd.evalBuffer, depth, EXACT, bd.bestMove);
        ds.up(-bd.evalBuffer);
        return;
    }
    bd.endMove = bd.board.getMoves(bd.moves);
    bd.curMove = bd.moves;

    if (bd.board.isFinished()) {
        bd.evalBuffer = bd.board.isChecked() ? MATED - depth : 0;
        tt.add(bd.getZobrist(), bd.evalBuffer, depth, EXACT, 0);
        ds.up(-bd.evalBuffer);
        return;
    }

    // sort
    for (int ptr = 0; ptr < bd.endMove - bd.moves; ++ptr) {
        if (pvUsed && PVPointer >= ds.getCurDepth() && bd.moves[ptr] == PVStack[ds.getCurDepth() - 1]) {
            moveStrength[ptr] = PERFECT_EVAL + 1;
        } else if (bd.moves[ptr] == bd.bestMove) {
            moveStrength[ptr] = PERFECT_EVAL;
        } else
            moveStrength[ptr] = bd.evaluator.getMoveStrength(bd.moves[ptr], bd.board.getCol());

        for (int j = ptr; j; --j) {
            if (moveStrength[j] > moveStrength[j - 1]) {
                std::swap(moveStrength[j], moveStrength[j - 1]);
                std::swap(bd.moves[j], bd.moves[j - 1]);
            }
        }
    }

    while (bd.curMove != bd.endMove) {
        if (pvUsed && *bd.curMove != PVStack[ds.getCurDepth() - 1]) {
            pvUsed = false;
        }
        ds.down(*bd.curMove);
        if (foundPV) {
            ds.setAB(-bd.alpha - 1, -bd.alpha);
            calculate(depth - 1);
            if (bd.evalBuffer > bd.alpha && bd.evalBuffer < bd.beta) {
                ds.down(*bd.curMove);
                calculate(depth - 1);
            }
        } else {
            calculate(depth - 1);
        }
        if (bd.evalBuffer >= bd.beta) {
            tt.add(bd.getZobrist(), bd.beta, depth, BETA, *bd.curMove);
            ds.up(-bd.beta);
            return;
        }
        if (bd.evalBuffer > bd.alpha) {
            et = EXACT;
            bd.alpha = bd.evalBuffer;
            bd.bestMove = *bd.curMove;
            foundPV = true;
            bd.pvLine[0] = *bd.curMove;
            memcpy(bd.pvLine + 1, ds.getFuturePVLine(), ds.getFuturePVLen() * sizeof(Move));
            bd.pvLen = ds.getFuturePVLen() + 1;
        }
        ++bd.curMove;
        if (workerHasToStop) {
            ds.up(-bd.alpha);
            return;
        }
    }

    // tt.add(bd.getZobrist(), bd.alpha, depth, et, bd.bestMove);
    ds.up(-bd.alpha);
    return;
}

template <Side side>
void Search::moveBySide() {
    vis = 0;
    Move move;
    BoardData &bd = ds.getTopBD();
    if constexpr (side) {
        float remainingTime;
        std::string moveStr;
        std::cin >> moveStr >> remainingTime;
        if (remainingTime < 3) moveTL = 80;
        if (remainingTime < 1) moveTL = 40;
        move = ds.getCurBD().board.parseStrToMove(moveStr);
    } else {
        PVPointer = 0;
        alpha = WORST_EVAL;
        beta = PERFECT_EVAL;
        workerHasToStop = false;
        std::thread timerThread(&Search::setWorkerTimer, this);
        for (int depth = 1;;) {
            pvUsed = 1;
            bd.isStart = 1;
            ds.setAB(alpha, beta);
            bd.pvLen = 0;

            calculate(depth);
            if (workerHasToStop) {
                break;
            }
            move = bd.bestMove;

            if (bd.evalBuffer <= alpha || bd.evalBuffer >= beta) {
                alpha = WORST_EVAL;
                beta = PERFECT_EVAL;
            } else {
                alpha = bd.evalBuffer - aspirationWindow;
                beta = bd.evalBuffer + aspirationWindow;
                ++depth;
            }

            // collect PV
            PVPointer = bd.pvLen;
            memcpy(PVStack, bd.pvLine, PVPointer * sizeof(Move));

            if (debug) {
                /*Board debb;
                bd.board.applyMove(bd.bestMove, &debb);
                std::cout << bd.board.parseMoveToStr(bd.bestMove) << std::endl;
                debb.print();
                bd.endMove = debb.getMoves(bd.moves);
                bd.curMove = bd.moves;
                for (int ptr = 0; ptr < bd.endMove - bd.moves; ++ptr) {
                    std::cout << debb.parseMoveToStr(bd.moves[ptr]) << ' ';
                }
                std::cout << std::endl;*/
                Board b1 = bd.board;
                Board b2;
                std::cout << "depth: " << depth << ' ';
                for (int i = 0; i < bd.pvLen; ++i) {
                    std::cout << b1.parseMoveToStr(PVStack[i]) << ' ';
                    b1.applyMove(PVStack[i], &b2);
                    b1 = b2;
                }
                std::cout << ", vis: " << vis << std::endl;
            }
        }
        timerThread.join();
        std::cout << bd.board.parseMoveToStr(move) << std::endl;
    }
    ds.makeMove(move);
}

void Search::start() {
    while (true) {
        moveBySide<US>();
        moveBySide<NOT_US>();
    }
}

void Search::setWorkerTimer() {
    std::this_thread::sleep_for(std::chrono::milliseconds(moveTL));
    workerHasToStop = true;
}
