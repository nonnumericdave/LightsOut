//
//  DAFLightsOutSolver.h
//  LightsOut
//
//  Created by David Flores on 5/11/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#ifndef LightsOutSolver_h
#define LightsOutSolver_h

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
namespace DAF
{
	class LightsOutSolver
	{
	public:
		// LightsOutSolver
		LightsOutSolver(std::size_t uDimension);
		
		bool Solve(const std::vector<bool>& vbStateMatrix, std::vector<bool>& vbOptimalSolutionMatrix);
		
	private:
		// LightsOutSolver
		std::size_t ConvertToLinearIndex(std::size_t uDimension, std::size_t uRow, std::size_t uColumn);
		void SwapRows(std::size_t uDimension, std::vector<bool>& vbMatrix, size_t uRowX, size_t uRowY);
		void AddRowToRow(std::size_t uDimension, std::vector<bool>& vbMatrix, size_t uRowSrc, size_t uRowDest);
		std::vector<bool> MatrixVectorProduct(std::size_t uDimension,
											  const std::vector<bool>& vbMatrix,
											  const std::vector<bool>& vbVector);
		bool VectorVectorScalarProduct(std::size_t uDimension,
									   const std::vector<bool>& vbVectorX,
									   const std::vector<bool>& vbVectorY);
		std::vector<bool> VectorVectorSum(std::size_t uDimension,
										  const std::vector<bool>& vbVectorX,
										  const std::vector<bool>& vbVectorY);
		std::vector<bool> IdentityMatrix();
		std::vector<bool> GameMatrix();
		void ReduceGameMatrix();
		void FindNullSpaceBasis();
		std::string PrintMatrix(std::size_t uDimension, const std::vector<bool>& kvectorMatrix);
		
		std::size_t _uDimension;
		std::vector<bool> _vbElementaryOperationMatrix;
		std::vector<bool> _vbReducedGameMatrix;
		std::vector<std::vector<bool> > _vvbGameMatrixNullSpaceBasis;
	};
};

#endif
