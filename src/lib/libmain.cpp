#include "../cpp/nn.hpp"
#include "../cpp/pos.hpp"
#include "../cpp/sfen.hpp"
#include "../cpp/move.hpp"

#include <boost/python/numpy.hpp>

namespace py = boost::python;
namespace np = boost::python::numpy;

TeeStream Tee;

void lib_sfen_to_tensor(py::list &sfen_list, np::ndarray &numpy_feat) {
    const auto len = py::len(sfen_list);
    auto feat = new float[len][POS_END_SIZE][FILE_SIZE][RANK_SIZE]();
    for(auto batch_index = 0; batch_index < len; batch_index++) {
        std::string sfen = py::extract<std::string>(sfen_list[batch_index]);
        Pos pos = pos_from_sfen(sfen);
        make_feat(pos, &feat[batch_index][0]);
    }
    py::tuple shape = py::make_tuple(len, int(POS_END_SIZE),int(FILE_SIZE),int(RANK_SIZE));
    py::tuple stride = py::make_tuple(sizeof(float)*int(POS_END_SIZE*FILE_SIZE*RANK_SIZE),sizeof(float)*int(FILE_SIZE*RANK_SIZE),sizeof(float)*int(RANK_SIZE),sizeof(float));
    np::dtype dt = np::dtype::get_builtin<float>();
    np::ndarray np_feat = np::from_data(feat, dt, shape, stride, py::object());
    numpy_feat = np_feat.copy();
    
    delete[] feat;
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
    py::def("sfen_to_tensor", lib_sfen_to_tensor);
    py::def("move_to_index", lib_move_to_index);

}

