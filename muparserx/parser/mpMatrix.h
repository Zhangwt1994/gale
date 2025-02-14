#ifndef MU_MATRIX_H
#define MU_MATRIX_H

#include <vector>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include "mpMatrixError.h"
#include <algorithm>


MUP_NAMESPACE_START

  //-----------------------------------------------------------------------------------------------
  template<class T>
  class Matrix  
  {
  template<class T2>
  friend Matrix operator+(const Matrix& lhs, const Matrix& rhs);

  template<class T2>
  friend Matrix operator-(const Matrix& lhs, const Matrix& rhs);

  public:

    //---------------------------------------------------------------------------------------------
    enum EMatrixStorageScheme
    {
      mssROWS_FIRST,
      mssCOLS_FIRST
    };

    //---------------------------------------------------------------------------------------------
    Matrix()
      :m_nCols(1)
      ,m_nRows(1)
      ,m_vData(1)
      ,m_eStorageScheme(mssROWS_FIRST)
    {}

    //---------------------------------------------------------------------------------------------
    Matrix(int nRows, const T &value = T())
      :m_nCols(1)
      ,m_nRows(nRows)
      ,m_vData(m_nRows, value)
      ,m_eStorageScheme(mssROWS_FIRST)
    {}

    //---------------------------------------------------------------------------------------------
    /* \brief Constructs a Matrix object representing a scalar value
    */
    Matrix(const T &v)
      :m_nCols(1)
      ,m_nRows(1)
      ,m_vData(1, v)
      ,m_eStorageScheme(mssROWS_FIRST)
    {}

    //---------------------------------------------------------------------------------------------
    /* \brief Constructs a Matrix object representing a vector
    */
    Matrix(const std::vector<T> &v)
      :m_nCols(1)
      ,m_nRows(v.size())
      ,m_vData(v)
      ,m_eStorageScheme(mssROWS_FIRST)
    {}

    //---------------------------------------------------------------------------------------------
    /* Constructs a Matrix object representing a vector
    */
    template<size_t TSize>
    Matrix(T (&v)[TSize])
      :m_nCols(1)
      ,m_nRows(TSize)
      ,m_vData(v, v + TSize)
      ,m_eStorageScheme(mssROWS_FIRST)
    {}

    //---------------------------------------------------------------------------------------------
    template<size_t TRows, size_t TCols>
    Matrix(T (&v)[TRows][TCols])
      :m_nCols(TCols)
      ,m_nRows(TRows)
      ,m_vData(TRows*TCols, 0)
      ,m_eStorageScheme(mssROWS_FIRST)
    {
      for (int m=0; m<TRows; ++m)
      {
        for (int n=0; n<TCols; ++n)
        {
          At(m,n) = v[m][n];
        }
      }
    }

    //---------------------------------------------------------------------------------------------
    Matrix(int nRows, int nCols, const T &value = T())
      :m_nCols(nCols)
      ,m_nRows(nRows)
      ,m_vData(m_nRows*m_nCols, value)
      ,m_eStorageScheme(mssROWS_FIRST)
    {}

    //---------------------------------------------------------------------------------------------
    Matrix(const Matrix &ref)
    {
      Assign(ref);
    }

    //---------------------------------------------------------------------------------------------
    Matrix& operator=(const Matrix &ref)
    {
      if (this!=&ref)
        Assign(ref);

      return *this;
    }

    //---------------------------------------------------------------------------------------------
    Matrix& operator=(const T &v)
    {
      m_nCols = 1;
      m_nRows = 1;
      m_eStorageScheme = mssROWS_FIRST;
      m_vData.assign(1, v);
      return *this;
    }

    //---------------------------------------------------------------------------------------------
    Matrix& operator+=(const Matrix &lhs)
    {
      if (m_nRows!=lhs.m_nRows || m_nCols!=lhs.m_nCols)
        throw MatrixError("Matrix dimension mismatch");

      for (int i=0; i<m_nRows; ++i)
      {
        for (int j=0; j<m_nCols; ++j)
        {
          At(i,j) += lhs.At(i,j);
        }
      }

      return *this;
    }

    //---------------------------------------------------------------------------------------------
    Matrix& operator-=(const Matrix &lhs)
    {
      if (m_nRows!=lhs.m_nRows || m_nCols!=lhs.m_nCols)
        throw MatrixError("Matrix dimension mismatch");

      for (int i=0; i<m_nRows; ++i)
      {
        for (int j=0; j<m_nCols; ++j)
        {
          At(i,j) -= lhs.At(i,j);
        }
      }

      return *this;
    }

    //---------------------------------------------------------------------------------------------
    Matrix& operator*=(const T &rhs)
    {
      // Matrix x Matrix multiplication
      for (int m=0; m<m_nRows; ++m)
      {
        for (int n=0; n<m_nCols; ++n)
        {
          At(m, n) *= rhs;
        }
      }

      return *this;
    }

    //---------------------------------------------------------------------------------------------
    Matrix& operator*=(const Matrix &rhs)
    {
      // Matrix x Matrix multiplication
      if (rhs.GetRows()==0)
      {
        T v = rhs.At(0,0);
        for (int m=0; m<m_nRows; ++m)
        {
          for (int n=0; n<m_nCols; ++n)
          {
            At(m, n) *= v;
          }
        }
      }
      else if (GetRows()==0)
      {
        T v = At(0,0);
        Assign(rhs);
        for (int m=0; m<m_nRows; ++m)
        {
          for (int n=0; n<m_nCols; ++n)
          {
            At(m, n) *= v;
          }
        }
      }
      else if (m_nCols==rhs.m_nRows)
      {
        Matrix<T> out(m_nRows, rhs.m_nCols);
        
        // For each cell in the output matrix
        for (int m=0; m<m_nRows; ++m)
        {
          for (int n=0; n<rhs.m_nCols; ++n)
          {
            T buf = 0;
            for (int i=0; i<m_nCols; ++i)
            {
              buf += At(m, i) * rhs.At(i, n);
            }
            out.At(m,n) = buf;
          } // for all rows
        } // for all columns

        Assign(out);
      }
      else
        throw MatrixError("Matrix dimensions don't allow multiplication");
      
      return *this;
    }

    //---------------------------------------------------------------------------------------------
    void AsciiDump(const char *szTitle) const
    {
      using namespace std;

      cout << szTitle << _T("\n");
      cout << _T("------------------\n");
      cout << _T("Cols: ") << GetCols() << _T("\n");
      cout << _T("Rows: ") << GetRows() << _T("\n");
      cout << _T("Dim:  ") << GetDim()  << _T("\n");

      for (int i=0; i<m_nRows; ++i)
      {
        for (int j=0; j<m_nCols; ++j)
        {
          cout << At(i, j) << _T("  ");
        }
        cout << _T("\n");
      }

      cout << _T("\n\n");
    }

    //---------------------------------------------------------------------------------------------
    std::string ToString() const
    {
      std::stringstream ss;
      for (int i=0; i<m_nRows; ++i)
      {
        for (int j=0; j<m_nCols; ++j)
        {
          ss << At(i, j) << "  ";
        }
        ss << "\n";
      }

      return ss.str();
    }

    //---------------------------------------------------------------------------------------------
   ~Matrix()
    {
      m_vData.clear();
    }

    //---------------------------------------------------------------------------------------------
    int GetRows() const
    {
      return m_nRows;
    }

    //---------------------------------------------------------------------------------------------
    int GetCols() const
    {
      return m_nCols;
    }

    //---------------------------------------------------------------------------------------------
    int GetDim() const
    {
      if (m_nCols==1)
      {
        return (m_nRows==1) ? 0 : 1;
      }
      else
        return 2;
    }
  
    //---------------------------------------------------------------------------------------------
    T& At(int nRow, int nCol = 0)
    {
      int i;
      if (m_eStorageScheme==mssROWS_FIRST)
      {
        i = nRow * m_nCols + nCol;
      }
      else
      {
        i = nCol * m_nRows + nRow;
      }

      assert(i<(int)m_vData.size());
      return m_vData[i];
    }

    //---------------------------------------------------------------------------------------------
    const T& At(int nRow, int nCol = 0) const
    {
      int i;
      if (m_eStorageScheme==mssROWS_FIRST)
      {
        i = nRow * m_nCols + nCol;
      }
      else
      {
        i = nCol * m_nRows + nRow;
      }

      assert(i<(int)m_vData.size());
      return m_vData[i];
    }

    //---------------------------------------------------------------------------------------------
    const T* GetData() const
    {
      assert(m_vData.size());
      return &m_vData[0];
    }

    //---------------------------------------------------------------------------------------------
    void SetStorageScheme(EMatrixStorageScheme eScheme)
    {
      m_eStorageScheme = eScheme;
    }

    //---------------------------------------------------------------------------------------------
    EMatrixStorageScheme GetStorageScheme() const
    {
      return m_eStorageScheme;
    }

    //---------------------------------------------------------------------------------------------
    Matrix<T>& Transpose()
    {
      if (GetDim()==0)
        return *this;

      m_eStorageScheme = (m_eStorageScheme==mssROWS_FIRST) ? mssCOLS_FIRST : mssROWS_FIRST;
      std::swap(m_nRows,m_nCols);

      return *this;
    }

    //---------------------------------------------------------------------------------------------
    void Fill(const T &v)
    {
      m_vData.assign(m_vData.size(), v);
    }

  private:
    int m_nCols;
    int m_nRows;
    std::vector<T> m_vData;
    EMatrixStorageScheme m_eStorageScheme;

    //---------------------------------------------------------------------------------------------
    void Assign(const Matrix &ref)
    {
      m_nCols = ref.m_nCols;
      m_nRows = ref.m_nRows;
      m_eStorageScheme = ref.m_eStorageScheme;
      m_vData = ref.m_vData;
    }
  };

  //---------------------------------------------------------------------------------------------
  template<class T>
  Matrix<T> operator*(const Matrix<T>& lhs, const T& rhs)
  {
    return Matrix<T>(lhs) *= rhs;
  }

  //---------------------------------------------------------------------------------------------
  template<class T>
  Matrix<T> operator*(const Matrix<T>& lhs, const Matrix<T>& rhs)
  {
    return Matrix<T>(lhs) *= rhs;
  }

  //---------------------------------------------------------------------------------------------
  template<class T>
  Matrix<T> operator+(const Matrix<T>& lhs, const Matrix<T>& rhs)
  {
    return Matrix<T>(lhs) += rhs;
  }

  //---------------------------------------------------------------------------------------------
  template<class T>
  Matrix<T> operator-(const Matrix<T>& lhs, const Matrix<T>& rhs)
  {
    return Matrix<T>(lhs) -= rhs;
  }

MUP_NAMESPACE_END

#endif
