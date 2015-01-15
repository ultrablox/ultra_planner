
#ifndef UltraCore_UMatrix_h
#define UltraCore_UMatrix_h

#include <array>
#include <iostream>

using namespace std;

namespace UltraCore
{

	template<typename DataType, int Rows, int Columns>
	class UMatrix
	{
	public:
		typedef UMatrix<DataType, Rows, Columns> MatType;

		UMatrix()
		{}

		int rows() const
		{
			return Rows;
		}

		int columns() const
		{
			return Columns;
		}

		DataType & at(int row, int column)
		{
			return m_data[Columns * row + column];
		}

		const DataType & at(int row, int column) const
		{
			return m_data[Columns * row + column];
		}

		void print() const
		{
			for(int r = 0; r < Rows; ++r)
			{
				for(int c = 0; c < Columns; ++c)
				{
					cout << at(r, c) << " ";
				}

				cout << "\n";
			}
		}

		/*
		Simple multiplication with O(n^3) complexity.
		*/
		friend MatType mult(const MatType & m1, const MatType & m2)
		{
			MatType res;

			for(int r = 0; r < m1.rows(); ++r)
			{
				for(int c = 0; c < m2.columns(); ++c)
				{
					DataType sum(0);
					for(int i = 0; i < m1.columns(); ++i)
						sum += m1.at(r, i) * m2.at(i, c);

					res.at(r, c) = sum;
				}
			}
			return res;
		}

		/*
		Strassen multiplication with O(n^log(7)) complexity.
		*/
		friend MatType mult_strassen(const MatType & m1, const MatType & m2)
		{
			MatType res;
			return res;
		}

		friend bool operator==(const MatType & m1, const MatType & m2)
		{
			return m1.m_data == m2.m_data;
		}
	private:

		std::array<DataType, Rows*Columns> m_data;
	};

};

#endif
