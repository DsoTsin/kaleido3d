/***********************************************
 *  Kaleido3D Math Library (Inverse Kinematics Solver)
 *  Implements Jacobian, CCD and FABRIK
 *  Author  : Qin Zhou
 *  Date    : 2017/2/18
 *  Email   : dsotsen@gmail.com
 ***********************************************/
#include "kMath.hpp"

NS_MATHLIB_BEGIN

class Joint
{
private:
    tMatrixNxN<float, 4>    m_LocalMatrix;
    tMatrixNxN<float, 4>    m_WorldMatrix;
    tVectorN<float, 3>      m_Direction;
    tVectorN<float, 3>      m_Dof;
};

class IKSolver
{
    
};

class CCDIKSolver : public IKSolver
{
    
};

class FABRIKSolver : public IKSolver
{

};

NS_MATHLIB_END
