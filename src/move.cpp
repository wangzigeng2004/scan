
// includes

#include <cctype>
#include <string>

#include "bit.hpp"
#include "common.hpp"
#include "libmy.hpp"
#include "list.hpp"
#include "move.hpp"
#include "move_gen.hpp"
#include "pos.hpp"
#include "util.hpp"

namespace move {

// prototypes

static Bit amb (const List & list, Square from, Square to);

// functions

Move make(Square from, Square to, Bit captured) {

   assert(bit::is_incl(captured, bit::Inner));
   assert(!bit::has(captured, from));
   assert(!bit::has(captured, to));

   return Move((uint64(from) << 6) | (uint64(to) << 0) | (uint64(captured) << 5));
}

bool is_capture(Move mv, const Pos & /* pos */) {
   return captured(mv) != 0;
}

bool is_promotion(Move mv, const Pos & pos) {
   return is_man(mv, pos) && square_is_promotion(move::to(mv), pos.turn());
}

bool is_man(Move mv, const Pos & pos) {
   return bit::has(pos.man(), move::from(mv));
}

bool is_conversion(Move mv, const Pos & pos) {
   return is_man(mv, pos) || is_capture(mv, pos);
}

bool is_legal(Move mv, const Pos & pos) {
   List list;
   gen_moves(list, pos);
   return list::has(list, mv);
}

std::string to_string(Move mv, const Pos & pos) {

   assert(is_legal(mv, pos));

   Square from = move::from(mv);
   Square to   = move::to(mv);
   Bit    caps = captured(mv);

   std::string s;

   s += square_to_string(from);
   s += (caps != 0) ? "x" : "-";
   s += square_to_string(to);

   if (caps != 0) { // capture => test for ambiguity

      List list;
      gen_captures(list, pos);

      for (Bit b = caps & ~amb(list, from, to); b != 0; b = bit::rest(b)) {
         Square sq = bit::first(b);
         s += "x";
         s += square_to_string(sq);
      }
   }

   return s;
}

static Bit amb(const List & list, Square from, Square to) {

   Bit b = bit::Squares;

   for (int i = 0; i < list.size(); i++) {

      Move mv = list.move(i);

      if (move::from(mv) == from && move::to(mv) == to) {
         b &= captured(mv);
      }
   }

   assert(b != bit::Squares);
   return b;
}

std::string to_hub(Move mv) {

   Square from = move::from(mv);
   Square to   = move::to(mv);
   Bit    caps = captured(mv);

   std::string s;

   s += square_to_string(from);
   s += (caps != 0) ? "x" : "-";
   s += square_to_string(to);

   for (Bit b = caps; b != 0; b = bit::rest(b)) {
      Square sq = bit::first(b);
      s += "x";
      s += square_to_string(sq);
   }

   return s;
}

Move from_string(const std::string & s, const Pos & pos) {

   Scanner_Number scan(s);
   std::string token;

   token = scan.get_token();
   Square from = square_from_string(token);

   token = scan.get_token();
   if (token != "-" && token != "x") throw Bad_Input();
   bool is_cap = token == "x";

   token = scan.get_token();
   Square to = square_from_string(token);

   Bit caps = Bit(0);

   while (!scan.eos()) {

      token = scan.get_token();
      if (token != "x") throw Bad_Input();

      token = scan.get_token();
      bit::set(caps, square_from_string(token));
   }

   if (!is_cap && caps == 0) { // quiet move
      return make(from, to);
   }

   // capture => check legality

   List list;
   gen_captures(list, pos);

   Move move = None;
   int size = 0;

   for (int i = 0; i < list.size(); i++) {

      Move mv = list.move(i);

      if (move::from(mv) == from && move::to(mv) == to && bit::is_incl(caps, captured(mv))) {
         move = mv;
         size++;
      }
   }

   if (size > 1) return None; // ambiguous move

   return move;
}

Move from_hub(const std::string & s) {

   Scanner_Number scan(s);
   std::string token;

   token = scan.get_token();
   Square from = square_from_string(token);

   token = scan.get_token();
   if (token != "-" && token != "x") throw Bad_Input();

   token = scan.get_token();
   Square to = square_from_string(token);

   Bit caps = Bit(0);

   while (!scan.eos()) {

      token = scan.get_token();
      if (token != "x") throw Bad_Input();

      token = scan.get_token();
      bit::set(caps, square_from_string(token));
   }

   return make(from, to, caps);
}

}

