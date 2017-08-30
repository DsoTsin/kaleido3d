#pragma once

#ifndef __k3d_Geometry_h__
#define __k3d_Geometry_h__

#include "Matrix.h"

namespace k3d
{
    namespace math
    {
        using Vec3f = TVector<float, 3>;
        using Vec4f = TVector<float, 4>;
       
        const float EPSILON = 0.0001f;

        // Available for SIMD acceralation
        KALIGN(16) class Plane
        {
        public:
            enum Face { COPLANAR, FRONT, BACK };

            Plane() [[simd]]
            {
                Data = { 0.f, 1.f, 0.f, 0 };
            }

            Plane(const Vec3f& normal, const Vec3f& position)
            {
                Normal = normal;
                Dist = -DotProduct(normal, position);
            }

            Plane(Vec4f const& Param) [[simd]]
            {
                Data = Param;
            }

            ~Plane() {}

            static Plane ConstructFromPoints(const Vec3f& v1, const Vec3f& v2, const Vec3f& v3)
            {
                Plane temp;
                Vec3f e1 = v2 - v1;
                Vec3f e2 = v3 - v1;
                temp.Normal = Normalize(CrossProduct(e1, e2));
                temp.Dist = -DotProduct(temp.Normal, v1);
                return temp;
            }

            float GetDistance(const Vec3f& p)
            {
                return DotProduct(Normal, p) + Dist;
            }

            Face Classify(const Vec3f& p)
            {
                float res = GetDistance(p);
                if (res > EPSILON)
                    return FRONT;
                else if (res < EPSILON)
                    return BACK;
                else
                    return COPLANAR;
            }

            union 
            {
                struct
                {
                    Vec3f Normal;
                    float Dist;
                };
                Vec4f Data;
            };
        }; // !Plane

        //! Sphere
        KALIGN(16) class Sphere
        {
        public:
            Sphere()
            {
                Data = { 0.f,0.f,0.f,0.f };
            }

            Sphere(Vec3f _Position, float _Radius)
            {
                Position = _Position;
                Radius = _Radius;
            }

            // Available For SIMD Acceralation
            explicit Sphere(Vec4f const& PosAndRad) [[simd]]
            {
                Data = PosAndRad;
            }

            union
            {
                struct
                {
                    Vec3f Position;
                    float Radius;
                };
                Vec4f Data;
            };
        }; //! Sphere

        //! AABB
        KALIGN(16) class AABB
        {
        public:
            AABB() {}
            AABB(Vec4f const& _Min, Vec4f const& _Max) : Min(_Min), Max(_Max) {}

            Vec4f Min;
            Vec4f Max;
            
            //! Available For SIMD Acceralation
            AABB& operator |= (AABB const & rhs) [[simd]]
            {
                Min = Minimize<float, 4>(rhs.Min, Min);
                Max = Maximize<float, 4>(rhs.Max, Max);
                return *this;
            }
        };

    }
}

#endif