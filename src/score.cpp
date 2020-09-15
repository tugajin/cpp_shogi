
// includes

#include "common.hpp"
#include "libmy.hpp"
#include "score.hpp"
#include "search.hpp" // for Ply_Max

namespace score {

// functions

bool is_ok(int sc) {
   return sc >= -INF && sc <= +INF;
}

Score win(Ply ply) {
   assert(ply >= 0 && ply <= PLY_MAX + 1);
   return Score(+INF) - Score(ply);
}

Score loss(Ply ply) {
   assert(ply >= 0 && ply <= PLY_MAX + 2);
   return Score(-INF) + Score(ply);
}

Score to_tt(Score sc, Ply ply) {

   assert(is_ok(sc));
   assert(ply >= 0 && ply <= PLY_MAX);

   if (is_win(sc)) {
      sc += Score(ply);
      assert(sc <= +INF);
   } else if (is_loss(sc)) {
      sc -= Score(ply);
      assert(sc >= -INF);
   }

   return sc;
}

Score from_tt(Score sc, Ply ply) {

   assert(is_ok(sc));
   assert(ply >= 0 && ply <= PLY_MAX);

   if (is_win(sc)) {
      sc -= Score(ply);
      assert(is_win(sc));
   } else if (is_loss(sc)) {
      sc += Score(ply);
      assert(is_loss(sc));
   }

   return sc;
}

Score clamp(Score sc) {

   if (is_win(sc)) {
      sc = Score(+EVAL_INF);
   } else if (is_loss(sc)) {
      sc = Score(-EVAL_INF);
   }

   assert(is_eval(sc));
   return sc;
}

Score add_safe(Score sc, Score inc) {

   if (is_eval(sc)) {
      return clamp(sc + inc);
   } else {
      return sc;
   }
}

bool is_win_loss(Score sc) {
   return std::abs(sc) > EVAL_INF;
}

bool is_win(Score sc) {
   return sc > +EVAL_INF;
}

bool is_loss(Score sc) {
   return sc < -EVAL_INF;
}

bool is_eval(Score sc) {
   return std::abs(sc) <= EVAL_INF;
}

int ply(Score sc) {
   assert(is_win_loss(sc));
   return INF - std::abs(sc);
}

}
