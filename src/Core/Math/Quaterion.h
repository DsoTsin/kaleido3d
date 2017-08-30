#pragma once
#ifndef __k3d_Quaterion_h__
#define __k3d_Quaterion_h__

#include "Matrix.h"

namespace k3d
{
    namespace math
    {
        //! \class  Quaternion
        //! \tparam T Generic data type parameter.
        template <typename T>
        class Quaternion
        {
        public:
            typedef T value_type;

            Quaternion() {}

            Quaternion(const T axis_angle, const TVector<T, 3> & axis)
            {
                m_data[0] = T(::cos(ToRadian(axis_angle) / 2.0));
                m_data[1] = axis[0] * T(::sin(axis_angle / 2.0));
                m_data[2] = axis[1] * T(::sin(axis_angle / 2.0));
                m_data[3] = axis[2] * T(::sin(axis_angle / 2.0));
            }

            explicit Quaternion(const TVector<T, 4> & vec4)
            {
                SetValue(vec4);
            }

            Quaternion(T _w, T _x, T _y, T _z)
                : w(_w), x(_x), y(_y), z(_z)
            {
            }

            KFORCE_INLINE void SetValue(const TVector<T, 4> & vec)
            {
                m_data[0] = vec[0];
                m_data[1] = vec[1];
                m_data[2] = vec[2];
                m_data[3] = vec[3];
            }

            /**
            * \fn  KFORCE_INLINE Quaternion & operator*= (const Quaternion & other)
            * \brief Grassmann Product
            * \param [in] another quaternion.
            * \return  The result of the operation.
            */
            KFORCE_INLINE Quaternion & operator *= (const Quaternion & p)
            {
                T t1 = w * p.w - x * p.x - y * p.y - z * p.z;
                T t2 = w * p.x + x * p.w + y * p.z - z * p.y;
                T t3 = w * p.y + y * p.w + z * p.x - x * p.z;
                T t4 = w * p.z + z * p.w + x * p.y - y * p.x;
                this->x = t1; this->y = t2; this->z = t3; this->z = t4;
                return *this;
            }

            //! \fn KFORCE_INLINE Quaternion Conjugate()
            //! \brief  Gets the conjugate.
            //! \return A Quaternion.
            KFORCE_INLINE Quaternion Conjugate() { return Quaternion(w, -x, -y, -z); }

            //! \fn Quaternion Inverse()
            //! \brief  Gets the inverse.
            //! \return A Quaternion.
            Quaternion Inverse() { return Conjugate() / DotProduct(*this, *this); }

            //! \fn TMatrixN<T, 4> Rotate( T axis_angle, const TVector<T,3> & axis );
            //! \brief  Rotates quaternion with axis by angle
            //! \param  axis_angle  The axis angle float in degrees.
            //! \param  axis        The axis Vec3f.
            //! \return A TMatrixN&lt;T,4&gt;
            TMatrixN<T, 4> Rotate(T axis_angle, const TVector<T, 3> & axis)
            {
                TVector<T, 3> Tmp = axis;
                typename Quaternion<T>::value_type len = Tmp.Length();
                if (abs(len - T(1)) > T(0.001))
                {
                    T oneOverLen = T(1) / len;
                    Tmp.x *= oneOverLen;
                    Tmp.y *= oneOverLen;
                    Tmp.z *= oneOverLen;
                }
                typename Quaternion<T>::value_type const Sin = ::sin(ToRadian(axis_angle) * T(0.5));

                return  (*this) * Quaternion<T>(cos(axis_angle * T(0.5)), Tmp.x * Sin, Tmp.y * Sin, Tmp.z * Sin);
            }

            //! \fn TVector<T, 3> ToEulerAngles()
            //! \brief  Converts Quaternion to an euler angles.
            //! \return Euler Angles as Vec3;
            TVector<T, 3> ToEulerAngles()
            {
                T _pitch = ToDegree(::atan2(value_type(2) * (y * z + w * x), w * w - x * x - y * y + z * z));
                T _yaw = ToDegree(::asin(value_type(-2) * (x * z - w * y)));
                T _roll = ToDegree(::atan2(value_type(2) * (x * y + w * z), w * w + x * x - y * y - z * z));
                return TVector<T, 3>(_pitch, _yaw, _roll);
            }

            //! \fn TMatrixN<T, 4> AsMatrix();
            //! \brief  Converts this Quaternion to a Mat4f.
            //! \return A TMatrixN&lt;T,4&gt;
            KFORCE_INLINE TMatrixN<T, 4> AsMatrix()
            {
                TMatrixN<T, 4> Result;
                Result[0][0] = T(1) - 2 * y * y - 2 * z * z;
                Result[0][1] = T(2) * x * y + 2 * w * z;
                Result[0][2] = T(2) * x * z - 2 * w * y;

                Result[1][0] = T(2) * x * y - 2 * w * z;
                Result[1][1] = T(1) - T(2) * x * x - 2 * z * z;
                Result[1][2] = T(2) * y * z + 2 * w * x;

                Result[2][0] = T(2) * x * z + 2 * w * y;
                Result[2][1] = T(2) * y * z - 2 * w * x;
                Result[2][2] = T(1) - 2 * x * x - 2 * y * y;

                Result[3][3] = T(1);
                return Result;
            }

            operator const TMatrixN<T, 4>() const
            {
                return AsMatrix();
            }

            static TMatrixN<T, 4> YawPitchRoll(T yaw, T pitch, T roll)
            {
                T tmp_ch = ::cos(yaw);
                T tmp_sh = ::sin(yaw);
                T tmp_cp = ::cos(pitch);
                T tmp_sp = ::sin(pitch);
                T tmp_cb = ::cos(roll);
                T tmp_sb = ::sin(roll);

                TMatrixN<T, 4> Result;
                Result[0][0] = tmp_ch * tmp_cb + tmp_sh * tmp_sp * tmp_sb;
                Result[0][1] = tmp_sb * tmp_cp;
                Result[0][2] = -tmp_sh * tmp_cb + tmp_ch * tmp_sp * tmp_sb;
                Result[1][0] = -tmp_ch * tmp_sb + tmp_sh * tmp_sp * tmp_cb;
                Result[1][1] = tmp_cb * tmp_cp;
                Result[1][2] = tmp_sb * tmp_sh + tmp_ch * tmp_sp * tmp_cb;
                Result[2][0] = tmp_sh * tmp_cp;
                Result[2][1] = -tmp_sp;
                Result[2][2] = tmp_ch * tmp_cp;
                Result[3][3] = T(1);
                return Result;
            }

            static Quaternion Slerp(const Quaternion & p, const Quaternion &q, value_type t)
            {
                value_type theta = q.w - p.w;
                value_type factor1 = ::sin(value_type(1.0 - t)*theta);
                value_type factor2 = ::sin(t*theta);
                value_type factor3 = ::sin(theta);
                return Quaternion((factor1*p.w + factor2*q.w) / factor3, (factor1*p.x + factor2*q.x) / factor3, (factor1*p.y + factor2*q.y) / factor3, (factor1*p.z + factor2*q.z) / factor3);
            }

        private:
            union {
                T m_data[4];
                struct {
                    T w, x, y, z;
                };
            };
        };
    }
}

#endif
