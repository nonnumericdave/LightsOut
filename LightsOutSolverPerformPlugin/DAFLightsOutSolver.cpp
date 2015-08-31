//
//  DAFLightsOutSolver.cpp
//  LightsOut
//
//  Created by David Flores on 5/11/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#include "DAFLightsOutSolver.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
DAF::LightsOutSolver::LightsOutSolver(std::size_t uDimension) :
    _uDimension(uDimension)
{
    ReduceGameMatrix();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool
DAF::LightsOutSolver::Solve(const std::vector<bool>& kvectorStateMatrix, std::vector<bool>& vbOptimalSolutionMatrix)
{
    const std::size_t kuDimension = _uDimension * _uDimension;
    for (const std::vector<bool>& vbBasisVector : _vvbGameMatrixNullSpaceBasis )
    {
        if ( VectorVectorScalarProduct(kuDimension, vbBasisVector, kvectorStateMatrix) )
            return false;
    }
    
    const std::vector<bool> kvectorBasicSolutionMatrix = MatrixVectorProduct(kuDimension, _vbElementaryOperationMatrix, kvectorStateMatrix);
    
    const std::size_t kuNullSpaceBasisDimension = _vvbGameMatrixNullSpaceBasis.size();
    for (std::size_t uNullSpaceBasisIndex = 0; uNullSpaceBasisIndex < kuNullSpaceBasisDimension; ++uNullSpaceBasisIndex)
    {
        if ( VectorVectorScalarProduct(kuDimension, _vvbGameMatrixNullSpaceBasis[uNullSpaceBasisIndex], kvectorStateMatrix) )
            return false;
    }
    
    std::size_t kuOptimalSolutionMoveCount = kuDimension;
    std::vector<bool> vbGameMatrixNullSpaceBasisCombination(kuNullSpaceBasisDimension, false);
    for (;;)
    {
        std::vector<bool> vbCurrentSolutionMatrix = kvectorBasicSolutionMatrix;
        for (std::size_t uNullSpaceBasisIndex = 0; uNullSpaceBasisIndex < kuNullSpaceBasisDimension; ++uNullSpaceBasisIndex)
        {
            if ( vbGameMatrixNullSpaceBasisCombination[uNullSpaceBasisIndex] )
                vbCurrentSolutionMatrix = VectorVectorSum(kuDimension, vbCurrentSolutionMatrix, _vvbGameMatrixNullSpaceBasis[uNullSpaceBasisIndex]);
        }
        
        const std::size_t kuCurrentSolutionMoveCount = std::count(vbCurrentSolutionMatrix.begin(), vbCurrentSolutionMatrix.end(), true);
        if ( kuCurrentSolutionMoveCount < kuOptimalSolutionMoveCount )
        {
            vbOptimalSolutionMatrix = vbCurrentSolutionMatrix;
            kuOptimalSolutionMoveCount = kuCurrentSolutionMoveCount;
        }
        
        std::size_t uCombinationIndex = 0;
        for (; uCombinationIndex < kuNullSpaceBasisDimension; ++uCombinationIndex)
        {
            if ( ! vbGameMatrixNullSpaceBasisCombination[uCombinationIndex] )
            {
                vbGameMatrixNullSpaceBasisCombination[uCombinationIndex] = true;
                break;
            }
            
            vbGameMatrixNullSpaceBasisCombination[uCombinationIndex] = false;
        }
        
        if ( uCombinationIndex == kuNullSpaceBasisDimension )
            break;
    }
    
    return true;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
std::size_t
DAF::LightsOutSolver::ConvertToLinearIndex(std::size_t uDimension, std::size_t uRow, std::size_t uColumn)
{
    return uRow * uDimension + uColumn;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
DAF::LightsOutSolver::SwapRows(std::size_t uDimension, std::vector<bool>& vbMatrix, size_t uRowX, size_t uRowY)
{
    for (std::size_t uColumn = 0; uColumn < uDimension; ++uColumn)
    {
        const std::size_t kuRowColumnX = ConvertToLinearIndex(uDimension, uRowX, uColumn);
        const std::size_t kuRowColumnY = ConvertToLinearIndex(uDimension, uRowY, uColumn);
        
        const bool kbSwapTemp = vbMatrix[kuRowColumnX];
        vbMatrix[kuRowColumnX] = vbMatrix[kuRowColumnY];
        vbMatrix[kuRowColumnY] = kbSwapTemp;
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
DAF::LightsOutSolver::AddRowToRow(std::size_t uDimension, std::vector<bool>& vbMatrix, size_t uRowSrc, size_t uRowDest)
{
    for (std::size_t uColumn = 0; uColumn < uDimension; ++uColumn)
    {
        const std::size_t kuRowColumnSrc = ConvertToLinearIndex(uDimension, uRowSrc, uColumn);
        const std::size_t kuRowColumnDest = ConvertToLinearIndex(uDimension, uRowDest, uColumn);
        
        vbMatrix[kuRowColumnDest] = vbMatrix[kuRowColumnSrc] ^ vbMatrix[kuRowColumnDest];
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
std::vector<bool>
DAF::LightsOutSolver::MatrixVectorProduct(std::size_t uDimension,
                                          const std::vector<bool>& kvectorMatrix,
                                          const std::vector<bool>& kvectorVector)
{
    std::vector<bool> vbResultVector(uDimension, false);
    
    for (std::size_t uRow = 0; uRow < uDimension; ++uRow)
    {
        const std::size_t kuMatrixStartRowLinearIndex = ConvertToLinearIndex(uDimension, uRow, 0);
        
        for (std::size_t uRowColumn = 0; uRowColumn < uDimension; ++uRowColumn)
        {
            if ( kvectorMatrix[kuMatrixStartRowLinearIndex + uRowColumn] && kvectorVector[uRowColumn] )
                vbResultVector[uRow] = ! vbResultVector[uRow];
        }
    }
    
    return vbResultVector;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool
DAF::LightsOutSolver::VectorVectorScalarProduct(std::size_t uDimension,
                                                const std::vector<bool>& kvectorVectorX,
                                                const std::vector<bool>& kvectorVectorY)
{
    bool bResult = false;
    
    for (std::size_t uIndex = 0; uIndex < uDimension; ++uIndex)
    {
        if ( kvectorVectorX[uIndex] && kvectorVectorY[uIndex] )
            bResult = ! bResult;
    }
    
    return bResult;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
std::vector<bool>
DAF::LightsOutSolver::VectorVectorSum(std::size_t uDimension,
                                      const std::vector<bool>& vbVectorX,
                                      const std::vector<bool>& vbVectorY)
{
    std::vector<bool> vbResultVector(uDimension);
    
    for (std::size_t uIndex = 0; uIndex < uDimension; ++uIndex)
        vbResultVector[uIndex] = vbVectorX[uIndex] ^ vbVectorY[uIndex];
    
    return vbResultVector;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
std::vector<bool>
DAF::LightsOutSolver::IdentityMatrix()
{
    const std::size_t kuIdentityDimension = _uDimension * _uDimension;
    const std::size_t kuVectorSize = kuIdentityDimension * kuIdentityDimension;
    std::vector<bool> vbIdentityMatrix(kuVectorSize, false);
    
    for (std::size_t uRowColumn = 0; uRowColumn < kuIdentityDimension; ++uRowColumn)
    {
        const std::size_t kuLinearIndex = ConvertToLinearIndex(kuIdentityDimension, uRowColumn, uRowColumn);
        vbIdentityMatrix[kuLinearIndex] = true;
    }
    
    return vbIdentityMatrix;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
std::vector<bool>
DAF::LightsOutSolver::GameMatrix()
{
    const std::size_t kuGameDimension = _uDimension * _uDimension;
    const std::size_t kuVectorSize = kuGameDimension * kuGameDimension;
    std::vector<bool> vbGameMatrix(kuVectorSize, false);
    
    for (std::size_t uBlockRow = 0; uBlockRow < _uDimension; ++uBlockRow)
    {
        const std::size_t kuStartRow = uBlockRow * _uDimension;
        
        for (std::size_t uBlockColumn = 0; uBlockColumn < _uDimension; ++uBlockColumn)
        {
            const std::size_t kuStartColumn = uBlockColumn * _uDimension;
            
            if ( uBlockRow == uBlockColumn )
            {
                for (std::size_t uRow = 0; uRow < _uDimension; ++uRow)
                {
                    for (std::size_t uColumn = 0; uColumn < _uDimension; ++uColumn)
                    {
                        const std::size_t kuLinearIndex = ConvertToLinearIndex(kuGameDimension, kuStartRow + uRow, kuStartColumn + uColumn);
                        vbGameMatrix[kuLinearIndex] = true;
                    }
                }
            }
            else
            {
                for (std::size_t uRow = 0; uRow < _uDimension; ++uRow)
                {
                    for (std::size_t uColumn = 0; uColumn < _uDimension; ++uColumn)
                    {
                        if ( uRow == uColumn )
                        {
                            const std::size_t kuLinearIndex = ConvertToLinearIndex(kuGameDimension, kuStartRow + uRow, kuStartColumn + uColumn);
                            vbGameMatrix[kuLinearIndex] = true;
                        }
                    }
                }
            }
        }
    }
    
    return vbGameMatrix;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
DAF::LightsOutSolver::ReduceGameMatrix()
{
    _vbElementaryOperationMatrix = IdentityMatrix();
    _vbReducedGameMatrix = GameMatrix();
    
    const std::size_t kuGameDimension = _uDimension * _uDimension;
    
    std::size_t uNextRow = 0;
    for (std::size_t uColumn = 0; uColumn < kuGameDimension; ++uColumn)
    {
        for (std::size_t uRow = uNextRow; uRow < kuGameDimension; ++uRow)
        {
            const std::size_t kuLinearIndex = ConvertToLinearIndex(kuGameDimension, uRow, uColumn);
            if ( _vbReducedGameMatrix[kuLinearIndex] )
            {
                SwapRows(kuGameDimension, _vbElementaryOperationMatrix, uNextRow, uRow);
                SwapRows(kuGameDimension, _vbReducedGameMatrix, uNextRow, uRow);
                
                for (size_t uClearRow = 0; uClearRow < kuGameDimension; ++uClearRow)
                {
                    if ( uClearRow == uNextRow )
                        continue;
                    
                    const size_t kuClearLinearIndex = ConvertToLinearIndex(kuGameDimension, uClearRow, uColumn);
                    if ( _vbReducedGameMatrix[kuClearLinearIndex] )
                    {
                        AddRowToRow(kuGameDimension, _vbElementaryOperationMatrix, uNextRow, uClearRow);
                        AddRowToRow(kuGameDimension, _vbReducedGameMatrix, uNextRow, uClearRow);
                    }
                }
                
                ++uNextRow;
                
                break;
            }
        }
    }
    
    FindNullSpaceBasis();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
DAF::LightsOutSolver::FindNullSpaceBasis()
{
    const std::size_t kuGameDimension = _uDimension * _uDimension;
    
    std::size_t uNextRow = 0;
    for (std::size_t uColumn = 0; uColumn < kuGameDimension; ++uColumn)
    {
        const std::size_t kuLinearIndex = ConvertToLinearIndex(kuGameDimension, uNextRow, uColumn);
        if ( _vbReducedGameMatrix[kuLinearIndex] )
        {
            ++uNextRow;
        }
        else
        {
            std::vector<bool> _vbBasisVector(kuGameDimension);
            for (std::size_t uRow = 0; uRow < kuGameDimension; ++uRow)
            {
                const std::size_t kuBasisLinearIndex = ConvertToLinearIndex(kuGameDimension, uRow, uColumn);
                _vbBasisVector[uRow] = _vbReducedGameMatrix[kuBasisLinearIndex];
            }
            
            _vbBasisVector[uColumn] = true;
            
            _vvbGameMatrixNullSpaceBasis.push_back(_vbBasisVector);
        }
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
std::string
DAF::LightsOutSolver::PrintMatrix(std::size_t uDimension, const std::vector<bool>& kvectorMatrix)
{
    std::string szResult;
    
    for (std::size_t uRow = 0; uRow < uDimension; ++uRow)
    {
        for (std::size_t uColumn = 0; uColumn < uDimension; ++uColumn)
        {
            const std::size_t kuLinearIndex = ConvertToLinearIndex(uDimension, uRow, uColumn);
            if ( kvectorMatrix[kuLinearIndex] )
            {
                szResult += "1 ";
            }
            else
            {
                szResult += "0 ";
            }
        }
        
        szResult += "\n";
    }
    
    return szResult;
}
