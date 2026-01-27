namespace Core
{
    public struct Vector2
    {
        public float x;
        public float y;

        public static readonly Vector2 zero = new Vector2(0, 0);
        public static readonly Vector2 one = new Vector2(1, 0);
        public static readonly Vector2 left = new Vector2(-1, 0);
        public static readonly Vector2 right = new Vector2(1, 0);
        public static readonly Vector2 up = new Vector2(0, 1);
        public static readonly Vector2 down = new Vector2(0, -1);

        public Vector2(float x, float y)
        {
            this.x = x;
            this.y = y;
        }

        public override string ToString()
        {
            return $"({x:F2}, {y:F2})";
        }
    }
}
