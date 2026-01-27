using System;
namespace Scripting.Utils
{
    public static class MathUtils
    {
        public static float Clamp01(this float value)
        {
            return Clamp(value, 0, 1);
        }

        public static float Clamp(this float value, float min, float max)
        {
            return Math.Max(Math.Min(value, max), min);
        }
    }
}
