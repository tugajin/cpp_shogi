#include "../cpp/nn.hpp"
#include "../cpp/pos.hpp"
#include "../cpp/sfen.hpp"
#include "../cpp/move.hpp"

#include <boost/python/numpy.hpp>

namespace p = boost::python;
namespace np = boost::python::numpy;

TeeStream Tee;

// 遅いので使わない
void lib_sfen_to_tensor(boost::python::list &sfen_list, boost::python::numpy::ndarray &numpy_feat) {
    const auto len = boost::python::len(sfen_list);
    for(auto batch_index = 0; batch_index < len; batch_index++) {
        std::string sfen = boost::python::extract<std::string>(sfen_list[batch_index]);
        Pos pos = pos_from_sfen(sfen);
        float feat[POS_END_SIZE][FILE_SIZE][RANK_SIZE] = {};
        make_feat(pos, feat);
        for(auto feat_index = 0; feat_index < POS_END_SIZE; feat_index++) {
            for(auto file = 0; file < FILE_SIZE; ++file) {
                for(auto rank = 0; rank < RANK_SIZE; ++rank) {
                    numpy_feat[batch_index][feat_index][file][rank] = feat[feat_index][file][rank];
                }
            }
        }
    }
}

boost::python::numpy::ndarray lib_sfen_to_tensor2(boost::python::list &sfen_list) {
    const auto len = p::len(sfen_list);

    //配列がでかいので落ちるので雑に対応
    static float feat[512][POS_END_SIZE][FILE_SIZE][RANK_SIZE];
    float *p = &feat[0][0][0][0];
    for(auto i = 0; i < 512*POS_END_SIZE*FILE_SIZE*RANK_SIZE; i++) { *p++ = 0; }

    for(auto batch_index = 0; batch_index < len; batch_index++) {
        std::string sfen = p::extract<std::string>(sfen_list[batch_index]);
        Pos pos = pos_from_sfen(sfen);
        make_feat(pos, feat[batch_index]);
    }
    p::tuple shape = p::make_tuple(len, int(POS_END_SIZE),int(FILE_SIZE),int(RANK_SIZE));
    p::tuple stride = p::make_tuple(sizeof(float)*int(POS_END_SIZE*FILE_SIZE*RANK_SIZE),sizeof(float)*int(FILE_SIZE*RANK_SIZE),sizeof(float)*int(RANK_SIZE),sizeof(float));
    np::dtype dt = np::dtype::get_builtin<float>();
    np::ndarray np_feat = np::from_data(feat, dt, shape, stride, p::object());
    return np_feat;
}


int lib_move_to_index(std::string sfen_move, std::string sfen_turn) {
    const auto turn = sfen_turn == "b" ? BLACK : WHITE;
    auto from = Square(SQUARE_SIZE);
    auto to = Square(SQUARE_SIZE);
    auto pc = PieceNone;
    auto prom = false;
	if (std::isdigit(sfen_move[0])) { //move
		from = sq_from_string(sfen_move.substr(0, 2));
		to = sq_from_string(sfen_move.substr(2, 2));
		prom = sfen_move.find("+") != std::string::npos;
	}
	else { //drop
		pc = sfen_to_piece(sfen_move.substr(0, 1));
		to = sq_from_string(sfen_move.substr(2, 2));
	}
    return int(move_to_index(from, to, pc, prom, turn));    
}

BOOST_PYTHON_MODULE(cpp_lib) {

    bit::init();
    hash::init();
    pos::init();
    common::init();

    Py_Initialize();
    np::initialize();
    p::def("sfen_to_tensor", lib_sfen_to_tensor);
    p::def("sfen_to_tensor2", lib_sfen_to_tensor2);
    p::def("move_to_index", lib_move_to_index);

}

