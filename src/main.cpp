#include "board.hpp"
#include "extra.hpp"
#include "search.hpp"
#include "types.hpp"

int main() {

    init();
    //=============|DEBUG PLAYGROUND|=============//

    std::cout << sizeof(Search) << std::endl;

    //============================================//

    std::string board;
    getline(std::cin, board);
    Search s((Board(board)));
    s.start();

    return 0;
}
