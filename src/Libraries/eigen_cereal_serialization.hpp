//
// Created by Filippo Vicentini on 02/03/2018.
//

#ifndef SIMULATOR_EIGEN_CEREAL_SERIALIZATION_HPP
#define SIMULATOR_EIGEN_CEREAL_SERIALIZATION_HPP

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <cereal/cereal.hpp>
#include <cereal/types/complex.hpp>
//#include <cereal/archives/binary.hpp>

namespace cereal
{
    /*
     * Dense Scalar Matrix
     */
    template <class Archive, class _Scalar, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols> inline
    typename std::enable_if<traits::is_output_serializable<BinaryData<_Scalar>, Archive>::value, void>::type
    save(Archive & ar, Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols> const & m)
    {
        int32_t rows = m.rows(), cols = m.cols();
        ar(rows);
        ar(cols);
        ar(binary_data(m.data(), rows * cols * sizeof(_Scalar)));
    }

    template <class Archive, class _Scalar, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols> inline
    typename std::enable_if<traits::is_input_serializable<BinaryData<_Scalar>, Archive>::value, void>::type
    load(Archive & ar, Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols> & m)
    {
        int32_t rows, cols;
        ar(rows);
        ar(cols);
        m.resize(rows, cols);
        ar(binary_data(m.data(), static_cast<std::size_t>(rows * cols * sizeof(_Scalar))));
    }

    // Overloads for Textual archives
    template <class Archive, class _Scalar, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols> inline
    typename std::enable_if<!traits::is_output_serializable<BinaryData<_Scalar>, Archive>::value, void>::type
    save(Archive & ar, Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols> const & m)
    {
        int32_t rows = m.rows(), cols = m.cols();
        ar(rows);
        ar(cols);
        ar.saveBinaryValue(m.data(), rows * cols * sizeof(_Scalar));
        /*for (size_t i = 0; i < rows*cols; i++)
        {
            ar(m.data()[i]);
        }*/
    }

    template <class Archive, class _Scalar, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols> inline
    typename std::enable_if<!traits::is_input_serializable<BinaryData<_Scalar>, Archive>::value, void>::type
    load(Archive & ar, Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols> & m)
    {
        int32_t rows, cols;
        ar(rows);
        ar(cols);
        m.resize(rows, cols);
        ar.loadBinaryValue(m.data(), static_cast<std::size_t>(rows * cols * sizeof(_Scalar)));
        /*for (size_t i = 0; i < rows*cols; i++)
        {
            ar(m.data()[i]);
        }*/
    }


    /*
     * Eigen::Triplet (the elements of a sparse matrix)
     */
    template <class Archive, class _Scalar>
    void save(Archive & ar, const Eigen::Triplet<_Scalar> & m) {
        int32_t row = m.row(), col = m.col();
        ar(row);
        ar(col);
        ar(m.value());
    }
    template <class Archive, class _Scalar>
    void load(Archive & ar, Eigen::Triplet<_Scalar> & m) {
        int32_t row,col;
        _Scalar value;
        ar(row);
        ar(col);
        ar(value);
        m = Eigen::Triplet<_Scalar>(row,col,value);
    }

    /*
     * Sparse Matrix
     */
    template <class Archive, typename _Scalar, int _Options,typename _Index>
    void save(Archive & ar, const Eigen::SparseMatrix<_Scalar,_Options,_Index> & m) {
        int32_t innerSize=m.innerSize(), outerSize=m.outerSize();
        typedef typename Eigen::Triplet<_Scalar> Triplet;

        // Store the whole matrix as a vector of triplets
        std::vector<Triplet> triplets;
        for(int i=0; i < outerSize; ++i) {
            for(typename Eigen::SparseMatrix<_Scalar,_Options,_Index>::InnerIterator it(m,i); it; ++it) {
                triplets.push_back(Triplet(it.row(), it.col(), it.value()));
            }
        }
        ar(innerSize); ar(outerSize);
        ar(triplets);
    }

    template <class Archive, typename _Scalar, int _Options, typename _Index>
    void load(Archive & ar, Eigen::SparseMatrix<_Scalar,_Options,_Index>  & m) {
        int32_t innerSize, outerSize;
        ar(innerSize); ar(outerSize);

        int rows = m.IsRowMajor?outerSize:innerSize;
        int cols = m.IsRowMajor?innerSize:outerSize;
        m.resize(rows,cols);

        typedef typename Eigen::Triplet<_Scalar> Triplet;
        std::vector<Triplet> triplets;
        ar(triplets);
        m.setFromTriplets(triplets.begin(), triplets.end());
    }
}


#endif //SIMULATOR_EIGEN_CEREAL_SERIALIZATION_HPP
