using Scripting.Utils;
using System;
using System.Runtime.InteropServices;

namespace Core
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector3 : IEquatable<Vector3>
    {
        public float x;
        public float y;
        public float z;

        public static readonly Vector3 zero = new Vector3(0, 0, 0);
        public static readonly Vector3 one = new Vector3(1, 0, 0);
        public static readonly Vector3 left = new Vector3(-1, 0, 0);
        public static readonly Vector3 right = new Vector3(1, 0, 0);
        public static readonly Vector3 up = new Vector3(0, 1, 0);
        public static readonly Vector3 down = new Vector3(0, -1, 0);
        public static readonly Vector3 forward = new Vector3(0, 0, 1);
        public static readonly Vector3 back= new Vector3(0, 0, -1);

        public Vector3(float x, float y, float z)
        {
            this.x = x;    
            this.y = y;    
            this.z = z;    
        }

        public float magnitude => (float)Math.Sqrt(x * x + y * y + z * z);

        // Квадрат длины вектора (используется для оптимизации сравнений)
        public float sqrMagnitude => x * x + y * y + z * z;

        // Нормализованный вектор
        public Vector3 normalized
        {
            get
            {
                if (magnitude > float.Epsilon)
                    return new Vector3(x / magnitude, y / magnitude, z / magnitude);
                return zero;
            }
        }

        public void Normalize()
        {
            if (magnitude > float.Epsilon)
            {
                x /= magnitude;
                y /= magnitude;
                z /= magnitude;
            }
            else
            {
                x = y = z = 0;
            }
        }

        public static float Distance(Vector3 a, Vector3 b)
        {
            float dx = a.x - b.x;
            float dy = a.y - b.y;
            float dz = a.z - b.z;

            return (float)Math.Sqrt(dx * dx + dy * dy + dz * dz);
        }

        public static float Dot(Vector3 a, Vector3 b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }

        public static Vector3 Cross(Vector3 a, Vector3 b)
        {
            return new Vector3(
                a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x
            );
        }

        public static Vector3 Lerp(Vector3 a, Vector3 b, float t)
        {
            t = MathUtils.Clamp01(t);

            return new Vector3(
                a.x + (b.x - a.x) * t,
                a.y + (b.y - a.y) * t,
                a.z + (b.z - a.z) * t
            );
        }

        public static Vector3 LerpUnclamped(Vector3 a, Vector3 b, float t)
        {
            return new Vector3(
                a.x + (b.x - a.x) * t,
                a.y + (b.y - a.y) * t,
                a.z + (b.z - a.z) * t
            );
        }

        public static Vector3 Slerp(Vector3 a, Vector3 b, float t)
        {
            t = MathUtils.Clamp01(t);

            float dot = Dot(a.normalized, b.normalized);
            dot = MathUtils.Clamp(dot, -1.0f, 1.0f);

            float theta = (float)Math.Acos(dot) * t;
            Vector3 relativeVec = b - a * dot;
            relativeVec.Normalize();

            return a * (float)Math.Cos(theta) + relativeVec * (float)Math.Sin(theta);
        }

        public static Vector3 ClampMagnitude(Vector3 vector, float maxLength)
        {
            return vector.sqrMagnitude > maxLength * maxLength ?
                vector.normalized * maxLength :
                vector;
        }

        public static Vector3 Reflect(Vector3 inDirection, Vector3 inNormal)
        {
            float factor = -2f * Dot(inNormal, inDirection);
            return new Vector3(
                factor * inNormal.x + inDirection.x,
                factor * inNormal.y + inDirection.y,
                factor * inNormal.z + inDirection.z
            );
        }

        public static Vector3 Project(Vector3 vector, Vector3 onNormal)
        {
            float normalSqrMagnitude = onNormal.sqrMagnitude;
            if (normalSqrMagnitude < float.Epsilon)
                return zero;

            float dot = Dot(vector, onNormal);
            return new Vector3(
                onNormal.x * dot / normalSqrMagnitude,
                onNormal.y * dot / normalSqrMagnitude,
                onNormal.z * dot / normalSqrMagnitude
            );
        }

        public static Vector3 ProjectOnPlane(Vector3 vector, Vector3 planeNormal)
        {
            return vector - Project(vector, planeNormal);
        }

        public static Vector3 Min(Vector3 a, Vector3 b)
        {
            return new Vector3(
                Math.Min(a.x, b.x),
                Math.Min(a.y, b.y),
                Math.Min(a.z, b.z)
            );
        }

        public static Vector3 Max(Vector3 a, Vector3 b)
        {
            return new Vector3(
                Math.Max(a.x, b.x),
                Math.Max(a.y, b.y),
                Math.Max(a.z, b.z)
            );
        }

        public static Vector3 operator +(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
        }

        public static Vector3 operator -(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
        }

        public static Vector3 operator -(Vector3 a)
        {
            return new Vector3(-a.x, -a.y, -a.z);
        }

        public static Vector3 operator *(Vector3 a, float d)
        {
            return new Vector3(a.x * d, a.y * d, a.z * d);
        }

        public static Vector3 operator *(float d, Vector3 a)
        {
            return a * d;
        }

        public static Vector3 operator *(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x * b.x, a.y * b.y, a.z * b.z);
        }

        public static Vector3 operator /(Vector3 a, float d)
        {
            if (Math.Abs(d) < float.Epsilon)
                throw new DivideByZeroException("Division by zero in Vector3");

            return new Vector3(a.x / d, a.y / d, a.z / d);
        }

        public static Vector3 operator /(Vector3 a, Vector3 b)
        {
            if (Math.Abs(b.x) < float.Epsilon || Math.Abs(b.y) < float.Epsilon || Math.Abs(b.z) < float.Epsilon)
                throw new DivideByZeroException("Division by zero in Vector3 component");

            return new Vector3(a.x / b.x, a.y / b.y, a.z / b.z);
        }

        public static bool operator ==(Vector3 lhs, Vector3 rhs)
        {
            return (lhs - rhs).sqrMagnitude < 9.99999944E-11f;
        }

        public static bool operator !=(Vector3 lhs, Vector3 rhs)
        {
            return !(lhs == rhs);
        }

        public override bool Equals(object obj)
        {
            return obj is Vector3 vector && Equals(vector);
        }

        public bool Equals(Vector3 other)
        {
            return this == other;
        }

        public override int GetHashCode()
        {
            return x.GetHashCode() ^ (y.GetHashCode() << 2) ^ (z.GetHashCode() >> 2);
        }

        public override string ToString()
        {
            return $"({x:F2}, {y:F2}, {z:F2})";
        }

        public string ToString(string format)
        {
            return $"({x.ToString(format)}, {y.ToString(format)}, {z.ToString(format)})";
        }

        public static float Angle(Vector3 from, Vector3 to)
        {
            float denominator = (float)Math.Sqrt(from.sqrMagnitude * to.sqrMagnitude);
            if (denominator < float.Epsilon)
                return 0f;

            float dot = MathUtils.Clamp(Dot(from, to) / denominator, -1f, 1f);
            return (float)Math.Acos(dot) * (180f / (float)Math.PI);
        }
    }
}
