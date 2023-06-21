// Game of Othello -- Example of main
// Universidad Simon Bolivar, 2012.
// Author: Blai Bonet
// Last Revision: 1/11/16
// Modified by: 

#include <iostream>
#include <limits>
#include "othello_cut.h" // won't work correctly until .h is fixed!
#include "utils.h"

#include <unordered_map>

using namespace std;

unsigned expanded = 0;
unsigned generated = 0;
int tt_threshold = 32; // threshold to save entries in TT
const int INF = 1000; // un valor grande para representar infinito


// Transposition table (it is not necessary to implement TT)
struct stored_info_t {
    int value_;
    int type_;
    enum { EXACT, LOWER, UPPER };
    stored_info_t(int value = -100, int type = LOWER) : value_(value), type_(type) { }
};

struct hash_function_t {
    size_t operator()(const state_t &state) const {
        return state.hash();
    }
};

class hash_table_t : public unordered_map<state_t, stored_info_t, hash_function_t> {
};

hash_table_t TTable[2];

//int maxmin(state_t state, int depth, bool use_tt);
//int minmax(state_t state, int depth, bool use_tt = false);
//int maxmin(state_t state, int depth, bool use_tt = false);
int scout(state_t state, int depth, int color, bool use_tt = false);
int negascout(state_t state, int depth, int alpha, int beta, int color, bool use_tt = false);

bool test(state_t state, int color, int score, bool conditional) {

    ++generated;
    if (state.terminal())
        return (conditional ? state.value() >= score : state.value() > score);

    ++expanded;
    auto moves = state.get_moves(color == 1);
    for (int i = 0; i < (int)moves.size(); ++i) {
        auto child = state.move(color == 1, moves[i]);
        if (color == 1 && test(child, -color, score, conditional))
            return true;
        if (color == -1 && !test(child, -color, score, conditional))
            return false;
    }

    if (moves.size() == 0) {
        if (color == 1 && test(state, -color, score, conditional))
            return true;
        if (color == -1 && !test(state, -color, score, conditional))
            return false;
    }

    return color == -1;
}

// Función de búsqueda Negamax
int negamax(state_t state, int depth, int color, bool use_tt = false) {
    ++generated;
    if (state.terminal())
        return color * state.value();

    int score = -INF;
    bool moved = false;
    for (int p : state.get_moves(color == 1)) {
        moved = true;
        score = std::max(
            score,
            -negamax(state.move(color == 1, p), depth - 1, -color, use_tt)
        );
    }

    if (!moved)
        score = -negamax(state, depth, -color, use_tt);

    ++expanded;
    return score;
}

//int negamax(state_t state, int depth, int alpha, int beta, int color, bool use_tt = false);
int negamax(state_t state, int depth, int alpha, int beta, int color, bool use_tt = false) {
    ++generated;
    if (state.terminal())
        return color * state.value();

    bool moved = false;
    for (int p : state.get_moves(color == 1)) {
        moved = true;
        int value = -negamax(state.move(color == 1, p), depth - 1, -beta, -alpha, -color, use_tt);
        alpha = std::max(alpha, value);
        if (alpha >= beta)
            break; // Poda alpha-beta
    }

    if (!moved)
        alpha = -negamax(state, depth, -beta, -alpha, -color, use_tt);

    ++expanded;
    return alpha;
}


