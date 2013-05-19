//=================================================================================================
/*!
//  \file blaze/math/views/SparseRow.h
//  \brief Header file for the SparseRow class template
//
//  Copyright (C) 2011 Klaus Iglberger - All Rights Reserved
//
//  This file is part of the Blaze library. This library is free software; you can redistribute
//  it and/or modify it under the terms of the GNU General Public License as published by the
//  Free Software Foundation; either version 3, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along with a special
//  exception for linking and compiling against the Blaze library, the so-called "runtime
//  exception"; see the file COPYING. If not, see http://www.gnu.org/licenses/.
*/
//=================================================================================================

#ifndef _BLAZE_MATH_VIEWS_SPARSEROW_H_
#define _BLAZE_MATH_VIEWS_SPARSEROW_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <iterator>
#include <stdexcept>
#include <blaze/math/constraints/Expression.h>
#include <blaze/math/constraints/RequiresEvaluation.h>
#include <blaze/math/constraints/SparseMatrix.h>
#include <blaze/math/constraints/StorageOrder.h>
#include <blaze/math/constraints/TransposeFlag.h>
#include <blaze/math/expressions/Expression.h>
#include <blaze/math/expressions/SparseVector.h>
#include <blaze/math/Forward.h>
#include <blaze/math/Functions.h>
#include <blaze/math/shims/IsDefault.h>
#include <blaze/math/shims/Reset.h>
#include <blaze/math/sparse/SparseElement.h>
#include <blaze/math/traits/AddTrait.h>
#include <blaze/math/traits/DivTrait.h>
#include <blaze/math/traits/MultTrait.h>
#include <blaze/math/traits/RowTrait.h>
#include <blaze/math/traits/SubTrait.h>
#include <blaze/math/typetraits/IsExpression.h>
#include <blaze/math/typetraits/IsRowMajorMatrix.h>
#include <blaze/util/Assert.h>
#include <blaze/util/DisableIf.h>
#include <blaze/util/EnableIf.h>
#include <blaze/util/logging/FunctionTrace.h>
#include <blaze/util/mpl/If.h>
#include <blaze/util/SelectType.h>
#include <blaze/util/Types.h>
#include <blaze/util/typetraits/IsConst.h>
#include <blaze/util/typetraits/IsFloatingPoint.h>
#include <blaze/util/typetraits/IsNumeric.h>


