#include "search.hpp"
#include "transposition.hpp"
#include "types.hpp"

void Search::quiescent() {
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

    bd.evalBuffer = bd.evaluator.getEval(bd.board.getCol());

    if (bd.evalBuffer >= bd.beta) {
        ds.up(-bd.beta);
        return;
    }
    if (bd.evalBuffer > bd.alpha) {
        bd.alpha = bd.evalBuffer;
    }

    bd.endMove = bd.board.getCaptureMoves(bd.moves);
    bd.curMove = bd.moves;

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
        quiescent();
        if (bd.evalBuffer >= bd.beta) {
            ds.up(-bd.beta);
            return;
        }
        if (bd.evalBuffer > bd.alpha) {
            bd.alpha = bd.evalBuffer;
        }
        ++bd.curMove;
    }
    ds.up(-bd.alpha);
    return;
}

void Search::calculate(uint32_t depth) {
    ++vis;
    bool foundPV = false;
    EvalType et = ALPHA;
    BoardData &bd = ds.getTopBD();
    if (ds.getRepetitions(bd.getZobrist()) > 2) {
        ds.up(0);
        return;
    }
    if ((bd.evalBuffer = tt.probe(bd.getZobrist(), depth, bd.alpha, bd.beta)) != INVALID_TABLE_VALUE) {
        ds.up(-bd.evalBuffer);
        return;
    }
    if (!depth) {
        ++bd.isStart;
        quiescent();
        tt.add(bd.getZobrist(), bd.evalBuffer, depth, EXACT);
        ds.up(-bd.evalBuffer);
        return;
    }
    bd.endMove = bd.board.getMoves(bd.moves);
    bd.curMove = bd.moves;

    if (bd.curMove == bd.endMove) {
        bd.evalBuffer = bd.board.isChecked() ? MATED : 0;
        tt.add(bd.getZobrist(), bd.evalBuffer, depth, EXACT);
        ds.up(-bd.evalBuffer);
        return;
    }

    // sort
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
            calculate(depth - 1);
            if (bd.evalBuffer > bd.alpha && bd.evalBuffer < bd.beta) {
                ds.down(*bd.curMove);
                calculate(depth - 1);
            }
        } else {
            calculate(depth - 1);
        }
        if (bd.evalBuffer >= bd.beta) {
            tt.add(bd.getZobrist(), bd.beta, depth, BETA);
            ds.up(-bd.beta);
            return;
        }
        if (bd.evalBuffer > bd.alpha) {
            et = EXACT;
            bd.alpha = bd.evalBuffer;
            // foundPV = true;
        }
        ++bd.curMove;
    }

    tt.add(bd.getZobrist(), bd.alpha, depth, et);
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
        if (remainingTime < 3) maxDepth = 4;
        move = ds.getCurBD().board.parseStrToMove(moveStr);
    } else {

        bd.endMove = bd.board.getMoves(bd.moves);
        bd.curMove = bd.moves;

        // sort
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
            calculate(maxDepth);
            if (bd.evalBuffer > bd.alpha) {
                bd.alpha = bd.evalBuffer;
                move = *bd.curMove;
            }
            ++bd.curMove;
        }

        std::cout << bd.board.parseMoveToStr(move) << std::endl;
        // std::cout << vis << std::endl;
    }
    ds.makeMove(move);
}

void Search::start() {
    while (true) {
        moveBySide<US>();
        moveBySide<NOT_US>();
    }
}