/*int negamax(const state_t& state, int depth, int alpha, int beta, int color, bool use_tt) {
    if (depth == 0 || state.terminal()) {
        return color * state.value();
    }

    int mejorValor = std::numeric_limits<int>::min();

    for (const auto& child : state.children()) {
        ++generated;
        int valor = -negamax(child, depth - 1, -beta, -alpha, -color, use_tt);
        mejorValor = std::max(mejorValor, valor);
        alpha = std::max(alpha, valor);
        if (alpha >= beta) {
            break;  // Poda alfa-beta
        }
    }

    ++expanded;
    return mejorValor;
}

int Scout(const EstadoJuego& estado, int alpha, int beta, int depth, int color, bool use_tt) {
    if (depth == 0 || JuegoTerminado(estado)) {
        return color * EvaluarEstado(estado, color);
    }

    int mejorValor;
    bool firstChild = true;

    for (const auto& child : estado.children()) {
        ++generated;

        if (firstChild) {
            mejorValor = -Scout(child, -beta, -alpha, depth - 1, -color, use_tt);
            firstChild = false;
        } else {
            int value = -Scout(child, -alpha - 1, -alpha, depth - 1, -color, use_tt);
            if (value > alpha && value < beta) {
                mejorValor = -Scout(child, -beta, -value, depth - 1, -color, use_tt);
            } else {
                mejorValor = value;
            }
        }

        alpha = std::max(alpha, mejorValor);
        if (alpha >= beta) {
            break;  // Poda alfa-beta
        }
    }

    ++expanded;
    return mejorValor;
}

int Negascout(const EstadoJuego& estado, int profundidad, int alpha, int beta, int color, bool use_tt) {
    if (profundidad == 0 || JuegoTerminado(estado)) {
        return color * EvaluarEstado(estado, color);
    }

    int mejorValor;
    bool firstChild = true;

    for (const auto& child : estado.children()) {
        ++generated;

        if (firstChild) {
            mejorValor = -Negascout(child, profundidad - 1, -beta, -alpha, -color, use_tt);
            firstChild = false;
        } else {
            int value = -Negascout(child, profundidad - 1, -alpha - 1, -alpha, -color, use_tt);
            if (value > alpha && value < beta) {
                mejorValor = -Negascout(child, profundidad - 1, -beta, -value, -color, use_tt);
            } else {
                mejorValor = value;
            }
        }

        alpha = std::max(alpha, mejorValor);
        if (alpha >= beta) {
            break;  // Poda alfa-beta
        }
    }

    ++expanded;
    return mejorValor;
}


*/

int main(int argc, const char **argv) {
    state_t pv[128];
    int npv = 0;
    for( int i = 0; PV[i] != -1; ++i ) ++npv;

    int algorithm = 0;
    if( argc > 1 ) algorithm = atoi(argv[1]);
    bool use_tt = argc > 2;

    // Extract principal variation of the game
    state_t state;
    cout << "Extracting principal variation (PV) with " << npv << " plays ... " << flush;
    for( int i = 0; PV[i] != -1; ++i ) {
        bool player = i % 2 == 0; // black moves first!
        int pos = PV[i];
        pv[npv - i] = state;
        state = state.move(player, pos);
    }
    pv[0] = state;
    cout << "done!" << endl;

#if 0
    // print principal variation
    for( int i = 0; i <= npv; ++i )
        cout << pv[npv - i];
#endif

    // Print name of algorithm
    cout << "Algorithm: ";
    if( algorithm == 1 )
        cout << "Negamax (minmax version)";
    else if( algorithm == 2 )
        cout << "Negamax (alpha-beta version)";
    else if( algorithm == 3 )
        cout << "Scout";
    else if( algorithm == 4 )
        cout << "Negascout";
    cout << (use_tt ? " w/ transposition table" : "") << endl;

    // Run algorithm along PV (bacwards)
    cout << "Moving along PV:" << endl;
    for( int i = 0; i <= npv; ++i ) {
        //cout << pv[i];
        int value = 0;
        TTable[0].clear();
        TTable[1].clear();
        float start_time = Utils::read_time_in_seconds();
        expanded = 0;
        generated = 0;
        int color = i % 2 == 1 ? 1 : -1;

        try {
            if( algorithm == 1 ) {
                value = negamax(pv[i], 0, color, use_tt);
            } else if( algorithm == 2 ) {
                value = negamax(pv[i], 0, -200, 200, color, use_tt);
            } else if( algorithm == 3 ) {
                //value = scout(pv[i], 0, color, use_tt);
            } else if( algorithm == 4 ) {
                //value = negascout(pv[i], 0, -200, 200, color, use_tt);
            }
        } catch( const bad_alloc &e ) {
            cout << "size TT[0]: size=" << TTable[0].size() << ", #buckets=" << TTable[0].bucket_count() << endl;
            cout << "size TT[1]: size=" << TTable[1].size() << ", #buckets=" << TTable[1].bucket_count() << endl;
            use_tt = false;
        }

        float elapsed_time = Utils::read_time_in_seconds() - start_time;

        cout << npv + 1 - i << ". " << (color == 1 ? "Black" : "White") << " moves: "
             << "value=" << color * value
             << ", #expanded=" << expanded
             << ", #generated=" << generated
             << ", seconds=" << elapsed_time
             << ", #generated/second=" << generated/elapsed_time
             << endl;
    }

    return 0;
}