namespace blaze {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\defgroup sparse_row SparseRow
// \ingroup views
*/
/*!\brief Reference to a specific row of a sparse matrix.
// \ingroup sparse_row
//
// The SparseRow template represents a reference to a specific row of a sparse matrix primitive.
// The type of the sparse matrix is specified via the first template parameter:

   \code
   template< typename MT, bool SO >
   class SparseRow;
   \endcode

//  - MT: specifies the type of the sparse matrix primitive. SparseRow can be used with any sparse
//        matrix primitive, but does not work with any matrix expression type.
//  - SO: specifies the storage order (blaze::rowMajor, blaze::columnMajor) of the sparse matrix.
//        This template parameter doesn't have to be explicitly defined, but is automatically
//        derived from the first template parameter.
//
// A reference to a sparse row can be conventiently created via the row() function. The row can
// be either used as an alias to grant write access to a specific row of a matrix primitive on
// the left-hand side of an assignment or to grant read-access to a specific row of a matrix
// primitive or expression on the right-hand side of an assignment:

   \code
   blaze::DynamicVector<double,rowVector> x;
   blaze::DynamicMatrix<double,rowMajor> A, B;
   // ... Resizing and initialization

   // Setting the 2nd row of matrix A to x
   row( A, 2UL ) = x;

   // Setting x to the 3rd row of the result of the matrix multiplication
   x = row( A * B, 3UL );
   \endcode

// Inserting/accessing elements in a sparse row can be done by several alternative functions.
// The following example demonstrates all options:

   \code
   using blaze::CompressedMatrix;
   using blaze::SparseRow;

   typedef CompressedMatrix<double,rowMajor>  MatrixType;
   MatrixType A( 10UL, 100UL );  // Non-initialized 10x100 matrix

   typedef SparseRow<MatrixType>  RowType;
   RowType row0( row( A, 0UL ) );  // Reference to the 0th row of A

   // The subscript operator provides access to all possible elements of the sparse row,
   // including the zero elements. In case the subscript operator is used to access an element
   // that is currently not stored in the sparse row, the element is inserted into the row.
   row0[42] = 2.0;

   // An alternative for inserting elements into the row is the insert() function. However,
   // it inserts the element only in case the element is not already contained in the row.
   row0.insert( 50UL, 3.7 );

   // A very efficient way to add new elements to a sparse row is the append() function.
   // Note that append() requires that the appended element's index is strictly larger than
   // the currently largest non-zero index of the row and that the row's capacity is large
   // enough to hold the new element.
   row0.reserve( 10UL );
   row0.append( 51UL, -2.1 );

   // In order to traverse all non-zero elements currently stored in the row, the begin()
   // and end() functions can be used. In the example, all non-zero elements of row are
   // traversed.
   for( RowType::Iterator i=a.begin(); i!=a.end(); ++i ) {
      ... = i->value();  // Access to the value of the non-zero element
      ... = i->index();  // Access to the index of the non-zero element
   }
   \endcode

// The following example gives an impression of the use of SparseRow. All operations (addition,
// subtraction, multiplication, scaling, ...) can be performed on all possible combinations of
// dense and sparse vectors with fitting element types:

   \code
   using blaze::DynamicVector;
   using blaze::CompressedVector;
   using blaze::DynamicMatrix;
   using blaze::SparseRow;

   CompressedVector<double,rowVector> a( 2UL ), b;
   a[1] = 2.0;
   DynamicVector<double,rowVector> c( 2UL, 3UL );

   typedef CompressedMatrix<double,rowMajor>  MatrixType;
   MatrixType A( 3UL, 2UL );  // Non-initialized 3x2 matrix

   typedef SparseRow<MatrixType>  RowType;
   RowType row0( row( A, 0UL ) );  // Reference to the 0th row of A

   row0[0] = 0UL;        // Manual initialization of the 0th row of A
   row0[1] = 0UL;
   row( A, 1UL ) = a;    // Dense vector initialization of the 1st row of A
   row( A, 2UL ) = c;    // Sparse vector initialization of the 2nd row of A

   b = row0 + a;              // Sparse vector/sparse vector addition
   b = c + row( A, 1UL );     // Dense vector/sparse vector addition
   b = row0 * row( A, 2UL );  // Component-wise vector multiplication

   row( A, 1UL ) *= 2.0;     // In-place scaling of the 1st row
   b = row( A, 1UL ) * 2.0;  // Scaling of the 1st row
   b = 2.0 * row( A, 1UL );  // Scaling of the 1st row

   row( A, 2UL ) += a;              // Addition assignment
   row( A, 2UL ) -= c;              // Subtraction assignment
   row( A, 2UL ) *= row( A, 0UL );  // Multiplication assignment

   double scalar = row( A, 1UL ) * trans( c );  // Scalar/dot/inner product between two vectors

   A = trans( c ) * row( A, 1UL );  // Outer product between two vectors
   \endcode

// It is possible to create a row view on both row-major and column-major matrices. However,
// please note that creating a row view on a matrix stored in column-major fashion can result
// in a considerable performance decrease in comparison to a row view on a row-major matrix
// due to the non-contiguous storage of the non-zero matrix elements. Therefore care has to
// be taken in the choice of the most suitable storage order:

   \code
   // Setup of two column-major matrices
   CompressedMatrix<double,columnMajor> A( 128UL, 128UL );
   CompressedMatrix<double,columnMajor> B( 128UL, 128UL );
   // ... Resizing and initialization

   // The computation of the 15th row of the multiplication between A and B ...
   CompressedVector<double,rowVector> x = row( A * B, 15UL );

   // ... is essentially the same as the following computation, which multiplies
   // the 15th row of the column-major matrix A with B.
   ComputationVector<double,rowVector> x = row( A, 15UL ) * B;
   \endcode

// Although Blaze performs the resulting vector/matrix multiplication as efficiently as possible
// using a row-major storage order for matrix A would result in a more efficient evaluation.
*/
template< typename MT                              // Type of the sparse matrix
        , bool SO = IsRowMajorMatrix<MT>::value >  // Storage order
class SparseRow : public SparseVector< SparseRow<MT,SO>, true >
                , private Expression
{
 private:
   //**********************************************************************************************
   //! Compilation switch for the non-const reference and iterator types.
   /*! The \a useConst compile time constant expression represents a compilation switch for
       the non-const reference and iterator types. In case the given sparse matrix of type
       \a MT is const qualified, \a useConst will be set to 1 and the sparse row will return
       references and iterators to const. Otherwise \a useConst will be set to 0 and the
       sparse row will offer write access to the sparse matrix elements both via the subscript
       operator and iterators. */
   enum { useConst = IsConst<MT>::value };
   //**********************************************************************************************

 public:
   //**Type definitions****************************************************************************
   typedef SparseRow<MT,SO>                    This;           //!< Type of this SparseRow instance.
   typedef typename RowTrait<MT>::Type         ResultType;     //!< Result type for expression template evaluations.
   typedef typename ResultType::TransposeType  TransposeType;  //!< Transpose type for expression template evaluations.
   typedef typename MT::ElementType            ElementType;    //!< Type of the row elements.
   typedef typename MT::ReturnType             ReturnType;     //!< Return type for expression template evaluations
   typedef const SparseRow&                    CompositeType;  //!< Data type for composite expression templates.

   //! Reference to a constant row value.
   typedef typename MT::ConstReference  ConstReference;

   //! Reference to a non-constant row value.
   typedef typename SelectType< useConst, ConstReference, typename MT::Reference >::Type  Reference;

   //! Iterator over constant elements.
   typedef typename MT::ConstIterator  ConstIterator;

   //! Iterator over non-constant elements.
   typedef typename SelectType< useConst, ConstIterator, typename MT::Iterator >::Type  Iterator;
   //**********************************************************************************************

   //**Constructors********************************************************************************
   /*!\name Constructors */
   //@{
   explicit inline SparseRow( MT& matrix, size_t index );
   // No explicitly declared copy constructor.
   //@}
   //**********************************************************************************************

   //**Destructor**********************************************************************************
   // No explicitly declared destructor.
   //**********************************************************************************************

   //**Data access functions***********************************************************************
   /*!\name Data access functions */
   //@{
   inline Reference      operator[]( size_t index );
   inline ConstReference operator[]( size_t index ) const;
   inline Iterator       begin ();
   inline ConstIterator  begin () const;
   inline ConstIterator  cbegin() const;
   inline Iterator       end   ();
   inline ConstIterator  end   () const;
   inline ConstIterator  cend  () const;
   //@}
   //**********************************************************************************************

   //**Assignment operators************************************************************************
   /*!\name Assignment operators */
   //@{
                           inline SparseRow& operator= ( const SparseRow& rhs );
   template< typename VT > inline SparseRow& operator= ( const DenseVector <VT,true>& rhs );
   template< typename VT > inline SparseRow& operator= ( const SparseVector<VT,true>& rhs );
   template< typename VT > inline SparseRow& operator+=( const Vector<VT,true>& rhs );
   template< typename VT > inline SparseRow& operator-=( const Vector<VT,true>& rhs );
   template< typename VT > inline SparseRow& operator*=( const Vector<VT,true>& rhs );

   template< typename Other >
   inline typename EnableIf< IsNumeric<Other>, SparseRow >::Type&
      operator*=( Other rhs );

   template< typename Other >
   inline typename EnableIf< IsNumeric<Other>, SparseRow >::Type&
      operator/=( Other rhs );
   //@}
   //**********************************************************************************************

   //**Utility functions***************************************************************************
   /*!\name Utility functions */
   //@{
                              inline size_t        size() const;
                              inline size_t        capacity() const;
                              inline size_t        nonZeros() const;
                              inline void          reset();
                              inline ElementType&  insert ( size_t index, const ElementType& value );
                              inline void          erase  ( size_t index );
                              inline Iterator      erase  ( Iterator pos );
                              inline Iterator      find   ( size_t index );
                              inline ConstIterator find   ( size_t index ) const;
                              inline void          reserve( size_t n );
   template< typename Other > inline SparseRow&    scale  ( Other scalar );
   //@}
   //**********************************************************************************************

   //**Low-level utility functions*****************************************************************
   /*!\name Low-level utility functions */
   //@{
   inline void append( size_t index, const ElementType& value, bool check=false );
   //@}
   //**********************************************************************************************

   //**Expression template evaluation functions****************************************************
   /*!\name Expression template evaluation functions */
   //@{
   template< typename Other > inline bool canAlias ( const Other* alias ) const;
   template< typename Other > inline bool isAliased( const Other* alias ) const;
   template< typename VT >    inline void assign   ( const DenseVector <VT,true>& rhs );
   template< typename VT >    inline void assign   ( const SparseVector<VT,true>& rhs );
   template< typename VT >    inline void addAssign( const DenseVector <VT,true>& rhs );
   template< typename VT >    inline void addAssign( const SparseVector<VT,true>& rhs );
   template< typename VT >    inline void subAssign( const DenseVector <VT,true>& rhs );
   template< typename VT >    inline void subAssign( const SparseVector<VT,true>& rhs );
   //@}
   //**********************************************************************************************

 private:
   //**Utility functions***************************************************************************
   /*!\name Utility functions */
   //@{
   inline size_t extendCapacity() const;
   //@}
   //**********************************************************************************************

   //**Member variables****************************************************************************
   /*!\name Member variables */
   //@{
   MT&          matrix_;  //!< The sparse matrix containing the row.
   const size_t row_;     //!< The index of the row in the matrix.
   //@}
   //**********************************************************************************************

   //**Compile time checks*************************************************************************
   /*! \cond BLAZE_INTERNAL */
   BLAZE_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE   ( MT );
   BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_EXPRESSION_TYPE  ( MT );
   /*! \endcond */
   //**********************************************************************************************
};
//*************************************************************************************************




//=================================================================================================
//
//  CONSTRUCTOR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief The constructor for SparseRow.
//
// \param matrix The matrix containing the row.
// \param index The index of the row.
// \exception std::invalid_argument Invalid row access index.
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline SparseRow<MT,SO>::SparseRow( MT& matrix, size_t index )
   : matrix_( matrix )  // The sparse matrix containing the row
   , row_   ( index  )  // The index of the row in the matrix
{
   if( matrix_.rows() <= index )
      throw std::invalid_argument( "Invalid row access index" );
}
//*************************************************************************************************




//=================================================================================================
//
//  DATA ACCESS FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Subscript operator for the direct access to the row elements.
//
// \param index Access index. The index must be smaller than the number of matrix columns.
// \return Reference to the accessed value.
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline typename SparseRow<MT,SO>::Reference SparseRow<MT,SO>::operator[]( size_t index )
{
   BLAZE_USER_ASSERT( index < size(), "Invalid row access index" );
   return matrix_(row_,index);
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Subscript operator for the direct access to the row elements.
//
// \param index Access index. The index must be smaller than the number of matrix columns.
// \return Reference to the accessed value.
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline typename SparseRow<MT,SO>::ConstReference SparseRow<MT,SO>::operator[]( size_t index ) const
{
   BLAZE_USER_ASSERT( index < size(), "Invalid row access index" );
   return const_cast<const MT&>( matrix_ )(row_,index);
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator to the first element of the row.
//
// \return Iterator to the first element of the row.
//
// This function returns an iterator to the first element of the row.
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline typename SparseRow<MT,SO>::Iterator SparseRow<MT,SO>::begin()
{
   return matrix_.begin( row_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator to the first element of the row.
//
// \return Iterator to the first element of the row.
//
// This function returns an iterator to the first element of the row.
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline typename SparseRow<MT,SO>::ConstIterator SparseRow<MT,SO>::begin() const
{
   return matrix_.begin( row_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator to the first element of the row.
//
// \return Iterator to the first element of the row.
//
// This function returns an iterator to the first element of the row.
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline typename SparseRow<MT,SO>::ConstIterator SparseRow<MT,SO>::cbegin() const
{
   return matrix_.begin( row_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator just past the last element of the row.
//
// \return Iterator just past the last element of the row.
//
// This function returns an iterator just past the last element of the row.
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline typename SparseRow<MT,SO>::Iterator SparseRow<MT,SO>::end()
{
   return matrix_.end( row_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator just past the last element of the row.
//
// \return Iterator just past the last element of the row.
//
// This function returns an iterator just past the last element of the row.
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline typename SparseRow<MT,SO>::ConstIterator SparseRow<MT,SO>::end() const
{
   return matrix_.end( row_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator just past the last element of the row.
//
// \return Iterator just past the last element of the row.
//
// This function returns an iterator just past the last element of the row.
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline typename SparseRow<MT,SO>::ConstIterator SparseRow<MT,SO>::cend() const
{
   return matrix_.end( row_ );
}
//*************************************************************************************************




//=================================================================================================
//
//  ASSIGNMENT OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Copy assignment operator for SparseRow.
//
// \param rhs Sparse row to be copied.
// \return Reference to the assigned row.
// \exception std::invalid_argument Row sizes do not match.
//
// In case the current sizes of the two rows don't match, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline SparseRow<MT,SO>& SparseRow<MT,SO>::operator=( const SparseRow& rhs )
{
   using blaze::assign;

   if( this == &rhs || ( &matrix_ == &rhs.matrix_ && row_ == rhs.row_ ) )
      return *this;

   if( size() != rhs.size() )
      throw std::invalid_argument( "Row sizes do not match" );

   if( rhs.canAlias( &matrix_ ) ) {
      const ResultType tmp( rhs );
      matrix_.reset  ( row_ );
      matrix_.reserve( row_, tmp.nonZeros() );
      assign( *this, tmp );
   }
   else {
      matrix_.reset  ( row_ );
      matrix_.reserve( row_, rhs.nonZeros() );
      assign( *this, rhs );
   }

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Assignment operator for different dense vectors.
//
// \param rhs Dense vector to be assigned.
// \return Reference to the assigned row.
// \exception std::invalid_argument Vector sizes do not match.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT    // Type of the sparse matrix
        , bool SO >      // Storage order
template< typename VT >  // Type of the right-hand side dense vector
inline SparseRow<MT,SO>& SparseRow<MT,SO>::operator=( const DenseVector<VT,true>& rhs )
{
   using blaze::assign;

   if( size() != (~rhs).size() )
      throw std::invalid_argument( "Vector sizes do not match" );

   if( (~rhs).canAlias( &matrix_ ) ) {
      const typename VT::ResultType tmp( ~rhs );
      matrix_.reset( row_ );
      assign( *this, tmp );
   }
   else {
      matrix_.reset( row_ );
      assign( *this, ~rhs );
   }

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Assignment operator for different sparse vectors.
//
// \param rhs Sparse vector to be assigned.
// \return Reference to the assigned row.
// \exception std::invalid_argument Vector sizes do not match.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT    // Type of the sparse matrix
        , bool SO >      // Storage order
template< typename VT >  // Type of the right-hand side sparse vector
inline SparseRow<MT,SO>& SparseRow<MT,SO>::operator=( const SparseVector<VT,true>& rhs )
{
   using blaze::assign;

   if( size() != (~rhs).size() )
      throw std::invalid_argument( "Vector sizes do not match" );

   if( (~rhs).canAlias( &matrix_ ) ) {
      const typename VT::ResultType tmp( ~rhs );
      matrix_.reset  ( row_ );
      matrix_.reserve( row_, tmp.nonZeros() );
      assign( *this, tmp );
   }
   else {
      matrix_.reset  ( row_ );
      matrix_.reserve( row_, (~rhs).nonZeros() );
      assign( *this, ~rhs );
   }

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Addition assignment operator for the addition of a vector (\f$ \vec{a}+=\vec{b} \f$).
//
// \param rhs The right-hand side vector to be added to the sparse row.
// \return Reference to the sparse row.
// \exception std::invalid_argument Vector sizes do not match.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename MT    // Type of the sparse matrix
        , bool SO >      // Storage order
template< typename VT >  // Type of the right-hand side vector
inline SparseRow<MT,SO>& SparseRow<MT,SO>::operator+=( const Vector<VT,true>& rhs )
{
   using blaze::addAssign;

   if( (~rhs).size() != size() )
      throw std::invalid_argument( "Vector sizes do not match" );

   addAssign( *this, ~rhs );

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Subtraction assignment operator for the subtraction of a vector (\f$ \vec{a}-=\vec{b} \f$).
//
// \param rhs The right-hand side vector to be subtracted from the sparse row.
// \return Reference to the sparse row.
// \exception std::invalid_argument Vector sizes do not match.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename MT    // Type of the sparse matrix
        , bool SO >      // Storage order
template< typename VT >  // Type of the right-hand side vector
inline SparseRow<MT,SO>& SparseRow<MT,SO>::operator-=( const Vector<VT,true>& rhs )
{
   using blaze::subAssign;

   if( (~rhs).size() != size() )
      throw std::invalid_argument( "Vector sizes do not match" );

   subAssign( *this, ~rhs );

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Multiplication assignment operator for the multiplication of a vector
//        (\f$ \vec{a}*=\vec{b} \f$).
//
// \param rhs The right-hand side vector to be multiplied with the sparse row.
// \return Reference to the sparse row.
// \exception std::invalid_argument Vector sizes do not match.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename MT    // Type of the sparse matrix
        , bool SO >      // Storage order
template< typename VT >  // Type of the right-hand side vector
inline SparseRow<MT,SO>& SparseRow<MT,SO>::operator*=( const Vector<VT,true>& rhs )
{
   if( (~rhs).size() != size() )
      throw std::invalid_argument( "Vector sizes do not match" );

   typedef typename MultTrait<This,typename VT::ResultType>::Type  MultType;

   BLAZE_CONSTRAINT_MUST_BE_TRANSPOSE_VECTOR_TYPE( MultType );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION  ( MultType );

   const MultType tmp( *this * (~rhs) );
   matrix_.reset( row_ );
   assign( tmp );

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Multiplication assignment operator for the multiplication between a sparse row
//        and a scalar value (\f$ \vec{a}*=s \f$).
//
// \param rhs The right-hand side scalar value for the multiplication.
// \return Reference to the sparse row.
//
// This operator can only be used for built-in data types. Additionally, the elements of
// the sparse row must support the multiplication assignment operator for the given scalar
// built-in data type.
*/
template< typename MT       // Type of the sparse matrix
        , bool SO >         // Storage order
template< typename Other >  // Data type of the right-hand side scalar
inline typename EnableIf< IsNumeric<Other>, SparseRow<MT,SO> >::Type&
   SparseRow<MT,SO>::operator*=( Other rhs )
{
   for( Iterator element=begin(); element!=end(); ++element )
      element->value() *= rhs;
   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Division assignment operator for the division of a sparse row by a scalar value
//        (\f$ \vec{a}/=s \f$).
//
// \param rhs The right-hand side scalar value for the division.
// \return Reference to the sparse row.
//
// This operator can only be used for built-in data types. Additionally, the elements of the
// sparse row must either support the multiplication assignment operator for the given
// floating point data type or the division assignment operator for the given integral data
// type.
*/
template< typename MT       // Type of the sparse matrix
        , bool SO >         // Storage order
template< typename Other >  // Data type of the right-hand side scalar
inline typename EnableIf< IsNumeric<Other>, SparseRow<MT,SO> >::Type&
   SparseRow<MT,SO>::operator/=( Other rhs )
{
   BLAZE_USER_ASSERT( rhs != Other(0), "Division by zero detected" );

   typedef typename DivTrait<ElementType,Other>::Type  DT;
   typedef typename If< IsNumeric<DT>, DT, Other >::Type  Tmp;

   // Depending on the two involved data types, an integer division is applied or a
   // floating point division is selected.
   if( IsNumeric<DT>::value && IsFloatingPoint<DT>::value ) {
      const Tmp tmp( Tmp(1)/static_cast<Tmp>( rhs ) );
      for( Iterator element=begin(); element!=end(); ++element )
         element->value() *= tmp;
   }
   else {
      for( Iterator element=begin(); element!=end(); ++element )
         element->value() /= rhs;
   }

   return *this;
}
//*************************************************************************************************




//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns the current size/dimension of the sparse row.
//
// \return The size of the sparse row.
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline size_t SparseRow<MT,SO>::size() const
{
   return matrix_.columns();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns the maximum capacity of the sparse row.
//
// \return The capacity of the sparse row.
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline size_t SparseRow<MT,SO>::capacity() const
{
   return matrix_.capacity( row_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns the number of non-zero elements in the row.
//
// \return The number of non-zero elements in the row.
//
// Note that the number of non-zero elements is always less than or equal to the current number
// of columns of the matrix containing the row.
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline size_t SparseRow<MT,SO>::nonZeros() const
{
   return matrix_.nonZeros( row_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Reset to the default initial values.
//
// \return void
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline void SparseRow<MT,SO>::reset()
{
   matrix_.reset( row_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Inserting an element into the sparse row.
//
// \param index The index of the new element. The index has to be in the range \f$[0..N-1]\f$.
// \param value The value of the element to be inserted.
// \return Reference to the inserted value.
// \exception std::invalid_argument Invalid sparse row access index.
//
// This function inserts a new element into the sparse row. However, duplicate elements
// are not allowed. In case the sparse row already contains an element at index \a index,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline typename SparseRow<MT,SO>::ElementType&
   SparseRow<MT,SO>::insert( size_t index, const ElementType& value )
{
   return matrix_.insert( row_, index, value )->value();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Erasing an element from the sparse row.
//
// \param index The index of the element to be erased. The index has to be in the range \f$[0..N-1]\f$.
// \return void
//
// This function erases an element from the sparse row.
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline void SparseRow<MT,SO>::erase( size_t index )
{
   matrix_.erase( row_, index );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Erasing an element from the sparse row.
//
// \param pos Iterator to the element to be erased.
// \return void
//
// This function erases an element from the sparse row.
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline typename SparseRow<MT,SO>::Iterator SparseRow<MT,SO>::erase( Iterator pos )
{
   return matrix_.erase( row_, pos );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Searches for a specific row element.
//
// \param index The index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \return Iterator to the element in case the index is found, end() iterator otherwise.
//
// This function can be used to check whether a specific element is contained in the sparse
// row. It specifically searches for the element with index \a index. In case the element is
// found, the function returns an iterator to the element. Otherwise an iterator just past
// the last non-zero element of the sparse row (the end() iterator) is returned. Note that
// the returned sparse row iterator is subject to invalidation due to inserting operations
// via the subscript operator or the insert() function!
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline typename SparseRow<MT,SO>::Iterator SparseRow<MT,SO>::find( size_t index )
{
   return matrix_.find( row_, index );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Searches for a specific row element.
//
// \param index The index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \return Iterator to the element in case the index is found, end() iterator otherwise.
//
// This function can be used to check whether a specific element is contained in the sparse
// row. It specifically searches for the element with index \a index. In case the element is
// found, the function returns an iterator to the element. Otherwise an iterator just past
// the last non-zero element of the sparse row (the end() iterator) is returned. Note that
// the returned sparse row iterator is subject to invalidation due to inserting operations
// via the subscript operator or the insert() function!
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline typename SparseRow<MT,SO>::ConstIterator SparseRow<MT,SO>::find( size_t index ) const
{
   return matrix_.find( row_, index );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Setting the minimum capacity of the sparse row.
//
// \param n The new minimum capacity of the sparse row.
// \return void
//
// This function increases the capacity of the sparse row to at least \a n elements. The
// current values of the row elements are preserved.
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
void SparseRow<MT,SO>::reserve( size_t n )
{
   matrix_.reserve( row_, n );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Scaling of the sparse row by the scalar value \a scalar (\f$ \vec{a}=\vec{b}*s \f$).
//
// \param scalar The scalar value for the row scaling.
// \return Reference to the sparse row.
*/
template< typename MT       // Type of the sparse matrix
        , bool SO >         // Storage order
template< typename Other >  // Data type of the scalar value
inline SparseRow<MT,SO>& SparseRow<MT,SO>::scale( Other scalar )
{
   for( Iterator element=begin(); element!=end(); ++element )
      element->value() *= scalar;
   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Calculating a new sparse row capacity.
//
// \return The new sparse row capacity.
//
// This function calculates a new row capacity based on the current capacity of the sparse
// row. Note that the new capacity is restricted to the interval \f$[7..size]\f$.
*/
template< typename MT       // Type of the sparse matrix
        , bool SO >         // Storage order
inline size_t SparseRow<MT,SO>::extendCapacity() const
{
   using blaze::max;
   using blaze::min;

   size_t nonzeros( 2UL*capacity()+1UL );
   nonzeros = max( nonzeros, 7UL    );
   nonzeros = min( nonzeros, size() );

   BLAZE_INTERNAL_ASSERT( nonzeros > capacity(), "Invalid capacity value" );

   return nonzeros;
}
//*************************************************************************************************




//=================================================================================================
//
//  LOW-LEVEL UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Appending an element to the sparse row.
//
// \param index The index of the new element. The index must be smaller than the number of matrix columns.
// \param value The value of the element to be appended.
// \param check \a true if the new value should be checked for default values, \a false if not.
// \return void
//
// This function provides a very efficient way to fill a sparse row with elements. It appends
// a new element to the end of the sparse row without any memory allocation. Therefore it is
// strictly necessary to keep the following preconditions in mind:
//
//  - the index of the new element must be strictly larger than the largest index of non-zero
//    elements in the sparse row
//  - the current number of non-zero elements must be smaller than the capacity of the row
//
// Ignoring these preconditions might result in undefined behavior! The optional \a check
// parameter specifies whether the new value should be tested for a default value. If the new
// value is a default value (for instance 0 in case of an integral element type) the value is
// not appended. Per default the values are not tested.
//
// \b Note: Although append() does not allocate new memory, it still invalidates all iterators
// returned by the end() functions!
*/
template< typename MT       // Type of the sparse matrix
        , bool SO >         // Storage order
inline void SparseRow<MT,SO>::append( size_t index, const ElementType& value, bool check )
{
   matrix_.append( row_, index, value, check );
}
//*************************************************************************************************




//=================================================================================================
//
//  EXPRESSION TEMPLATE EVALUATION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns whether the sparse row can alias with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this sparse row, \a false if not.
//
// This function returns whether the given address can alias with the sparse row. In contrast
// to the isAliased() function this function is allowed to use compile time expressions to
// optimize the evaluation.
*/
template< typename MT       // Type of the sparse matrix
        , bool SO >         // Storage order
template< typename Other >  // Data type of the foreign expression
inline bool SparseRow<MT,SO>::canAlias( const Other* alias ) const
{
   return static_cast<const void*>( &matrix_ ) == static_cast<const void*>( alias );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns whether the sparse row is aliased with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this sparse row, \a false if not.
//
// This function returns whether the given address is aliased with the sparse row. In contrast
// to the canAlias() function this function is not allowed to use compile time expressions to
// optimize the evaluation.
*/
template< typename MT       // Type of the sparse matrix
        , bool SO >         // Storage order
template< typename Other >  // Data type of the foreign expression
inline bool SparseRow<MT,SO>::isAliased( const Other* alias ) const
{
   return static_cast<const void*>( &matrix_ ) == static_cast<const void*>( alias );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the sparse matrix
        , bool SO >      // Storage order
template< typename VT >  // Type of the right-hand side dense vector
inline void SparseRow<MT,SO>::assign( const DenseVector<VT,true>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   size_t nonzeros( 0UL );

   for( size_t j=0UL; j<size(); ++j )
   {
      if( matrix_.nonZeros( row_ ) == matrix_.capacity( row_ ) )
         matrix_.reserve( row_, extendCapacity() );

      matrix_.append( row_, j, (~rhs)[j], true );
   }
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the sparse matrix
        , bool SO >      // Storage order
template< typename VT >  // Type of the right-hand side sparse vector
inline void SparseRow<MT,SO>::assign( const SparseVector<VT,true>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   for( typename VT::ConstIterator element=(~rhs).begin(); element!=(~rhs).end(); ++element ) {
      matrix_.append( row_, element->index(), element->value() );
   }
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the addition assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the sparse matrix
        , bool SO >      // Storage order
template< typename VT >  // Type of the right-hand side dense vector
inline void SparseRow<MT,SO>::addAssign( const DenseVector<VT,true>& rhs )
{
   typedef typename AddTrait<This,typename VT::ResultType>::Type  AddType;

   BLAZE_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE    ( AddType );
   BLAZE_CONSTRAINT_MUST_BE_TRANSPOSE_VECTOR_TYPE( AddType );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION  ( AddType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const AddType tmp( *this + (~rhs) );
   matrix_.reset( row_ );
   assign( tmp );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the addition assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the sparse matrix
        , bool SO >      // Storage order
template< typename VT >  // Type of the right-hand side sparse vector
inline void SparseRow<MT,SO>::addAssign( const SparseVector<VT,true>& rhs )
{
   typedef typename AddTrait<This,typename VT::ResultType>::Type  AddType;

   BLAZE_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE   ( AddType );
   BLAZE_CONSTRAINT_MUST_BE_TRANSPOSE_VECTOR_TYPE( AddType );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION  ( AddType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const AddType tmp( *this + (~rhs) );
   matrix_.reset  ( row_ );
   matrix_.reserve( row_, tmp.nonZeros() );
   assign( tmp );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the subtraction assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the sparse matrix
        , bool SO >      // Storage order
template< typename VT >  // Type of the right-hand side dense vector
inline void SparseRow<MT,SO>::subAssign( const DenseVector<VT,true>& rhs )
{
   typedef typename SubTrait<This,typename VT::ResultType>::Type  SubType;

   BLAZE_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE    ( SubType );
   BLAZE_CONSTRAINT_MUST_BE_TRANSPOSE_VECTOR_TYPE( SubType );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION  ( SubType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const SubType tmp( *this - (~rhs) );
   matrix_.reset  ( row_ );
   assign( tmp );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the subtraction assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT    // Type of the sparse matrix
        , bool SO >      // Storage order
template< typename VT >  // Type of the right-hand side sparse vector
inline void SparseRow<MT,SO>::subAssign( const SparseVector<VT,true>& rhs )
{
   typedef typename SubTrait<This,typename VT::ResultType>::Type  SubType;

   BLAZE_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE   ( SubType );
   BLAZE_CONSTRAINT_MUST_BE_TRANSPOSE_VECTOR_TYPE( SubType );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION  ( SubType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const SubType tmp( *this - (~rhs) );
   matrix_.reset  ( row_ );
   matrix_.reserve( row_, tmp.nonZeros() );
   assign( tmp );
}
//*************************************************************************************************








//=================================================================================================
//
//  CLASS TEMPLATE SPECIALIZATION FOR COLUMN-MAJOR MATRICES
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Specialization of SparseRow for column-major matrices.
// \ingroup sparse_row
//
// This specialization of SparseRow adapts the class template to the requirements of
// column-major matrices.
*/
template< typename MT >  // Type of the sparse matrix
class SparseRow<MT,false> : public SparseVector< SparseRow<MT,false>, true >
                          , private Expression
{
 private:
   //**********************************************************************************************
   //! Compilation switch for the non-const reference and iterator types.
   /*! The \a useConst compile time constant expression represents a compilation switch for
       the non-const reference and iterator types. In case the given sparse matrix of type
       \a MT is const qualified, \a useConst will be set to 1 and the sparse row will return
       references and iterators to const. Otherwise \a useConst will be set to 0 and the
       sparse row will offer write access to the sparse matrix elements both via the subscript
       operator and iterators. */
   enum { useConst = IsConst<MT>::value };
   //**********************************************************************************************

 public:
   //**Type definitions****************************************************************************
   typedef SparseRow<MT,false>                 This;            //!< Type of this SparseRow instance.
   typedef typename RowTrait<MT>::Type         ResultType;      //!< Result type for expression template evaluations.
   typedef typename ResultType::TransposeType  TransposeType;   //!< Transpose type for expression template evaluations.
   typedef typename MT::ElementType            ElementType;     //!< Type of the row elements.
   typedef typename MT::ReturnType             ReturnType;      //!< Return type for expression template evaluations
   typedef const ResultType                    CompositeType;   //!< Data type for composite expression templates.

   //! Reference to a constant row value.
   typedef typename MT::ConstReference  ConstReference;

   //! Reference to a non-constant row value.
   typedef typename SelectType< useConst, ConstReference, typename MT::Reference >::Type  Reference;
   //**********************************************************************************************

   //**RowElement class definition*****************************************************************
   /*!\brief Access proxy for a specific element of the sparse row.
   */
   template< typename MatrixType      // Type of the sparse matrix
           , typename IteratorType >  // Type of the sparse matrix iterator
   class RowElement
   {
    private:
      //*******************************************************************************************
      //! Compilation switch for the return type of the value member function.
      /*! The \a returnConst compile time constant expression represents a compilation switch for
          the return type of the value member function. In case the given matrix type \a MatrixType
          is const qualified, \a returnConst will be set to 1 and the value member function will
          return a reference to const. Otherwise \a returnConst will be set to 0 and the value
          member function will offer write access to the sparse matrix elements. */
      enum { returnConst = IsConst<MatrixType>::value };
      //*******************************************************************************************

    public:
      //**Type definitions*************************************************************************
      typedef typename SelectType< returnConst, const ElementType&, ElementType& >::Type  ReferenceType;
      //*******************************************************************************************

      //**Constructor******************************************************************************
      /*!\brief Constructor for the RowElement class.
      //
      // \param pos Iterator to the current position within the sparse row.
      // \param column The column index.
      */
      inline RowElement( IteratorType pos, size_t column )
         : pos_   ( pos    )  // Iterator to the current position within the sparse row
         , column_( column )  // Index of the according column
      {}
      //*******************************************************************************************

      //**Assignment operator**********************************************************************
      /*!\brief Assignment to the accessed sparse row element.
      //
      // \param value The new value of the sparse row element.
      // \return Reference to the sparse row element.
      */
      template< typename T > inline RowElement& operator=( const T& v ) {
         *pos_ = v;
         return *this;
      }
      //*******************************************************************************************

      //**Addition assignment operator*************************************************************
      /*!\brief Addition assignment to the accessed sparse row element.
      //
      // \param value The right-hand side value for the addition.
      // \return Reference to the sparse row element.
      */
      template< typename T > inline RowElement& operator+=( const T& v ) {
         *pos_ += v;
         return *this;
      }
      //*******************************************************************************************

      //**Subtraction assignment operator**********************************************************
      /*!\brief Subtraction assignment to the accessed sparse row element.
      //
      // \param value The right-hand side value for the subtraction.
      // \return Reference to the sparse row element.
      */
      template< typename T > inline RowElement& operator-=( const T& v ) {
         *pos_ -= v;
         return *this;
      }
      //*******************************************************************************************

      //**Multiplication assignment operator*******************************************************
      /*!\brief Multiplication assignment to the accessed sparse row element.
      //
      // \param value The right-hand side value for the multiplication.
      // \return Reference to the sparse row element.
      */
      template< typename T > inline RowElement& operator*=( const T& v ) {
         *pos_ *= v;
         return *this;
      }
      //*******************************************************************************************

      //**Division assignment operator*************************************************************
      /*!\brief Division assignment to the accessed sparse row element.
      //
      // \param value The right-hand side value for the division.
      // \return Reference to the sparse row element.
      */
      template< typename T > inline RowElement& operator/=( const T& v ) {
         *pos_ /= v;
         return *this;
      }
      //*******************************************************************************************

      //**Element access operator******************************************************************
      /*!\brief Direct access to the sparse vector element at the current iterator position.
      //
      // \return Reference to the sparse vector element at the current iterator position.
      */
      inline const RowElement* operator->() const {
         return this;
      }
      //*******************************************************************************************

      //**Value function***************************************************************************
      /*!\brief Access to the current value of the sparse row element.
      //
      // \return The current value of the sparse row element.
      */
      inline ReferenceType value() const {
         return pos_->value();
      }
      //*******************************************************************************************

      //**Index function***************************************************************************
      /*!\brief Access to the current index of the sparse element.
      //
      // \return The current index of the sparse element.
      */
      inline size_t index() const {
         return column_;
      }
      //*******************************************************************************************

    private:
      //**Member variables*************************************************************************
      IteratorType pos_;  //!< Iterator to the current position within the sparse row.
      size_t column_;     //!< Index of the according column.
      //*******************************************************************************************
   };
   //**********************************************************************************************

   //**RowIterator class definition****************************************************************
   /*!\brief Iterator over the elements of the sparse row.
   */
   template< typename MatrixType      // Type of the sparse matrix
           , typename IteratorType >  // Type of the sparse matrix iterator
   class RowIterator
   {
    private:
      //*******************************************************************************************
      //! Compilation switch for the return type of the value member function.
      /*! The \a returnConst compile time constant expression represents a compilation switch for
          the return type of the value member function. In case the given matrix type \a MatrixType
          is const qualified, \a returnConst will be set to 1 and the value member function will
          return a reference to const. Otherwise \a returnConst will be set to 0 and the value
          member function will offer write access to the sparse matrix elements. */
      enum { returnConst = IsConst<MatrixType>::value };
      //*******************************************************************************************

    public:
      //**Type definitions*************************************************************************
      typedef std::forward_iterator_tag            IteratorCategory;  //!< The iterator category.
      typedef RowElement<MatrixType,IteratorType>  ValueType;         //!< Type of the underlying elements.
      typedef ValueType                            PointerType;       //!< Pointer return type.
      typedef ValueType                            ReferenceType;     //!< Reference return type.
      typedef ptrdiff_t                            DifferenceType;    //!< Difference between two iterators.

      // STL iterator requirements
      typedef IteratorCategory  iterator_category;  //!< The iterator category.
      typedef ValueType         value_type;         //!< Type of the underlying elements.
      typedef PointerType       pointer;            //!< Pointer return type.
      typedef ReferenceType     reference;          //!< Reference return type.
      typedef DifferenceType    difference_type;    //!< Difference between two iterators.

      //! Return type for the access to the value of a sparse element.
      typedef typename SelectType< returnConst, ReturnType, ElementType& >::Type  Value;
      //*******************************************************************************************

      //**Constructor******************************************************************************
      /*!\brief Constructor for the RowIterator class.
      //
      // \param matrix The matrix containing the row.
      // \param row The row index.
      // \param column The column index.
      */
      inline RowIterator( MatrixType& matrix, size_t row, size_t column )
         : matrix_( matrix )  // The sparse matrix containing the row.
         , row_   ( row    )  // The current row index.
         , column_( column )  // The current column index.
         , pos_   ()          // Iterator to the current sparse element.
      {
         for( ; column_<matrix_.columns(); ++column_ ) {
            pos_ = matrix_.find( row_, column_ );
            if( pos_ != matrix_.end( column_ ) ) break;
         }
      }
      //*******************************************************************************************

      //**Constructor******************************************************************************
      /*!\brief Constructor for the RowIterator class.
      //
      // \param matrix The matrix containing the row.
      // \param row The row index.
      // \param column The column index.
      // \param pos Initial position of the iterator
      */
      inline RowIterator( MatrixType& matrix, size_t row, size_t column, IteratorType pos )
         : matrix_( matrix )  // The sparse matrix containing the row.
         , row_   ( row    )  // The current row index.
         , column_( column )  // The current column index.
         , pos_   ( pos    )  // Iterator to the current sparse element.
      {
         BLAZE_INTERNAL_ASSERT( matrix.find( row, column ) == pos, "Invalid initial iterator position" );
      }
      //*******************************************************************************************

      //**Constructor******************************************************************************
      /*!\brief Conversion constructor from different RowIterator instances.
      //
      // \param it The row iterator to be copied.
      */
      template< typename MatrixType2, typename IteratorType2 >
      inline RowIterator( const RowIterator<MatrixType2,IteratorType2>& it )
         : matrix_( it.matrix_ )  // The sparse matrix containing the row.
         , row_   ( it.row_    )  // The current row index.
         , column_( it.column_ )  // The current column index.
         , pos_   ( it.pos_    )  // Iterator to the current sparse element.
      {}
      //*******************************************************************************************

      //**Prefix increment operator****************************************************************
      /*!\brief Pre-increment operator.
      //
      // \return Reference to the incremented iterator.
      */
      inline RowIterator& operator++() {
         ++column_;
         for( ; column_<matrix_.columns(); ++column_ ) {
            pos_ = matrix_.find( row_, column_ );
            if( pos_ != matrix_.end( column_ ) ) break;
         }

         return *this;
      }
      //*******************************************************************************************

      //**Postfix increment operator***************************************************************
      /*!\brief Post-increment operator.
      //
      // \return The previous position of the iterator.
      */
      inline const RowIterator operator++( int ) {
         const RowIterator tmp( *this );
         ++(*this);
         return tmp;
      }
      //*******************************************************************************************

      //**Element access operator******************************************************************
      /*!\brief Direct access to the sparse vector element at the current iterator position.
      //
      // \return The current value of the sparse element.
      */
      inline ReferenceType operator*() const {
         return ReferenceType( pos_, column_ );
      }
      //*******************************************************************************************

      //**Element access operator******************************************************************
      /*!\brief Direct access to the sparse vector element at the current iterator position.
      //
      // \return Reference to the sparse vector element at the current iterator position.
      */
      inline PointerType operator->() const {
         return PointerType( pos_, column_ );
      }
      //*******************************************************************************************

      //**Equality operator************************************************************************
      /*!\brief Equality comparison between two RowIterator objects.
      //
      // \param rhs The right-hand side expression iterator.
      // \return \a true if the iterators refer to the same element, \a false if not.
      */
      template< typename MatrixType2, typename IteratorType2 >
      inline bool operator==( const RowIterator<MatrixType2,IteratorType2>& rhs ) const {
         return ( &matrix_ == &rhs.matrix_ ) && ( row_ == rhs.row_ ) && ( column_ == rhs.column_ );
      }
      //*******************************************************************************************

      //**Inequality operator**********************************************************************
      /*!\brief Inequality comparison between two RowIterator objects.
      //
      // \param rhs The right-hand side expression iterator.
      // \return \a true if the iterators don't refer to the same element, \a false if they do.
      */
      template< typename MatrixType2, typename IteratorType2 >
      inline bool operator!=( const RowIterator<MatrixType2,IteratorType2>& rhs ) const {
         return !( *this == rhs );
      }
      //*******************************************************************************************

      //**Subtraction operator*********************************************************************
      /*!\brief Calculating the number of elements between two expression iterators.
      //
      // \param rhs The right-hand side expression iterator.
      // \return The number of elements between the two expression iterators.
      */
      inline DifferenceType operator-( const RowIterator& rhs ) const {
         size_t counter( 0UL );
         for( size_t j=rhs.column_; j<column_; ++j ) {
            if( matrix_.find( row_, j ) != matrix_.end( j ) )
               ++counter;
         }
         return counter;
      }
      //*******************************************************************************************

    private:
      //**Member variables*************************************************************************
      MatrixType&  matrix_;  //!< The sparse matrix containing the row.
      size_t       row_;     //!< The current row index.
      size_t       column_;  //!< The current column index.
      IteratorType pos_;     //!< Iterator to the current sparse element.
      //*******************************************************************************************

      //**Friend declarations**********************************************************************
      /*! \cond BLAZE_INTERNAL */
      template< typename MatrixType2, typename IteratorType2 > friend class RowIterator;
      template< typename MT2, bool SO2 > friend class SparseRow;
      /*! \endcond */
      //*******************************************************************************************
   };
   //**********************************************************************************************

   //**Type definitions****************************************************************************
   //! Iterator over constant elements.
   typedef RowIterator<const MT,typename MT::ConstIterator>  ConstIterator;

   //! Iterator over non-constant elements.
   typedef typename SelectType< useConst, ConstIterator, RowIterator<MT,typename MT::Iterator> >::Type  Iterator;
   //**********************************************************************************************

   //**Constructors********************************************************************************
   /*!\name Constructors */
   //@{
   explicit inline SparseRow( MT& matrix, size_t index );
   // No explicitly declared copy constructor.
   //@}
   //**********************************************************************************************

   //**Destructor**********************************************************************************
   // No explicitly declared destructor.
   //**********************************************************************************************

   //**Data access functions***********************************************************************
   /*!\name Data access functions */
   //@{
   inline Reference      operator[]( size_t index );
   inline ConstReference operator[]( size_t index ) const;
   inline Iterator       begin ();
   inline ConstIterator  begin () const;
   inline ConstIterator  cbegin() const;
   inline Iterator       end   ();
   inline ConstIterator  end   () const;
   inline ConstIterator  cend  () const;
   //@}
   //**********************************************************************************************

   //**Assignment operators************************************************************************
   /*!\name Assignment operators */
   //@{
                           inline SparseRow& operator= ( const SparseRow& rhs );
   template< typename VT > inline SparseRow& operator= ( const Vector<VT,true>& rhs );
   template< typename VT > inline SparseRow& operator+=( const Vector<VT,true>& rhs );
   template< typename VT > inline SparseRow& operator-=( const Vector<VT,true>& rhs );
   template< typename VT > inline SparseRow& operator*=( const Vector<VT,true>& rhs );

   template< typename Other >
   inline typename EnableIf< IsNumeric<Other>, SparseRow >::Type&
      operator*=( Other rhs );

   template< typename Other >
   inline typename EnableIf< IsNumeric<Other>, SparseRow >::Type&
      operator/=( Other rhs );
   //@}
   //**********************************************************************************************

   //**Utility functions***************************************************************************
   /*!\name Utility functions */
   //@{
                              inline size_t        size() const;
                              inline size_t        capacity() const;
                              inline size_t        nonZeros() const;
                              inline void          reset();
                              inline ElementType&  insert ( size_t index, const ElementType& value );
                              inline void          erase  ( size_t index );
                              inline Iterator      erase  ( Iterator pos );
                              inline Iterator      find   ( size_t index );
                              inline ConstIterator find   ( size_t index ) const;
                              inline void          reserve( size_t n );
   template< typename Other > inline SparseRow&    scale  ( Other scalar );
   //@}
   //**********************************************************************************************

   //**Low-level utility functions*****************************************************************
   /*!\name Low-level utility functions */
   //@{
   inline void append( size_t index, const ElementType& value, bool check=false );
   //@}
   //**********************************************************************************************

   //**Expression template evaluation functions****************************************************
   /*!\name Expression template evaluation functions */
   //@{
   template< typename Other > inline bool canAlias ( const Other* alias ) const;
   template< typename Other > inline bool isAliased( const Other* alias ) const;
   template< typename VT >    inline void assign   ( const DenseVector <VT,true>& rhs );
   template< typename VT >    inline void assign   ( const SparseVector<VT,true>& rhs );
   template< typename VT >    inline void addAssign( const Vector<VT,true>& rhs );
   template< typename VT >    inline void subAssign( const Vector<VT,true>& rhs );
   //@}
   //**********************************************************************************************

 private:
   //**Member variables****************************************************************************
   /*!\name Member variables */
   //@{
   MT&          matrix_;  //!< The sparse matrix containing the row.
   const size_t row_;     //!< The index of the row in the matrix.
   //@}
   //**********************************************************************************************

   //**Compile time checks*************************************************************************
   /*! \cond BLAZE_INTERNAL */
   BLAZE_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE      ( MT );
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_EXPRESSION_TYPE     ( MT );
   /*! \endcond */
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  CONSTRUCTOR
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief The constructor for SparseRow.
//
// \param matrix The matrix containing the row.
// \param index The index of the row.
// \exception std::invalid_argument Invalid row access index.
*/
template< typename MT >  // Type of the sparse matrix
inline SparseRow<MT,false>::SparseRow( MT& matrix, size_t index )
   : matrix_( matrix )  // The sparse matrix containing the row
   , row_   ( index  )  // The index of the row in the matrix
{
   if( matrix_.rows() <= index )
      throw std::invalid_argument( "Invalid row access index" );
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  DATA ACCESS FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Subscript operator for the direct access to the row elements.
//
// \param index Access index. The index must be smaller than the number of matrix columns.
// \return Reference to the accessed value.
*/
template< typename MT >  // Type of the sparse matrix
inline typename SparseRow<MT,false>::Reference SparseRow<MT,false>::operator[]( size_t index )
{
   BLAZE_USER_ASSERT( index < size(), "Invalid row access index" );
   return matrix_(row_,index);
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Subscript operator for the direct access to the row elements.
//
// \param index Access index. The index must be smaller than the number of matrix columns.
// \return Reference to the accessed value.
*/
template< typename MT >  // Type of the sparse matrix
inline typename SparseRow<MT,false>::ConstReference SparseRow<MT,false>::operator[]( size_t index ) const
{
   BLAZE_USER_ASSERT( index < size(), "Invalid row access index" );
   return const_cast<const MT&>( matrix_ )(row_,index);
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator to the first element of the row.
//
// \return Iterator to the first element of the row.
//
// This function returns an iterator to the first element of the row.
*/
template< typename MT >  // Type of the sparse matrix
inline typename SparseRow<MT,false>::Iterator SparseRow<MT,false>::begin()
{
   return Iterator( matrix_, row_, 0UL );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator to the first element of the row.
//
// \return Iterator to the first element of the row.
//
// This function returns an iterator to the first element of the row.
*/
template< typename MT >  // Type of the sparse matrix
inline typename SparseRow<MT,false>::ConstIterator SparseRow<MT,false>::begin() const
{
   return ConstIterator( matrix_, row_, 0UL );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator to the first element of the row.
//
// \return Iterator to the first element of the row.
//
// This function returns an iterator to the first element of the row.
*/
template< typename MT >  // Type of the sparse matrix
inline typename SparseRow<MT,false>::ConstIterator SparseRow<MT,false>::cbegin() const
{
   return ConstIterator( matrix_, row_, 0UL );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator just past the last element of the row.
//
// \return Iterator just past the last element of the row.
//
// This function returns an iterator just past the last element of the row.
*/
template< typename MT >  // Type of the sparse matrix
inline typename SparseRow<MT,false>::Iterator SparseRow<MT,false>::end()
{
   return Iterator( matrix_, row_, size() );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator just past the last element of the row.
//
// \return Iterator just past the last element of the row.
//
// This function returns an iterator just past the last element of the row.
*/
template< typename MT >  // Type of the sparse matrix
inline typename SparseRow<MT,false>::ConstIterator SparseRow<MT,false>::end() const
{
   return ConstIterator( matrix_, row_, size() );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns an iterator just past the last element of the row.
//
// \return Iterator just past the last element of the row.
//
// This function returns an iterator just past the last element of the row.
*/
template< typename MT >  // Type of the sparse matrix
inline typename SparseRow<MT,false>::ConstIterator SparseRow<MT,false>::cend() const
{
   return ConstIterator( matrix_, row_, size() );
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ASSIGNMENT OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Copy assignment operator for SparseRow.
//
// \param rhs Sparse row to be copied.
// \return Reference to the assigned row.
// \exception std::invalid_argument Row sizes do not match.
//
// In case the current sizes of the two rows don't match, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT >  // Type of the sparse matrix
inline SparseRow<MT,false>& SparseRow<MT,false>::operator=( const SparseRow& rhs )
{
   using blaze::assign;

   if( this == &rhs || ( &matrix_ == &rhs.matrix_ && row_ == rhs.row_ ) )
      return *this;

   if( size() != rhs.size() )
      throw std::invalid_argument( "Row sizes do not match" );

   if( rhs.canAlias( &matrix_ ) ) {
      const ResultType tmp( rhs );
      assign( *this, tmp );
   }
   else {
      assign( *this, rhs );
   }

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Assignment operator for different vectors.
//
// \param rhs Vector to be assigned.
// \return Reference to the assigned row.
// \exception std::invalid_argument Vector sizes do not match.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument
// exception is thrown.
*/
template< typename MT >  // Type of the sparse matrix
template< typename VT >  // Type of the right-hand side vector
inline SparseRow<MT,false>& SparseRow<MT,false>::operator=( const Vector<VT,true>& rhs )
{
   using blaze::assign;

   if( size() != (~rhs).size() )
      throw std::invalid_argument( "Vector sizes do not match" );

   const typename VT::CompositeType tmp( ~rhs );
   assign( *this, tmp );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Addition assignment operator for the addition of a vector (\f$ \vec{a}+=\vec{b} \f$).
//
// \param rhs The right-hand side vector to be added to the sparse row.
// \return Reference to the sparse row.
// \exception std::invalid_argument Vector sizes do not match.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename MT >  // Type of the sparse matrix
template< typename VT >  // Type of the right-hand side vector
inline SparseRow<MT,false>& SparseRow<MT,false>::operator+=( const Vector<VT,true>& rhs )
{
   using blaze::addAssign;

   if( (~rhs).size() != size() )
      throw std::invalid_argument( "Vector sizes do not match" );

   addAssign( *this, ~rhs );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Subtraction assignment operator for the subtraction of a vector (\f$ \vec{a}-=\vec{b} \f$).
//
// \param rhs The right-hand side vector to be subtracted from the sparse row.
// \return Reference to the sparse row.
// \exception std::invalid_argument Vector sizes do not match.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename MT >  // Type of the sparse matrix
template< typename VT >  // Type of the right-hand side vector
inline SparseRow<MT,false>& SparseRow<MT,false>::operator-=( const Vector<VT,true>& rhs )
{
   using blaze::subAssign;

   if( (~rhs).size() != size() )
      throw std::invalid_argument( "Vector sizes do not match" );

   subAssign( *this, ~rhs );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Multiplication assignment operator for the multiplication of a vector
//        (\f$ \vec{a}*=\vec{b} \f$).
//
// \param rhs The right-hand side vector to be multiplied with the sparse row.
// \return Reference to the sparse row.
// \exception std::invalid_argument Vector sizes do not match.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename MT >  // Type of the sparse matrix
template< typename VT >  // Type of the right-hand side vector
inline SparseRow<MT,false>& SparseRow<MT,false>::operator*=( const Vector<VT,true>& rhs )
{
   if( (~rhs).size() != size() )
      throw std::invalid_argument( "Vector sizes do not match" );

   typedef typename MultTrait<This,typename VT::ResultType>::Type  MultType;

   BLAZE_CONSTRAINT_MUST_BE_TRANSPOSE_VECTOR_TYPE( MultType );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION  ( MultType );

   const MultType tmp( *this * (~rhs) );
   assign( tmp );

   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Multiplication assignment operator for the multiplication between a sparse row
//        and a scalar value (\f$ \vec{a}*=s \f$).
//
// \param rhs The right-hand side scalar value for the multiplication.
// \return Reference to the sparse row.
//
// This operator can only be used for built-in data types. Additionally, the elements of
// the sparse row must support the multiplication assignment operator for the given scalar
// built-in data type.
*/
template< typename MT >     // Type of the sparse matrix
template< typename Other >  // Data type of the right-hand side scalar
inline typename EnableIf< IsNumeric<Other>, SparseRow<MT,false> >::Type&
   SparseRow<MT,false>::operator*=( Other rhs )
{
   for( Iterator element=begin(); element!=end(); ++element )
      element->value() *= rhs;
   return *this;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Division assignment operator for the division of a sparse row by a scalar value
//        (\f$ \vec{a}/=s \f$).
//
// \param rhs The right-hand side scalar value for the division.
// \return Reference to the sparse row.
//
// This operator can only be used for built-in data types. Additionally, the elements of the
// sparse row must either support the multiplication assignment operator for the given
// floating point data type or the division assignment operator for the given integral data
// type.
*/
template< typename MT >     // Type of the sparse matrix
template< typename Other >  // Data type of the right-hand side scalar
inline typename EnableIf< IsNumeric<Other>, SparseRow<MT,false> >::Type&
   SparseRow<MT,false>::operator/=( Other rhs )
{
   BLAZE_USER_ASSERT( rhs != Other(0), "Division by zero detected" );

   typedef typename DivTrait<ElementType,Other>::Type  DT;
   typedef typename If< IsNumeric<DT>, DT, Other >::Type  Tmp;

   // Depending on the two involved data types, an integer division is applied or a
   // floating point division is selected.
   if( IsNumeric<DT>::value && IsFloatingPoint<DT>::value ) {
      const Tmp tmp( Tmp(1)/static_cast<Tmp>( rhs ) );
      for( Iterator element=begin(); element!=end(); ++element )
         element->value() *= tmp;
   }
   else {
      for( Iterator element=begin(); element!=end(); ++element )
         element->value() /= rhs;
   }

   return *this;
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns the current size/dimension of the row.
//
// \return The size of the row.
*/
template< typename MT >  // Type of the sparse matrix
inline size_t SparseRow<MT,false>::size() const
{
   return matrix_.columns();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns the maximum capacity of the sparse row.
//
// \return The capacity of the sparse row.
*/
template< typename MT >  // Type of the sparse matrix
inline size_t SparseRow<MT,false>::capacity() const
{
   return matrix_.columns();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns the number of non-zero elements in the row.
//
// \return The number of non-zero elements in the row.
//
// Note that the number of non-zero elements is always less than or equal to the current number
// of columns of the matrix containing the row.
*/
template< typename MT >  // Type of the sparse matrix
inline size_t SparseRow<MT,false>::nonZeros() const
{
   size_t counter( 0UL );
   for( ConstIterator element=begin(); element!=end(); ++element ) {
      ++counter;
   }
   return counter;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Reset to the default initial values.
//
// \return void
*/
template< typename MT >  // Type of the sparse matrix
inline void SparseRow<MT,false>::reset()
{
   for( size_t j=0UL; j<size(); ++j ) {
      matrix_.erase( row_, j );
   }
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Inserting an element into the sparse row.
//
// \param index The index of the new element. The index has to be in the range \f$[0..N-1]\f$.
// \param value The value of the element to be inserted.
// \return Reference to the inserted value.
// \exception std::invalid_argument Invalid sparse row access index.
//
// This function inserts a new element into the sparse row. However, duplicate elements
// are not allowed. In case the sparse row already contains an element at index \a index,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Type of the sparse matrix
inline typename SparseRow<MT,false>::ElementType&
   SparseRow<MT,false>::insert( size_t index, const ElementType& value )
{
   return matrix_.insert( row_, index, value )->value();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Erasing an element from the sparse row.
//
// \param index The index of the element to be erased. The index has to be in the range \f$[0..N-1]\f$.
// \return void
//
// This function erases an element from the sparse row.
*/
template< typename MT >  // Type of the sparse matrix
inline void SparseRow<MT,false>::erase( size_t index )
{
   matrix_.erase( row_, index );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Erasing an element from the sparse row.
//
// \param index The index of the element to be erased. The index has to be in the range \f$[0..N-1]\f$.
// \return void
//
// This function erases an element from the sparse row.
*/
template< typename MT >  // Type of the sparse matrix
inline typename SparseRow<MT,false>::Iterator SparseRow<MT,false>::erase( Iterator pos )
{
   const size_t column( pos.column_ );

   if( column == size() )
      return pos;

   matrix_.erase( column, pos.pos_ );
   return Iterator( matrix_, row_, column+1UL );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Searches for a specific row element.
//
// \param index The index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \return Iterator to the element in case the index is found, end() iterator otherwise.
//
// This function can be used to check whether a specific element is contained in the sparse
// row. It specifically searches for the element with index \a index. In case the element is
// found, the function returns an iterator to the element. Otherwise an iterator just past
// the last non-zero element of the sparse row (the end() iterator) is returned. Note that
// the returned sparse row iterator is subject to invalidation due to inserting operations
// via the subscript operator or the insert() function!
*/
template< typename MT >  // Type of the sparse matrix
inline typename SparseRow<MT,false>::Iterator SparseRow<MT,false>::find( size_t index )
{
   const typename MT::Iterator pos( matrix_.find( row_, index ) );

   if( pos != matrix_.end( index ) )
      return Iterator( matrix_, row_, index, pos );
   else
      return end();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Searches for a specific row element.
//
// \param index The index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \return Iterator to the element in case the index is found, end() iterator otherwise.
//
// This function can be used to check whether a specific element is contained in the sparse
// row. It specifically searches for the element with index \a index. In case the element is
// found, the function returns an iterator to the element. Otherwise an iterator just past
// the last non-zero element of the sparse row (the end() iterator) is returned. Note that
// the returned sparse row iterator is subject to invalidation due to inserting operations
// via the subscript operator or the insert() function!
*/
template< typename MT >  // Type of the sparse matrix
inline typename SparseRow<MT,false>::ConstIterator SparseRow<MT,false>::find( size_t index ) const
{
   const typename MT::ConstIterator pos( matrix_.find( row_, index ) );

   if( pos != matrix_.end( index ) )
      return ConstIterator( matrix_, row_, index, pos );
   else
      return end();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Setting the minimum capacity of the sparse row.
//
// \param n The new minimum capacity of the sparse row.
// \return void
//
// This function increases the capacity of the sparse row to at least \a n elements. The
// current values of the row elements are preserved.
*/
template< typename MT >  // Type of the sparse matrix
void SparseRow<MT,false>::reserve( size_t n )
{
   return;
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Scaling of the sparse row by the scalar value \a scalar (\f$ \vec{a}=\vec{b}*s \f$).
//
// \param scalar The scalar value for the row scaling.
// \return Reference to the sparse row.
*/
template< typename MT >     // Type of the sparse matrix
template< typename Other >  // Data type of the scalar value
inline SparseRow<MT,false>& SparseRow<MT,false>::scale( Other scalar )
{
   for( Iterator element=begin(); element!=end(); ++element )
      element->value() *= scalar;
   return *this;
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  LOW-LEVEL UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Appending an element to the sparse row.
//
// \param index The index of the new element. The index must be smaller than the number of matrix columns.
// \param value The value of the element to be appended.
// \param check \a true if the new value should be checked for default values, \a false if not.
// \return void
//
// This function provides a very efficient way to fill a sparse row with elements. It appends
// a new element to the end of the sparse row without any memory allocation. Therefore it is
// strictly necessary to keep the following preconditions in mind:
//
//  - the index of the new element must be strictly larger than the largest index of non-zero
//    elements in the sparse row
//  - the current number of non-zero elements must be smaller than the capacity of the row
//
// Ignoring these preconditions might result in undefined behavior! The optional \a check
// parameter specifies whether the new value should be tested for a default value. If the new
// value is a default value (for instance 0 in case of an integral element type) the value is
// not appended. Per default the values are not tested.
//
// \b Note: Although append() does not allocate new memory, it still invalidates all iterators
// returned by the end() functions!
*/
template< typename MT >  // Type of the sparse matrix
inline void SparseRow<MT,false>::append( size_t index, const ElementType& value, bool /*check*/ )
{
   matrix_.insert( row_, index, value );
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  EXPRESSION TEMPLATE EVALUATION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the sparse row can alias with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this sparse row, \a false if not.
//
// This function returns whether the given address can alias with the sparse row. In contrast
// to the isAliased() function this function is allowed to use compile time expressions to
// optimize the evaluation.
*/
template< typename MT >     // Type of the sparse matrix
template< typename Other >  // Data type of the foreign expression
inline bool SparseRow<MT,false>::canAlias( const Other* alias ) const
{
   return static_cast<const void*>( &matrix_ ) == static_cast<const void*>( alias );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Returns whether the sparse row is aliased with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this row, \a false if not.
*/
template< typename MT >     // Type of the sparse matrix
template< typename Other >  // Data type of the foreign expression
inline bool SparseRow<MT,false>::isAliased( const Other* alias ) const
{
   return static_cast<const void*>( &matrix_ ) == static_cast<const void*>( alias );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the sparse matrix
template< typename VT >  // Type of the right-hand side dense vector
inline void SparseRow<MT,false>::assign( const DenseVector<VT,true>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   for( size_t j=0UL; j<(~rhs).size(); ++j ) {
      if( !isDefault( (~rhs)[j] ) ) {
         matrix_(row_,j) = (~rhs)[j];
      }
      else {
         matrix_.erase( row_, j );
      }
   }
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the sparse matrix
template< typename VT >  // Type of the right-hand side sparse vector
inline void SparseRow<MT,false>::assign( const SparseVector<VT,true>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   size_t j( 0UL );

   for( typename VT::ConstIterator element=(~rhs).begin(); element!=(~rhs).end(); ++element ) {
      for( ; j<element->index(); ++j )
         matrix_.erase( row_, j );
      matrix_(row_,j++) = element->value();
   }
   for( ; j<size(); ++j ) {
      matrix_.erase( row_, j );
   }
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the addition assignment of a vector.
//
// \param rhs The right-hand side vector to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the sparse matrix
template< typename VT >  // Type of the right-hand side vector
inline void SparseRow<MT,false>::addAssign( const Vector<VT,true>& rhs )
{
   typedef typename AddTrait<This,typename VT::ResultType>::Type  AddType;

   BLAZE_CONSTRAINT_MUST_BE_TRANSPOSE_VECTOR_TYPE( AddType );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION  ( AddType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const AddType tmp( *this + (~rhs) );
   assign( tmp );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Default implementation of the subtraction assignment of a vector.
//
// \param rhs The right-hand side vector to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT >  // Type of the sparse matrix
template< typename VT >  // Type of the right-hand side vector
inline void SparseRow<MT,false>::subAssign( const Vector<VT,true>& rhs )
{
   typedef typename SubTrait<This,typename VT::ResultType>::Type  SubType;

   BLAZE_CONSTRAINT_MUST_BE_TRANSPOSE_VECTOR_TYPE( SubType );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION  ( SubType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const SubType tmp( *this - (~rhs) );
   assign( tmp );
}
/*! \endcond */
//*************************************************************************************************








//=================================================================================================
//
//  SPARSEROW OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\name SparseRow operators */
//@{
template< typename MT, bool SO >
inline void reset( SparseRow<MT,SO>& row );

template< typename MT, bool SO >
inline void clear( SparseRow<MT,SO>& row );

template< typename MT, bool SO >
inline bool isnan( const SparseRow<MT,SO>& row );

template< typename MT, bool SO >
inline bool isDefault( const SparseRow<MT,SO>& row );
//@}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Resetting the given sparse row.
// \ingroup sparse_row
//
// \param row The sparse row to be resetted.
// \return void
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline void reset( SparseRow<MT,SO>& row )
{
   row.reset();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Clearing the given sparse row.
// \ingroup sparse_row
//
// \param row The sparse row to be cleared.
// \return void
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline void clear( SparseRow<MT,SO>& row )
{
   row.reset();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Checks the given sparse row for not-a-number elements.
// \ingroup sparse_row
//
// \param row The sparse row to be checked for not-a-number elements.
// \return \a true if at least one element of the row is not-a-number, \a false otherwise.
//
// This function checks the sparse row for not-a-number (NaN) elements. If at least one element
// of the row is not-a-number, the function returns \a true, otherwise it returns \a false.

   \code
   blaze::CompressedMatrix<double,rowMajor> A;
   // ... Resizing and initialization
   if( isnan( row( A, 0UL ) ) ) { ... }
   \endcode
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline bool isnan( const SparseRow<MT,SO>& row )
{
   typedef typename SparseRow<MT,SO>::ConstIterator  ConstIterator;

   const ConstIterator end( row.end() );
   for( ConstIterator element=row.begin(); element!=end; ++element ) {
      if( isnan( element->value() ) ) return true;
   }
   return false;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns whether the given sparse row is in default state.
// \ingroup sparse_row
//
// \param row The sparse row to be tested for its default state.
// \return \a true in case the given row is component-wise zero, \a false otherwise.
//
// This function checks whether the sparse row is in default state. For instance, in case the
// row is instantiated for a built-in integral or floating point data type, the function returns
// \a true in case all row elements are 0 and \a false in case any vector element is not 0. The
// following example demonstrates the use of the \a isDefault function:

   \code
   blaze::CompressedMatrix<double,rowMajor> A;
   // ... Resizing and initialization
   if( isDefault( row( A, 0UL ) ) ) { ... }
   \endcode
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline bool isDefault( const SparseRow<MT,SO>& row )
{
   typedef typename SparseRow<MT,SO>::ConstIterator  ConstIterator;

   const ConstIterator end( row.end() );
   for( ConstIterator element=row.begin(); element!=end; ++element )
      if( !isDefault( element->value() ) ) return false;
   return true;
}
//*************************************************************************************************




//=================================================================================================
//
//  GLOBAL OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Creating a view on a specific row of the given sparse matrix.
// \ingroup views
//
// \param sm The sparse matrix containing the row.
// \param index The index of the row.
// \return View on the specified row of the matrix.
// \exception std::invalid_argument Invalid row access index.
//
// This function returns an expression representing the specified row of the given sparse matrix.

   \code
   typedef blaze::CompressedMatrix<double,rowMajor>  Matrix;

   Matrix A;
   // ... Resizing and initialization
   blaze::SparseRow<Matrix> = row( A, 3UL );
   \endcode
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline SparseRow<MT> row( SparseMatrix<MT,SO>& sm, size_t index )
{
   BLAZE_FUNCTION_TRACE;

   return SparseRow<MT>( ~sm, index );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Creating a view on a specific row of the given constant sparse matrix.
// \ingroup views
//
// \param sm The constant sparse matrix containing the row.
// \param index The index of the row.
// \return View on the specified row of the matrix.
// \exception std::invalid_argument Invalid row access index.
//
// This function returns an expression representing the specified row of the given sparse matrix.

   \code
   typedef blaze::CompressedMatrix<double,rowMajor>  Matrix;

   const Matrix A;
   // ... Resizing and initialization
   blaze::SparseRow<const Matrix> = row( A, 3UL );
   \endcode
*/
template< typename MT  // Type of the sparse matrix
        , bool SO >    // Storage order
inline SparseRow<const MT> row( const SparseMatrix<MT,SO>& sm, size_t index )
{
   BLAZE_FUNCTION_TRACE;

   return SparseRow<const MT>( ~sm, index );
}
//*************************************************************************************************




//=================================================================================================
//
//  ADDTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T1, bool SO, typename T2, size_t N >
struct AddTrait< SparseRow<T1,SO>, StaticVector<T2,N,true> >
{
   typedef typename AddTrait< typename SparseRow<T1,SO>::ResultType,
                              StaticVector<T2,N,true> >::Type  Type;
};

template< typename T1, size_t N, typename T2, bool SO >
struct AddTrait< StaticVector<T1,N,true>, SparseRow<T2,SO> >
{
   typedef typename AddTrait< StaticVector<T1,N,true>,
                              typename SparseRow<T2,SO>::ResultType >::Type  Type;
};

template< typename T1, bool SO, typename T2 >
struct AddTrait< SparseRow<T1,SO>, DynamicVector<T2,true> >
{
   typedef typename AddTrait< typename SparseRow<T1,SO>::ResultType,
                              DynamicVector<T2,true> >::Type  Type;
};

template< typename T1, typename T2, bool SO >
struct AddTrait< DynamicVector<T1,true>, SparseRow<T2,SO> >
{
   typedef typename AddTrait< DynamicVector<T1,true>,
                              typename SparseRow<T2,SO>::ResultType >::Type  Type;
};

template< typename T1, bool SO1, typename T2, bool SO2 >
struct AddTrait< SparseRow<T1,SO1>, DenseRow<T2,SO2> >
{
   typedef typename AddTrait< typename SparseRow<T1,SO1>::ResultType,
                              typename DenseRow <T2,SO2>::ResultType >::Type  Type;
};

template< typename T1, bool SO1, typename T2, bool SO2 >
struct AddTrait< DenseRow<T1,SO1>, SparseRow<T2,SO2> >
{
   typedef typename AddTrait< typename DenseRow <T1,SO1>::ResultType,
                              typename SparseRow<T2,SO2>::ResultType >::Type  Type;
};

template< typename T1, bool SO, typename T2 >
struct AddTrait< SparseRow<T1,SO>, CompressedVector<T2,true> >
{
   typedef typename AddTrait< typename SparseRow<T1,SO>::ResultType,
                              CompressedVector<T2,true> >::Type  Type;
};

template< typename T1, typename T2, bool SO >
struct AddTrait< CompressedVector<T1,true>, SparseRow<T2,SO> >
{
   typedef typename AddTrait< CompressedVector<T1,true>,
                              typename SparseRow<T2,SO>::ResultType >::Type  Type;
};

template< typename T1, bool SO1, typename T2, bool SO2 >
struct AddTrait< SparseRow<T1,SO1>, SparseRow<T2,SO2> >
{
   typedef typename AddTrait< typename SparseRow<T1,SO1>::ResultType,
                              typename SparseRow<T2,SO2>::ResultType >::Type  Type;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  SUBTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T1, bool SO, typename T2, size_t N >
struct SubTrait< SparseRow<T1,SO>, StaticVector<T2,N,true> >
{
   typedef typename SubTrait< typename SparseRow<T1,SO>::ResultType,
                              StaticVector<T2,N,true> >::Type  Type;
};

template< typename T1, size_t N, typename T2, bool SO >
struct SubTrait< StaticVector<T1,N,true>, SparseRow<T2,SO> >
{
   typedef typename SubTrait< StaticVector<T1,N,true>,
                              typename SparseRow<T2,SO>::ResultType >::Type  Type;
};

template< typename T1, bool SO, typename T2 >
struct SubTrait< SparseRow<T1,SO>, DynamicVector<T2,true> >
{
   typedef typename SubTrait< typename SparseRow<T1,SO>::ResultType,
                              DynamicVector<T2,true> >::Type  Type;
};

template< typename T1, typename T2, bool SO >
struct SubTrait< DynamicVector<T1,true>, SparseRow<T2,SO> >
{
   typedef typename SubTrait< DynamicVector<T1,true>,
                              typename SparseRow<T2,SO>::ResultType >::Type  Type;
};

template< typename T1, bool SO1, typename T2, bool SO2 >
struct SubTrait< SparseRow<T1,SO1>, DenseRow<T2,SO2> >
{
   typedef typename SubTrait< typename SparseRow<T1,SO1>::ResultType,
                              typename DenseRow <T2,SO2>::ResultType >::Type  Type;
};

template< typename T1, bool SO1, typename T2, bool SO2 >
struct SubTrait< DenseRow<T1,SO1>, SparseRow<T2,SO2> >
{
   typedef typename SubTrait< typename DenseRow <T1,SO1>::ResultType,
                              typename SparseRow<T2,SO2>::ResultType >::Type  Type;
};

template< typename T1, bool SO, typename T2 >
struct SubTrait< SparseRow<T1,SO>, CompressedVector<T2,true> >
{
   typedef typename SubTrait< typename SparseRow<T1,SO>::ResultType,
                              CompressedVector<T2,true> >::Type  Type;
};

template< typename T1, typename T2, bool SO >
struct SubTrait< CompressedVector<T1,true>, SparseRow<T2,SO> >
{
   typedef typename SubTrait< CompressedVector<T1,true>,
                              typename SparseRow<T2,SO>::ResultType >::Type  Type;
};

template< typename T1, bool SO1, typename T2, bool SO2 >
struct SubTrait< SparseRow<T1,SO1>, SparseRow<T2,SO2> >
{
   typedef typename SubTrait< typename SparseRow<T1,SO1>::ResultType,
                              typename SparseRow<T2,SO2>::ResultType >::Type  Type;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  MULTTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T1, bool SO, typename T2 >
struct MultTrait< SparseRow<T1,SO>, T2 >
{
   typedef typename MultTrait< typename SparseRow<T1,SO>::ResultType, T2 >::Type  Type;
   BLAZE_CONSTRAINT_MUST_BE_NUMERIC_TYPE( T2 );
};

template< typename T1, typename T2, bool SO >
struct MultTrait< T1, SparseRow<T2,SO> >
{
   typedef typename MultTrait< T1, typename SparseRow<T2,SO>::ResultType >::Type  Type;
   BLAZE_CONSTRAINT_MUST_BE_NUMERIC_TYPE( T1 );
};

template< typename T1, bool SO, typename T2, size_t N, bool TF >
struct MultTrait< SparseRow<T1,SO>, StaticVector<T2,N,TF> >
{
   typedef typename MultTrait< typename SparseRow<T1,SO>::ResultType,
                               StaticVector<T2,N,TF> >::Type  Type;
};

template< typename T1, size_t N, bool TF, typename T2, bool SO >
struct MultTrait< StaticVector<T1,N,TF>, SparseRow<T2,SO> >
{
   typedef typename MultTrait< StaticVector<T1,N,TF>,
                               typename SparseRow<T2,SO>::ResultType >::Type  Type;
};

template< typename T1, bool SO, typename T2, bool TF >
struct MultTrait< SparseRow<T1,SO>, DynamicVector<T2,TF> >
{
   typedef typename MultTrait< typename SparseRow<T1,SO>::ResultType,
                               DynamicVector<T2,TF> >::Type  Type;
};

template< typename T1, bool TF, typename T2, bool SO >
struct MultTrait< DynamicVector<T1,TF>, SparseRow<T2,SO> >
{
   typedef typename MultTrait< DynamicVector<T1,TF>,
                               typename SparseRow<T2,SO>::ResultType >::Type  Type;
};

template< typename T1, bool SO1, typename T2, bool SO2 >
struct MultTrait< SparseRow<T1,SO1>, DenseRow<T2,SO2> >
{
   typedef typename MultTrait< typename SparseRow<T1,SO1>::ResultType,
                               typename DenseRow <T2,SO2>::ResultType >::Type  Type;
};

template< typename T1, bool SO1, typename T2, bool SO2 >
struct MultTrait< DenseRow<T1,SO1>, SparseRow<T2,SO2> >
{
   typedef typename MultTrait< typename DenseRow <T1,SO1>::ResultType,
                               typename SparseRow<T2,SO2>::ResultType >::Type  Type;
};

template< typename T1, bool SO, typename T2, bool TF >
struct MultTrait< SparseRow<T1,SO>, CompressedVector<T2,TF> >
{
   typedef typename MultTrait< typename SparseRow<T1,SO>::ResultType,
                               CompressedVector<T2,TF> >::Type  Type;
};

template< typename T1, bool TF, typename T2, bool SO >
struct MultTrait< CompressedVector<T1,TF>, SparseRow<T2,SO> >
{
   typedef typename MultTrait< CompressedVector<T1,TF>,
                               typename SparseRow<T2,SO>::ResultType >::Type  Type;
};

template< typename T1, bool SO1, typename T2, bool SO2 >
struct MultTrait< SparseRow<T1,SO1>, SparseRow<T2,SO2> >
{
   typedef typename MultTrait< typename SparseRow<T1,SO1>::ResultType,
                               typename SparseRow<T2,SO2>::ResultType >::Type  Type;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  DIVTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T1, bool SO, typename T2 >
struct DivTrait< SparseRow<T1,SO>, T2 >
{
   typedef typename DivTrait< typename SparseRow<T1,SO>::ResultType, T2 >::Type  Type;
   BLAZE_CONSTRAINT_MUST_BE_NUMERIC_TYPE( T2 );
};
/*! \endcond */
//*************************************************************************************************

} // namespace blaze

#endif
